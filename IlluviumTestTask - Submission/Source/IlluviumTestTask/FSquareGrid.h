// SquareGrid.h
#pragma once

#include "CoreMinimal.h"
#include "IGridGeometry.h"

class FSquareGrid : public IGridGeometry
{
public:
    explicit FSquareGrid(int GridSize, float InTileSize);

    virtual TArray<FIntPoint> GetNeighbors(const FIntPoint& Cell) const override;
    virtual FVector GetTileWorldPosition(const FIntPoint& Cell, const FVector& GridOrigin) const override;
    virtual FIntPoint WorldToGrid(const FVector& WorldLocation) const override;
    virtual TArray<FIntPoint> GetCellsInRange(const FIntPoint& Center, int32 Range) const override;
    virtual float GetTileSize() const override { return TileSize; }
    virtual int GetGridSize() const override { return GridSize; }
    virtual float HeuristicDistance(const FIntPoint& A, const FIntPoint& B) const override;

private:
    float TileSize;
    int GridSize;
};
