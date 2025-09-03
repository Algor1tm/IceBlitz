// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityMove.h"
#include "CursorLocationTargetActor.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "../BaseSkaterCharacter.h"


UAbilityMove::UAbilityMove()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAbilityMove::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		FActorSpawnParameters Params;
		Params.bDeferConstruction = true;

		AGameplayAbilityTargetActor* SpawnedActor = GetWorld()->SpawnActor<AGameplayAbilityTargetActor>(ACursorLocationTargetActor::StaticClass(), FTransform::Identity, Params);

		UAbilityTask_WaitTargetData* WaitTargetTask = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(
			this, 
			TEXT("WaitTargetData"), 
			EGameplayTargetingConfirmation::Instant, 
			SpawnedActor);

		if (WaitTargetTask)
		{
			WaitTargetTask->ValidData.AddDynamic(this, &UAbilityMove::OnTargetDataReceived);
			WaitTargetTask->ReadyForActivation();
		}
		else
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		}
	}
}

void UAbilityMove::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data)
{
	// Right now this code does not validate move destination
	
	if (Data.IsValid(0))
	{
		const FHitResult* HitResult = Data.Get(0)->GetHitResult();
		if (HitResult->bBlockingHit)
		{
			FVector2D Destination = FVector2D(HitResult->Location);

			ABaseSkaterCharacter* Skater = Cast<ABaseSkaterCharacter>(GetAvatarActorFromActorInfo());
			if (Skater)
			{
				Skater->SetMoveDestination(Destination);

				if (MoveDestinationVFX && IsLocallyControlled())
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						Skater->GetWorld(),
						MoveDestinationVFX,
						FVector(Destination, 0),
						FRotator::ZeroRotator
					);
				}
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
