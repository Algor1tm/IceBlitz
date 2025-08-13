// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseSkaterCharacter.h"
#include "Puck.h"
#include "PlayerCamera.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "EnhancedInputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#define ECC_CursorTrace ECC_GameTraceChannel2
#define STOP_SPEED 0.1f
#define STOP_TOLERANCE 5.f
#define STEAL_INTERSECTIONS 4


// Sets default values
ABaseSkaterCharacter::ABaseSkaterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	EnableOrientRotationToMovement(false);

	StickMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stick"));
	StickMesh->SetupAttachment(GetMesh());
	StickMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StickPickUpTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("StickPickUpTrigger"));
	StickPickUpTrigger->SetupAttachment(StickMesh);

	MeshPickUpTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("MeshPickUpTrigger"));
	MeshPickUpTrigger->SetupAttachment(GetMesh());

	StickStealTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("StickStealTrigger"));
	StickStealTrigger->SetupAttachment(StickMesh);

	MeshStealTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("MeshStealTrigger"));
	MeshStealTrigger->SetupAttachment(GetMesh());
}

void ABaseSkaterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->GroundFriction = GroundFriction;
	GetCharacterMovement()->BrakingFriction = BrakingFriction;
	GetCharacterMovement()->MaxWalkSpeed = MaxSkateSpeed;
	GetCharacterMovement()->MaxAcceleration = MaxAcceleration;
	GetCharacterMovement()->RotationRate = FRotator(0.f, RotationSpeed, 0.f);
	EnableOrientRotationToMovement(bOrientRotationToMovement);

	StickPickUpTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnPuckPickUp);
	MeshPickUpTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnPuckPickUp);

	StickStealTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnStealRangeBegin);
	StickStealTrigger->OnComponentEndOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnStealRangeEnd);

	MeshStealTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnStealRangeBegin);
	MeshStealTrigger->OnComponentEndOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnStealRangeEnd);
}

void ABaseSkaterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving)
	{
		FVector Velocity = GetCharacterMovement()->Velocity;
		FVector Direction = ComputeDirectionTo(MoveDestination);

		float Speed = Velocity.Length();
		float Distance = Direction.Length();

		if (Speed <= MinSpeed && Distance <= MinDistance)
		{
			AddMovementInput(Direction.GetSafeNormal(), STOP_SPEED);

			FVector2D PlanarLocation = FVector2D(GetActorLocation().X, GetActorLocation().Y);
			if (PlanarLocation.Equals(MoveDestination, STOP_TOLERANCE))
			{
				bIsMoving = false;
				GetCharacterMovement()->Velocity = FVector(0.f);
			}
		}
		else
		{
			AddMovementInput(Direction.GetSafeNormal(), SkateSpeed);
		}
	}

	if (bIsCharging)
	{
		float Multipliyer = bFastFill ? FastFillFactor : 1.f;
		ShootCharge += DeltaTime * Multipliyer * ShootChargeSpeed;
		ShootCharge = FMath::Clamp(ShootCharge, 0.f, 1.f);
	}

	if (bIsBoosting)
	{
		GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(MaxSkateSpeed, BoostMaxSkateSpeed, CurrentBoostTime / BoostDuration);
		CurrentBoostTime -= DeltaTime;

		if (CurrentBoostTime <= 0.f)
			bIsBoosting = false;
	}
}

void ABaseSkaterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnMoveInput);
		EnhancedInputComponent->BindAction(StopAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnStopInput);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnShootBeginInput);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &ABaseSkaterCharacter::OnShootEndInput);
		EnhancedInputComponent->BindAction(CameraResetAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnCameraResetInput);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}

void ABaseSkaterCharacter::OnMoveInput()
{
	FVector2D Destination = GetLocationUnderCursor();
	SetMoveDestination(Destination);
}

void ABaseSkaterCharacter::OnStopInput()
{
	Stop();
}

void ABaseSkaterCharacter::OnShootBeginInput()
{
	if (HasPuck())
	{
		ShootBegin();
	}
	else
	{
		TrySteal();
	}
}

void ABaseSkaterCharacter::OnShootEndInput()
{
	if (HasPuck() && bIsCharging)
		ShootEnd();
}

void ABaseSkaterCharacter::OnCameraResetInput()
{
	CenterCamera();
}

