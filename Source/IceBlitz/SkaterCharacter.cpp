// Fill out your copyright notice in the Description page of Project Settings.


#include "SkaterCharacter.h"
#include "Puck.h"
#include "PlayerCamera.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "EnhancedInputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#define ECC_CursorTrace ECC_GameTraceChannel2


ASkaterCharacter::ASkaterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	EnableOrientRotationToMovement(false);
	MoveDestination = FVector2D(0);
	bIsMoving = false;

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

void ASkaterCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->GroundFriction = GroundFriction;
	GetCharacterMovement()->BrakingFriction = BrakingFriction;
	GetCharacterMovement()->MaxWalkSpeed = MaxSkateSpeed;
	GetCharacterMovement()->MaxAcceleration = MaxAcceleration;
	GetCharacterMovement()->RotationRate = FRotator(0.f, RotationSpeed, 0.f);

	StickPickUpTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASkaterCharacter::OnPuckPickUp);
	MeshPickUpTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASkaterCharacter::OnPuckPickUp);

	StickStealTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASkaterCharacter::OnStealRangeBegin);
	StickStealTrigger->OnComponentEndOverlap.AddDynamic(this, &ASkaterCharacter::OnStealRangeEnd);

	MeshStealTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASkaterCharacter::OnStealRangeBegin);
	MeshStealTrigger->OnComponentEndOverlap.AddDynamic(this, &ASkaterCharacter::OnStealRangeEnd);
}

void ASkaterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving)
	{
		FVector Direction = ComputeDirectionTo(MoveDestination);
		AddMovementInput(Direction.GetSafeNormal(), SkateSpeed);
	}

	if (bIsCharging)
	{
		if(bFastFill)
			ShootCharge += FastFillFactor * DeltaTime * ShootChargeSpeed;
		else
			ShootCharge += DeltaTime * ShootChargeSpeed;

		ShootCharge = FMath::Clamp(ShootCharge, 0.f, 1.f);
	}
}

void ASkaterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnMove);
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnSlide);
		EnhancedInputComponent->BindAction(StopAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnStop);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnShootBeginInput);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &ASkaterCharacter::OnShootEndInput);
		EnhancedInputComponent->BindAction(CameraResetAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnCameraReset);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}

void ASkaterCharacter::OnMove(const FInputActionValue& Value)
{
	bIsMoving = true;
	MoveDestination = GetLocationUnderCursor();
	EnableOrientRotationToMovement(true);

	if (bIsCharging)
	{
		bFastFill = false;
		ShootCharge -= MoveShootChargeDecrease;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, FVector(MoveDestination, 0), FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
}

void ASkaterCharacter::OnSlide()
{
	GetCharacterMovement()->Velocity *= (1.f - SlideIntensity);
	//UGameplayStatics::PlaySound2D(this, SlideSound);
}

void ASkaterCharacter::OnStop()
{
	bIsMoving = false;
	MoveDestination = FVector2D(0.f);
	EnableOrientRotationToMovement(false);
}

void ASkaterCharacter::OnCameraReset()
{
	if (PlayerCamera)
	{
		PlayerCamera->CenterOnPossesedPawn();
	}
}

void ASkaterCharacter::OnShootBeginInput()
{
	if (OwnedPuck)
	{
		ShootBegin();
	}
	else
	{
		TrySteal();
	}
}

void ASkaterCharacter::OnShootEndInput()
{
	if (bIsCharging && OwnedPuck)
		ShootEnd();
}

void ASkaterCharacter::ShootBegin()
{
	ShootCharge = 0.0f;
	bIsCharging = true;
	bFastFill = true;
}

void ASkaterCharacter::ShootEnd()
{
	FVector2D Cursor = GetLocationUnderCursor();
	FVector Direction = ComputeDirectionFromPuckTo(Cursor);
	float DistanceToCursor = Direction.Length();
	Direction = Direction.GetSafeNormal();

	float Power = ComputeShotPower(Direction, DistanceToCursor, GetCharacterMovement()->Velocity);

	DisablePuckPickUpForTime();

	OwnedPuck->ReleaseOwner();
	OwnedPuck->Shoot(Direction, Power);
	OwnedPuck = nullptr;
	bIsCharging = false;

	OnStop();
	FaceDirection(Direction);
}

void ASkaterCharacter::TrySteal()
{
	if (StealableCharacters.IsEmpty())
		return;

	ASkaterCharacter* Target = nullptr;
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

	APuck* Puck = Target->GetPuck();
	Target->OnPuckStealed();

	OwnedPuck = Puck;
	OwnedPuck->SetSkaterOwner(this);
}

float ASkaterCharacter::ComputeShotPower(const FVector& Direction, float DistanceToCursor, const FVector& SkaterVelocity) const
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

void ASkaterCharacter::EnableOrientRotationToMovement(bool Enable)
{
	bUseControllerRotationPitch = !Enable;
	bUseControllerRotationYaw = !Enable;
	bUseControllerRotationRoll = !Enable;

	GetCharacterMovement()->bOrientRotationToMovement = Enable;

	if(Controller && !Enable)
		Controller->SetControlRotation(GetActorRotation());
}

void ASkaterCharacter::FaceDirection(const FVector& Direction)
{
	if (Controller && !Direction.IsNearlyZero())
	{
		FRotator Rotation = Direction.Rotation();
		Controller->SetControlRotation(Rotation);
	}
}

void ASkaterCharacter::SetPlayerCamera(TObjectPtr<APlayerCamera> Camera)
{
	PlayerCamera = Camera;
}

UStaticMeshComponent* ASkaterCharacter::GetStickMeshComponent() const
{
	return StickMesh;
}

void ASkaterCharacter::OnPuckPickUp(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APuck* Puck = Cast<APuck>(OtherActor);

	if (Puck)
	{
		OwnedPuck = Puck;
		OwnedPuck->SetSkaterOwner(this);
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("PUCK PICK UP"));
	}
}

