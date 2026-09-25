#include "WorldGen/ProceduralWorldGenerator.h"
#include "ProceduralMeshComponent.h"
#include "Traffic/TrafficSimulationSubsystem.h"
#include "Traffic/TrafficVehicleAgent.h"
#include "Roads/RoadNetworkSubsystem.h"
#include "Roads/RoadSegmentActor.h"
#include "Roads/RoadSplineComponent.h"
#include "Railway/LevelCrossingActor.h"
#include "Railway/RailwayTrackComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "AutopistasEspana.h"

AProceduralWorldGenerator::AProceduralWorldGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. Malla del Terreno Base
	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	RootComponent = TerrainMesh;
	TerrainMesh->bUseAsyncCooking = true;

	// 2. Mallas de Infraestructura Integrada
	ViaductMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ViaductMesh"));
	ViaductMesh->SetupAttachment(RootComponent);
	ViaductMesh->bUseAsyncCooking = true;

	TunnelMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TunnelMesh"));
	TunnelMesh->SetupAttachment(RootComponent);
	TunnelMesh->bUseAsyncCooking = true;

	TollPlazaMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TollPlazaMesh"));
	TollPlazaMesh->SetupAttachment(RootComponent);
	TollPlazaMesh->bUseAsyncCooking = true;

	ServiceAreaMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ServiceAreaMesh"));
	ServiceAreaMesh->SetupAttachment(RootComponent);
	ServiceAreaMesh->bUseAsyncCooking = true;

	UrbanHubsMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("UrbanHubsMesh"));
	UrbanHubsMesh->SetupAttachment(RootComponent);
	UrbanHubsMesh->bUseAsyncCooking = true;

	// 3. Linea Ferrea ADIF
	RailwayTrackSpline = CreateDefaultSubobject<URailwayTrackComponent>(TEXT("RailwayTrackSpline"));
	RailwayTrackSpline->SetupAttachment(RootComponent);

	RailwayTrackMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RailwayTrackMesh"));
	RailwayTrackMesh->SetupAttachment(RailwayTrackSpline);
	RailwayTrackMesh->bUseAsyncCooking = true;
}

void AProceduralWorldGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateWorldTerrain();
	SpawnCitiesAndIndustries();
	GenerateOriginDestinationMatrix();

	if (GenerationSettings.bSpawnInitialInfrastructure)
	{
		SpawnInitialInfrastructure();
	}
}

