#include "BallAgent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"

ABallAgent::ABallAgent()
{
    PrimaryActorTick.bCanEverTick = true;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
}

void ABallAgent::Initialize(const FVector& StartWorldPosition, int32 InHP)
{
    MaxHP = HP = InHP;
    CurrentLogicalWorldPosition = StartWorldPosition;
    CurrentVisualWorldPosition = StartWorldPosition;
    SetActorLocation(StartWorldPosition);
    CurrentState = EAgentState::Idle;
}

void ABallAgent::BeginPlay()
{
    Super::BeginPlay();
}

void ABallAgent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TimeSinceLastAttack += DeltaTime;

    // Visual interpolation
    SetActorLocation(FMath::VInterpTo(GetActorLocation(), CurrentVisualWorldPosition, DeltaTime, 10.f));

    switch (CurrentState)
    {
    case EAgentState::WaitingForCombat:
        PauseBeforeCombatTimer += DeltaTime;
        if (PauseBeforeCombatTimer >= PauseBeforeCombatDuration)
        {
            SetState(EAgentState::InCombat);
            PauseBeforeCombatTimer = 0.f;
        }
        return;

    case EAgentState::InCombat:
        if (bIsAttacking)
        {
            UpdateAttackAnimation(DeltaTime);
        }
        return;

    case EAgentState::Moving:
        if (bMoving)
        {
            FollowPath(DeltaTime);
        }
        return;

    default:
        break;
    }

    UpdateMaterialFlash(DeltaTime);
}

bool ABallAgent::CanAttack() const
{
    return TimeSinceLastAttack >= AttackCooldown && CurrentState == EAgentState::Idle;
}

void ABallAgent::ResetAttackCooldown()
{
    TimeSinceLastAttack = 0.f;
}

bool ABallAgent::TryAttack(ABallAgent* Enemy, int32 Damage, const FVector& EnemyWorldLocation)
{
    if (!Enemy || !Enemy->IsAlive()) return false;
    if (!CanAttack()) return false;

    SetQueuedCombatTarget(Enemy);
    PendingDamageToDeal = Damage;
    PlayAttackAnimationTowards(EnemyWorldLocation);
    return true;
}

void ABallAgent::SetQueuedCombatTarget(ABallAgent* Target)
{
    if (QueuedCombatTarget.IsValid())
    {
        QueuedCombatTarget->OnDeathEvent().RemoveDynamic(this, &ABallAgent::HandleTargetAgentDeath);
    }

    QueuedCombatTarget = Target;

    if (QueuedCombatTarget.IsValid())
    {
        QueuedCombatTarget->OnDeathEvent().AddDynamic(this, &ABallAgent::HandleTargetAgentDeath);
    }
}

void ABallAgent::ClearQueuedCombatTarget()
{
    QueuedCombatTarget = nullptr;
    PendingDamageToDeal = 0;
}

void ABallAgent::MoveToWorldLocation(const FVector& TargetWorldLocation)
{
    if (bMoving) return;

    WorldPath = { CurrentLogicalWorldPosition, TargetWorldLocation };
    CurrentPathIndex = 1;

    StartPosition = CurrentLogicalWorldPosition;
    EndPosition = TargetWorldLocation;
    Progress = 0.f;
    bMoving = true;
    SetState(EAgentState::Moving);

#if WITH_EDITOR
    //DrawDebugLine(GetWorld(), StartPosition + FVector(0, 0, 20), EndPosition + FVector(0, 0, 20), FColor::Green, false, 1.0f, 0, 2.0f);
#endif
}

void ABallAgent::SetPath(const TArray<FVector>& InWorldPath)
{
    WorldPath = InWorldPath;
    CurrentPathIndex = 1;

    StartPosition = CurrentLogicalWorldPosition;
    EndPosition = WorldPath.IsValidIndex(CurrentPathIndex) ? WorldPath[CurrentPathIndex] : FVector::ZeroVector;
    Progress = 0.f;
    bMoving = true;
    SetState(EAgentState::Moving);
}

void ABallAgent::FollowPath(float DeltaTime)
{
    if (!WorldPath.IsValidIndex(CurrentPathIndex)) return;

    float SegmentLength = FVector::Dist(StartPosition, EndPosition);
    if (SegmentLength < KINDA_SMALL_NUMBER) return;

    Progress += DeltaTime * MoveSpeed / SegmentLength;
    CurrentVisualWorldPosition = FMath::Lerp(StartPosition, EndPosition, Progress);

    if (Progress >= 1.0f)
    {
        CurrentPathIndex++;

        if (CurrentPathIndex >= WorldPath.Num())
        {
            bMoving = false;
            SetState(EAgentState::Idle);
            return;
        }

        StartPosition = EndPosition;
        EndPosition = WorldPath[CurrentPathIndex];
        Progress = 0.f;
    }
}

