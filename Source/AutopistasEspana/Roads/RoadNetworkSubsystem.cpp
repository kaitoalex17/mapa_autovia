#include "Roads/RoadNetworkSubsystem.h"
#include "Roads/RoadSplineComponent.h"
#include "AutopistasEspana.h"

void URoadNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogAutopistas, Log, TEXT("URoadNetworkSubsystem inicializado con exito."));
}

void URoadNetworkSubsystem::Deinitialize()
{
	RegisteredRoads.Empty();
	Super::Deinitialize();
}

void URoadNetworkSubsystem::RegisterRoadSegment(URoadSplineComponent* RoadSegment)
{
	if (RoadSegment && !RegisteredRoads.Contains(RoadSegment))
	{
		RegisteredRoads.Add(RoadSegment);
		UE_LOG(LogAutopistas, Log, TEXT("Tramo vial registrado. Total tramos en red: %d"), RegisteredRoads.Num());
	}
}

void URoadNetworkSubsystem::UnregisterRoadSegment(URoadSplineComponent* RoadSegment)
{
	if (RoadSegment)
	{
		RegisteredRoads.Remove(RoadSegment);
		UE_LOG(LogAutopistas, Log, TEXT("Tramo vial retirado. Total tramos restantes: %d"), RegisteredRoads.Num());
	}
}

float URoadNetworkSubsystem::GetTotalNetworkLengthKm() const
{
	float TotalCentimeters = 0.0f;

	for (const auto& RoadPtr : RegisteredRoads)
	{
		if (RoadPtr.IsValid())
		{
			TotalCentimeters += RoadPtr->GetSplineLength();
		}
	}

	// 100.000 cm = 1 km
	return TotalCentimeters / 100000.0f;
}
