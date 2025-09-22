// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseSkaterCharacter.h"
#include "SkaterCharacter.generated.h"

class USkaterAbility;
class APuck;

UCLASS()
class ICEBLITZ_API ASkaterCharacter : public ABaseSkaterCharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* BoostInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlideInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* OneTimerInputAction;

	void OnBoostInput();

	void OnSlideInput();

	void OnOneTimerInput();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<USkaterAbility> SlideAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<USkaterAbility> BoostAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<USkaterAbility> OneTimerAbility;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OneTimer")
	float PuckSpeedFactor = 0.3f;

public:
	ASkaterCharacter();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void OneTimerShootPuck(APuck* aPuck);

	float OneTimerComputeShotPower(const FVector& Direction, float DistanceToCursor, const FVector& SkaterVelocity, const FVector& PuckVelocity) const;

	UFUNCTION(Client, Reliable)
	void ClientFaceDirection(FVector ShotDirection);

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void PickUpPuck(APuck* aPuck) override;
};
