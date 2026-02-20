// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"

void FMainAnimInstanceProxy::PreUpdate(UAnimInstance* Instance, float DeltaTime)
{
	Super::PreUpdate(Instance, DeltaTime);

	TurnYawWeightCurve = Instance->GetCurveValue(CurveName_TurnYawWeight);
	RemainingTurnYawCurve = Instance->GetCurveValue(CurveName_RemainingTurnYaw);
	DisableLegIKCurve = Instance->GetCurveValue(CurveName_DisableLegIK);
}

FAnimInstanceProxy* UCharacterAnimInstance::CreateAnimInstanceProxy()
{
	return &MainAnimInstanceProxy;
}
void UCharacterAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy)
{
}

UCharacterAnimInstance::UCharacterAnimInstance()
{
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
//Get data from outside
void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!IsValid(ActorRef) || !IsValid(PawnRef) || !IsValid(MovementRef))
		return;

	MainAnimInstanceProxy.CachedLocation = ActorRef->GetActorLocation();
	MainAnimInstanceProxy.CachedRotation = ActorRef->GetActorRotation();
	MainAnimInstanceProxy.CachedVelocity = PawnRef->GetVelocity();
	MainAnimInstanceProxy.CachedAcceleration = MovementRef->GetCurrentAcceleration();
	MainAnimInstanceProxy.CachedIsMovingOnGround = MovementRef->IsMovingOnGround();
	MainAnimInstanceProxy.CachedIsCroching = MovementRef->IsCrouching();
	MainAnimInstanceProxy.CachedMovementMode = MovementRef->MovementMode;
	MainAnimInstanceProxy.CachedIsAnyMontagePlaying = IsAnyMontagePlaying();
	MainAnimInstanceProxy.CachedAimPitch = PawnRef->GetBaseAimRotation().Pitch;
	MainAnimInstanceProxy.CachedGravityZ = PawnRef->GetMovementComponent()->GetGravityZ();
	MainAnimInstanceProxy.GroundDistance = MainCharacterRef->GetGroundDistance();

	//Gameplay tags
	MainAnimInstanceProxy.bGameplayTagIsADS = SafebGameplayTagIsADS;
	MainAnimInstanceProxy.bGameplayTagIsFiring = SafebGameplayTagIsFiring;
	MainAnimInstanceProxy.bGameplayTagIsDashing = SafebGameplayTagIsDashing;
	MainAnimInstanceProxy.bGameplayTagIsMelee = SafebGameplayTagIsMelee;
}
//Calculate data
void UCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
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

