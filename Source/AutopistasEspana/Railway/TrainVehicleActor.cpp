#include "Railway/TrainVehicleActor.h"
#include "Railway/RailwayTrackComponent.h"
#include "Railway/LevelCrossingActor.h"
#include "Traffic/TrafficVehicleAgent.h"
#include "Traffic/TrafficSimulationSubsystem.h"
#include "Traffic/TrafficTypes.h"
#include "Economy/EconomySubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "AutopistasEspana.h"

ATrainVehicleActor::ATrainVehicleActor()
{
	PrimaryActorTick.bCanEverTick = true;

	TrainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TrainRoot"));
	RootComponent = TrainRoot;

	LocomotiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LocomotiveMesh"));
	LocomotiveMesh->SetupAttachment(TrainRoot);
	LocomotiveMesh->SetCollisionProfileName(TEXT("Vehicle"));
	LocomotiveMesh->SetGenerateOverlapEvents(true);

	// Dimensiones iniciales del convoy
	TrainType = ETrainType::MercanciasContenedores;
	WagonCount = 5;
	WagonLengthCm = 1400.0f; // 14 metros por vagon
	WagonCouplingSpacingCm = 150.0f;
	TrainMassTons = 1200.0f;

	TargetCruiseSpeedKmh = 100.0f;
	MaxSpeedKmh = 120.0f;
	AccelerationCmS2 = 60.0f;
	ServiceBrakingDecelerationCmS2 = 90.0f;

	HighwaySweepRadiusCm = 6000.0f; // 60 metros de radio de absorcion intermodal
	bAutoDecongestHighway = true;
}

void ATrainVehicleActor::BeginPlay()
{
	Super::BeginPlay();

	// Convertir velocidad de crucero a cm/s
	CurrentSpeedCmS = (TargetCruiseSpeedKmh * 0.75f) * (100000.0f / 3600.0f);
	CurrentSpeedKmh = CurrentSpeedCmS * (3600.0f / 100000.0f);

	// Inicializar componentes de vagones en cascada
	InitializeWagons();

	// Buscar pasos a nivel en el mapa para el registro de enclavamientos
	TArray<AActor*> FoundCrossings;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALevelCrossingActor::StaticClass(), FoundCrossings);
	for (AActor* Actor : FoundCrossings)
	{
		if (ALevelCrossingActor* Crossing = Cast<ALevelCrossingActor>(Actor))
		{
			MonitoredCrossings.Add(Crossing);
		}
	}

	UE_LOG(LogAutopistas, Log, TEXT("Convoy ferroviario [%s] iniciado con %d vagones y %d pasos a nivel supervisados."), *GetName(), WagonCount, MonitoredCrossings.Num());
}

void ATrainVehicleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Cinemática y avance de locomotora y vagones por el spline
	UpdateTrainKinematics(DeltaTime);

	// 2. Comunicacion predictiva con pasos a nivel
	MonitorLevelCrossings(DeltaTime);

	// 3. Sistema de Autopista Ferroviaria: descongestion de camiones y coches de la autovia
	TickHighwayDecongestion(DeltaTime);
}

void ATrainVehicleActor::InitializeWagons()
{
	WagonMeshes.Empty();

	// Instanciamos los componentes visuales para cada vagon acoplado
	for (int32 i = 0; i < WagonCount; ++i)
	{
		const FName WagonCompName = *FString::Printf(TEXT("WagonMesh_%d"), i + 1);
		UStaticMeshComponent* NewWagon = NewObject<UStaticMeshComponent>(this, WagonCompName);
		if (NewWagon)
		{
			NewWagon->RegisterComponent();
			NewWagon->AttachToComponent(TrainRoot, FAttachmentTransformRules::KeepWorldTransform);
			NewWagon->SetCollisionProfileName(TEXT("Vehicle"));
			NewWagon->SetGenerateOverlapEvents(true);
			WagonMeshes.Add(NewWagon);
		}
	}
}

float ATrainVehicleActor::GetTotalTrainLengthCm() const
{
	const float LocomotiveLengthCm = 1800.0f; // 18 metros
	return LocomotiveLengthCm + (WagonCount * (WagonLengthCm + WagonCouplingSpacingCm));
}

