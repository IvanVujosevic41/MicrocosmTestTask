#include "AStarPathfinder.h"
#include "Algo/Reverse.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "HAL/Platform.h"

struct FNode
{
    FIntPoint Coord;
    float PathCost;

    bool operator<(const FNode& Other) const
    {
        return PathCost < Other.PathCost;
    }
};

TArray<FIntPoint> AStarPathfinder::FindPath(
    const FIntPoint& Start,
    const FIntPoint& Goal,
    const IGridGeometry& Geometry,
    const FIntPoint* PreviousCellBias,
    const TSet<FIntPoint>* TempUnwalkable)
{
    const int32 GridSize = Geometry.GetGridSize();
    const int32 MaxSteps = GridSize * GridSize;
    int32 StepsTaken = 0;

    TArray<FNode> Frontier;
    TSet<FIntPoint> FrontierSet;
    TSet<FIntPoint> ClosedSet;

    TMap<FIntPoint, FIntPoint> CameFrom;
    TMap<FIntPoint, float> CostSoFar;

    Frontier.HeapPush(FNode{ Start, 0.0f });
    FrontierSet.Add(Start);
    CameFrom.Add(Start, Start);
    CostSoFar.Add(Start, 0.0f);

    while (!Frontier.IsEmpty())
    {
        StepsTaken++;
        if (StepsTaken > MaxSteps)
        {
            UE_LOG(LogTemp, Error, TEXT("A* exceeded max steps. Start: %s Goal: %s"), *Start.ToString(), *Goal.ToString());
            return {};
        }

        FNode Current;
        Frontier.HeapPop(Current, true);
        FrontierSet.Remove(Current.Coord);

        if (ClosedSet.Contains(Current.Coord))
            continue;

        ClosedSet.Add(Current.Coord);

        if (Current.Coord == Goal)
        {
            return ReconstructPath(CameFrom, Goal);
        }

        float CurrentCost = CostSoFar[Current.Coord];

        for (const FIntPoint& Neighbor : Geometry.GetNeighbors(Current.Coord))
        {
            if (TempUnwalkable && TempUnwalkable->Contains(Neighbor))
                continue;

            if (ClosedSet.Contains(Neighbor))
                continue;

            float NewCost = CurrentCost + 1.0f;

            //Set a high penalty for returning to the previous cell in order to prevent oscillating with close targets
            if (PreviousCellBias && Neighbor == *PreviousCellBias)
                NewCost += 10000.0f;

            float* ExistingCost = CostSoFar.Find(Neighbor);
            if (!ExistingCost || NewCost < *ExistingCost)
            {
                CostSoFar.Add(Neighbor, NewCost);
                CameFrom.Add(Neighbor, Current.Coord);

                if (!FrontierSet.Contains(Neighbor))
                {
                    float Priority = NewCost + Geometry.HeuristicDistance(Neighbor, Goal);
                    Frontier.HeapPush(FNode{ Neighbor, Priority });
                    FrontierSet.Add(Neighbor);
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("A* could not find path from %s to %s"), *Start.ToString(), *Goal.ToString());
    return {};
}

TArray<FIntPoint> AStarPathfinder::ReconstructPath(
    const TMap<FIntPoint, FIntPoint>& CameFrom,
    const FIntPoint& Current) const
{
    TArray<FIntPoint> Path;
    FIntPoint Step = Current;

    while (CameFrom.Contains(Step) && CameFrom[Step] != Step)
    {
        Path.Add(Step);
        Step = CameFrom[Step];
    }

    Path.Add(Step);
    Algo::Reverse(Path);
    return Path;
}
