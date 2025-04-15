#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyGridManager.h"
#include "SimulationSystem.h"

#include "GridGeometryConfig.h"
#include "SimulationDriver.generated.h"

/*
====================================================================================
  ASimulationDriver - Top-Level Actor for Simulation Coordination
====================================================================================

- Serves as the main actor that owns and initializes all major simulation systems.
- Ticks every `IntervalSeconds` (default: 0.1s) to drive simulation forward by
  calling `AdvanceStep()` on the simulation system.

Holds references to:
• UMyGridManager         ? Manages spatial grid, tile spawning, and agent registration.
• USimulationSystem      ? Handles agent behavior, pathfinding, combat, and lifecycle.
• IGridGeometry          ? Provides coordinate conversions and neighbor lookup logic.
• IPathfinder            ? Strategy object used for grid-based pathfinding (e.g. A*).
*/


UCLASS()
class ILLUVIUMTESTTASK_API ASimulationDriver : public AActor
{
    GENERATED_BODY()

public:
    ASimulationDriver();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY()
    USimulationSystem* Simulation;

    UPROPERTY()
    UMyGridManager* GridManager;

    float ElapsedTime = 0.f;
    float StepInterval = 0.f;

    //Hard-coded seed for testing deterministic behavior
    //Would be randomly generated on the server and replicated to the client in a real scenario
    UPROPERTY(EditAnywhere, Category = "Simulation")
    int32 Seed = 123;

    UPROPERTY(EditAnywhere, Category = "Simulation")
    float IntervalSeconds = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Simulation")
    int NumAgentsPerTeam = 3;

    UPROPERTY(EditAnywhere, Category = "Simulation")
    TSubclassOf<ABallAgent> BallAgentClass;

    UPROPERTY(EditAnywhere, Category = "Grid")
    UGridGeometryConfig* GridConfig;

    TSharedPtr<IGridGeometry> Geometry;
    TSharedPtr<IPathfinder> Pathfinder;

    void InitializeSimulation();
};
