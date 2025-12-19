// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"
#include "Camera/CameraComponent.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "CharacterGameplayTags.h"
#include "MainCharacter.generated.h"

UENUM(BlueprintType)
enum class EMoveState :uint8
{
	Walk,
	Crouch,
	Dash,
	Sprint
};

UCLASS()
class PROJECTSSA_API AMainCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const EMoveState GetMoveState();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const EMoveState GetPrevMoveState();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const bool GetIsAiming();

private:
	//Components
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* Camera;

	UCurveFloat* SmoothCurve;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UTimelineComponent* CameraTimeline;
	UFUNCTION()
	void UpdateCameraLocation(float Alpha);
	FVector StartCameraLocation = FVector::ZeroVector;
	FVector TargetCameraLocation = FVector::ZeroVector;
	UFUNCTION(BlueprintCallable)
	void SetCameraLocation(const FVector NewLoc, float Rate = 1.0f);
	void AddCameraLocation(const FVector NewLoc, float Rate = 1.0f);

	//Inputs
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputMappingContext* DefaultContext;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* MoveInput;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* LookInput;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* FireInput;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* AimInput;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* JumpInput;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* CrouInput;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* SpriInput;

	//Movements
	void Move(const FInputActionValue& Value);
	void StopMove();
	void Look(const FInputActionValue& Value);
	void StartAim();
	void StopAim();
	void StartFire();
	void StopFire();
	void Walk();
	void StartCrouch();
	void StopCrouch();
	void Dash();
	void Sprint();

	EMoveState MoveState = EMoveState::Walk;
	EMoveState PrevState = EMoveState::Walk;
	void ChangeMoveState(const EMoveState NewState);

	FTimerHandle DashTimer;
	bool bShouldSprintAfterDash = true;
	void CheckSprintAfterDash();

	bool bIsAiming = false;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Movement(Walk)")
	float CapsuleHeight;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Walk)")
	FVector CameraLoc;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Walk)")
	float WalkSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Walk)")
	float WalkAcc;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Walk)")
	FRotator RotationRate;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Aim)")
	FVector AimCameraLoc;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Crouch)")
	float CrouchSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Crouch)")
	FVector CrouchCameraLoc;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Dash)")
	float DashSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Dash)")
	float DashAcc;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Dash)")
	float DashTime;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Sprint)")
	float SprintSpeed;
};
