#include "Traffic/TrafficVehicleAgent.h"
#include "Components/StaticMeshComponent.h"
#include "Roads/RoadSplineComponent.h"
#include "Kismet/KismetMathLibrary.h"

ATrafficVehicleAgent::ATrafficVehicleAgent()
{
	PrimaryActorTick.bCanEverTick = true;

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	RootComponent = VehicleMesh;
	VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));
	VehicleMesh->SetGenerateOverlapEvents(true);

	// Valores iniciales por defecto (Turismo)
	IDMParams.DesiredSpeedKmh = 120.0f;
	IDMParams.SafeTimeHeadwaySeconds = 1.4f;
	IDMParams.MinimumJamDistanceCm = 250.0f;
	IDMParams.MaxAccelerationCmS2 = 280.0f;
	IDMParams.DesiredDecelerationCmS2 = 350.0f;
	IDMParams.EmergencyBrakingCmS2 = 750.0f;
}

void ATrafficVehicleAgent::BeginPlay()
{
	Super::BeginPlay();
	
	// Convertir km/h a cm/s para calculos fisicos internos
	// 1 km/h = 1000 m / 3600 s = (1000 * 100 cm) / 3600 s = 27.7778 cm/s
	CurrentSpeedCmS = (IDMParams.DesiredSpeedKmh * 0.8f) * (100000.0f / 3600.0f);
	CurrentSpeedKmh = CurrentSpeedCmS * (3600.0f / 100000.0f);
}

void ATrafficVehicleAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Si el vehiculo esta colisionado o averiado, no avanza; permanece bloqueando el carril
	if (IncidentState == EIncidentState::Colisionado || IncidentState == EIncidentState::AveriadoArcen)
	{
		CurrentSpeedCmS = 0.0f;
		CurrentSpeedKmh = 0.0f;
		return;
	}

	// 1. Logica de cambio de carril suave (interpolacion lateral)
	if (bIsChangingLanes)
	{
		LaneChangeProgress += DeltaTime / LaneChangeDuration;
		CurrentLaneOffsetCm = FMath::FInterpTo(CurrentLaneOffsetCm, TargetLaneOffsetCm, DeltaTime, 4.0f);

		if (LaneChangeProgress >= 1.0f || FMath::IsNearlyEqual(CurrentLaneOffsetCm, TargetLaneOffsetCm, 2.0f))
		{
			CurrentLaneOffsetCm = TargetLaneOffsetCm;
			bIsChangingLanes = false;
			LaneChangeProgress = 0.0f;
		}
	}

	// 2. Si no hay obstaculo delante, acelerar hacia la velocidad deseada con IDM
	const float Acceleration = CalculateIDMAcceleration(50000.0f, 0.0f); // Sin vehiculo lider proximo
	CurrentSpeedCmS = FMath::Clamp(CurrentSpeedCmS + (Acceleration * DeltaTime), 0.0f, IDMParams.DesiredSpeedKmh * (100000.0f / 3600.0f));
	CurrentSpeedKmh = CurrentSpeedCmS * (3600.0f / 100000.0f);

	// 2.1. Actualizacion de Psicologia y Frustracion del Conductor
	const float DesiredSpeedKmh = IDMParams.DesiredSpeedKmh;
	if (CurrentSpeedKmh < DesiredSpeedKmh * 0.4f)
	{
		// Si va a menos del 40% de la velocidad deseada o esta parado en atasco, se frustra
		const float JamSeverity = (CurrentSpeedKmh <= 5.0f) ? 4.5f : 1.8f;
		FrustrationPercent = FMath::Clamp(FrustrationPercent + (JamSeverity * DeltaTime), 0.0f, 100.0f);
	}
	else
	{
		// Si circula fluido, se relaja progresivamente
		FrustrationPercent = FMath::Clamp(FrustrationPercent - (2.0f * DeltaTime), 0.0f, 100.0f);
	}

	// Clasificar nivel de humor
	if (FrustrationPercent < 25.0f)
	{
		DriverMood = EDriverMood::Tranquilo;
	}
	else if (FrustrationPercent < 50.0f)
	{
		DriverMood = EDriverMood::Impaciente;
	}
	else if (FrustrationPercent < 75.0f)
	{
		DriverMood = EDriverMood::Irritado;
	}
	else
	{
		DriverMood = EDriverMood::FuriaAlVolante;
	}

	// 3. Avanzar a lo largo del spline
	if (CurrentRoadSpline.IsValid())
	{
		DistanceAlongSpline += CurrentSpeedCmS * DeltaTime;
		const float SplineTotalLength = CurrentRoadSpline->GetSplineLength();

		// Si llegamos al final del tramo
		if (DistanceAlongSpline >= SplineTotalLength)
		{
			DistanceAlongSpline = FMath::Fmod(DistanceAlongSpline, SplineTotalLength);
		}

		// Posicion y Orientacion sobre el spline
		const FVector SplinePos = CurrentRoadSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
		const FVector SplineTangent = CurrentRoadSpline->GetTangentAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World).GetSafeNormal();
		const FVector SplineUp = CurrentRoadSpline->GetUpVectorAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
		const FVector SplineRight = FVector::CrossProduct(SplineTangent, SplineUp).GetSafeNormal();

		// Desplazar lateralmente segun el carril ocupado
		const FVector FinalLocation = SplinePos + (SplineRight * CurrentLaneOffsetCm);
		const FRotator FinalRotation = UKismetMathLibrary::MakeRotFromXZ(SplineTangent, SplineUp);

		SetActorLocationAndRotation(FinalLocation, FinalRotation);
	}
}

