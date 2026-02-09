// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemAnimLayerInstance.h"

//Data Getter Proxy
FAnimInstanceProxy* UItemAnimLayerInstance::CreateAnimInstanceProxy()
{
	return &ItemAnimLayerProxy;
}
void UItemAnimLayerInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy)
{
}
void FItemAnimLayerInstanceProxy::PreUpdate(UAnimInstance* Instance, float DeltaSeconds)
{
	Super::PreUpdate(Instance, DeltaSeconds);
	
	UCharacterAnimInstance* MainAnimBPRef = nullptr;
	if (USkeletalMeshComponent* SKMesh = Instance->GetOwningComponent())
		if (UAnimInstance* AnimBP = SKMesh->GetAnimInstance())
			if (UCharacterAnimInstance* Result = Cast<UCharacterAnimInstance>(AnimBP))
				MainAnimBPRef = Result;

	UCharacterMovementComponent* MovementComponent = nullptr;
	if (APawn* PawnOwner = Instance->TryGetPawnOwner())
		if (UMovementComponent* Movement = PawnOwner->GetMovementComponent())
			if (UCharacterMovementComponent* CMC = Cast<UCharacterMovementComponent>(Movement))
				MovementComponent = CMC;

	if (MainAnimBPRef == nullptr || MovementComponent == nullptr)
		return;

	CachedbIsCrouching = MainAnimBPRef->CachedIsCroching;
	CachedbCrouchStateChange = MainAnimBPRef->bCrouchStateChange;
	CachedbGameplayTagIsADS = MainAnimBPRef->SafebGameplayTagIsADS;
	CachedbGameplayTagIsFiring = MainAnimBPRef->SafebGameplayTagIsFiring;
	CachedbIsOnGround = MainAnimBPRef->CachedIsMovingOnGround;
	CachedbIsJumping = MainAnimBPRef->bIsJumping;
	CachedbIsFalling = MainAnimBPRef->bIsFalling;
	CachedbHasVelocity = MainAnimBPRef->bHasVelocity;
	CachedbHasAcceleration = MainAnimBPRef->bHasAcceleration;
	CachedbIsRunningIntoWall = MainAnimBPRef->bIsRunningIntoWall;
	CachedbIsAnyMontagePlaying = MainAnimBPRef->IsAnyMontagePlaying();
	CachedbUseSeperateBrakingFriction = MovementComponent->bUseSeparateBrakingFriction;
	CachedBrakingFriction = MovementComponent->BrakingFriction;
	CachedBrakingFrictionFactor = MovementComponent->BrakingFrictionFactor;
	CachedDisplacementSpeed = MainAnimBPRef->DisplacementSpeed;
	CachedbUseFootPlacement = MainAnimBPRef->bUseFootPlacement;
	CachedRootYawOffset = MainAnimBPRef->RootYawOffset;
	CachedGroundDistance = MainAnimBPRef->GroundDistance;
	CachedTimeSinceFiredWeapon = MainAnimBPRef->TimeSinceFiredWeapon;
	CachedLastPivotTime = MainAnimBPRef->LastPivotTime;
	CachedGroundFriction = MovementComponent->GroundFriction;
	CachedBrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
	CachedDisplacementSinceLastUpdate = MainAnimBPRef->DisplacementSinceLastUpdate;
	CachedWorldLocation = MainAnimBPRef->WorldLocation;
	CachedLocalVelocity = MainAnimBPRef->LocalVelocity2D;
	CachedLocalVelocityDirection = MainAnimBPRef->LocalVelocityDirection;
	CachedLocalVelocityDirectionNoOffset = MainAnimBPRef->LocalVelocityDirectionNoOffset;
	CachedLocalAcceleration = MainAnimBPRef->LocalAcceleration2D;
	CachedCurrentAcceleration = MovementComponent->GetCurrentAcceleration();
	CachedLastUpdateVelocity = MovementComponent->GetLastUpdateVelocity();
	CachedCardinalDirectionFromAcceleration = MainAnimBPRef->CardinalDirectionFromAcceleration;
	ApplyHipFireCurve = Instance->GetCurveValue(CurveValue_ApplyHipFireCurve);
	DisableRHandCurve = Instance->GetCurveValue(CurveValue_DisableRHandCurve);
	DisableLHandCurve = Instance->GetCurveValue(CurveValue_DisableLHandCurve);
	DisableLegCurve = Instance->GetCurveValue(CurveValue_DisableLegCurve);
	DisableLeftHandPoseOverride = Instance->GetCurveValue(CurveValue_DisableLeftHandPoseOverride);
}
void FItemAnimLayerInstanceProxy::Update(float DeltaSeconds)
{
}

