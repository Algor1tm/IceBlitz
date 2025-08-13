// Fill out your copyright notice in the Description page of Project Settings.


#include "HockeyGameMode.h"
#include "PlayerCamera.h"
#include "SkaterCharacter.h"
#include "Kismet/GameplayStatics.h"


AHockeyGameMode::AHockeyGameMode()
{

}

void AHockeyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!PlayerCameraClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AHockeyGameMode: Invalid Camera Class."));
		return;
	}

	const FVector CameraStartPos = FVector(0, 0 ,0);
	const FRotator CameraStartRotation = FRotator(0, 0, 0.f);

	TObjectPtr<APlayerCamera> CameraActor = GetWorld()->SpawnActor<APlayerCamera>(PlayerCameraClass, CameraStartPos, CameraStartRotation, FActorSpawnParameters());
	TObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	PlayerController->SetViewTarget(CameraActor);

	TObjectPtr<ABaseSkaterCharacter> PossessedSkater = Cast<ABaseSkaterCharacter>(PlayerController->GetPawn());
	if (PossessedSkater)
	{
		PossessedSkater->SetPlayerCamera(CameraActor);
	}

	CameraActor->CenterOnPossesedPawn();
}
