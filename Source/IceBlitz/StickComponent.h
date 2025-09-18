// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "StickComponent.generated.h"


enum class StickOrientation
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

private:
	FRotator DefaultLocalRotation;

	float TotalSwingTime = 0.f;

	float ElapsedSwingTime = 0.f;

	bool bIsSwingAnimating = true;

	StickOrientation Orientation = StickOrientation::Default;

	FRotator GetInitialLocalRotation() const;

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

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
