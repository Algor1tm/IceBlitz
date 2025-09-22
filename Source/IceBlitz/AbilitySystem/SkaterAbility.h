// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "SkaterAbility.generated.h"

class ABaseSkaterCharacter;

UCLASS()
class ICEBLITZ_API USkaterAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	UFUNCTION(BlueprintNativeEvent)
	void OnGiveAbility();

	UFUNCTION(BlueprintNativeEvent)
	void OnInputReleased();

	UFUNCTION(BlueprintPure)
	bool AvatarHasGameplayTag(UPARAM(DisplayName = "Tag To Check") FGameplayTag TagToCheck) const;

	UFUNCTION(BlueprintPure)
	float GetWorldTimeSeconds() const;

	UFUNCTION(BlueprintCallable)
	void CancelAbilitiesWithTags(const FGameplayTagContainer& Tags);

protected:
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	ABaseSkaterCharacter* Skater;
};
