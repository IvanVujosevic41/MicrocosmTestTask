#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GridSpatialPartition.h"
#include "IGridGeometry.h"
#include "IPathfinder.h"
#include "BallAgent.h"
#include "MyGridManager.generated.h"

/*
====================================================================================
  UMyGridManager - Central Grid Management System
====================================================================================

- Handles grid initialization, agent registration, and spatial queries.

Responsibilities:
• Tile Spawning                → Spawns tile actors and assigns materials.
• Grid-to-World Conversion     → Maps between grid coordinates and world space.
• Agent Spatial Partitioning   → Tracks agent positions on the grid.

Notes:
- Holds a reference to UGridSpatialPartition and acts as an interface for querying
  spatial information such as nearby or neighboring agents.
- Maintains internal state about walkability, occupancy, and logical grid size.
- Used by USimulationSystem to interact with the grid and drive agent behavior.
*/

class ATileActor;

UCLASS()
class UMyGridManager : public UObject
{
    GENERATED_BODY()

public:
    UMyGridManager();
    UMyGridManager(const FObjectInitializer& ObjectInitializer);

    void SetGridGeometry(TSharedPtr<IGridGeometry> InGeometry);
    void SetPathfinder(TSharedPtr<IPathfinder> InPathfinder);
    void SetTileActorClass(UClass* InClass);
    void SetWorld(UWorld* InWorld);
    void SetGridOrigin(FVector InOrigin);
    void SetOwningActor(AActor* InActor);
    void InitializeGrid(int32 Size);

    void RegisterAgent(ABallAgent* Agent);
    void RemoveAgent(ABallAgent* Agent);

    void UpdateAgentPosition(ABallAgent* Agent, const FIntPoint& NewCell);
    FIntPoint GetPreviousCellForAgent(ABallAgent* Agent) const;

    bool IsValidCell(const FIntPoint& Cell) const;
    bool IsOccupied(const FIntPoint& Cell) const;

    //Agents that surround the given agent within the given radius
    TArray<ABallAgent*> GetSurroundingAgents(const FIntPoint& Center, int32 Range) const;

    //Neighbours which can be traversed to with the cost of 1 tile
    TArray<ABallAgent*> GetNeighbouringAgents(const FIntPoint& Center) const;

    TArray<FIntPoint> GetPath(const FIntPoint& From, const FIntPoint& To) const;

    FVector GridToWorld(const FIntPoint& Cell) const;
    FIntPoint WorldToGrid(const FVector& WorldLocation) const;

    float GetTileSize() const;
    FVector GetGridOrigin() const { return GridOrigin; }
    int32 GetGridSize() const { return GridSize; }

    TSharedPtr<IGridGeometry> GetGridGeometry() const { return GridGeometry; }
    TSharedPtr<IPathfinder> GetPathfinder() const { return Pathfinder; }
    UWorld* GetWorld() const { return WorldContext; }

private:
    int32 GridSize;

    TMap<FIntPoint, bool> WalkableMap;
    TMap<FIntPoint, ATileActor*> SpawnedTiles;

    TSharedPtr<IGridGeometry> GridGeometry;
    TSharedPtr<IPathfinder> Pathfinder;

    UPROPERTY()
    UClass* TileActorClass;

    UPROPERTY()
    UWorld* WorldContext;

    UPROPERTY()
    AActor* OwningActor;

    UPROPERTY()
    UGridSpatialPartition* SpatialPartition;

    UPROPERTY()
    TMap<ABallAgent*, FIntPoint> AgentToPreviousCellMap;

    FVector GridOrigin;
};
