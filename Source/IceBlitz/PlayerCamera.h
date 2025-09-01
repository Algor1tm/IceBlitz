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


public:	
	APlayerCamera();

	virtual void BeginPlay() override;
};
