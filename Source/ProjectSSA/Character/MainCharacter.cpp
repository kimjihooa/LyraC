// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Components
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 145.0f));
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MESH
	(TEXT("/Game/Assets/Test/SKM_Quinn.SKM_Quinn"));
	if (MESH.Succeeded())
		GetMesh()->SetSkeletalMesh(MESH.Object);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -80.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	CameraTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CameraTimeline"));

	//Movements
	CapsuleHeight = 88.0f;
	CameraLoc = FVector(0.0f, 0.0f, 0.0f);
	AimCameraLoc = FVector(100.0f, 70.0f, 0.0f);
	WalkSpeed = 700.0f;
	WalkAcc = 2048.0f;
	RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	DashSpeed = 2500.0f;
	DashAcc = 8192.0f;
	DashTime = 0.25f;
	SprintSpeed = 1200.0f;
	CrouchSpeed = 400.0f;
	CrouchCameraLoc = FVector(0.0f, 0.0f, -40.0f);

	GetCapsuleComponent()->SetCapsuleHalfHeight(CapsuleHeight);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->RotationRate = RotationRate;

	//Inputs
	static ConstructorHelpers::FObjectFinder<UInputMappingContext>DEFAULT_CONTEXT
	(TEXT("/Game/Inputs/Character/IMC_Character.IMC_Character"));
	if (DEFAULT_CONTEXT.Succeeded())
	{
		DefaultContext = DEFAULT_CONTEXT.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_MOVE
	(TEXT("/Game/Inputs/Character/IA_Move.IA_Move"));
	if (IA_MOVE.Succeeded())
	{
		MoveInput = IA_MOVE.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_LOOK
	(TEXT("/Game/Inputs/Character/IA_Look.IA_Look"));
	if (IA_LOOK.Succeeded())
	{
		LookInput = IA_LOOK.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_AIM
	(TEXT("/Game/Inputs/Character/IA_Aim.IA_Aim"));
	if (IA_AIM.Succeeded())
	{
		AimInput = IA_AIM.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_JUMP
	(TEXT("/Game/Inputs/Character/IA_Jump.IA_Jump"));
	if (IA_JUMP.Succeeded())
	{
		JumpInput = IA_JUMP.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_CROU
	(TEXT("/Game/Inputs/Character/IA_Crouch.IA_Crouch"));
	if (IA_CROU.Succeeded())
	{
		CrouInput = IA_CROU.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_SPRI
	(TEXT("/Game/Inputs/Character/IA_Sprint.IA_Sprint"));
	if (IA_SPRI.Succeeded())
	{
		SpriInput = IA_SPRI.Object;
	}
	
	//Animations
	static ConstructorHelpers::FClassFinder<UAnimInstance>ANIMINS
	(TEXT("/Game/Assets/Test/Animations/ABP_Character.ABP_Character_C"));
	if (ANIMINS.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(ANIMINS.Class);
	}

	//Curves
	static ConstructorHelpers::FObjectFinder<UCurveFloat>SMOOCUR
	(TEXT("/Game/Assets/Curves/C_Smooth.C_Smooth"));
	if (SMOOCUR.Succeeded())
	{
		SmoothCurve = SMOOCUR.Object;
	}
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	FOnTimelineFloat UpdateCamera;
	UpdateCamera.BindUFunction(this, FName("UpdateCameraLocation"));
	if (SmoothCurve)
		CameraTimeline->AddInterpFloat(SmoothCurve, UpdateCamera);
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			SubSystem->AddMappingContext(DefaultContext, 0);
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Triggered, this, &AMainCharacter::Move);
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Completed, this, &AMainCharacter::StopMove);
		EnhancedInputComponent->BindAction(LookInput, ETriggerEvent::Triggered, this, &AMainCharacter::Look);
		EnhancedInputComponent->BindAction(AimInput, ETriggerEvent::Started, this, &AMainCharacter::StartAim);
		EnhancedInputComponent->BindAction(AimInput, ETriggerEvent::Completed, this, &AMainCharacter::StopAim);
		EnhancedInputComponent->BindAction(JumpInput, ETriggerEvent::Triggered, this, &AMainCharacter::Jump);
		EnhancedInputComponent->BindAction(CrouInput, ETriggerEvent::Started, this, &AMainCharacter::StartCrouch);
		EnhancedInputComponent->BindAction(CrouInput, ETriggerEvent::Completed, this, &AMainCharacter::StopCrouch);
		EnhancedInputComponent->BindAction(SpriInput, ETriggerEvent::Started, this, &AMainCharacter::Dash);
		EnhancedInputComponent->BindAction(SpriInput, ETriggerEvent::Completed, this, &AMainCharacter::CheckSprintAfterDash);
	}
}
const EMoveState AMainCharacter::GetMoveState()
{
	return MoveState;
}
const EMoveState AMainCharacter::GetPrevMoveState()
{
	return PrevState;
}
const bool AMainCharacter::GetIsAiming()
{
	return bIsAiming;
}

void AMainCharacter::UpdateCameraLocation(float Alpha)
{
	FVector NewLoc = FMath::Lerp(StartCameraLocation, TargetCameraLocation, Alpha);
	SpringArm->SocketOffset = NewLoc;
}
void AMainCharacter::SetCameraLocation(const FVector NewLoc, float Rate)
{
	if ((NewLoc - SpringArm->SocketOffset).Length() < KINDA_SMALL_NUMBER)
		return;

	StartCameraLocation = SpringArm->SocketOffset;
	TargetCameraLocation = NewLoc;
	CameraTimeline->SetPlayRate(Rate);
	CameraTimeline->PlayFromStart();
}
void AMainCharacter::AddCameraLocation(const FVector NewLoc, float Rate)
{
	SetCameraLocation(SpringArm->SocketOffset + NewLoc, Rate);
}

//Movements
void AMainCharacter::Move(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();
	FRotator YawRotation(0, GetControlRotation().Yaw, 0);

	FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, InputVector.Y);
	AddMovementInput(RightDir, InputVector.X);
}
void AMainCharacter::StopMove()
{
	if (MoveState == EMoveState::Sprint)
		Walk();
}
void AMainCharacter::Look(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();
	if (GetController()->IsValidLowLevelFast())
	{
		AddControllerYawInput(InputVector.X);
		AddControllerPitchInput(-InputVector.Y);
	}
}
void AMainCharacter::StartAim()
{
	if (bIsAiming)
		return;

	if (MoveState == EMoveState::Crouch)
		SetCameraLocation(AimCameraLoc + CrouchCameraLoc, 2.0f);
	else
		SetCameraLocation(AimCameraLoc, 2.0f);
}
void AMainCharacter::StopAim()
{
	if (!bIsAiming)
		return;

	if (MoveState == EMoveState::Crouch)
		SetCameraLocation(CameraLoc + CrouchCameraLoc, 2.0f);
	else
		SetCameraLocation(CameraLoc, 2.0f);
}
void AMainCharacter::Walk()
{
	if (MoveState == EMoveState::Walk)
		return;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxAcceleration = WalkAcc;
	SetCameraLocation(CameraLoc, 2.0f);
	ChangeMoveState(EMoveState::Walk);
}
void AMainCharacter::StartCrouch()
{
	if (MoveState == EMoveState::Crouch)
		return;

	Crouch();
	if (bIsAiming)
		SetCameraLocation(AimCameraLoc + CrouchCameraLoc, 2.0f);
	else
		SetCameraLocation(CrouchCameraLoc, 2.0f);
	ChangeMoveState(EMoveState::Crouch);
}
void AMainCharacter::StopCrouch()
{
	if (MoveState != EMoveState::Crouch)
		return;

	UnCrouch();
	if (bIsAiming)
		SetCameraLocation(AimCameraLoc + CameraLoc, 2.0f);
	else
		SetCameraLocation(CameraLoc, 2.0f);
	ChangeMoveState(EMoveState::Walk);
}
void AMainCharacter::Dash()
{
	if (MoveState == EMoveState::Dash)
		return;

	GetCharacterMovement()->MaxWalkSpeed = DashSpeed;
	GetCharacterMovement()->MaxAcceleration = DashAcc;
	bShouldSprintAfterDash = true;
	ChangeMoveState(EMoveState::Dash);

	GetWorld()->GetTimerManager().SetTimer(DashTimer, FTimerDelegate::CreateLambda([this]() {
		if (bShouldSprintAfterDash)
			Sprint();
		else
			Walk();
		}), DashTime, false);
}
void AMainCharacter::CheckSprintAfterDash()
{
	bShouldSprintAfterDash = false;
}
void AMainCharacter::Sprint()
{
	if (MoveState == EMoveState::Sprint)
		return;

	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	GetCharacterMovement()->MaxAcceleration = WalkAcc;
	ChangeMoveState(EMoveState::Sprint);
}

void AMainCharacter::ChangeMoveState(const EMoveState NewState)
{
	if (NewState == MoveState)
		return;

	PrevState = MoveState;
	MoveState = NewState;
}

