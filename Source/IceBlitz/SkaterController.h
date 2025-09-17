// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "SkaterController.generated.h"

class APlayerCamera;
class UInputAction;
class UInputMappingContext;

UCLASS()
class ICEBLITZ_API ASkaterController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<UInputMappingContext*> MappingContexts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CenterCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ToggleCameraAction;

	void OnCenterCameraInput();

	void OnToggleCameraInput();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<APlayerCamera> PlayerCameraClass;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float ScrollSpeed = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float EdgePixelsTolerance = 4.f;

	UPROPERTY(EditAnywhere, Category = "Network")
	float ClientCursorUpdateRate = 60.f;

	UPROPERTY(EditAnywhere, Category = "Network")
	float ServerCursorUpdateRate = 10.f;

private:
	APlayerCamera* CameraActor = nullptr;

	bool bCameraEnabled = false;

	FTimerHandle ClientCursorUpdateTimerHandle;
	FTimerHandle ServerCursorUpdateTimerHandle;

	FVector2f CachedCursorPosition;
	FVector2f CursorTarget;
	FVector2f CurrentServerCursorTarget;

public:
	ASkaterController();

	UFUNCTION(BlueprintPure)
	FVector2f GetCursorTarget() const { return CursorTarget; }

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* aPawn) override;

	virtual void OnRep_Pawn() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupInputComponent() override;

protected:
	void SpawnAndSetCamera();

	void CenterCamera();

	void ToggleCamera();

	void EnableCamera(bool Enable);

	void StartCursorTargetUpdates();

	void StopCursorTargetUpdates();

	void UpdateClientCursorTarget();

	void UpdateServerCursorTarget();

	// maybe Unreliable?
	// should probably use quantized vector
	UFUNCTION(Server, Reliable)
	void ServerSendCursorTarget(FVector2f ClientCursorTarget);
};
