// Fill out your copyright notice in the Description page of Project Settings.


#include "SkaterAbility.h"
#include "AbilitySystemComponent.h"
#include "../BaseSkaterCharacter.h"


void USkaterAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	Skater = Cast<ABaseSkaterCharacter>(ActorInfo->AvatarActor);

	OnGiveAbility();
}

void USkaterAbility::OnGiveAbility_Implementation()
{
	
}

void USkaterAbility::OnInputReleased_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s | LocalController=%d] On Input Released"),
		*GetNameSafe(this),
		IsLocallyControlled() ? 1 : 0);
}

void USkaterAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	OnInputReleased();
}

bool USkaterAbility::AvatarHasGameplayTag(FGameplayTag TagToCheck) const
{
	return GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(TagToCheck);
}

float USkaterAbility::GetWorldTimeSeconds() const
{
	return GetWorld()->GetTimeSeconds();
}
