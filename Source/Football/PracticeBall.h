#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PracticeBall.generated.h"

class APracticeCharacter;
class UStaticMeshComponent;

UCLASS()
class FOOTBALL_API APracticeBall : public AActor
{
    GENERATED_BODY()
public:
    APracticeBall();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    bool TryPickup(APracticeCharacter* Player);
    void Drop();
    void ResetBall();
    void RefreshGripAttachment();
    bool IsHeld() const { return Carrier.IsValid(); }
    APracticeCharacter* GetCarrier() const { return Carrier.Get(); }
    UStaticMeshComponent* GetBallMesh() const { return Mesh; }
private:
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY() TWeakObjectPtr<APracticeCharacter> Carrier;
    FVector HomeLocation;
};
