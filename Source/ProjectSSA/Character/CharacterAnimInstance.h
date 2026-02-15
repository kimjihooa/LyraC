// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "AnimationStateMachineLibrary.h"
#include "AnimExecutionContextLibrary.h"
#include "LinkedAnimGraphLibrary.h"
#include "Animation/AnimInstanceProxy.h"
#include "CharacterAnimInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ECardinalDirection : uint8
{
	Forward,
	Backward,
	Left,
	Right
};
UENUM(BlueprintType)
enum class ERootYawOffsetMode : uint8
{
	BlendOut,
	Hold,
	Accumulate
};

class UItemAnimLayerInstance;
struct FItemAnimLayerInstanceProxy;

USTRUCT(BlueprintType)
struct FMainAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FMainAnimInstanceProxy() : FAnimInstanceProxy() {}
	FMainAnimInstanceProxy(UAnimInstance* Instance) : FAnimInstanceProxy(Instance) {}

	virtual void PreUpdate(UAnimInstance* Instance, float DeltaSeconds) override;
	virtual void Update(float DeltaSeconds) override;

	FVector CachedLocation = FVector::ZeroVector;
	FRotator CachedRotation = FRotator::ZeroRotator;
	FVector CachedVelocity = FVector::ZeroVector;
	FVector CachedAcceleration = FVector::ZeroVector;
	bool CachedIsMovingOnGround = false;
	bool CachedIsCroching = false;
	EMovementMode CachedMovementMode = EMovementMode::MOVE_Walking;
	bool CachedIsAnyMontagePlaying = false;
	float CachedAimPitch = 0.0f;
	float CachedGravityZ = 0.0f;
};

UCLASS()
class PROJECTSSA_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	FMainAnimInstanceProxy MainAnimInstanceProxy;
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) override;

