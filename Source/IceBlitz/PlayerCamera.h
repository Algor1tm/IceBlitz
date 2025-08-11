// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerCamera.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASkaterCharacter;

UCLASS()
class ICEBLITZ_API APlayerCamera : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere)
	float ScrollSpeed = 2000.f;

public:	
	APlayerCamera();

protected:
	virtual void BeginPlay() override;
	void HandleEdgeScrolling(float DeltaTime);

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void CenterOnPossesedPawn();
};
