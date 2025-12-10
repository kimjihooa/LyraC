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
#include "MainCharacter.generated.h"

UCLASS()
class PROJECTSSA_API AMainCharacter : public ACharacter
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
	
private:
	//Components
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* Camera;

	UCurveFloat* SmoothCurve;
	UCurveFloat* ExpCurve;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UTimelineComponent* CameraTimeline;
	UFUNCTION()
	void UpdateCameraLocation(float Alpha);
	FVector StartCameraLocation = FVector::ZeroVector;
	FVector TargetCameraLocation = FVector::ZeroVector;
	void SetCameraLocation(FVector NewLoc, float Rate = 1.0f);

	//Inputs
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputMappingContext* DefaultContext;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* MoveInput;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* LookInput;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* JumpInput;

	//Movements
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Walk)")
	float CapsuleHeight;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Walk)")
	FVector CameraLoc;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Walk)")
	float WalkSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Walk)")
	FRotator RotationRate;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Aim)")
	FVector AimCameraLoc;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Sprint)")
	float SprintSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Crouch)")
	float CrouchSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement(Crouch)")
	FVector CrouchCameraLoc;
};