//Data update functions
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
	MainAnimInstanceProxy.DisplacementSinceLastUpdate = UKismetMathLibrary::VSizeXY(MainAnimInstanceProxy.CachedLocation - MainAnimInstanceProxy.WorldLocation);
	MainAnimInstanceProxy.WorldLocation = MainAnimInstanceProxy.CachedLocation;
	MainAnimInstanceProxy.DisplacementSpeed = UKismetMathLibrary::SafeDivide(MainAnimInstanceProxy.DisplacementSinceLastUpdate, DeltaTime);
	if (bIsFirstUpdate)
	{
		MainAnimInstanceProxy.DisplacementSinceLastUpdate = 0.0f;
		MainAnimInstanceProxy.DisplacementSpeed = 0.0f;
	}
}
void UCharacterAnimInstance::UpdateRotationData(float DeltaTime)
{
	MainAnimInstanceProxy.YawDeltaSinceLastUpdate = UKismetMathLibrary::NormalizeAxis(MainAnimInstanceProxy.CachedRotation.Yaw - MainAnimInstanceProxy.WorldRotation.Yaw);
	MainAnimInstanceProxy.YawDeltaSpeed = UKismetMathLibrary::SafeDivide(MainAnimInstanceProxy.YawDeltaSinceLastUpdate, DeltaTime);
	MainAnimInstanceProxy.WorldRotation = MainAnimInstanceProxy.CachedRotation;
	MainAnimInstanceProxy.AdditiveLeanAngle = (MainAnimInstanceProxy.bIsCrouching || MainAnimInstanceProxy.bGameplayTagIsADS ? 0.025 : 0.0375) * MainAnimInstanceProxy.YawDeltaSpeed;
	if (bIsFirstUpdate)
	{
		MainAnimInstanceProxy.YawDeltaSinceLastUpdate = 0.0f;
		MainAnimInstanceProxy.AdditiveLeanAngle = 0.0f;
	}
}
void UCharacterAnimInstance::UpdateVelocityData()
{
	MainAnimInstanceProxy.bWasMovingLastUpdate = !MainAnimInstanceProxy.LocalVelocity2D.IsZero();
	MainAnimInstanceProxy.WorldVelocity = MainAnimInstanceProxy.CachedVelocity;
	MainAnimInstanceProxy.WorldVelocity2D = FVector(MainAnimInstanceProxy.WorldVelocity.X, MainAnimInstanceProxy.WorldVelocity.Y, 0.0f);
	MainAnimInstanceProxy.LocalVelocity2D = MainAnimInstanceProxy.WorldRotation.UnrotateVector(MainAnimInstanceProxy.WorldVelocity2D);
	MainAnimInstanceProxy.LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(MainAnimInstanceProxy.WorldVelocity2D, MainAnimInstanceProxy.WorldRotation);
	MainAnimInstanceProxy.LocalVelocityDirectionAngleWithOffset = MainAnimInstanceProxy.LocalVelocityDirectionAngle - MainAnimInstanceProxy.RootYawOffset;
	MainAnimInstanceProxy.LocalVelocityDirection = SelectCarialDirectionFromAngle(MainAnimInstanceProxy.LocalVelocityDirectionAngleWithOffset, MainAnimInstanceProxy.CardinalDirectionDeadZone, MainAnimInstanceProxy.LocalVelocityDirection, MainAnimInstanceProxy.bWasMovingLastUpdate);
	MainAnimInstanceProxy.LocalVelocityDirectionNoOffset = SelectCarialDirectionFromAngle(MainAnimInstanceProxy.LocalVelocityDirectionAngle, MainAnimInstanceProxy.CardinalDirectionDeadZone, MainAnimInstanceProxy.LocalVelocityDirectionNoOffset, MainAnimInstanceProxy.bWasMovingLastUpdate);
	MainAnimInstanceProxy.bHasVelocity = !UKismetMathLibrary::NearlyEqual_FloatFloat(MainAnimInstanceProxy.LocalVelocity2D.SizeSquared2D(), 0.0f);
}
void UCharacterAnimInstance::UpdateAccelerationData()
{
	MainAnimInstanceProxy.WorldAcceleration2D = FVector(MainAnimInstanceProxy.CachedAcceleration.X, MainAnimInstanceProxy.CachedAcceleration.Y, 0.0f);
	MainAnimInstanceProxy.LocalAcceleration2D = MainAnimInstanceProxy.WorldRotation.UnrotateVector(MainAnimInstanceProxy.WorldAcceleration2D);
	MainAnimInstanceProxy.bHasAcceleration = !UKismetMathLibrary::NearlyEqual_FloatFloat(MainAnimInstanceProxy.LocalAcceleration2D.SizeSquared2D(), 0.0f);
	MainAnimInstanceProxy.PivotDirection2D = FMath::Lerp(MainAnimInstanceProxy.PivotDirection2D, MainAnimInstanceProxy.WorldAcceleration2D.GetSafeNormal(), 0.5f).GetSafeNormal();
	MainAnimInstanceProxy.CardinalDirectionFromAcceleration = GetOppositeCardinalDirection(SelectCarialDirectionFromAngle(UKismetAnimationLibrary::CalculateDirection(MainAnimInstanceProxy.PivotDirection2D, MainAnimInstanceProxy.WorldRotation), MainAnimInstanceProxy.CardinalDirectionDeadZone, ECardinalDirection::Forward, false));
}
void UCharacterAnimInstance::UpdateWallDetectionHeuristic()
{
	bool AccLen = MainAnimInstanceProxy.LocalAcceleration2D.Size2D() > 0.1f;
	bool VelLen = MainAnimInstanceProxy.LocalVelocity2D.Size2D() < 200.0f;
	bool Dot = UKismetMathLibrary::InRange_FloatFloat(FVector::DotProduct(MainAnimInstanceProxy.LocalAcceleration2D.GetSafeNormal(), MainAnimInstanceProxy.LocalVelocity2D.GetSafeNormal()), -0.6f, 0.6f, true, true);
	MainAnimInstanceProxy.bIsRunningIntoWall = AccLen && VelLen && Dot;
}
void UCharacterAnimInstance::UpdateCharacterStateData(float DeltaTime)
{
	MainAnimInstanceProxy.bIsOnGround = MainAnimInstanceProxy.CachedIsMovingOnGround;
	MainAnimInstanceProxy.bWasCrouchingLastUpdate = MainAnimInstanceProxy.bIsCrouching;
	MainAnimInstanceProxy.bIsCrouching = MainAnimInstanceProxy.CachedIsCroching;
	MainAnimInstanceProxy.bCrouchStateChange = MainAnimInstanceProxy.bWasCrouchingLastUpdate != MainAnimInstanceProxy.bIsCrouching;
	MainAnimInstanceProxy.bADSStateChanged = MainAnimInstanceProxy.bGameplayTagIsADS != MainAnimInstanceProxy.bWasADSLastUpdate;
	MainAnimInstanceProxy.bWasADSLastUpdate = MainAnimInstanceProxy.bGameplayTagIsADS;
	MainAnimInstanceProxy.TimeSinceFiredWeapon = MainAnimInstanceProxy.bGameplayTagIsFiring ? 0.0f : MainAnimInstanceProxy.TimeSinceFiredWeapon + DeltaTime;
	MainAnimInstanceProxy.bIsJumping = false;
	MainAnimInstanceProxy.bIsFalling = false;
	if (MainAnimInstanceProxy.CachedMovementMode == EMovementMode::MOVE_Falling)
	{
		if (MainAnimInstanceProxy.WorldVelocity.Z > 0)
			MainAnimInstanceProxy.bIsJumping = true;
		else
			MainAnimInstanceProxy.bIsFalling = true;
	}
}
void UCharacterAnimInstance::UpdateBlendWeightData(float DeltaTime)
{
	MainAnimInstanceProxy.UpperbodyDynamicAdditiveWeight = (MainAnimInstanceProxy.CachedIsAnyMontagePlaying && MainAnimInstanceProxy.bIsOnGround) ? 1.0f : (FMath::FInterpTo(MainAnimInstanceProxy.UpperbodyDynamicAdditiveWeight, 0.0f, DeltaTime, 6.0f));
}
void UCharacterAnimInstance::UpdateRootYawOffset(float DeltaTime)
{
	if (MainAnimInstanceProxy.RootYawOffsetMode == ERootYawOffsetMode::Accumulate)
		SetRootYawOffset(MainAnimInstanceProxy.RootYawOffset - MainAnimInstanceProxy.YawDeltaSinceLastUpdate);
	if (MainAnimInstanceProxy.RootYawOffsetMode == ERootYawOffsetMode::BlendOut)
		SetRootYawOffset(UKismetMathLibrary::FloatSpringInterp(MainAnimInstanceProxy.RootYawOffset, 0.0f, MainAnimInstanceProxy.RootYawOffsetSpringState, 80.0f, 1.0f, DeltaTime, 1.0f, 0.5f));
	MainAnimInstanceProxy.RootYawOffsetMode = ERootYawOffsetMode::BlendOut;
}
void UCharacterAnimInstance::UpdateAimingData()
{
	MainAnimInstanceProxy.AimPitch = UKismetMathLibrary::NormalizeAxis(MainAnimInstanceProxy.CachedAimPitch);
}
void UCharacterAnimInstance::UpdateJumpFallData()
{
	if (MainAnimInstanceProxy.bIsJumping)
		MainAnimInstanceProxy.TimeToJumpApex = UKismetMathLibrary::SafeDivide((0.0f - MainAnimInstanceProxy.WorldVelocity.Z), MainAnimInstanceProxy.CachedGravityZ);
	else
		MainAnimInstanceProxy.TimeToJumpApex = 0.0f;
}

