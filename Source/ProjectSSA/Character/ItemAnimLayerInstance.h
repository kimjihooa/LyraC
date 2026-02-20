// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "SequenceEvaluatorLibrary.h"
#include "AnimCharacterMovementLibrary.h"
#include "AnimDistanceMatchingLibrary.h"
#include "CharacterAnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "ItemAnimLayerInstance.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FAnimStructCardinalDirections
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> Forward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> Backward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> Left;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> Right;
};

USTRUCT(BlueprintType)
struct FLinkedAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FLinkedAnimInstanceProxy() : FAnimInstanceProxy() {}
	FLinkedAnimInstanceProxy(UAnimInstance* Instance) : FAnimInstanceProxy(Instance) {}
	virtual void PreUpdate(UAnimInstance* Instance, float DeltaTime) override;

	//Data from outside
	bool CachedbIsCrouching = false;
	bool CachedbCrouchStateChange = false;
	bool CachedbGameplayTagIsADS = false;
	bool CachedbGameplayTagIsFiring = false;
	bool CachedbIsOnGround = false;
	bool CachedbIsJumping = false;
	bool CachedbIsFalling = false;
	bool CachedbHasVelocity = false;
	bool CachedbHasAcceleration = false;
	bool CachedbUseFootPlacement = false;
	bool CachedbUseSeperateBrakingFriction = false;
	bool CachedbIsRunningIntoWall = false;
	bool CachedbIsAnyMontagePlaying = false;
	float CachedDisplacementSpeed = 0.0f;
	float CachedRootYawOffset = 0.0f;
	float CachedGroundDistance = 0.0f;
	float CachedTimeSinceFiredWeapon = 0.0f;
	float CachedLastPivotTime = 0.0;
	float CachedGroundFriction = 8.0f;
	float CachedBrakingFriction = 0.0f;
	float CachedBrakingFrictionFactor = 2.0f;
	float CachedBrakingDecelerationWalking = 2048.0f;
	float CachedDisplacementSinceLastUpdate = 0.0f;
	//float LastPivotTime = 0.0f;
	FVector CachedWorldLocation = FVector::ZeroVector;
	FVector CachedLocalVelocity = FVector::ZeroVector;
	FVector CachedLocalAcceleration = FVector::ZeroVector;
	FVector CachedCurrentAcceleration = FVector::ZeroVector;
	FVector CachedLastUpdateVelocity = FVector::ZeroVector;
	ECardinalDirection CachedCardinalDirectionFromAcceleration = ECardinalDirection::Forward;
	ECardinalDirection CachedLocalVelocityDirection = ECardinalDirection::Forward;
	ECardinalDirection CachedLocalVelocityDirectionNoOffset = ECardinalDirection::Forward;

	//Curves
	FName CurveValue_ApplyHipFireCurve = FName("applyHipfireOverridePose");
	FName CurveValue_DisableRHandCurve = FName("DisableRHandIK");
	FName CurveValue_DisableLHandCurve = FName("DisableLHandIK");
	FName CurveValue_DisableLegCurve = FName("DisableLegIK");
	FName CurveValue_DisableLeftHandPoseOverride = FName("DisableLeftHandPoseOverride");
	float ApplyHipFireCurve = 0.0f;
	float DisableRHandCurve = 0.0f;
	float DisableLHandCurve = 0.0f;
	float DisableLegCurve = 0.0f;
	float DisableLeftHandPoseOverride = 0.0f;

	//Calculated Data
	UPROPERTY(BlueprintReadOnly, Category = "BlendWeightData")
	float HipFireUpperBodyOverrideWeight = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "BlendWeightData")
	float AimOffsetBlendWeight = 1.0f;
	UPROPERTY(BlueprintReadOnly, Category = "JumpFallData")
	float TimeFalling = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "SkelControlData", meta = (BlueprintThreadSafe))
	float HandIKRightAlpha = 1.0f;
	UPROPERTY(BlueprintReadOnly, Category = "SkelControlData", meta = (BlueprintThreadSafe))
	float HandIKLeftAlpha = 1.0f;
	UPROPERTY(BlueprintReadOnly)
	float LeftHandOverrideWeight = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float HandFKWeight = 1.0f;
	UPROPERTY(BlueprintReadOnly)
	FVector PivotStartingAcceleration = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly)
	float TimeAtPivotStop = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float StrideWarpingStartAlpha = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float StrideWarpingPivotAlpha = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float StrideWarpingCycleAlpha = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float IdleBreakDelayTime = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float TimeUntilNextIdleBreak = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	int CurrentIdleBreakIndex = 0;
	UPROPERTY(BlueprintReadOnly)
	float TurnInPlaceAnimTime = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float TurnInPlaceRotationDirection = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float TurnInPlaceRecoveryDirection = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float LandRecoveryAlpha = 0.0f;
};

UCLASS()
class PROJECTSSA_API UItemAnimLayerInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	//Proxy
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) override;
	FLinkedAnimInstanceProxy LinkedAnimInstanceProxy;

