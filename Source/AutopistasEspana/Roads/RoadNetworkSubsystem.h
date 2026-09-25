#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Roads/RoadTypes.h"
#include "RoadNetworkSubsystem.generated.h"

class URoadSplineComponent;
class ARoadSegmentActor;

/**
 * Subsistema Global de la Red de Carreteras y Enlaces Viales.
 * Administra el grafo vial completo, el registro de tramos, el enrutamiento topologico a nivel de carril
 * (Lane-Level Routing segun Norma 8.1-IC), la creacion de glorietas multicarril y la deteccion
 * automatica de pasos a distinto nivel (Flyovers / Overpasses) con verificacion de galibo MITMA (5.5m).
 */
UCLASS()
class AUTOPISTASESPANA_API URoadNetworkSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Galibo vertical libre minimo reglamentario segun MITMA (5.50 metros = 550 cm / UU)
	static constexpr float MITMA_MIN_OVERPASS_CLEARANCE_CM = 550.0f;

	// --- Gestion de Segmentos Viales ---

	// Registrar un nuevo segmento vial en el grafo del mundo
	UFUNCTION(BlueprintCallable, Category = "Road Network")
	void RegisterRoadSegment(URoadSplineComponent* RoadSegment);

	// Eliminar un segmento vial (demolicion)
	UFUNCTION(BlueprintCallable, Category = "Road Network")
	void UnregisterRoadSegment(URoadSplineComponent* RoadSegment);

	// Obtener el numero de tramos registrados
	UFUNCTION(BlueprintPure, Category = "Road Network")
	int32 GetTotalRoadSegmentsCount() const { return RegisteredRoads.Num(); }

	// Calcular longitud total de la red en kilometros
	UFUNCTION(BlueprintPure, Category = "Road Network")
	float GetTotalNetworkLengthKm() const;

	// Obtener lista de tramos registrados en la red
	const TArray<TWeakObjectPtr<URoadSplineComponent>>& GetRegisteredRoads() const { return RegisteredRoads; }

	// --- Registro y Gestion de Nodos de Enlace (Junction Nodes) ---

	// Registrar un nuevo nodo de enlace o interseccion en la red
	UFUNCTION(BlueprintCallable, Category = "Road Network|Junctions")
	void RegisterJunctionNode(const FRoadJunctionNode& JunctionNode);

	// Eliminar un nodo de enlace por su identificador unico
	UFUNCTION(BlueprintCallable, Category = "Road Network|Junctions")
	void UnregisterJunctionNode(const FGuid& JunctionId);

	// Obtener el numero total de nudos de enlace registrados
	UFUNCTION(BlueprintPure, Category = "Road Network|Junctions")
	int32 GetTotalJunctionsCount() const { return RegisteredJunctions.Num(); }

	// Obtener todos los nodos de enlace de la red
	UFUNCTION(BlueprintPure, Category = "Road Network|Junctions")
	const TArray<FRoadJunctionNode>& GetAllJunctionNodes() const { return RegisteredJunctions; }

	// Buscar un nodo de enlace por su ID
	UFUNCTION(BlueprintPure, Category = "Road Network|Junctions")
	bool FindJunctionNodeById(const FGuid& JunctionId, FRoadJunctionNode& OutNode) const;

	// Buscar nodo de enlace en una ubicacion de mundo dada dentro de un radio de tolerancia
	UFUNCTION(BlueprintPure, Category = "Road Network|Junctions")
	bool FindJunctionNodeAtLocation(const FVector& Location, float ToleranceCm, FRoadJunctionNode& OutNode) const;

	// Obtener lista de nodos de enlace asociados a una carretera
	UFUNCTION(BlueprintPure, Category = "Road Network|Junctions")
	TArray<FRoadJunctionNode> GetJunctionsForRoad(URoadSplineComponent* Road) const;

	// --- Algoritmo de Ruteo de Carril (Lane-Level Routing) ---

	/**
	 * Determina a que carril exacto de la via destino debe incorporarse un vehiculo proveniente de un ramal o calzada.
	 * Aplica la normativa de circulacion por la derecha (Norma 8.1-IC) y las conexiones del nudo de enlace.
	 * @param CurrentRoad Carretera / ramal emisor actual.
	 * @param CurrentLane Carril por el que circula el vehiculo (0 = carril derecho).
	 * @param NextRoad Carretera / autovia a la que se va a incorporar.
	 * @return Indice del carril de destino en NextRoad.
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Network|Lane Routing")
	int32 GetNextConnectedLane(URoadSplineComponent* CurrentRoad, int32 CurrentLane, URoadSplineComponent* NextRoad) const;

	/**
	 * Determina el carril de destino y devuelve la longitud de transicion requerida para la cuna lateral suave.
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Network|Lane Routing")
	int32 GetNextConnectedLaneWithTransition(URoadSplineComponent* CurrentRoad, int32 CurrentLane, URoadSplineComponent* NextRoad, float& OutTransitionLength) const;

	/**
	 * Obtiene los detalles completos de la conexion de carril si existe configurada en algun nudo de enlace.
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Network|Lane Routing")
	bool GetLaneConnectionDetails(URoadSplineComponent* CurrentRoad, int32 CurrentLane, URoadSplineComponent* NextRoad, FLaneConnection& OutConnection) const;

	// --- Soporte de Glorietas Multicarril ---

	/**
	 * Crea proceduralmente una glorieta multicarril completa segun Norma 8.1-IC.
	 * Genera un anillo giratorio de 2 carriles concéntricos (sentido antihorario) y 4 ramales radiales
	 * tangenciales (Norte, Este, Sur, Oeste) con todas sus conexiones de carril reglamentarias.
	 * @param CenterLocation Ubicacion en coordenadas de mundo del centro de la glorieta.
	 * @param RadiusCm Radio del eje del anillo giratorio (por defecto 4000 cm = 40m).
	 * @param RadialArmLengthCm Longitud de los 4 accesos radiales (por defecto 8000 cm = 80m).
	 * @return El nodo de enlace FRoadJunctionNode registrado con la configuracion del nudo.
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Network|Interchanges")
	FRoadJunctionNode CreateMultiLaneRoundabout(const FVector& CenterLocation, float RadiusCm = 4000.0f, float RadialArmLengthCm = 8000.0f);

	// --- Comprobacion de Pasos a Distinto Nivel (Flyovers / Overpasses) ---

	/**
	 * Evalua el cruce geometrico entre dos carreteras. Si se cruzan en planta y el desnivel vertical
	 * Delta Z >= 550 UU (5.5m de galibo libre MITMA), confirma que es un paso a distinto nivel,
	 * modela la via superior como viaducto sobre pilares y NO genera colision fisica a nivel.
	 * @param RoadA Primera carretera a comprobar.
	 * @param RoadB Segunda carretera a comprobar.
	 * @param OutCrossingLocation Ubicacion tridimensional del punto de cruce.
	 * @param OutDeltaZ Desnivel vertical libre en centimetros / UU.
	 * @param bOutIsGradeSeparated True si cumple galibo MITMA (Delta Z >= 550 UU).
	 * @return True si existe interseccion en la proyeccion horizontal (en planta).
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Network|Flyovers")
	bool CheckOverpassClearance(URoadSplineComponent* RoadA, URoadSplineComponent* RoadB, FVector& OutCrossingLocation, float& OutDeltaZ, bool& bOutIsGradeSeparated);

	/**
	 * Escanea toda la red vial en busca de cruces en planta entre carreteras registradas.
	 * Configura viaductos continuos sobre pilares para los pasos superiores que cumplan el galibo MITMA.
	 * @return Numero de pasos a distinto nivel confirmados.
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Network|Flyovers")
	int32 EvaluateRoadNetworkOverpasses();

	/**
	 * Evalua si un nuevo tramo de carretera cruza a distinto nivel con las carreteras preexistentes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Network|Flyovers")
	void EvaluateOverpassesForRoad(URoadSplineComponent* NewRoad);

	/**
	 * Detecta si dos splines viales se cruzan en planta (proyeccion XY 2D).
	 */
	UFUNCTION(BlueprintPure, Category = "Road Network|Geometry")
	bool DetectPlanCrossing(URoadSplineComponent* RoadA, URoadSplineComponent* RoadB, float& OutDistA, float& OutDistB, FVector& OutPointA, FVector& OutPointB) const;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<URoadSplineComponent>> RegisteredRoads;

	UPROPERTY()
	TArray<FRoadJunctionNode> RegisteredJunctions;
};
