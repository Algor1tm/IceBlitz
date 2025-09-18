// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Puck.generated.h"

class ABaseSkaterCharacter;
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
	ABaseSkaterCharacter* SkaterOwner = nullptr;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void SetSkaterOwner(ABaseSkaterCharacter* Skater);

	void OnRelease();

	UFUNCTION(BlueprintPure)
	bool HasOwner() const;

	UFUNCTION(BlueprintPure)
	ABaseSkaterCharacter* GetSkaterOwner() const;

	void Shoot(FVector Direction, float Power);
};
