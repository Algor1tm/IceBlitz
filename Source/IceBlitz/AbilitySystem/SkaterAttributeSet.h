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
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MaxAcceleration;
    ATTRIBUTE_ACCESSORS(USkaterAttributeSet, MaxAcceleration);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData SkateSpeed;
    ATTRIBUTE_ACCESSORS(USkaterAttributeSet, SkateSpeed);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MaxSkateSpeed;
    ATTRIBUTE_ACCESSORS(USkaterAttributeSet, MaxSkateSpeed);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData ShotCharge;
    ATTRIBUTE_ACCESSORS(USkaterAttributeSet, ShotCharge);
};
