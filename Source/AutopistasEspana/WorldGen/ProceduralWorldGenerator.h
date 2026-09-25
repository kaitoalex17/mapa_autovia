#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGen/WorldGenTypes.h"
#include "ProceduralWorldGenerator.generated.h"

class UProceduralMeshComponent;
class UTrafficSimulationSubsystem;

/**
 * Generador Procedural del Mundo Sandbox, Terreno y Ciudades.
 * Modela la orografia de Espana segun el bioma seleccionado, distribuye los nucleos urbanos
 * y genera la demanda de viajes Origen-Destino (O-D) que alimenta el trafico de las autopistas.
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
	// Componente de Malla del Terreno
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	UProceduralMeshComponent* TerrainMesh;

	// Configuracion del mundo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation Settings")
	FWorldGenerationSettings GenerationSettings;

	// Generar el terreno procedural completo
	UFUNCTION(BlueprintCallable, Category = "World Generation")
	void GenerateWorldTerrain();

	// Distribuir ciudades, bloques de pisos y poligonos industriales
	UFUNCTION(BlueprintCallable, Category = "World Generation")
	void SpawnCitiesAndIndustries();

	// Generar matriz de viajes Origen -> Destino para el motor de trafico
	UFUNCTION(BlueprintCallable, Category = "World Generation")
	void GenerateOriginDestinationMatrix();

	// Obtener la altura del terreno Z en unas coordenadas X, Y determinadas
	UFUNCTION(BlueprintPure, Category = "World Generation")
	float GetTerrainHeightAtLocation(float WorldX, float WorldY) const;

private:
	// Coordenadas centrales de ciudades generadas
	TArray<FVector> CityCenters;
	TArray<FVector> IndustrialHubCenters;
};
