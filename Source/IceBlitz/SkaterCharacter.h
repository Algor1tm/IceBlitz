// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseSkaterCharacter.h"
#include "SkaterCharacter.generated.h"

UCLASS()
class ICEBLITZ_API ASkaterCharacter : public ABaseSkaterCharacter
{
	GENERATED_BODY()

	// Sounds
protected:
	UPROPERTY(EditAnywhere, Category = "Sounds")
	TObjectPtr<USoundBase> SlideSound;

	// Inputs
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BoostAction;

	void OnSlideInput();

	void OnBoostInput();

	void OnSlideBegin();

	void OnSlideEnd();

	// Abilities
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlideDecreaseIntensity = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	float SlideDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	float SlideSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	float SlideAccelerationMultiplier = 1.5f;

	// Private impl
protected:
	FTimerHandle SlideTimerHandle;
	float NormalSkateSpeed = 1.f;

public:
	ASkaterCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
