#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGen/WorldGenTypes.h"
#include "ProceduralWorldGenerator.generated.h"

class UProceduralMeshComponent;
class UTrafficSimulationSubsystem;
class URoadNetworkSubsystem;
class ARoadSegmentActor;
class URoadSplineComponent;
class ALevelCrossingActor;
class URailwayTrackComponent;
class ATrafficVehicleAgent;
class ATrainVehicleActor;

/**
 * Generador Procedural del Mundo, Orografia Espanola y Macroinfraestructuras.
 * Modela la orografia de Espana segun la region geografica seleccionada (Sierra de Guadarrama,
 * Despenaperros, Picos de Europa, Pirineos, Meseta Central), generando cordilleras y picos
 * escarpados de fondo a cotas de +1.500m a +2.600m en el horizonte.
 * 
 * Implementa la generacion automatica de la red viva inicial mediante SpawnInitialInfrastructure(),
 * desplegando la Autovia del Sur (A-4) de mas de 10 km, viaducto sobre barranco fluvial, tunel bitubo
 * central, estacion de peaje troncal, area de servicio 24h, cruce ferroviario ADIF, conexion entre
 * 'Madrid Metropolis Norte' y 'Poligono Logistico e Industrial Sur', y una flota de mas de 40 vehiculos.
 */
UCLASS()
class AUTOPISTASESPANA_API AProceduralWorldGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	AProceduralWorldGenerator();

protected:
	virtual void BeginPlay() override;

public:	
	// --- COMPONENTES DE MALLA PROCEDURAL ---

	/** Componente de Malla del Terreno Natural de Espana */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	UProceduralMeshComponent* TerrainMesh;

	/** Componente de Malla del Viaducto Elevado y Lecho Fluvial */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Infrastructure|Viaduct")
	UProceduralMeshComponent* ViaductMesh;

	/** Componente de Malla de las Bocas y Bovedas del Tunel Bitubo */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Infrastructure|Tunnel")
	UProceduralMeshComponent* TunnelMesh;

	/** Componente de Malla de la Estacion de Peaje Troncal */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Infrastructure|Toll")
	UProceduralMeshComponent* TollPlazaMesh;

	/** Componente de Malla del Area de Servicio, Gasolinera y Aparcamientos */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Infrastructure|ServiceArea")
	UProceduralMeshComponent* ServiceAreaMesh;

	/** Componente de Malla de los Nucleos Urbanos (Madrid Norte y Poligono Sur) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Infrastructure|UrbanHubs")
	UProceduralMeshComponent* UrbanHubsMesh;

	/** Spline de la linea ferrea ADIF que intersecta el mapa */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Infrastructure|Railway")
	URailwayTrackComponent* RailwayTrackSpline;

	/** Malla procedural de la superestructura ferroviaria (balasto, railes UIC 60, traviesas) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Infrastructure|Railway")
	UProceduralMeshComponent* RailwayTrackMesh;

	// --- CONFIGURACION Y ESTADO DEL MUNDO ---

	/** Configuracion de generacion orografica, limites y red preexistente */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation Settings")
	FWorldGenerationSettings GenerationSettings;

	/** Actor del corredor troncal principal de la Autovia del Sur (A-4) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Infrastructure|Corridor")
	ARoadSegmentActor* MainTrunkHighway = nullptr;

	/** Actor del Paso a Nivel en el cruce con la via ferrea ADIF */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Infrastructure|Railway")
	ALevelCrossingActor* ADIFLevelCrossing = nullptr;

	/** Lista de todos los actores de infraestructura y vehiculos generados */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Infrastructure")
	TArray<AActor*> SpawnedInfrastructureActors;

	// --- METODOS DE GENERACION PRINCIPALES ---

	/** Genera la malla del terreno procedural completo a 16x16 km segun la region geografica */
	UFUNCTION(BlueprintCallable, Category = "World Generation")
	void GenerateWorldTerrain();

	/** Distribuye ciudades secundarias, fincas y poligonos industriales en el mapa */
	UFUNCTION(BlueprintCallable, Category = "World Generation")
	void SpawnCitiesAndIndustries();

	/** Genera la matriz de demanda de viajes Origen-Destino (O-D) para la simulacion */
	UFUNCTION(BlueprintCallable, Category = "World Generation")
	void GenerateOriginDestinationMatrix();

	/** Despliega el corredor troncal A-4 y toda la infraestructura preexistente viva */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Infrastructure")
	void SpawnInitialInfrastructure();

	/** Obtiene la cota Z de elevacion del terreno en coordenadas X, Y */
	UFUNCTION(BlueprintPure, Category = "World Generation")
	float GetTerrainHeightAtLocation(float WorldX, float WorldY) const;

	/** Coordenadas centrales de Madrid Metropolis Norte */
	UFUNCTION(BlueprintPure, Category = "World Generation|UrbanHubs")
	FVector GetMadridMetropolisLocation() const { return FVector(0.0f, 560000.0f, 70000.0f); }

	/** Coordenadas centrales del Poligono Logistico e Industrial Sur */
	UFUNCTION(BlueprintPure, Category = "World Generation|UrbanHubs")
	FVector GetPoligonoIndustrialSurLocation() const { return FVector(0.0f, -560000.0f, 67500.0f); }

private:
	// Coordenadas centrales de ciudades y poligonos generados
	TArray<FVector> CityCenters;
	TArray<FVector> IndustrialHubCenters;

	// Submetodos de construccion de la infraestructura viva preexistente
	ARoadSegmentActor* BuildMainHighwayCorridor();
	void BuildRiverViaduct(ARoadSegmentActor* HighwayActor);
	void BuildTwinTubeTunnel(ARoadSegmentActor* HighwayActor);
	void BuildTollPlaza(ARoadSegmentActor* HighwayActor);
	void BuildServiceArea(ARoadSegmentActor* HighwayActor);
	void BuildADIFRailwayCrossing(ARoadSegmentActor* HighwayActor);
	void BuildUrbanHubs();
	void BuildInitialTrafficFleet(ARoadSegmentActor* HighwayActor);

	// Helpers geometricos estaticos para construccion procedural de mallas 3D
	static void AddBoxGeometry(
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UV0,
		const FVector& Center,
		const FVector& HalfSize);

	static void AddCylinderGeometry(
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UV0,
		const FVector& BaseCenter,
		float Radius,
		float Height,
		int32 RadialSegments = 16);
};