float ATrafficVehicleAgent::CalculateIDMAcceleration(float DistanceToLeadCm, float RelativeSpeedCmS) const
{
	const float DesiredSpeedCmS = IDMParams.DesiredSpeedKmh * (100000.0f / 3600.0f);
	const float MaxAccel = IDMParams.MaxAccelerationCmS2;

	// Termino de via libre: a * [1 - (v / v0)^delta]
	const float FreeRoadTerm = 1.0f - FMath::Pow(CurrentSpeedCmS / FMath::Max(1.0f, DesiredSpeedCmS), IDMParams.AccelerationExponent);

	// Termino de interaccion con el vehiculo de delante
	float InteractionTerm = 0.0f;
	if (DistanceToLeadCm > 0.0f)
	{
		// s*(v, delta_v) = s0 + v*T + (v * delta_v) / (2 * sqrt(a * b))
		const float SqrtAB = 2.0f * FMath::Sqrt(MaxAccel * IDMParams.DesiredDecelerationCmS2);
		const float DynamicJamDistance = IDMParams.MinimumJamDistanceCm + (CurrentSpeedCmS * IDMParams.SafeTimeHeadwaySeconds) + ((CurrentSpeedCmS * RelativeSpeedCmS) / FMath::Max(1.0f, SqrtAB));

		InteractionTerm = FMath::Square(DynamicJamDistance / FMath::Max(1.0f, DistanceToLeadCm));
	}

	const float AccelResult = MaxAccel * (FreeRoadTerm - InteractionTerm);

	// Limitar a la frenada maxima de emergencia
	return FMath::Clamp(AccelResult, -IDMParams.EmergencyBrakingCmS2, MaxAccel);
}

bool ATrafficVehicleAgent::EvaluateMOBILLaneChange(bool bWantsToOvertake)
{
	if (bIsChangingLanes)
	{
		return false;
	}

	// Regla DGT: Si estamos en el carril izquierdo (1) y no hay nadie delante, volver inmediatamente al carril derecho (0)
	if (CurrentLaneIndex == 1 && !bWantsToOvertake)
	{
		TargetLaneOffsetCm = 175.0f; // Carril derecho
		CurrentLaneIndex = 0;
		bIsChangingLanes = true;
		LaneChangeProgress = 0.0f;
		return true;
	}

	// Si estamos en el carril derecho (0) y queremos adelantar a un coche lento
	if (CurrentLaneIndex == 0 && bWantsToOvertake)
	{
		TargetLaneOffsetCm = -175.0f; // Carril izquierdo (adelantamiento)
		CurrentLaneIndex = 1;
		bIsChangingLanes = true;
		LaneChangeProgress = 0.0f;
		return true;
	}

	return false;
}

void ATrafficVehicleAgent::TriggerAccidentCollision(FVector ImpactDirection)
{
	if (IncidentState != EIncidentState::Colisionado)
	{
		IncidentState = EIncidentState::Colisionado;
		CurrentSpeedCmS = 0.0f;
		CurrentSpeedKmh = 0.0f;

		// Rotar ligeramente simulando el golpe
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
		// Mover lateralmente hacia el arcen exterior (+350 UU)
		TargetLaneOffsetCm = CurrentLaneOffsetCm + 250.0f;
		bIsChangingLanes = true;
	}
}
