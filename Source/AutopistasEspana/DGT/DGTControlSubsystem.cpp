#include "DGT/DGTControlSubsystem.h"
#include "DGT/PegasusHelicopterActor.h"
#include "Traffic/TrafficSimulationSubsystem.h"
#include "Traffic/TrafficVehicleAgent.h"
#include "Roads/RoadSplineComponent.h"
#include "Roads/RoadNetworkSubsystem.h"
#include "Economy/EconomySubsystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "AutopistasEspana.h"

void UDGTControlSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OperationalStats = FDGTSummaryStats();
	NextControlId = 1001;

	// Iniciar ciclo periodico de inspeccion de calzada cada segundo
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CheckpointTimerHandle,
			this,
			&UDGTControlSubsystem::ProcessCheckpointInspections,
			1.0f,
			true
		);
	}

	UE_LOG(LogAutopistas, Log, TEXT("UDGTControlSubsystem inicializado con exito. Servicio de vigilancia y balizamiento activo."));
}

void UDGTControlSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CheckpointTimerHandle);
	}

	ClearAllControlPoints();
	ActivePegasusHelicopters.Empty();
	InfractionHistory.Empty();

	Super::Deinitialize();
}

int32 UDGTControlSubsystem::DeployControlPoint(EDGTControlType ControlType, URoadSplineComponent* Road, float DistanceAlongSpline, float TaperLengthMeters, int32 ConesCount)
{
	if (!Road || !GetWorld())
	{
		UE_LOG(LogAutopistas, Warning, TEXT("DeployControlPoint fallo: Tramo de carretera no valido."));
		return -1;
	}

	FDGTControlPoint NewPoint;
	NewPoint.ControlId = NextControlId++;
	NewPoint.ControlType = ControlType;
	NewPoint.RoadSpline = Road;
	NewPoint.DistanceAlongSpline = DistanceAlongSpline;
	NewPoint.ConeTaperLengthMeters = TaperLengthMeters;
	NewPoint.NumConesDeployed = ConesCount;
	NewPoint.bActive = true;

	// Coordenadas mundiales extraidas del spline
	NewPoint.WorldLocation = Road->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	NewPoint.WorldRotation = Road->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);

	// Parametros operativos segun el tipo de control
	switch (ControlType)
	{
	case EDGTControlType::Alcoholemia:
		NewPoint.TargetSpeedLimitKmh = 30.0f;
		NewPoint.AccidentReductionFactor = 0.70f; // -70% accidentes en el sector
		NewPoint.DailyOperationalCostEuros = 500;
		break;

	case EDGTControlType::PesajeCamiones:
		NewPoint.TargetSpeedLimitKmh = 20.0f;
		NewPoint.AccidentReductionFactor = 0.50f; // -50% colisiones por fallo de frenos/sobrepeso
		NewPoint.DailyOperationalCostEuros = 350;
		break;

	case EDGTControlType::RadarFijo:
		NewPoint.TargetSpeedLimitKmh = 120.0f;
		NewPoint.ConeTaperLengthMeters = 0.0f; // El radar no requiere conificacion en calzada
		NewPoint.NumConesDeployed = 0;
		NewPoint.AccidentReductionFactor = 0.30f; // -30% siniestralidad por calmado de velocidad
		NewPoint.DailyOperationalCostEuros = 80;
		break;

	case EDGTControlType::Pegasus:
		NewPoint.TargetSpeedLimitKmh = 120.0f;
		NewPoint.ConeTaperLengthMeters = 0.0f;
		NewPoint.NumConesDeployed = 0;
		NewPoint.AccidentReductionFactor = 0.45f;
		NewPoint.DailyOperationalCostEuros = 1200;
		break;
	}

	// Balizamiento fisico con conos de obra en calzada y furgoneta/radar
	SpawnBalizamientoCones(NewPoint);

	ActiveControlPoints.Add(NewPoint);

	// Actualizar factor global de reduccion de siniestralidad
	CalculateGlobalAccidentReductionPercent();

	// Emitir delegado
	OnControlPointDeployed.Broadcast(NewPoint);

	UE_LOG(LogAutopistas, Log, TEXT("DGT Control #%d desplegado (%s) en Spline %s a distancia %.1f m."),
		NewPoint.ControlId,
		*UEnum::GetValueAsString(ControlType),
		*Road->GetName(),
		DistanceAlongSpline / 100.0f);

	return NewPoint.ControlId;
}

