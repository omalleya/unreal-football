#include "PlayerAppearance.h"
#include "Animation/AnimClassInterface.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"

#if WITH_EDITOR
bool UPlayerAppearance::AddFootballGripSocket(USkeletalMesh* TargetMesh, FVector Location, FRotator Rotation)
{
    if (!TargetMesh || TargetMesh->GetRefSkeleton().FindBoneIndex(TEXT("hand_r")) == INDEX_NONE) return false;
    if (const USkeletalMeshSocket* Existing = TargetMesh->FindSocket(TEXT("FootballGrip")))
        return Existing->BoneName == TEXT("hand_r");
    TargetMesh->Modify();
    USkeletalMeshSocket* Socket = NewObject<USkeletalMeshSocket>(TargetMesh, NAME_None, RF_Transactional);
    Socket->SocketName = TEXT("FootballGrip");
    Socket->BoneName = TEXT("hand_r");
    Socket->RelativeLocation = Location;
    Socket->RelativeRotation = Rotation;
    TargetMesh->GetMeshOnlySocketList().Add(Socket);
    TargetMesh->MarkPackageDirty();
    return true;
}
#endif

bool UPlayerAppearance::Validate(FString& OutReason) const
{
    if (!Mesh || !AnimationClass)
    {
        OutReason = TEXT("A skeletal mesh and animation class are required.");
        return false;
    }
    const IAnimClassInterface* Animation = IAnimClassInterface::GetFromClass(AnimationClass);
    if (!Animation || Animation->GetTargetSkeleton() != Mesh->GetSkeleton())
    {
        OutReason = TEXT("The mesh and animation blueprint must use the same skeleton.");
        return false;
    }
    const USkeletalMeshSocket* Socket = Mesh->FindSocket(GripSocket);
    if (!Socket || Socket->BoneName != TEXT("hand_r"))
    {
        OutReason = TEXT("FootballGrip must be a socket parented to hand_r.");
        return false;
    }
    for (FName Bone : {FName("root"), FName("pelvis"), FName("spine_03"), FName("hand_r"), FName("hand_l"), FName("foot_r"), FName("foot_l")})
    {
        if (Mesh->GetRefSkeleton().FindBoneIndex(Bone) == INDEX_NONE)
        {
            OutReason = FString::Printf(TEXT("Required shared-rig bone missing: %s"), *Bone.ToString());
            return false;
        }
    }
    if (!MeshTransform.GetScale3D().Equals(FVector::OneVector) || !GripOffset.GetScale3D().Equals(FVector::OneVector) || !Socket->RelativeScale.Equals(FVector::OneVector))
    {
        OutReason = TEXT("Import at player scale; appearance and grip transforms must have unit scale.");
        return false;
    }
    OutReason.Empty();
    return true;
}
