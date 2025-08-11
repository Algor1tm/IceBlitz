// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SkaterController.generated.h"

class UInputMappingContext;

UCLASS()
class ICEBLITZ_API ASkaterController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<UInputMappingContext*> MappingContexts;

public:
	ASkaterController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
