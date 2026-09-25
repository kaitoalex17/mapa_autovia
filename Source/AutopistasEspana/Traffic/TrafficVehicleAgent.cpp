#include "Traffic/TrafficVehicleAgent.h"
#include "Components/StaticMeshComponent.h"
#include "Roads/RoadSplineComponent.h"
#include "Roads/RoadSegmentActor.h"
#include "Roads/RoadNetworkSubsystem.h"
#include "Traffic/TrafficSimulationSubsystem.h"
#include "Economy/EconomySubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "AutopistasEspana.h"

ATrafficVehicleAgent::ATrafficVehicleAgent()
{
	PrimaryActorTick.bCanEverTick = true;

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	RootComponent = VehicleMesh;
	VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));
	VehicleMesh->SetGenerateOverlapEvents(true);

	// Valores iniciales por defecto (Norma DGT - Turismo)
	IDMParams.DesiredSpeedKmh = 120.0f;
	IDMParams.SafeTimeHeadwaySeconds = 1.4f;
	IDMParams.MinimumJamDistanceCm = 250.0f;
	IDMParams.MaxAccelerationCmS2 = 280.0f;
	IDMParams.DesiredDecelerationCmS2 = 350.0f;
	IDMParams.EmergencyBrakingCmS2 = 750.0f;
	IDMParams.AccelerationExponent = 4.0f;

	BaseIDMParams = IDMParams;

	// Rasgo de paciencia individual aleatorizado para variabilidad de trafico
	IndividualPatienceTolerance = FMath::RandRange(12.0f, 25.0f);
}

void ATrafficVehicleAgent::BeginPlay()
{
	Super::BeginPlay();
	
	BaseIDMParams = IDMParams;

	// Convertir km/h a cm/s para calculos fisicos internos
	// 1 km/h = 27.7778 cm/s
	CurrentSpeedCmS = (IDMParams.DesiredSpeedKmh * 0.8f) * (100000.0f / 3600.0f);
	CurrentSpeedKmh = CurrentSpeedCmS * (3600.0f / 100000.0f);
}

void ATrafficVehicleAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Si el vehiculo esta colisionado o averiado, permanece detenido bloqueando la via
	if (IncidentState == EIncidentState::Colisionado || IncidentState == EIncidentState::AveriadoArcen)
	{
		CurrentSpeedCmS = 0.0f;
		CurrentSpeedKmh = 0.0f;
		return;
	}

	// 1. Deteccion de Carretera y Obras Viales
	ARoadSegmentActor* RoadActor = nullptr;
	float EffectiveRoadSpeedLimitKmh = 120.0f;

	if (CurrentRoadSpline.IsValid())
	{
		RoadActor = Cast<ARoadSegmentActor>(CurrentRoadSpline->GetOwner());
		if (RoadActor)
		{
			EffectiveRoadSpeedLimitKmh = RoadActor->GetEffectiveSpeedLimitKmh();

			// Si el carril actual ha sido cortado por obras, forzar incorporacion al carril abierto
			if (RoadActor->IsLaneClosed(CurrentLaneIndex) && !bIsChangingLanes)
			{
				// Cambiar al carril 0 (abierto a la circulacion)
				TargetLaneOffsetCm = 175.0f;
				CurrentLaneIndex = 0;
				bIsChangingLanes = true;
				LaneChangeProgress = 0.0f;
				LaneChangeDuration = (CondicionPsicologica == ECondicionPsicologica::FuriaAlVolante) ? 1.0f : 2.0f;

				UE_LOG(LogAutopistas, Log, TEXT("Vehiculo [%s]: Carril cortado por obras, incorporandose al carril abierto."), *GetName());
			}
		}
		else
		{
			EffectiveRoadSpeedLimitKmh = CurrentRoadSpline->CrossSection.SpeedLimitKmh;
		}
	}

	// La velocidad deseada del IDM se adapta a la senalizacion de obras (ej. 40 o 60 km/h)
	const float DesiredSpeedKmh = FMath::Min(IDMParams.DesiredSpeedKmh, EffectiveRoadSpeedLimitKmh);
	const float DesiredSpeedCmS = DesiredSpeedKmh * (100000.0f / 3600.0f);

	// 2. Transicion lateral de cambio de carril MOBIL
	if (bIsChangingLanes)
	{
		LaneChangeProgress += DeltaTime / FMath::Max(0.1f, LaneChangeDuration);
		CurrentLaneOffsetCm = FMath::FInterpTo(CurrentLaneOffsetCm, TargetLaneOffsetCm, DeltaTime, 4.5f);

		if (LaneChangeProgress >= 1.0f || FMath::IsNearlyEqual(CurrentLaneOffsetCm, TargetLaneOffsetCm, 3.0f))
		{
			CurrentLaneOffsetCm = TargetLaneOffsetCm;
			bIsChangingLanes = false;
			LaneChangeProgress = 0.0f;
		}
	}

	// 3. Deteccion de Vehiculo Lider / Retencion de Obras
	float DistanceToLeadCm = 50000.0f;
	float RelativeSpeedCmS = 0.0f;
	ATrafficVehicleAgent* LeadVehicle = nullptr;
	const bool bHasLead = FindLeadVehicle(25000.0f, DistanceToLeadCm, RelativeSpeedCmS, LeadVehicle);

	// 4. Calculo de Aceleracion mediante el Modelo de Conductor Inteligente (IDM)
	const float MaxAccel = IDMParams.MaxAccelerationCmS2;
	const float FreeRoadTerm = 1.0f - FMath::Pow(CurrentSpeedCmS / FMath::Max(1.0f, DesiredSpeedCmS), IDMParams.AccelerationExponent);

	float InteractionTerm = 0.0f;
	if (DistanceToLeadCm > 0.0f && DistanceToLeadCm < 40000.0f)
	{
		const float SqrtAB = 2.0f * FMath::Sqrt(MaxAccel * IDMParams.DesiredDecelerationCmS2);
		const float DynamicJamDistance = IDMParams.MinimumJamDistanceCm + 
			(CurrentSpeedCmS * IDMParams.SafeTimeHeadwaySeconds) + 
			((CurrentSpeedCmS * RelativeSpeedCmS) / FMath::Max(1.0f, SqrtAB));

		InteractionTerm = FMath::Square(DynamicJamDistance / FMath::Max(1.0f, DistanceToLeadCm));
	}

	const float AccelResult = MaxAccel * (FreeRoadTerm - InteractionTerm);
	const float ClampedAccel = FMath::Clamp(AccelResult, -IDMParams.EmergencyBrakingCmS2, MaxAccel);

	CurrentSpeedCmS = FMath::Clamp(CurrentSpeedCmS + (ClampedAccel * DeltaTime), 0.0f, DesiredSpeedCmS);
	CurrentSpeedKmh = CurrentSpeedCmS * (3600.0f / 100000.0f);

	// 5. Actualizacion de Psicologia, Frustracion y Furia al Volante
	UpdateDriverPsychology(DeltaTime, DistanceToLeadCm);

	// 6. Evaluacion periodica de adelantamiento MOBIL
	OvertakeCheckTimer += DeltaTime;
	if (OvertakeCheckTimer >= 0.5f)
	{
		OvertakeCheckTimer = 0.0f;

		// Si llevamos vehiculo lento delante y circulamos por debajo del 80% del limite
		if (bHasLead && DistanceToLeadCm < 3500.0f && CurrentSpeedKmh < DesiredSpeedKmh * 0.85f)
		{
			EvaluateMOBILLaneChange(true);
		}
		// Regla de retorno a la derecha si carril libre y no hay obras
		else if (CurrentLaneIndex == 1 && (!bHasLead || DistanceToLeadCm > 6000.0f))
		{
			EvaluateMOBILLaneChange(false);
		}
	}

	// 7. Avance cinematico a lo largo del spline
	if (CurrentRoadSpline.IsValid())
	{
		DistanceAlongSpline += CurrentSpeedCmS * DeltaTime;
		const float SplineTotalLength = CurrentRoadSpline->GetSplineLength();

		if (DistanceAlongSpline >= SplineTotalLength)
		{
			bool bTransitioned = false;
			if (UWorld* World = GetWorld())
			{
				if (URoadNetworkSubsystem* RoadNetwork = World->GetSubsystem<URoadNetworkSubsystem>())
				{
					const TArray<FRoadJunctionNode> Junctions = RoadNetwork->GetJunctionsForRoad(CurrentRoadSpline.Get());
					for (const FRoadJunctionNode& Junction : Junctions)
					{
						for (const FLaneConnection& Conn : Junction.LaneConnections)
						{
							if (Conn.SourceLaneIndex == CurrentLaneIndex && Conn.TargetRoad && Conn.TargetRoad != CurrentRoadSpline.Get())
							{
								// Transicion al carril conectado de la nueva carretera segun Norma 8.1-IC
								float TransitionLen = Conn.TransitionLength;
								const int32 NewLane = RoadNetwork->GetNextConnectedLaneWithTransition(CurrentRoadSpline.Get(), CurrentLaneIndex, Conn.TargetRoad, TransitionLen);

								const FVector CurrentWorldLocation = GetActorLocation();
								const float TargetKey = Conn.TargetRoad->FindInputKeyClosestToWorldLocation(CurrentWorldLocation);
								const float TargetDist = Conn.TargetRoad->GetDistanceAlongSplineAtSplineInputKey(TargetKey);

								CurrentRoadSpline = Conn.TargetRoad;
								DistanceAlongSpline = TargetDist;
								CurrentLaneIndex = NewLane;
								TargetLaneOffsetCm = Conn.TargetRoad->GetLaneCenterOffset(NewLane);
								bIsChangingLanes = true;
								LaneChangeProgress = 0.0f;
								LaneChangeDuration = FMath::Clamp(TransitionLen / FMath::Max(100.0f, CurrentSpeedCmS), 1.5f, 3.5f);
								bTransitioned = true;
								break;
							}
						}
						if (bTransitioned)
						{
							break;
						}
					}
				}
			}

			if (!bTransitioned)
			{
				DistanceAlongSpline = FMath::Fmod(DistanceAlongSpline, SplineTotalLength);
			}
		}

		const FVector SplinePos = CurrentRoadSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
		const FVector SplineTangent = CurrentRoadSpline->GetTangentAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World).GetSafeNormal();
		const FVector SplineUp = CurrentRoadSpline->GetUpVectorAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
		const FVector SplineRight = FVector::CrossProduct(SplineTangent, SplineUp).GetSafeNormal();

		const FVector FinalLocation = SplinePos + (SplineRight * CurrentLaneOffsetCm);
		const FRotator FinalRotation = UKismetMathLibrary::MakeRotFromXZ(SplineTangent, SplineUp);

		SetActorLocationAndRotation(FinalLocation, FinalRotation);
	}
}

