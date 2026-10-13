#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PracticeHUD.generated.h"

UCLASS()
class FOOTBALL_API APracticeHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