bool UDGTControlSubsystem::RemoveControlPoint(int32 ControlId)
{
	for (int32 i = 0; i < ActiveControlPoints.Num(); ++i)
	{
		if (ActiveControlPoints[i].ControlId == ControlId)
		{
			DestroyControlVisualProps(ActiveControlPoints[i]);
			ActiveControlPoints.RemoveAt(i);

			CalculateGlobalAccidentReductionPercent();
			OnControlPointRemoved.Broadcast(ControlId);

			UE_LOG(LogAutopistas, Log, TEXT("DGT Control #%d levantado y retirado de calzada."), ControlId);
			return true;
		}
	}

	return false;
}

void UDGTControlSubsystem::ClearAllControlPoints()
{
	for (auto& Point : ActiveControlPoints)
	{
		DestroyControlVisualProps(Point);
		OnControlPointRemoved.Broadcast(Point.ControlId);
	}

	ActiveControlPoints.Empty();
	CalculateGlobalAccidentReductionPercent();
}

bool UDGTControlSubsystem::GetControlPointById(int32 ControlId, FDGTControlPoint& OutControlPoint) const
{
	for (const auto& Point : ActiveControlPoints)
	{
		if (Point.ControlId == ControlId)
		{
			OutControlPoint = Point;
			return true;
		}
	}
	return false;
}

