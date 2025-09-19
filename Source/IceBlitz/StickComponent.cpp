// Fill out your copyright notice in the Description page of Project Settings.


#include "StickComponent.h"
#include "Components/SphereComponent.h"


UStickComponent::UStickComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0167f; // 60 Hz

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

    DefaultRelativeRotation = GetRelativeRotation();
}

void UStickComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetOwner()->HasAuthority())
    {
        HandleWallCollision();
        PlaySwingAnimation(DeltaTime);
    }
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

    SetRelativeRotation(GetRotation());
    bIsSwingAnimating = false;
}

FRotator UStickComponent::GetRotation() const
{
    if (Orientation == EStickOrientation::Default)
        return DefaultRelativeRotation;

    if(Orientation == EStickOrientation::Left)
        return DefaultRelativeRotation + FRotator(0.f, -90.f, 0.f);

    if(Orientation == EStickOrientation::Right)
        return DefaultRelativeRotation + FRotator(0.f, 90.f, 0.f);
       
    return DefaultRelativeRotation;
}

void UStickComponent::SetOrientation(EStickOrientation StickOrientation)
{
    Orientation = StickOrientation;
    SetRelativeRotation(GetRotation());
}

void UStickComponent::HandleWallCollision()
{
    FVector ActorForward = GetOwner()->GetActorForwardVector();

    FVector StartTrace = GetOwner()->GetActorLocation();
    StartTrace.Z = 1.f;

    FVector EndTrace = StartTrace + ActorForward * LineTraceDistance;

    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByObjectType(
        HitResult,
        StartTrace,
        EndTrace,
        ECC_WorldStatic
    );

    if (bDebugTrace)
    {
        FColor DebugColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugLine(
            GetWorld(),
            StartTrace,
            EndTrace,
            DebugColor,
            false, -1.f, 0, 2.f
        );
    }

    if (bHit)
    {
        FVector Cross = FVector::CrossProduct(-ActorForward, -HitResult.Normal);

        float AngleSign = Cross.Z;

        if (AngleSign > 0.0f)
        {
            SetOrientation(EStickOrientation::Right);
        }
        else if (AngleSign <= 0.0f)
        {
            SetOrientation(EStickOrientation::Left);
        }
    }
    else if (Orientation != EStickOrientation::Default)
    {
        SetOrientation(EStickOrientation::Default);
    }
}

void UStickComponent::PlaySwingAnimation(float DeltaTime)
{
    if (!bIsSwingAnimating)
        return;

    ElapsedSwingTime += DeltaTime;
    float Alpha = ElapsedSwingTime / TotalSwingTime;

    if (Alpha >= 1.f)
    {
        StopSwingAnimation();
        return;
    }

    float AngleOffset = -FMath::Sin(Alpha * PI) * SwingAngle;
    SetRelativeRotation(GetRotation() + FRotator(AngleOffset, 0.f, 0.f));
}
