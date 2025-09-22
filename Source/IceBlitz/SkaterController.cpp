// Fill out your copyright notice in the Description page of Project Settings.


#include "SkaterController.h"
#include "BaseSkaterCharacter.h"
#include "PlayerCamera.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"

#define ECC_CursorTrace ECC_GameTraceChannel2


ASkaterController::ASkaterController()
{
	PrimaryActorTick.bCanEverTick = true;
	CursorTarget = FVector2f(0.f);
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
		StartCursorTargetUpdates();
	}
}

void ASkaterController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// For pure clients
	if (IsLocalController() && GetPawn() != nullptr && !HasAuthority() && !CameraActor)
	{
		SpawnAndSetCamera();
		StartCursorTargetUpdates();
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

		//UE_LOG(LogTemp, Warning, TEXT("[%s | LocalController=%d | HasAuthority=%d] Camera spawned"),
		//	*GetNameSafe(this),
		//	IsLocalController() ? 1 : 0,
		//	HasAuthority() ? 1 : 0);
	}
}

void ASkaterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CameraActor || !IsLocalController())
		return;

	if (bCameraEnabled)
	{
		FIntPoint ViewportSize;
		GetViewportSize(ViewportSize.X, ViewportSize.Y);

		FVector2f MouseCursor;
		GetMousePosition(MouseCursor.X, MouseCursor.Y);

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

		if (!CameraOffset.IsZero())
			CameraActor->AddActorWorldOffset(CameraOffset);
	}

	FVector CamLocation = CameraActor->GetActorLocation();
	CamLocation.X = FMath::Clamp(CamLocation.X, CameraBoundsMin.X, CameraBoundsMax.X);
	CamLocation.Y = FMath::Clamp(CamLocation.Y, CameraBoundsMin.Y, CameraBoundsMax.Y);
	CamLocation.Z = FMath::Clamp(CamLocation.Z, CameraBoundsMin.Z, CameraBoundsMax.Z);

	if(CamLocation != CameraActor->GetActorLocation())
		CameraActor->SetActorLocation(CamLocation);
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
		EnhancedInputComponent->BindAction(CenterCameraAction, ETriggerEvent::Triggered, this, &ASkaterController::OnCenterCameraInput);
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &ASkaterController::OnToggleCameraInput);
	}
}

void ASkaterController::OnCenterCameraInput()
{
	CenterCamera();
}

void ASkaterController::OnToggleCameraInput()
{
	ToggleCamera();
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

void ASkaterController::StartCursorTargetUpdates()
{
	if (!IsLocalController())
		return;

	CachedCursorPosition = FVector2f(0.f);
	CurrentServerCursorTarget = CursorTarget = FVector2f(0.f);

	GetWorld()->GetTimerManager().SetTimer(
		ClientCursorUpdateTimerHandle,
		this,
		&ASkaterController::UpdateClientCursorTarget, 1.f / ClientCursorUpdateRate, true);

	GetWorld()->GetTimerManager().SetTimer(
		ServerCursorUpdateTimerHandle,
		this,
		&ASkaterController::UpdateServerCursorTarget, 1.f / ServerCursorUpdateRate, true);
}

void ASkaterController::StopCursorTargetUpdates()
{
	if (!IsLocalController())
		return;

	GetWorld()->GetTimerManager().ClearTimer(ServerCursorUpdateTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ClientCursorUpdateTimerHandle);
}

void ASkaterController::UpdateClientCursorTarget()
{
	FVector2f CurrentCursorPosition;
	GetMousePosition(CurrentCursorPosition.X, CurrentCursorPosition.Y);

	FIntPoint ViewportSize;
	GetViewportSize(ViewportSize.X, ViewportSize.Y);

	bool PerformHit = false;

	if (CurrentCursorPosition.X > 1 && CurrentCursorPosition.X < ViewportSize.X - 1
		&& CurrentCursorPosition.Y > 1 && CurrentCursorPosition.Y < ViewportSize.Y - 1)
	{
		if (!CachedCursorPosition.Equals(CurrentCursorPosition))
		{
			CachedCursorPosition = CurrentCursorPosition;
			PerformHit = true;
		}
	}

	if (!PerformHit)
		return;

	FHitResult HitResult;
	bool bHitSuccess = GetHitResultUnderCursor(ECollisionChannel::ECC_CursorTrace, false, HitResult);

	if (bHitSuccess)
	{
		CursorTarget = FVector2f(HitResult.Location.X, HitResult.Location.Y);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find location under cursor!"));
	}
}

void ASkaterController::UpdateServerCursorTarget()
{
	ServerSendCursorTarget(CursorTarget);
}

void ASkaterController::ServerSendCursorTarget_Implementation(FVector2f ClientCursorTarget)
{
	CursorTarget = ClientCursorTarget;
}
