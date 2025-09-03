// Fill out your copyright notice in the Description page of Project Settings.


#include "CursorLocationTargetActor.h"
#include "Abilities/GameplayAbility.h"

#define ECC_CursorTrace ECC_GameTraceChannel2


ACursorLocationTargetActor::ACursorLocationTargetActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACursorLocationTargetActor::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	PrimaryPC = Cast<APlayerController>(Ability->GetAvatarActorFromActorInfo()->GetInstigatorController());

	ConfirmTargetingAndContinue();
}

void ACursorLocationTargetActor::ConfirmTargetingAndContinue()
{
	if (!PrimaryPC)
	{
		CancelTargeting();
		return;
	}

	FHitResult HitResult;
	PrimaryPC->GetHitResultUnderCursor(ECollisionChannel::ECC_CursorTrace, true, HitResult);

	if (!HitResult.bBlockingHit)
	{
		CancelTargeting();
		return;
	}

	FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit();
	TargetData->HitResult = HitResult;

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Add(TargetData);
	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}