float AProceduralWorldGenerator::GetTerrainHeightAtLocation(float WorldX, float WorldY) const
{
	const float ScaleValles = 0.000008f;
	const float ScaleMontanas = 0.000025f;
	const float ScalePicos = 0.000085f;
	const float ScaleRocas = 0.00035f;

	float ElevationCm = 0.0f;

	switch (GenerationSettings.GeographicRegion)
	{
	case ESpanishGeographicRegion::SierraGuadarrama_SistemaCentral:
		{
			// Cota base de meseta a +700m (70.000 UU)
			const float BaseMeseta = 70000.0f;

			// Cresta montanosa central orientada SW-NE (Pico Penalara +2.428m = 242.800 UU)
			const float DistToCentralRidge = FMath::Abs(WorldY - (WorldX * 0.25f));
			const float RidgeFactor = FMath::Clamp(1.0f - (DistToCentralRidge / 350000.0f), 0.0f, 1.0f);
			const float MountainPeaks = FMath::Pow(RidgeFactor, 2.2f) * 172800.0f;

			// Relieve granitico y collados (Navacerrada / Somosierra)
			const float GraniticRelief = (FMath::Sin(WorldX * ScaleMontanas) * FMath::Cos(WorldY * ScaleMontanas)) * 25000.0f
				                       + (FMath::Abs(FMath::Sin(WorldX * ScalePicos + GenerationSettings.RandomSeed))) * 12000.0f;

			// Barranco fluvial intermedio
			const float CanyonCut = (FMath::Abs(WorldY + 180000.0f) < 40000.0f) ? -35000.0f * (1.0f - (FMath::Abs(WorldY + 180000.0f) / 40000.0f)) : 0.0f;

			ElevationCm = BaseMeseta + MountainPeaks + GraniticRelief + CanyonCut;
		}
		break;

	case ESpanishGeographicRegion::Despenaperros_SierraMorena:
		{
			// Desfiladero escarpado de roca rojiza (+500m a +1.100m) con canones profundos
			const float BasePaso = 55000.0f;
			const float Canones = FMath::Abs(FMath::Sin(WorldX * ScaleMontanas * 1.5f)) * 55000.0f;
			const float GargantaFluvial = -40000.0f * FMath::Exp(-FMath::Square(WorldX / 120000.0f));
			ElevationCm = BasePaso + Canones + GargantaFluvial;
		}
		break;

	case ESpanishGeographicRegion::PicosDeEuropa_Cantabrico:
		{
			// Farallones calizos abruptos a +2.600m (Torre Cerredo) que se elevan desde el nivel del mar
			const float BaseCosta = 15000.0f;
			const float MacizoCalizo = FMath::Pow(FMath::Abs(FMath::Sin(WorldX * ScaleValles) * FMath::Cos(WorldY * ScaleValles)), 1.8f) * 245000.0f;
			const float Desfiladeros = (FMath::Sin(WorldX * ScalePicos) * FMath::Cos(WorldY * ScalePicos)) * 22000.0f;
			ElevationCm = BaseCosta + MacizoCalizo + Desfiladeros;
		}
		break;

	case ESpanishGeographicRegion::Pirineos_ValleAran:
		{
			// Cumbres glaciares a +3.404m (Aneto), circos glaciares y valles en U
			const float BaseValle = 85000.0f;
			const float AltaMontana = (FMath::Abs(FMath::Sin(WorldX * ScaleValles) + FMath::Cos(WorldY * ScaleValles))) * 180000.0f;
			ElevationCm = BaseValle + AltaMontana;
		}
		break;

	case ESpanishGeographicRegion::MesetaCentral_ValleTajo:
	default:
		{
			// Llanura infinita de cereal a +650m con suaves parameras y escarpes de vega
			const float BaseLlanura = 65000.0f;
			const float OrografiaSuave = (FMath::Sin(WorldX * ScaleValles * 0.5f) * FMath::Cos(WorldY * ScaleValles * 0.5f)) * 12000.0f
				                       + (FMath::Sin(WorldX * ScalePicos) * FMath::Cos(WorldY * ScalePicos)) * 3500.0f;
			ElevationCm = BaseLlanura + OrografiaSuave;
		}
		break;
	}

	return ElevationCm * GenerationSettings.MountainReliefScale;
}