bool ATrafficVehicleAgent::FindLeadVehicle(float ScanDistanceCm, float& OutDistanceToLeadCm, float& OutRelativeSpeedCmS, ATrafficVehicleAgent*& OutLeadVehicle)
{
	OutDistanceToLeadCm = 50000.0f;
	OutRelativeSpeedCmS = 0.0f;
	OutLeadVehicle = nullptr;

	if (!CurrentRoadSpline.IsValid())
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

	const float SplineLen = CurrentRoadSpline->GetSplineLength();
	float NearestDist = ScanDistanceCm;
	ATrafficVehicleAgent* ClosestVeh = nullptr;

	const auto& Vehicles = TrafficSys->GetActiveVehicles();
	for (const auto& OtherPtr : Vehicles)
	{
		ATrafficVehicleAgent* OtherVeh = OtherPtr.Get();
		if (!OtherVeh || OtherVeh == this)
		{
			continue;
		}

		if (OtherVeh->CurrentRoadSpline != CurrentRoadSpline)
		{
			continue;
		}

		// Mismo carril
		if (OtherVeh->CurrentLaneIndex != CurrentLaneIndex)
		{
			continue;
		}

		// Calcular distancia longitudinal a lo largo del spline
		float DistAhead = 0.0f;
		if (OtherVeh->DistanceAlongSpline >= DistanceAlongSpline)
		{
			DistAhead = OtherVeh->DistanceAlongSpline - DistanceAlongSpline - OtherVeh->VehicleLengthCm;
		}
		else
		{
			// En caso de spline cerrado o bucle
			DistAhead = (SplineLen - DistanceAlongSpline + OtherVeh->DistanceAlongSpline) - OtherVeh->VehicleLengthCm;
		}

		if (DistAhead > 0.0f && DistAhead < NearestDist)
		{
			NearestDist = DistAhead;
			ClosestVeh = OtherVeh;
		}
	}

	if (ClosestVeh)
	{
		OutDistanceToLeadCm = NearestDist;
		OutRelativeSpeedCmS = CurrentSpeedCmS - ClosestVeh->CurrentSpeedCmS;
		OutLeadVehicle = ClosestVeh;
		return true;
	}

	return false;
}

