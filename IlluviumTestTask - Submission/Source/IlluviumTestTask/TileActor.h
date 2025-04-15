#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileActor.generated.h"

UCLASS()
class ATileActor : public AActor
{
    GENERATED_BODY()

public:
    ATileActor();

    void SetGridCoord(FIntPoint InCoord);
    FIntPoint GetGridCoord() const;

    void BeginPlay() override;

    UPROPERTY(EditAnywhere)
    UMaterialInterface* DefaultMaterial;

    UPROPERTY(EditAnywhere)
    UMaterialInterface* AlternateMaterial;

    UFUNCTION(BlueprintCallable, Category = "Tile")
    UStaticMeshComponent* GetStaticMeshComponent() const;

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, Category = "Tile")
    float DebugTileSize = 100.f;

    UPROPERTY(EditAnywhere, Category = "Tile")
    float DebugZOffset = 5.f;

    UPROPERTY(EditAnywhere, Category = "Tile")
    FColor DebugColor = FColor::Green;

    UPROPERTY(EditAnywhere, Category = "Tile")
    float DebugThickness = 2.f;


private:
    FIntPoint GridCoord;
};
