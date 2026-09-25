#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.generated.h"

/** Regiones geograficas y orograficas caracteristicas del territorio espanol */
UENUM(BlueprintType)
enum class ESpanishGeographicRegion : uint8
{
	SierraGuadarrama_SistemaCentral UMETA(DisplayName = "Sierra de Guadarrama - Sistema Central (Cumbres graniticas a +2.400m, pinares, puertos de Somosierra y Navacerrada)"),
	Despenaperros_SierraMorena      UMETA(DisplayName = "Desfiladero de Despenaperros - Sierra Morena (Desfiladero escarpado con canones de roca rojiza, viaductos de gran altura)"),
	PicosDeEuropa_Cantabrico        UMETA(DisplayName = "Picos de Europa - Cornisa Cantabrica (Farallones calizos abruptos a +2.600m y desfiladeros fluviales)"),
	Pirineos_ValleAran              UMETA(DisplayName = "Pirineos - Valle de Aran (Alta montana pirenaica con desniveles extremos)"),
	MesetaCentral_ValleTajo         UMETA(DisplayName = "Meseta Central - Valle del Tajo (Llanura infinita de cereal y escarpes fluviales)")
};

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
	ResidencialBaja    UMETA(DisplayName = "Residencial Baja (Pueblos / Chalets)"),
	ResidencialAlta    UMETA(DisplayName = "Residencial Alta / Ensanche (Bloques de pisos)"),
	ComercialServicios UMETA(DisplayName = "Comercial y Oficinas"),
	PoligonoIndustrial UMETA(DisplayName = "Parque Logistico / Naves Industriales"),
	AgricolaRural      UMETA(DisplayName = "Finca Agricola")
};

/** Configuracion de generacion del mapa sandbox */
USTRUCT(BlueprintType)
struct FWorldGenerationSettings
{
	GENERATED_BODY()

	/** Region geografica representativa de Espana que define el relieve montanoso y orografia */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen|Geography")
	ESpanishGeographicRegion GeographicRegion = ESpanishGeographicRegion::SierraGuadarrama_SistemaCentral;

	/** Bioma base (mantenido por compatibilidad) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen|Geography")
	EBiomeType Biome = EBiomeType::MesetaCentral;

	/** Dimensiones del mapa cuadrado en kilometros (16.0 km = 1.600.000 UU, compatible con World Partition y LWC) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen|Dimensions")
	float MapDimensionsKm = 16.0f;

	/** Toggle para generar automaticamente la red de infraestructura viva preexistente al iniciar partida */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen|Infrastructure")
	bool bSpawnInitialInfrastructure = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen|Parameters")
	int32 RandomSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen|Parameters")
	int32 NumCities = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen|Parameters")
	int32 NumIndustrialHubs = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen|Parameters")
	float MountainReliefScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Gen|Parameters")
	bool bGenerateNavigableRivers = true;
};
