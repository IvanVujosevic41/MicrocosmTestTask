#include "SimulationSystem.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/LogMacros.h"

void USimulationSystem::Initialize(
    int32 InSeed,
    float InStepInterval,
    UMyGridManager* InGridManager,
    int NumAgentsPerTeam,
    TSubclassOf<ABallAgent> InAgentClass)
{
    RandomStream.Initialize(InSeed);
    StepInterval = InStepInterval;

    GridManager = InGridManager;

    GridGeometry = GridManager->GetGridGeometry();
    Pathfinder = GridManager->GetPathfinder();
    WorldContext = GridManager->GetWorld();
    GridSize = GridManager->GetGridSize();
    AgentClass = InAgentClass;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    FVector GridOrigin = GridManager->GetGridOrigin();
    
    SpawnAllAgents(NumAgentsPerTeam, Params, GridOrigin);

    CurrentStep = 0;
    UE_LOG(LogTemp, Log, TEXT("Simulation initialized with seed %d"), InSeed);
}

void USimulationSystem::AdvanceStep()
{
    if (!WorldContext || WorldContext->bIsTearingDown)
        return;

    if (AllAgents.FilterByPredicate([](ABallAgent* Agent) {
        return Agent && Agent->IsAlive() && Agent->GetTeam() == ETeam::Red;
        }).Num() == 0 ||
            AllAgents.FilterByPredicate([](ABallAgent* Agent) {
            return Agent && Agent->IsAlive() && Agent->GetTeam() == ETeam::Blue;
                }).Num() == 0)
        return;  //TODO: Send an event to GameMode to determine the winner and end the simulation

    AllAgents.RemoveAll([](ABallAgent* Agent) {
        return !IsValid(Agent) || !Agent->IsAlive();
        });

    TSet<FIntPoint> TempUnwalkable;

    for (ABallAgent* Agent : AllAgents)
    {
        if (IsValid(Agent) && Agent->IsAlive())
        {
            TempUnwalkable.Add(GridManager->WorldToGrid(Agent->GetCurrentWorldPosition()));
        }
    }

    for (ABallAgent* Agent : AllAgents)
    {
        if (!IsValid(Agent) || Agent->IsPendingKillPending() || !Agent->IsAlive())
            continue;

        if (!SimulateAttack(Agent))
        {
            SimulateMovement(Agent, TempUnwalkable);
        }
    }

    ++CurrentStep;
}

void USimulationSystem::CleanUp()
{
    for (ABallAgent* Agent : AllAgents)
    {
        if (IsValid(Agent))
        {
            Agent->OnAttackImpact.RemoveDynamic(this, &USimulationSystem::HandleAgentImpact);
            Agent->OnDeathEvent().RemoveDynamic(this, &USimulationSystem::HandleAgentDeath);
            Agent->Destroy();
        }
    }
    AllAgents.Empty();
}

void USimulationSystem::HandleAgentImpact(ABallAgent* Attacker)
{
    if (!Attacker || !Attacker->IsAlive()) return;

    ABallAgent* Target = Attacker->GetQueuedCombatTarget();
    if (!Target || !Target->IsAlive()) return;

    Target->ReceiveDamage({
        .Amount = Attacker->GetPendingDamage(),
        .Instigator = Attacker
        });
}

bool USimulationSystem::SimulateAttack(ABallAgent* Agent)
{
    if (!Agent || !Agent->CanAttack())
        return false;

    const float Range = 100.0f;
    const ETeam MyTeam = Agent->GetTeam();
    const FVector MyLocation = Agent->GetCurrentWorldPosition();
    const FIntPoint MyCell = GridManager->WorldToGrid(MyLocation);

    TArray<ABallAgent*> NearbyAgents = GridManager->GetNeighbouringAgents(MyCell);
    for (ABallAgent* Other : NearbyAgents)
    {
        if (!Other || !Other->IsAlive())
            continue;

        if (Other->GetTeam() != MyTeam)
        {
            return Agent->TryAttack(Other, DamagePerAttack, Other->GetCurrentWorldPosition());
        }
    }

    return false;
}

