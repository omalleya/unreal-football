#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PracticeGameMode.generated.h"

UCLASS()
class FOOTBALL_API APracticeGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    APracticeGameMode();
    virtual void StartPlay() override;
};