public:
	UItemAnimLayerInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;

	//Data update function
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	UCharacterAnimInstance* GetMainAnimBP();
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateBlendWeightsData(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateJumpFallData(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateSkelControlData();

	//Data
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TObjectPtr<UCharacterAnimInstance> MainAnimBPRef;
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	//Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bRaiseWeaponAfterFiringWhenCrouched = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float RaiseWeaponAfterFiringWeapon = 0.5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float StrideWarpingBlendInStartOffset = 0.15;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float StrideWarpingBlendInDurationScaled = 0.2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FVector2D PlayRateClampStartsPivots = FVector2D(0.6f, 5.0f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FVector2D PlayRateClampCycle = FVector2D(0.8f, 1.2f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bDisableHandIK = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bEnableLeftHandPoseOverride = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FName JumpDistanceCurveName = FName("GroundDistance");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FName LocomotionDistanceCurveName = FName("Distance");

	//Functions to bind to node
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetLeftHandPoseOverrideWeight(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateHipFireRaiseWeaponPose(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateCycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateIdleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpIdleTransition(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpIdleBreakAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpTurnInPlaceRotationState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateTurnInPlaceRecoveryAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpTurnInPlaceRecoveryState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void LandRecoveryStart(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	//Helper functions
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	bool ShouldEnableFootPlacement();
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	bool ShouldDistanceMatchStop();
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetPredictedStopDistance();
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void ChooseIdleBreakDelayTime();
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	bool CanPlayIdleBreak();
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void ResetIdleBreakTransitionLogic();
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void ProcessIdleBreakTransitionLogic(float DeltaTime);
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	UAnimSequence* GetDesiredPivotSequence(ECardinalDirection InDirection);
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	UAnimSequence* SelectTurnInPlaceAnimation(float Direction);
	bool bChangeLastPivotTime = false;

	//Getter functions
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	bool GetCrouchStateChanged() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetHipFireUpperBodyOverrideWeight() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetAimOffsetBlendWeight() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetStrideWarpingStartAlpha() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetStrideWarpingCycleAlpha() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetStrideWarpingPivotAlpha() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetLeftHandOverrideWeight() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetHandFKWeight() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetHandIKLeftAlpha() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetHandIKRightAlpha() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetTimeUntilNextIdleBreak() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetLandRecoveryAlpha() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	float GetTurnInPlaceAnimTime() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	FVector GetPivotStartingAcceleration() const;


	//Animation assets
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Idle")
	TObjectPtr<UAnimSequence> LeftHandPose;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Jump")
	TObjectPtr<UAnimSequence> JumpStartLoop;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Jump")
	TObjectPtr<UAnimSequence> JumpFallLoop;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Jump")
	TObjectPtr<UAnimSequence> JumpFallLand;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Jump")
	TObjectPtr<UAnimSequence> JumpApex;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Jump")
	TObjectPtr<UAnimSequence> JumpStart;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Jump")
	TObjectPtr<UAnimSequence> JumpRecoveryAdditive;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Idle")
	TObjectPtr<UAnimSequence> IdleHipFire;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Idle")
	TObjectPtr<UAnimSequence> IdleADS;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Idle")
	TObjectPtr<UAnimSequence> IdleCrouch;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Idle")
	TObjectPtr<UAnimSequence> IdleCrouchEntry;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Idle")
	TObjectPtr<UAnimSequence> IdleCrouchExit;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Idle")
	TArray<UAnimSequence*> IdleBreaks;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Aim")
	TObjectPtr<UAnimSequence> AimHipFirePose;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Aim")
	TObjectPtr<UAnimSequence> AimHipFirePoseCrouch;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Aim")
	TObjectPtr<UAimOffsetBlendSpace> RelaxedAimOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Aim")
	TObjectPtr<UAimOffsetBlendSpace> IdleAimOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Pivots")
	FAnimStructCardinalDirections JogPivotCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Pivots")
	FAnimStructCardinalDirections ADSPivotCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Pivots")
	FAnimStructCardinalDirections CrouchPivotCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Stops")
	FAnimStructCardinalDirections JogStopCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Stops")
	FAnimStructCardinalDirections ADSStopCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Stops")
	FAnimStructCardinalDirections CrouchStopCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Walk")
	FAnimStructCardinalDirections JogCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Walk")
	FAnimStructCardinalDirections ADSCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Walk")
	FAnimStructCardinalDirections CrouchCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Starts")
	FAnimStructCardinalDirections JogStartCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Starts")
	FAnimStructCardinalDirections ADSStartCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-Starts")
	FAnimStructCardinalDirections CrouchStartCardinal;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-TurnInPlace")
	TObjectPtr<UAnimSequence> TurnInPlaceLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-TurnInPlace")
	TObjectPtr<UAnimSequence> CrouchTurnInPlaceLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-TurnInPlace")
	TObjectPtr<UAnimSequence> TurnInPlaceRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimSet-TurnInPlace")
	TObjectPtr<UAnimSequence> CrouchTurnInPlaceRight;
};