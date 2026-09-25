#include "DGT/PegasusHelicopterActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Roads/RoadSplineComponent.h"
#include "Roads/RoadNetworkSubsystem.h"
#include "Traffic/TrafficVehicleAgent.h"
#include "Traffic/TrafficSimulationSubsystem.h"
#include "DGT/DGTControlSubsystem.h"
#include "Economy/EconomySubsystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "AutopistasEspana.h"

APegasusHelicopterActor::APegasusHelicopterActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	HelicopterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HelicopterMesh"));
	HelicopterMesh->SetupAttachment(SceneRoot);
	HelicopterMesh->SetCollisionProfileName(TEXT("NoCollision"));

	MainRotorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainRotorMesh"));
	MainRotorMesh->SetupAttachment(HelicopterMesh);
	MainRotorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	MainRotorMesh->SetCollisionProfileName(TEXT("NoCollision"));

	TailRotorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TailRotorMesh"));
	TailRotorMesh->SetupAttachment(HelicopterMesh);
	TailRotorMesh->SetRelativeLocation(FVector(-380.0f, 20.0f, 80.0f));
	TailRotorMesh->SetCollisionProfileName(TEXT("NoCollision"));

	CameraGimbalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CameraGimbalMesh"));
	CameraGimbalMesh->SetupAttachment(HelicopterMesh);
	CameraGimbalMesh->SetRelativeLocation(FVector(120.0f, 0.0f, -60.0f));
	CameraGimbalMesh->SetCollisionProfileName(TEXT("NoCollision"));
}

void APegasusHelicopterActor::BeginPlay()
{
	Super::BeginPlay();

	// Registrarse en el subsistema DGT si existe
	if (UWorld* World = GetWorld())
	{
		if (UDGTControlSubsystem* DGTSubsystem = World->GetSubsystem<UDGTControlSubsystem>())
		{
			DGTSubsystem->RegisterPegasus(this);
		}

		// Asignar primer tramo de autovia disponible si no habia carretera asignada
		if (!CurrentPatrolRoad.IsValid())
		{
			if (URoadNetworkSubsystem* RoadNetwork = World->GetSubsystem<URoadNetworkSubsystem>())
			{
				const auto& RegisteredRoads = RoadNetwork->GetRegisteredRoads();
				for (const auto& RoadPtr : RegisteredRoads)
				{
					if (RoadPtr.IsValid())
					{
						AssignRoadToPatrol(RoadPtr.Get());
						break;
					}
				}
			}
		}
	}

	UE_LOG(LogAutopistas, Log, TEXT("Pegasus Helicopter inicializado. Cota cenital: %.1f m. Radar operativo."), FlightAltitudeMeters);
}

void APegasusHelicopterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Giro continuo de rotores aerodinamicos
	if (MainRotorMesh)
	{
		MainRotorMesh->AddLocalRotation(FRotator(0.0f, MainRotorRpmSpeed * DeltaTime, 0.0f));
	}
	if (TailRotorMesh)
	{
		TailRotorMesh->AddLocalRotation(FRotator(TailRotorRpmSpeed * DeltaTime, 0.0f, 0.0f));
	}

	// 2. Transicion de estado de sancion
	if (FlightState == EPegasusFlightState::Sancionando)
	{
		PostSanctionCooldown -= DeltaTime;
		if (PostSanctionCooldown <= 0.0f)
		{
			FlightState = EPegasusFlightState::Patrullando;
			ClearCurrentTarget();
		}
	}

	// 3. Gestion de patrulla y seguimiento cinemático
	UpdateFlightPath(DeltaTime);

	// 4. Logica de radar laser y deteccion de infracciones
	if (FlightState == EPegasusFlightState::Patrullando)
	{
		ScanForInfractors(DeltaTime);
	}
	else if (FlightState == EPegasusFlightState::FijandoObjetivo)
	{
		if (CurrentTargetVehicle.IsValid())
		{
			// Orientar camara gimbal MX-15 hacia el vehiculo sospechoso
			UpdateCameraGimbal(DeltaTime);

			// Proyectar haz visual del radar laser
			if (bDrawLaserDebugBeam)
			{
				RenderLaserRadarBeam();
			}

			// Acumular tiempo de fijacion continua
			CurrentLockTime += DeltaTime;

			// Verificar si sigue cometiendo la infraccion o si se completa la telemetria
			const float TargetSpeed = CurrentTargetVehicle->CurrentSpeedKmh;
			float PredecessorDistMeters = 0.0f;
			const bool bIsTailgating = EvaluateTailgating(CurrentTargetVehicle.Get(), PredecessorDistMeters);
			const bool bIsSpeeding = (TargetSpeed > SpeedingThresholdKmh);

			if (CurrentLockTime >= LaserLockRequiredSeconds)
			{
				EInfractionType TypeToFine = EInfractionType::ExcesoVelocidad;
				if (bIsTailgating && !bIsSpeeding)
				{
					TypeToFine = EInfractionType::AcosoTrasero;
				}

				IssueElectronicFine(TypeToFine);
			}
			else if (!bIsSpeeding && !bIsTailgating)
			{
				// Si el conductor frena y guarda distancia antes de completar los 2.5s, se libra de la sancion
				CurrentLockTime = FMath::Max(0.0f, CurrentLockTime - (DeltaTime * 0.5f));
			}
		}
		else
		{
			ClearCurrentTarget();
			FlightState = EPegasusFlightState::Patrullando;
		}
	}

	// 5. Aplicar efecto disuasorio en el cono visual de la aeronave
	ApplyDissuasiveEffect();
}

void APegasusHelicopterActor::AssignRoadToPatrol(URoadSplineComponent* RoadSpline)
{
	if (!RoadSpline)
	{
		return;
	}

	CurrentPatrolRoad = RoadSpline;
	CurrentDistanceAlongSpline = 0.0f;
	bPatrolForward = true;

	const FVector RoadLoc = RoadSpline->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
	const FVector FlightLoc = RoadLoc + FVector(0.0f, 0.0f, FlightAltitudeMeters * 100.0f);
	SetActorLocation(FlightLoc);

	UE_LOG(LogAutopistas, Log, TEXT("Pegasus asignado a patrullar spline: %s"), *RoadSpline->GetName());
}

void APegasusHelicopterActor::SetZenitalAltitude(float InAltitudeMeters)
{
	FlightAltitudeMeters = FMath::Clamp(InAltitudeMeters, 100.0f, 800.0f);
}

void APegasusHelicopterActor::ManualLockVehicle(ATrafficVehicleAgent* TargetVehicle)
{
	if (!TargetVehicle)
	{
		return;
	}

	CurrentTargetVehicle = TargetVehicle;
	CurrentLockTime = 0.0f;
	FlightState = EPegasusFlightState::FijandoObjetivo;
	UE_LOG(LogAutopistas, Log, TEXT("Pegasus: Objetivo fijado manualmente en %s"), *TargetVehicle->GetName());
}

void APegasusHelicopterActor::ClearCurrentTarget()
{
	CurrentTargetVehicle = nullptr;
	CurrentLockTime = 0.0f;
	if (CameraGimbalMesh)
	{
		CameraGimbalMesh->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
	}
}

