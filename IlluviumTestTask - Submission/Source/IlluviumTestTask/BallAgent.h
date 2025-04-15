#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BallAgent.generated.h"

/*
====================================================================================
  ABallAgent - Agent Actor for Simulation
====================================================================================

- Represents a single agent on the grid, belonging to either the Red or Blue team.
- Maintains logical and visual positions for clean interpolation.
- Updates per tick for animation, movement, and attack feedback.

Core Responsibilities:
• Movement:
    - Interpolates between grid cells using MoveToWorldLocation() and FollowPath().
    - Uses CurrentLogicalWorldPosition for simulation logic and CurrentVisualWorldPosition for rendering.

• Combat:
    - Executes attack animations toward enemies.
    - Triggers damage via OnAttackImpact delegate.
    - Manages state transitions (Idle, Moving, WaitingForCombat, InCombat, Dead).

• Damage Handling:
    - Updates health.
    - Triggers visual feedback (material flash).
    - Broadcasts OnDeathEvent() on death.

• Team Assignment:
    - Sets team material on spawn via SetTeam().

TODO:
- Extract visual logic (e.g., material flash, color based on HP)
  and health/damage logic into separate Actor Components to improve modularity and testability.
*/


UENUM(BlueprintType)
enum class ETeam : uint8
{
    Red,
    Blue
};

UENUM(BlueprintType)
enum class EAgentState : uint8
{
    Idle,
    Moving,
    WaitingForCombat,
    InCombat,
    Dead
};

USTRUCT()
struct FAgentDamageContext
{
    GENERATED_BODY()

    int32 Amount;
    TWeakObjectPtr<class ABallAgent> Instigator;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAgentDied, ABallAgent*, Agent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackImpact, ABallAgent*, Attacker);

UCLASS()
class ABallAgent : public AActor
{
    GENERATED_BODY()

public:
    ABallAgent();

    void Initialize(const FVector& StartWorldPosition, int32 InHP);
    void SetTeam(ETeam InTeam);
    void SetState(EAgentState NewState);
    void MoveToWorldLocation(const FVector& TargetWorldLocation);
    void SetPath(const TArray<FVector>& InWorldPath);
    bool TryAttack(ABallAgent* Enemy, int32 Damage, const FVector& EnemyWorldLocation);
    void ReceiveDamage(const FAgentDamageContext& Context);
    bool IsAlive() const;
    bool CanAttack() const;
    void ResetAttackCooldown();

    FORCEINLINE EAgentState GetState() const { return CurrentState; }
    FORCEINLINE ETeam GetTeam() const { return Team; }
    FORCEINLINE ABallAgent* GetQueuedCombatTarget() const { return QueuedCombatTarget.Get(); }
    FORCEINLINE int32 GetPendingDamage() const { return PendingDamageToDeal; }

    FORCEINLINE FVector GetCurrentWorldPosition() const { return CurrentLogicalWorldPosition; }

    void SetCurrentLogicalWorldPosition(const FVector& NewPosition)
    {
        CurrentLogicalWorldPosition = NewPosition;
    }

    FORCEINLINE void SetPreviousWorldPosition(const FVector& Pos) { PreviousLogicalWorldPosition = Pos; }

    FOnAgentDied& OnDeathEvent() { return DeathEvent; }

    UPROPERTY(BlueprintAssignable)
    FOnAttackImpact OnAttackImpact;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void PlayAttackAnimationTowards(const FVector& TargetWorldLocation);
    void UpdateAttackAnimation(float DeltaTime);
    void FollowPath(float DeltaTime);

    void SetQueuedCombatTarget(ABallAgent* Target);
    void ClearQueuedCombatTarget();
    void OnDeath();

    void UpdateMaterialFlash(float DeltaTime);

    void UpdateBaseColorBasedOnHealth();

    UFUNCTION()
    void HandleTargetAgentDeath(ABallAgent* DeadAgent);

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditDefaultsOnly)
    float MoveSpeed = 130.f;

    UPROPERTY(EditDefaultsOnly)
    float AttackCooldown = 0.7f;

    UPROPERTY(EditDefaultsOnly)
    float PauseBeforeCombatDuration = 1.f;

    UPROPERTY(EditDefaultsOnly)
    UMaterialInterface* RedMaterial;

    UPROPERTY(EditDefaultsOnly)
    UMaterialInterface* BlueMaterial;

    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;

    ETeam Team;
    EAgentState CurrentState = EAgentState::Idle;

    int32 MaxHP = 1;
    int32 HP = 1;
    float TimeSinceLastAttack = 0.f;

    float DamageFlashTimer = 0.f;
    const float DamageFlashDuration = 0.2f;

    // Movement
    TArray<FVector> WorldPath;
    int32 CurrentPathIndex = 0;
    bool bMoving = false;
    float Progress = 0.f;

    FVector StartPosition;
    FVector EndPosition;

    // Logical movement
    FVector CurrentLogicalWorldPosition;
    FVector PreviousLogicalWorldPosition;

    // Visual smoothing
    FVector CurrentVisualWorldPosition;

    // Attack animation
    bool bIsAttacking = false;
    FVector AttackStart;
    FVector AttackEnd;
    int32 AttackPhase = 0;
    float AttackProgress = 0.f;

    // Delayed combat transition
    float PauseBeforeCombatTimer = 0.f;

    // Target handling
    TWeakObjectPtr<ABallAgent> QueuedCombatTarget;
    int32 PendingDamageToDeal = 0;

    FOnAgentDied DeathEvent;
};