void AProceduralWorldGenerator::GenerateWorldTerrain()
{
	const float WorldExtentCm = (GenerationSettings.MapDimensionsKm * 100000.0f) * 0.5f; // 800.000 UU para mapa de 16x16 km
	const int32 GridResolution = 128; // Malla de 128x128 vertices (celdas de 125m) para altisimo detalle
	const float CellSize = (WorldExtentCm * 2.0f) / static_cast<float>(GridResolution);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;

	Vertices.Reserve((GridResolution + 1) * (GridResolution + 1));
	Triangles.Reserve(GridResolution * GridResolution * 6);
	Normals.Reserve(Vertices.Num());
	UV0.Reserve(Vertices.Num());

	for (int32 Y = 0; Y <= GridResolution; ++Y)
	{
		const float WorldY = -WorldExtentCm + (Y * CellSize);
		for (int32 X = 0; X <= GridResolution; ++X)
		{
			const float WorldX = -WorldExtentCm + (X * CellSize);
			const float WorldZ = GetTerrainHeightAtLocation(WorldX, WorldY);

			Vertices.Add(FVector(WorldX, WorldY, WorldZ));
			Normals.Add(FVector::UpVector);
			UV0.Add(FVector2D(static_cast<float>(X) / GridResolution * 64.0f, static_cast<float>(Y) / GridResolution * 64.0f));
			Tangents.Add(FProcMeshTangent(FVector::ForwardVector, false));
		}
	}

	for (int32 Y = 0; Y < GridResolution; ++Y)
	{
		for (int32 X = 0; X < GridResolution; ++X)
		{
			const int32 TopLeft = Y * (GridResolution + 1) + X;
			const int32 TopRight = TopLeft + 1;
			const int32 BottomLeft = (Y + 1) * (GridResolution + 1) + X;
			const int32 BottomRight = BottomLeft + 1;

			Triangles.Add(TopLeft);
			Triangles.Add(BottomLeft);
			Triangles.Add(TopRight);

			Triangles.Add(TopRight);
			Triangles.Add(BottomLeft);
			Triangles.Add(BottomRight);
		}
	}

	TerrainMesh->ClearAllMeshSections();
	TerrainMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	TerrainMesh->SetMeshSectionCastsShadow(0, true);

	UE_LOG(LogAutopistas, Log, TEXT("Orografia espanola generada con exito: %d vertices, mapa de %.1f x %.1f km (Region: %d)."),
		Vertices.Num(), GenerationSettings.MapDimensionsKm, GenerationSettings.MapDimensionsKm, static_cast<int32>(GenerationSettings.GeographicRegion));
}

void AProceduralWorldGenerator::SpawnCitiesAndIndustries()
{
	CityCenters.Empty();
	IndustrialHubCenters.Empty();

	const float SafeRange = (GenerationSettings.MapDimensionsKm * 100000.0f) * 0.35f;

	for (int32 i = 0; i < GenerationSettings.NumCities; ++i)
	{
		const float Angle = (static_cast<float>(i) / static_cast<float>(GenerationSettings.NumCities)) * 2.0f * PI;
		const float Radius = FMath::RandRange(SafeRange * 0.4f, SafeRange);
		const float CityX = FMath::Cos(Angle) * Radius;
		const float CityY = FMath::Sin(Angle) * Radius;
		const float CityZ = GetTerrainHeightAtLocation(CityX, CityY);
		CityCenters.Add(FVector(CityX, CityY, CityZ));
	}

	for (int32 j = 0; j < GenerationSettings.NumIndustrialHubs; ++j)
	{
		const float Angle = (static_cast<float>(j) / static_cast<float>(GenerationSettings.NumIndustrialHubs)) * 2.0f * PI + (PI * 0.5f);
		const float Radius = SafeRange * 0.85f;
		const float IndX = FMath::Cos(Angle) * Radius;
		const float IndY = FMath::Sin(Angle) * Radius;
		const float IndZ = GetTerrainHeightAtLocation(IndX, IndY);
		IndustrialHubCenters.Add(FVector(IndX, IndY, IndZ));
	}
}

void AProceduralWorldGenerator::GenerateOriginDestinationMatrix()
{
	UE_LOG(LogAutopistas, Log, TEXT("Matriz O-D generada: %d rutas residenciales y %d rutas logisticas listas para conectar con autovias."),
		CityCenters.Num() * (CityCenters.Num() - 1), CityCenters.Num() * IndustrialHubCenters.Num());
}

