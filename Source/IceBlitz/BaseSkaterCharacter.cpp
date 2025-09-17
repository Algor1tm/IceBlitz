// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseSkaterCharacter.h"
#include "Puck.h"
#include "PlayerCamera.h"
#include "SkaterController.h"
#include "AbilitySystem/SkaterAbility.h"
#include "AbilitySystem/SkaterAttributeSet.h"

#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

#define STOP_SPEED 0.1f
#define STOP_TOLERANCE 5.f


ABaseSkaterCharacter::ABaseSkaterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	StickMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stick"));
	StickMesh->SetupAttachment(GetMesh());
	StickMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetIsReplicated(true);
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	}

	AttributeSet = CreateDefaultSubobject<USkaterAttributeSet>(TEXT("AttributeSet"));

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



	if (HasAuthority())
	{
		StickPickUpTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnPuckPickUp);
		MeshPickUpTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnPuckPickUp);


		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AttributeSet->InitMaxAcceleration(MaxAcceleration);
		AttributeSet->InitMaxSkateSpeed(MaxSkateSpeed);
		AttributeSet->InitSkateSpeed(SkateSpeed);
		AttributeSet->InitShotCharge(0.f);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			USkaterAttributeSet::GetMaxAccelerationAttribute()).AddUObject(this, &ABaseSkaterCharacter::OnMaxAccelerationChanged);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			USkaterAttributeSet::GetMaxSkateSpeedAttribute()).AddUObject(this, &ABaseSkaterCharacter::OnMaxSkateSpeedChanged);

		FGameplayAbilitySpec MoveAbilitySpec(MoveAbility, 1, (uint32)ESkaterAbilityInputID::Move, this);
		AbilitySystemComponent->GiveAbility(MoveAbilitySpec);

		FGameplayAbilitySpec StopAbilitySpec(StopAbility, 1, (uint32)ESkaterAbilityInputID::Stop, this);
		AbilitySystemComponent->GiveAbility(StopAbilitySpec);

		FGameplayAbilitySpec ShootAbilitySpec(ShootAbility, 1, (uint32)ESkaterAbilityInputID::Shoot, this);
		AbilitySystemComponent->GiveAbility(ShootAbilitySpec);
	}
}

void ABaseSkaterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving && IsLocallyControlled())
	{
		FVector Velocity = GetCharacterMovement()->Velocity;
		FVector Direction = ComputeDirectionTo(MoveDestination);

		float Speed = Velocity.Length();
		float Distance = Direction.Length();

		if (Speed <= MinSpeed && Distance <= MinDistance)
		{
			AddMovementInput(Direction.GetSafeNormal(), STOP_SPEED);

			FVector ActorLocation = GetActorLocation();
			FVector2f ActorPlanarLocation = FVector2f(ActorLocation.X, ActorLocation.Y);

			if (ActorPlanarLocation.Equals(MoveDestination, STOP_TOLERANCE))
			{
				bIsMoving = false;
				GetCharacterMovement()->StopMovementImmediately();
			}
		}
		else
		{
			AddMovementInput(Direction.GetSafeNormal(), AttributeSet->GetSkateSpeed());
		}
	}
}

void ABaseSkaterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnMoveInput);
		EnhancedInputComponent->BindAction(StopInputAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnStopInput);
		EnhancedInputComponent->BindAction(BoostInputAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnBoostInput);
		EnhancedInputComponent->BindAction(ShootInputAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnShootInputPressed);
		EnhancedInputComponent->BindAction(ShootInputAction, ETriggerEvent::Completed, this, &ABaseSkaterCharacter::OnShootInputReleased);
	}
}

void ABaseSkaterCharacter::OnMoveInput()
{
	FVector2f CursorTarget = GetCursorTarget();
	FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit();
	TargetData->HitResult.bBlockingHit = true;
	TargetData->HitResult.Location = FVector(CursorTarget.X, CursorTarget.Y, 0.f);

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Add(TargetData);

	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName("AbilityTrigger.Move"));

	FGameplayEventData Payload;
	Payload.Instigator = this;
	Payload.Target = this;
	Payload.TargetData = TargetDataHandle;

	AbilitySystemComponent->HandleGameplayEvent(EventTag, &Payload);
}

void ABaseSkaterCharacter::OnStopInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Stop);
}

void ABaseSkaterCharacter::OnBoostInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Boost);
}

void ABaseSkaterCharacter::OnShootInputPressed()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Shoot);
}

void ABaseSkaterCharacter::OnShootInputReleased()
{
	FGameplayAbilitySpec* ShootSpec = AbilitySystemComponent->FindAbilitySpecFromInputID((uint32)ESkaterAbilityInputID::Shoot);

	if (ShootSpec && ShootSpec->Ability)
	{
		ShootSpec->InputPressed = false;

		// Call RPC without local checking if ability is active
		ServerOnShootInputReleased();
	}
}

void ABaseSkaterCharacter::ServerOnShootInputReleased_Implementation()
{
	FGameplayAbilitySpec* ShootSpec = AbilitySystemComponent->FindAbilitySpecFromInputID((uint32)ESkaterAbilityInputID::Shoot);
	if (ShootSpec && ShootSpec->IsActive())
	{
		AbilitySystemComponent->AbilitySpecInputReleased(*ShootSpec);
	}
}

UAbilitySystemComponent* ABaseSkaterCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABaseSkaterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseSkaterCharacter, bIsMoving);
	DOREPLIFETIME(ABaseSkaterCharacter, MoveDestination);
	DOREPLIFETIME(ABaseSkaterCharacter, Puck);
}