public:
	UCharacterAnimInstance();
	friend class UItemAnimLayerInstance;
	friend struct FItemAnimLayerInstanceProxy;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HelperFunctions", meta = (BlueprintThreadSafe))
	bool IsMovingPerpendicularToInitialPivot() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HelperFunctions")
	ECardinalDirection GetOppositeCardinalDirection(const ECardinalDirection CurrentDir);
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	bool ShouldEnableControlRig();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	UCharacterMovementComponent* GetMovementComponent();

	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateLocationData(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateRotationData(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateVelocityData();
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateAccelerationData();
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateWallDetectionHeuristic();
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateCharacterStateData(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateBlendWeightData(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateRootYawOffset(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void SetRootYawOffset(float InRootYawOffset);
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void ProcessTurnYawCurve();
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateAimingData();
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateJumpFallData();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HelperFunctions")
	ECardinalDirection SelectCarialDirectionFromAngle(const float Angle, const float DeadZone, const ECardinalDirection CurrentDirection, const bool bUseCurrentDirection);

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TObjectPtr<AActor> ActorRef;
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TObjectPtr<APawn> PawnRef;
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TObjectPtr<AMainCharacter> MainCharacterRef;
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TObjectPtr<UCharacterMovementComponent> MovementRef;
	UPROPERTY(BlueprintReadOnly, Category = "LocationData")
	float DisplacementSinceLastUpdate = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LocationData")
	FVector WorldLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "LocationData")
	float DisplacementSpeed = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "RotationData")
	float YawDeltaSinceLastUpdate = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "RotationData")
	float YawDeltaSpeed = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "RotationData")
	FRotator WorldRotation = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadOnly, Category = "RotationData")
	float AdditiveLeanAngle = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	bool bWasMovingLastUpdate = false;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	FVector WorldVelocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	FVector WorldVelocity2D = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	FVector LocalVelocity2D = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	float LocalVelocityDirectionAngle = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	float LocalVelocityDirectionAngleWithOffset = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	ECardinalDirection LocalVelocityDirection = ECardinalDirection::Forward;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	ECardinalDirection LocalVelocityDirectionNoOffset = ECardinalDirection::Forward;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	bool bHasVelocity = false;
	UPROPERTY(BlueprintReadOnly, Category = "VelocityData")
	float CardinalDirectionDeadZone = 10.0f;
	UPROPERTY(BlueprintReadOnly, Category = "AccelerationData")
	FVector WorldAcceleration2D = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AccelerationData")
	FVector LocalAcceleration2D = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AccelerationData")
	bool bHasAcceleration = false;
	UPROPERTY(BlueprintReadOnly, Category = "AccelerationData")
	FVector PivotDirection2D = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AccelerationData")
	ECardinalDirection CardinalDirectionFromAcceleration = ECardinalDirection::Backward;
	UPROPERTY(BlueprintReadOnly, Category = "WallDetection")
	bool bIsRunningIntoWall = false;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	bool bIsOnGround = false;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	bool bWasCrouchingLastUpdate = false;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	bool bIsCrouching = false;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	bool bCrouchStateChange = false;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	bool bADSStateChanged = false;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	bool bWasADSLastUpdate = false;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	float TimeSinceFiredWeapon = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	bool bIsJumping = false;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	bool bIsFalling = false;
	UPROPERTY(BlueprintReadOnly, Category = "CharacterStateData")
	float GroundDistance = -1.0f;
	UPROPERTY(BlueprintReadOnly, Category = "BlendWeightData")
	float UpperbodyDynamicAdditiveWeight = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "RootYawOffset")
	ERootYawOffsetMode RootYawOffsetMode = ERootYawOffsetMode::BlendOut;
	UPROPERTY(BlueprintReadOnly, Category = "RootYawOffset")
	float RootYawOffset = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "RootYawOffset")
	FFloatSpringState RootYawOffsetSpringState;
	UPROPERTY(BlueprintReadOnly, Category = "RootYawOffset")
	FVector2D RootYawOffsetAngleClamp = FVector2D(-120.0f, 100.0f);
	UPROPERTY(BlueprintReadOnly, Category = "RootYawOffset")
	FVector2D RootYawOffsetAngleClampCrouched = FVector2D(-90.0f, 80.0f);
	UPROPERTY(BlueprintReadOnly, Category = "RootYawOffset")
	float TurnYawCurveValue = 0;
	UPROPERTY(BlueprintReadOnly, Category = "AimingData")
	float AimYaw = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "AimingData")
	float AimPitch = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "JumpFallData")
	float TimeToJumpApex = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LocomotionSMData")
	ECardinalDirection StartDirection = ECardinalDirection::Forward;
	UPROPERTY(BlueprintReadOnly, Category = "LocomotionSMData")
	ECardinalDirection PivotInitialDirection = ECardinalDirection::Forward;
	UPROPERTY(BlueprintReadWrite, Category = "LocomotionSMData")
	float LastPivotTime = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LinkedLayerData")
	UAnimInstance* LastLinkedLayer = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "LinkedLayerData")
	bool bLinkedLayerChanged = false;

	UPROPERTY(BlueprintReadOnly, Category = "GameplayTags")
	bool bGameplayTagIsADS = false;
	UPROPERTY(BlueprintReadOnly, Category = "GameplayTags")
	bool bGameplayTagIsFiring = false;
	UPROPERTY(BlueprintReadOnly, Category = "GameplayTags")
	bool bGameplayTagIsDashing = false;
	UPROPERTY(BlueprintReadOnly, Category = "GameplayTags")
	bool bGameplayTagIsMelee = false;

	FThreadSafeBool SafebGameplayTagIsADS = false;
	FThreadSafeBool SafebGameplayTagIsFiring = false;
	FThreadSafeBool SafebGameplayTagIsDashing = false;
	FThreadSafeBool SafebGameplayTagIsMelee = false;

	void OnFiringTagChanged(const FGameplayTag Tag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly)
	bool bIsFirstUpdate = true;
	UPROPERTY(BlueprintReadOnly)
	bool bEnableRootYawOffset = true;
	UPROPERTY(BlueprintReadOnly, meta = (BlueprintThreadSafe))
	bool bEnableControlRig = false;
	UPROPERTY(BlueprintReadOnly)
	bool bUseFootPlacement = false;


	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateStopState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetUpPivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdatePivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateLocomotionStateMachine(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	FName CurveName_TurnYawWeight;
	FName CurveName_RemainingTurnYaw;
	FName CurveName_DisableLegIK;
	float TurnYawWeightCurve = 0.0f;
	float RemainingTurnYawCurve = 0.0f;
	float DisableLegIKCurve = 0.0f;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	bool GetIsCroching() { return MainAnimInstanceProxy.CachedIsCroching; }
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	bool GetIsMovingOnGround() { return MainAnimInstanceProxy.CachedIsMovingOnGround; }
};