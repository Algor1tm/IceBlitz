// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilityMove.generated.h"

class UNiagaraSystem;

UCLASS()
class ICEBLITZ_API UAbilityMove : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* MoveDestinationVFX = nullptr;

public:
	UAbilityMove();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	UFUNCTION()
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data);
};
