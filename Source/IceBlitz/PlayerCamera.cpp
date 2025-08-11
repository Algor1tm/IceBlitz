// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCamera.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"


APlayerCamera::APlayerCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bDoCollisionTest = true;

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->SetActive(true);
}

void APlayerCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleEdgeScrolling(DeltaTime);
}

void APlayerCamera::HandleEdgeScrolling(float DeltaTime)
{
	TObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	int32 SizeX, SizeY;
	PlayerController->GetViewportSize(SizeX, SizeY);

	float MouseCursorX, MouseCursorY;
	PlayerController->GetMousePosition(MouseCursorX, MouseCursorY);

	float RatioX = MouseCursorX / SizeX;
	float RatioY = MouseCursorY / SizeY;
	const float EdgePixelsTolerance = 4.f;

	if (SizeX - MouseCursorX <= EdgePixelsTolerance && MouseCursorX <= SizeX)
	{
		float DeltaX = DeltaTime * ScrollSpeed;
		AddActorWorldOffset(FVector(DeltaX, 0, 0));
	}
	else if (MouseCursorX <= EdgePixelsTolerance && MouseCursorX >= 0)
	{
		float DeltaX = -DeltaTime * ScrollSpeed;
		AddActorWorldOffset(FVector(DeltaX, 0, 0));
	}

	if (SizeY - MouseCursorY <= EdgePixelsTolerance && MouseCursorY <= SizeY)
	{
		float DeltaY = DeltaTime * ScrollSpeed;
		AddActorWorldOffset(FVector(0, DeltaY, 0));
	}
	else if (MouseCursorY <= EdgePixelsTolerance && MouseCursorY >= 0)
	{
		float DeltaY = -DeltaTime * ScrollSpeed;
		AddActorWorldOffset(FVector(0, DeltaY, 0));
	}
}

void APlayerCamera::CenterOnPossesedPawn()
{
	TObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector PossessedSkaterLocation = PlayerController->GetPawn()->GetActorLocation();

	FVector NewLocation = FVector(PossessedSkaterLocation.X, PossessedSkaterLocation.Y, 0.f);
	SetActorLocation(NewLocation);
}
