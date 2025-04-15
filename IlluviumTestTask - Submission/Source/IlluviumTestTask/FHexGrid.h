#pragma once

#include "CoreMinimal.h"
#include "IGridGeometry.h"

class FHexGrid : public IGridGeometry
{
public:
    explicit FHexGrid(int GridSize, float InHexSize);

    virtual TArray<FIntPoint> GetNeighbors(const FIntPoint& Cell) const override;
    virtual FIntPoint WorldToGrid(const FVector& WorldLocation) const override;
    virtual FVector GetTileWorldPosition(const FIntPoint& Cell, const FVector& GridOrigin) const override;
    virtual float GetTileSize() const override { return TileSize; }
    virtual int GetGridSize() const override { return GridSize; }
    virtual TArray<FIntPoint> GetCellsInRange(const FIntPoint& Center, int32 Range) const override;
    virtual float HeuristicDistance(const FIntPoint& A, const FIntPoint& B) const override;

private:
    // Converts an odd-r offset coordinate to a cube coordinate
    FIntVector CubeFromOffset(const FIntPoint& Offset) const;

    // Converts a cube coordinate to an odd-r offset coordinate
    FIntPoint OffsetFromCube(const FIntVector& Cube) const;

    float TileSize; // Outer radius (distance from center to a corner)

    int GridSize;
};