void AProceduralWorldGenerator::SpawnInitialInfrastructure()
{
	UE_LOG(LogAutopistas, Log, TEXT("Iniciando despliegue de la macroinfraestructura viva preexistente de Espana..."));

	// 1. Desplegar Corredor Troncal A-4 (Autovia del Sur de mas de 10 km)
	MainTrunkHighway = BuildMainHighwayCorridor();

	// 2. Viaducto sobre barranco fluvial
	BuildRiverViaduct(MainTrunkHighway);

	// 3. Tunel bitubo a traves del macizo montanoso central
	BuildTwinTubeTunnel(MainTrunkHighway);

	// 4. Estacion de Peaje Troncal con cabinas Via-T
	BuildTollPlaza(MainTrunkHighway);

	// 5. Area de Servicio y descanso con gasolinera 24h
	BuildServiceArea(MainTrunkHighway);

	// 6. Cruce con linea ferrea ADIF y Paso a Nivel Clase C
	BuildADIFRailwayCrossing(MainTrunkHighway);

	// 7. Nucleos urbanos principales (Madrid Norte y Poligono Logistico Sur)
	BuildUrbanHubs();

	// 8. Inyeccion de flota inicial activa de mas de 40 vehiculos en circulacion
	BuildInitialTrafficFleet(MainTrunkHighway);

	UE_LOG(LogAutopistas, Log, TEXT("Macroinfraestructura viva inicial desplegada con exito. El mundo se encuentra activo."));
}

ARoadSegmentActor* AProceduralWorldGenerator::BuildMainHighwayCorridor()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARoadSegmentActor* HighwayActor = World->SpawnActor<ARoadSegmentActor>(ARoadSegmentActor::StaticClass(), FVector(0.0f, 550000.0f, 70000.0f), FRotator::ZeroRotator, SpawnParams);
	if (!HighwayActor || !HighwayActor->RoadSpline)
	{
		return nullptr;
	}

	URoadSplineComponent* Spline = HighwayActor->RoadSpline;
	Spline->RoadCategory = ERoadCategory::Autovia_120;
	Spline->CrossSection.NumLanesDirection = 2; // Autovia 2x2
	Spline->CrossSection.LaneWidth = 350.0f;    // 3.50 m segun Norma 3.1-IC
	Spline->CrossSection.OuterShoulderWidth = 250.0f; // Arcen 2.50 m
	Spline->CrossSection.InnerShoulderWidth = 100.0f; // Arcen int 1.00 m
	Spline->CrossSection.MedianWidth = 200.0f;        // Mediana 2.00 m
	Spline->CrossSection.SpeedLimitKmh = 120.0f;
	Spline->CrossSection.bHasGuardrail = true;
	Spline->ClearSplinePoints();

	// Trazar el corredor troncal de mas de 10 km (11.35 km reales) de Norte a Sur
	const TArray<FVector> CorridorPoints = {
		FVector(0.0f, 560000.0f, 70000.0f),      // PK 0+000: Madrid Metropolis Norte
		FVector(-12000.0f, 480000.0f, 71500.0f),  // PK 0+800: Transicion a campo abierto
		FVector(-25000.0f, 380000.0f, 72000.0f),  // PK 1+800: Inicio Viaducto sobre Barranco
		FVector(-15000.0f, 260000.0f, 72000.0f),  // PK 3+000: Fin Viaducto
		FVector(0.0f, 150000.0f, 73500.0f),       // PK 4+100: Estacion de Peaje Troncal
		FVector(18000.0f, 60000.0f, 74000.0f),    // PK 5+000: Aproximacion al macizo
		FVector(25000.0f, -40000.0f, 74500.0f),   // PK 6+000: Boca Norte Tunel Bitubo
		FVector(20000.0f, -160000.0f, 74500.0f),  // PK 7+200: Boca Sur Tunel Bitubo
		FVector(10000.0f, -250000.0f, 71000.0f),  // PK 8+100: Area de Servicio 24h
		FVector(0.0f, -370000.0f, 69000.0f),      // PK 9+300: Cruce Ferroviario ADIF
		FVector(-8000.0f, -470000.0f, 68000.0f),  // PK 10+300: Aproximacion Logistica
		FVector(0.0f, -560000.0f, 67500.0f)       // PK 11+200: Poligono Logistico Sur
	};

	for (const FVector& Pt : CorridorPoints)
	{
		Spline->AddSplinePoint(Pt, ESplineCoordinateSpace::World);
	}

	HighwayActor->CompleteConstruction(); // Autovia abierta y plenamente operativa
	HighwayActor->RebuildRoadGeometry();

	SpawnedInfrastructureActors.Add(HighwayActor);
	UE_LOG(LogAutopistas, Log, TEXT("Corredor troncal A-4 trazado con exito. Longitud total: %.2f km."), Spline->GetSplineLength() / 100000.0f);
	return HighwayActor;
}

