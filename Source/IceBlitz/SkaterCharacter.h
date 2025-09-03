// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseSkaterCharacter.h"
#include "SkaterCharacter.generated.h"

UCLASS()
class ICEBLITZ_API ASkaterCharacter : public ABaseSkaterCharacter
{
	GENERATED_BODY()

protected:


public:
	ASkaterCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
