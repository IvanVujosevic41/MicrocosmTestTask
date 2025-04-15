#include "TileActor.h"
#include "DrawDebugHelpers.h"

ATileActor::ATileActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
}

void ATileActor::SetGridCoord(FIntPoint InCoord)
{
    GridCoord = InCoord;
}

FIntPoint ATileActor::GetGridCoord() const
{
    return GridCoord;
}

void ATileActor::BeginPlay()
{
    Super::BeginPlay();

    const FVector Origin = GetActorLocation();
    const float Z = Origin.Z + DebugZOffset;
    const float Size = DebugTileSize;
}

UStaticMeshComponent* ATileActor::GetStaticMeshComponent() const
{
    return Mesh;
}

void ATileActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    const FVector Origin = GetActorLocation();
    const float Z = Origin.Z + DebugZOffset;
    const float Size = DebugTileSize;

    const FVector A = FVector(0, 0, Z);
    const FVector B = FVector(Size, 0, Z);
    const FVector C = FVector(Size, Size, Z);
    const FVector D = FVector(0, Size, Z);
}
