#include "SimulationDriver.h"
#include "FHexGrid.h"
#include "FSquareGrid.h"
#include "AStarPathfinder.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ASimulationDriver::ASimulationDriver()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASimulationDriver::BeginPlay()
{
    Super::BeginPlay();
    InitializeSimulation();
}

void ASimulationDriver::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ElapsedTime += DeltaTime;
    if (ElapsedTime >= StepInterval && Simulation)
    {
        Simulation->AdvanceStep();
        ElapsedTime = 0.f;
    }
}

void ASimulationDriver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Simulation)
    {
        Simulation->CleanUp();
    }

    Super::EndPlay(EndPlayReason);
}

void ASimulationDriver::InitializeSimulation()
{
    if (!GridConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("GridConfig is null. Simulation will not start."));
        return;
    }

    ElapsedTime = 0.f;
    StepInterval = IntervalSeconds;

    GridManager = NewObject<UMyGridManager>(this);

    GridManager->SetGridOrigin(GetRootComponent()->GetComponentLocation());
    GridManager->SetWorld(GetWorld()); 
    GridManager->SetOwningActor(this);

    if (!GridConfig->TileBlueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("TileBlueprint not set in GridConfig"));
        return;
    }

    GridManager->SetTileActorClass(GridConfig->TileBlueprint);

    // Create geometry from config
    switch (GridConfig->GridType)
    {
    case EGridType::Hex:
        Geometry = MakeShared<FHexGrid>(GridConfig->GridSize, GridConfig->TileSize);
        break;
    case EGridType::Square:
    default:
        Geometry = MakeShared<FSquareGrid>(GridConfig->GridSize, GridConfig->TileSize);
        break;
    }

    GridManager->SetGridGeometry(Geometry);
    GridManager->SetPathfinder(MakeShared<AStarPathfinder>());
    GridManager->InitializeGrid(GridConfig->GridSize);

    // Create and initialize the simulation system
    Simulation = NewObject<USimulationSystem>(this);
    Simulation->Initialize(Seed, StepInterval, GridManager, NumAgentsPerTeam, BallAgentClass);

    UE_LOG(LogTemp, Log, TEXT("Simulation initialized with GridSize=%d, TileSize=%.1f, Type=%s"),
        GridConfig->GridSize,
        GridConfig->TileSize,
        *UEnum::GetValueAsString(GridConfig->GridType));
}
