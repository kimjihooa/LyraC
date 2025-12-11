// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
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

UCLASS()
class PROJECTSSA_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UCharacterAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	UCharacterMovementComponent* GetMovementComponent();

	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateLocationData(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateRotationData();
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
	void UpdateAimingData();
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void UpdateJumpFallData();
	UFUNCTION(BlueprintCallable, Category = "UpdateData")
	void SetRootYawOffset(float InRootYawOffset);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UpdateData")
	ECardinalDirection SelectCarialDirectionFromAngle(const float Angle, const float DeadZone, const ECardinalDirection CurrentDirection, const bool bUseCurrentDirection);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UpdateData")
	ECardinalDirection GetOppositeCardinalDirection(const ECardinalDirection CurrentDir);

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
	UPROPERTY(BlueprintReadOnly, Category = "AimingData")
	float AimYaw = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "AimingData")
	float AimPitch = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "JumpFallData")
	float TimeToJumpApex = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "others")
	bool bGameplayTagIsADS = false;
	UPROPERTY(BlueprintReadOnly, Category = "others")
	bool bGameplayTagIsFiring = false;
	UPROPERTY(BlueprintReadOnly, Category = "others")
	bool bGameplayTagIsDashing = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsFirstUpdate = true;
	UPROPERTY(BlueprintReadOnly)
	bool bEnableRootYawOffset = true;

};
