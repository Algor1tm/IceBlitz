// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	TObjectPtr<UInputAction> CenterCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	TObjectPtr<UInputAction> ToggleCameraAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TSubclassOf<APlayerCamera> PlayerCameraClass;

	UPROPERTY(EditAnywhere, Category = "Gameplay")
	float ScrollSpeed = 2000.f;

private:
	APlayerCamera* CameraActor = nullptr;

	bool bCameraEnabled = false;

public:
	ASkaterController();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* aPawn) override;

	virtual void OnRep_Pawn() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupInputComponent() override;

	void SpawnAndSetCamera();

	void CenterCamera();

	void ToggleCamera();

	void EnableCamera(bool Enable);
};