void ABaseSkaterCharacter::SetMoveDestination(FVector2D Destination)
{
	bIsMoving = true;
	MoveDestination = Destination;

	if (bOrientRotationToMovement)
	{
		EnableOrientRotationToMovement(true);
	}

	if (bIsCharging)
	{
		bFastFill = false;
		ShootCharge -= MoveShootChargeDecrease;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,
		FXCursor,
		FVector(MoveDestination, 0),
		FRotator::ZeroRotator,
		FVector(1.f, 1.f, 1.f),
		true,
		true,
		ENCPoolMethod::None,
		true);
}

void ABaseSkaterCharacter::Stop()
{
	bIsMoving = false;
	MoveDestination = FVector2D(0.f);

	if(bOrientRotationToMovement)
		EnableOrientRotationToMovement(false);
}

void ABaseSkaterCharacter::CenterCamera()
{
	if (PlayerCamera)
		PlayerCamera->CenterOnPossesedPawn();
}

void ABaseSkaterCharacter::ShootBegin()
{
	ShootCharge = 0.0f;
	bIsCharging = true;
	bFastFill = true;
}

void ABaseSkaterCharacter::ShootEnd()
{
	FVector2D Cursor = GetLocationUnderCursor();
	FVector Direction = ComputeDirectionFromPuckTo(Cursor);
	float DistanceToCursor = Direction.Length();
	Direction = Direction.GetSafeNormal();

	float Power = ComputeShotPower(Direction, DistanceToCursor, GetCharacterMovement()->Velocity);

	DisablePuckPickUpForTime();

	Puck->ReleaseOwner();
	Puck->Shoot(Direction, Power);
	Puck = nullptr;
	bIsCharging = false;

	Stop();
	FaceDirection(Direction);

	PlayShotSound();
}

void ABaseSkaterCharacter::TrySteal()
{
	UGameplayStatics::PlaySound2D(this, StealSound);

	if (StealableCharacters.IsEmpty())
		return;

	ABaseSkaterCharacter* Target = nullptr;
	for (const auto& [Skater, Count] : StealableCharacters)
	{
		if (Skater->GetPuck())
		{
			Target = Skater;
			break;
		}
	}

	if (!Target)
		return;

	APuck* OtherPuck = Target->GetPuck();
	Target->OnPuckStealed();

	Puck = OtherPuck;
	Puck->SetSkaterOwner(this);
}

void ABaseSkaterCharacter::EnableOrientRotationToMovement(bool Enable)
{
	bUseControllerRotationPitch = !Enable;
	bUseControllerRotationYaw = !Enable;
	bUseControllerRotationRoll = !Enable;

	GetCharacterMovement()->bOrientRotationToMovement = Enable;

	if (Controller && !Enable)
		Controller->SetControlRotation(GetActorRotation());
}

void ABaseSkaterCharacter::FaceDirection(const FVector& Direction)
{
	if (Controller && !Direction.IsNearlyZero())
	{
		FRotator Rotation = Direction.Rotation();
		Controller->SetControlRotation(Rotation);
	}
}

float ABaseSkaterCharacter::ComputeShotPower(const FVector& Direction, float DistanceToCursor, const FVector& SkaterVelocity) const
{
	float SpeedFactor = (FVector::DotProduct(SkaterVelocity.GetSafeNormal(), Direction) + 1) * 0.5f;
	float Speed = SkaterVelocity.Length() * SpeedFactor;

	float NormalizedCharge = FMath::Clamp(ShootCharge, 0.01f, 1.f);
	float NormalizedDistanceToCursor = FMath::Clamp(DistanceToCursor / ShootMaxDistanceToCursor, 0.f, 1.f);
	float NormalizedSpeed = FMath::Clamp(Speed / ShootMaxSkaterSpeed, 0.f, 1.f);

	float TotalModifiers = ShootChargeFactor * NormalizedCharge + ShootDistanceToCursorFactor * NormalizedDistanceToCursor + ShootSpeedFactor * NormalizedSpeed;
	float Power = ShootBasePower * TotalModifiers;

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("SHOOT, POWER = %f, DistanceToCursor = %f, SpeedFactor = %f, Direction = %s"), Power, DistanceToCursor, SpeedFactor, *(Direction.ToString())));

	return Power;
}

void ABaseSkaterCharacter::PlayShotSound()
{
	if(ShootCharge <= 0.3f)
		UGameplayStatics::PlaySound2D(this, ShotSound, 0.4f);
	else if (ShootCharge <= 0.7f)
		UGameplayStatics::PlaySound2D(this, ShotSound, 0.7f);
	else
		UGameplayStatics::PlaySound2D(this, ShotSound, 1.f);

	//if(ShootCharge >= 0.85f)
	//	UGameplayStatics::PlaySound2D(this, PowerShotSound);
}