void AProceduralWorldGenerator::BuildRiverViaduct(ARoadSegmentActor* HighwayActor)
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;

	// Generar 5 pilas de hormigon armado sobre el barranco fluvial (entre Y=380000 y Y=260000)
	const float ViaductZ = 72000.0f;
	const float RiverBedZ = 38000.0f; // Cota del fondo del barranco
	const float PillarHeight = ViaductZ - RiverBedZ;

	const float PillarYs[] = { 360000.0f, 335000.0f, 310000.0f, 285000.0f };
	for (float PillarY : PillarYs)
	{
		const FVector Center(-20000.0f, PillarY, RiverBedZ + (PillarHeight * 0.5f));
		AddBoxGeometry(Vertices, Triangles, Normals, UV0, Center, FVector(200.0f, 350.0f, PillarHeight * 0.5f));
	}

	ViaductMesh->ClearAllMeshSections();
	ViaductMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	ViaductMesh->SetMeshSectionCastsShadow(0, true);
	UE_LOG(LogAutopistas, Log, TEXT("Viaducto sobre barranco fluvial generado con %d pilas."), UE_ARRAY_COUNT(PillarYs));
}

void AProceduralWorldGenerator::BuildTwinTubeTunnel(ARoadSegmentActor* HighwayActor)
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;

	// Bocas de emboquille monumentales de hormigon en Y=-40000 y Y=-160000
	const FVector Portales[] = {
		FVector(25000.0f, -40000.0f, 74500.0f),  // Boca Norte
		FVector(20000.0f, -160000.0f, 74500.0f)  // Boca Sur
	};

	for (const FVector& Portal : Portales)
	{
		// Emboquille tubo calzada 1 (izquierdo)
		AddBoxGeometry(Vertices, Triangles, Normals, UV0, Portal + FVector(-600.0f, 0.0f, 400.0f), FVector(550.0f, 300.0f, 450.0f));
		// Emboquille tubo calzada 2 (derecho)
		AddBoxGeometry(Vertices, Triangles, Normals, UV0, Portal + FVector(600.0f, 0.0f, 400.0f), FVector(550.0f, 300.0f, 450.0f));
	}

	TunnelMesh->ClearAllMeshSections();
	TunnelMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	TunnelMesh->SetMeshSectionCastsShadow(0, true);
	UE_LOG(LogAutopistas, Log, TEXT("Tunel bitubo central de 1.200m modelado con bocas de emboquille Norte y Sur."));
}

void AProceduralWorldGenerator::BuildTollPlaza(ARoadSegmentActor* HighwayActor)
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;

	const FVector TollCenter(0.0f, 150000.0f, 73500.0f);

	// Marquesina principal de peaje (60m x 25m x 6m de altura libre)
	AddBoxGeometry(Vertices, Triangles, Normals, UV0, TollCenter + FVector(0.0f, 0.0f, 750.0f), FVector(3000.0f, 1250.0f, 60.0f));

	// 6 Cabinas de cobro y telepeaje Via-T
	for (int32 i = -3; i <= 3; ++i)
	{
		if (i == 0) continue;
		const FVector CabinPos = TollCenter + FVector(i * 750.0f, 0.0f, 150.0f);
		AddBoxGeometry(Vertices, Triangles, Normals, UV0, CabinPos, FVector(120.0f, 300.0f, 150.0f));
	}

	TollPlazaMesh->ClearAllMeshSections();
	TollPlazaMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	TollPlazaMesh->SetMeshSectionCastsShadow(0, true);
	UE_LOG(LogAutopistas, Log, TEXT("Estacion de Peaje Troncal generada en PK 4+100 con 6 vias de cobro Via-T."));
}

