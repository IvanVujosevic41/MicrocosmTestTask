#pragma once

#include "IPathfinder.h"
#include "MyGridManager.h"

class AStarPathfinder : public IPathfinder
{
public:
    TArray<FIntPoint> FindPath(
        const FIntPoint& Start,
        const FIntPoint& Goal,
        const IGridGeometry& Geometry,
        const FIntPoint* PreviousCell = nullptr,
        const TSet<FIntPoint>* TempUnwalkable = nullptr);

private:

    TArray<FIntPoint> ReconstructPath(
        const TMap<FIntPoint, FIntPoint>& CameFrom,
        const FIntPoint& Current) const;
};