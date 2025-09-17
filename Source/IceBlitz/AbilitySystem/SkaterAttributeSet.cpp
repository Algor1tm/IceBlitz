// Fill out your copyright notice in the Description page of Project Settings.


#include "SkaterAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"


void USkaterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetShotChargeAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, 1.f);
    }
}

void USkaterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetShotChargeAttribute())
    {
        float CurrentValue = GetShotCharge();
        SetShotCharge(FMath::Clamp(CurrentValue, 0.0f, 1.0f));
    }
}

void USkaterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(USkaterAttributeSet, SkateSpeed, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(USkaterAttributeSet, ShotCharge, COND_None, REPNOTIFY_Always);
}

void USkaterAttributeSet::OnRep_SkateSpeed(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(USkaterAttributeSet, SkateSpeed, OldValue);
}

void USkaterAttributeSet::OnRep_ShotCharge(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(USkaterAttributeSet, ShotCharge, OldValue);
}
