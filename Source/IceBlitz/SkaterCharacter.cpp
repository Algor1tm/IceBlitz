// Fill out your copyright notice in the Description page of Project Settings.


#include "SkaterCharacter.h"
#include "Puck.h"
#include "PlayerCamera.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "EnhancedInputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#define ECC_CursorTrace ECC_GameTraceChannel2


ASkaterCharacter::ASkaterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASkaterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASkaterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASkaterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnSlideInput);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Started, this, &ASkaterCharacter::OnBoostInput);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}

void ASkaterCharacter::OnSlideInput()
{
	OnSlideBegin();
}

void ASkaterCharacter::OnBoostInput()
{
	OnBoostBegin();
}

void ASkaterCharacter::OnSlideBegin()
{
	GetCharacterMovement()->Velocity *= (1.f - SlideDecreaseIntensity);
	GetCharacterMovement()->MaxAcceleration *= SlideAccelerationMultiplier;

	NormalSkateSpeed = SkateSpeed;
	SkateSpeed *= SlideSpeedMultiplier;

	GetWorld()->GetTimerManager().SetTimer(
		SlideTimerHandle,
		this,
		&ASkaterCharacter::OnSlideEnd,
		SlideDuration,
		false
	);

	UGameplayStatics::PlaySound2D(this, SlideSound);
}

void ASkaterCharacter::OnSlideEnd()
{
	GetCharacterMovement()->MaxAcceleration = MaxAcceleration;
	SkateSpeed = NormalSkateSpeed;
}
