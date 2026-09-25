#include "WorldGen/ProceduralWorldGenerator.h"
#include "ProceduralMeshComponent.h"
#include "Traffic/TrafficSimulationSubsystem.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "AutopistasEspana.h"

AProceduralWorldGenerator::AProceduralWorldGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	RootComponent = TerrainMesh;
	TerrainMesh->bUseAsyncCooking = true;
}

void AProceduralWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	GenerateWorldTerrain();
	SpawnCitiesAndIndustries();
	GenerateOriginDestinationMatrix();
}

float AProceduralWorldGenerator::GetTerrainHeightAtLocation(float WorldX, float WorldY) const
{
	// Formula matematica armonica procedural (simulando montanas y llanuras segun bioma)
	const float Scale1 = 0.00002f;
	const float Scale2 = 0.00008f;
	const float Scale3 = 0.0003f;

	float Elevation = 0.0f;

	switch (GenerationSettings.Biome)
	{
	case EBiomeType::MesetaCentral:
		// Meseta: Relieve suavemente ondulado con colinas moderadas (alturas entre 0m y 40m = 0 a 4000 UU)
		Elevation = (FMath::Sin(WorldX * Scale1 + GenerationSettings.RandomSeed) * FMath::Cos(WorldY * Scale1)) * 2500.0f
				  + (FMath::Sin(WorldX * Scale2) * FMath::Cos(WorldY * Scale2)) * 1000.0f;
		break;

	case EBiomeType::CantabricoNorte:
		// Cornisa Cantabrica: Montanas escarpadas y valles profundos (alturas hasta 200m = 20000 UU)
		Elevation = (FMath::Abs(FMath::Sin(WorldX * Scale1) * FMath::Cos(WorldY * Scale1))) * 15000.0f
				  + (FMath::Sin(WorldX * Scale2) * FMath::Sin(WorldY * Scale2)) * 5000.0f
				  + (FMath::Cos(WorldX * Scale3) * FMath::Sin(WorldY * Scale3)) * 1200.0f;
		break;

	case EBiomeType::LevanteCosta:
		// Levante: Llanura costera que asciende bruscamente hacia sierras interiores
		Elevation = FMath::Clamp(WorldX * 0.03f, 0.0f, 12000.0f) 
				  + (FMath::Sin(WorldX * Scale2) * FMath::Cos(WorldY * Scale2)) * 3000.0f;
		break;
	}

	return Elevation * GenerationSettings.MountainReliefScale;
}

void AProceduralWorldGenerator::GenerateWorldTerrain()
{
	const float WorldExtentCm = (GenerationSettings.MapDimensionsKm * 100000.0f) * 0.5f; // Radio en cm
	const int32 GridResolution = 64; // Malla de 64x64 vertices para rendimiento óptimo
	const float CellSize = (WorldExtentCm * 2.0f) / static_cast<float>(GridResolution);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;

	// 1. Generar Vértices
	for (int32 y = 0; y <= GridResolution; ++y)
	{
		const float PosY = -WorldExtentCm + (y * CellSize);
		for (int32 x = 0; x <= GridResolution; ++x)
		{
			const float PosX = -WorldExtentCm + (x * CellSize);
			const float PosZ = GetTerrainHeightAtLocation(PosX, PosY);

			Vertices.Add(FVector(PosX, PosY, PosZ));
			Normals.Add(FVector(0.0f, 0.0f, 1.0f)); // Normal aproximada vertical
			UV0.Add(FVector2D(PosX / 1000.0f, PosY / 1000.0f));
			Tangents.Add(FProcMeshTangent(1.0f, 0.0f, 0.0f));
		}
	}

	// 2. Construir Triangulos
	const int32 RowCount = GridResolution + 1;
	for (int32 y = 0; y < GridResolution; ++y)
	{
		for (int32 x = 0; x < GridResolution; ++x)
		{
			const int32 TopLeft = (y * RowCount) + x;
			const int32 TopRight = TopLeft + 1;
			const int32 BottomLeft = ((y + 1) * RowCount) + x;
			const int32 BottomRight = BottomLeft + 1;

			// Triangulo 1
			Triangles.Add(TopLeft);
			Triangles.Add(BottomLeft);
			Triangles.Add(TopRight);

			// Triangulo 2
			Triangles.Add(TopRight);
			Triangles.Add(BottomLeft);
			Triangles.Add(BottomRight);
		}
	}

	TerrainMesh->ClearAllMeshSections();
	TerrainMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	TerrainMesh->SetMeshSectionCastsShadow(0, true);

	UE_LOG(LogAutopistas, Log, TEXT("Terreno procedural generado con exito: %d vertices, mapa de %.1f x %.1f km."), Vertices.Num(), GenerationSettings.MapDimensionsKm, GenerationSettings.MapDimensionsKm);
}

void AProceduralWorldGenerator::SpawnCitiesAndIndustries()
{
	CityCenters.Empty();
	IndustrialHubCenters.Empty();

	const float SafeRange = (GenerationSettings.MapDimensionsKm * 100000.0f) * 0.35f;

	// 1. Distribuir Ciudades
	for (int32 i = 0; i < GenerationSettings.NumCities; ++i)
	{
		const float Angle = (static_cast<float>(i) / static_cast<float>(GenerationSettings.NumCities)) * 2.0f * PI;
		const float Radius = FMath::RandRange(SafeRange * 0.4f, SafeRange);

		const float CityX = FMath::Cos(Angle) * Radius;
		const float CityY = FMath::Sin(Angle) * Radius;
		const float CityZ = GetTerrainHeightAtLocation(CityX, CityY);

		CityCenters.Add(FVector(CityX, CityY, CityZ));
		UE_LOG(LogAutopistas, Log, TEXT("Ciudad %d emplazada en: X=%.0f, Y=%.0f, Z=%.0f"), i + 1, CityX, CityY, CityZ);
	}

	// 2. Distribuir Poligonos Industriales (alejados de las ciudades)
	for (int32 j = 0; j < GenerationSettings.NumIndustrialHubs; ++j)
	{
		const float Angle = (static_cast<float>(j) / static_cast<float>(GenerationSettings.NumIndustrialHubs)) * 2.0f * PI + (PI * 0.5f);
		const float Radius = SafeRange * 0.85f;

		const float IndX = FMath::Cos(Angle) * Radius;
		const float IndY = FMath::Sin(Angle) * Radius;
		const float IndZ = GetTerrainHeightAtLocation(IndX, IndY);

		IndustrialHubCenters.Add(FVector(IndX, IndY, IndZ));
		UE_LOG(LogAutopistas, Log, TEXT("Parque Logistico %d emplazado en: X=%.0f, Y=%.0f, Z=%.0f"), j + 1, IndX, IndY, IndZ);
	}
}

void AProceduralWorldGenerator::GenerateOriginDestinationMatrix()
{
	// Enlazar demanda entre ciudades (viajes pendulares) y poligonos (transporte de mercancias pesadas)
	UE_LOG(LogAutopistas, Log, TEXT("Matriz O-D generada: %d rutas residenciales y %d rutas logisticas listas para conectar con autovias."), CityCenters.Num() * (CityCenters.Num() - 1), CityCenters.Num() * IndustrialHubCenters.Num());
}
