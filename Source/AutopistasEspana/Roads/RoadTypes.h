#pragma once

#include "CoreMinimal.h"
#include "RoadTypes.generated.h"

/** Tipologias de Carreteras segun la Red Viaria de Espana */
UENUM(BlueprintType)
enum class ERoadCategory : uint8
{
	CaminoRural        UMETA(DisplayName = "Camino Rural / Asfalto Basico (40 km/h)"),
	Convencional_90    UMETA(DisplayName = "Carretera Nacional Convencional (90 km/h)"),
	Autovia_120        UMETA(DisplayName = "Autovia 2x2 (120 km/h)"),
	Autopista_3x3      UMETA(DisplayName = "Autopista de Gran Capacidad 3x3 (120 km/h)"),
	RamalEnlace        UMETA(DisplayName = "Ramal de Incorporacion / Salida (60-80 km/h)")
};

/** Tipo de estructura segun la rasante respecto al terreno */
UENUM(BlueprintType)
enum class ERoadElevationType : uint8
{
	RasanteTerreno     UMETA(DisplayName = "Sobre Rasante de Terreno"),
	ViaductoPuente     UMETA(DisplayName = "Viaducto / Puente Elevado"),
	TunelSubterraneo   UMETA(DisplayName = "Tunel Bajo Montana")
};

/** Fases del tiempo de construccion de carreteras (activable/desactivable en ajustes) */
UENUM(BlueprintType)
enum class ERoadConstructionPhase : uint8
{
	AbiertaAlTrafico          UMETA(DisplayName = "Abierta al Tráfico (Completada)"),
	MovimientoTierras         UMETA(DisplayName = "Fase 1: Replanteo y Desmonte (40 km/h)"),
	ExtendidoAsfaltado        UMETA(DisplayName = "Fase 2: Asfaltado y Compactación (40 km/h)"),
	PinturaYBalizamiento      UMETA(DisplayName = "Fase 3: Pintado y Balizamiento (60 km/h)")
};

/** Parametros de configuracion temporal y fisica para las obras viales */
USTRUCT(BlueprintType)
struct FRoadConstructionParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Times")
	float Phase1DurationSeconds = 10.0f; // Replanteo y Desmonte

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Times")
	float Phase2DurationSeconds = 15.0f; // Asfaltado y Compactacion

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Times")
	float Phase3DurationSeconds = 8.0f;  // Pintado y Balizamiento

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Speed Limits")
	float Phase1SpeedLimitKmh = 40.0f; // Reduccion drastica a 40 km/h en desmonte

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Speed Limits")
	float Phase2SpeedLimitKmh = 40.0f; // 40 km/h junto a maquinaria pesada y asfalto

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Speed Limits")
	float Phase3SpeedLimitKmh = 60.0f; // 60 km/h en fase final de balizamiento

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Layout")
	float ConeSpacingCm = 700.0f; // Balizamiento con conos reflectantes cada 7 metros

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Layout")
	int32 ClosedLanesCount = 1; // Numero de carriles cerrados provisionalmente por obras
};

/** Parametros geometricos de la seccion transversal en Unreal Units (1 UU = 1 cm) */
USTRUCT(BlueprintType)
struct FRoadCrossSection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Geometry")
	int32 NumLanesDirection = 2; // Numero de carriles por sentido

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Geometry")
	float LaneWidth = 350.0f; // 3.50 metros segun Norma 3.1-IC

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Geometry")
	float OuterShoulderWidth = 250.0f; // Arcén exterior (2.50 metros)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Geometry")
	float InnerShoulderWidth = 100.0f; // Arcén interior (1.00 metro)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Geometry")
	float MedianWidth = 200.0f; // Mediana central (2.00 metros)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Geometry")
	float SpeedLimitKmh = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Geometry")
	bool bHasGuardrail = true; // Quitamiedos / Bionda con SPM

	// Calcula la anchura total de la calzada de un sentido (incluyendo arcenes)
	float GetOneWayCarriagewayWidth() const
	{
		return OuterShoulderWidth + (NumLanesDirection * LaneWidth) + InnerShoulderWidth;
	}
};
