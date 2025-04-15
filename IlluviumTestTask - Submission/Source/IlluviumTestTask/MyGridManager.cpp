#include "MyGridManager.h"
#include "TileActor.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GridSpatialPartition.h"
#include "UObject/ConstructorHelpers.h"

UMyGridManager::UMyGridManager(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UMyGridManager::UMyGridManager()
{
}

void UMyGridManager::InitializeGrid(int32 Size)
{
    GridSize = Size;
    WalkableMap.Empty();
    SpawnedTiles.Empty();

    SpatialPartition = NewObject<UGridSpatialPartition>(this);
    SpatialPartition->Initialize(Size);

    for (int32 X = 0; X < GridSize; ++X)
    {
        for (int32 Y = 0; Y < GridSize; ++Y)
        {
            FIntPoint Coord(X, Y);
            WalkableMap.Add(Coord, true);

            if (WorldContext && TileActorClass)
            {
                FVector WorldPos = GridGeometry->GetTileWorldPosition(Coord, GridOrigin);
                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                if (!ensure(TileActorClass && TileActorClass->IsChildOf(ATileActor::StaticClass())))
                {
                    UE_LOG(LogTemp, Error, TEXT("TileActorClass is invalid or not a subclass of ATileActor."));
                    return;
                }

                ATileActor* Tile = WorldContext->SpawnActor<ATileActor>(TileActorClass, WorldPos, FRotator::ZeroRotator, Params);
                if (Tile)
                {
                    Tile->SetGridCoord(Coord);
                    SpawnedTiles.Add(Coord, Tile);

                    if ((X + Y) % 2 == 0 && Tile->DefaultMaterial)
                    {
                        Tile->GetStaticMeshComponent()->SetMaterial(0, Tile->DefaultMaterial);
                    }
                    else if (Tile->AlternateMaterial)
                    {
                        Tile->GetStaticMeshComponent()->SetMaterial(0, Tile->AlternateMaterial);
                    }

                    if (OwningActor)
                    {
                        Tile->AttachToActor(OwningActor, FAttachmentTransformRules::KeepWorldTransform);
                    }
                }
            }
        }
    }
}

void UMyGridManager::RegisterAgent(ABallAgent* Agent)
{
    if (!SpatialPartition || !Agent) return;

    FIntPoint Cell = WorldToGrid(Agent->GetCurrentWorldPosition());
    SpatialPartition->RegisterAgent(Agent, Cell);
    AgentToPreviousCellMap.Add(Agent, Cell);
}

bool UMyGridManager::IsValidCell(const FIntPoint& Cell) const
{
    return Cell.X >= 0 && Cell.X < GridSize && Cell.Y >= 0 && Cell.Y < GridSize;
}

TArray<FIntPoint> UMyGridManager::GetPath(const FIntPoint& From, const FIntPoint& To) const
{
    if (!Pathfinder || !GridGeometry) return {};
    return Pathfinder->FindPath(From, To, *GridGeometry);
}

FVector UMyGridManager::GridToWorld(const FIntPoint& Cell) const
{
    return GridGeometry ? GridGeometry->GetTileWorldPosition(Cell, GridOrigin) : FVector::ZeroVector;
}

FIntPoint UMyGridManager::WorldToGrid(const FVector& WorldLocation) const
{
    return GridGeometry ? GridGeometry->WorldToGrid(WorldLocation - GridOrigin) : FIntPoint::ZeroValue;
}

void UMyGridManager::UpdateAgentPosition(ABallAgent* Agent, const FIntPoint& NewCell)
{
    if (!SpatialPartition || !IsValid(Agent) || !GridGeometry.IsValid() || !IsValidCell(NewCell))
        return;

    // Only update if the cell actually changed
    if (const FIntPoint* ExistingCell = AgentToPreviousCellMap.Find(Agent))
    {
        if (*ExistingCell == NewCell)
        {
            return; // Cell hasn't changed, skip update
        }

        // Update the spatial partition with the new cell
        SpatialPartition->UpdateAgentCell(Agent, *ExistingCell, NewCell);
        AgentToPreviousCellMap[Agent] = NewCell;
    }
    else
    {
        // First-time registration
        AgentToPreviousCellMap.Add(Agent, NewCell);
        SpatialPartition->RegisterAgent(Agent, NewCell);
    }
}

FIntPoint UMyGridManager::GetPreviousCellForAgent(ABallAgent* Agent) const
{
    if (const FIntPoint* Ptr = AgentToPreviousCellMap.Find(Agent))
    {
        return *Ptr;
    }
    return WorldToGrid(Agent->GetCurrentWorldPosition()); // fallback
}

void UMyGridManager::RemoveAgent(ABallAgent* Agent)
{
    if (!IsValid(Agent) || !SpatialPartition || !GridGeometry.IsValid())
        return;

    // Get the cell the system registered the agent in
    if (FIntPoint* TrackedCell = AgentToPreviousCellMap.Find(Agent))
    {
        SpatialPartition->RemoveAgent(Agent, *TrackedCell);
        AgentToPreviousCellMap.Remove(Agent);
    }
    else
    {
        // Fallback: remove based on current world position
        const FVector& CurrentPosition = Agent->GetCurrentWorldPosition();
        const FIntPoint Cell = GridGeometry->WorldToGrid(CurrentPosition);
        SpatialPartition->RemoveAgent(Agent, Cell);
    }
}

TArray<ABallAgent*> UMyGridManager::GetSurroundingAgents(const FIntPoint& Center, int32 Range) const
{
    TArray<ABallAgent*> Result;

    TArray<FIntPoint> Cells = GridGeometry->GetCellsInRange(Center, Range);
    for (const FIntPoint& Cell : Cells)
    {
        if (const TSet<ABallAgent*>* Agents = SpatialPartition->GetAgentsAt(Cell))
        {
            Result.Append(Agents->Array());
        }
    }

    return Result;
}

TArray<ABallAgent*> UMyGridManager::GetNeighbouringAgents(const FIntPoint& Center) const
{
    TArray<ABallAgent*> Result;

    if (!GridGeometry || !SpatialPartition)
        return Result;

    // Get the neighbors via the geometry interface (4 orthogonal for square, 6 for hex)
    TArray<FIntPoint> Neighbors = GridGeometry->GetNeighbors(Center);

    for (const FIntPoint& Cell : Neighbors)
    {
        if (const TSet<ABallAgent*>* Agents = SpatialPartition->GetAgentsAt(Cell))
        {
            Result.Append(Agents->Array());
        }
    }

    return Result;
}

float UMyGridManager::GetTileSize() const
{
    return GridGeometry.IsValid() ? GridGeometry->GetTileSize() : 0.0f;
}

bool UMyGridManager::IsOccupied(const FIntPoint& Cell) const
{
    if (!SpatialPartition)
    {
        return false;
    }

    const TSet<ABallAgent*>* Agents = SpatialPartition->GetAgentsAt(Cell);
    if (!Agents)
    {
        return false;
    }

    for (ABallAgent* Agent : *Agents)
    {
        if (Agent && Agent->IsAlive())
        {
            return true;
        }
    }

    return false;
}

void UMyGridManager::SetGridGeometry(TSharedPtr<IGridGeometry> InGeometry)
{
    GridGeometry = InGeometry;
}

void UMyGridManager::SetPathfinder(TSharedPtr<IPathfinder> InPathfinder)
{
    Pathfinder = InPathfinder;
}

void UMyGridManager::SetTileActorClass(UClass* InClass)
{
    TileActorClass = InClass;
}

void UMyGridManager::SetWorld(UWorld* InWorld)
{
    WorldContext = InWorld;
}

void UMyGridManager::SetGridOrigin(FVector InOrigin)
{
    GridOrigin = InOrigin;
}

void UMyGridManager::SetOwningActor(AActor* InActor)
{
    OwningActor = InActor;
}