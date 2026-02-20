// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemAnimLayerInstance.h"


void FLinkedAnimInstanceProxy::PreUpdate(UAnimInstance* Instance, float DeltaTime)
{
	Super::PreUpdate(Instance, DeltaTime);

	ApplyHipFireCurve = Instance->GetCurveValue(CurveValue_ApplyHipFireCurve);
	DisableRHandCurve = Instance->GetCurveValue(CurveValue_DisableRHandCurve);
	DisableLHandCurve = Instance->GetCurveValue(CurveValue_DisableLHandCurve);
	DisableLegCurve = Instance->GetCurveValue(CurveValue_DisableLegCurve);
	DisableLeftHandPoseOverride = Instance->GetCurveValue(CurveValue_DisableLeftHandPoseOverride);
}

FAnimInstanceProxy* UItemAnimLayerInstance::CreateAnimInstanceProxy()
{
	return &LinkedAnimInstanceProxy;
}
void UItemAnimLayerInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy)
{
}

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
//Get Data from outside
void UItemAnimLayerInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MainAnimBPRef == nullptr || MovementComponent == nullptr)
		return;

	LinkedAnimInstanceProxy.CachedbIsCrouching = MainAnimBPRef->GetIsCrouching();
	LinkedAnimInstanceProxy.CachedbCrouchStateChange = MainAnimBPRef->GetCrouchStateChanged();
	LinkedAnimInstanceProxy.CachedbGameplayTagIsADS = MainAnimBPRef->SafebGameplayTagIsADS;
	LinkedAnimInstanceProxy.CachedbGameplayTagIsFiring = MainAnimBPRef->SafebGameplayTagIsFiring;
	LinkedAnimInstanceProxy.CachedbIsOnGround = MainAnimBPRef->GetIsMovingOnGround();
	LinkedAnimInstanceProxy.CachedbIsJumping = MainAnimBPRef->GetIsJumping();
	LinkedAnimInstanceProxy.CachedbIsFalling = MainAnimBPRef->GetIsFalling();
	LinkedAnimInstanceProxy.CachedbHasVelocity = MainAnimBPRef->GetHasVelocity();
	LinkedAnimInstanceProxy.CachedbHasAcceleration = MainAnimBPRef->GetHasAcceleration();
	LinkedAnimInstanceProxy.CachedbIsRunningIntoWall = MainAnimBPRef->GetIsRunningIntoWall();
	LinkedAnimInstanceProxy.CachedbIsAnyMontagePlaying = MainAnimBPRef->IsAnyMontagePlaying();
	LinkedAnimInstanceProxy.CachedbUseSeperateBrakingFriction = MovementComponent->bUseSeparateBrakingFriction;
	LinkedAnimInstanceProxy.CachedBrakingFriction = MovementComponent->BrakingFriction;
	LinkedAnimInstanceProxy.CachedBrakingFrictionFactor = MovementComponent->BrakingFrictionFactor;
	LinkedAnimInstanceProxy.CachedDisplacementSpeed = MainAnimBPRef->GetDisplacementSpeed();
	LinkedAnimInstanceProxy.CachedbUseFootPlacement = MainAnimBPRef->bUseFootPlacement;
	LinkedAnimInstanceProxy.CachedRootYawOffset = MainAnimBPRef->GetRootYawOffset();
	LinkedAnimInstanceProxy.CachedGroundDistance = MainAnimBPRef->GetGroundDistance();
	LinkedAnimInstanceProxy.CachedTimeSinceFiredWeapon = MainAnimBPRef->GetTimeSinceFiredWeapon();
	LinkedAnimInstanceProxy.CachedLastPivotTime = MainAnimBPRef->GetLastPivotTime();
	LinkedAnimInstanceProxy.CachedGroundFriction = MovementComponent->GroundFriction;
	LinkedAnimInstanceProxy.CachedBrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
	LinkedAnimInstanceProxy.CachedDisplacementSinceLastUpdate = MainAnimBPRef->GetDisplacementSinceLastUpdate();
	LinkedAnimInstanceProxy.CachedWorldLocation = MainAnimBPRef->GetWorldLocation();
	LinkedAnimInstanceProxy.CachedLocalVelocity = MainAnimBPRef->GetLocalVelocty2D();
	LinkedAnimInstanceProxy.CachedLocalVelocityDirection = MainAnimBPRef->GetLocalVelocityDirection();
	LinkedAnimInstanceProxy.CachedLocalVelocityDirectionNoOffset = MainAnimBPRef->GetLocalVelocityDirectionNoOffset();
	LinkedAnimInstanceProxy.CachedLocalAcceleration = MainAnimBPRef->GetLocalAcceleration2D();
	LinkedAnimInstanceProxy.CachedCurrentAcceleration = MovementComponent->GetCurrentAcceleration();
	LinkedAnimInstanceProxy.CachedLastUpdateVelocity = MovementComponent->GetLastUpdateVelocity();
	LinkedAnimInstanceProxy.CachedCardinalDirectionFromAcceleration = MainAnimBPRef->GetCardinalDirectionFromAcceleration();
	if (bChangeLastPivotTime)
	{
		MainAnimBPRef->SetLastPivotTime();
		bChangeLastPivotTime = false;
	}
}
//Calculate data
void UItemAnimLayerInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaTime);

	UpdateBlendWeightsData(DeltaTime);
	UpdateJumpFallData(DeltaTime);
	UpdateSkelControlData();
}