bool APegasusHelicopterActor::IssueElectronicFine(EInfractionType OverrideType)
{
	if (!CurrentTargetVehicle.IsValid())
	{
		return false;
	}

	ATrafficVehicleAgent* Target = CurrentTargetVehicle.Get();

	FDGTInfractionRecord Record;
	Record.InfractionType = OverrideType;
	Record.DetectedBy = EDGTControlType::Pegasus;
	Record.VehicleDescription = Target->GetName();
	Record.VehicleCategory = Target->VehicleCategory;
	Record.DetectedSpeedKmh = Target->CurrentSpeedKmh;
	Record.SpeedLimitKmh = 120.0f;
	Record.InfractionLocation = Target->GetActorLocation();

	if (OverrideType == EInfractionType::ExcesoVelocidad)
	{
		Record.FineAmountEuros = 600;
		Record.PointsDeducted = 6;
		Record.DistanceToPredecessorMeters = 0.0f;
	}
	else if (OverrideType == EInfractionType::AcosoTrasero)
	{
		Record.FineAmountEuros = 500;
		Record.PointsDeducted = 4;
		float HeadwayMeters = 0.0f;
		EvaluateTailgating(Target, HeadwayMeters);
		Record.DistanceToPredecessorMeters = HeadwayMeters;
	}

	// Notificar e ingresar en subsistemas
	if (UWorld* World = GetWorld())
	{
		if (UDGTControlSubsystem* DGTSubsystem = World->GetSubsystem<UDGTControlSubsystem>())
		{
			DGTSubsystem->RecordInfraction(Record);
		}

		if (UEconomySubsystem* EconomySubsystem = World->GetSubsystem<UEconomySubsystem>())
		{
			EconomySubsystem->AddFunds(Record.FineAmountEuros);
		}
	}

	// Obligar al conductor infractor a reducir velocidad a la legal
	Target->IDMParams.DesiredSpeedKmh = 120.0f;
	Target->FrustrationPercent = FMath::Clamp(Target->FrustrationPercent - 30.0f, 0.0f, 100.0f);
	Target->DriverMood = EDriverMood::Tranquilo;

	// Emitir delegado
	OnInfractionDetected.Broadcast(Record);

	UE_LOG(LogAutopistas, Warning, TEXT("[DGT PEGASUS] Multa expedida a %s! Infraccion: %s, Velocidad: %.1f km/h, Sancion: %lld €, Puntos: -%d"),
		*Record.VehicleDescription,
		*UEnum::GetValueAsString(Record.InfractionType),
		Record.DetectedSpeedKmh,
		Record.FineAmountEuros,
		Record.PointsDeducted);

	// Estado transitorio post-sancion
	FlightState = EPegasusFlightState::Sancionando;
	PostSanctionCooldown = 2.0f;

	return true;
}

