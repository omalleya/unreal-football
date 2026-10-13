#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PracticeCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UPlayerAppearance;

UCLASS(Config=Game)
class FOOTBALL_API APracticeCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    APracticeCharacter();
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void Tick(float DeltaSeconds) override;
    void Interact();
    void ResetPractice();
    UFUNCTION(BlueprintCallable, Category="Player Appearance")
    bool ApplyAppearance(UPlayerAppearance* NewAppearance);
    UFUNCTION(BlueprintPure, Category="Football")
    bool IsCarryingBall() const;
    FName GetFootballGripSocket() const;
    FTransform GetFootballGripOffset() const;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player Appearance")
    TObjectPtr<UPlayerAppearance> Appearance;
private:
    UPROPERTY(Config)
    FString DefaultAppearancePath = TEXT("/Game/Football/Characters/Shared/DA_PracticeMannequin.DA_PracticeMannequin");
    void Forward(const FInputActionValue& Value);
    void Right(const FInputActionValue& Value);
    void LookYaw(const FInputActionValue& Value);
    void LookPitch(const FInputActionValue& Value);
    void StartSprint();
    void StopSprint();
    UPROPERTY() TObjectPtr<UInputMappingContext> InputContext;
    UPROPERTY() TArray<TObjectPtr<UInputAction>> InputActions;
};