//Data update functions
UCharacterAnimInstance* UItemAnimLayerInstance::GetMainAnimBP()
{
	if (!MainAnimBPRef)
		return nullptr;
	return MainAnimBPRef;
}
void UItemAnimLayerInstance::UpdateBlendWeightsData(float DeltaTime)
{
	if ((!bRaiseWeaponAfterFiringWhenCrouched && LinkedAnimInstanceProxy.CachedbIsCrouching) || ((!LinkedAnimInstanceProxy.CachedbIsCrouching && LinkedAnimInstanceProxy.CachedbGameplayTagIsADS) && LinkedAnimInstanceProxy.CachedbIsOnGround))
	{
		LinkedAnimInstanceProxy.HipFireUpperBodyOverrideWeight = 0.0f;
		LinkedAnimInstanceProxy.AimOffsetBlendWeight = 1.0f;
	}
	else
	{
		if ((LinkedAnimInstanceProxy.CachedTimeSinceFiredWeapon < RaiseWeaponAfterFiringWeapon) || (LinkedAnimInstanceProxy.CachedbGameplayTagIsADS && (LinkedAnimInstanceProxy.CachedbIsCrouching || !LinkedAnimInstanceProxy.CachedbIsOnGround)) || (LinkedAnimInstanceProxy.ApplyHipFireCurve > 0.0f))
		{
			LinkedAnimInstanceProxy.HipFireUpperBodyOverrideWeight = 1.0f;
			LinkedAnimInstanceProxy.AimOffsetBlendWeight = 1.0f;
		}
		else
		{
			LinkedAnimInstanceProxy.HipFireUpperBodyOverrideWeight = FMath::FInterpTo(LinkedAnimInstanceProxy.HipFireUpperBodyOverrideWeight, 0.0f, DeltaTime, 1.0f);
			float NewAimOffsetBlendWeight = (FMath::Abs(LinkedAnimInstanceProxy.CachedRootYawOffset) < 10.0f && LinkedAnimInstanceProxy.CachedbHasAcceleration) ? LinkedAnimInstanceProxy.HipFireUpperBodyOverrideWeight : 1.0f;
			LinkedAnimInstanceProxy.AimOffsetBlendWeight = FMath::FInterpTo(LinkedAnimInstanceProxy.AimOffsetBlendWeight, NewAimOffsetBlendWeight, DeltaTime, 10.0f);
		}
	}
}
void UItemAnimLayerInstance::UpdateJumpFallData(float DeltaTime)
{
	if (LinkedAnimInstanceProxy.CachedbIsFalling)
	{
		LinkedAnimInstanceProxy.TimeFalling = LinkedAnimInstanceProxy.TimeFalling + DeltaTime;
	}
	else
	{
		if (LinkedAnimInstanceProxy.CachedbIsJumping)
		{
			LinkedAnimInstanceProxy.TimeFalling = 0.0f;
		}
	}
}
void UItemAnimLayerInstance::UpdateSkelControlData()
{
	float HandIKDIsable = bDisableHandIK ? 0.0f : 1.0f;
	LinkedAnimInstanceProxy.HandIKRightAlpha = FMath::Clamp(HandIKDIsable - LinkedAnimInstanceProxy.DisableRHandCurve, 0.0f, 1.0f);
	LinkedAnimInstanceProxy.HandIKLeftAlpha = FMath::Clamp(HandIKDIsable - LinkedAnimInstanceProxy.DisableLHandCurve, 0.0f, 1.0f);
}