float APegasusHelicopterActor::GetLockProgressPercent() const
{
	if (FlightState != EPegasusFlightState::FijandoObjetivo || LaserLockRequiredSeconds <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(CurrentLockTime / LaserLockRequiredSeconds, 0.0f, 1.0f) * 100.0f;
}

FVector APegasusHelicopterActor::GetCameraGimbalWorldLocation() const
{
	if (CameraGimbalMesh)
	{
		return CameraGimbalMesh->GetComponentLocation();
	}
	return GetActorLocation();
}

void APegasusHelicopterActor::ScanForInfractors(float DeltaTime)
{
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

	const FVector PegasusLoc = GetActorLocation();
	const float MaxRangeSq = FMath::Square(DetectionRadiusMeters * 100.0f);

	ATrafficVehicleAgent* WorstOffender = nullptr;
	float HighestSeverity = 0.0f;

	const auto& Vehicles = TrafficSys->GetActiveVehicles();
	for (const auto& VehPtr : Vehicles)
	{
		if (!VehPtr.IsValid() || VehPtr->IncidentState != EIncidentState::Normal)
		{
			continue;
		}

		ATrafficVehicleAgent* Candidate = VehPtr.Get();
		const float DistSq = FVector::DistSquaredXY(PegasusLoc, Candidate->GetActorLocation());
		if (DistSq > MaxRangeSq)
		{
			continue;
		}

		// 1. Evaluar exceso de velocidad (>150 km/h)
		if (Candidate->CurrentSpeedKmh > SpeedingThresholdKmh)
		{
			const float ExcessSeverity = (Candidate->CurrentSpeedKmh - SpeedingThresholdKmh) * 2.0f;
			if (ExcessSeverity > HighestSeverity)
			{
				HighestSeverity = ExcessSeverity;
				WorstOffender = Candidate;
			}
		}

		// 2. Evaluar acoso trasero (distancia de seguridad precaria a gran velocidad)
		float HeadwayMeters = 0.0f;
		if (EvaluateTailgating(Candidate, HeadwayMeters))
		{
			const float TailgateSeverity = 80.0f + (TailgatingSafetyDistanceMeters - HeadwayMeters) * 10.0f;
			if (TailgateSeverity > HighestSeverity)
			{
				HighestSeverity = TailgateSeverity;
				WorstOffender = Candidate;
			}
		}
	}

	if (WorstOffender)
	{
		ManualLockVehicle(WorstOffender);
	}
}

bool APegasusHelicopterActor::EvaluateTailgating(ATrafficVehicleAgent* SubjectVehicle, float& OutPredecessorDistanceMeters) const
{
	OutPredecessorDistanceMeters = 999.0f;

	if (!SubjectVehicle || SubjectVehicle->CurrentSpeedKmh < TailgatingMinSpeedKmh)
	{
		return false;
	}

	if (!SubjectVehicle->CurrentRoadSpline.IsValid())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UTrafficSimulationSubsystem* TrafficSys = World->GetSubsystem<UTrafficSimulationSubsystem>();
	if (!TrafficSys)
	{
		return false;
	}

	const URoadSplineComponent* MyRoad = SubjectVehicle->CurrentRoadSpline.Get();
	const int32 MyLane = SubjectVehicle->CurrentLaneIndex;
	const float MyDist = SubjectVehicle->DistanceAlongSpline;

	float MinPositiveGapCm = 999999.0f;
	bool bFoundLeader = false;

	const auto& Vehicles = TrafficSys->GetActiveVehicles();
	for (const auto& OtherPtr : Vehicles)
	{
		if (!OtherPtr.IsValid() || OtherPtr.Get() == SubjectVehicle)
		{
			continue;
		}

		ATrafficVehicleAgent* Other = OtherPtr.Get();
		if (Other->CurrentRoadSpline.Get() == MyRoad && Other->CurrentLaneIndex == MyLane)
		{
			const float Gap = Other->DistanceAlongSpline - MyDist;
			if (Gap > 0.0f && Gap < MinPositiveGapCm)
			{
				MinPositiveGapCm = Gap;
				bFoundLeader = true;
			}
		}
	}

	if (bFoundLeader)
	{
		OutPredecessorDistanceMeters = MinPositiveGapCm / 100.0f;

		// Acoso trasero: menos de TailgatingSafetyDistanceMeters (8m) a mas de 80 km/h
		if (OutPredecessorDistanceMeters < TailgatingSafetyDistanceMeters)
		{
			return true;
		}
	}

	return false;
}

void APegasusHelicopterActor::UpdateFlightPath(float DeltaTime)
{
	const float SpeedCmS = CruiseSpeedKmh * (100000.0f / 3600.0f);

	if (CurrentPatrolRoad.IsValid())
	{
		URoadSplineComponent* Road = CurrentPatrolRoad.Get();
		const float SplineLen = Road->GetSplineLength();

		if (bPatrolForward)
		{
			CurrentDistanceAlongSpline += SpeedCmS * DeltaTime;
			if (CurrentDistanceAlongSpline >= SplineLen)
			{
				CurrentDistanceAlongSpline = SplineLen;
				bPatrolForward = false;
			}
		}
		else
		{
			CurrentDistanceAlongSpline -= SpeedCmS * DeltaTime;
			if (CurrentDistanceAlongSpline <= 0.0f)
			{
				CurrentDistanceAlongSpline = 0.0f;
				bPatrolForward = true;
			}
		}

		const FVector RoadPos = Road->GetLocationAtDistanceAlongSpline(CurrentDistanceAlongSpline, ESplineCoordinateSpace::World);
		const FVector RoadDir = Road->GetDirectionAtDistanceAlongSpline(CurrentDistanceAlongSpline, ESplineCoordinateSpace::World);

		// Posicion cenital a 300 metros por encima de la calzada
		const FVector DesiredFlightPos = RoadPos + FVector(0.0f, 0.0f, FlightAltitudeMeters * 100.0f);
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), DesiredFlightPos, DeltaTime, 3.0f));

		// Orientacion con cabeceo suave
		FRotator DesiredRot = (bPatrolForward ? RoadDir : -RoadDir).Rotation();
		DesiredRot.Pitch = -3.0f; // Ligera inclinacion de morro propia de avance en helicoptero
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRot, DeltaTime, 4.0f));
	}
	else
	{
		// Si no tiene tramo asignado, avance libre manteniendo 300m
		const FVector ForwardMove = GetActorForwardVector() * SpeedCmS * DeltaTime;
		FVector CurrentPos = GetActorLocation();
		CurrentPos.Z = FlightAltitudeMeters * 100.0f;
		SetActorLocation(CurrentPos + ForwardMove);
	}
}

