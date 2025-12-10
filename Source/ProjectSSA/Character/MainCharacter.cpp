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
	WalkSpeed = 500.0f;
	RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	SprintSpeed = 900.0f;
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
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_JUMP
	(TEXT("/Game/Inputs/Character/IA_Jump.IA_Jump"));
	if (IA_JUMP.Succeeded())
	{
		JumpInput = IA_JUMP.Object;
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
	(TEXT("/Game/Blueprints/Characters/C_SmothCurve.C_SmothCurve"));
	if (SMOOCUR.Succeeded())
	{
		SmoothCurve = SMOOCUR.Object;
	}
	static ConstructorHelpers::FObjectFinder<UCurveFloat>EXPCUR
	(TEXT("/Game/Blueprints/Characters/C_ExpCurve.C_ExpCurve"));
	if (EXPCUR.Succeeded())
	{
		ExpCurve = EXPCUR.Object;
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
		EnhancedInputComponent->BindAction(LookInput, ETriggerEvent::Triggered, this, &AMainCharacter::Look);
		EnhancedInputComponent->BindAction(JumpInput, ETriggerEvent::Triggered, this, &AMainCharacter::Jump);
	}

}

void AMainCharacter::UpdateCameraLocation(float Alpha)
{
	FVector NewLoc = FMath::Lerp(StartCameraLocation, TargetCameraLocation, Alpha);
	SpringArm->SocketOffset = NewLoc;
}
void AMainCharacter::SetCameraLocation(FVector NewLoc, float Rate)
{
	StartCameraLocation = SpringArm->SocketOffset;
	TargetCameraLocation = NewLoc;
	CameraTimeline->SetPlayRate(Rate);
	CameraTimeline->PlayFromStart();
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
void AMainCharacter::Look(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();
	if (GetController()->IsValidLowLevelFast())
	{
		AddControllerYawInput(InputVector.X);
		AddControllerPitchInput(-InputVector.Y);
	}
}