void ATrainVehicleActor::UpdateTrainKinematics(float DeltaTime)
{
	if (!CurrentTrackSpline.IsValid())
	{
		return;
	}

	// Inercia ferroviaria: aceleracion suave hacia la velocidad objetivo
	const float TargetSpeedCmS = FMath::Clamp(TargetCruiseSpeedKmh, 0.0f, MaxSpeedKmh) * (100000.0f / 3600.0f);
	if (CurrentSpeedCmS < TargetSpeedCmS)
	{
		CurrentSpeedCmS = FMath::Min(CurrentSpeedCmS + (AccelerationCmS2 * DeltaTime), TargetSpeedCmS);
	}
	else if (CurrentSpeedCmS > TargetSpeedCmS)
	{
		CurrentSpeedCmS = FMath::Max(CurrentSpeedCmS - (ServiceBrakingDecelerationCmS2 * DeltaTime), TargetSpeedCmS);
	}

	CurrentSpeedKmh = CurrentSpeedCmS * (3600.0f / 100000.0f);

	// Avance de la cabeza tractora sobre el spline
	DistanceAlongSpline += CurrentSpeedCmS * DeltaTime;
	const float SplineLength = CurrentTrackSpline->GetSplineLength();

	if (DistanceAlongSpline >= SplineLength)
	{
		DistanceAlongSpline = FMath::Fmod(DistanceAlongSpline, SplineLength);
	}

	// Posicionar locomotora
	const FVector LocoPos = CurrentTrackSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	const FRotator LocoRot = CurrentTrackSpline->GetTrackRotationAtDistance(DistanceAlongSpline, ESplineCoordinateSpace::World);
	SetActorLocationAndRotation(LocoPos, LocoRot);

	// Posicionar cada vagon consecutivamente en la curva del spline
	const float LocoOffsetCm = 1000.0f; // Mitad de la locomotora hacia el enganche trasero
	for (int32 i = 0; i < WagonMeshes.Num(); ++i)
	{
		UStaticMeshComponent* WagonComp = WagonMeshes[i];
		if (!WagonComp)
		{
			continue;
		}

		// Distancia longitudinal de este vagon tras la locomotora
		const float WagonDistBehind = LocoOffsetCm + ((i + 1) * (WagonLengthCm + WagonCouplingSpacingCm)) - (WagonLengthCm * 0.5f);
		float WagonSplinePos = DistanceAlongSpline - WagonDistBehind;

		// Compensar bucle ciclico del spline si es necesario
		if (WagonSplinePos < 0.0f)
		{
			WagonSplinePos += SplineLength;
		}

		const FVector WagonWorldPos = CurrentTrackSpline->GetLocationAtDistanceAlongSpline(WagonSplinePos, ESplineCoordinateSpace::World);
		const FRotator WagonWorldRot = CurrentTrackSpline->GetTrackRotationAtDistance(WagonSplinePos, ESplineCoordinateSpace::World);

		WagonComp->SetWorldLocationAndRotation(WagonWorldPos, WagonWorldRot);
	}
}

