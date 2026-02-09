// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"

//Main Instance Data Update
UCharacterAnimInstance::UCharacterAnimInstance()
{
	CurveName_TurnYawWeight = TEXT("TurnYawWeight");
	CurveName_RemainingTurnYaw = TEXT("RemainingTurnYaw");
	CurveName_DisableLegIK = TEXT("DisableLegIK");
}
void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	//Initialize Character Data
	AActor* OwningActor = GetOwningActor();
	APawn* OwningPawn = TryGetPawnOwner();
	
	if (IsValid(OwningActor))
		ActorRef = OwningActor;
	if (IsValid(OwningPawn))
	{
		PawnRef = OwningPawn;
		UCharacterMovementComponent* MovementC = GetMovementComponent();
		if (IsValid(MovementC))
			MovementRef = MovementC;
	}
	if (IsValid(OwningActor))
		MainCharacterRef = Cast<AMainCharacter>(OwningActor);

	//Bind bools with GameplayTag
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ActorRef))
	{
		FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Firing"));
		ASC->RegisterGameplayTagEvent(FireTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UCharacterAnimInstance::OnFiringTagChanged);
		SafebGameplayTagIsFiring = ASC->HasMatchingGameplayTag(FireTag);
	}
	
}
void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	//For Data Update
	Super::NativeUpdateAnimation(DeltaTime);

	if (!IsValid(ActorRef) || !IsValid(PawnRef) || !IsValid(MovementRef))
		return;

	CachedLocation = ActorRef->GetActorLocation();
	CachedRotation = ActorRef->GetActorRotation();
	CachedVelocity = PawnRef->GetVelocity();
	CachedAcceleration = MovementRef->GetCurrentAcceleration();
	CachedIsMovingOnGround = MovementRef->IsMovingOnGround();
	CachedIsCroching = MovementRef->IsCrouching();
	CachedMovementMode = MovementRef->MovementMode;
	CachedIsAnyMontagePlaying = IsAnyMontagePlaying();
	CachedAimPitch = PawnRef->GetBaseAimRotation().Pitch;
	CachedGravityZ = PawnRef->GetMovementComponent()->GetGravityZ();
	GroundDistance = MainCharacterRef->GetGroundDistance();
	TurnYawWeightCurve = GetCurveValue(CurveName_TurnYawWeight);
	RemainingTurnYawCurve = GetCurveValue(CurveName_RemainingTurnYaw);
	DisableLegIKCurve = GetCurveValue(CurveName_DisableLegIK);
}
void UCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
	//For Data Calculation (Safe multithread)
	Super::NativeThreadSafeUpdateAnimation(DeltaTime);

	UpdateLocationData(DeltaTime);
	UpdateRotationData(DeltaTime);
	UpdateVelocityData();
	UpdateAccelerationData();
	UpdateWallDetectionHeuristic();
	UpdateCharacterStateData(DeltaTime);
	UpdateBlendWeightData(DeltaTime);
	UpdateRootYawOffset(DeltaTime);
	UpdateAimingData();
	UpdateJumpFallData();

	bIsFirstUpdate = false;
}

//Tag BindingFunctions
void UCharacterAnimInstance::OnFiringTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SafebGameplayTagIsFiring = (NewCount > 0);
}

//Node Binding Functions
void UCharacterAnimInstance::UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference State;
	bool bSucceeded = true;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResultPure(Node, State, bSucceeded);
	if (UAnimationStateMachineLibrary::IsStateBlendingOut(Context, State))
		TurnYawCurveValue = 0.0f;
	else
	{
		RootYawOffsetMode = ERootYawOffsetMode::Accumulate;
		ProcessTurnYawCurve();
	}
}
void UCharacterAnimInstance::SetUpStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	StartDirection = LocalVelocityDirection;
}
void UCharacterAnimInstance::UpdateStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference State;
	bool bSucceeded = true;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResultPure(Node, State, bSucceeded);
	if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, State))
		RootYawOffsetMode = ERootYawOffsetMode::Hold;
}
void UCharacterAnimInstance::UpdateStopState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference State;
	bool bSucceeded = true;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResultPure(Node, State, bSucceeded);
	if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, State))
		RootYawOffsetMode = ERootYawOffsetMode::Accumulate;
}
void UCharacterAnimInstance::SetUpPivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	PivotInitialDirection = LocalVelocityDirection;
}
void UCharacterAnimInstance::UpdatePivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (LastPivotTime > 0.0f)
		LastPivotTime -= UAnimExecutionContextLibrary::GetDeltaTime(Context);
}
void UCharacterAnimInstance::UpdateLocomotionStateMachine(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FLinkedAnimGraphReference LinkedGraph = ULinkedAnimGraphLibrary::ConvertToLinkedAnimGraph(Node, Result);
	UAnimInstance* CurrentInstance = ULinkedAnimGraphLibrary::GetLinkedAnimInstance(LinkedGraph);
	if (!bIsFirstUpdate)
		bLinkedLayerChanged = CurrentInstance != LastLinkedLayer;
	LastLinkedLayer = CurrentInstance;
}
bool UCharacterAnimInstance::ShouldEnableControlRig()
{
	return (DisableLegIKCurve <= 0.0f) && !bUseFootPlacement;
}

