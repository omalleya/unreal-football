#include "PracticeSmokeTest.h"
#include "PracticeCharacter.h"
#include "PracticeBall.h"
#include "PlayerAppearance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

APracticeSmokeTest::APracticeSmokeTest() { PrimaryActorTick.bCanEverTick = true; }
void APracticeSmokeTest::Check(bool Condition, const TCHAR* Description)
{
    UE_LOG(LogTemp, Display, TEXT("FOOTBALL_TEST %s: %s"), Condition ? TEXT("PASS") : TEXT("FAIL"), Description);
    bFailed |= !Condition;
}
void APracticeSmokeTest::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    Elapsed += DeltaSeconds;
    APracticeCharacter* Player = Cast<APracticeCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    APracticeBall* Ball = nullptr;
    if (TActorIterator<APracticeBall> It(GetWorld()); It) Ball = *It;
    if (!Player || !Ball)
    {
        if (Elapsed > 10) { Check(false, TEXT("Player and ball spawn")); FPlatformMisc::RequestExitWithStatus(false, 1); }
        return;
    }
    if (Stage == 0 && Elapsed > 2)
    {
        FString RequestedAppearance;
        if (FParse::Value(FCommandLine::Get(), TEXT("FootballAppearance="), RequestedAppearance))
            Check(Player->Appearance == LoadObject<UPlayerAppearance>(nullptr, *RequestedAppearance), TEXT("Requested appearance is active, not the fallback"));
        Check(Player->GetCharacterMovement()->IsMovingOnGround(), TEXT("Player stands on field collision"));
        Check(Ball->GetActorLocation().Z > 0 && Ball->GetActorLocation().Z < 20, TEXT("Ball settles on turf"));
        StartPosition = Player->GetActorLocation();
        Stage = 1; Elapsed = 0;
    }
    else if (Stage == 1)
    {
        Player->AddMovementInput(FVector::YAxisVector, 1);
        if (Elapsed > 1)
        {
            Check(FVector::Dist2D(StartPosition, Player->GetActorLocation()) > 250, TEXT("Character movement travels across field"));
            Player->ResetPractice();
            Stage = 2; Elapsed = 0;
        }
    }
    else if (Stage == 2 && Elapsed > 0.5)
    {
        Check(Ball->TryPickup(Player), TEXT("Nearby pickup succeeds"));
        Check(Ball->GetCarrier() == Player && !Ball->GetBallMesh()->IsSimulatingPhysics(), TEXT("Held ball has one carrier and no physics"));
        Check(Ball->GetRootComponent()->GetAttachParent() == Player->GetMesh() &&
            Ball->GetRootComponent()->GetAttachSocketName() == Player->GetFootballGripSocket(), TEXT("Ball is attached to the animated grip socket"));
        UPlayerAppearance* Original = Player->Appearance;
        if (Original)
        {
            const float Speed = Player->GetCharacterMovement()->MaxWalkSpeed;
            const float Radius = Player->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
            UPlayerAppearance* Alternate = DuplicateObject<UPlayerAppearance>(Original, this);
            Alternate->Mesh = DuplicateObject<USkeletalMesh>(Original->Mesh, Alternate);
            Alternate->GripOffset.SetTranslation(FVector(1, 2, 3));
            Check(Player->ApplyAppearance(Alternate), TEXT("Compatible mesh can be swapped while carrying"));
            Check(Ball->GetCarrier() == Player && Ball->GetActorScale3D().Equals(FVector(0.30, 0.18, 0.18), 0.001), TEXT("Swap preserves ball ownership and world size"));
            Check(Player->GetCharacterMovement()->MaxWalkSpeed == Speed && Player->GetCapsuleComponent()->GetUnscaledCapsuleRadius() == Radius, TEXT("Appearance swap preserves speed and gameplay capsule"));
            Check(Ball->GetRootComponent()->GetRelativeLocation().Equals(FVector(1, 2, 3)), TEXT("Swap applies per-model grip calibration"));
            UPlayerAppearance* Invalid = DuplicateObject<UPlayerAppearance>(Alternate, this);
            Invalid->GripSocket = TEXT("MissingSocket");
            Check(!Player->ApplyAppearance(Invalid) && Player->Appearance == Alternate, TEXT("Invalid grip rejected without replacing working appearance"));
            Player->ApplyAppearance(Original);
        }
        else Check(false, TEXT("Default appearance is configured"));
        Ball->Drop();
        Check(!Ball->IsHeld() && Ball->GetBallMesh()->IsSimulatingPhysics(), TEXT("Drop restores physics and clears ownership"));
        Stage = 3; Elapsed = 0;
    }
    else if (Stage == 3 && Elapsed > 1)
    {
        Check(Ball->GetActorLocation().Z > 0, TEXT("Dropped ball remains above field"));
        Player->SetActorLocation(FVector(1000, 1000, 100));
        Check(!Ball->TryPickup(Player), TEXT("Distant pickup rejected"));
        Player->ResetPractice();
        Check(FMath::Abs(Player->GetActorLocation().X + 3657.6) < 1 && !Ball->IsHeld(), TEXT("Reset restores player and loose ball"));
        Player->Jump();
        Stage = 4; Elapsed = 0;
    }
    else if (Stage == 4 && Elapsed > 0.2)
    {
        Check(Player->GetActorLocation().Z > 125, TEXT("Jump lifts character off turf"));
        Player->StopJumping();
        Stage = 5; Elapsed = 0;
    }
    else if (Stage == 5 && Elapsed > 2)
    {
        Check(Player->GetCharacterMovement()->IsMovingOnGround(), TEXT("Jump lands on turf"));
        UE_LOG(LogTemp, Display, TEXT("FOOTBALL_TEST RESULT: %s"), bFailed ? TEXT("FAIL") : TEXT("PASS"));
        FPlatformMisc::RequestExitWithStatus(false, bFailed ? 1 : 0);
    }
}
