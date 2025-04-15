// IPathfinder.h
#pragma once

#include "CoreMinimal.h"
#include "IGridGeometry.h"
#include <queue>
#include <set>
#include <map>

class IPathfinder
{
public:
    virtual ~IPathfinder() = default;

    virtual TArray<FIntPoint> FindPath(
        const FIntPoint& Start,
        const FIntPoint& Goal,
        const IGridGeometry& Geometry,
        const FIntPoint* PreviousCellBias = nullptr,
        const TSet<FIntPoint>* TempUnwalkable = nullptr
    ) = 0;
};