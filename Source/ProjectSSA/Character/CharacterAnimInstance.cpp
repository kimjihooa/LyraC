// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"

UCharacterAnimInstance::UCharacterAnimInstance()
{
	
}

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	if (AActor* OwningActor = GetOwningActor())
	{
		//Bind bools with GameplayTag
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Firing"));
			ASC->RegisterGameplayTagEvent(FireTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UCharacterAnimInstance::OnFiringTagChanged);
			bGameplayTagIsFiring = ASC->HasMatchingGameplayTag(FireTag);
		}
	}
}
void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	void UpdateLocationData(float DeltaTime);
	void UpdateRotationData();
	void UpdateVelocityData();
	void UpdateAccelerationData();
	void UpdateWallDetectionHeuristic();
	void UpdateCharacterStateData(float DeltaTime);
	void UpdateBlendWeightData(float DeltaTime);
	void UpdateRootYawOffset(float DeltaTime);
	void UpdateAimingData();
	void UpdateJumpFallData();
	bIsFirstUpdate = false;
}

UCharacterMovementComponent* UCharacterAnimInstance::GetMovementComponent()
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn->IsValidLowLevelFast())
		return nullptr;
	UCharacterMovementComponent* CharacterMovement = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());
	return CharacterMovement;
}
void UCharacterAnimInstance::OnFiringTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	bGameplayTagIsFiring = (NewCount > 0);
}

void UCharacterAnimInstance::UpdateLocationData(float DeltaTime)
{
	DisplacementSinceLastUpdate = UKismetMathLibrary::VSizeXY(GetOwningActor()->GetActorLocation() - WorldLocation);
	WorldLocation = GetOwningActor()->GetActorLocation();
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, DeltaTime);
	if (bIsFirstUpdate)
	{
		DisplacementSinceLastUpdate = 0.0f;
		DisplacementSpeed = 0.0f;
	}
}
void UCharacterAnimInstance::UpdateRotationData()
{
	DisplacementSinceLastUpdate = GetOwningActor()->GetActorRotation().Yaw - WorldRotation.Yaw;
	YawDeltaSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, GetDeltaSeconds());
	WorldRotation = GetOwningActor()->GetActorRotation();
	AdditiveLeanAngle = (bIsCrouching || bGameplayTagIsADS ? 0.025 : 0.0375) * YawDeltaSpeed;
	if (bIsFirstUpdate)
	{
		YawDeltaSinceLastUpdate = 0.0f;
		AdditiveLeanAngle = 0.0f;
	}
}
void UCharacterAnimInstance::UpdateVelocityData()
{
	bWasMovingLastUpdate = !LocalVelocity2D.IsZero();
	WorldVelocity = TryGetPawnOwner()->GetVelocity();
	WorldVelocity2D = FVector(WorldVelocity.X, WorldVelocity.Y, 0.0f);
	LocalVelocity2D = WorldRotation.UnrotateVector(WorldVelocity2D);
	LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(WorldVelocity2D, WorldRotation);
	LocalVelocityDirectionAngleWithOffset = LocalVelocityDirectionAngle - RootYawOffset;
	LocalVelocityDirection = SelectCarialDirectionFromAngle(LocalVelocityDirectionAngleWithOffset, CardinalDirectionDeadZone, LocalVelocityDirection, bWasMovingLastUpdate);
	LocalVelocityDirectionNoOffset = SelectCarialDirectionFromAngle(LocalVelocityDirectionAngle, CardinalDirectionDeadZone, LocalVelocityDirectionNoOffset, bWasMovingLastUpdate);
	bHasVelocity = !UKismetMathLibrary::NearlyEqual_FloatFloat(LocalVelocity2D.SizeSquared2D(), 0.0f);
}
void UCharacterAnimInstance::UpdateAccelerationData()
{
	WorldAcceleration2D = FVector(GetMovementComponent()->GetCurrentAcceleration().X, GetMovementComponent()->GetCurrentAcceleration().Y, 0.0f);
	LocalAcceleration2D = WorldRotation.UnrotateVector(WorldAcceleration2D);
	bHasAcceleration = !UKismetMathLibrary::NearlyEqual_FloatFloat(LocalAcceleration2D.SizeSquared2D(), 0.0f);
	PivotDirection2D = FMath::Lerp(PivotDirection2D, WorldAcceleration2D.GetSafeNormal(), 0.5f).GetSafeNormal();
	CardinalDirectionFromAcceleration = SelectCarialDirectionFromAngle(UKismetAnimationLibrary::CalculateDirection(PivotDirection2D, WorldRotation), CardinalDirectionDeadZone, ECardinalDirection::Forward, false);
}
void UCharacterAnimInstance::UpdateWallDetectionHeuristic()
{
	bool AccLen = LocalAcceleration2D.Size2D() > 0.1f;
	bool VelLen = LocalVelocity2D.Size2D() < 200.0f;
	bool Dot = UKismetMathLibrary::InRange_FloatFloat(FVector::DotProduct(LocalAcceleration2D.GetSafeNormal(), LocalVelocity2D.GetSafeNormal()), -0.6f, 0.6f, true, true);
	bIsRunningIntoWall = AccLen && VelLen && Dot;
}
void UCharacterAnimInstance::UpdateCharacterStateData(float DeltaTime)
{
	bIsOnGround = GetMovementComponent()->IsMovingOnGround();
	bWasCrouchingLastUpdate = bIsCrouching;
	bIsCrouching = GetMovementComponent()->IsCrouching();
	bCrouchStateChange = bWasCrouchingLastUpdate != bIsCrouching;
	bADSStateChanged = bGameplayTagIsADS != bWasADSLastUpdate;
	bWasADSLastUpdate = bGameplayTagIsADS;
	TimeSinceFiredWeapon = bGameplayTagIsFiring ? 0.0f : TimeSinceFiredWeapon + DeltaTime;
	bIsJumping = false;
	bIsFalling = false;
	if (GetMovementComponent()->MovementMode == EMovementMode::MOVE_Falling)
	{
		if (WorldVelocity.Z > 0)
			bIsJumping = true;
		else
			bIsFalling = true;
	}
}
void UCharacterAnimInstance::UpdateBlendWeightData(float DeltaTime)
{
	UpperbodyDynamicAdditiveWeight = (IsAnyMontagePlaying() && bIsOnGround) ? 1.0f : (FMath::FInterpTo(UpperbodyDynamicAdditiveWeight, 0.0f, DeltaTime, 6.0f));
}
void UCharacterAnimInstance::UpdateRootYawOffset(float DeltaTime)
{
	if (RootYawOffsetMode == ERootYawOffsetMode::Accumulate)
		SetRootYawOffset(RootYawOffset - YawDeltaSinceLastUpdate);
	if (RootYawOffsetMode == ERootYawOffsetMode::BlendOut)
		SetRootYawOffset(UKismetMathLibrary::FloatSpringInterp(RootYawOffset, 0.0f, RootYawOffsetSpringState, 80.0f, 1.0f, DeltaTime, 1.0f, 0.5f));
	RootYawOffsetMode = ERootYawOffsetMode::BlendOut;
}
void UCharacterAnimInstance::UpdateAimingData()
{
	AimPitch = UKismetMathLibrary::NormalizeAxis(TryGetPawnOwner()->GetBaseAimRotation().Pitch);
}
void UCharacterAnimInstance::UpdateJumpFallData()
{
	if (bIsJumping)
		TimeToJumpApex = (0.0f - WorldVelocity.Z) / TryGetPawnOwner()->GetMovementComponent()->GetGravityZ();
	else
		TimeToJumpApex = 0.0f;
}