//Functions to bind to node
void UItemAnimLayerInstance::SetLeftHandPoseOverrideWeight(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	Proxy.LeftHandOverrideWeight = FMath::Clamp((bEnableLeftHandPoseOverride ? 1.0f : 0.0f) - Proxy.DisableLeftHandPoseOverride, 0.0f, 1.0f);
}
void UItemAnimLayerInstance::UpdateHipFireRaiseWeaponPose(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	USequenceEvaluatorLibrary::SetSequence(SequenceEval, Proxy.CachedbIsCrouching ? AimHipFirePoseCrouch : AimHipFirePose);
}
void UItemAnimLayerInstance::SetUpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEval, 0.0f);
}
void UItemAnimLayerInstance::UpdateFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEval, Proxy.CachedGroundDistance, JumpDistanceCurveName);
}
void UItemAnimLayerInstance::SetUpPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	Proxy.PivotStartingAcceleration = Proxy.CachedLocalAcceleration;
	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEval, GetDesiredPivotSequence(Proxy.CachedCardinalDirectionFromAcceleration));
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEval, 0.0f);
	Proxy.StrideWarpingPivotAlpha = 0.0f;
	Proxy.TimeAtPivotStop = 0.0f;
	//LastPivotTime = 0.2f;
	bChangeLastPivotTime = true;
}
void UItemAnimLayerInstance::UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEval);
	if (Proxy.CachedLastPivotTime > 0.0f)
	{
		UAnimSequence* NewDesiredSeq = GetDesiredPivotSequence(Proxy.CachedCardinalDirectionFromAcceleration);
		if (NewDesiredSeq != USequenceEvaluatorLibrary::GetSequence(SequenceEval))
		{
			USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEval, NewDesiredSeq, 0.2f);
			Proxy.PivotStartingAcceleration = Proxy.CachedLocalAcceleration;
		}
	}
	if (FVector::DotProduct(Proxy.CachedLocalVelocity, Proxy.CachedLocalAcceleration) < 0.0f)
	{
		float DistanceTarget = UAnimCharacterMovementLibrary::PredictGroundMovementPivotLocation(Proxy.CachedCurrentAcceleration, Proxy.CachedLastUpdateVelocity, Proxy.CachedGroundFriction).Size2D();
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEval, DistanceTarget, LocomotionDistanceCurveName);
		Proxy.TimeAtPivotStop = ExplicitTime;
		return;
	}
	else
	{
		Proxy.StrideWarpingPivotAlpha = UKismetMathLibrary::MapRangeClamped(ExplicitTime - Proxy.TimeAtPivotStop - StrideWarpingBlendInStartOffset, 0.0f, StrideWarpingBlendInDurationScaled, 0.0f, 1.0f);
		FVector2D Clamp = FVector2D(FMath::Lerp(0.2, PlayRateClampStartsPivots.X, Proxy.StrideWarpingPivotAlpha), PlayRateClampStartsPivots.Y);
		UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, SequenceEval, Proxy.CachedDisplacementSinceLastUpdate, LocomotionDistanceCurveName, Clamp);
		return;
	}
}
void UItemAnimLayerInstance::SetUpStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	if (Proxy.CachedbIsCrouching)
	{
		switch (Proxy.CachedLocalVelocityDirection)
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
		if (Proxy.CachedbGameplayTagIsADS)
		{
			switch (Proxy.CachedLocalVelocityDirection)
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
			switch (Proxy.CachedLocalVelocityDirection)
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
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	if (ShouldDistanceMatchStop())
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
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	if (Proxy.CachedbIsCrouching)
	{
		switch (Proxy.CachedLocalVelocityDirectionNoOffset)
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
		if (Proxy.CachedbGameplayTagIsADS)
		{
			switch (Proxy.CachedLocalVelocityDirectionNoOffset)
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
			switch (Proxy.CachedLocalVelocityDirectionNoOffset)
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
	UAnimDistanceMatchingLibrary::SetPlayrateToMatchSpeed(SequencePlayer, Proxy.CachedDisplacementSpeed, PlayRateClampCycle);
	Proxy.StrideWarpingCycleAlpha = FMath::FInterpTo(Proxy.StrideWarpingCycleAlpha, Proxy.CachedbIsRunningIntoWall ? 0.5f : 1.0f, Context.GetContext()->GetDeltaTime(), 10.0f);
	return;
}
void UItemAnimLayerInstance::SetUpStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	if (Proxy.CachedbIsCrouching)
	{
		switch (Proxy.CachedLocalVelocityDirection)
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
		if (Proxy.CachedbGameplayTagIsADS)
		{
			switch (Proxy.CachedLocalVelocityDirection)
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
			switch (Proxy.CachedLocalVelocityDirection)
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
	Proxy.StrideWarpingStartAlpha = 0.0f;
}
void UItemAnimLayerInstance::UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEval);
	Proxy.StrideWarpingStartAlpha = UKismetMathLibrary::MapRangeClamped(ExplicitTime - StrideWarpingBlendInStartOffset, 0.0f, StrideWarpingBlendInDurationScaled, 0.0f, 1.0f);
	FVector2D Clamp = FVector2D(FMath::Lerp(StrideWarpingBlendInDurationScaled, PlayRateClampStartsPivots.X, Proxy.StrideWarpingStartAlpha), PlayRateClampStartsPivots.Y);
	UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, SequenceEval, Proxy.CachedDisplacementSinceLastUpdate, LocomotionDistanceCurveName, Clamp);
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
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	if (Proxy.CachedbIsCrouching)
	{
		USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, IdleCrouch, 0.2f);
	}
	else
	{
		if (Proxy.CachedbGameplayTagIsADS)
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
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	USequencePlayerLibrary::SetSequence(SequencePlayer, Proxy.CachedbIsCrouching ? IdleCrouchEntry : IdleCrouchExit);
	return;
}
void UItemAnimLayerInstance::SetUpIdleBreakAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	USequencePlayerLibrary::SetSequence(SequencePlayer, IdleBreaks[Proxy.CurrentIdleBreakIndex]);
	Proxy.CurrentIdleBreakIndex++;
	if (Proxy.CurrentIdleBreakIndex >= IdleBreaks.Num())
		Proxy.CurrentIdleBreakIndex = 0;
}
void UItemAnimLayerInstance::SetUpTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	
	Proxy.TurnInPlaceAnimTime = 0.0f;
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEval, 0.0f);
}
void UItemAnimLayerInstance::UpdateTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEval = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEval, SelectTurnInPlaceAnimation(Proxy.TurnInPlaceRotationDirection), 0.2f);
	Proxy.TurnInPlaceAnimTime = Proxy.TurnInPlaceAnimTime + Context.GetContext()->GetDeltaTime();
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEval, Proxy.TurnInPlaceAnimTime);
}
void UItemAnimLayerInstance::SetUpTurnInPlaceRotationState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	Proxy.TurnInPlaceRotationDirection = FMath::Sign(Proxy.CachedRootYawOffset) * -1.0f;
}
void UItemAnimLayerInstance::UpdateTurnInPlaceRecoveryAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, SelectTurnInPlaceAnimation(Proxy.TurnInPlaceRecoveryDirection), 0.2f);
}
void UItemAnimLayerInstance::SetUpTurnInPlaceRecoveryState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	Proxy.TurnInPlaceRecoveryDirection = Proxy.TurnInPlaceRotationDirection;
}
void UItemAnimLayerInstance::LandRecoveryStart(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	Proxy.LandRecoveryAlpha = UKismetMathLibrary::MapRangeClamped(Proxy.TimeFalling, 0.0f, 0.4f, 0.1f, 1.0f) * (Proxy.CachedbIsCrouching ? 0.5f : 1.0f);
}

