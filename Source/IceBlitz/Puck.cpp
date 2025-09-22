// Fill out your copyright notice in the Description page of Project Settings.


#include "Puck.h"
#include "BaseSkaterCharacter.h"
#include "StickComponent.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"


APuck::APuck()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(30.f);

	CylinderCollider = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CylinderColliderMesh"));
	RootComponent = CylinderCollider;
	CylinderCollider->SetVisibility(false);
	CylinderCollider->SetIsReplicated(true);
	CylinderCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	PuckMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuckMesh"));
	PuckMesh->SetupAttachment(CylinderCollider);
	PuckMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PuckMesh->SetSimulatePhysics(false);
}

void APuck::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CylinderCollider->SetSimulatePhysics(true);
	}
}

void APuck::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APuck::SetSkaterOwner(ABaseSkaterCharacter* Skater)
{
	if (!HasAuthority() || !Skater)
		return;

	UStickComponent* Stick = Skater->GetStick();
	Stick->StopSwingAnimation();

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
	RootComponent->AttachToComponent(Stick, AttachmentRules, Stick->GetPuckSocketName());

	CylinderCollider->SetSimulatePhysics(false);

	SkaterOwner = Skater;
}

void APuck::OnRelease()
{
	if (!HasAuthority() || !SkaterOwner)
		return;

	// Check if puck is inside another skater when released and let him instantly pick up 
	// otherwise simulate physics

	TArray<AActor*> OverlappingSkaters;
	GetOverlappingActors(OverlappingSkaters, ABaseSkaterCharacter::StaticClass());

	ABaseSkaterCharacter* ClosestSkater = nullptr;
	float ClosestDistSq = TNumericLimits<float>::Max();

	const FVector PuckLocation = GetActorLocation();

	for (AActor* Actor : OverlappingSkaters)
	{
		ABaseSkaterCharacter* Skater = Cast<ABaseSkaterCharacter>(Actor);
		if (Skater && Skater != SkaterOwner)
		{
			const float DistSq = FVector::DistSquared(PuckLocation, Skater->GetActorLocation());
			if (DistSq < ClosestDistSq)
			{
				ClosestDistSq = DistSq;
				ClosestSkater = Skater;
			}
		}
	}

	if (ClosestSkater)
	{
		ClosestSkater->PickUpPuck(this);
	}
	else
	{
		FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false);
		RootComponent->DetachFromComponent(DetachmentRules);

		CylinderCollider->SetSimulatePhysics(true);

		SkaterOwner = nullptr;
	}
}

bool APuck::HasOwner() const
{
	return SkaterOwner != nullptr;
}

ABaseSkaterCharacter* APuck::GetSkaterOwner() const
{
	return SkaterOwner;
}

void APuck::Shoot(FVector Direction, float Power, bool RemoveCurrentVelocity)
{
	if(RemoveCurrentVelocity)
		CylinderCollider->SetPhysicsLinearVelocity(FVector::ZeroVector);

	CylinderCollider->AddImpulse(Direction * Power);
}
