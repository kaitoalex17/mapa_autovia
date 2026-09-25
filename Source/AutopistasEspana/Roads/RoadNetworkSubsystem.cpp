#include "Roads/RoadNetworkSubsystem.h"
#include "Roads/RoadSplineComponent.h"
#include "Roads/RoadSegmentActor.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "AutopistasEspana.h"

void URoadNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisteredRoads.Empty();
	RegisteredJunctions.Empty();
	UE_LOG(LogAutopistas, Log, TEXT("URoadNetworkSubsystem inicializado con exito."));
}

void URoadNetworkSubsystem::Deinitialize()
{
	RegisteredRoads.Empty();
	RegisteredJunctions.Empty();
	Super::Deinitialize();
}

void URoadNetworkSubsystem::RegisterRoadSegment(URoadSplineComponent* RoadSegment)
{
	if (RoadSegment && !RegisteredRoads.Contains(RoadSegment))
	{
		RegisteredRoads.Add(RoadSegment);
		UE_LOG(LogAutopistas, Log, TEXT("Tramo vial registrado. Total tramos en red: %d"), RegisteredRoads.Num());

		// Evaluar cruces a distinto nivel con vias existentes
		EvaluateOverpassesForRoad(RoadSegment);
	}
}

void URoadNetworkSubsystem::UnregisterRoadSegment(URoadSplineComponent* RoadSegment)
{
	if (RoadSegment)
	{
		RegisteredRoads.Remove(RoadSegment);

		// Eliminar de los nodos de enlace
		for (int32 i = RegisteredJunctions.Num() - 1; i >= 0; --i)
		{
			RegisteredJunctions[i].ConnectedRoads.Remove(RoadSegment);
			if (RegisteredJunctions[i].ConnectedRoads.Num() == 0)
			{
				RegisteredJunctions.RemoveAt(i);
			}
		}

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
	return TotalCentimeters / 100000.0f;
}

void URoadNetworkSubsystem::RegisterJunctionNode(const FRoadJunctionNode& JunctionNode)
{
	RegisteredJunctions.Add(JunctionNode);
	UE_LOG(LogAutopistas, Log, TEXT("Nodo de enlace [%s] registrado (Tipo: %d, Conexiones de carril: %d). Total enlaces: %d"),
		*JunctionNode.JunctionName.ToString(), static_cast<int32>(JunctionNode.JunctionType), JunctionNode.LaneConnections.Num(), RegisteredJunctions.Num());
}

void URoadNetworkSubsystem::UnregisterJunctionNode(const FGuid& JunctionId)
{
	RegisteredJunctions.RemoveAll([&JunctionId](const FRoadJunctionNode& Node)
	{
		return Node.JunctionId == JunctionId;
	});
}

bool URoadNetworkSubsystem::FindJunctionNodeById(const FGuid& JunctionId, FRoadJunctionNode& OutNode) const
{
	for (const auto& Node : RegisteredJunctions)
	{
		if (Node.JunctionId == JunctionId)
		{
			OutNode = Node;
			return true;
		}
	}
	return false;
}

bool URoadNetworkSubsystem::FindJunctionNodeAtLocation(const FVector& Location, float ToleranceCm, FRoadJunctionNode& OutNode) const
{
	const float TolSq = FMath::Square(ToleranceCm);
	for (const auto& Node : RegisteredJunctions)
	{
		if (FVector::DistSquared(Node.WorldLocation, Location) <= TolSq)
		{
			OutNode = Node;
			return true;
		}
	}
	return false;
}

TArray<FRoadJunctionNode> URoadNetworkSubsystem::GetJunctionsForRoad(URoadSplineComponent* Road) const
{
	TArray<FRoadJunctionNode> Result;
	if (!Road)
	{
		return Result;
	}

	for (const auto& Node : RegisteredJunctions)
	{
		if (Node.ConnectedRoads.Contains(Road))
		{
			Result.Add(Node);
		}
	}
	return Result;
}

int32 URoadNetworkSubsystem::GetNextConnectedLane(URoadSplineComponent* CurrentRoad, int32 CurrentLane, URoadSplineComponent* NextRoad) const
{
	float TransitionLength = 15000.0f;
	return GetNextConnectedLaneWithTransition(CurrentRoad, CurrentLane, NextRoad, TransitionLength);
}

int32 URoadNetworkSubsystem::GetNextConnectedLaneWithTransition(URoadSplineComponent* CurrentRoad, int32 CurrentLane, URoadSplineComponent* NextRoad, float& OutTransitionLength) const
{
	OutTransitionLength = 15000.0f; // Norma 8.1-IC: 150 metros por defecto

	if (!CurrentRoad || !NextRoad)
	{
		return 0; // Por defecto carril derecho
	}

	FLaneConnection FoundConn;
	if (GetLaneConnectionDetails(CurrentRoad, CurrentLane, NextRoad, FoundConn))
	{
		OutTransitionLength = FoundConn.TransitionLength;
		return FoundConn.TargetLaneIndex;
	}

	// Regla por defecto Norma 8.1-IC: incorporacion por la derecha (carril 0)
	const int32 TargetMaxLanes = NextRoad->CrossSection.NumLanesDirection;
	return FMath::Clamp(CurrentLane, 0, FMath::Max(0, TargetMaxLanes - 1));
}

bool URoadNetworkSubsystem::GetLaneConnectionDetails(URoadSplineComponent* CurrentRoad, int32 CurrentLane, URoadSplineComponent* NextRoad, FLaneConnection& OutConnection) const
{
	for (const auto& Junction : RegisteredJunctions)
	{
		if (Junction.ConnectedRoads.Contains(CurrentRoad) && Junction.ConnectedRoads.Contains(NextRoad))
		{
			for (const auto& Conn : Junction.LaneConnections)
			{
				if (Conn.SourceLaneIndex == CurrentLane && Conn.TargetRoad == NextRoad)
				{
					OutConnection = Conn;
					return true;
				}
			}
		}
	}
	return false;
}

FRoadJunctionNode URoadNetworkSubsystem::CreateMultiLaneRoundabout(const FVector& CenterLocation, float RadiusCm, float RadialArmLengthCm)
{
	FRoadJunctionNode RoundaboutNode;
	RoundaboutNode.JunctionName = FName(TEXT("Glorieta_Multicarril_Norma8.1-IC"));
	RoundaboutNode.JunctionType = EJunctionType::Roundabout;
	RoundaboutNode.WorldLocation = CenterLocation;

	UWorld* World = GetWorld();
	if (!World)
	{
		RegisterJunctionNode(RoundaboutNode);
		return RoundaboutNode;
	}

	// 1. Crear el anillo giratorio de 2 carriles
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARoadSegmentActor* RingActor = World->SpawnActor<ARoadSegmentActor>(ARoadSegmentActor::StaticClass(), CenterLocation, FRotator::ZeroRotator, SpawnParams);
	if (RingActor && RingActor->RoadSpline)
	{
		URoadSplineComponent* RingSpline = RingActor->RoadSpline;
		RingSpline->RoadCategory = ERoadCategory::Convencional_90;
		RingSpline->CrossSection.NumLanesDirection = 2; // 2 carriles giratorios
		RingSpline->CrossSection.LaneWidth = 400.0f; // 4.0m de anchura de carril en glorieta segun 8.1-IC
		RingSpline->CrossSection.SpeedLimitKmh = 50.0f;
		RingSpline->ClearSplinePoints();

		// Trazar circulo con 8 puntos tangenciales suaves
		const int32 NumPoints = 8;
		for (int32 i = 0; i < NumPoints; ++i)
		{
			const float Angle = (static_cast<float>(i) / static_cast<float>(NumPoints)) * 2.0f * PI;
			const FVector PointPos = CenterLocation + FVector(FMath::Cos(Angle) * RadiusCm, FMath::Sin(Angle) * RadiusCm, 0.0f);
			RingSpline->AddSplinePoint(PointPos, ESplineCoordinateSpace::World);
		}
		RingSpline->SetClosedLoop(true);
		RingActor->RebuildRoadGeometry();

		RegisterRoadSegment(RingSpline);
		RoundaboutNode.ConnectedRoads.Add(RingSpline);
	}

	// 2. Crear los 4 ramales radiales tangenciales (Norte = 0, Este = 1, Sur = 2, Oeste = 3)
	const float CardinalAngles[4] = { 0.0f, PI * 0.5f, PI, PI * 1.5f };
	for (int32 i = 0; i < 4; ++i)
	{
		const float Angle = CardinalAngles[i];
		const FVector RadialDir(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
		const FVector ArmStart = CenterLocation + (RadialDir * (RadiusCm + RadialArmLengthCm));
		const FVector ArmEnd = CenterLocation + (RadialDir * (RadiusCm + 350.0f));

		ARoadSegmentActor* ArmActor = World->SpawnActor<ARoadSegmentActor>(ARoadSegmentActor::StaticClass(), ArmStart, RadialDir.Rotation(), SpawnParams);
		if (ArmActor && ArmActor->RoadSpline)
		{
			URoadSplineComponent* ArmSpline = ArmActor->RoadSpline;
			ArmSpline->RoadCategory = ERoadCategory::RamalEnlace;
			ArmSpline->CrossSection.NumLanesDirection = 2; // 1 de entrada + 1 de salida
			ArmSpline->CrossSection.LaneWidth = 350.0f;
			ArmSpline->CrossSection.SpeedLimitKmh = 60.0f;
			ArmSpline->ClearSplinePoints();
			ArmSpline->AddSplinePoint(ArmStart, ESplineCoordinateSpace::World);
			ArmSpline->AddSplinePoint(ArmEnd, ESplineCoordinateSpace::World);
			ArmActor->RebuildRoadGeometry();

			RegisterRoadSegment(ArmSpline);
			RoundaboutNode.ConnectedRoads.Add(ArmSpline);

			// Conexiones de carril: carril 0 ramal -> carril 0 anillo (exterior)
			if (RoundaboutNode.ConnectedRoads.Num() > 0 && RoundaboutNode.ConnectedRoads[0])
			{
				RoundaboutNode.LaneConnections.Add(FLaneConnection(0, 0, RoundaboutNode.ConnectedRoads[0], 5000.0f, true, 40.0f));
				RoundaboutNode.LaneConnections.Add(FLaneConnection(1, 1, RoundaboutNode.ConnectedRoads[0], 5000.0f, true, 40.0f));
			}
		}
	}

	RegisterJunctionNode(RoundaboutNode);
	UE_LOG(LogAutopistas, Log, TEXT("Glorieta Multicarril generada con exito en (X=%.0f, Y=%.0f). 4 accesos radiales enlazados."), CenterLocation.X, CenterLocation.Y);
	return RoundaboutNode;
}

bool URoadNetworkSubsystem::DetectPlanCrossing(URoadSplineComponent* RoadA, URoadSplineComponent* RoadB, float& OutDistA, float& OutDistB, FVector& OutPointA, FVector& OutPointB) const
{
	OutDistA = 0.0f;
	OutDistB = 0.0f;
	OutPointA = FVector::ZeroVector;
	OutPointB = FVector::ZeroVector;

	if (!RoadA || !RoadB || RoadA == RoadB)
	{
		return false;
	}

	const float StepA = 500.0f; // Muestreo cada 5m
	const float StepB = 500.0f;
	const float LenA = RoadA->GetSplineLength();
	const float LenB = RoadB->GetSplineLength();

	float BestDistSq2D = 100000000.0f;
	float BestDistA = 0.0f;
	float BestDistB = 0.0f;

	for (float DistA = 0.0f; DistA < LenA; DistA += StepA)
	{
		const FVector PosA = RoadA->GetLocationAtDistanceAlongSpline(DistA, ESplineCoordinateSpace::World);
		for (float DistB = 0.0f; DistB < LenB; DistB += StepB)
		{
			const FVector PosB = RoadB->GetLocationAtDistanceAlongSpline(DistB, ESplineCoordinateSpace::World);
			const float DistSq2D = FMath::Square(PosA.X - PosB.X) + FMath::Square(PosA.Y - PosB.Y);

			if (DistSq2D < BestDistSq2D)
			{
				BestDistSq2D = DistSq2D;
				BestDistA = DistA;
				BestDistB = DistB;
			}
		}
	}

	// Umbral de cruce en planta: 4.0 metros (400 cm)
	if (BestDistSq2D <= FMath::Square(400.0f))
	{
		OutDistA = BestDistA;
		OutDistB = BestDistB;
		OutPointA = RoadA->GetLocationAtDistanceAlongSpline(BestDistA, ESplineCoordinateSpace::World);
		OutPointB = RoadB->GetLocationAtDistanceAlongSpline(BestDistB, ESplineCoordinateSpace::World);
		return true;
	}

	return false;
}

bool URoadNetworkSubsystem::CheckOverpassClearance(URoadSplineComponent* RoadA, URoadSplineComponent* RoadB, FVector& OutCrossingLocation, float& OutDeltaZ, bool& bOutIsGradeSeparated)
{
	float DistA = 0.0f, DistB = 0.0f;
	FVector PointA = FVector::ZeroVector, PointB = FVector::ZeroVector;

	if (!DetectPlanCrossing(RoadA, RoadB, DistA, DistB, PointA, PointB))
	{
		OutCrossingLocation = FVector::ZeroVector;
		OutDeltaZ = 0.0f;
		bOutIsGradeSeparated = false;
		return false;
	}

	OutCrossingLocation = (PointA + PointB) * 0.5f;
	OutDeltaZ = FMath::Abs(PointA.Z - PointB.Z);
	bOutIsGradeSeparated = (OutDeltaZ >= MITMA_MIN_OVERPASS_CLEARANCE_CM); // >= 5.50m

	if (bOutIsGradeSeparated)
	{
		// Identificar la via superior y configurarla como viaducto continuo sobre pilares
		URoadSplineComponent* UpperRoad = (PointA.Z >= PointB.Z) ? RoadA : RoadB;
		UpperRoad->bIsViaductFlyover = true;

		if (ARoadSegmentActor* UpperActor = Cast<ARoadSegmentActor>(UpperRoad->GetOwner()))
		{
			UpperActor->RebuildRoadGeometry();
		}

		UE_LOG(LogAutopistas, Log, TEXT("Paso a Distinto Nivel confirmado segun MITMA! Galibo vertical: %.1f m (>= 5.5 m). Sin colision a nivel."), OutDeltaZ / 100.0f);
	}
	else
	{
		UE_LOG(LogAutopistas, Warning, TEXT("Cruce a nivel critico detectado (Galibo: %.1f m < 5.5 m MITMA). Requiere glorieta, enlace o elevacion de rasante."), OutDeltaZ / 100.0f);
	}

	return true;
}

void URoadNetworkSubsystem::EvaluateOverpassesForRoad(URoadSplineComponent* NewRoad)
{
	if (!NewRoad)
	{
		return;
	}

	for (const auto& ExistingRoadPtr : RegisteredRoads)
	{
		URoadSplineComponent* ExistingRoad = ExistingRoadPtr.Get();
		if (!ExistingRoad || ExistingRoad == NewRoad)
		{
			continue;
		}

		FVector CrossingLoc;
		float DeltaZ = 0.0f;
		bool bGradeSep = false;

		if (CheckOverpassClearance(NewRoad, ExistingRoad, CrossingLoc, DeltaZ, bGradeSep) && bGradeSep)
		{
			FRoadJunctionNode FlyoverNode;
			FlyoverNode.JunctionName = FName(TEXT("Paso_Distinto_Nivel_MITMA"));
			FlyoverNode.JunctionType = EJunctionType::OverpassFlyover;
			FlyoverNode.WorldLocation = CrossingLoc;
			FlyoverNode.ConnectedRoads.Add(NewRoad);
			FlyoverNode.ConnectedRoads.Add(ExistingRoad);
			RegisterJunctionNode(FlyoverNode);
		}
	}
}

int32 URoadNetworkSubsystem::EvaluateRoadNetworkOverpasses()
{
	int32 OverpassCount = 0;
	const int32 NumRoads = RegisteredRoads.Num();

	for (int32 i = 0; i < NumRoads; ++i)
	{
		URoadSplineComponent* RoadA = RegisteredRoads[i].Get();
		if (!RoadA) continue;

		for (int32 j = i + 1; j < NumRoads; ++j)
		{
			URoadSplineComponent* RoadB = RegisteredRoads[j].Get();
			if (!RoadB) continue;

			FVector CrossingLoc;
			float DeltaZ = 0.0f;
			bool bGradeSep = false;

			if (CheckOverpassClearance(RoadA, RoadB, CrossingLoc, DeltaZ, bGradeSep) && bGradeSep)
			{
				OverpassCount++;
			}
		}
	}

	return OverpassCount;
}
