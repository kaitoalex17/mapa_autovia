#include "Roads/RoadSegmentActor.h"
#include "Roads/RoadSplineComponent.h"
#include "Roads/RoadNetworkSubsystem.h"
#include "ProceduralMeshComponent.h"
#include "Engine/World.h"
#include "AutopistasEspana.h"

ARoadSegmentActor::ARoadSegmentActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RoadSpline = CreateDefaultSubobject<URoadSplineComponent>(TEXT("RoadSpline"));
	RootComponent = RoadSpline;

	RoadMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RoadMesh"));
	RoadMesh->SetupAttachment(RootComponent);
	RoadMesh->bUseAsyncCooking = true;
}

void ARoadSegmentActor::BeginPlay()
{
	Super::BeginPlay();

	// Registrar automaticamente el tramo en la red global
	if (UWorld* World = GetWorld())
	{
		if (URoadNetworkSubsystem* NetworkSubsystem = World->GetSubsystem<URoadNetworkSubsystem>())
		{
			NetworkSubsystem->RegisterRoadSegment(RoadSpline);
		}
	}

	RebuildRoadGeometry();
}

void ARoadSegmentActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (URoadNetworkSubsystem* NetworkSubsystem = World->GetSubsystem<URoadNetworkSubsystem>())
		{
			NetworkSubsystem->UnregisterRoadSegment(RoadSpline);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ARoadSegmentActor::RebuildRoadGeometry()
{
	if (RoadSpline && RoadMesh)
	{
		RoadSpline->GenerateRoadMesh(RoadMesh);
	}
}

void ARoadSegmentActor::AddSplinePointAtWorldLocation(const FVector& WorldLocation)
{
	if (RoadSpline)
	{
		RoadSpline->AddSplinePoint(WorldLocation, ESplineCoordinateSpace::World);
		RebuildRoadGeometry();
	}
}

void ARoadSegmentActor::SetRoadCategory(ERoadCategory NewCategory)
{
	if (RoadSpline)
	{
		RoadSpline->RoadCategory = NewCategory;

		switch (NewCategory)
		{
		case ERoadCategory::Convencional_90:
			RoadSpline->CrossSection.NumLanesDirection = 1;
			RoadSpline->CrossSection.LaneWidth = 350.0f;
			RoadSpline->CrossSection.OuterShoulderWidth = 150.0f;
			RoadSpline->CrossSection.InnerShoulderWidth = 0.0f;
			RoadSpline->CrossSection.MedianWidth = 0.0f;
			RoadSpline->CrossSection.SpeedLimitKmh = 90.0f;
			break;

		case ERoadCategory::Autovia_120:
			RoadSpline->CrossSection.NumLanesDirection = 2;
			RoadSpline->CrossSection.LaneWidth = 350.0f;
			RoadSpline->CrossSection.OuterShoulderWidth = 250.0f;
			RoadSpline->CrossSection.InnerShoulderWidth = 100.0f;
			RoadSpline->CrossSection.MedianWidth = 200.0f;
			RoadSpline->CrossSection.SpeedLimitKmh = 120.0f;
			break;

		case ERoadCategory::Autopista_3x3:
			RoadSpline->CrossSection.NumLanesDirection = 3;
			RoadSpline->CrossSection.LaneWidth = 350.0f;
			RoadSpline->CrossSection.OuterShoulderWidth = 250.0f;
			RoadSpline->CrossSection.InnerShoulderWidth = 150.0f;
			RoadSpline->CrossSection.MedianWidth = 300.0f;
			RoadSpline->CrossSection.SpeedLimitKmh = 120.0f;
			break;

		default:
			break;
		}

		RebuildRoadGeometry();
	}
}