//Helper functions
bool UItemAnimLayerInstance::ShouldEnableFootPlacement()
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	if (MainAnimBPRef == nullptr)
		return false;
	else
		return (Proxy.DisableLegCurve <= 0 && Proxy.CachedbUseFootPlacement);
}
bool UItemAnimLayerInstance::ShouldDistanceMatchStop()
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	return Proxy.CachedbHasVelocity && !Proxy.CachedbHasAcceleration;
}
float UItemAnimLayerInstance::GetPredictedStopDistance()
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	FVector Prediction = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(Proxy.CachedLastUpdateVelocity, Proxy.CachedbUseSeperateBrakingFriction, Proxy.CachedBrakingFriction, Proxy.CachedGroundFriction, Proxy.CachedBrakingFrictionFactor, Proxy.CachedBrakingDecelerationWalking);
	return Prediction.Size2D();
}
void UItemAnimLayerInstance::ChooseIdleBreakDelayTime()
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	Proxy.IdleBreakDelayTime = (FMath::TruncToInt(FMath::Abs(Proxy.CachedWorldLocation.X + Proxy.CachedWorldLocation.Y)) % 10) + 6.0f;
}
bool UItemAnimLayerInstance::CanPlayIdleBreak()
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	return (IdleBreaks.Num() > 0) && !(Proxy.CachedbIsCrouching || Proxy.CachedbGameplayTagIsADS || Proxy.CachedbGameplayTagIsFiring || Proxy.CachedbIsAnyMontagePlaying || Proxy.CachedbHasVelocity || Proxy.CachedbIsJumping);
}
void UItemAnimLayerInstance::ResetIdleBreakTransitionLogic()
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	Proxy.TimeUntilNextIdleBreak = Proxy.IdleBreakDelayTime;
}
void UItemAnimLayerInstance::ProcessIdleBreakTransitionLogic(float DeltaTime)
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();

	if (CanPlayIdleBreak())
		Proxy.TimeUntilNextIdleBreak = Proxy.TimeUntilNextIdleBreak - DeltaTime;
	else
		ResetIdleBreakTransitionLogic();
}
UAnimSequence* UItemAnimLayerInstance::GetDesiredPivotSequence(ECardinalDirection InDirection)
{
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	if (Proxy.CachedbIsCrouching)
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
		if (Proxy.CachedbGameplayTagIsADS)
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
	FLinkedAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FLinkedAnimInstanceProxy>();
	if (Direction > 0.0f)
		return Proxy.CachedbIsCrouching ? CrouchTurnInPlaceRight : TurnInPlaceRight;
	else
		return Proxy.CachedbIsCrouching ? CrouchTurnInPlaceLeft : TurnInPlaceLeft;
}

