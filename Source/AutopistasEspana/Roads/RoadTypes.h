#pragma once

#include "CoreMinimal.h"
#include "RoadTypes.generated.h"

class URoadSplineComponent;

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

/** Tipos de Enlaces, Intersecciones y Pasos Viales */
UENUM(BlueprintType)
enum class EJunctionType : uint8
{
	T_Junction         UMETA(DisplayName = "Interseccion en T"),
	Trumpet            UMETA(DisplayName = "Enlace tipo Trompeta"),
	Diamond            UMETA(DisplayName = "Enlace tipo Diamante"),
	Roundabout         UMETA(DisplayName = "Glorieta / Rotonda Multicarril"),
	DirectMerge        UMETA(DisplayName = "Incorporacion Directa (Cuna de Aceleracion)"),
	DirectDiverge      UMETA(DisplayName = "Salida Directa (Cuna de Deceleracion)"),
	OverpassFlyover    UMETA(DisplayName = "Paso a Distinto Nivel (Flyover / Viaducto Continuo)")
};

/** Fases del tiempo de construccion de carreteras (activable/desactivable en ajustes) */
UENUM(BlueprintType)
enum class ERoadConstructionPhase : uint8
{
	AbiertaAlTrafico          UMETA(DisplayName = "Abierta al Trafico (Completada)"),
	MovimientoTierras         UMETA(DisplayName = "Fase 1: Replanteo y Desmonte (40 km/h)"),
	ExtendidoAsfaltado        UMETA(DisplayName = "Fase 2: Asfaltado y Compactacion (40 km/h)"),
	PinturaYBalizamiento      UMETA(DisplayName = "Fase 3: Pintado y Balizamiento (60 km/h)")
};

/**
 * Conexion a nivel de carril entre dos carreteras segun Norma 8.1-IC.
 * Define el carril de origen, carril de destino, la via receptora y la longitud de transicion lateral.
 */
USTRUCT(BlueprintType)
struct FLaneConnection
{
	GENERATED_BODY()

	/** Indice del carril de origen en la via emisora (0 = carril derecho) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane Connection")
	int32 SourceLaneIndex = 0;

	/** Indice del carril de destino en la via receptora (0 = carril derecho) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane Connection")
	int32 TargetLaneIndex = 0;

	/** Carretera / Spline vial de destino */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane Connection")
	URoadSplineComponent* TargetRoad = nullptr;

	/** Longitud de transicion o cuna de aceleracion / deceleracion segun Norma 8.1-IC (en cm / UU, ej. 15000 a 20000 UU = 150-200m) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane Connection")
	float TransitionLength = 15000.0f;

	/** Indica si es incorporacion (merge) o salida (diverge) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane Connection")
	bool bIsMerge = true;

	/** Velocidad de diseno aconsejada en el ramal / enlace (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane Connection")
	float AdvisorySpeedKmh = 60.0f;

	FLaneConnection()
		: SourceLaneIndex(0)
		, TargetLaneIndex(0)
		, TargetRoad(nullptr)
		, TransitionLength(15000.0f)
		, bIsMerge(true)
		, AdvisorySpeedKmh(60.0f)
	{
	}

	FLaneConnection(int32 InSourceLane, int32 InTargetLane, URoadSplineComponent* InTargetRoad, float InTransitionLength = 15000.0f, bool bInMerge = true, float InAdvisorySpeed = 60.0f)
		: SourceLaneIndex(InSourceLane)
		, TargetLaneIndex(InTargetLane)
		, TargetRoad(InTargetRoad)
		, TransitionLength(InTransitionLength)
		, bIsMerge(bInMerge)
		, AdvisorySpeedKmh(InAdvisorySpeed)
	{
	}
};

/**
 * Nodo de Enlace / Interseccion Vial.
 * Agrupa las carreteras convergentes y las conexiones carril a carril validas,
 * permitiendo ruteo topologico exacto y prevencion de colisiones en pasos a distinto nivel.
 */
USTRUCT(BlueprintType)
struct FRoadJunctionNode
{
	GENERATED_BODY()

	/** Identificador unico del nodo de enlace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Junction")
	FGuid JunctionId;

	/** Denominacion tecnica o cartografica del enlace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Junction")
	FName JunctionName = NAME_None;

	/** Ubicacion tridimensional en coordenadas de mundo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Junction")
	FVector WorldLocation = FVector::ZeroVector;

	/** Tipologia del enlace vial */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Junction")
	EJunctionType JunctionType = EJunctionType::DirectMerge;

	/** Carreteras o ramales que confluyen en este nudo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Junction")
	TArray<URoadSplineComponent*> ConnectedRoads;

	/** Lista de conexiones de carril validas y reglamentarias */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Junction")
	TArray<FLaneConnection> LaneConnections;

	FRoadJunctionNode()
		: JunctionId(FGuid::NewGuid())
		, JunctionName(NAME_None)
		, WorldLocation(FVector::ZeroVector)
		, JunctionType(EJunctionType::DirectMerge)
	{
	}

	FRoadJunctionNode(const FVector& InLocation, EJunctionType InType, FName InName = NAME_None)
		: JunctionId(FGuid::NewGuid())
		, JunctionName(InName)
		, WorldLocation(InLocation)
		, JunctionType(InType)
	{
	}

	bool IsValid() const
	{
		return JunctionId.IsValid();
	}

	bool ContainsRoad(const URoadSplineComponent* Road) const
	{
		return ConnectedRoads.Contains(Road);
	}

	void AddRoad(URoadSplineComponent* Road)
	{
		if (Road && !ConnectedRoads.Contains(Road))
		{
			ConnectedRoads.Add(Road);
		}
	}

	void AddLaneConnection(int32 SourceLane, int32 TargetLane, URoadSplineComponent* TargetRoad, float TransitionLength = 15000.0f, bool bIsMerge = true, float AdvisorySpeed = 60.0f)
	{
		LaneConnections.Add(FLaneConnection(SourceLane, TargetLane, TargetRoad, TransitionLength, bIsMerge, AdvisorySpeed));
		AddRoad(TargetRoad);
	}
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
	float OuterShoulderWidth = 250.0f; // Arcen exterior (2.50 metros)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Geometry")
	float InnerShoulderWidth = 100.0f; // Arcen interior (1.00 metro)

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

	// Calcula la distancia lateral desde el eje del spline al centro del carril indicado (0 = carril derecho)
	float GetLaneCenterOffset(int32 LaneIndex) const
	{
		const float ClampedLane = FMath::Clamp(LaneIndex, 0, FMath::Max(0, NumLanesDirection - 1));
		return (static_cast<float>(ClampedLane) + 0.5f) * LaneWidth;
	}
};
