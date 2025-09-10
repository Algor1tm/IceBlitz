// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseSkaterCharacter.h"
#include "Puck.h"
#include "PlayerCamera.h"
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
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		AttributeSet->InitMaxAcceleration(MaxAcceleration);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			USkaterAttributeSet::GetMaxAccelerationAttribute()).AddUObject(this, &ABaseSkaterCharacter::OnMaxAccelerationChanged);

		AttributeSet->InitSkateSpeed(SkateSpeed);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			USkaterAttributeSet::GetSkateSpeedAttribute()).AddUObject(this, &ABaseSkaterCharacter::OnSkateSpeedChanged);

		AttributeSet->InitMaxSkateSpeed(MaxSkateSpeed);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			USkaterAttributeSet::GetMaxSkateSpeedAttribute()).AddUObject(this, &ABaseSkaterCharacter::OnMaxSkateSpeedChanged);

		AttributeSet->InitShotCharge(ShotCharge);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			USkaterAttributeSet::GetShotChargeAttribute()).AddUObject(this, &ABaseSkaterCharacter::OnShotChargeChanged);


		FGameplayAbilitySpec MoveAbilitySpec(MoveAbility, 1, (uint32)ESkaterAbilityInputID::Move, this);
		AbilitySystemComponent->GiveAbility(MoveAbilitySpec);

		FGameplayAbilitySpec StopAbilitySpec(StopAbility, 1, (uint32)ESkaterAbilityInputID::Stop, this);
		AbilitySystemComponent->GiveAbility(StopAbilitySpec);
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

			FVector2D PlanarLocation = FVector2D(GetActorLocation());
			if (PlanarLocation.Equals(MoveDestination, STOP_TOLERANCE))
			{
				bIsMoving = false;
				GetCharacterMovement()->StopMovementImmediately();
			}
		}
		else
		{
			AddMovementInput(Direction.GetSafeNormal(), SkateSpeed);
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
	}
}

void ABaseSkaterCharacter::OnMoveInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Move);
}

void ABaseSkaterCharacter::OnStopInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Stop);
}

void ABaseSkaterCharacter::OnBoostInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Boost);
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
}

void ABaseSkaterCharacter::SetMoveDestination(FVector2D Destination)
{
	MoveDestination = Destination;
	bIsMoving = true;
}

void ABaseSkaterCharacter::StopMovement()
{
	bIsMoving = false;
	MoveDestination = FVector2D(0.f);
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

FVector ABaseSkaterCharacter::ComputeDirectionTo(FVector2D Location) const
{
	FVector ActorLocation = GetActorLocation();
	FVector2D ActorPlanarLocation = FVector2D(ActorLocation.X, ActorLocation.Y);

	FVector2D PlanarWorldDirection = Location - ActorPlanarLocation;
	FVector Direction = FVector(PlanarWorldDirection, 0);

	return Direction;
}

void ABaseSkaterCharacter::OnMaxAccelerationChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxAcceleration = Data.NewValue;
}

void ABaseSkaterCharacter::OnSkateSpeedChanged(const FOnAttributeChangeData& Data)
{
	SkateSpeed = Data.NewValue;
}

void ABaseSkaterCharacter::OnMaxSkateSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;

	//UE_LOG(LogTemp, Warning, TEXT("[%s | LocalController=%d | HasAuthority=%d] OnMaxSkateSpeedChanged = %f"),
	//	*GetNameSafe(this),
	//	IsLocallyControlled() ? 1 : 0,
	//	HasAuthority() ? 1 : 0, MaxSkateSpeed);
}

void ABaseSkaterCharacter::OnShotChargeChanged(const FOnAttributeChangeData& Data)
{
	ShotCharge = Data.NewValue;
}