void ATrainVehicleActor::MonitorLevelCrossings(float DeltaTime)
{
	if (!CurrentTrackSpline.IsValid())
	{
		return;
	}

	const float SplineLength = CurrentTrackSpline->GetSplineLength();
	const float TotalLength = GetTotalTrainLengthCm();

	for (TWeakObjectPtr<ALevelCrossingActor> CrossingPtr : MonitoredCrossings)
	{
		if (!CrossingPtr.IsValid())
		{
			continue;
		}

		ALevelCrossingActor* Crossing = CrossingPtr.Get();

		// Si el paso a nivel no esta asignado a esta via o no tiene coordenada, calcular por distancia euclidea
		float CrossingTrackDistance = Crossing->CrossingDistanceOnTrack;
		if (CrossingTrackDistance <= 0.0f)
		{
			// Encontrar distancia sobre el spline mas cercana al actor del paso
			CrossingTrackDistance = CurrentTrackSpline->FindInputKeyClosestToWorldLocation(Crossing->GetActorLocation());
			CrossingTrackDistance = CurrentTrackSpline->GetDistanceAlongSplineAtSplineInputKey(CrossingTrackDistance);
			Crossing->CrossingDistanceOnTrack = CrossingTrackDistance;
		}

		// Distancia desde la cabeza de la locomotora al cruce
		float DistanceToCrossing = CrossingTrackDistance - DistanceAlongSpline;
		if (DistanceToCrossing < -TotalLength && DistanceAlongSpline < CrossingTrackDistance)
		{
			DistanceToCrossing += SplineLength;
		}

		// 1. ZONA DE DETECCION PREVIA: Iniciar ciclo de aviso acustico y semibarreras
		const float DetectionLimit = Crossing->Settings.TrainDetectionDistanceCm;
		if (DistanceToCrossing > 0.0f && DistanceToCrossing <= DetectionLimit)
		{
			Crossing->NotifyTrainApproaching(this, DistanceToCrossing);

			// Si estamos a menos de 100 metros del paso, accionar bocina reglamentaria
			if (DistanceToCrossing <= 10000.0f && DistanceToCrossing >= 8000.0f)
			{
				SoundTrainHorn();
			}
		}
		// 2. ZONA DE OCUPACION: La locomotora o los vagones estan cruzando la calzada
		else if (DistanceToCrossing <= 0.0f && DistanceToCrossing >= -TotalLength)
		{
			Crossing->NotifyTrainEnteredCrossing(this);
		}
		// 3. ZONA DE LIBERACION: El ultimo vagon ha rebasado el margen de seguridad
		else if (DistanceToCrossing < -TotalLength && DistanceToCrossing >= -(TotalLength + Crossing->Settings.TrainClearanceDistanceCm + 1500.0f))
		{
			Crossing->NotifyTrainClearedCrossing(this);
		}
	}
}

void ATrainVehicleActor::TickHighwayDecongestion(float DeltaTime)
{
	if (!bAutoDecongestHighway)
	{
		return;
	}

	DecongestionTimer += DeltaTime;
	if (DecongestionTimer >= DecongestionInterval)
	{
		DecongestionTimer = 0.0f;

		// 1. Rescatar posibles vehiculos atascados en pasos a nivel proximos
		ClearBlockedVehiclesAtUpcomingCrossing();

		// 2. Absorber camiones y coches de autovias paralelas (Autopista Ferroviaria)
		DecongestParallelHighway(HighwaySweepRadiusCm);
	}
}

bool ATrainVehicleActor::RemoveHighwayVehicle(ATrafficVehicleAgent* RoadVehicle, FString& OutLog)
{
	if (!RoadVehicle || !IsValid(RoadVehicle))
	{
		OutLog = TEXT("Vehiculo invalido o ya destruido.");
		return false;
	}

	const EVehicleCategory Category = RoadVehicle->VehicleCategory;
	int64 EconomicRevenue = 0;

	switch (Category)
	{
	case EVehicleCategory::CamionTrailer:
		// Modal Shift prioritario: retirada de camion de gran tonelaje
		CargoManifest.SemiTrucksCarried++;
		CargoManifest.TotalCargoTons += 28.0f; // 28 toneladas brutas de carga
		CargoManifest.EstimatedCO2SavedKg += 35.0f; // Ahorro neto emisiones
		EconomicRevenue = 450; // Canon de 450 € por semirremolque transferido a vagon Modalohr
		OutLog = FString::Printf(TEXT("Camion Trailer retirado de la autovia. Carga bruta: 28t. Ahorro CO2: 35kg. Canon: +%lld EUR."), EconomicRevenue);
		break;

	case EVehicleCategory::Furgoneta:
	case EVehicleCategory::Autobus:
		CargoManifest.LightVehiclesCarried++;
		CargoManifest.TotalCargoTons += 6.5f;
		CargoManifest.EstimatedCO2SavedKg += 10.0f;
		EconomicRevenue = 180;
		OutLog = FString::Printf(TEXT("Vehiculo comercial/autobus incorporado al convoy ferroviario. Canon: +%lld EUR."), EconomicRevenue);
		break;

	case EVehicleCategory::Turismo:
	case EVehicleCategory::Motocicleta:
	default:
		// Retirada de turismo para vagon portacoches o alivio de atasco
		CargoManifest.LightVehiclesCarried++;
		CargoManifest.TotalCargoTons += 1.8f;
		CargoManifest.EstimatedCO2SavedKg += 4.5f;
		EconomicRevenue = 95;
		OutLog = FString::Printf(TEXT("Turismo retirado de la autovia e ingresado en vagon portacoches. Canon: +%lld EUR."), EconomicRevenue);
		break;
	}

	CargoManifest.IntermodalRevenueEuros += EconomicRevenue;

	// Bonificar al sistema economico del juego
	UWorld* World = GetWorld();
	if (World)
	{
		UEconomySubsystem* Economy = World->GetSubsystem<UEconomySubsystem>();
		if (Economy)
		{
			Economy->AddFunds(EconomicRevenue);
		}

		// Desregistrar de la simulacion de trafico para mejorar la fluidez
		UTrafficSimulationSubsystem* TrafficSub = World->GetSubsystem<UTrafficSimulationSubsystem>();
		if (TrafficSub)
		{
			TrafficSub->UnregisterVehicle(RoadVehicle);
		}
	}

	UE_LOG(LogAutopistas, Log, TEXT("Autopista Ferroviaria: %s"), *OutLog);

	// Eliminar de forma limpia el vehiculo del asfalto
	RoadVehicle->Destroy();
	return true;
}

