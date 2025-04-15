#include "GridSpatialPartition.h"
#include "BallAgent.h"

void UGridSpatialPartition::Initialize(int32 InGridSize)
{
    GridSize = InGridSize;
    CellsToAgents.Empty();
}

void UGridSpatialPartition::RegisterAgent(ABallAgent* Agent, const FIntPoint& Cell)
{
    if (!Agent) return;

    // Prevent adding duplicate
    if (!CellsToAgents.FindOrAdd(Cell).Contains(Agent))
    {
        CellsToAgents.FindOrAdd(Cell).Add(Agent);
    }
}

void UGridSpatialPartition::UpdateAgentCell(ABallAgent* Agent, const FIntPoint& OldCell, const FIntPoint& NewCell)
{
    if (!Agent) return;

    // Remove from old cell
    if (TSet<ABallAgent*>* OldSet = CellsToAgents.Find(OldCell))
    {
        OldSet->Remove(Agent);
        if (OldSet->Num() == 0)
        {
            CellsToAgents.Remove(OldCell);
        }
    }

    // Add to new cell
    CellsToAgents.FindOrAdd(NewCell).Add(Agent);
}

void UGridSpatialPartition::RemoveAgent(ABallAgent* Agent, const FIntPoint& Cell)
{
    if (!Agent) return;

    if (TSet<ABallAgent*>* Set = CellsToAgents.Find(Cell))
    {
        Set->Remove(Agent);
        if (Set->Num() == 0)
        {
            CellsToAgents.Remove(Cell);
        }
    }
}

void UGridSpatialPartition::Clear()
{
    CellsToAgents.Empty();
}

bool UGridSpatialPartition::IsCellOccupied(const FIntPoint& Cell) const
{
    const TSet<ABallAgent*>* Set = CellsToAgents.Find(Cell);
    return Set && Set->Num() > 0;
}

const TSet<ABallAgent*>* UGridSpatialPartition::GetAgentsAt(const FIntPoint& Cell) const
{
    return CellsToAgents.Find(Cell);
}