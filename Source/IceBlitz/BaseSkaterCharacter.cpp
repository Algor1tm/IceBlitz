// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseSkaterCharacter.h"
#include "Puck.h"
#include "PlayerCamera.h"
#include "AbilitySystem/SkaterAttributeSet.h"
#include "AbilitySystem/StopAbility.h"

#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

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

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (HasAuthority())
	{
		FGameplayAbilitySpec MoveAbilitySpec(MoveAbility, 1, (uint32)ESkaterAbilityInputID::Move, this);
		AbilitySystemComponent->GiveAbility(MoveAbilitySpec);

		FGameplayAbilitySpec StopAbilitySpec(UStopAbility::StaticClass(), 1, (uint32)ESkaterAbilityInputID::Stop, this);
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
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
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
