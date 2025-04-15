#pragma once

#include "CoreMinimal.h"
#include "TileActor.h"
#include "Engine/DataAsset.h"
#include "GridGeometryConfig.generated.h"

UENUM(BlueprintType)
enum class EGridType : uint8
{
    Square UMETA(DisplayName = "Square Grid"),
    Hex UMETA(DisplayName = "Hex Grid")
};

UCLASS(BlueprintType)
class UGridGeometryConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    EGridType GridType = EGridType::Square;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    TSubclassOf<ATileActor> TileBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    int GridSize = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    float TileSize = 100.f;
};
