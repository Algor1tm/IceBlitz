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

void USkaterAbility::CancelAbilitiesWithTags(const FGameplayTagContainer& Tags)
{
	GetAbilitySystemComponentFromActorInfo()->CancelAbilities(&Tags);
}
