#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Roads/RoadTypes.h"
#include "Traffic/TrafficTypes.h"
#include "WorldGen/WorldGenTypes.h"
#include "AutopistasSaveGame.generated.h"

/** Datos guardados de un tramo vial */
USTRUCT(BlueprintType)
struct FSavedRoadSegmentData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	ERoadCategory Category = ERoadCategory::Autovia_120;

	UPROPERTY(SaveGame)
	TArray<FVector> SplinePoints;

	UPROPERTY(SaveGame)
	FRoadCrossSection CrossSection;
};

/**
 * Objeto SaveGame para la serializacion persistente del mundo de Autopistas de Espana.
 */
UCLASS()
class AUTOPISTASESPANA_API UAutopistasSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Metadata")
	FString SaveSlotName = TEXT("PartidaGuardada");

	UPROPERTY(VisibleAnywhere, Category = "Metadata")
	FDateTime Timestamp;

	UPROPERTY(VisibleAnywhere, Category = "Metadata")
	float TotalPlayTimeSeconds = 0.0f;

	// Estado Economico
	UPROPERTY(VisibleAnywhere, Category = "Economy")
	int64 CashEuros = 5000000;

	// Configuracion del Terreno
	UPROPERTY(VisibleAnywhere, Category = "World")
	FWorldGenerationSettings WorldSettings;

	// Red de Carreteras
	UPROPERTY(VisibleAnywhere, Category = "Roads")
	TArray<FSavedRoadSegmentData> SavedRoads;
};
