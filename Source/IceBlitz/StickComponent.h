// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "StickComponent.generated.h"


class USphereComponent;

enum class EStickOrientation
{
	Default,
	Left,
	Right
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ICEBLITZ_API UStickComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere)
	FName PuckSocketName = "PuckSocket";

	UPROPERTY(EditAnywhere)
	float SwingAngle = 35.f;

	UPROPERTY(EditAnywhere)
	float LineTraceDistance = 120.f;

	UPROPERTY(EditAnywhere)
	bool bDebugTrace = false;

private:
	FRotator DefaultRelativeRotation;

	float TotalSwingTime = 0.f;

	float ElapsedSwingTime = 0.f;

	bool bIsSwingAnimating = true;

	EStickOrientation Orientation = EStickOrientation::Default;

public:
	UStickComponent();

	UFUNCTION(BlueprintPure)
	FName GetPuckSocketName() const { return PuckSocketName; }

	UFUNCTION(BlueprintCallable)
	void SetStickMaterial(UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable)
	void StartSwingAnimation(float Time);

	UFUNCTION(BlueprintCallable)
	void StopSwingAnimation();

	void SetOrientation(EStickOrientation StickOrientation);

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void HandleWallCollision();

	void PlaySwingAnimation(float DeltaTime);

	FRotator GetRotation() const;
};
