#pragma once

#include "CoreMinimal.h"
#include "BallAgent.h"
#include "GridSpatialPartition.generated.h"

/*
====================================================================================
  UGridSpatialPartition - Lightweight Spatial Partitioning System
====================================================================================

- Lightweight UObject storing agents by grid cell.
- Allows optimized spatial queries for agent lookup and occupancy checks.
*/


class ABallAgent;

UCLASS()
class UGridSpatialPartition : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(int32 InGridSize);

    void RegisterAgent(ABallAgent* Agent, const FIntPoint& Cell);

    void UpdateAgentCell(ABallAgent* Agent, const FIntPoint& OldCell, const FIntPoint& NewCell);

    void RemoveAgent(ABallAgent* Agent, const FIntPoint& Cell);

    void Clear();

    bool IsCellOccupied(const FIntPoint& Cell) const;

    const TSet<ABallAgent*>* GetAgentsAt(const FIntPoint& Cell) const;

private:
    TMap<FIntPoint, TSet<ABallAgent*>> CellsToAgents;
    int32 GridSize = 0;
};
