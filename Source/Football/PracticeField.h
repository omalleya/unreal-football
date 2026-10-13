#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PracticeField.generated.h"

class UInstancedStaticMeshComponent;

UCLASS()
class FOOTBALL_API APracticeField : public AActor
{
    GENERATED_BODY()
public:
    APracticeField();
    virtual void OnConstruction(const FTransform& Transform) override;
private:
    UPROPERTY() TArray<TObjectPtr<UInstancedStaticMeshComponent>> Surfaces;
};
