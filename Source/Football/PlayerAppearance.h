#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerAppearance.generated.h"

class USkeletalMesh;
class UAnimInstance;

/** Visual configuration only. Movement, collision, and football rules stay on the shared player. */
UCLASS(BlueprintType)
class FOOTBALL_API UPlayerAppearance : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
    TObjectPtr<USkeletalMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
    TSubclassOf<UAnimInstance> AnimationClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
    FTransform MeshTransform = FTransform(FRotator(0, -90, 0), FVector(0, 0, -96));

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Football")
    FName GripSocket = TEXT("FootballGrip");

    // Ball-center transform relative to the grip socket. Scale stays one; ball dimensions are shared.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Football")
    FTransform GripOffset = FTransform::Identity;

    bool Validate(FString& OutReason) const;
#if WITH_EDITOR
    // Editor import helper; leaves an existing artist-calibrated socket untouched.
    UFUNCTION(BlueprintCallable, Category="Player Appearance|Authoring")
    static bool AddFootballGripSocket(USkeletalMesh* TargetMesh, FVector Location, FRotator Rotation);
#endif
};
