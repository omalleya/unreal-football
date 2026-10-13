#include "PracticeCharacter.h"
#include "PracticeBall.h"
#include "PlayerAppearance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

APracticeCharacter::APracticeCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(36, 96);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0, 540, 0);
    GetCharacterMovement()->MaxWalkSpeed = 550;
    GetCharacterMovement()->MaxAcceleration = 1600;
    GetCharacterMovement()->BrakingDecelerationWalking = 1600;
    GetCharacterMovement()->JumpZVelocity = 420;
    GetCharacterMovement()->AirControl = 0.2f;
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> Mannequin(TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin"));
    static ConstructorHelpers::FClassFinder<UAnimInstance> Animation(TEXT("/Game/Mannequin/Animations/ThirdPerson_AnimBP"));
    GetMesh()->SetSkeletalMeshAsset(Mannequin.Object);
    GetMesh()->SetAnimInstanceClass(Animation.Class);
    GetMesh()->SetRelativeLocation(FVector(0, 0, -96));
    GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Hand sockets must stay current even when the player is offscreen or tested without rendering.
    GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    USpringArmComponent* Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    Boom->SetupAttachment(RootComponent);
    Boom->TargetArmLength = 520;
    Boom->SocketOffset = FVector(0, 45, 100);
    Boom->bUsePawnControlRotation = true;
    Boom->bEnableCameraLag = true;
    Boom->CameraLagSpeed = 12;
    UCameraComponent* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    Camera->SetupAttachment(Boom, USpringArmComponent::SocketName);
    Camera->FieldOfView = 85;
}

void APracticeCharacter::BeginPlay()
{
    Super::BeginPlay();
    FString AppearancePath;
    UPlayerAppearance* Candidate = Appearance;
    if (FParse::Value(FCommandLine::Get(), TEXT("FootballAppearance="), AppearancePath))
        Candidate = LoadObject<UPlayerAppearance>(nullptr, *AppearancePath);
    if (!Candidate)
        Candidate = LoadObject<UPlayerAppearance>(nullptr, *DefaultAppearancePath);
    if (!ApplyAppearance(Candidate))
        ApplyAppearance(LoadObject<UPlayerAppearance>(nullptr, TEXT("/Game/Football/Characters/Shared/DA_PracticeMannequin.DA_PracticeMannequin")));
}

bool APracticeCharacter::ApplyAppearance(UPlayerAppearance* NewAppearance)
{
    FString Reason;
    if (!NewAppearance || !NewAppearance->Validate(Reason))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player appearance rejected: %s"), NewAppearance ? *Reason : TEXT("No appearance asset"));
        return false;
    }
    // Reject a different rig rather than silently breaking the shared animation set.
    if (GetMesh()->GetSkeletalMeshAsset()->GetSkeleton() != NewAppearance->Mesh->GetSkeleton())
    {
        UE_LOG(LogTemp, Warning, TEXT("Player appearance rejected: use the shared mannequin skeleton."));
        return false;
    }
    Appearance = NewAppearance;
    GetMesh()->SetSkeletalMeshAsset(Appearance->Mesh);
    GetMesh()->SetAnimInstanceClass(Appearance->AnimationClass);
    GetMesh()->SetRelativeTransform(Appearance->MeshTransform);
    for (TActorIterator<APracticeBall> It(GetWorld()); It; ++It)
        if (It->GetCarrier() == this) It->RefreshGripAttachment();
    UE_LOG(LogTemp, Display, TEXT("Player appearance applied: %s"), *Appearance->GetPathName());
    return true;
}

bool APracticeCharacter::IsCarryingBall() const
{
    for (TActorIterator<APracticeBall> It(GetWorld()); It; ++It)
        if (It->GetCarrier() == this) return true;
    return false;
}

FName APracticeCharacter::GetFootballGripSocket() const
{
    return Appearance ? Appearance->GripSocket : FName(TEXT("FootballGrip"));
}

FTransform APracticeCharacter::GetFootballGripOffset() const
{
    return Appearance ? Appearance->GripOffset : FTransform::Identity;
}

void APracticeCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    UEnhancedInputComponent* Enhanced = CastChecked<UEnhancedInputComponent>(Input);
    InputContext = NewObject<UInputMappingContext>(this);
    auto Action = [this](EInputActionValueType Type)
    {
        UInputAction* Result = NewObject<UInputAction>(this);
        Result->ValueType = Type;
        InputActions.Add(Result);
        return Result;
    };
    auto Map = [this](UInputAction* InputAction, FKey Key, bool Negate = false)
    {
        FEnhancedActionKeyMapping& Mapping = InputContext->MapKey(InputAction, Key);
        if (Negate) Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
    };
    UInputAction* MoveForward = Action(EInputActionValueType::Axis1D);
    Map(MoveForward, EKeys::W); Map(MoveForward, EKeys::S, true); Map(MoveForward, EKeys::Gamepad_LeftY);
    Enhanced->BindAction(MoveForward, ETriggerEvent::Triggered, this, &APracticeCharacter::Forward);
    UInputAction* MoveRight = Action(EInputActionValueType::Axis1D);
    Map(MoveRight, EKeys::D); Map(MoveRight, EKeys::A, true); Map(MoveRight, EKeys::Gamepad_LeftX);
    Enhanced->BindAction(MoveRight, ETriggerEvent::Triggered, this, &APracticeCharacter::Right);
    UInputAction* Yaw = Action(EInputActionValueType::Axis1D);
    Map(Yaw, EKeys::MouseX);
    Enhanced->BindAction(Yaw, ETriggerEvent::Triggered, this, &APracticeCharacter::LookYaw);
    UInputAction* Pitch = Action(EInputActionValueType::Axis1D);
    Map(Pitch, EKeys::MouseY, true);
    Enhanced->BindAction(Pitch, ETriggerEvent::Triggered, this, &APracticeCharacter::LookPitch);
    UInputAction* JumpAction = Action(EInputActionValueType::Boolean);
    Map(JumpAction, EKeys::SpaceBar); Map(JumpAction, EKeys::Gamepad_FaceButton_Bottom);
    Enhanced->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
    Enhanced->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    UInputAction* Sprint = Action(EInputActionValueType::Boolean);
    Map(Sprint, EKeys::LeftShift); Map(Sprint, EKeys::Gamepad_LeftThumbstick);
    Enhanced->BindAction(Sprint, ETriggerEvent::Started, this, &APracticeCharacter::StartSprint);
    Enhanced->BindAction(Sprint, ETriggerEvent::Completed, this, &APracticeCharacter::StopSprint);
    Enhanced->BindAction(Sprint, ETriggerEvent::Canceled, this, &APracticeCharacter::StopSprint);
    UInputAction* Pickup = Action(EInputActionValueType::Boolean);
    Map(Pickup, EKeys::E); Map(Pickup, EKeys::Gamepad_FaceButton_Left);
    Enhanced->BindAction(Pickup, ETriggerEvent::Started, this, &APracticeCharacter::Interact);
    UInputAction* Reset = Action(EInputActionValueType::Boolean);
    Map(Reset, EKeys::R); Map(Reset, EKeys::Gamepad_Special_Right);
    Enhanced->BindAction(Reset, ETriggerEvent::Started, this, &APracticeCharacter::ResetPractice);
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
            Subsystem->AddMappingContext(InputContext, 0);
        PC->SetControlRotation(FRotator(-18, 0, 0));
    }
}

void APracticeCharacter::Forward(const FInputActionValue& Value)
{
    if (Controller) AddMovementInput(FRotationMatrix(FRotator(0, Controller->GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::X), Value.Get<float>());
}
void APracticeCharacter::Right(const FInputActionValue& Value)
{
    if (Controller) AddMovementInput(FRotationMatrix(FRotator(0, Controller->GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::Y), Value.Get<float>());
}
void APracticeCharacter::LookYaw(const FInputActionValue& Value) { AddControllerYawInput(Value.Get<float>()); }
void APracticeCharacter::LookPitch(const FInputActionValue& Value) { AddControllerPitchInput(Value.Get<float>()); }
void APracticeCharacter::StartSprint() { GetCharacterMovement()->MaxWalkSpeed = 850; }
void APracticeCharacter::StopSprint() { GetCharacterMovement()->MaxWalkSpeed = 550; }

void APracticeCharacter::Interact()
{
    for (TActorIterator<APracticeBall> It(GetWorld()); It; ++It)
    {
        if (It->GetCarrier() == this) { It->Drop(); return; }
        if (It->TryPickup(this)) return;
    }
}

void APracticeCharacter::ResetPractice()
{
    GetCharacterMovement()->StopMovementImmediately();
    StopSprint();
    SetActorLocationAndRotation(FVector(-3657.6, 0, 100), FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
    if (Controller) Controller->SetControlRotation(FRotator(-18, 0, 0));
    for (TActorIterator<APracticeBall> It(GetWorld()); It; ++It) It->ResetBall();
}

void APracticeCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (GetActorLocation().Z < -300 || GetActorLocation().Size2D() > 14000) ResetPractice();
}
