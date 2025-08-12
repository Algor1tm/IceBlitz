// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "SkaterCharacter.generated.h"

class APuck;
class APlayerCamera;
class UNiagaraSystem;
class UInputAction;
class UPrimitiveComponent;
class UInputComponent;
class UStaticMeshComponent;
class USphereComponent;
class USoundBase;

UCLASS()
class ICEBLITZ_API ASkaterCharacter : public ACharacter
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

	// VFX
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> FXCursor;

	// Sounds
protected:
	UPROPERTY(EditAnywhere, Category = "Sounds")
	TObjectPtr<USoundBase> SlideSound;

	// Inputs
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> StopAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CameraResetAction;

	void OnMove(const FInputActionValue& Value);

	void OnSlide();

	void OnStop();

	void OnShootBeginInput();

	void OnShootEndInput();

	void OnCameraReset();

	void ShootBegin();

	void ShootEnd();

	void TrySteal();

	// Movement
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundFriction = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BrakingFriction = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SkateSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxSkateSpeed = 325.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxAcceleration = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationSpeed = 900.f;

	void EnableOrientRotationToMovement(bool Enable);

	void FaceDirection(const FVector& Direction);

	// Abilities
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlideIntensity = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShootChargeSpeed = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootBasePower = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootChargeFactor = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootMaxDistanceToCursor = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootDistanceToCursorFactor = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootMaxSkaterSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float ShootSpeedFactor = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float FastFillFactor = 1.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MoveShootChargeDecrease = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Shoot")
	float PuckPickUpCooldown = 0.275f;

	// Private impl
protected:
	UPROPERTY(BlueprintReadOnly)
	FVector2D MoveDestination;

	UPROPERTY(BlueprintReadOnly)
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly)
	APuck* OwnedPuck = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float ShootCharge = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCharging = false;

	bool bFastFill = false;

	FTimerHandle PickUpTimerHandle;

	// 1 - character actor, 2 - number of intersections
	TMap<ASkaterCharacter*, uint8> StealableCharacters;

public:
	ASkaterCharacter();

protected:
	virtual void BeginPlay() override;

	float ComputeShotPower(const FVector& Direction, float DistanceToCursor, const FVector& SkaterVelocity) const;

	UFUNCTION()
	void OnPuckPickUp(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void DisablePuckPickUpForTime();

	void EnablePuckPickUp();

	UFUNCTION()
	void OnStealRangeBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnStealRangeEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	APuck* GetPuck() const;

	UFUNCTION(BlueprintCallable)
	void OnPuckStealed();

	UFUNCTION(BlueprintCallable)
	FVector2D GetLocationUnderCursor() const;

	UFUNCTION(BlueprintCallable)
	FVector ComputeDirectionTo(FVector2D Location) const;

	UFUNCTION(BlueprintCallable)
	FVector ComputeDirectionFromPuckTo(FVector2D Location) const;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UStaticMeshComponent* GetStickMeshComponent() const;

	void SetPlayerCamera(TObjectPtr<APlayerCamera> Camera);
};