//Gameplay tag
void UCharacterAnimInstance::OnFiringTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SafebGameplayTagIsFiring = (NewCount > 0);
}

//Node Binding functions
void UCharacterAnimInstance::UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference State;
	bool bSucceeded = true;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResultPure(Node, State, bSucceeded);
	FMainAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FMainAnimInstanceProxy>();

	if (UAnimationStateMachineLibrary::IsStateBlendingOut(Context, State))
		Proxy.TurnYawCurveValue = 0.0f;
	else
	{
		Proxy.RootYawOffsetMode = ERootYawOffsetMode::Accumulate;
		ProcessTurnYawCurve();
	}
}
void UCharacterAnimInstance::SetUpStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FMainAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FMainAnimInstanceProxy>();
	Proxy.StartDirection = Proxy.LocalVelocityDirection;
}
void UCharacterAnimInstance::UpdateStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference State;
	bool bSucceeded = true;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResultPure(Node, State, bSucceeded);
	FMainAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FMainAnimInstanceProxy>();

	if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, State))
		Proxy.RootYawOffsetMode = ERootYawOffsetMode::Hold;
}
void UCharacterAnimInstance::UpdateStopState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference State;
	bool bSucceeded = true;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResultPure(Node, State, bSucceeded);
	FMainAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FMainAnimInstanceProxy>();

	if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, State))
		Proxy.RootYawOffsetMode = ERootYawOffsetMode::Accumulate;
}
void UCharacterAnimInstance::SetUpPivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FMainAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FMainAnimInstanceProxy>();
	Proxy.PivotInitialDirection = Proxy.LocalVelocityDirection;
}
void UCharacterAnimInstance::UpdatePivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FMainAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FMainAnimInstanceProxy>();

	if (Proxy.LastPivotTime > 0.0f)
		Proxy.LastPivotTime -= UAnimExecutionContextLibrary::GetDeltaTime(Context);
}
void UCharacterAnimInstance::UpdateLocomotionStateMachine(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FLinkedAnimGraphReference LinkedGraph = ULinkedAnimGraphLibrary::ConvertToLinkedAnimGraph(Node, Result);
	FMainAnimInstanceProxy& Proxy = GetProxyOnAnyThread<FMainAnimInstanceProxy>();

	UAnimInstance* CurrentInstance = ULinkedAnimGraphLibrary::GetLinkedAnimInstance(LinkedGraph);
	if (!bIsFirstUpdate)
		Proxy.bLinkedLayerChanged = CurrentInstance != Proxy.LastLinkedLayer;
	Proxy.LastLinkedLayer = CurrentInstance;
}