void ABaseSkaterCharacter::OnBoostBegin()
{
	CurrentBoostTime = BoostDuration;
	bIsBoosting = true;

	GetCharacterMovement()->MaxWalkSpeed = BoostMaxSkateSpeed;
	float BoostSpeed = BoostMaxSkateSpeed - MaxSkateSpeed;

	GetCharacterMovement()->Velocity += GetActorForwardVector() * BoostSpeed;

	UGameplayStatics::PlaySound2D(this, BoostSound);
}

void ABaseSkaterCharacter::OnPuckPickUp(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APuck* PuckActor = Cast<APuck>(OtherActor);

	if (PuckActor && !PuckActor->HasOwner())
	{
		Puck = PuckActor;
		Puck->SetSkaterOwner(this);
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("PUCK PICK UP"));
	}
}

void ABaseSkaterCharacter::DisablePuckPickUpForTime()
{
	StickPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorld()->GetTimerManager().SetTimer(
		PickUpTimerHandle,
		this,
		&ABaseSkaterCharacter::EnablePuckPickUp,
		PuckPickUpCooldown,
		false
	);
}

void ABaseSkaterCharacter::EnablePuckPickUp()
{
	StickPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ABaseSkaterCharacter::OnStealRangeBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABaseSkaterCharacter* SkaterActor = Cast<ABaseSkaterCharacter>(OtherActor);

	if (!IsValid(SkaterActor))
		return;

	uint8& OverlapCount = StealableCharacters.FindOrAdd(SkaterActor, 0);
	OverlapCount++;

	if (OverlapCount > STEAL_INTERSECTIONS)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Invalid stealable characters map!"));
		return;
	}
}

void ABaseSkaterCharacter::OnStealRangeEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABaseSkaterCharacter* SkaterActor = Cast<ABaseSkaterCharacter>(OtherActor);

	if (!IsValid(SkaterActor))
		return;

	uint8* OverlapCountPtr = StealableCharacters.Find(SkaterActor);

	if (OverlapCountPtr == nullptr || *OverlapCountPtr == 0 || *OverlapCountPtr > STEAL_INTERSECTIONS)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Invalid stealable characters map!"));
		return;
	}

	*OverlapCountPtr -= 1;

	if (*OverlapCountPtr == 0)
	{
		StealableCharacters.Remove(SkaterActor);
	}
}

void ABaseSkaterCharacter::OnPuckStealed()
{
	Puck = nullptr;
	bIsCharging = false;
}

FVector2D ABaseSkaterCharacter::GetLocationUnderCursor() const
{
	TObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	FHitResult Hit;
	bool bHitSuccessful = PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_CursorTrace, true, Hit);

	FVector2D Result;
	if (bHitSuccessful)
	{
		Result = FVector2D(Hit.Location.X, Hit.Location.Y);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Failed to find location under cursor!"));
		FVector ActorLocation = GetActorLocation();
		Result = FVector2D(ActorLocation.X, ActorLocation.Y);
	}

	return Result;
}

FVector ABaseSkaterCharacter::ComputeDirectionTo(FVector2D Location) const
{
	FVector ActorLocation = GetActorLocation();
	FVector2D ActorPlanarLocation = FVector2D(ActorLocation.X, ActorLocation.Y);

	FVector2D PlanarWorldDirection = Location - ActorPlanarLocation;
	FVector Direction = FVector(PlanarWorldDirection, 0);

	return Direction;
}

FVector ABaseSkaterCharacter::ComputeDirectionFromPuckTo(FVector2D Location) const
{
	if (!Puck)
		return FVector(0);

	FVector ActorLocation = Puck->GetActorLocation();
	FVector2D ActorPlanarLocation = FVector2D(ActorLocation.X, ActorLocation.Y);

	FVector2D PlanarWorldDirection = Location - ActorPlanarLocation;
	FVector Direction = FVector(PlanarWorldDirection, 0);

	return Direction;
}

bool ABaseSkaterCharacter::HasPuck() const
{
	return Puck != nullptr;
}

APuck* ABaseSkaterCharacter::GetPuck() const
{
	return Puck;
}

UStaticMeshComponent* ABaseSkaterCharacter::GetStickMeshComponent() const
{
	return StickMesh;
}

void ABaseSkaterCharacter::SetPlayerCamera(APlayerCamera* Camera)
{
	PlayerCamera = Camera;
}
