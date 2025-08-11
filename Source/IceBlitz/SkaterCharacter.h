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

	void OnShootBegin();

	void OnShootEnd();

	void OnCameraReset();

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

	void FaceDirection(FVector Direction);

	// Abilities
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlideIntensity = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShootChargeSpeed = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	float ShootBasePower = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	float ShootMaxDistanceToCursor = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	float ShootMaxSkaterSpeed = 1000.f;

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

public:
	ASkaterCharacter();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPuckPickUp(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void DisablePuckPickUpForTime();

	void EnablePuckPickUp();

	UFUNCTION(BlueprintCallable)
	FVector2D GetLocationUnderCursor() const;

	UFUNCTION(BlueprintCallable)
	FVector GetDirectionByCursor() const;

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