bool ABaseSkaterCharacter::ShootPuck()
{
	if (!Puck)
		return false;

	FVector2f Cursor = GetCursorTarget();
	UE_LOG(LogTemp, Warning, TEXT("cursor X = %f, Y = %f!"), Cursor.X, Cursor.Y);
	FVector Direction = ComputeDirectionFromPuckTo(Cursor);
	float DistanceToCursor = Direction.Length();
	Direction = Direction.GetSafeNormal();

	float Power = ComputeShotPower(Direction, DistanceToCursor, GetCharacterMovement()->Velocity);

	FGameplayTag HasPuckTag = FGameplayTag::RequestGameplayTag(FName("SkaterState.HasPuck"));
	AbilitySystemComponent->RemoveLooseGameplayTag(HasPuckTag);

	DisablePuckPickUpForTime();

	Puck->OnRelease();
	Puck->Shoot(Direction, Power);
	Puck = nullptr;

	ClientStopPostShot(Direction);

	return true;
}

float ABaseSkaterCharacter::ComputeShotPower(const FVector& Direction, float DistanceToCursor, const FVector& SkaterVelocity) const
{
	float VelocityProjection = FVector::DotProduct(SkaterVelocity.GetSafeNormal(), Direction);
	float Speed = SkaterVelocity.Length() * VelocityProjection;

	float Charge = AttributeSet->GetShotCharge();
	float NormDistanceToCursor = FMath::Clamp(DistanceToCursor / ShootMaxDistanceToCursor, 0.f, 1.f);
	float NormalizedSpeed = FMath::Clamp(Speed / ShootMaxSkaterSpeed, -1.f, 1.f);

	float TotalModifiers = ShootChargeFactor * Charge + ShootDistanceToCursorFactor * NormDistanceToCursor + ShootSpeedFactor * NormalizedSpeed;

	float MinPower = ShootBasePower * 0.2f;
	float Power = MinPower + ShootBasePower * TotalModifiers;

	UE_LOG(LogTemp, Warning, TEXT("Shot Power: %f, Speed: %f"), Power, VelocityProjection);

	return Power;
}

void ABaseSkaterCharacter::ClientStopPostShot_Implementation(FVector ShotDirection)
{
	FGameplayAbilitySpec* StopSpec = AbilitySystemComponent->FindAbilitySpecFromInputID((uint32)ESkaterAbilityInputID::Stop);
	if (StopSpec)
		AbilitySystemComponent->TryActivateAbility(StopSpec->Handle);

	FaceDirection(ShotDirection);
}

void ABaseSkaterCharacter::SetMoveDestination(FVector2f Destination)
{
	MoveDestination = Destination;
	bIsMoving = true;

	if (bOrientRotationToMovement)
		EnableOrientRotationToMovement(true);
}

void ABaseSkaterCharacter::StopMovement()
{
	bIsMoving = false;
	MoveDestination = FVector2f(0.f);

	if (bOrientRotationToMovement)
		EnableOrientRotationToMovement(false);
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

void ABaseSkaterCharacter::OnPuckPickUp(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APuck* PuckActor = Cast<APuck>(OtherActor);

	if (PuckActor && !PuckActor->HasOwner() && !Puck)
	{
		Puck = PuckActor;
		Puck->OnPickUp(this);

		FGameplayTag HasPuckTag = FGameplayTag::RequestGameplayTag(FName("SkaterState.HasPuck"));
		AbilitySystemComponent->AddLooseGameplayTag(HasPuckTag);
	}
}

void ABaseSkaterCharacter::DisablePuckPickUpForTime()
{
	StickPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorld()->GetTimerManager().SetTimer(
		DisablePickUpTimerHandle,
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

FVector2f ABaseSkaterCharacter::GetCursorTarget() const
{
	ASkaterController* SkaterController = Cast<ASkaterController>(GetController());
	if (!SkaterController)
	{
		UE_LOG(LogTemp, Error, TEXT("ABaseSkaterCharacter: failed to get SkaterController"));
		return FVector2f(0.f);

	}

	return SkaterController->GetCursorTarget();
}

FVector ABaseSkaterCharacter::ComputeDirectionTo(FVector2f Location) const
{
	FVector ActorLocation = GetActorLocation();
	FVector2f ActorPlanarLocation = FVector2f(ActorLocation.X, ActorLocation.Y);

	FVector2f PlanarWorldDirection = Location - ActorPlanarLocation;
	FVector Direction = FVector(PlanarWorldDirection.X, PlanarWorldDirection.Y, 0);

	return Direction;
}

FVector ABaseSkaterCharacter::ComputeDirectionFromPuckTo(FVector2f Location) const
{
	if (!Puck)
		return FVector(0);

	FVector ActorLocation = Puck->GetActorLocation();
	FVector2f ActorPlanarLocation = FVector2f(ActorLocation.X, ActorLocation.Y);

	FVector2f PlanarWorldDirection = Location - ActorPlanarLocation;
	FVector Direction = FVector(PlanarWorldDirection.X, PlanarWorldDirection.Y, 0);

	return Direction;
}

void ABaseSkaterCharacter::OnMaxAccelerationChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxAcceleration = Data.NewValue;
}

void ABaseSkaterCharacter::OnMaxSkateSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;

	//UE_LOG(LogTemp, Warning, TEXT("[%s | LocalController=%d | HasAuthority=%d] OnMaxSkateSpeedChanged = %f"),
	//	*GetNameSafe(this),
	//	IsLocallyControlled() ? 1 : 0,
	//	HasAuthority() ? 1 : 0, MaxSkateSpeed);
}