ECardinalDirection UCharacterAnimInstance::SelectCarialDirectionFromAngle(const float Angle, const float DeadZone, const ECardinalDirection CurrentDirection, const bool bUseCurrentDirection)
{
	float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = DeadZone;

	if (bUseCurrentDirection)
	{
		switch (CurrentDirection)
		{
		case ECardinalDirection::Forward:
			FwdDeadZone *= 2.0f;
			break;
		case ECardinalDirection::Backward:
			BwdDeadZone *= 2.0f;
			break;
		default:
			break;
		}
	}

	if (AbsAngle <= 45.0f + FwdDeadZone)
		return ECardinalDirection::Forward;
	if (AbsAngle >= 135.0f - BwdDeadZone)
		return ECardinalDirection::Backward;
	if (Angle > 0)
		return ECardinalDirection::Right;
	return ECardinalDirection::Left;
}
ECardinalDirection UCharacterAnimInstance::GetOppositeCardinalDirection(const ECardinalDirection CurrentDir)
{
	switch (CurrentDir)
	{
	case ECardinalDirection::Forward:
		return ECardinalDirection::Backward;
		break;
	case ECardinalDirection::Backward:
		return ECardinalDirection::Forward;
		break;
	case ECardinalDirection::Left:
		return ECardinalDirection::Right;
		break;
	case ECardinalDirection::Right:
		return ECardinalDirection::Left;
		break;
	}
	return ECardinalDirection::Forward;
}
void UCharacterAnimInstance::SetRootYawOffset(float InRootYawOffset)
{
	if (!bEnableRootYawOffset)
	{
		RootYawOffset = 0.0f;
		AimYaw = 0.0f;
	}
	else
	{
		FVector2D Clamp = bIsCrouching ? RootYawOffsetAngleClampCrouched : RootYawOffsetAngleClamp;
		float Normalized = UKismetMathLibrary::NormalizeAxis(InRootYawOffset);
		float Clamped = UKismetMathLibrary::ClampAngle(Normalized, Clamp.X, Clamp.Y);
		RootYawOffset = Clamp.X == Clamp.Y ? Normalized : Clamped;
		AimYaw = RootYawOffset * -1.0f;
	}
}