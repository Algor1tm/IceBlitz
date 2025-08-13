// Fill out your copyright notice in the Description page of Project Settings.


#include "Puck.h"
#include "BaseSkaterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


APuck::APuck()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CylinderCollider = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CylinderColliderMesh"));
	RootComponent = CylinderCollider;
	CylinderCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CylinderCollider->SetVisibility(false);

	CylinderCollider->GetBodyInstance()->bLockXRotation = true;
	CylinderCollider->GetBodyInstance()->bLockYRotation = true;
	CylinderCollider->GetBodyInstance()->bLockZRotation = true;
	CylinderCollider->SetSimulatePhysics(true);

	PuckMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuckMesh"));
	PuckMesh->SetupAttachment(CylinderCollider);
	PuckMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APuck::BeginPlay()
{
	Super::BeginPlay();
}

void APuck::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APuck::SetSkaterOwner(ABaseSkaterCharacter* Skater)
{
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
	RootComponent->AttachToComponent(Skater->GetStickMeshComponent(), AttachmentRules, FName("PuckSocket"));

	CylinderCollider->SetSimulatePhysics(false);
	CylinderCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bHasOwner = true;
}

void APuck::ReleaseOwner()
{
	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false);
	RootComponent->DetachFromComponent(DetachmentRules);

	CylinderCollider->SetPhysicsLinearVelocity(FVector::ZeroVector);
	CylinderCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CylinderCollider->SetSimulatePhysics(true);

	bHasOwner = false;
}

bool APuck::HasOwner() const
{
	return bHasOwner;
}

void APuck::Shoot(FVector Direction, float Power)
{
	CylinderCollider->AddImpulse(Direction * Power);
}
