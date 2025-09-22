// Fill out your copyright notice in the Description page of Project Settings.


#include "SkaterCharacter.h"
#include "Puck.h"
#include "AbilitySystem/SkaterAbility.h"
#include "AbilitySystem/SkaterAttributeSet.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"

#define ECC_CursorTrace ECC_GameTraceChannel2


ASkaterCharacter::ASkaterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASkaterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FGameplayAbilitySpec BoostAbilitySpec(BoostAbility, 1, (uint32)ESkaterAbilityInputID::Boost, this);
		AbilitySystemComponent->GiveAbility(BoostAbilitySpec);

		FGameplayAbilitySpec SlideAbilitySpec(SlideAbility, 1, (uint32)ESkaterAbilityInputID::Slide, this);
		AbilitySystemComponent->GiveAbility(SlideAbilitySpec);

		FGameplayAbilitySpec OneTimerAbilitySpec(OneTimerAbility, 1, (uint32)ESkaterAbilityInputID::OneTimer, this);
		AbilitySystemComponent->GiveAbility(OneTimerAbilitySpec);
	}
}

void ASkaterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASkaterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(BoostInputAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnBoostInput);
		EnhancedInputComponent->BindAction(SlideInputAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnSlideInput);
		EnhancedInputComponent->BindAction(OneTimerInputAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnOneTimerInput);
	}
}

void ASkaterCharacter::OnBoostInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Boost);
}

void ASkaterCharacter::OnSlideInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Slide);
}

void ASkaterCharacter::OnOneTimerInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::OneTimer);
}

void ASkaterCharacter::PickUpPuck(APuck* aPuck)
{
	FGameplayTag OneTimerActiveTag = FGameplayTag::RequestGameplayTag(FName("SkaterState.Ability.OneTimer"));
	if (AbilitySystemComponent->HasMatchingGameplayTag(OneTimerActiveTag))
	{
		FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName("Event.OneTimerShoot"));

		FGameplayEventData Payload;
		Payload.Instigator = this;
		Payload.Target = this;
		Payload.OptionalObject = aPuck;

		AbilitySystemComponent->HandleGameplayEvent(EventTag, &Payload);
	}
	else
	{
		Super::PickUpPuck(aPuck);
	}
}

void ASkaterCharacter::OneTimerShootPuck(APuck* aPuck)
{
	if (!aPuck || !HasAuthority())
		return;

	FVector2f Cursor = GetCursorTarget();
	FVector Direction = ComputePlanarVector(aPuck->GetActorLocation(), Cursor);

	float DistanceToCursor = Direction.Length();
	Direction = Direction.GetSafeNormal();

	float Power = OneTimerComputeShotPower(Direction, DistanceToCursor, GetCharacterMovement()->Velocity, aPuck->GetVelocity());

	DisablePuckPickUpForTime();

	if (!aPuck->HasOwner())
		aPuck->Shoot(Direction, Power);

	ClientStop(Direction);
}

float ASkaterCharacter::OneTimerComputeShotPower(const FVector& Direction, float DistanceToCursor, const FVector& SkaterVelocity, const FVector& PuckVelocity) const
{
	float VelocityProjection = FVector::DotProduct(SkaterVelocity.GetSafeNormal(), Direction);
	float Speed = SkaterVelocity.Length() * VelocityProjection;

	float NormDistanceToCursor = FMath::Clamp(DistanceToCursor / ShootMaxDistanceToCursor, 0.f, 1.f);
	float NormalizedSpeed = Speed / MaxSkateSpeed;
	float PuckSpeed = PuckVelocity.Length();

	float TotalModifiers = ShootDistanceToCursorFactor * NormDistanceToCursor + ShootSpeedFactor * NormalizedSpeed;
	TotalModifiers = FMath::Clamp(TotalModifiers, 0.f, 1.f);

	float Power = PuckSpeedFactor * PuckSpeed * TotalModifiers;
	Power = FMath::Clamp(Power, ShootMinPower, Power);

	return Power;
}

void ASkaterCharacter::ClientFaceDirection_Implementation(FVector ShotDirection)
{
	FaceDirection(ShotDirection);
}
