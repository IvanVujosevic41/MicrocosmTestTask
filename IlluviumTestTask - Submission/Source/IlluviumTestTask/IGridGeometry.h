// IGridGeometry.h
#pragma once

#include "CoreMinimal.h"

class IGridGeometry
{
public:
    virtual ~IGridGeometry() = default;

    // Returns all neighboring cells based on the grid type
    virtual TArray<FIntPoint> GetNeighbors(const FIntPoint& Cell) const = 0;

    virtual FVector GetTileWorldPosition(const FIntPoint& Cell, const FVector& GridOrigin) const = 0;

    virtual TArray<FIntPoint> GetCellsInRange(const FIntPoint& Center, int32 Range) const = 0;

    // Converts world location to grid coordinate
    virtual FIntPoint WorldToGrid(const FVector& WorldLocation) const = 0;

    virtual float GetTileSize() const = 0;

    virtual int GetGridSize() const = 0;

    virtual float HeuristicDistance(const FIntPoint& A, const FIntPoint& B) const = 0;
};