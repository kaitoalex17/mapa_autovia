#include "Traffic/TrafficSimulationSubsystem.h"
#include "Traffic/TrafficVehicleAgent.h"
#include "Roads/RoadSplineComponent.h"
#include "Engine/World.h"
#include "AutopistasEspana.h"

void UTrafficSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogAutopistas, Log, TEXT("UTrafficSimulationSubsystem inicializado con exito."));
}

void UTrafficSimulationSubsystem::Deinitialize()
{
	ActiveVehicles.Empty();
	Super::Deinitialize();
}

void UTrafficSimulationSubsystem::RegisterVehicle(ATrafficVehicleAgent* Vehicle)
{
	if (Vehicle && !ActiveVehicles.Contains(Vehicle))
	{
		ActiveVehicles.Add(Vehicle);
	}
}

void UTrafficSimulationSubsystem::UnregisterVehicle(ATrafficVehicleAgent* Vehicle)
{
	if (Vehicle)
	{
		ActiveVehicles.Remove(Vehicle);
	}
}

ATrafficVehicleAgent* UTrafficSimulationSubsystem::SpawnVehicleOnRoad(TSubclassOf<ATrafficVehicleAgent> VehicleClass, URoadSplineComponent* Road, float StartDistance, int32 LaneIndex)
{
	if (!Road || !VehicleClass || !GetWorld())
	{
		return nullptr;
	}

	const FVector SpawnLoc = Road->GetLocationAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::World);
	const FRotator SpawnRot = Road->GetRotationAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::World);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATrafficVehicleAgent* NewVehicle = GetWorld()->SpawnActor<ATrafficVehicleAgent>(VehicleClass, SpawnLoc, SpawnRot, SpawnParams);
	if (NewVehicle)
	{
		NewVehicle->CurrentRoadSpline = Road;
		NewVehicle->DistanceAlongSpline = StartDistance;
		NewVehicle->CurrentLaneIndex = LaneIndex;
		NewVehicle->CurrentLaneOffsetCm = (LaneIndex == 0) ? 175.0f : -175.0f; // Carril derecho (+1.75m) o izquierdo (-1.75m)
		RegisterVehicle(NewVehicle);
	}

	return NewVehicle;
}

void UTrafficSimulationSubsystem::TriggerRandomAccident()
{
	if (ActiveVehicles.Num() == 0)
	{
		return;
	}

	// Seleccionar un vehiculo aleatorio que este circulando normalmente
	TArray<ATrafficVehicleAgent*> NormalVehicles;
	for (const auto& VehPtr : ActiveVehicles)
	{
		if (VehPtr.IsValid() && VehPtr->IncidentState == EIncidentState::Normal)
		{
			NormalVehicles.Add(VehPtr.Get());
		}
	}

	if (NormalVehicles.Num() > 0)
	{
		const int32 RandomIdx = FMath::RandRange(0, NormalVehicles.Num() - 1);
		NormalVehicles[RandomIdx]->TriggerAccidentCollision(FVector(1.0f, 0.0f, 0.0f));
		UE_LOG(LogAutopistas, Warning, TEXT("Accidente aleatorio provocado en vehiculo: %s"), *NormalVehicles[RandomIdx]->GetName());
	}
}

int32 UTrafficSimulationSubsystem::GetActiveAccidentsCount() const
{
	int32 Accidents = 0;
	for (const auto& VehPtr : ActiveVehicles)
	{
		if (VehPtr.IsValid() && VehPtr->IncidentState == EIncidentState::Colisionado)
		{
			++Accidents;
		}
	}
	return Accidents;
}

float UTrafficSimulationSubsystem::GetAverageNetworkSpeedKmh() const
{
	if (ActiveVehicles.Num() == 0)
	{
		return 120.0f;
	}

	float SpeedSum = 0.0f;
	int32 ValidCount = 0;

	for (const auto& VehPtr : ActiveVehicles)
	{
		if (VehPtr.IsValid())
		{
			SpeedSum += VehPtr->CurrentSpeedKmh;
			++ValidCount;
		}
	}

	return ValidCount > 0 ? (SpeedSum / ValidCount) : 120.0f;
}

float UTrafficSimulationSubsystem::GetCongestionPercentage() const
{
	const float AvgSpeed = GetAverageNetworkSpeedKmh();
	const float FreeFlowSpeed = 120.0f;

	// Si la velocidad media cae por debajo de 30 km/h, la congestion es del 100%
	const float Ratio = FMath::Clamp(AvgSpeed / FreeFlowSpeed, 0.0f, 1.0f);
	return (1.0f - Ratio) * 100.0f;
}
