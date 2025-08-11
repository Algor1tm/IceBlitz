// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Puck.generated.h"

class ASkaterCharacter;
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class ICEBLITZ_API APuck : public AActor
{
	GENERATED_BODY()
	
public:	
	APuck();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puck")
	TObjectPtr<UStaticMeshComponent> CylinderCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puck")
	TObjectPtr<UStaticMeshComponent> PuckMesh;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void SetSkaterOwner(ASkaterCharacter* Skater);

	void ReleaseOwner();

	void Shoot(FVector Direction, float Power);
};
