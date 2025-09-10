// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalieCharacter.h"
#include "AbilitySystem/SkaterAbility.h"
#include "AbilitySystem/SkaterAttributeSet.h"


AGoalieCharacter::AGoalieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGoalieCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FGameplayAbilitySpec BoostAbilitySpec(BoostAbility, 2, (uint32)ESkaterAbilityInputID::Boost, this);
		AbilitySystemComponent->GiveAbility(BoostAbilitySpec);
	}
}

void AGoalieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGoalieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
