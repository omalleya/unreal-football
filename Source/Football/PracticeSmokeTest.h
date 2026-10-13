#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PracticeSmokeTest.generated.h"

// Opt-in integration test, launched with -FootballSmokeTest. Never spawned in shipping builds.
UCLASS(NotBlueprintable)
class APracticeSmokeTest : public AActor
{
    GENERATED_BODY()
public:
    APracticeSmokeTest();
    virtual void Tick(float DeltaSeconds) override;
private:
    void Check(bool Condition, const TCHAR* Description);
    float Elapsed = 0;
    int32 Stage = 0;
    bool bFailed = false;
    FVector StartPosition;
};
