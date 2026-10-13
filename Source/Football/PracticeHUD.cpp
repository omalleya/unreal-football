#include "PracticeHUD.h"
#include "PracticeBall.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

void APracticeHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !PlayerOwner) return;
    const float X = 28, Y = 28;
    DrawRect(FLinearColor(0.015f, 0.025f, 0.04f, 0.88f), X, Y, 375, 112);
    DrawRect(FLinearColor(0.95f, 0.52f, 0.13f), X, Y, 4, 112);
    DrawText(TEXT("FOOTBALL  /  PRACTICE FIELD"), FLinearColor::White, X + 18, Y + 14, GEngine->GetMediumFont(), 1.0f);
    DrawText(TEXT("WASD  Move     Mouse  Look     Shift  Sprint"), FLinearColor(0.8f, 0.86f, 0.9f), X + 18, Y + 48, GEngine->GetSmallFont());
    DrawText(TEXT("Space  Jump      E  Pick up / drop      R  Reset"), FLinearColor(0.8f, 0.86f, 0.9f), X + 18, Y + 72, GEngine->GetSmallFont());
    APawn* Player = PlayerOwner->GetPawn();
    if (!Player) return;
    FString Status = TEXT("Find the ball ahead. Press E when close.");
    if (TActorIterator<APracticeBall> It(GetWorld()); It)
    {
        if (It->IsHeld()) Status = TEXT("BALL CARRIED   /   E to drop");
        else if (FVector::Dist(It->GetActorLocation(), Player->GetActorLocation()) <= 200)
            Status = TEXT("E   PICK UP BALL");
    }
    const float Bottom = Canvas->ClipY - 80;
    DrawRect(FLinearColor(0.015f, 0.025f, 0.04f, 0.88f), X, Bottom, 375, 48);
    DrawText(Status, FLinearColor::White, X + 18, Bottom + 16, GEngine->GetSmallFont());
}
