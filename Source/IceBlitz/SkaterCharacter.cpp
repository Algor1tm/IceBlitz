// Fill out your copyright notice in the Description page of Project Settings.


#include "SkaterCharacter.h"
#include "AbilitySystem/SkaterAbility.h"
#include "AbilitySystem/SkaterAttributeSet.h"

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
		EnhancedInputComponent->BindAction(SlideInputAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnSlideInput);
	}
}

void ASkaterCharacter::OnSlideInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Slide);
}