void AProceduralWorldGenerator::BuildServiceArea(ARoadSegmentActor* HighwayActor)
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;

	const FVector AreaCenter(10000.0f, -250000.0f, 71000.0f);

	// Edificio central cafetería / tienda / aseos (40m x 20m)
	AddBoxGeometry(Vertices, Triangles, Normals, UV0, AreaCenter + FVector(2500.0f, 0.0f, 250.0f), FVector(2000.0f, 1000.0f, 250.0f));

	// Marquesina de surtidores de combustible (gasolinera 24h)
	AddBoxGeometry(Vertices, Triangles, Normals, UV0, AreaCenter + FVector(1200.0f, 0.0f, 500.0f), FVector(800.0f, 1800.0f, 50.0f));

	ServiceAreaMesh->ClearAllMeshSections();
	ServiceAreaMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	ServiceAreaMesh->SetMeshSectionCastsShadow(0, true);
	UE_LOG(LogAutopistas, Log, TEXT("Area de Servicio 24h y Gasolinera desplegada en PK 8+100."));
}

void AProceduralWorldGenerator::BuildADIFRailwayCrossing(ARoadSegmentActor* HighwayActor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector CrossingLoc(0.0f, -370000.0f, 69000.0f);

	// 1. Configurar trazado de la via ferrea ADIF perpendicular a la autovia (orientacion Este-Oeste)
	if (RailwayTrackSpline)
	{
		RailwayTrackSpline->ClearSplinePoints();
		RailwayTrackSpline->AddSplinePoint(CrossingLoc + FVector(-350000.0f, 0.0f, 0.0f), ESplineCoordinateSpace::World);
		RailwayTrackSpline->AddSplinePoint(CrossingLoc, ESplineCoordinateSpace::World);
		RailwayTrackSpline->AddSplinePoint(CrossingLoc + FVector(350000.0f, 0.0f, 0.0f), ESplineCoordinateSpace::World);
		RailwayTrackSpline->GenerateTrackMesh(RailwayTrackMesh);
	}

	// 2. Spawnear Actor del Paso a Nivel Clase C
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADIFLevelCrossing = World->SpawnActor<ALevelCrossingActor>(ALevelCrossingActor::StaticClass(), CrossingLoc, FRotator::ZeroRotator, SpawnParams);
	if (ADIFLevelCrossing && HighwayActor)
	{
		ADIFLevelCrossing->AssociatedRoad = HighwayActor->RoadSpline;
		ADIFLevelCrossing->CrossingDistanceOnRoad = 930000.0f; // PK 9+300
		SpawnedInfrastructureActors.Add(ADIFLevelCrossing);
	}

	UE_LOG(LogAutopistas, Log, TEXT("Cruce Ferroviario ADIF y Paso a Nivel operativo enlazados en PK 9+300."));
}

void AProceduralWorldGenerator::BuildUrbanHubs()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;

	// 1. Madrid Metropolis Norte: Bloques de oficinas y edificios residenciales
	const FVector MadridLoc = GetMadridMetropolisLocation();
	for (int32 x = -2; x <= 2; ++x)
	{
		for (int32 y = 0; y <= 3; ++y)
		{
			const FVector BuildingCenter = MadridLoc + FVector(x * 6000.0f, y * 6000.0f + 15000.0f, 1500.0f);
			const float BuildingHeight = FMath::RandRange(1200.0f, 3500.0f);
			AddBoxGeometry(Vertices, Triangles, Normals, UV0, BuildingCenter, FVector(1800.0f, 1800.0f, BuildingHeight));
		}
	}

	// 2. Poligono Logistico e Industrial Sur: Naves logisticas modulares alargadas
	const FVector PoligonoLoc = GetPoligonoIndustrialSurLocation();
	for (int32 x = -3; x <= 3; ++x)
	{
		for (int32 y = -3; y <= 0; ++y)
		{
			const FVector WarehouseCenter = PoligonoLoc + FVector(x * 7500.0f, y * 7000.0f - 15000.0f, 400.0f);
			AddBoxGeometry(Vertices, Triangles, Normals, UV0, WarehouseCenter, FVector(2800.0f, 1800.0f, 400.0f));
		}
	}

	UrbanHubsMesh->ClearAllMeshSections();
	UrbanHubsMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	UrbanHubsMesh->SetMeshSectionCastsShadow(0, true);
	UE_LOG(LogAutopistas, Log, TEXT("Nucleos urbanos 'Madrid Norte' y 'Poligono Industrial Sur' modelados proceduralmente."));
}