//Main Instance Data Update
UItemAnimLayerInstance::UItemAnimLayerInstance()
{
}
void UItemAnimLayerInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MainAnimBPRef = nullptr;
	if (USkeletalMeshComponent* SKMesh = GetOwningComponent())
		if (UAnimInstance* AnimBP = SKMesh->GetAnimInstance())
			if (UCharacterAnimInstance* Result = Cast<UCharacterAnimInstance>(AnimBP))
				MainAnimBPRef = Result;
	
	MovementComponent = nullptr;
	if (APawn* PawnOwner = TryGetPawnOwner())
		if (UMovementComponent* Movement = PawnOwner->GetMovementComponent())
			if (UCharacterMovementComponent* CMC = Cast<UCharacterMovementComponent>(Movement))
				MovementComponent = CMC;
}
void UItemAnimLayerInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MainAnimBPRef == nullptr || MovementComponent == nullptr)
		return;

	if (bChangeLastPivotTime)
	{
		MainAnimBPRef->LastPivotTime = LastPivotTime;
		bChangeLastPivotTime = false;
	}
	CachedCrouchStateChanged = ItemAnimLayerProxy.CachedbCrouchStateChange;
}
void UItemAnimLayerInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaTime);

	UpdateBlendWeightsData(DeltaTime);
	UpdateJumpFallData(DeltaTime);
	UpdateSkelControlData();
}

//Data Calculation Functions
UCharacterAnimInstance* UItemAnimLayerInstance::GetMainAnimBP()
{
	if (!MainAnimBPRef)
		return nullptr;
	return MainAnimBPRef;
}
void UItemAnimLayerInstance::UpdateBlendWeightsData(float DeltaTime)
{
	if ((!bRaiseWeaponAfterFiringWhenCrouched && ItemAnimLayerProxy.CachedbIsCrouching) || ((!ItemAnimLayerProxy.CachedbIsCrouching && ItemAnimLayerProxy.CachedbGameplayTagIsADS) && ItemAnimLayerProxy.CachedbIsOnGround))
	{
		HipFireUpperBodyOverrideWeight = 0.0f;
		AimOffsetBlendWeight = 1.0f;
	}
	else
	{
		if ((ItemAnimLayerProxy.CachedTimeSinceFiredWeapon < RaiseWeaponAfterFiringWeapon) || (ItemAnimLayerProxy.CachedbGameplayTagIsADS && (ItemAnimLayerProxy.CachedbIsCrouching || !ItemAnimLayerProxy.CachedbIsOnGround)) || (ItemAnimLayerProxy.ApplyHipFireCurve > 0.0f))
		{
			HipFireUpperBodyOverrideWeight = 1.0f;
			AimOffsetBlendWeight = 1.0f;
		}
		else
		{
			HipFireUpperBodyOverrideWeight = FMath::FInterpTo(HipFireUpperBodyOverrideWeight, 0.0f, DeltaTime, 1.0f);
			float NewAimOffsetBlendWeight = (FMath::Abs(ItemAnimLayerProxy.CachedRootYawOffset) < 10.0f && ItemAnimLayerProxy.CachedbHasAcceleration) ? HipFireUpperBodyOverrideWeight : 1.0f;
			AimOffsetBlendWeight = FMath::FInterpTo(AimOffsetBlendWeight, NewAimOffsetBlendWeight, DeltaTime, 10.0f);
		}
	}
}
void UItemAnimLayerInstance::UpdateJumpFallData(float DeltaTime)
{
	if (ItemAnimLayerProxy.CachedbIsFalling)
	{
		TimeFalling = TimeFalling + DeltaTime;
	}
	else
	{
		if (ItemAnimLayerProxy.CachedbIsJumping)
		{
			TimeFalling = 0.0f;
		}
	}
}
void UItemAnimLayerInstance::UpdateSkelControlData()
{
	float HandIKDIsable = bDisableHandIK ? 0.0f : 1.0f;
	HandIKRightAlpha = FMath::Clamp(HandIKDIsable - ItemAnimLayerProxy.DisableRHandCurve, 0.0f, 1.0f);
	HandIKLeftAlpha = FMath::Clamp(HandIKDIsable - ItemAnimLayerProxy.DisableLHandCurve, 0.0f, 1.0f);
}

