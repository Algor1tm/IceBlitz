// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseSkaterCharacter.h"
#include "GoalieCharacter.generated.h"

UCLASS()
class ICEBLITZ_API AGoalieCharacter : public ABaseSkaterCharacter
{
	GENERATED_BODY()

	// Private impl
protected:

public:
	AGoalieCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
