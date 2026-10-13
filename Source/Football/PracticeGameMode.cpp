#include "PracticeGameMode.h"
#include "PracticeCharacter.h"
#include "PracticeBall.h"
#include "PracticeHUD.h"
#include "PracticeSmokeTest.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

APracticeGameMode::APracticeGameMode()
{
    DefaultPawnClass = APracticeCharacter::StaticClass();
    HUDClass = APracticeHUD::StaticClass();
}

void APracticeGameMode::StartPlay()
{
    Super::StartPlay();
    GetWorld()->SpawnActor<APracticeBall>(FVector(-3490, 0, 35), FRotator::ZeroRotator);
    UE_LOG(LogTemp, Display, TEXT("Football practice ready: WASD move, mouse look, Shift sprint, Space jump, E ball, R reset."));
#if !UE_BUILD_SHIPPING
    if (FParse::Param(FCommandLine::Get(), TEXT("FootballSmokeTest")))
        GetWorld()->SpawnActor<APracticeSmokeTest>();
#endif
}
