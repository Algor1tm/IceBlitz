// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseSkaterCharacter.h"
#include "Puck.h"
#include "PlayerCamera.h"
#include "StickComponent.h"
#include "SkaterController.h"
#include "AbilitySystem/SkaterAbility.h"
#include "AbilitySystem/SkaterAttributeSet.h"

#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

#define STOP_SPEED 0.1f
#define STOP_TOLERANCE 5.f


ABaseSkaterCharacter::ABaseSkaterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Stick = CreateDefaultSubobject<UStickComponent>(TEXT("Stick"));
	Stick->SetupAttachment(GetMesh());

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<USkaterAttributeSet>(TEXT("AttributeSet"));

	StickPickUpTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("StickPickUpTrigger"));
	StickPickUpTrigger->SetupAttachment(Stick);

	MeshPickUpTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("MeshPickUpTrigger"));
	MeshPickUpTrigger->SetupAttachment(GetMesh());

	StickStealRangeTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("StickStealTrigger"));
	StickStealRangeTrigger->SetupAttachment(Stick);

	MeshStealRangeTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("MeshStealTrigger"));
	MeshStealRangeTrigger->SetupAttachment(GetMesh());

	EnemyStealRangeTrigger = CreateDefaultSubobject<UCapsuleComponent>(TEXT("EnemyStealTrigger"));
	EnemyStealRangeTrigger->SetupAttachment(GetMesh());
	EnemyStealRangeTrigger->SetIsReplicated(true);
}

void ABaseSkaterCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->GroundFriction = GroundFriction;
	GetCharacterMovement()->BrakingFriction = BrakingFriction;
	GetCharacterMovement()->MaxWalkSpeed = MaxSkateSpeed;
	GetCharacterMovement()->MaxAcceleration = MaxAcceleration;
	GetCharacterMovement()->RotationRate = FRotator(0.f, RotationSpeed, 0.f);
	EnableOrientRotationToMovement(bOrientRotationToMovement);


	if (HasAuthority())
	{
		StickPickUpTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnPuckOverlap);
		MeshPickUpTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnPuckOverlap);

		StickStealRangeTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnStealRangeBegin);
		MeshStealRangeTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnStealRangeBegin);

		StickStealRangeTrigger->OnComponentEndOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnStealRangeEnd);
		MeshStealRangeTrigger->OnComponentEndOverlap.AddDynamic(this, &ABaseSkaterCharacter::OnStealRangeEnd);


		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AttributeSet->InitMaxAcceleration(MaxAcceleration);
		AttributeSet->InitMaxSkateSpeed(MaxSkateSpeed);
		AttributeSet->InitSkateSpeed(SkateSpeed);
		AttributeSet->InitShotCharge(0.f);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			USkaterAttributeSet::GetMaxAccelerationAttribute()).AddUObject(this, &ABaseSkaterCharacter::OnMaxAccelerationChanged);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			USkaterAttributeSet::GetMaxSkateSpeedAttribute()).AddUObject(this, &ABaseSkaterCharacter::OnMaxSkateSpeedChanged);

		FGameplayAbilitySpec MoveAbilitySpec(MoveAbility, 1, (uint32)ESkaterAbilityInputID::Move, this);
		AbilitySystemComponent->GiveAbility(MoveAbilitySpec);

		FGameplayAbilitySpec StopAbilitySpec(StopAbility, 1, (uint32)ESkaterAbilityInputID::Stop, this);
		AbilitySystemComponent->GiveAbility(StopAbilitySpec);

		FGameplayAbilitySpec ShootAbilitySpec(ShootAbility, 1, (uint32)ESkaterAbilityInputID::Shoot, this);
		AbilitySystemComponent->GiveAbility(ShootAbilitySpec);

		FGameplayAbilitySpec StealAbilitySpec(StealAbility, 1, (uint32)ESkaterAbilityInputID::Steal, this);
		AbilitySystemComponent->GiveAbility(StealAbilitySpec);
	}
}

void ABaseSkaterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving && IsLocallyControlled())
	{
		FVector Velocity = GetCharacterMovement()->Velocity;
		FVector Direction = ComputePlanarVector(GetActorLocation(), MoveDestination);

		float Speed = Velocity.Length();
		float Distance = Direction.Length();

		if (Speed <= MinSpeed && Distance <= MinDistance)
		{
			AddMovementInput(Direction.GetSafeNormal(), STOP_SPEED);

			FVector ActorLocation = GetActorLocation();
			FVector2f ActorPlanarLocation = FVector2f(ActorLocation.X, ActorLocation.Y);

			if (ActorPlanarLocation.Equals(MoveDestination, STOP_TOLERANCE))
			{
				bIsMoving = false;
				GetCharacterMovement()->StopMovementImmediately();
			}
		}
		else
		{
			AddMovementInput(Direction.GetSafeNormal(), AttributeSet->GetSkateSpeed());
		}
	}
}

void ABaseSkaterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnMoveInput);
		EnhancedInputComponent->BindAction(StopInputAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnStopInput);
		EnhancedInputComponent->BindAction(ShootInputAction, ETriggerEvent::Started, this, &ABaseSkaterCharacter::OnShootInputPressed);
		EnhancedInputComponent->BindAction(ShootInputAction, ETriggerEvent::Completed, this, &ABaseSkaterCharacter::OnShootInputReleased);
	}
}

void ABaseSkaterCharacter::OnMoveInput()
{
	FVector2f CursorTarget = GetCursorTarget();
	FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit();
	TargetData->HitResult.bBlockingHit = true;
	TargetData->HitResult.Location = FVector(CursorTarget.X, CursorTarget.Y, 0.f);

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Add(TargetData);

	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName("Event.AbilityTrigger.Move"));

	FGameplayEventData Payload;
	Payload.Instigator = this;
	Payload.Target = this;
	Payload.TargetData = TargetDataHandle;

	AbilitySystemComponent->HandleGameplayEvent(EventTag, &Payload);
}

void ABaseSkaterCharacter::OnStopInput()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Stop);
}

void ABaseSkaterCharacter::OnShootInputPressed()
{
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Shoot);
	AbilitySystemComponent->AbilityLocalInputPressed((uint32)ESkaterAbilityInputID::Steal);
}

void ABaseSkaterCharacter::OnShootInputReleased()
{
	FGameplayAbilitySpec* ShootSpec = AbilitySystemComponent->FindAbilitySpecFromInputID((uint32)ESkaterAbilityInputID::Shoot);

	if (ShootSpec && ShootSpec->Ability)
	{
		ShootSpec->InputPressed = false;

		// Call RPC without local checking if ability is active
		ServerOnShootInputReleased();
	}
}

void ABaseSkaterCharacter::ServerOnShootInputReleased_Implementation()
{
	FGameplayAbilitySpec* ShootSpec = AbilitySystemComponent->FindAbilitySpecFromInputID((uint32)ESkaterAbilityInputID::Shoot);
	if (ShootSpec && ShootSpec->IsActive())
	{
		AbilitySystemComponent->AbilitySpecInputReleased(*ShootSpec);
	}
}

UAbilitySystemComponent* ABaseSkaterCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABaseSkaterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseSkaterCharacter, bIsMoving);
	DOREPLIFETIME(ABaseSkaterCharacter, MoveDestination);
	DOREPLIFETIME(ABaseSkaterCharacter, Puck);
}

float ABaseSkaterCharacter::ShootPuck()
{
	if (!Puck || !HasAuthority())
		return 0.f;

	FVector2f Cursor = GetCursorTarget();
	FVector Direction = ComputePlanarVector(Puck->GetActorLocation(), Cursor);
	float DistanceToCursor = Direction.Length();
	Direction = Direction.GetSafeNormal();
	float Charge = AttributeSet->GetShotCharge();

	float Power = ComputeShotPower(Charge, Direction, DistanceToCursor, GetCharacterMovement()->Velocity);

	FGameplayTag HasPuckTag = FGameplayTag::RequestGameplayTag(FName("SkaterState.HasPuck"));
	AbilitySystemComponent->RemoveLooseGameplayTag(HasPuckTag);

	DisablePuckPickUpForTime();

	Puck->OnRelease();
	if(!Puck->HasOwner())
		Puck->Shoot(Direction, Power);

	Puck = nullptr;

	ClientStop(Direction);

	return Power;
}

float ABaseSkaterCharacter::ComputeShotPower(float Charge, const FVector& Direction, float DistanceToCursor, const FVector& SkaterVelocity) const
{
	float VelocityProjection = FVector::DotProduct(SkaterVelocity.GetSafeNormal(), Direction);
	float Speed = SkaterVelocity.Length() * VelocityProjection;

	float NormDistanceToCursor = FMath::Clamp(DistanceToCursor / ShootMaxDistanceToCursor, 0.f, 1.f);
	float NormalizedSpeed = Speed / MaxSkateSpeed;

	float TotalModifiers = ShootChargeFactor * Charge + ShootDistanceToCursorFactor * NormDistanceToCursor + ShootSpeedFactor * NormalizedSpeed;
	float Power = ShootMinPower + ShootBasePower * TotalModifiers;

	return Power;
}

void ABaseSkaterCharacter::ClientStop_Implementation(FVector DirectionToFace)
{
	FGameplayAbilitySpec* StopSpec = AbilitySystemComponent->FindAbilitySpecFromInputID((uint32)ESkaterAbilityInputID::Stop);
	if (StopSpec)
		AbilitySystemComponent->TryActivateAbility(StopSpec->Handle);

	FaceDirection(DirectionToFace);
}

void ABaseSkaterCharacter::SetMoveDestination(FVector2f Destination)
{
	MoveDestination = Destination;
	bIsMoving = true;

	if (bOrientRotationToMovement)
		EnableOrientRotationToMovement(true);
}

void ABaseSkaterCharacter::StopMovement()
{
	bIsMoving = false;
	MoveDestination = FVector2f(0.f);

	if (bOrientRotationToMovement)
		EnableOrientRotationToMovement(false);
}

