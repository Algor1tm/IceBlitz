// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "SkaterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ICEBLITZ_API USkaterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FGameplayAttributeData MaxAcceleration;
    ATTRIBUTE_ACCESSORS(USkaterAttributeSet, MaxAcceleration);

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FGameplayAttributeData MaxSkateSpeed;
    ATTRIBUTE_ACCESSORS(USkaterAttributeSet, MaxSkateSpeed);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SkateSpeed, Category = "Movement")
    FGameplayAttributeData SkateSpeed;
    ATTRIBUTE_ACCESSORS(USkaterAttributeSet, SkateSpeed);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ShotCharge, Category = "Shooting")
    FGameplayAttributeData ShotCharge;
    ATTRIBUTE_ACCESSORS(USkaterAttributeSet, ShotCharge);

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_SkateSpeed(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_ShotCharge(const FGameplayAttributeData& OldValue);
};
