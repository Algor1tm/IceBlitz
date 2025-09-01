// Fill out your copyright notice in the Description page of Project Settings.


#include "SkaterController.h"
#include "BaseSkaterCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "PlayerCamera.h"
#include "Engine/LocalPlayer.h"


ASkaterController::ASkaterController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASkaterController::BeginPlay()
{
	Super::BeginPlay();
	
	EnableCamera(bCameraEnabled);

	bShowMouseCursor = true;
	bAutoManageActiveCameraTarget = false;
}

void ASkaterController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	// For listen server clients
	if (IsLocalController() && HasAuthority() && !CameraActor)
	{
		SpawnAndSetCamera();
	}
}

void ASkaterController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// For pure clients
	if (IsLocalController() && GetPawn() != nullptr && !HasAuthority() && !CameraActor)
	{
		SpawnAndSetCamera();
	}
}

void ASkaterController::SpawnAndSetCamera()
{
	if (!PlayerCameraClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ASkaterController: Invalid Camera Class."));
		return;
	}

	FActorSpawnParameters CameraSpawnParams;
	CameraSpawnParams.Owner = this;
	CameraSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CameraSpawnParams.Instigator = GetPawn();

	const FVector CameraStartPos = FVector(0, 0, 0);
	const FRotator CameraStartRotation = FRotator(0, 0, 0.f);

	CameraActor = GetWorld()->SpawnActor<APlayerCamera>(PlayerCameraClass, CameraStartPos, CameraStartRotation, CameraSpawnParams);

	if (CameraActor)
	{
		SetViewTarget(CameraActor);
		CenterCamera();

		UE_LOG(LogTemp, Warning, TEXT("[%s | LocalController=%d | HasAuthority=%d] Camera spawned"),
			*GetNameSafe(this),
			IsLocalController() ? 1 : 0,
			HasAuthority() ? 1 : 0);
	}
}

void ASkaterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CameraActor || !bCameraEnabled || !IsLocalController())
		return;

	FIntPoint ViewportSize;
	GetViewportSize(ViewportSize.X, ViewportSize.Y);

	FVector2f MouseCursor;
	GetMousePosition(MouseCursor.X, MouseCursor.Y);

	float RatioX = MouseCursor.X / ViewportSize.X;
	float RatioY = MouseCursor.Y / ViewportSize.Y;
	const float EdgePixelsTolerance = 4.f;

	float Delta = DeltaTime * ScrollSpeed;
	FVector CameraOffset = FVector(0);

	if (ViewportSize.X - MouseCursor.X <= EdgePixelsTolerance && MouseCursor.X <= ViewportSize.X)
	{
		CameraOffset.X += Delta;
	}
	else if (MouseCursor.X <= EdgePixelsTolerance && MouseCursor.X >= 0)
	{
		CameraOffset.X -= Delta;
	}

	if (ViewportSize.Y - MouseCursor.Y <= EdgePixelsTolerance && MouseCursor.Y <= ViewportSize.Y)
	{
		CameraOffset.Y += Delta;
	}
	else if (MouseCursor.Y <= EdgePixelsTolerance && MouseCursor.Y >= 0)
	{
		CameraOffset.Y -= Delta;
	}
	
	if(!CameraOffset.IsZero())
		CameraActor->AddActorWorldOffset(CameraOffset);
}

void ASkaterController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : MappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(CenterCameraAction, ETriggerEvent::Triggered, this, &ASkaterController::CenterCamera);
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &ASkaterController::ToggleCamera);
	}
}

void ASkaterController::CenterCamera()
{
	if (!CameraActor)
		return;

	FVector PossessedSkaterLocation = GetPawn()->GetActorLocation();
	FVector NewLocation = FVector(PossessedSkaterLocation.X, PossessedSkaterLocation.Y, CameraActor->GetActorLocation().Z);

	CameraActor->SetActorLocation(NewLocation);
}

void ASkaterController::ToggleCamera()
{
	bCameraEnabled = !bCameraEnabled;
	EnableCamera(bCameraEnabled);
}

void ASkaterController::EnableCamera(bool Enable)
{
	if (!IsLocalController())
		return;

	EMouseLockMode MouseMode = Enable ? EMouseLockMode::LockAlways : EMouseLockMode::DoNotLock;

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(MouseMode);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}
