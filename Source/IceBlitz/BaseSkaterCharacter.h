// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "BaseSkaterCharacter.generated.h"

class APuck;
class APlayerCamera;
class ASkaterController;
class FLifetimeProperty;
class UStickComponent;
class USphereComponent;
class UCapsuleComponent;
class UInputAction;
class USkaterAbility;
class USkaterAttributeSet;

UENUM(BlueprintType)
enum class ESkaterAbilityID : uint8
{
	None        UMETA(DisplayName = "None"),
	Move        UMETA(DisplayName = "Move"),
	Stop        UMETA(DisplayName = "Stop"),
	Shoot       UMETA(DisplayName = "Shoot"),
	Steal       UMETA(DisplayName = "Steal"),

	Boost       UMETA(DisplayName = "Boost"),
	Slide       UMETA(DisplayName = "Slide"),
	OneTimer    UMETA(DisplayName = "OneTimer"),
	Shield      UMETA(DisplayName = "Shield"),
};

UCLASS(Abstract)
class ICEBLITZ_API ABaseSkaterCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// General
protected:
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStickComponent* Stick;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	USphereComponent* StickPickUpTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	USphereComponent* MeshPickUpTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	USphereComponent* StickStealRangeTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	USphereComponent* MeshStealRangeTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	UCapsuleComponent* EnemyStealRangeTrigger;

	// Input
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* StopInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ShootInputAction;

	void OnMoveInput();

	void OnStopInput();

	void OnShootInputPressed();

	void OnShootInputReleased();

	UFUNCTION(Server, Reliable)
	void ServerOnShootInputReleased();

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
	TSubclassOf<USkaterAbility> ShootAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<USkaterAbility> StealAbility;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	float ShootBasePower = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	float ShootMinPower = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	float ShootChargeFactor = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	float ShootMaxDistanceToCursor = 1750.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	float ShootDistanceToCursorFactor = 1.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	float ShootSpeedFactor = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	float PuckPickUpCooldown = 0.275f;

	// Movement
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundFriction = 0.82f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BrakingFriction = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SkateSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxSkateSpeed = 410.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxAcceleration = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinDistance = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationSpeed = 1175.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bOrientRotationToMovement = true;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float BoostMaxSkateSpeed = 810.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float BoostDuration = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	int BoostCharges = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float BoostChargeCooldown = 3.f;

public:
	float GetBoostCharges() const { return BoostCharges; };

	float GetBoostChargeCooldown() const { return BoostChargeCooldown; };

protected:
	UPROPERTY(BlueprintReadOnly, Replicated)
	FVector2f MoveDestination;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
	APuck* Puck;

	FTimerHandle DisablePickUpTimerHandle;

	// 1 - skater actor, 2 - number of intersections > 0
	TMap<ABaseSkaterCharacter*, uint8> StealableCharacters;

public:
	ABaseSkaterCharacter();

	UFUNCTION(BlueprintCallable)
	void SetMoveDestination(FVector2f Destination);

	UFUNCTION(BlueprintCallable)
	void StopMovement();

	UFUNCTION(BlueprintCallable)
	void EnableOrientRotationToMovement(bool Enable);

	UFUNCTION(BlueprintCallable)
	void FaceDirection(const FVector& Direction);

	virtual void PickUpPuck(APuck* aPuck);

protected:
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UFUNCTION(BlueprintCallable)
	float ShootPuck();

	float ComputeShotPower(float Charge, const FVector& Direction, float DistanceToCursor, const FVector& SkaterVelocity) const;

	UFUNCTION(BlueprintCallable, Client, Reliable)
	void ClientStop(FVector DirectionToFace);

	UFUNCTION()
	void OnPuckOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void OnPuckReceive(APuck* aPuck);

	void DisablePuckPickUpForTime();

	void EnablePuckPickUp();

	UFUNCTION()
	void OnStealRangeBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnStealRangeEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	bool TryStealPuck();

	void OnPuckStolen();

	UFUNCTION(BlueprintPure)
	FVector2f GetCursorTarget() const;

	UFUNCTION(BlueprintPure)
	FVector ComputePlanarVector(FVector Left, FVector2f Right) const;

	// Attributes
protected:
	void OnMaxAccelerationChanged(const FOnAttributeChangeData& Data);

	void OnMaxSkateSpeedChanged(const FOnAttributeChangeData& Data);

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure)
	UStickComponent* GetStick() const { return Stick; }
};