void AProceduralWorldGenerator::BuildInitialTrafficFleet(ARoadSegmentActor* HighwayActor)
{
	if (!HighwayActor || !HighwayActor->RoadSpline)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UTrafficSimulationSubsystem* TrafficSys = World->GetSubsystem<UTrafficSimulationSubsystem>();
	if (!TrafficSys)
	{
		return;
	}

	URoadSplineComponent* Spline = HighwayActor->RoadSpline;
	const float SplineLen = Spline->GetSplineLength();
	const int32 TotalInitialVehicles = 44; // Mas de 40 vehiculos para trafico vivo inicial

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < TotalInitialVehicles; ++i)
	{
		// Distribuir longitudinalmente a lo largo de los 11.35 km
		const float StartDist = (static_cast<float>(i) / static_cast<float>(TotalInitialVehicles)) * (SplineLen - 20000.0f) + 10000.0f;
		const int32 LaneIndex = (i % 2); // Alternar entre carril derecho (0) y carril izquierdo (1)

		ATrafficVehicleAgent* Agent = TrafficSys->SpawnVehicleOnRoad(ATrafficVehicleAgent::StaticClass(), Spline, StartDist, LaneIndex);
		if (Agent)
		{
			// Asignar categoria tipologica realista segun distribucion de trafico espanol
			if (i % 8 == 0)
			{
				Agent->VehicleCategory = EVehicleCategory::CamionTrailer; // Trailer articulado 16.5m
				Agent->IDMParams.DesiredSpeedKmh = 90.0f;
			}
			else if (i % 12 == 1)
			{
				Agent->VehicleCategory = EVehicleCategory::GuardiaCivil; // Patrulla de vigilancia DGT
				Agent->IDMParams.DesiredSpeedKmh = 125.0f;
			}
			else if (i % 6 == 2)
			{
				Agent->VehicleCategory = EVehicleCategory::Furgoneta; // Reparto y paqueteria
				Agent->IDMParams.DesiredSpeedKmh = 100.0f;
			}
			else
			{
				Agent->VehicleCategory = EVehicleCategory::Turismo; // Turismo estandar
				Agent->IDMParams.DesiredSpeedKmh = FMath::RandRange(110.0f, 130.0f);
			}

			SpawnedInfrastructureActors.Add(Agent);
		}
	}

	UE_LOG(LogAutopistas, Log, TEXT("Flota inicial viva de %d vehiculos inyectada en la Autovia A-4. Simulacion IDM activa."), TotalInitialVehicles);
}

