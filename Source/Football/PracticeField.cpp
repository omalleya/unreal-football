#include "PracticeField.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

APracticeField::APracticeField()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("FieldRoot")));
    RootComponent->SetMobility(EComponentMobility::Static);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    const TCHAR* Names[] = { TEXT("Surround"), TEXT("Turf"), TEXT("AlternateTurf"), TEXT("Markings"), TEXT("HomeEndzone"), TEXT("AwayEndzone"), TEXT("Goalposts") };
    for (int32 Index = 0; Index < 7; ++Index)
    {
        UInstancedStaticMeshComponent* Surface = CreateDefaultSubobject<UInstancedStaticMeshComponent>(Names[Index]);
        Surface->SetupAttachment(RootComponent);
        Surface->SetStaticMesh(Cube.Object);
        Surface->SetMobility(EComponentMobility::Static);
        Surface->SetCollisionEnabled(Index < 3 ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Surface->SetCollisionResponseToAllChannels(ECR_Block);
        Surfaces.Add(Surface);
    }
}

void APracticeField::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    const TCHAR* Names[] = { TEXT("Surround"), TEXT("Turf"), TEXT("AlternateTurf"), TEXT("Markings"), TEXT("HomeEndzone"), TEXT("AwayEndzone"), TEXT("Goalposts") };
    for (int32 Index = 0; Index < Surfaces.Num(); ++Index)
    {
        Surfaces[Index]->ClearInstances();
        const FString Path = FString::Printf(TEXT("/Game/Football/Materials/MI_%s.MI_%s"), Names[Index], Names[Index]);
        if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *Path))
            Surfaces[Index]->SetMaterial(0, Material);
    }
    auto Box = [this](int32 Group, FVector Position, FVector Size)
    {
        Surfaces[Group]->AddInstance(FTransform(FQuat::Identity, Position, Size / 100));
    };
    // Unreal uses centimeters: 120 x 53 1/3 yards, including two 10-yard endzones.
    constexpr double Yard = 91.44;
    constexpr double HalfWidth = 2438.4;
    Box(0, FVector(0, 0, -45), FVector(30000, 22000, 80));
    Box(1, FVector(0, 0, -6), FVector(120 * Yard, HalfWidth * 2, 12));
    for (int32 Stripe = 0; Stripe < 20; ++Stripe)
        Box(Stripe % 2 ? 1 : 2, FVector((-47.5 + Stripe * 5) * Yard, 0, 0.5), FVector(5 * Yard, HalfWidth * 2, 1));
    for (int32 Side : {-1, 1})
    {
        Box(Side < 0 ? 4 : 5, FVector(Side * 55 * Yard, 0, 1), FVector(10 * Yard, HalfWidth * 2, 2));
        Box(3, FVector(0, Side * HalfWidth, 2.5), FVector(120 * Yard + 15, 15, 2));
        Box(3, FVector(Side * 60 * Yard, 0, 2.5), FVector(15, HalfWidth * 2, 2));
        // Simple non-colliding uprights beyond the endline.
        const double X = Side * 60.5 * Yard;
        Box(6, FVector(X, 0, 150), FVector(12, 12, 300));
        Box(6, FVector(X, 0, 305), FVector(12, 564, 12));
        Box(6, FVector(X, -282, 530), FVector(12, 12, 450));
        Box(6, FVector(X, 282, 530), FVector(12, 12, 450));
    }
    for (int32 Line = -50; Line <= 50; Line += 5)
        Box(3, FVector(Line * Yard, 0, 2.5), FVector(10, HalfWidth * 2, 2));
    for (int32 Line = -49; Line < 50; ++Line)
    {
        if (Line % 5 == 0) continue;
        for (double Y : {-HalfWidth + 35, -282.0, 282.0, HalfWidth - 35})
            Box(3, FVector(Line * Yard, Y, 2.5), FVector(5, 60, 2));
    }

    TArray<UTextRenderComponent*> OldLabels;
    GetComponents<UTextRenderComponent>(OldLabels);
    for (UTextRenderComponent* Label : OldLabels) Label->DestroyComponent();
    auto Label = [this](const FString& Text, FVector Position, float Size, double Yaw)
    {
        UTextRenderComponent* Component = NewObject<UTextRenderComponent>(this);
        Component->CreationMethod = EComponentCreationMethod::UserConstructionScript;
        Component->SetupAttachment(RootComponent);
        Component->SetText(FText::FromString(Text));
        Component->SetWorldSize(Size);
        Component->SetHorizontalAlignment(EHTA_Center);
        Component->SetVerticalAlignment(EVRTA_TextCenter);
        Component->SetTextRenderColor(FColor(230, 237, 224));
        Component->SetRelativeLocation(Position);
        Component->SetRelativeRotation(FRotator(90, Yaw, 0));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->RegisterComponent();
    };
    for (int32 Line = -40; Line <= 40; Line += 10)
    {
        const FString Number = FString::FromInt(50 - FMath::Abs(Line));
        Label(Number, FVector(Line * Yard, -1800, 4), 120, 90);
        Label(Number, FVector(Line * Yard, 1800, 4), 120, -90);
    }
    Label(TEXT("HOME"), FVector(-55 * Yard, 0, 4), 320, 0);
    Label(TEXT("AWAY"), FVector(55 * Yard, 0, 4), 320, 180);
}