int32 ATrainVehicleActor::DecongestParallelHighway(float SearchRadiusCm)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	const FVector TrainWorldLocation = GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionShape QuerySphere = FCollisionShape::MakeSphere(SearchRadiusCm);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	int32 RemovedCount = 0;
	const int32 MaxVehiclesPerSweep = 2; // Absorber 1 o 2 vehiculos por barrido para fluidez natural

	if (World->OverlapMultiByChannel(Overlaps, TrainWorldLocation, FQuat::Identity, ECC_WorldDynamic, QuerySphere, QueryParams))
	{
		for (const FOverlapResult& Hit : Overlaps)
		{
			ATrafficVehicleAgent* Vehicle = Cast<ATrafficVehicleAgent>(Hit.GetActor());
			if (Vehicle && IsValid(Vehicle))
			{
				// Prioridad maxima a camiones pesados y a vehiculos accidentados/bloqueados
				const bool bIsHighPriority = (Vehicle->VehicleCategory == EVehicleCategory::CamionTrailer) 
					|| (Vehicle->IncidentState == EIncidentState::Colisionado)
					|| (Vehicle->IncidentState == EIncidentState::AveriadoArcen);

				if (bIsHighPriority || (RemovedCount < 1 && Vehicle->CurrentSpeedKmh < 30.0f))
				{
					FString LogMsg;
					if (RemoveHighwayVehicle(Vehicle, LogMsg))
					{
						RemovedCount++;
						if (RemovedCount >= MaxVehiclesPerSweep)
						{
							break;
						}
					}
				}
			}
		}
	}

	return RemovedCount;
}

void ATrainVehicleActor::ClearBlockedVehiclesAtUpcomingCrossing()
{
	for (TWeakObjectPtr<ALevelCrossingActor> CrossingPtr : MonitoredCrossings)
	{
		if (!CrossingPtr.IsValid())
		{
			continue;
		}

		ALevelCrossingActor* Crossing = CrossingPtr.Get();
		const float Dist = FVector::Dist(GetActorLocation(), Crossing->GetActorLocation());

		// Si el paso esta a menos de 200 metros y tiene algun vehiculo atrapado
		if (Dist <= 20000.0f && Crossing->HasTrappedVehicleOnTracks())
		{
			TArray<ATrafficVehicleAgent*> Trapped = Crossing->GetTrappedVehicles();
			for (ATrafficVehicleAgent* StuckVehicle : Trapped)
			{
				if (StuckVehicle && IsValid(StuckVehicle))
				{
					FString LogMsg;
					UE_LOG(LogAutopistas, Warning, TEXT("PELIGRO EN PASO A NIVEL: Vehiculo atrapado en via detectado por el tren. Procediendo a rescate de emergencia..."));
					RemoveHighwayVehicle(StuckVehicle, LogMsg);
				}
			}
		}
	}
}

void ATrainVehicleActor::SoundTrainHorn()
{
	UE_LOG(LogAutopistas, Warning, TEXT("TREN [%s]: Accionando bocina / silbato reglamentario de proximidad ferroviaria."), *GetName());
}