void AProceduralWorldGenerator::AddBoxGeometry(
	TArray<FVector>& Vertices,
	TArray<int32>& Triangles,
	TArray<FVector>& Normals,
	TArray<FVector2D>& UV0,
	const FVector& Center,
	const FVector& HalfSize)
{
	const int32 BaseIndex = Vertices.Num();

	const FVector Corners[8] = {
		Center + FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		Center + FVector( HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		Center + FVector( HalfSize.X,  HalfSize.Y, -HalfSize.Z),
		Center + FVector(-HalfSize.X,  HalfSize.Y, -HalfSize.Z),
		Center + FVector(-HalfSize.X, -HalfSize.Y,  HalfSize.Z),
		Center + FVector( HalfSize.X, -HalfSize.Y,  HalfSize.Z),
		Center + FVector( HalfSize.X,  HalfSize.Y,  HalfSize.Z),
		Center + FVector(-HalfSize.X,  HalfSize.Y,  HalfSize.Z)
	};

	const int32 Faces[6][4] = {
		{ 0, 1, 5, 4 }, // Front
		{ 1, 2, 6, 5 }, // Right
		{ 2, 3, 7, 6 }, // Back
		{ 3, 0, 4, 7 }, // Left
		{ 4, 5, 6, 7 }, // Top
		{ 3, 2, 1, 0 }  // Bottom
	};

	const FVector FaceNormals[6] = {
		FVector(0.0f, -1.0f, 0.0f),
		FVector(1.0f, 0.0f, 0.0f),
		FVector(0.0f, 1.0f, 0.0f),
		FVector(-1.0f, 0.0f, 0.0f),
		FVector(0.0f, 0.0f, 1.0f),
		FVector(0.0f, 0.0f, -1.0f)
	};

	for (int32 f = 0; f < 6; ++f)
	{
		const int32 FaceBase = Vertices.Num();
		Vertices.Add(Corners[Faces[f][0]]);
		Vertices.Add(Corners[Faces[f][1]]);
		Vertices.Add(Corners[Faces[f][2]]);
		Vertices.Add(Corners[Faces[f][3]]);

		for (int32 v = 0; v < 4; ++v)
		{
			Normals.Add(FaceNormals[f]);
		}

		UV0.Add(FVector2D(0.0f, 0.0f));
		UV0.Add(FVector2D(1.0f, 0.0f));
		UV0.Add(FVector2D(1.0f, 1.0f));
		UV0.Add(FVector2D(0.0f, 1.0f));

		Triangles.Add(FaceBase + 0);
		Triangles.Add(FaceBase + 1);
		Triangles.Add(FaceBase + 2);
		Triangles.Add(FaceBase + 0);
		Triangles.Add(FaceBase + 2);
		Triangles.Add(FaceBase + 3);
	}
}

void AProceduralWorldGenerator::AddCylinderGeometry(
	TArray<FVector>& Vertices,
	TArray<int32>& Triangles,
	TArray<FVector>& Normals,
	TArray<FVector2D>& UV0,
	const FVector& BaseCenter,
	float Radius,
	float Height,
	int32 RadialSegments)
{
	const int32 BaseIndex = Vertices.Num();
	const float AngleStep = (2.0f * PI) / static_cast<float>(RadialSegments);

	for (int32 i = 0; i <= RadialSegments; ++i)
	{
		const float Angle = i * AngleStep;
		const float CosA = FMath::Cos(Angle);
		const float SinA = FMath::Sin(Angle);

		const FVector BottomPos = BaseCenter + FVector(CosA * Radius, SinA * Radius, 0.0f);
		const FVector TopPos = BaseCenter + FVector(CosA * Radius, SinA * Radius, Height);
		const FVector RadialNormal(CosA, SinA, 0.0f);

		Vertices.Add(BottomPos);
		Normals.Add(RadialNormal);
		UV0.Add(FVector2D(static_cast<float>(i) / RadialSegments, 0.0f));

		Vertices.Add(TopPos);
		Normals.Add(RadialNormal);
		UV0.Add(FVector2D(static_cast<float>(i) / RadialSegments, 1.0f));
	}

	for (int32 i = 0; i < RadialSegments; ++i)
	{
		const int32 V0 = BaseIndex + (i * 2);
		const int32 V1 = V0 + 1;
		const int32 V2 = V0 + 2;
		const int32 V3 = V0 + 3;

		Triangles.Add(V0);
		Triangles.Add(V1);
		Triangles.Add(V2);

		Triangles.Add(V2);
		Triangles.Add(V1);
		Triangles.Add(V3);
	}
}