void USimulationSystem::SimulateMovement(ABallAgent* Agent, const TSet<FIntPoint>& TempUnwalkable)
{
    if (!GridGeometry || !Pathfinder || Agent->GetState() != EAgentState::Idle)
        return;

    ABallAgent* ClosestEnemy = FindClosestEnemy(Agent, GridSize);
    if (!ClosestEnemy) return;

    FIntPoint AgentGridPos = GridManager->WorldToGrid(Agent->GetCurrentWorldPosition());

    TSet<FIntPoint> LocalUnwalkable = TempUnwalkable;
    FIntPoint TargetGridPos = GridManager->WorldToGrid(ClosestEnemy->GetCurrentWorldPosition());
    LocalUnwalkable.Remove(AgentGridPos);
    LocalUnwalkable.Remove(TargetGridPos);

    FIntPoint PreviousCell = GridManager->GetPreviousCellForAgent(Agent);

    TArray<FIntPoint> Path = Pathfinder->FindPath(
        AgentGridPos,
        TargetGridPos,
        *GridGeometry,
        &PreviousCell,
        &LocalUnwalkable
    );

    if (Path.Num() <= 1)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[%s] No path to enemy at %s"), *Agent->GetName(), *TargetGridPos.ToString());
        return;
    }
    else
    {
        const FIntPoint& NextStep = Path[1];

        if (GridManager->IsOccupied(NextStep))
        {
            // The cell has already been reserved in this step by an enemy coming to us - wait for it to come and start attacking  
            return;
        }

        FVector TargetPos = GridManager->GridToWorld(NextStep);
        GridManager->UpdateAgentPosition(Agent, NextStep);
        Agent->MoveToWorldLocation(TargetPos);
        Agent->SetCurrentLogicalWorldPosition(TargetPos);
    }
}

ABallAgent* USimulationSystem::FindClosestEnemy(ABallAgent* Seeker, int32 MaxSearchRadius) const
{
    if (!Seeker || !GridManager) return nullptr;

    const ETeam MyTeam = Seeker->GetTeam();
    const FVector MyLocation = Seeker->GetCurrentWorldPosition();
    const FIntPoint MyCell = GridManager->WorldToGrid(MyLocation);

    ABallAgent* ClosestEnemy = nullptr;
    float ClosestDistSq = TNumericLimits<float>::Max();

    for (int32 Radius = 1; Radius <= MaxSearchRadius; ++Radius)
    {
        TArray<ABallAgent*> Nearby = GridManager->GetSurroundingAgents(MyCell, Radius);
        for (ABallAgent* Other : Nearby)
        {
            if (!Other || !Other->IsAlive() || Other->GetTeam() == MyTeam)
                continue;

            float DistSq = FVector::DistSquared(MyLocation, Other->GetCurrentWorldPosition());
            if (DistSq < ClosestDistSq)
            {
                ClosestDistSq = DistSq;
                ClosestEnemy = Other;
            }
        }

        if (ClosestEnemy)
            break; // Stop as soon as a valid enemy is found in this ring
    }

    return ClosestEnemy;
}


void USimulationSystem::HandleAgentDeath(ABallAgent* Agent)
{
    GridManager->RemoveAgent(Agent);
}

void USimulationSystem::SpawnAllAgents(int32 NumAgentsPerTeam, const FActorSpawnParameters& Params, FVector GridOrigin)
{
    TSet<FIntPoint> OccupiedCells;

    for (ETeam Team : { ETeam::Red, ETeam::Blue })
    {
        int32 Spawned = 0;
        const int32 MaxAttempts = GridSize * GridSize * 2;

        while (Spawned < NumAgentsPerTeam)
        {
            bool bFound = false;
            FIntPoint Start;

            for (int32 Attempts = 0; Attempts < MaxAttempts; ++Attempts)
            {
                Start = FIntPoint(RandomStream.RandRange(0, GridSize - 1), RandomStream.RandRange(0, GridSize - 1));
                if (!OccupiedCells.Contains(Start))
                {
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to find free tile to spawn %s agent #%d."), *UEnum::GetValueAsString(Team), Spawned + 1);
                break;
            }

            OccupiedCells.Add(Start);
            FVector Location = GridGeometry->GetTileWorldPosition(Start, GridOrigin);

            ABallAgent* Agent = SpawnAgent(Team, Location, Start, Params);
            if (Agent)
            {
                AllAgents.Add(Agent);
                Agent->OnAttackImpact.AddDynamic(this, &USimulationSystem::HandleAgentImpact);
                Agent->OnDeathEvent().AddDynamic(this, &USimulationSystem::HandleAgentDeath);
                ++Spawned;
            }
        }
    }
}

ABallAgent* USimulationSystem::SpawnAgent(ETeam Team, const FVector& Location, const FIntPoint& GridCell, const FActorSpawnParameters& Params)
{
    ABallAgent* Agent = WorldContext->SpawnActor<ABallAgent>(AgentClass, Location, FRotator::ZeroRotator, Params);
    if (!Agent) return nullptr;

    const int32 HP = RandomStream.RandRange(2, 5);
    Agent->Initialize(Location, HP);
    Agent->SetTeam(Team);
    GridManager->RegisterAgent(Agent);

    return Agent;
}

