// Fill out your copyright notice in the Description page of Project Settings.


#include "StopAbility.h"
#include "../BaseSkaterCharacter.h"
#include "AbilitySystemComponent.h"


UStopAbility::UStopAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UStopAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s | LocalController=%d] Camera spawned"),
		*GetNameSafe(this),
		IsLocallyControlled() ? 1 : 0);

	if (CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		ABaseSkaterCharacter* Skater = Cast<ABaseSkaterCharacter>(GetAvatarActorFromActorInfo());
		if (Skater)
		{
			Skater->StopMovement();
		}

		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
