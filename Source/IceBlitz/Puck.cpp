// Fill out your copyright notice in the Description page of Project Settings.


#include "Puck.h"
#include "BaseSkaterCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"


APuck::APuck()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

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

void APuck::OnPickUp(ABaseSkaterCharacter* Skater)
{
	if (!HasAuthority() || !Skater)
		return;

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
	RootComponent->AttachToComponent(Skater->GetStickMeshComponent(), AttachmentRules, FName("PuckSocket"));

	CylinderCollider->SetSimulatePhysics(false);

	SkaterOwner = Skater;
}

void APuck::OnRelease()
{
	if (!HasAuthority())
		return;

	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false);
	RootComponent->DetachFromComponent(DetachmentRules);

	CylinderCollider->SetPhysicsLinearVelocity(FVector::ZeroVector);
	CylinderCollider->SetSimulatePhysics(true);

	SkaterOwner = nullptr;
}

bool APuck::HasOwner() const
{
	return SkaterOwner != nullptr;
}

ABaseSkaterCharacter* APuck::GetSkaterOwner() const
{
	return SkaterOwner;
}

void APuck::Shoot(FVector Direction, float Power)
{
	CylinderCollider->AddImpulse(Direction * Power);
}