void APegasusHelicopterActor::UpdateCameraGimbal(float DeltaTime)
{
	if (!CameraGimbalMesh || !CurrentTargetVehicle.IsValid())
	{
		return;
	}

	const FVector GimbalLoc = CameraGimbalMesh->GetComponentLocation();
	const FVector TargetLoc = CurrentTargetVehicle->GetActorLocation();
	const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(GimbalLoc, TargetLoc);

	// Convertir a rotacion relativa respecto al helicoptero
	const FRotator LocalLookAt = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRot, GetActorRotation());
	CameraGimbalMesh->SetRelativeRotation(FMath::RInterpTo(CameraGimbalMesh->GetRelativeRotation(), LocalLookAt, DeltaTime, 8.0f));
}

void APegasusHelicopterActor::RenderLaserRadarBeam()
{
	if (!CurrentTargetVehicle.IsValid() || !GetWorld())
	{
		return;
	}

	const FVector GimbalLoc = GetCameraGimbalWorldLocation();
	const FVector TargetLoc = CurrentTargetVehicle->GetActorLocation();

	// Color del haz: transicion de rojo a verde brillante segun progreso de fijacion
	const float Progress = GetLockProgressPercent() / 100.0f;
	const FColor BeamColor = FColor::MakeRedToGreenColorFromScalar(Progress);

	// Haz laser central de telemetria
	DrawDebugLine(GetWorld(), GimbalLoc, TargetLoc, BeamColor, false, -1.0f, 0, 4.0f);

	// Reticula de bloqueo proyectada sobre el techo del vehiculo
	DrawDebugCircle(GetWorld(), TargetLoc + FVector(0.0f, 0.0f, 30.0f), 130.0f, 32, BeamColor, false, -1.0f, 0, 3.0f, FVector(1,0,0), FVector(0,1,0), false);
	DrawDebugCircle(GetWorld(), TargetLoc + FVector(0.0f, 0.0f, 30.0f), 60.0f * (1.0f - Progress * 0.5f), 16, BeamColor, false, -1.0f, 0, 2.0f, FVector(1,0,0), FVector(0,1,0), false);
}

void APegasusHelicopterActor::ApplyDissuasiveEffect()
{
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

	const FVector PegasusLoc = GetActorLocation();
	const float CoverageRadiusSq = FMath::Square(DetectionRadiusMeters * 100.0f);

	const auto& Vehicles = TrafficSys->GetActiveVehicles();
	for (const auto& VehPtr : Vehicles)
	{
		if (!VehPtr.IsValid())
		{
			continue;
		}

		ATrafficVehicleAgent* Veh = VehPtr.Get();
		if (FVector::DistSquaredXY(PegasusLoc, Veh->GetActorLocation()) <= CoverageRadiusSq)
		{
			// Disuasion: el 95% de los conductores que ven el helicoptero se autorregulan al limite de 120 km/h
			if (Veh->IDMParams.DesiredSpeedKmh > 120.0f && Veh != CurrentTargetVehicle.Get())
			{
				Veh->IDMParams.DesiredSpeedKmh = 120.0f;
			}
		}
	}
}