//Getter functions
bool UItemAnimLayerInstance::GetCrouchStateChanged() const
{
	return LinkedAnimInstanceProxy.CachedbCrouchStateChange;
}
float UItemAnimLayerInstance::GetHipFireUpperBodyOverrideWeight() const
{
	return LinkedAnimInstanceProxy.HipFireUpperBodyOverrideWeight;
}
float UItemAnimLayerInstance::GetAimOffsetBlendWeight() const
{
	return LinkedAnimInstanceProxy.AimOffsetBlendWeight;
}
float UItemAnimLayerInstance::GetStrideWarpingStartAlpha() const
{
	return LinkedAnimInstanceProxy.StrideWarpingStartAlpha;
}
float UItemAnimLayerInstance::GetStrideWarpingCycleAlpha() const
{
	return LinkedAnimInstanceProxy.StrideWarpingCycleAlpha;
}
float UItemAnimLayerInstance::GetStrideWarpingPivotAlpha() const
{
	return LinkedAnimInstanceProxy.StrideWarpingPivotAlpha;
}
float UItemAnimLayerInstance::GetLeftHandOverrideWeight() const
{
	return LinkedAnimInstanceProxy.LeftHandOverrideWeight;
}
float UItemAnimLayerInstance::GetHandFKWeight() const
{
	return LinkedAnimInstanceProxy.HandFKWeight;
}
float UItemAnimLayerInstance::GetHandIKLeftAlpha() const
{
	return LinkedAnimInstanceProxy.HandIKLeftAlpha;
}
float UItemAnimLayerInstance::GetHandIKRightAlpha() const
{
	return LinkedAnimInstanceProxy.HandIKRightAlpha;
}
float UItemAnimLayerInstance::GetTimeUntilNextIdleBreak() const
{
	return LinkedAnimInstanceProxy.TimeUntilNextIdleBreak;
}
float UItemAnimLayerInstance::GetLandRecoveryAlpha() const
{
	return LinkedAnimInstanceProxy.LandRecoveryAlpha;
}
float UItemAnimLayerInstance::GetTurnInPlaceAnimTime() const
{
	return LinkedAnimInstanceProxy.TurnInPlaceAnimTime;
}
FVector UItemAnimLayerInstance::GetPivotStartingAcceleration() const
{
	return LinkedAnimInstanceProxy.PivotStartingAcceleration;
}