void ABaseSkaterCharacter::EnableOrientRotationToMovement(bool Enable)
{
	bUseControllerRotationPitch = !Enable;
	bUseControllerRotationYaw = !Enable;
	bUseControllerRotationRoll = !Enable;

	GetCharacterMovement()->bOrientRotationToMovement = Enable;

	if (Controller && !Enable)
		Controller->SetControlRotation(GetActorRotation());
}

void ABaseSkaterCharacter::FaceDirection(const FVector& Direction)
{
	if (Controller && !Direction.IsNearlyZero())
	{
		FRotator Rotation = Direction.Rotation();
		Controller->SetControlRotation(Rotation);
	}
}

void ABaseSkaterCharacter::OnPuckOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!HasAuthority())
		return;

	APuck* PuckActor = Cast<APuck>(OtherActor);

	if (PuckActor && !PuckActor->HasOwner() && !Puck)
	{
		PickUpPuck(PuckActor);
	}
}

void ABaseSkaterCharacter::PickUpPuck(APuck* aPuck)
{
	// Future: Trigger one-timer shot here

	OnPuckReceive(aPuck);
}

void ABaseSkaterCharacter::OnPuckReceive(APuck* aPuck)
{
	if (!HasAuthority())
		return;

	Puck = aPuck;
	Puck->SetSkaterOwner(this);

	FGameplayTag HasPuckTag = FGameplayTag::RequestGameplayTag(FName("SkaterState.HasPuck"));
	AbilitySystemComponent->AddLooseGameplayTag(HasPuckTag);
}

void ABaseSkaterCharacter::DisablePuckPickUpForTime()
{
	StickPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorld()->GetTimerManager().SetTimer(
		DisablePickUpTimerHandle,
		this,
		&ABaseSkaterCharacter::EnablePuckPickUp,
		PuckPickUpCooldown,
		false
	);
}

void ABaseSkaterCharacter::EnablePuckPickUp()
{
	StickPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshPickUpTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ABaseSkaterCharacter::OnStealRangeBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
		return;

	ABaseSkaterCharacter* SkaterActor = Cast<ABaseSkaterCharacter>(OtherActor);

	if (!IsValid(SkaterActor) || SkaterActor == this)
		return;

	uint8& OverlapCount = StealableCharacters.FindOrAdd(SkaterActor, 0);
	OverlapCount++;

	if (OverlapCount > 2)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Invalid stealable characters map!"));
		return;
	}
}

void ABaseSkaterCharacter::OnStealRangeEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
		return;

	ABaseSkaterCharacter* SkaterActor = Cast<ABaseSkaterCharacter>(OtherActor);

	if (!IsValid(SkaterActor) || SkaterActor == this)
		return;

	uint8* OverlapCountPtr = StealableCharacters.Find(SkaterActor);

	if (OverlapCountPtr == nullptr || *OverlapCountPtr == 0 || *OverlapCountPtr > 2)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Invalid stealable characters map!"));
		return;
	}

	*OverlapCountPtr -= 1;

	if (*OverlapCountPtr == 0)
	{
		StealableCharacters.Remove(SkaterActor);
	}

}

bool ABaseSkaterCharacter::TryStealPuck()
{
	if (Puck || !HasAuthority())
		return false;
	
	ABaseSkaterCharacter* Target = nullptr;
	for (const auto& [Skater, Count] : StealableCharacters)
	{
		if (Skater->Puck != nullptr)
		{
			Target = Skater;
			break;
		}
	}

	if (!Target)
		return false;

	APuck* OtherPuck = Target->Puck;
	Target->OnPuckStolen();

	OnPuckReceive(OtherPuck);

	return true;
}

void ABaseSkaterCharacter::OnPuckStolen()
{
	if (!Puck || !HasAuthority())
		return;

	Puck = nullptr;

	FGameplayTag HasPuckTag = FGameplayTag::RequestGameplayTag(FName("SkaterState.HasPuck"));
	AbilitySystemComponent->RemoveLooseGameplayTag(HasPuckTag);
}

FVector2f ABaseSkaterCharacter::GetCursorTarget() const
{
	ASkaterController* SkaterController = Cast<ASkaterController>(GetController());
	if (!SkaterController)
	{
		UE_LOG(LogTemp, Error, TEXT("ABaseSkaterCharacter: failed to get SkaterController"));
		return FVector2f(0.f);
	}

	return SkaterController->GetCursorTarget();
}

FVector ABaseSkaterCharacter::ComputePlanarVector(FVector Left, FVector2f Right) const
{
	FVector2f PlanarLocation = FVector2f(Left.X, Left.Y);

	FVector2f PlanarWorldVector = Right - PlanarLocation;
	FVector Vector = FVector(PlanarWorldVector.X, PlanarWorldVector.Y, 0);

	return Vector;
}

void ABaseSkaterCharacter::OnMaxAccelerationChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxAcceleration = Data.NewValue;
}

void ABaseSkaterCharacter::OnMaxSkateSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}
