#include "FSquareGrid.h"

FSquareGrid::FSquareGrid(int InGridSize, float InTileSize)
    : TileSize(InTileSize) , GridSize(InGridSize)
{
}

TArray<FIntPoint> FSquareGrid::GetNeighbors(const FIntPoint& Cell) const
{
    return {
        Cell + FIntPoint(1, 0),
        Cell + FIntPoint(-1, 0),
        Cell + FIntPoint(0, 1),
        Cell + FIntPoint(0, -1)
    };
}

FIntPoint FSquareGrid::WorldToGrid(const FVector& WorldLocation) const
{
    // Convert world X and Y to grid coordinates
    int32 X = FMath::RoundToInt(WorldLocation.X / TileSize);
    int32 Y = FMath::RoundToInt(WorldLocation.Y / TileSize);

    return FIntPoint(X, Y);
}

TArray<FIntPoint> FSquareGrid::GetCellsInRange(const FIntPoint& Center, int32 Range) const
{
    TArray<FIntPoint> Result;

    for (int32 dx = -Range; dx <= Range; ++dx)
    {
        for (int32 dy = -Range; dy <= Range; ++dy)
        {
            if (FMath::Abs(dx) + FMath::Abs(dy) > Range) continue;
            Result.Add(Center + FIntPoint(dx, dy));
        }
    }

    return Result;
}

FVector FSquareGrid::GetTileWorldPosition(const FIntPoint& Cell, const FVector& GridOrigin) const
{
    return GridOrigin + FVector(Cell.X * TileSize, Cell.Y * TileSize, 0.f);
}

float FSquareGrid::HeuristicDistance(const FIntPoint& A, const FIntPoint& B) const
{
    return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y); // Manhattan distance
}