void ABallAgent::ReceiveDamage(const FAgentDamageContext& Context)
{
    HP = FMath::Max(HP - Context.Amount, 0);
    UE_LOG(LogTemp, Log, TEXT("%s received %d damage from %s"), *GetNameSafe(this), Context.Amount, *GetNameSafe(Context.Instigator.Get()));

    if (DynamicMaterial)
    {
        UpdateBaseColorBasedOnHealth();
        DamageFlashTimer = DamageFlashDuration;
    }

    if (HP <= 0)
    {
        OnDeath();
    }
}


//Health logic
//-----------------------------------------------------------------------------
void ABallAgent::OnDeath()
{
    bMoving = false;
    bIsAttacking = false;
    SetState(EAgentState::Dead);

    if (QueuedCombatTarget.IsValid())
    {
        QueuedCombatTarget->OnDeathEvent().RemoveDynamic(this, &ABallAgent::HandleTargetAgentDeath);
        QueuedCombatTarget = nullptr;
    }

    DeathEvent.Broadcast(this);
    SetActorTickEnabled(false);
    SetLifeSpan(0.1f);
}

bool ABallAgent::IsAlive() const
{
    return HP > 0;
}

void ABallAgent::HandleTargetAgentDeath(ABallAgent* DeadAgent)
{
    if (QueuedCombatTarget == DeadAgent)
    {
        ClearQueuedCombatTarget();
        SetState(EAgentState::Idle);
    }
}


//Visual logic
//-----------------------------------------------------------------------------
void ABallAgent::PlayAttackAnimationTowards(const FVector& TargetWorldLocation)
{
    SetState(EAgentState::WaitingForCombat);
    FVector Direction = (TargetWorldLocation - CurrentLogicalWorldPosition).GetSafeNormal();
    float Distance = 25.f;

    AttackStart = CurrentLogicalWorldPosition;
    AttackEnd = AttackStart + Direction * Distance;

    bIsAttacking = true;
    AttackPhase = 0;
    AttackProgress = 0.f;
}

void ABallAgent::UpdateAttackAnimation(float DeltaTime)
{
    float SpeedMultiplier = (AttackPhase == 0) ? 3.f : 2.f;
    float Speed = MoveSpeed * SpeedMultiplier;

    float SegmentLength = FVector::Dist(AttackStart, AttackEnd);
    if (SegmentLength < KINDA_SMALL_NUMBER) return;

    AttackProgress += DeltaTime * Speed / SegmentLength;
    CurrentVisualWorldPosition = FMath::Lerp(AttackStart, AttackEnd, AttackProgress);

    if (AttackProgress >= 1.0f)
    {
        if (AttackPhase == 0)
        {
            AttackPhase = 1;
            AttackProgress = 0.f;
            Swap(AttackStart, AttackEnd);
        }
        else
        {
            if (QueuedCombatTarget.IsValid() && OnAttackImpact.IsBound())
            {
                OnAttackImpact.Broadcast(this);
            }
            bIsAttacking = false;
            SetState(EAgentState::Idle);
        }
    }
}

void ABallAgent::SetState(EAgentState NewState)
{
    CurrentState = NewState;
}

void ABallAgent::UpdateMaterialFlash(float DeltaTime)
{
    if (!DynamicMaterial) return;

    if (DamageFlashTimer > 0.f)
    {
        DamageFlashTimer -= DeltaTime;
        float FlashAlpha = FMath::Clamp(DamageFlashTimer / DamageFlashDuration, 0.f, 1.f);
        DynamicMaterial->SetScalarParameterValue("FlashAlpha", FlashAlpha);
    }
    else
    {
        DynamicMaterial->SetScalarParameterValue("FlashAlpha", 0.f);
    }
}

void ABallAgent::UpdateBaseColorBasedOnHealth()
{
    if (!DynamicMaterial || HP <= 0 || MaxHP <= 0) return;

    float HealthRatio = (float)HP / (float)MaxHP;
    DynamicMaterial->SetScalarParameterValue("HealthTintAmount", 1.f - HealthRatio);
}

void ABallAgent::SetTeam(ETeam InTeam)
{
    Team = InTeam;

    UMaterialInterface* BaseMaterial = nullptr;
    switch (Team)
    {
    case ETeam::Red:
        BaseMaterial = RedMaterial;
        break;
    case ETeam::Blue:
        BaseMaterial = BlueMaterial;
        break;
    }

    if (BaseMaterial && Mesh)
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        Mesh->SetMaterial(0, DynamicMaterial);
    }
}