//Node Binding Functions
void UItemAnimLayerInstance::SetLeftHandPoseOverrideWeight(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	LeftHandOverrideWeight = FMath::Clamp((bEnableLeftHandPoseOverride ? 1.0f : 0.0f) - ItemAnimLayerProxy.DisableLeftHandPoseOverride, 0.0f, 1.0f);
}
void UItemAnimLayerInstance::UpdateHipFireRaiseWeaponPose(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	USequenceEvaluatorLibrary::SetSequence(SequenceEval, ItemAnimLayerProxy.CachedbIsCrouching ? AimHipFirePoseCrouch : AimHipFirePose);
}
void UItemAnimLayerInstance::SetUpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEval, 0.0f);
}
void UItemAnimLayerInstance::UpdateFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEval, ItemAnimLayerProxy.CachedGroundDistance, JumpDistanceCurveName);
}
void UItemAnimLayerInstance::SetUpPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	PivotStartingAcceleration = ItemAnimLayerProxy.CachedLocalAcceleration;
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEval, GetDesiredPivotSequence(ItemAnimLayerProxy.CachedCardinalDirectionFromAcceleration));
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEval, 0.0f);
	StrideWarpingPivotAlpha = 0.0f;
	TimeAtPivotStop = 0.0f;
	LastPivotTime = 0.2f;
	bChangeLastPivotTime = true;
}
void UItemAnimLayerInstance::UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEval);
	if (ItemAnimLayerProxy.CachedLastPivotTime > 0.0f)
	{
		UAnimSequence* NewDesiredSeq = GetDesiredPivotSequence(ItemAnimLayerProxy.CachedCardinalDirectionFromAcceleration);
		if (NewDesiredSeq != USequenceEvaluatorLibrary::GetSequence(SequenceEval))
		{
			USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEval, NewDesiredSeq, 0.2f);
			PivotStartingAcceleration = ItemAnimLayerProxy.CachedLocalAcceleration;
		}
	}
	if (FVector::DotProduct(ItemAnimLayerProxy.CachedLocalVelocity, ItemAnimLayerProxy.CachedLocalAcceleration) < 0.0f)
	{
		float DistanceTarget = UAnimCharacterMovementLibrary::PredictGroundMovementPivotLocation(ItemAnimLayerProxy.CachedCurrentAcceleration, ItemAnimLayerProxy.CachedLastUpdateVelocity, ItemAnimLayerProxy.CachedGroundFriction).Size2D();
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEval, DistanceTarget, LocomotionDistanceCurveName);
		TimeAtPivotStop = ExplicitTime;
		bCanPivotRentry = false;
		return;
	}
	else
	{
		StrideWarpingPivotAlpha = UKismetMathLibrary::MapRangeClamped(ExplicitTime - TimeAtPivotStop - StrideWarpingBlendInStartOffset, 0.0f, StrideWarpingBlendInDurationScaled, 0.0f, 1.0f);
		FVector2D Clamp = FVector2D(FMath::Lerp(0.2, PlayRateClampStartsPivots.X, StrideWarpingPivotAlpha), PlayRateClampStartsPivots.Y);
		UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, SequenceEval, ItemAnimLayerProxy.CachedDisplacementSinceLastUpdate, LocomotionDistanceCurveName, Clamp);
		bCanPivotRentry = true;
		return;
	}
}
void UItemAnimLayerInstance::SetUpStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if (ItemAnimLayerProxy.CachedbIsCrouching)
	{
		switch (ItemAnimLayerProxy.CachedLocalVelocityDirection)
		{
		case ECardinalDirection::Forward:
			USequenceEvaluatorLibrary::SetSequence(SequenceEval, CrouchStopCardinal.Forward);
			break;
		case ECardinalDirection::Backward:
			USequenceEvaluatorLibrary::SetSequence(SequenceEval, CrouchStopCardinal.Backward);
			break;
		case ECardinalDirection::Left:
			USequenceEvaluatorLibrary::SetSequence(SequenceEval, CrouchStopCardinal.Left);
			break;
		case ECardinalDirection::Right:
			USequenceEvaluatorLibrary::SetSequence(SequenceEval, CrouchStopCardinal.Right);
			break;
		}
	}
	else
	{
		if (ItemAnimLayerProxy.CachedbGameplayTagIsADS)
		{
			switch (ItemAnimLayerProxy.CachedLocalVelocityDirection)
			{
			case ECardinalDirection::Forward:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, ADSStopCardinal.Forward);
				break;
			case ECardinalDirection::Backward:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, ADSStopCardinal.Backward);
				break;
			case ECardinalDirection::Left:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, ADSStopCardinal.Left);
				break;
			case ECardinalDirection::Right:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, ADSStopCardinal.Right);
				break;
			}
		}
		else
		{
			switch (ItemAnimLayerProxy.CachedLocalVelocityDirection)
			{
			case ECardinalDirection::Forward:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, JogStopCardinal.Forward);
				break;
			case ECardinalDirection::Backward:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, JogStopCardinal.Backward);
				break;
			case ECardinalDirection::Left:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, JogStopCardinal.Left);
				break;
			case ECardinalDirection::Right:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, JogStopCardinal.Right);
				break;
			}
		}
	}
	if (!ShouldDistanceMatchStop())
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEval, 0.0f, LocomotionDistanceCurveName);
	
	return;
}
void UItemAnimLayerInstance::UpdateStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if(ShouldDistanceMatchStop())
	{
		float DistanceToMatch = GetPredictedStopDistance();
		if (DistanceToMatch > 0.0f)
		{
			UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEval, DistanceToMatch, LocomotionDistanceCurveName);
			return;
		}
		else
		{
			USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEval, 1.0f);
			return;
		}
	}
	else
	{
		USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEval, 1.0f);
		return;
	}
}
void UItemAnimLayerInstance::UpdateCycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	if (ItemAnimLayerProxy.CachedbIsCrouching)
	{
		switch (ItemAnimLayerProxy.CachedLocalVelocityDirectionNoOffset)
		{
		case ECardinalDirection::Forward:
			USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, CrouchCardinal.Forward, 0.2f);
			break;
		case ECardinalDirection::Backward:
			USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, CrouchCardinal.Backward, 0.2f);
			break;
		case ECardinalDirection::Left:
			USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, CrouchCardinal.Left, 0.2f);
			break;
		case ECardinalDirection::Right:
			USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, CrouchCardinal.Right, 0.2f);
			break;
		}
	}
	else
	{
		if (ItemAnimLayerProxy.CachedbGameplayTagIsADS)
		{
			switch (ItemAnimLayerProxy.CachedLocalVelocityDirectionNoOffset)
			{
			case ECardinalDirection::Forward:
				USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, ADSCardinal.Forward, 0.2f);
				break;
			case ECardinalDirection::Backward:
				USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, ADSCardinal.Backward, 0.2f);
				break;
			case ECardinalDirection::Left:
				USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, ADSCardinal.Left, 0.2f);
				break;
			case ECardinalDirection::Right:
				USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, ADSCardinal.Right, 0.2f);
				break;
			}
		}
		else
		{
			switch (ItemAnimLayerProxy.CachedLocalVelocityDirectionNoOffset)
			{
			case ECardinalDirection::Forward:
				USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, JogCardinal.Forward, 0.2f);
				break;
			case ECardinalDirection::Backward:
				USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, JogCardinal.Backward, 0.2f);
				break;
			case ECardinalDirection::Left:
				USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, JogCardinal.Left, 0.2f);
				break;
			case ECardinalDirection::Right:
				USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, JogCardinal.Right, 0.2f);
				break;
			}
		}
	}
	UAnimDistanceMatchingLibrary::SetPlayrateToMatchSpeed(SequencePlayer, ItemAnimLayerProxy.CachedDisplacementSpeed, PlayRateClampCycle);
	StrideWarpingCycleAlpha = FMath::FInterpTo(StrideWarpingCycleAlpha, ItemAnimLayerProxy.CachedbIsRunningIntoWall ? 0.5f : 1.0f, Context.GetContext()->GetDeltaTime(), 10.0f);
	return;
}
void UItemAnimLayerInstance::SetUpStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if (ItemAnimLayerProxy.CachedbIsCrouching)
	{
		switch (ItemAnimLayerProxy.CachedLocalVelocityDirection)
		{
		case ECardinalDirection::Forward:
			USequenceEvaluatorLibrary::SetSequence(SequenceEval, CrouchStartCardinal.Forward);
			break;
		case ECardinalDirection::Backward:
			USequenceEvaluatorLibrary::SetSequence(SequenceEval, CrouchStartCardinal.Backward);
			break;
		case ECardinalDirection::Left:
			USequenceEvaluatorLibrary::SetSequence(SequenceEval, CrouchStartCardinal.Left);
			break;
		case ECardinalDirection::Right:
			USequenceEvaluatorLibrary::SetSequence(SequenceEval, CrouchStartCardinal.Right);
			break;
		}
	}
	else
	{
		if (ItemAnimLayerProxy.CachedbGameplayTagIsADS)
		{
			switch (ItemAnimLayerProxy.CachedLocalVelocityDirection)
			{
			case ECardinalDirection::Forward:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, ADSStartCardinal.Forward);
				break;
			case ECardinalDirection::Backward:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, ADSStartCardinal.Backward);
				break;
			case ECardinalDirection::Left:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, ADSStartCardinal.Left);
				break;
			case ECardinalDirection::Right:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, ADSStartCardinal.Right);
				break;
			}
		}
		else
		{
			switch (ItemAnimLayerProxy.CachedLocalVelocityDirection)
			{
			case ECardinalDirection::Forward:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, JogStartCardinal.Forward);
				break;
			case ECardinalDirection::Backward:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, JogStartCardinal.Backward);
				break;
			case ECardinalDirection::Left:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, JogStartCardinal.Left);
				break;
			case ECardinalDirection::Right:
				USequenceEvaluatorLibrary::SetSequence(SequenceEval, JogStartCardinal.Right);
				break;
			}
		}
	}
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEval, 0.0f);
	StrideWarpingStartAlpha = 0.0f;
}
void UItemAnimLayerInstance::UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEval);
	StrideWarpingStartAlpha = UKismetMathLibrary::MapRangeClamped(ExplicitTime - StrideWarpingBlendInStartOffset, 0.0f, StrideWarpingBlendInDurationScaled, 0.0f, 1.0f);
	FVector2D Clamp = FVector2D(FMath::Lerp(StrideWarpingBlendInDurationScaled, PlayRateClampStartsPivots.X, StrideWarpingStartAlpha), PlayRateClampStartsPivots.Y);
	UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, SequenceEval, ItemAnimLayerProxy.CachedDisplacementSinceLastUpdate, LocomotionDistanceCurveName, Clamp);
}
void UItemAnimLayerInstance::SetUpIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	ChooseIdleBreakDelayTime();
	ResetIdleBreakTransitionLogic();
}
void UItemAnimLayerInstance::UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	bool Result = false;
	FAnimationStateResultReference AnimState;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResultPure(Node, AnimState, Result);
	if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimState))
		ProcessIdleBreakTransitionLogic(Context.GetContext()->GetDeltaTime());
}
void UItemAnimLayerInstance::UpdateIdleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	if (ItemAnimLayerProxy.CachedbIsCrouching)
	{
		USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, IdleCrouch, 0.2f);
	}
	else
	{
		if (ItemAnimLayerProxy.CachedbGameplayTagIsADS)
		{
			USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, IdleADS, 0.2f);
		}
		else
		{
			USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, IdleHipFire, 0.2f);
		}
	}
}
void UItemAnimLayerInstance::SetUpIdleTransition(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	USequencePlayerLibrary::SetSequence(SequencePlayer, ItemAnimLayerProxy.CachedbIsCrouching ? IdleCrouchEntry : IdleCrouchExit);
	//UE_LOG(LogTemp, Warning, TEXT("Transtion"));
	return;
}
void UItemAnimLayerInstance::SetUpIdleBreakAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	USequencePlayerLibrary::SetSequence(SequencePlayer, IdleBreaks[CurrentIdleBreakIndex]);
	CurrentIdleBreakIndex++;
	if (CurrentIdleBreakIndex >= IdleBreaks.Num())
		CurrentIdleBreakIndex = 0;
}
void UItemAnimLayerInstance::SetUpTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceAnimTime = 0.0f;

	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEval, 0.0f);
}
void UItemAnimLayerInstance::UpdateTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEval, SelectTurnInPlaceAnimation(TurnInPlaceRotationDirection), 0.2f);
	TurnInPlaceAnimTime = TurnInPlaceAnimTime + Context.GetContext()->GetDeltaTime();
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEval, TurnInPlaceAnimTime);
}
void UItemAnimLayerInstance::SetUpTurnInPlaceRotationState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceRotationDirection = FMath::Sign(ItemAnimLayerProxy.CachedRootYawOffset) * -1.0f;
}
void UItemAnimLayerInstance::UpdateTurnInPlaceRecoveryAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, SelectTurnInPlaceAnimation(TurnInPlaceRecoveryDirection), 0.2f);
}
void UItemAnimLayerInstance::SetUpTurnInPlaceRecoveryState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceRecoveryDirection = TurnInPlaceRotationDirection;
}
void UItemAnimLayerInstance::LandRecoveryStart(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	LandRecoveryAlpha = UKismetMathLibrary::MapRangeClamped(TimeFalling, 0.0f, 0.4f, 0.1f, 1.0f) * (ItemAnimLayerProxy.CachedbIsCrouching ? 0.5f : 1.0f);
}