//Data Update Functions
UCharacterMovementComponent* UCharacterAnimInstance::GetMovementComponent()
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn->IsValidLowLevelFast())
		return nullptr;
	UCharacterMovementComponent* CharacterMovement = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());
	return CharacterMovement;
}
void UCharacterAnimInstance::UpdateLocationData(float DeltaTime)
{
	DisplacementSinceLastUpdate = UKismetMathLibrary::VSizeXY(CachedLocation - WorldLocation);
	WorldLocation = CachedLocation;
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, DeltaTime);
	if (bIsFirstUpdate)
	{
		DisplacementSinceLastUpdate = 0.0f;
		DisplacementSpeed = 0.0f;
	}
}
void UCharacterAnimInstance::UpdateRotationData(float DeltaTime)
{
	YawDeltaSinceLastUpdate = UKismetMathLibrary::NormalizeAxis(CachedRotation.Yaw - WorldRotation.Yaw);
	YawDeltaSpeed = UKismetMathLibrary::SafeDivide(YawDeltaSinceLastUpdate, DeltaTime);
	WorldRotation = CachedRotation;
	bGameplayTagIsADS = SafebGameplayTagIsADS;
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
	WorldVelocity = CachedVelocity;
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
	WorldAcceleration2D = FVector(CachedAcceleration.X, CachedAcceleration.Y, 0.0f);
	LocalAcceleration2D = WorldRotation.UnrotateVector(WorldAcceleration2D);
	bHasAcceleration = !UKismetMathLibrary::NearlyEqual_FloatFloat(LocalAcceleration2D.SizeSquared2D(), 0.0f);
	PivotDirection2D = FMath::Lerp(PivotDirection2D, WorldAcceleration2D.GetSafeNormal(), 0.5f).GetSafeNormal();
	CardinalDirectionFromAcceleration = GetOppositeCardinalDirection(SelectCarialDirectionFromAngle(UKismetAnimationLibrary::CalculateDirection(PivotDirection2D, WorldRotation), CardinalDirectionDeadZone, ECardinalDirection::Forward, false));
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
	bIsOnGround = CachedIsMovingOnGround;
	bWasCrouchingLastUpdate = bIsCrouching;
	bIsCrouching = CachedIsCroching;
	bCrouchStateChange = bWasCrouchingLastUpdate != bIsCrouching;
	bGameplayTagIsADS = SafebGameplayTagIsADS;
	bADSStateChanged = bGameplayTagIsADS != bWasADSLastUpdate;
	bWasADSLastUpdate = bGameplayTagIsADS;
	bGameplayTagIsFiring = SafebGameplayTagIsFiring;
	TimeSinceFiredWeapon = bGameplayTagIsFiring ? 0.0f : TimeSinceFiredWeapon + DeltaTime;
	bIsJumping = false;
	bIsFalling = false;
	if (CachedMovementMode == EMovementMode::MOVE_Falling)
	{
		if (WorldVelocity.Z > 0)
			bIsJumping = true;
		else
			bIsFalling = true;
	}
}
void UCharacterAnimInstance::UpdateBlendWeightData(float DeltaTime)
{
	UpperbodyDynamicAdditiveWeight = (CachedIsAnyMontagePlaying && bIsOnGround) ? 1.0f : (FMath::FInterpTo(UpperbodyDynamicAdditiveWeight, 0.0f, DeltaTime, 6.0f));
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
	AimPitch = UKismetMathLibrary::NormalizeAxis(CachedAimPitch);
}
void UCharacterAnimInstance::UpdateJumpFallData()
{
	if (bIsJumping)
		TimeToJumpApex = UKismetMathLibrary::SafeDivide((0.0f - WorldVelocity.Z), CachedGravityZ);
	else
		TimeToJumpApex = 0.0f;
}

//Helper Functions
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
bool UCharacterAnimInstance::IsMovingPerpendicularToInitialPivot() const
{
	bool PivotFB = (PivotInitialDirection == ECardinalDirection::Forward) || (PivotInitialDirection == ECardinalDirection::Backward);
	bool VelocityFB = (LocalVelocityDirection == ECardinalDirection::Forward) || (LocalVelocityDirection == ECardinalDirection::Backward);
	bool PivotLR = (PivotInitialDirection == ECardinalDirection::Left) || (PivotInitialDirection == ECardinalDirection::Right);
	bool VelocityLR = (LocalVelocityDirection == ECardinalDirection::Left) || (LocalVelocityDirection == ECardinalDirection::Right);
	return (PivotFB && !VelocityFB) || (PivotLR && !VelocityLR);
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
void UCharacterAnimInstance::ProcessTurnYawCurve()
{
	float PreviousTurnYawCurveValue = TurnYawCurveValue;
	
	float TurnYawWeight = TurnYawWeightCurve;
	if (UKismetMathLibrary::NearlyEqual_FloatFloat(TurnYawWeight, 0.0f))
	{
		TurnYawCurveValue = 0.0f;
		PreviousTurnYawCurveValue = 0.0f;
	}
	else
	{
		TurnYawCurveValue = RemainingTurnYawCurve / TurnYawWeight;
		if (PreviousTurnYawCurveValue != 0.0f)
			SetRootYawOffset(RootYawOffset - (TurnYawCurveValue - PreviousTurnYawCurveValue));
	}
}
