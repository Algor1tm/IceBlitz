// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "BaseSkaterCharacter.generated.h"

class APuck;
class APlayerCamera;
class UInputAction;
class UNiagaraSystem;
class USoundBase;
class USphereComponent;

UCLASS()
class ICEBLITZ_API ABaseSkaterCharacter : public ACharacter
{
	GENERATED_BODY()

	// General
protected:
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> StickMesh;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerCamera> PlayerCamera;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	TObjectPtr<USphereComponent> StickPickUpTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	TObjectPtr<USphereComponent> MeshPickUpTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	TObjectPtr<USphereComponent> StickStealTrigger;

	UPROPERTY(EditAnywhere, Category = "Triggers")
	TObjectPtr<USphereComponent> MeshStealTrigger;

	// Sounds
protected:
	UPROPERTY(EditAnywhere, Category = "Sounds")
	TObjectPtr<USoundBase> ShotSound;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	TObjectPtr<USoundBase> PowerShotSound;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	TObjectPtr<USoundBase> StealSound;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	TObjectPtr<USoundBase> BoostSound;

	// VFX
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> FXCursor;

	// Inputs
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> StopAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CameraResetAction;

	void OnMoveInput();

	void OnStopInput();

	void OnShootBeginInput();

	void OnShootEndInput();

	void OnCameraResetInput();

	void SetMoveDestination(FVector2D Destination);

	void Stop();

	void CenterCamera();

	void ShootBegin();

	void ShootEnd();

	void TrySteal();

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

	void EnableOrientRotationToMovement(bool Enable);

	void FaceDirection(const FVector& Direction);

	// Abilities
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShootChargeSpeed = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootBasePower = 315.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootChargeFactor = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootMaxDistanceToCursor = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootDistanceToCursorFactor = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootMaxSkaterSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootSpeedFactor = 1.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float FastFillFactor = 1.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MoveShootChargeDecrease = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float PuckPickUpCooldown = 0.275f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Boost")
	float BoostMaxSkateSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Boost")
	float BoostDuration = 1.f;

	// Private impl
protected:
	UPROPERTY(BlueprintReadOnly)
	FVector2D MoveDestination = FVector2D(0);

	UPROPERTY(BlueprintReadOnly)
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly)
	APuck* Puck = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float ShootCharge = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCharging = false;

	bool bFastFill = false;

	bool bIsBoosting = false;

	float CurrentBoostTime = 0.f;

	FTimerHandle PickUpTimerHandle;

	// 1 - character actor, 2 - number of intersections
	TMap<ABaseSkaterCharacter*, uint8> StealableCharacters;

public:
	ABaseSkaterCharacter();

protected:
	virtual void BeginPlay() override;

	void OnBoostBegin();

	UFUNCTION()
	void OnPuckPickUp(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	float ComputeShotPower(const FVector& Direction, float DistanceToCursor, const FVector& SkaterVelocity) const;

	void PlayShotSound();

	void DisablePuckPickUpForTime();

	void EnablePuckPickUp();

	UFUNCTION()
	void OnStealRangeBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnStealRangeEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void OnPuckStealed();

	UFUNCTION(BlueprintCallable)
	FVector2D GetLocationUnderCursor() const;

	UFUNCTION(BlueprintCallable)
	FVector ComputeDirectionTo(FVector2D Location) const;

	UFUNCTION(BlueprintCallable)
	FVector ComputeDirectionFromPuckTo(FVector2D Location) const;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	bool HasPuck() const;

	UFUNCTION(BlueprintCallable)
	APuck* GetPuck() const;

	UStaticMeshComponent* GetStickMeshComponent() const;

	void SetPlayerCamera(APlayerCamera* Camera);
};
