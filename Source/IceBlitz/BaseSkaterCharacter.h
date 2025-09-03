// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "BaseSkaterCharacter.generated.h"

class APuck;
class APlayerCamera;
class FLifetimeProperty;
class USphereComponent;
class UInputAction;
class USkaterAttributeSet;

UENUM(BlueprintType)
enum class ESkaterAbilityInputID : uint8
{
	None        UMETA(DisplayName = "None"),
	Move        UMETA(DisplayName = "Move"),
	Stop        UMETA(DisplayName = "Stop"),
};

UCLASS(Abstract)
class ICEBLITZ_API ABaseSkaterCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// General
protected:
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* StickMesh;

	UPROPERTY(BlueprintReadWrite)
	APlayerCamera* PlayerCamera;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	USphereComponent* StickPickUpTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	USphereComponent* MeshPickUpTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	USphereComponent* StickStealTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	USphereComponent* MeshStealTrigger;

	// Input
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* StopInputAction;

	void OnMoveInput();

	void OnStopInput();

	// Abilities
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	USkaterAttributeSet* AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> MoveAbility;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Movement
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundFriction = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BrakingFriction = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SkateSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxSkateSpeed = 410.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxAcceleration = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinDistance = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationSpeed = 1100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bOrientRotationToMovement = true;

private:
	UPROPERTY(Replicated)
	FVector2D MoveDestination;

	UPROPERTY(Replicated)
	bool bIsMoving = false;

public:
	ABaseSkaterCharacter();

	void SetMoveDestination(FVector2D Destination);

	void StopMovement();

	void EnableOrientRotationToMovement(bool Enable);

	void FaceDirection(const FVector& Direction);

protected:
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UFUNCTION(BlueprintPure)
	FVector ComputeDirectionTo(FVector2D Location) const;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