float ATrafficVehicleAgent::CalculateIDMAcceleration(float DistanceToLeadCm, float RelativeSpeedCmS) const
{
	const float DesiredSpeedCmS = IDMParams.DesiredSpeedKmh * (100000.0f / 3600.0f);
	const float MaxAccel = IDMParams.MaxAccelerationCmS2;

	const float FreeRoadTerm = 1.0f - FMath::Pow(CurrentSpeedCmS / FMath::Max(1.0f, DesiredSpeedCmS), IDMParams.AccelerationExponent);

	float InteractionTerm = 0.0f;
	if (DistanceToLeadCm > 0.0f && DistanceToLeadCm < 40000.0f)
	{
		const float SqrtAB = 2.0f * FMath::Sqrt(MaxAccel * IDMParams.DesiredDecelerationCmS2);
		const float DynamicJamDistance = IDMParams.MinimumJamDistanceCm + (CurrentSpeedCmS * IDMParams.SafeTimeHeadwaySeconds) + ((CurrentSpeedCmS * RelativeSpeedCmS) / FMath::Max(1.0f, SqrtAB));

		InteractionTerm = FMath::Square(DynamicJamDistance / FMath::Max(1.0f, DistanceToLeadCm));
	}

	const float AccelResult = MaxAccel * (FreeRoadTerm - InteractionTerm);
	return FMath::Clamp(AccelResult, -IDMParams.EmergencyBrakingCmS2, MaxAccel);
}

