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
class USkaterAbility;
class USkaterAttributeSet;

UENUM(BlueprintType)
enum class ESkaterAbilityInputID : uint8
{
	None        UMETA(DisplayName = "None"),
	Move        UMETA(DisplayName = "Move"),
	Stop        UMETA(DisplayName = "Stop"),
	Slide       UMETA(DisplayName = "Slide"),
	Boost       UMETA(DisplayName = "Boost"),
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* BoostInputAction;

	void OnMoveInput();

	void OnStopInput();

	void OnBoostInput();

	// Abilities
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	USkaterAttributeSet* AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<USkaterAbility> MoveAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<USkaterAbility> StopAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<USkaterAbility> BoostAbility;

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

protected:
	UPROPERTY(BlueprintReadOnly, Replicated)
	FVector2D MoveDestination;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly)
	float ShotCharge = 0.f;

public:
	ABaseSkaterCharacter();

	UFUNCTION(BlueprintCallable)
	void SetMoveDestination(FVector2D Destination);

	UFUNCTION(BlueprintCallable)
	void StopMovement();

	UFUNCTION(BlueprintCallable)
	void EnableOrientRotationToMovement(bool Enable);

	UFUNCTION(BlueprintCallable)
	void FaceDirection(const FVector& Direction);

protected:
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UFUNCTION(BlueprintPure)
	FVector ComputeDirectionTo(FVector2D Location) const;

	void OnMaxAccelerationChanged(const FOnAttributeChangeData& Data);
	void OnSkateSpeedChanged(const FOnAttributeChangeData& Data);
	void OnMaxSkateSpeedChanged(const FOnAttributeChangeData& Data);
	void OnShotChargeChanged(const FOnAttributeChangeData& Data);

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
