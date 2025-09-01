// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HockeyGameMode.generated.h"


UCLASS()
class ICEBLITZ_API AHockeyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHockeyGameMode();

protected:
	virtual void BeginPlay() override;
};