void ATrafficVehicleAgent::UpdateDriverPsychology(float DeltaTime, float DistanceToLeadCm)
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (UEconomySubsystem* Economy = World->GetSubsystem<UEconomySubsystem>())
		{
			if (!Economy->IsDriverFrustrationEnabled())
			{
				ResetPsychology();
				return;
			}
		}
	}

	const float DesiredSpeedKmh = IDMParams.DesiredSpeedKmh;

	// 1. Dinamica de acumulacion y alivio del estres dF/dt
	if (CurrentSpeedKmh <= 10.0f)
	{
		// Detenido en atasco o avance a paso de tortuga
		WaitTimeInJam += DeltaTime;
		const float JamAccMult = (WaitTimeInJam > IndividualPatienceTolerance) ? 1.6f : 1.0f;
		FrustrationPercent = FMath::Clamp(FrustrationPercent + (PsychologyParams.JamStressRatePerSec * JamAccMult * DeltaTime), 0.0f, 100.0f);
	}
	else if (CurrentSpeedKmh < DesiredSpeedKmh * 0.45f)
	{
		// Circulando en trafico lento por estrechamiento u obras
		WaitTimeInJam += DeltaTime * 0.5f;
		FrustrationPercent = FMath::Clamp(FrustrationPercent + (PsychologyParams.SlowTrafficStressRatePerSec * DeltaTime), 0.0f, 100.0f);
	}
	else if (CurrentSpeedKmh >= DesiredSpeedKmh * 0.85f)
	{
		// Circulacion fluida: el conductor se relaja progresivamente
		WaitTimeInJam = FMath::FInterpTo(WaitTimeInJam, 0.0f, DeltaTime, 1.2f);
		FrustrationPercent = FMath::Clamp(FrustrationPercent - (PsychologyParams.FreeFlowReliefRatePerSec * DeltaTime), 0.0f, 100.0f);
	}

	// 2. Clasificacion del Estado Psicologico
	const ECondicionPsicologica PrevCondition = CondicionPsicologica;

	if (FrustrationPercent < 25.0f)
	{
		CondicionPsicologica = ECondicionPsicologica::Calmado;
		DriverMood = EDriverMood::Tranquilo;
		IDMParams.SafeTimeHeadwaySeconds = PsychologyParams.CalmSafeTimeHeadway; // 1.4s
		IDMParams.MinimumJamDistanceCm = PsychologyParams.CalmJamDistanceCm;     // 250 cm
		RearEndCollisionRiskMultiplier = 1.0f;
		bFlashingHeadlights = false;
		bHonkingHorn = false;
	}
	else if (FrustrationPercent < 50.0f)
	{
		CondicionPsicologica = ECondicionPsicologica::Impaciente;
		DriverMood = EDriverMood::Impaciente;
		IDMParams.SafeTimeHeadwaySeconds = PsychologyParams.ImpatientSafeTimeHeadway; // 0.9s
		IDMParams.MinimumJamDistanceCm = 180.0f;
		RearEndCollisionRiskMultiplier = 1.5f;
		bFlashingHeadlights = false;
		bHonkingHorn = false;
	}
	else if (FrustrationPercent < 75.0f)
	{
		CondicionPsicologica = ECondicionPsicologica::Estresado;
		DriverMood = EDriverMood::Irritado;
		IDMParams.SafeTimeHeadwaySeconds = PsychologyParams.StressedSafeTimeHeadway; // 0.5s
		IDMParams.MinimumJamDistanceCm = 100.0f;
		RearEndCollisionRiskMultiplier = 2.5f;

		// Tocar la bocina esporadicamente si esta totalmente detenido en atasco
		HornCooldownTimer += DeltaTime;
		if (CurrentSpeedKmh <= 3.0f && HornCooldownTimer > 7.0f)
		{
			HonkHorn();
			HornCooldownTimer = 0.0f;
		}
	}
	else
	{
		// FURIA AL VOLANTE (ROAD RAGE)
		CondicionPsicologica = ECondicionPsicologica::FuriaAlVolante;
		DriverMood = EDriverMood::FuriaAlVolante;

		// Consecuencia 1: Acoso trasero agresivo (tailgating critico a 0.2s y 40 cm)
		IDMParams.SafeTimeHeadwaySeconds = PsychologyParams.RageSafeTimeHeadway;
		IDMParams.MinimumJamDistanceCm = PsychologyParams.RageJamDistanceCm;

		// Consecuencia 2: Multiplicador x5 del riesgo de siniestro por alcance
		RearEndCollisionRiskMultiplier = PsychologyParams.RearEndCollisionMultiplier;

		// Consecuencia 3: Rafagas intermitentes de luces si va pegado a un coche delantero
		if (DistanceToLeadCm < 1500.0f && CurrentSpeedKmh > 20.0f)
		{
			HeadlightFlashTimer += DeltaTime;
			if (HeadlightFlashTimer >= 0.35f)
			{
				TriggerHeadlightFlash();
				HeadlightFlashTimer = 0.0f;
			}
		}

		// Consecuencia 4: Pitidos frecuentes de claxon en retencion
		HornCooldownTimer += DeltaTime;
		if (CurrentSpeedKmh <= 4.0f && HornCooldownTimer > 3.5f)
		{
			HonkHorn();
			HornCooldownTimer = 0.0f;
		}

		// Consecuencia 5: Probabilidad aumentada de colision violenta por alcance ante frenada brusca
		if (DistanceToLeadCm < 120.0f && CurrentSpeedCmS > 450.0f)
		{
			const float CollisionProbability = 0.04f * RearEndCollisionRiskMultiplier; // ~20%
			if (FMath::FRand() < CollisionProbability)
			{
				TriggerAccidentCollision(FVector(1.0f, 0.0f, 0.0f));
				UE_LOG(LogAutopistas, Error, TEXT("ACCIDENTE POR ALCANCE provocado por FURIA AL VOLANTE en vehiculo: %s!"), *GetName());
			}
		}
	}

	if (PrevCondition != CondicionPsicologica)
	{
		OnDriverConditionChanged.Broadcast(this, CondicionPsicologica);
		UE_LOG(LogAutopistas, Warning, TEXT("Vehiculo [%s] cambia de estado psicologico a: %d (Frustracion: %.1f%%, Espera en atasco: %.1fs)"),
			*GetName(), static_cast<int32>(CondicionPsicologica), FrustrationPercent, WaitTimeInJam);
	}
}

