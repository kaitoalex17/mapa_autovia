#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.generated.h"

/** Biomas representativos de la geografia de Espana */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
	MesetaCentral    UMETA(DisplayName = "Meseta Central (Llanuras, Encinas, Campos de Cereal)"),
	LevanteCosta     UMETA(DisplayName = "Levante y Costa (Ramblas secas, Huerta, Montanas costeras)"),
	CantabricoNorte  UMETA(DisplayName = "Cornisa Cantabrica (Valles profundos, Puertos escarpados, Lluvia)")
};

/** Zonificacion funcional catastral */
UENUM(BlueprintType)
enum class EParcelZoneType : uint8
{
	ResidencialBaja  UMETA(DisplayName = "Residencial Baja (Pueblos / Chalets)"),
	ResidencialAlta  UMETA(DisplayName = "Residencial Alta / Ensanche (Bloques de pisos)"),
	ComercialServicios UMETA(DisplayName = "Comercial y Oficinas"),
	PoligonoIndustrial UMETA(DisplayName = "Parque Logistico / Naves Industriales"),
	AgricolaRural    UMETA(DisplayName = "Finca Agricola")
};

/** Configuracion de generacion del mapa sandbox */
USTRUCT(BlueprintType)
struct FWorldGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen")
	EBiomeType Biome = EBiomeType::MesetaCentral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen")
	float MapDimensionsKm = 8.0f; // 8x8 km por defecto (800.000 UU)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen")
	int32 RandomSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen")
	int32 NumCities = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen")
	int32 NumIndustrialHubs = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen")
	float MountainReliefScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen")
	bool bGenerateNavigableRivers = true;
};