APegasusHelicopterActor* UDGTControlSubsystem::SpawnPegasusPatrol(URoadSplineComponent* InitialRoad, FVector CustomLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FVector SpawnPos = CustomLocation;
	FRotator SpawnRot = FRotator::ZeroRotator;

	if (InitialRoad)
	{
		const FVector RoadPos = InitialRoad->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
		SpawnPos = RoadPos + FVector(0.0f, 0.0f, 30000.0f); // 300m cota cenital
		SpawnRot = InitialRoad->GetRotationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
	}
	else if (SpawnPos.IsNearlyZero())
	{
		SpawnPos = FVector(0.0f, 0.0f, 30000.0f);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APegasusHelicopterActor* Pegasus = World->SpawnActor<APegasusHelicopterActor>(APegasusHelicopterActor::StaticClass(), SpawnPos, SpawnRot, SpawnParams);
	if (Pegasus)
	{
		if (InitialRoad)
		{
			Pegasus->AssignRoadToPatrol(InitialRoad);
		}
		RegisterPegasus(Pegasus);
	}

	return Pegasus;
}

void UDGTControlSubsystem::RegisterPegasus(APegasusHelicopterActor* Helicopter)
{
	if (Helicopter && !ActivePegasusHelicopters.Contains(Helicopter))
	{
		ActivePegasusHelicopters.Add(Helicopter);
		CalculateGlobalAccidentReductionPercent();
		UE_LOG(LogAutopistas, Log, TEXT("Pegasus Helicopter registrado en DGT Subsystem. Unidades aereas activas: %d"), ActivePegasusHelicopters.Num());
	}
}

void UDGTControlSubsystem::UnregisterPegasus(APegasusHelicopterActor* Helicopter)
{
	if (Helicopter)
	{
		ActivePegasusHelicopters.Remove(Helicopter);
		CalculateGlobalAccidentReductionPercent();
		UE_LOG(LogAutopistas, Log, TEXT("Pegasus Helicopter dado de baja. Unidades aereas activas: %d"), ActivePegasusHelicopters.Num());
	}
}

void UDGTControlSubsystem::RecordInfraction(const FDGTInfractionRecord& Record)
{
	InfractionHistory.Add(Record);

	// Actualizar metricas operativas
	OperationalStats.TotalInfractions++;
	OperationalStats.TotalRevenueCollectedEuros += Record.FineAmountEuros;
	OperationalStats.TotalPointsWithdrawn += Record.PointsDeducted;

	switch (Record.InfractionType)
	{
	case EInfractionType::ExcesoVelocidad:
		OperationalStats.SpeedingInfractions++;
		break;
	case EInfractionType::AcosoTrasero:
		OperationalStats.TailgatingInfractions++;
		break;
	case EInfractionType::AlcoholemiaPositiva:
		OperationalStats.AlcoholInfractions++;
		break;
	case EInfractionType::SobrecargaPesaje:
		OperationalStats.OverloadInfractions++;
		break;
	default:
		break;
	}

	// Ingreso en el subsistema de economia
	if (UWorld* World = GetWorld())
	{
		if (UEconomySubsystem* Economy = World->GetSubsystem<UEconomySubsystem>())
		{
			Economy->AddFunds(Record.FineAmountEuros);
		}
	}

	// Notificar a observadores y HUD
	OnInfractionIssued.Broadcast(Record);

	UE_LOG(LogAutopistas, Log, TEXT("[DGT EXPEDIENTE] Nueva sancion registrada. Tipo: %s, Importe: %lld €, Puntos: %d. Total Recaudado: %lld €"),
		*UEnum::GetValueAsString(Record.InfractionType),
		Record.FineAmountEuros,
		Record.PointsDeducted,
		OperationalStats.TotalRevenueCollectedEuros);
}

float UDGTControlSubsystem::CalculateGlobalAccidentReductionPercent() const
{
	// Formula probabilistica ponderada:
	// Cada tipo de operativo reduce una fraccion del riesgo residual
	// R_total = 1 - (1 - R_alc)^N * (1 - R_pes)^M * (1 - R_rad)^K * (1 - R_peg)^L

	float ResidualRisk = 1.0f;

	for (const auto& Point : ActiveControlPoints)
	{
		if (!Point.bActive)
		{
			continue;
		}

		switch (Point.ControlType)
		{
		case EDGTControlType::Alcoholemia:
			ResidualRisk *= (1.0f - 0.15f); // Cada control preventivo reduce 15% riesgo global
			break;
		case EDGTControlType::PesajeCamiones:
			ResidualRisk *= (1.0f - 0.10f); // Cada control de pesaje reduce 10%
			break;
		case EDGTControlType::RadarFijo:
			ResidualRisk *= (1.0f - 0.08f); // Cada cinemometro fijo reduce 8%
			break;
		default:
			break;
		}
	}

	// Impacto de la vigilancia aerea Pegasus
	for (const auto& PegPtr : ActivePegasusHelicopters)
	{
		if (PegPtr.IsValid())
		{
			ResidualRisk *= (1.0f - 0.25f); // Cada Pegasus en vuelo reduce un 25% la siniestralidad por disuasion
		}
	}

	// Limitar reduccion maxima al 85% (siempre existe riesgo residual de fallo humano o meteorologico)
	const float ReductionFraction = FMath::Clamp(1.0f - ResidualRisk, 0.0f, 0.85f);
	const float FinalPercentage = ReductionFraction * 100.0f;

	// Asignar al struct de estadisticas
	const_cast<UDGTControlSubsystem*>(this)->OperationalStats.GlobalAccidentReductionPercentage = FinalPercentage;

	return FinalPercentage;
}

float UDGTControlSubsystem::GetAccidentReductionAtLocation(const FVector& Location) const
{
	float LocalRisk = 1.0f;

	// 1. Proximidad a puntos de control en calzada (influencia hasta 2.5 km)
	const float MaxControlInfluenceCm = 250000.0f; // 2.5 km
	for (const auto& Point : ActiveControlPoints)
	{
		if (!Point.bActive)
		{
			continue;
		}

		const float Dist = FVector::Dist(Location, Point.WorldLocation);
		if (Dist < MaxControlInfluenceCm)
		{
			const float Factor = 1.0f - (Dist / MaxControlInfluenceCm);
			const float LocalMitigation = Point.AccidentReductionFactor * Factor;
			LocalRisk *= (1.0f - LocalMitigation);
		}
	}

	// 2. Proximidad a helicoptero Pegasus (influencia directa del cono de 1.2 km)
	const float PegasusInfluenceCm = 120000.0f;
	for (const auto& PegPtr : ActivePegasusHelicopters)
	{
		if (PegPtr.IsValid())
		{
			const float DistXY = FVector::Dist2D(Location, PegPtr->GetActorLocation());
			if (DistXY < PegasusInfluenceCm)
			{
				const float AirFactor = 1.0f - (DistXY / PegasusInfluenceCm);
				LocalRisk *= (1.0f - 0.40f * AirFactor);
			}
		}
	}

	return FMath::Clamp(1.0f - LocalRisk, 0.0f, 0.85f);
}

void UDGTControlSubsystem::ProcessCheckpointInspections()
{
	UWorld* World = GetWorld();
	if (!World || ActiveControlPoints.Num() == 0)
	{
		return;
	}

	UTrafficSimulationSubsystem* TrafficSys = World->GetSubsystem<UTrafficSimulationSubsystem>();
	if (!TrafficSys)
	{
		return;
	}

	const auto& Vehicles = TrafficSys->GetActiveVehicles();

	for (const auto& Point : ActiveControlPoints)
	{
		if (!Point.bActive || !Point.RoadSpline.IsValid())
		{
			continue;
		}

		const URoadSplineComponent* Road = Point.RoadSpline.Get();
		const float CheckpointDist = Point.DistanceAlongSpline;
		const float TaperLengthCm = Point.ConeTaperLengthMeters * 100.0f;

		for (const auto& VehPtr : Vehicles)
		{
			if (!VehPtr.IsValid() || VehPtr->IncidentState != EIncidentState::Normal)
			{
				continue;
			}

			ATrafficVehicleAgent* Veh = VehPtr.Get();
			if (Veh->CurrentRoadSpline.Get() != Road)
			{
				continue;
			}

			// Distancia relativa del vehiculo al punto focal del control
			const float RelDist = Veh->DistanceAlongSpline - CheckpointDist;

			// Zona de actuacion: desde el inicio de la conificacion hasta 30m despues
			if (RelDist >= -TaperLengthCm && RelDist <= 3000.0f)
			{
				// Reduccion de velocidad adaptativa al pasar por el cono de control
				if (Veh->IDMParams.DesiredSpeedKmh > Point.TargetSpeedLimitKmh)
				{
					Veh->IDMParams.DesiredSpeedKmh = Point.TargetSpeedLimitKmh;
				}

				// Inspeccion segun tipologia
				if (Point.ControlType == EDGTControlType::RadarFijo)
				{
					// Cinemometro fijo: sancion automatica si excede limite + margen de 5 km/h
					if (Veh->CurrentSpeedKmh > Point.TargetSpeedLimitKmh + 5.0f && FMath::Abs(RelDist) < 500.0f)
					{
						FDGTInfractionRecord Rec;
						Rec.InfractionType = EInfractionType::ExcesoVelocidad;
						Rec.DetectedBy = EDGTControlType::RadarFijo;
						Rec.VehicleDescription = Veh->GetName();
						Rec.VehicleCategory = Veh->VehicleCategory;
						Rec.DetectedSpeedKmh = Veh->CurrentSpeedKmh;
						Rec.SpeedLimitKmh = Point.TargetSpeedLimitKmh;
						Rec.InfractionLocation = Veh->GetActorLocation();

						if (Veh->CurrentSpeedKmh > 150.0f)
						{
							Rec.FineAmountEuros = 600;
							Rec.PointsDeducted = 6;
						}
						else
						{
							Rec.FineAmountEuros = 300;
							Rec.PointsDeducted = 2;
						}

						RecordInfraction(Rec);

						// Forzar reduccion inmediata
						Veh->IDMParams.DesiredSpeedKmh = Point.TargetSpeedLimitKmh;
					}
				}
				else if (Point.ControlType == EDGTControlType::Alcoholemia)
				{
					// Control de alcoholemia: test a conductores erráticos o con ira al volante
					if (FMath::Abs(RelDist) < 400.0f)
					{
						const bool bHighRiskDriver = (Veh->DriverMood == EDriverMood::FuriaAlVolante || Veh->FrustrationPercent > 70.0f);
						const float AlcoholChance = bHighRiskDriver ? 0.20f : 0.015f;

						if (FMath::FRand() < AlcoholChance)
						{
							FDGTInfractionRecord Rec;
							Rec.InfractionType = EInfractionType::AlcoholemiaPositiva;
							Rec.DetectedBy = EDGTControlType::Alcoholemia;
							Rec.VehicleDescription = Veh->GetName();
							Rec.VehicleCategory = Veh->VehicleCategory;
							Rec.DetectedSpeedKmh = Veh->CurrentSpeedKmh;
							Rec.SpeedLimitKmh = Point.TargetSpeedLimitKmh;
							Rec.FineAmountEuros = 1000;
							Rec.PointsDeducted = 6;
							Rec.InfractionLocation = Veh->GetActorLocation();

							RecordInfraction(Rec);

							// Inmovilizacion del vehiculo en el arcen por la Guardia Civil
							Veh->IncidentState = EIncidentState::AveriadoArcen;
							Veh->CurrentSpeedCmS = 0.0f;
							Veh->CurrentSpeedKmh = 0.0f;

							UE_LOG(LogAutopistas, Warning, TEXT("[DGT ALCOHOLEMIA] Positivo detectado en %s! Vehiculo inmovilizado en arcen."), *Veh->GetName());
						}
					}
				}
				else if (Point.ControlType == EDGTControlType::PesajeCamiones)
				{
					// Bascula de pesaje para transporte de mercancías
					if (Veh->VehicleCategory == EVehicleCategory::CamionTrailer && FMath::Abs(RelDist) < 400.0f)
					{
						// 12% probabilidad de sobrepeso de mercancia o infraccion de tacografo
						if (FMath::FRand() < 0.12f)
						{
							FDGTInfractionRecord Rec;
							Rec.InfractionType = EInfractionType::SobrecargaPesaje;
							Rec.DetectedBy = EDGTControlType::PesajeCamiones;
							Rec.VehicleDescription = Veh->GetName();
							Rec.VehicleCategory = Veh->VehicleCategory;
							Rec.DetectedSpeedKmh = Veh->CurrentSpeedKmh;
							Rec.SpeedLimitKmh = Point.TargetSpeedLimitKmh;
							Rec.FineAmountEuros = 2500;
							Rec.PointsDeducted = 0; // Sancion economica mercantil a la empresa de transportes
							Rec.InfractionLocation = Veh->GetActorLocation();

							RecordInfraction(Rec);

							// Obligar a estacionar en area de servicio para descarga o descanso
							Veh->IncidentState = EIncidentState::AveriadoArcen;
							Veh->CurrentSpeedCmS = 0.0f;
							Veh->CurrentSpeedKmh = 0.0f;

							UE_LOG(LogAutopistas, Warning, TEXT("[DGT PESAJE] Camion %s inmovilizado por exceso de peso MMA! Multa: 2500 €"), *Veh->GetName());
						}
					}
				}
			}
		}
	}
}

void UDGTControlSubsystem::SpawnBalizamientoCones(FDGTControlPoint& ControlPoint)
{
	UWorld* World = GetWorld();
	if (!World || !ControlPoint.RoadSpline.IsValid())
	{
		return;
	}

	URoadSplineComponent* Road = ControlPoint.RoadSpline.Get();
	const float TaperLengthCm = ControlPoint.ConeTaperLengthMeters * 100.0f;
	const int32 NumCones = FMath::Max(2, ControlPoint.NumConesDeployed);

	// Intentar cargar malla oficial de cono DGT (SM_Cono_Obra_75)
	UStaticMesh* ConeMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Game/Meshes/RoadProps/SM_Cono_Obra_75")));
	UStaticMesh* RadarMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Game/Meshes/RoadProps/SM_Radar_Fijo_DGT")));
	UStaticMesh* PatrolMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Game/Meshes/Vehicles/Patrulla_Guardia_Civil")));

	if (ControlPoint.ControlType == EDGTControlType::RadarFijo)
	{
		// Spawn de cabina de radar fijo en el arcen exterior
		const float RadarDist = ControlPoint.DistanceAlongSpline;
		const FVector CenterLoc = Road->GetLocationAtDistanceAlongSpline(RadarDist, ESplineCoordinateSpace::World);
		const FVector RightVec = Road->GetRightVectorAtDistanceAlongSpline(RadarDist, ESplineCoordinateSpace::World);
		const FRotator Rot = Road->GetRotationAtDistanceAlongSpline(RadarDist, ESplineCoordinateSpace::World);

		const FVector RadarLoc = CenterLoc + (RightVec * 420.0f); // 4.2m a la derecha en el arcen

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* RadarActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), RadarLoc, Rot, SpawnParams);
		if (RadarActor)
		{
			if (RadarMesh)
			{
				RadarActor->GetStaticMeshComponent()->SetStaticMesh(RadarMesh);
			}
			RadarActor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
			ControlPoint.SpawnedVisualActors.Add(RadarActor);
		}
		return;
	}

	// Para Alcoholemia y Pesaje: conificacion diagonal de carril (Taper DGT)
	const float StartDist = ControlPoint.DistanceAlongSpline - TaperLengthCm;
	const float DistStep = TaperLengthCm / (NumCones - 1);

	// Anchura a cerrar: estrechamiento desde arcen (+350cm) hasta el limite del carril central (+50cm)
	const float StartLateralOffset = 380.0f;
	const float EndLateralOffset = 60.0f;

	for (int32 i = 0; i < NumCones; ++i)
	{
		const float CurrentDist = StartDist + (i * DistStep);
		const float Alpha = (float)i / (float)(NumCones - 1);
		const float CurrentLateralOffset = FMath::Lerp(StartLateralOffset, EndLateralOffset, Alpha);

		const FVector SplinePos = Road->GetLocationAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::World);
		const FVector RightVec = Road->GetRightVectorAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::World);
		const FRotator Rot = Road->GetRotationAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::World);

		const FVector ConeLoc = SplinePos + (RightVec * CurrentLateralOffset);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* ConeActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), ConeLoc, Rot, SpawnParams);
		if (ConeActor)
		{
			if (ConeMesh)
			{
				ConeActor->GetStaticMeshComponent()->SetStaticMesh(ConeMesh);
			}
			ConeActor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("NoCollision"));
			ConeActor->SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
			ControlPoint.SpawnedVisualActors.Add(ConeActor);
		}
	}

	// Vehiculo de patrulla Guardia Civil estacionado en el arcen al final del embudo
	const FVector PatrolCenter = Road->GetLocationAtDistanceAlongSpline(ControlPoint.DistanceAlongSpline + 200.0f, ESplineCoordinateSpace::World);
	const FVector PatrolRight = Road->GetRightVectorAtDistanceAlongSpline(ControlPoint.DistanceAlongSpline + 200.0f, ESplineCoordinateSpace::World);
	const FRotator PatrolRot = Road->GetRotationAtDistanceAlongSpline(ControlPoint.DistanceAlongSpline + 200.0f, ESplineCoordinateSpace::World);

	const FVector PatrolLoc = PatrolCenter + (PatrolRight * 360.0f); // Estacionado en el arcen

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* PatrolActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), PatrolLoc, PatrolRot, SpawnParams);
	if (PatrolActor)
	{
		if (PatrolMesh)
		{
			PatrolActor->GetStaticMeshComponent()->SetStaticMesh(PatrolMesh);
		}
		PatrolActor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
		ControlPoint.SpawnedVisualActors.Add(PatrolActor);
	}
}

void UDGTControlSubsystem::DestroyControlVisualProps(FDGTControlPoint& ControlPoint)
{
	for (auto& ActorPtr : ControlPoint.SpawnedVisualActors)
	{
		if (ActorPtr.IsValid())
		{
			ActorPtr->Destroy();
		}
	}
	ControlPoint.SpawnedVisualActors.Empty();
}
