#include "PracticeBall.h"
#include "PracticeCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APracticeBall::APracticeBall()
{
    PrimaryActorTick.bCanEverTick = true;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball"));
    SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Mesh->SetStaticMesh(Sphere.Object);
    Mesh->SetRelativeScale3D(FVector(0.30, 0.18, 0.18));
    Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    Mesh->SetSimulatePhysics(true);
    Mesh->SetLinearDamping(0.35f);
    Mesh->SetAngularDamping(0.6f);
    Mesh->BodyInstance.bUseCCD = true;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    for (int32 Index = 0; Index < 5; ++Index)
    {
        UStaticMeshComponent* Lace = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Lace%d"), Index));
        Lace->SetupAttachment(Mesh);
        Lace->SetStaticMesh(Cube.Object);
        Lace->SetRelativeLocation(FVector((Index - 2) * 7, 0, 46));
        Lace->SetRelativeScale3D(FVector(0.025, 0.30, 0.04));
        Lace->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void APracticeBall::BeginPlay()
{
    Super::BeginPlay();
    HomeLocation = GetActorLocation();
    Mesh->SetMassOverrideInKg(NAME_None, 0.42f);
    Mesh->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Football/Materials/M_Prototype.M_Prototype")));
    if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
        Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor(0.22f, 0.065f, 0.025f));
}

void APracticeBall::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsHeld() && (GetActorLocation().Z < -300 || GetActorLocation().Size2D() > 15000))
        ResetBall();
}

bool APracticeBall::TryPickup(APracticeCharacter* Player)
{
    if (!Player || IsHeld() || FVector::Dist(GetActorLocation(), Player->GetActorLocation()) > 200)
        return false;
    if (!Player->GetMesh()->DoesSocketExist(Player->GetFootballGripSocket()))
        return false;
    Mesh->SetSimulatePhysics(false);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Carrier = Player;
    SetOwner(Player);
    RefreshGripAttachment();
    return true;
}

void APracticeBall::RefreshGripAttachment()
{
    APracticeCharacter* Player = Carrier.Get();
    if (!Player) return;
    AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Player->GetFootballGripSocket());
    const FTransform Offset = Player->GetFootballGripOffset();
    SetActorRelativeLocation(Offset.GetLocation());
    SetActorRelativeRotation(Offset.GetRotation());
    SetActorScale3D(FVector(0.30, 0.18, 0.18));
}

void APracticeBall::Drop()
{
    APracticeCharacter* Player = Carrier.Get();
    if (!Player) return;
    const FVector Location = Player->GetActorLocation() + Player->GetActorForwardVector() * 85;
    const FVector Velocity = Player->GetVelocity() + Player->GetActorForwardVector() * 120;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Carrier.Reset();
    SetOwner(nullptr);
    SetActorLocation(Location);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetSimulatePhysics(true);
    Mesh->SetPhysicsLinearVelocity(Velocity);
}

void APracticeBall::ResetBall()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Carrier.Reset();
    SetOwner(nullptr);
    Mesh->SetSimulatePhysics(false);
    SetActorLocationAndRotation(HomeLocation, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetSimulatePhysics(true);
    Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}
