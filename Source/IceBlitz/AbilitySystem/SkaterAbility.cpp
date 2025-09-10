// Fill out your copyright notice in the Description page of Project Settings.


#include "SkaterAbility.h"


void USkaterAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	OnGiveAbilityEvent();
}

void USkaterAbility::OnGiveAbilityEvent_Implementation()
{

}
