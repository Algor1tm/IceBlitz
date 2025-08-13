// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalieCharacter.h"


AGoalieCharacter::AGoalieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGoalieCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AGoalieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGoalieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