bool ATrafficVehicleAgent::EvaluateMOBILLaneChange(bool bWantsToOvertake)
{
	if (bIsChangingLanes)
	{
		return false;
	}

	ARoadSegmentActor* RoadActor = CurrentRoadSpline.IsValid() ? Cast<ARoadSegmentActor>(CurrentRoadSpline->GetOwner()) : nullptr;

	// Si queremos volver a la derecha (carril 0)
	if (CurrentLaneIndex == 1 && !bWantsToOvertake)
	{
		TargetLaneOffsetCm = 175.0f; // Carril derecho
		CurrentLaneIndex = 0;
		bIsChangingLanes = true;
		LaneChangeProgress = 0.0f;
		LaneChangeDuration = 2.0f;
		return true;
	}

	// Si queremos adelantar hacia la izquierda (carril 1)
	if (CurrentLaneIndex == 0 && bWantsToOvertake)
	{
		// Comprobar si el carril 1 esta cerrado por obras con conos reflectantes
		if (RoadActor && RoadActor->IsLaneClosed(1))
		{
			// El carril izquierdo esta en obras; adelantamiento prohibido
			return false;
		}

		TargetLaneOffsetCm = -175.0f; // Carril izquierdo
		CurrentLaneIndex = 1;
		bIsChangingLanes = true;
		LaneChangeProgress = 0.0f;

		// Conductor furioso hace un volantazo agresivo y rapido (1.1 segundos)
		LaneChangeDuration = (CondicionPsicologica == ECondicionPsicologica::FuriaAlVolante) ? 1.1f : 2.0f;
		return true;
	}

	return false;
}

void ATrafficVehicleAgent::TriggerHeadlightFlash()
{
	bFlashingHeadlights = true;
	bHeadlightsHighBeam = !bHeadlightsHighBeam;
	OnDriverHeadlightFlash.Broadcast(this);
}

void ATrafficVehicleAgent::HonkHorn()
{
	bHonkingHorn = true;
	OnDriverHonkHorn.Broadcast(this);
	UE_LOG(LogAutopistas, Verbose, TEXT("Vehiculo [%s]: ¡BOCINA! Conductor desesperado en atasco."), *GetName());
}

void ATrafficVehicleAgent::ResetPsychology()
{
	FrustrationPercent = 0.0f;
	WaitTimeInJam = 0.0f;
	CondicionPsicologica = ECondicionPsicologica::Calmado;
	DriverMood = EDriverMood::Tranquilo;
	IDMParams = BaseIDMParams;
	RearEndCollisionRiskMultiplier = 1.0f;
	bFlashingHeadlights = false;
	bHonkingHorn = false;
}

void ATrafficVehicleAgent::TriggerAccidentCollision(FVector ImpactDirection)
{
	if (IncidentState != EIncidentState::Colisionado)
	{
		IncidentState = EIncidentState::Colisionado;
		CurrentSpeedCmS = 0.0f;
		CurrentSpeedKmh = 0.0f;
		bFlashingHeadlights = false;
		bHonkingHorn = false;

		FRotator CurrentRot = GetActorRotation();
		CurrentRot.Yaw += FMath::FRandRange(-25.0f, 25.0f);
		SetActorRotation(CurrentRot);
	}
}

void ATrafficVehicleAgent::TriggerMechanicalBreakdown()
{
	if (IncidentState == EIncidentState::Normal)
	{
		IncidentState = EIncidentState::AveriadoArcen;
		TargetLaneOffsetCm = CurrentLaneOffsetCm + 250.0f; // Arcen
		bIsChangingLanes = true;
	}
}