//Helper functions
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
	bool PivotFB = (MainAnimInstanceProxy.PivotInitialDirection == ECardinalDirection::Forward) || (MainAnimInstanceProxy.PivotInitialDirection == ECardinalDirection::Backward);
	bool VelocityFB = (MainAnimInstanceProxy.LocalVelocityDirection == ECardinalDirection::Forward) || (MainAnimInstanceProxy.LocalVelocityDirection == ECardinalDirection::Backward);
	bool PivotLR = (MainAnimInstanceProxy.PivotInitialDirection == ECardinalDirection::Left) || (MainAnimInstanceProxy.PivotInitialDirection == ECardinalDirection::Right);
	bool VelocityLR = (MainAnimInstanceProxy.LocalVelocityDirection == ECardinalDirection::Left) || (MainAnimInstanceProxy.LocalVelocityDirection == ECardinalDirection::Right);
	return (PivotFB && !VelocityFB) || (PivotLR && !VelocityLR);
}
void UCharacterAnimInstance::SetRootYawOffset(float InRootYawOffset)
{
	if (!bEnableRootYawOffset)
	{
		MainAnimInstanceProxy.RootYawOffset = 0.0f;
		MainAnimInstanceProxy.AimYaw = 0.0f;
	}
	else
	{
		FVector2D Clamp = MainAnimInstanceProxy.bIsCrouching ? MainAnimInstanceProxy.RootYawOffsetAngleClampCrouched : MainAnimInstanceProxy.RootYawOffsetAngleClamp;
		float Normalized = UKismetMathLibrary::NormalizeAxis(InRootYawOffset);
		float Clamped = UKismetMathLibrary::ClampAngle(Normalized, Clamp.X, Clamp.Y);
		MainAnimInstanceProxy.RootYawOffset = Clamp.X == Clamp.Y ? Normalized : Clamped;
		MainAnimInstanceProxy.AimYaw = MainAnimInstanceProxy.RootYawOffset * -1.0f;
	}
}
void UCharacterAnimInstance::ProcessTurnYawCurve()
{
	float PreviousTurnYawCurveValue = MainAnimInstanceProxy.TurnYawCurveValue;

	float TurnYawWeight = MainAnimInstanceProxy.TurnYawWeightCurve;
	if (UKismetMathLibrary::NearlyEqual_FloatFloat(TurnYawWeight, 0.0f))
	{
		MainAnimInstanceProxy.TurnYawCurveValue = 0.0f;
		PreviousTurnYawCurveValue = 0.0f;
	}
	else
	{
		MainAnimInstanceProxy.TurnYawCurveValue = MainAnimInstanceProxy.RemainingTurnYawCurve / TurnYawWeight;
		if (PreviousTurnYawCurveValue != 0.0f)
			SetRootYawOffset(MainAnimInstanceProxy.RootYawOffset - (MainAnimInstanceProxy.TurnYawCurveValue - PreviousTurnYawCurveValue));
	}
}
bool UCharacterAnimInstance::ShouldEnableControlRig()
{
	return (MainAnimInstanceProxy.DisableLegIKCurve <= 0.0f) && !bUseFootPlacement;
}