//Helper Functions
bool UItemAnimLayerInstance::ShouldEnableFootPlacement() const
{
	if (MainAnimBPRef == nullptr)
		return false;
	else
		return (ItemAnimLayerProxy.DisableLegCurve <= 0 && ItemAnimLayerProxy.CachedbUseFootPlacement);
}
bool UItemAnimLayerInstance::ShouldDistanceMatchStop() const
{
	return ItemAnimLayerProxy.CachedbHasVelocity && !ItemAnimLayerProxy.CachedbHasAcceleration;
}
float UItemAnimLayerInstance::GetPredictedStopDistance() const
{
	FVector Prediction = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(ItemAnimLayerProxy.CachedLastUpdateVelocity, ItemAnimLayerProxy.CachedbUseSeperateBrakingFriction, ItemAnimLayerProxy.CachedBrakingFriction, ItemAnimLayerProxy.CachedGroundFriction, ItemAnimLayerProxy.CachedBrakingFrictionFactor, ItemAnimLayerProxy.CachedBrakingDecelerationWalking);
	return Prediction.Size2D();
}
void UItemAnimLayerInstance::ChooseIdleBreakDelayTime()
{
	IdleBreakDelayTime = (FMath::TruncToInt(FMath::Abs(ItemAnimLayerProxy.CachedWorldLocation.X + ItemAnimLayerProxy.CachedWorldLocation.Y)) % 10) + 6.0f;
}
bool UItemAnimLayerInstance::CanPlayIdleBreak() const
{
	return (IdleBreaks.Num() > 0) && !(ItemAnimLayerProxy.CachedbIsCrouching || ItemAnimLayerProxy.CachedbGameplayTagIsADS || ItemAnimLayerProxy.CachedbGameplayTagIsFiring || ItemAnimLayerProxy.CachedbIsAnyMontagePlaying || ItemAnimLayerProxy.CachedbHasVelocity || ItemAnimLayerProxy.CachedbIsJumping);
}
void UItemAnimLayerInstance::ResetIdleBreakTransitionLogic()
{
	TimeUntilNextIdleBreak = IdleBreakDelayTime;
}
void UItemAnimLayerInstance::ProcessIdleBreakTransitionLogic(float DeltaTime)
{
	if (CanPlayIdleBreak())
		TimeUntilNextIdleBreak = TimeUntilNextIdleBreak - DeltaTime;
	else
		ResetIdleBreakTransitionLogic();
}
UAnimSequence* UItemAnimLayerInstance::GetDesiredPivotSequence(ECardinalDirection InDirection) const
{
	if (ItemAnimLayerProxy.CachedbIsCrouching)
	{
		switch (InDirection)
		{
		case ECardinalDirection::Forward:
			return CrouchPivotCardinal.Forward;
			break;
		case ECardinalDirection::Backward:
			return CrouchPivotCardinal.Backward;
			break;
		case ECardinalDirection::Left:
			return CrouchPivotCardinal.Left;
			break;
		case ECardinalDirection::Right:
			return CrouchPivotCardinal.Right;
			break;
		}
	}
	else
	{
		if (ItemAnimLayerProxy.CachedbGameplayTagIsADS)
		{
			switch (InDirection)
			{
			case ECardinalDirection::Forward:
				return ADSPivotCardinal.Forward;
				break;
			case ECardinalDirection::Backward:
				return ADSPivotCardinal.Backward;
				break;
			case ECardinalDirection::Left:
				return ADSPivotCardinal.Left;
				break;
			case ECardinalDirection::Right:
				return ADSPivotCardinal.Right;
				break;
			}
		}
		else
		{
			switch (InDirection)
			{
			case ECardinalDirection::Forward:
				return JogPivotCardinal.Forward;
				break;
			case ECardinalDirection::Backward:
				return JogPivotCardinal.Backward;
				break;
			case ECardinalDirection::Left:
				return JogPivotCardinal.Left;
				break;
			case ECardinalDirection::Right:
				return JogPivotCardinal.Right;
				break;
			}
		}
	}
	return JogPivotCardinal.Forward;
}
UAnimSequence* UItemAnimLayerInstance::SelectTurnInPlaceAnimation(float Direction)
{
	if (Direction > 0.0f)
		return ItemAnimLayerProxy.CachedbIsCrouching ? CrouchTurnInPlaceRight : TurnInPlaceRight;
	else
		return ItemAnimLayerProxy.CachedbIsCrouching ? CrouchTurnInPlaceLeft : TurnInPlaceLeft;
}

