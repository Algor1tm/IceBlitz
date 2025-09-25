// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalieCharacter.h"
#include "AbilitySystem/SkaterAbility.h"
#include "AbilitySystem/SkaterAttributeSet.h"

#include "EnhancedInputComponent.h"


AGoalieCharacter::AGoalieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGoalieCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FGameplayAbilitySpec BoostAbilitySpec(BoostAbility, 2, (uint32)ESkaterAbilityID::Boost, this);
		AbilitySystemComponent->GiveAbility(BoostAbilitySpec);

		FGameplayAbilitySpec ShieldAbilitySpec(ShieldAbility, 1, (uint32)ESkaterAbilityID::Shield, this);
		AbilitySystemComponent->GiveAbility(ShieldAbilitySpec);
	}
}

void AGoalieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGoalieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(BoostInputAction, ETriggerEvent::Started, this, &AGoalieCharacter::OnBoostInput);
		EnhancedInputComponent->BindAction(ShieldInputAction, ETriggerEvent::Started, this, &AGoalieCharacter::OnShieldInput);
	}
}

void AGoalieCharacter::OnBoostInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityID::Boost);
}

void AGoalieCharacter::OnShieldInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityID::Shield);
}