//Getter functions
bool UCharacterAnimInstance::GetIsCrouching() const
{
	return MainAnimInstanceProxy.bIsCrouching;
}
bool UCharacterAnimInstance::GetCrouchStateChanged() const
{
	return MainAnimInstanceProxy.bCrouchStateChange;
}
bool UCharacterAnimInstance::GetADSStateChanged() const
{
	return MainAnimInstanceProxy.bADSStateChanged;
}
bool UCharacterAnimInstance::GetIsOnGround() const
{
	return MainAnimInstanceProxy.bIsOnGround;
}
bool UCharacterAnimInstance::GetIsMovingOnGround() const
{
	return MainAnimInstanceProxy.CachedIsMovingOnGround;
}
bool UCharacterAnimInstance::GetIsJumping() const
{
	return MainAnimInstanceProxy.bIsJumping;
}
bool UCharacterAnimInstance::GetIsFalling() const
{
	return MainAnimInstanceProxy.bIsFalling;
}
bool UCharacterAnimInstance::GetHasVelocity() const
{
	return MainAnimInstanceProxy.bHasVelocity;
}
bool UCharacterAnimInstance::GetHasAcceleration() const
{
	return MainAnimInstanceProxy.bHasAcceleration;
}
bool UCharacterAnimInstance::GetIsRunningIntoWall() const
{
	return MainAnimInstanceProxy.bIsRunningIntoWall;
}
bool UCharacterAnimInstance::GetGameplayTagIsFiring() const
{
	return MainAnimInstanceProxy.bGameplayTagIsFiring;
}
bool UCharacterAnimInstance::GetGameplayTagIsMelee() const
{
	return MainAnimInstanceProxy.bGameplayTagIsMelee;
}
bool UCharacterAnimInstance::GetLinkedLayerChanged() const
{
	return MainAnimInstanceProxy.bLinkedLayerChanged;
}
float UCharacterAnimInstance::GetDisplacementSpeed() const
{
	return MainAnimInstanceProxy.DisplacementSpeed;
}
float UCharacterAnimInstance::GetRootYawOffset() const
{
	return MainAnimInstanceProxy.RootYawOffset;
}
float UCharacterAnimInstance::GetGroundDistance() const
{
	return MainAnimInstanceProxy.GroundDistance;
}
float UCharacterAnimInstance::GetTimeSinceFiredWeapon() const
{
	return MainAnimInstanceProxy.TimeSinceFiredWeapon;
}
float UCharacterAnimInstance::GetLastPivotTime() const
{
	return MainAnimInstanceProxy.LastPivotTime;
}
float UCharacterAnimInstance::GetDisplacementSinceLastUpdate() const
{
	return MainAnimInstanceProxy.DisplacementSinceLastUpdate;
}
float UCharacterAnimInstance::GetTimeToJumpApex() const
{
	return MainAnimInstanceProxy.TimeToJumpApex;
}
float UCharacterAnimInstance::GetAimYaw() const
{
	return MainAnimInstanceProxy.AimYaw;
}
float UCharacterAnimInstance::GetAimPitch() const
{
	return MainAnimInstanceProxy.AimPitch;
}
float UCharacterAnimInstance::GetUpperBodyDynamicAdditiveWeight() const
{
	return MainAnimInstanceProxy.UpperbodyDynamicAdditiveWeight;
}
float UCharacterAnimInstance::GetAdditiveLeanAngle() const
{
	return MainAnimInstanceProxy.AdditiveLeanAngle;
}
float UCharacterAnimInstance::GetLocalVelocityDirectionAngle() const
{
	return MainAnimInstanceProxy.LocalVelocityDirectionAngle;
}
float UCharacterAnimInstance::GetLocalVelocityDirectionAngleWithOffset() const
{
	return MainAnimInstanceProxy.LocalVelocityDirectionAngleWithOffset;
}
FVector UCharacterAnimInstance::GetWorldLocation() const
{
	return MainAnimInstanceProxy.WorldLocation;
}
FVector UCharacterAnimInstance::GetLocalVelocty2D() const
{
	return MainAnimInstanceProxy.LocalVelocity2D;
}
FVector UCharacterAnimInstance::GetLocalAcceleration2D() const
{
	return MainAnimInstanceProxy.LocalAcceleration2D;
}
ECardinalDirection UCharacterAnimInstance::GetLocalVelocityDirection() const
{
	return MainAnimInstanceProxy.LocalVelocityDirection;
}
ECardinalDirection UCharacterAnimInstance::GetLocalVelocityDirectionNoOffset() const
{
	return MainAnimInstanceProxy.LocalVelocityDirectionNoOffset;
}
ECardinalDirection UCharacterAnimInstance::GetCardinalDirectionFromAcceleration() const
{
	return MainAnimInstanceProxy.CardinalDirectionFromAcceleration;
}
ECardinalDirection UCharacterAnimInstance::GetStartDirection() const
{
	return MainAnimInstanceProxy.StartDirection;
}
void UCharacterAnimInstance::SetLastPivotTime()
{
	MainAnimInstanceProxy.LastPivotTime = 0.2f;
}
