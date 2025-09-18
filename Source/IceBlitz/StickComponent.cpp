// Fill out your copyright notice in the Description page of Project Settings.


#include "StickComponent.h"


UStickComponent::UStickComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicated(true);
    SetSimulatePhysics(false);
}

void UStickComponent::SetStickMaterial(UMaterialInterface* Material)
{
    SetMaterial(0, Material);
}

void UStickComponent::BeginPlay()
{
    Super::BeginPlay();

    DefaultLocalRotation = GetRelativeRotation();
}

void UStickComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsSwingAnimating) 
        return;

    ElapsedSwingTime += DeltaTime;
    float Alpha = ElapsedSwingTime / TotalSwingTime;

    if (Alpha >= 1.f)
    {
        StopSwingAnimation();
        return;
    }

    float AngleOffset = - FMath::Sin(Alpha * PI) * SwingAngle;
    SetRelativeRotation(GetInitialLocalRotation() + FRotator(AngleOffset, 0.f, 0.f));
}

void UStickComponent::StartSwingAnimation(float Time)
{
    TotalSwingTime = Time;
    ElapsedSwingTime = 0.f;
    bIsSwingAnimating = true;
}

void UStickComponent::StopSwingAnimation()
{
    if (!bIsSwingAnimating)
        return;

    SetRelativeRotation(GetInitialLocalRotation());
    bIsSwingAnimating = false;
}

FRotator UStickComponent::GetInitialLocalRotation() const
{
    if (Orientation == StickOrientation::Default)
        return DefaultLocalRotation;

    if(Orientation == StickOrientation::Left)
        return DefaultLocalRotation;

    if(Orientation == StickOrientation::Right)
        return DefaultLocalRotation;
       
    return DefaultLocalRotation;
}