void ASkaterCharacter::DisablePuckPickUpForTime()
{
	StickPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorld()->GetTimerManager().SetTimer(
		PickUpTimerHandle,
		this,
		&ASkaterCharacter::EnablePuckPickUp,
		PuckPickUpCooldown,
		false
	);
}

void ASkaterCharacter::EnablePuckPickUp()
{
	StickPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	GetWorld()->GetTimerManager().ClearTimer(PickUpTimerHandle);
}

void ASkaterCharacter::OnStealRangeBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASkaterCharacter* SkaterActor = Cast<ASkaterCharacter>(OtherActor);

	if (!IsValid(SkaterActor))
		return;

	uint8& OverlapCount = StealableCharacters.FindOrAdd(SkaterActor, 0);
	OverlapCount++;

	if (OverlapCount > 4)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Invalid stealable characters map!"));
		return;
	}
}

void ASkaterCharacter::OnStealRangeEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASkaterCharacter* SkaterActor = Cast<ASkaterCharacter>(OtherActor);

	if (!IsValid(SkaterActor))
		return;

	uint8* OverlapCountPtr = StealableCharacters.Find(SkaterActor);

	if (OverlapCountPtr == nullptr || *OverlapCountPtr == 0 || *OverlapCountPtr > 4)
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

APuck* ASkaterCharacter::GetPuck() const
{
	return OwnedPuck;
}

void ASkaterCharacter::OnPuckStealed()
{
	OwnedPuck = nullptr;
	bIsCharging = false;
}

FVector2D ASkaterCharacter::GetLocationUnderCursor() const
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

FVector ASkaterCharacter::ComputeDirectionTo(FVector2D Location) const
{
	FVector ActorLocation = GetActorLocation();
	FVector2D ActorPlanarLocation = FVector2D(ActorLocation.X, ActorLocation.Y);

	FVector2D PlanarWorldDirection = Location - ActorPlanarLocation;
	FVector Direction = FVector(PlanarWorldDirection, 0);

	return Direction;
}

FVector ASkaterCharacter::ComputeDirectionFromPuckTo(FVector2D Location) const
{
	if (!OwnedPuck)
		return FVector(0);

	FVector ActorLocation = OwnedPuck->GetActorLocation();
	FVector2D ActorPlanarLocation = FVector2D(ActorLocation.X, ActorLocation.Y);

	FVector2D PlanarWorldDirection = Location - ActorPlanarLocation;
	FVector Direction = FVector(PlanarWorldDirection, 0);

	return Direction;
}
