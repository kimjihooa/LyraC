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
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	//Movements
	CapsuleHeight = 88.0f;
	CameraLoc = FVector(0.0f, 0.0f, 0.0f);
	AimCameraLoc = FVector(100.0f, 70.0f, 0.0f);
	WalkSpeed = 700.0f;
	WalkAcc = 2048.0f;
	RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	DashSpeed = 2500.0f;
	DashAcc = 10000.0f;
	DashTime = 0.25f;
	SprintSpeed = 1400.0f;
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
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_FIRE
	(TEXT("/Game/Inputs/Character/IA_Fire.IA_Fire"));
	if (IA_FIRE.Succeeded())
	{
		FireInput = IA_FIRE.Object;
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

	if (AbilitySystemComponent)
		AbilitySystemComponent->InitAbilityActorInfo(this, this); //Can be PlayerState, this
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
		EnhancedInputComponent->BindAction(FireInput, ETriggerEvent::Started, this, &AMainCharacter::StartFire);
		EnhancedInputComponent->BindAction(FireInput, ETriggerEvent::Completed, this, &AMainCharacter::StopFire);
		EnhancedInputComponent->BindAction(JumpInput, ETriggerEvent::Triggered, this, &AMainCharacter::Jump);
		EnhancedInputComponent->BindAction(CrouInput, ETriggerEvent::Started, this, &AMainCharacter::StartCrouch);
		EnhancedInputComponent->BindAction(CrouInput, ETriggerEvent::Completed, this, &AMainCharacter::StopCrouch);
		EnhancedInputComponent->BindAction(SpriInput, ETriggerEvent::Started, this, &AMainCharacter::Dash);
		EnhancedInputComponent->BindAction(SpriInput, ETriggerEvent::Completed, this, &AMainCharacter::CheckSprintAfterDash);
	}
}
UAbilitySystemComponent* AMainCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
EMoveState AMainCharacter::GetMoveState() const
{
	return MoveState;
}
EMoveState AMainCharacter::GetPrevMoveState() const
{
	return PrevState;
}
bool AMainCharacter::GetIsAiming() const
{
	return bIsAiming;
}
float AMainCharacter::GetGroundDistance() const
{
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = FVector(TraceStart.X, TraceStart.Y, TraceStart.Z - 100000.0f);
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);
	if (bHit)
		return FMath::Max(HitResult.Distance - (GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()), 0.0f);
	else
		return -1.0f;
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
		SetCameraLocation(CrouchCameraLoc + AimCameraLoc);
	else
		SetCameraLocation(AimCameraLoc);
	bIsAiming = true;
}
void AMainCharacter::StopAim()
{
	if (!bIsAiming)
		return;

	if (MoveState == EMoveState::Crouch)
		SetCameraLocation(CrouchCameraLoc);
	else
		SetCameraLocation(CameraLoc);
	bIsAiming = false;
}
void AMainCharacter::StartFire()
{
	//UE_LOG(LogTemp, Warning, TEXT("Fire Start"));
	FGameplayTag FireTag = FCharacterGameplayTags::Get().State_Combat_Firing;
	AbilitySystemComponent->AddLooseGameplayTag(FireTag);
}
void AMainCharacter::StopFire()
{
	//UE_LOG(LogTemp, Warning, TEXT("Fire Stop"));
	FGameplayTag FireTag = FCharacterGameplayTags::Get().State_Combat_Firing;
	AbilitySystemComponent->RemoveLooseGameplayTag(FireTag);
}
void AMainCharacter::Walk()
{
	if (MoveState == EMoveState::Walk)
		return;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxAcceleration = WalkAcc;
	//SetCameraLocation(CameraLoc);
	ChangeMoveState(EMoveState::Walk);
}
void AMainCharacter::StartCrouch()
{
	if (MoveState == EMoveState::Crouch)
		return;

	Crouch();
	if (bIsAiming)
		SetCameraLocation(CrouchCameraLoc + AimCameraLoc);
	else
		SetCameraLocation(CrouchCameraLoc);
	ChangeMoveState(EMoveState::Crouch);
}
void AMainCharacter::StopCrouch()
{
	if (MoveState != EMoveState::Crouch)
		return;

	UnCrouch();
	if (bIsAiming)
		SetCameraLocation(CameraLoc + AimCameraLoc);
	else
		SetCameraLocation(CameraLoc);
	Walk();
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

