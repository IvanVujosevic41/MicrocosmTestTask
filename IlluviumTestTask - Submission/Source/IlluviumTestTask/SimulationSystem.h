#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BallAgent.h"
#include "MyGridManager.h"
#include "SimulationSystem.generated.h"

/*
====================================================================================
  USimulationSystem - Core Simulation Logic Manager
====================================================================================

- Owns the simulation logic and agent state.
- Called by the SimulationDriver every IntervalSeconds to advance the simulation.

Responsibilities:
• AdvanceStep()
    - Checks if both teams are still alive.
    - Triggers attacks if enemies are in range.
    - Moves agents toward closest enemy if no attack is performed.

• SpawnAgent / SpawnAllAgents()
    - Spawns agents at random walkable grid positions.
    - Registers them with the grid manager and binds to death/impact delegates.

• HandleAgentImpact / HandleAgentDeath
    - Applies damage and removes agents from spatial partition on death.

Notes:
- Agent actions are deterministic based on RandomStream seed.
*/


class FMyGridManager;
class IGridGeometry;
class IPathfinder;

UCLASS()
class USimulationSystem : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(
        int32 InSeed,
        float InStepInterval,
        UMyGridManager* InGridManager,
        int NumAgentsPerTeam,
        TSubclassOf<ABallAgent> InAgentClass);

    void CleanUp();

    void AdvanceStep();

private:
    void SimulateMovement(ABallAgent* Agent, const TSet<FIntPoint>& TempUnwalkale);
    bool SimulateAttack(ABallAgent* Agent);

    void SpawnAllAgents(int32 NumAgentsPerTeam, const FActorSpawnParameters& Params, FVector GridOrigin);
    ABallAgent* SpawnAgent(ETeam Team, const FVector& Location, const FIntPoint& GridCell, const FActorSpawnParameters& Params);

    ABallAgent* FindClosestEnemy(ABallAgent* Seeker, int32 MaxSearchRadius) const;

    UFUNCTION()
    void HandleAgentImpact(ABallAgent* Attacker);

    UFUNCTION()
    void HandleAgentDeath(ABallAgent* Agent);

private:
    UPROPERTY()
    TArray<ABallAgent*> AllAgents;

    UPROPERTY()
    TObjectPtr<UMyGridManager> GridManager;

    TSharedPtr<IGridGeometry> GridGeometry;
    TSharedPtr<IPathfinder> Pathfinder;
    UWorld* WorldContext = nullptr;

    TSubclassOf<ABallAgent> AgentClass;
    int32 GridSize = 0;
    int32 CurrentStep = 0;

    float StepInterval = 0.1f;
    float DamagePerAttack = 1.f;

    FRandomStream RandomStream;
};
