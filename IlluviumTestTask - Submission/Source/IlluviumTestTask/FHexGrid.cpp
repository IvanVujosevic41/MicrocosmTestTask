#include "FHexGrid.h"
#include "Math/UnrealMathUtility.h"

FHexGrid::FHexGrid(int InGridSize, float InTileSize)
    : TileSize(InTileSize), GridSize(InGridSize)
{
}

TArray<FIntPoint> FHexGrid::GetNeighbors(const FIntPoint& Cell) const
{
    const bool bOddRow = Cell.Y % 2 != 0;

    if (bOddRow)
    {
        return {
            Cell + FIntPoint(+1,  0),
            Cell + FIntPoint(0, +1),
            Cell + FIntPoint(+1, +1),
            Cell + FIntPoint(-1,  0),
            Cell + FIntPoint(0, -1),
            Cell + FIntPoint(+1, -1),
        };
    }
    else
    {
        return {
            Cell + FIntPoint(+1,  0),
            Cell + FIntPoint(-1, +1),
            Cell + FIntPoint(0, +1),
            Cell + FIntPoint(-1,  0),
            Cell + FIntPoint(-1, -1),
            Cell + FIntPoint(0, -1),
        };
    }
}

FVector FHexGrid::GetTileWorldPosition(const FIntPoint& Cell, const FVector& GridOrigin) const
{
    const float Radius = TileSize;
    const float Width = Radius * 2.f;
    const float Height = FMath::Sqrt(3.f) * Radius;

    const float XOffset = Width * 0.75f;
    const float YOffset = Height;

    const float OffsetX = (Cell.Y % 2 == 0) ? 0.f : Width * 0.5f;

    const float X = Cell.X * XOffset + OffsetX;
    const float Y = Cell.Y * (YOffset * 0.5f);

    return GridOrigin + FVector(X, Y, 0.f);
}

FIntPoint FHexGrid::WorldToGrid(const FVector& WorldLocation) const
{
    const float Radius = TileSize;
    const float Width = Radius * 2.f;
    const float Height = FMath::Sqrt(3.f) * Radius;

    const float XOffset = Width * 0.75f;
    const float YOffset = Height;

    int approxY = FMath::RoundToInt(WorldLocation.Y / (YOffset * 0.5f));
    bool bOddRow = approxY % 2 != 0;

    float xOffset = bOddRow ? Width * 0.5f : 0.f;
    int approxX = FMath::RoundToInt((WorldLocation.X - xOffset) / XOffset);

    return FIntPoint(approxX, approxY);
}

float FHexGrid::HeuristicDistance(const FIntPoint& A, const FIntPoint& B) const
{
    FIntVector ACube = CubeFromOffset(A);
    FIntVector BCube = CubeFromOffset(B);

    int dx = FMath::Abs(ACube.X - BCube.X);
    int dy = FMath::Abs(ACube.Y - BCube.Y);
    int dz = FMath::Abs(ACube.Z - BCube.Z);

    return (dx + dy + dz) / 2.0f;
}

TArray<FIntPoint> FHexGrid::GetCellsInRange(const FIntPoint& Center, int32 Range) const
{
    TArray<FIntPoint> Result;

    FIntVector CenterCube = CubeFromOffset(Center);

    for (int dx = -Range; dx <= Range; ++dx)
    {
        for (int dy = FMath::Max(-Range, -dx - Range); dy <= FMath::Min(Range, -dx + Range); ++dy)
        {
            int dz = -dx - dy;
            FIntVector Cube = FIntVector(CenterCube.X + dx, CenterCube.Y + dy, CenterCube.Z + dz);
            Result.Add(OffsetFromCube(Cube));
        }
    }

    return Result;
}

FIntVector FHexGrid::CubeFromOffset(const FIntPoint& Offset) const
{
    int x = Offset.X - (Offset.Y - (Offset.Y & 1)) / 2;
    int z = Offset.Y;
    int y = -x - z;
    return FIntVector(x, y, z);
}

FIntPoint FHexGrid::OffsetFromCube(const FIntVector& Cube) const
{
    int col = Cube.X + (Cube.Z - (Cube.Z & 1)) / 2;
    int row = Cube.Z;
    return FIntPoint(col, row);
}
