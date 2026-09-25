#include "Roads/RoadSplineComponent.h"
#include "ProceduralMeshComponent.h"

URoadSplineComponent::URoadSplineComponent()
{
	bDrawDebug = true;
	SetClosedLoop(false);

	// Por defecto inicializamos como Autovia 120 (2x2)
	CrossSection.NumLanesDirection = 2;
	CrossSection.LaneWidth = 350.0f;
	CrossSection.OuterShoulderWidth = 250.0f;
	CrossSection.InnerShoulderWidth = 100.0f;
	CrossSection.MedianWidth = 200.0f;
	CrossSection.SpeedLimitKmh = 120.0f;
}

ERoadElevationType URoadSplineComponent::GetElevationTypeAtDistance(float DistanceAlongSpline) const
{
	if (bIsViaductFlyover)
	{
		return ERoadElevationType::ViaductoPuente;
	}

	const FVector WorldPos = GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	
	if (WorldPos.Z > ViaductHeightThreshold)
	{
		return ERoadElevationType::ViaductoPuente;
	}
	else if (WorldPos.Z < -ViaductHeightThreshold)
	{
		return ERoadElevationType::TunelSubterraneo;
	}

	return ERoadElevationType::RasanteTerreno;
}

float URoadSplineComponent::GetLaneCenterOffset(int32 LaneIndex) const
{
	return CrossSection.GetLaneCenterOffset(LaneIndex);
}

float URoadSplineComponent::GetCarriagewayWidthAtDistance(float DistanceAlongSpline, float& OutLeftWidth, float& OutRightWidth) const
{
	const float BaseCarriageway = CrossSection.GetOneWayCarriagewayWidth();
	const float HalfMedian = CrossSection.MedianWidth * 0.5f;

	OutLeftWidth = HalfMedian + BaseCarriageway;
	OutRightWidth = HalfMedian + BaseCarriageway;

	if (bHasAccelerationTaper && AccelerationTaperLength > 0.0f)
	{
		const float SplineLen = GetSplineLength();
		const float TaperStart = bTaperAtEnd ? FMath::Max(0.0f, SplineLen - AccelerationTaperLength) : 0.0f;
		const float TaperEnd = bTaperAtEnd ? SplineLen : FMath::Min(SplineLen, AccelerationTaperLength);

		if (DistanceAlongSpline >= TaperStart && DistanceAlongSpline <= TaperEnd)
		{
			float Alpha = (DistanceAlongSpline - TaperStart) / AccelerationTaperLength;
			if (!bTaperAtEnd)
			{
				Alpha = 1.0f - Alpha; // Al inicio de salida se ensancha
			}

			// Factor lineal o cuadratico suave de cuna segun Norma 8.1-IC
			// TaperFactor va de 1.0 (ancho integro) a 0.0 (carril desvanecido en el merge)
			const float TaperFactor = 1.0f - FMath::Clamp(Alpha, 0.0f, 1.0f);

			// Estrechamiento progresivo del carril exterior derecho en la incorporacion
			OutRightWidth = HalfMedian + (CrossSection.NumLanesDirection * CrossSection.LaneWidth * TaperFactor) + (CrossSection.OuterShoulderWidth * TaperFactor);
			OutRightWidth = FMath::Max(OutRightWidth, 15.0f);
		}
	}

	return OutLeftWidth + OutRightWidth;
}

void URoadSplineComponent::EnableAccelerationTaper(bool bEnable, float Length, bool bAtEnd)
{
	bHasAccelerationTaper = bEnable;
	AccelerationTaperLength = Length;
	bTaperAtEnd = bAtEnd;
}

void URoadSplineComponent::ConnectSplineEndToRoadLane(bool bConnectStart, URoadSplineComponent* TargetRoad, int32 TargetLaneIndex, float TargetDistanceAlongSpline)
{
	if (!TargetRoad)
	{
		return;
	}

	const int32 PointIndex = bConnectStart ? 0 : FMath::Max(0, GetNumberOfSplinePoints() - 1);
	const FVector CurrentWorldPos = GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);

	float TargetDist = TargetDistanceAlongSpline;
	if (TargetDist < 0.0f)
	{
		const float InputKey = TargetRoad->FindInputKeyClosestToWorldLocation(CurrentWorldPos);
		TargetDist = TargetRoad->GetDistanceAlongSplineAtSplineInputKey(InputKey);
	}

	const FVector TargetCenter = TargetRoad->GetLocationAtDistanceAlongSpline(TargetDist, ESplineCoordinateSpace::World);
	const FVector TargetTangent = TargetRoad->GetTangentAtDistanceAlongSpline(TargetDist, ESplineCoordinateSpace::World);
	const FVector TargetUp = TargetRoad->GetUpVectorAtDistanceAlongSpline(TargetDist, ESplineCoordinateSpace::World);
	const FVector TargetRight = FVector::CrossProduct(TargetTangent.GetSafeNormal(), TargetUp).GetSafeNormal();

	const float LaneOffset = TargetRoad->GetLaneCenterOffset(TargetLaneIndex);
	const FVector TargetLaneWorldPos = TargetCenter + (TargetRight * LaneOffset);

	const FVector LocalPos = GetComponentTransform().InverseTransformPosition(TargetLaneWorldPos);
	const FVector LocalTangent = GetComponentTransform().InverseTransformVector(TargetTangent);

	SetLocationAtSplinePoint(PointIndex, LocalPos, ESplineCoordinateSpace::Local);
	SetTangentAtSplinePoint(PointIndex, LocalTangent, ESplineCoordinateSpace::Local);
	SetSplinePointType(PointIndex, ESplinePointType::CurveCustomTangent);
	UpdateSpline();
}

void URoadSplineComponent::ConnectStartToLane(URoadSplineComponent* TargetRoad, int32 TargetLaneIndex, float TargetDistanceAlongSpline)
{
	ConnectSplineEndToRoadLane(true, TargetRoad, TargetLaneIndex, TargetDistanceAlongSpline);
}

void URoadSplineComponent::ConnectEndToLane(URoadSplineComponent* TargetRoad, int32 TargetLaneIndex, float TargetDistanceAlongSpline)
{
	ConnectSplineEndToRoadLane(false, TargetRoad, TargetLaneIndex, TargetDistanceAlongSpline);
}

void URoadSplineComponent::GenerateRoadMesh(UProceduralMeshComponent* TargetMesh)
{
	if (!TargetMesh)
	{
		return;
	}

	const float TotalSplineLength = GetSplineLength();
	if (TotalSplineLength <= 100.0f)
	{
		return;
	}

	// Malla de la calzada / firme asfaltico (Seccion 0)
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;

	// Malla de la estructura de viaducto / pilares de hormigon (Seccion 1)
	TArray<FVector> StructureVertices;
	TArray<int32> StructureTriangles;
	TArray<FVector> StructureNormals;
	TArray<FVector2D> StructureUV0;
	TArray<FProcMeshTangent> StructureTangents;

	const int32 NumSteps = FMath::Max(2, FMath::CeilToInt(TotalSplineLength / SegmentStepLength));
	const float ActualStepLength = TotalSplineLength / static_cast<float>(NumSteps);

	float LastPillarDistance = -ViaductPillarSpacing;

	// Generar secciones transversales a lo largo del spline
	for (int32 i = 0; i <= NumSteps; ++i)
	{
		const float CurrentDistance = i * ActualStepLength;
		const FVector CenterLocation = GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::Local);
		const FVector SplineTangent = GetTangentAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::Local).GetSafeNormal();
		const FVector SplineUp = GetUpVectorAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::Local);
		const FVector SplineRight = FVector::CrossProduct(SplineTangent, SplineUp).GetSafeNormal();

		// Anchos laterales considerando cunas de aceleracion (Norma 8.1-IC)
		float LeftWidth = 0.0f;
		float RightWidth = 0.0f;
		GetCarriagewayWidthAtDistance(CurrentDistance, LeftWidth, RightWidth);

		// Coordenada V de textura (cada 4 metros repite textura y balizamiento longitudinal)
		const float VCoord = CurrentDistance / 400.0f;

		// Vertice 0: Borde exterior izquierdo
		const FVector LeftEdge = CenterLocation - (SplineRight * LeftWidth);
		Vertices.Add(LeftEdge);
		Normals.Add(SplineUp);
		UV0.Add(FVector2D(0.0f, VCoord));
		Tangents.Add(FProcMeshTangent(SplineTangent, false));

		// Vertice 1: Eje central de la via
		Vertices.Add(CenterLocation);
		Normals.Add(SplineUp);
		UV0.Add(FVector2D(0.5f, VCoord));
		Tangents.Add(FProcMeshTangent(SplineTangent, false));

		// Vertice 2: Borde exterior derecho (con cuna si procede)
		const FVector RightEdge = CenterLocation + (SplineRight * RightWidth);
		Vertices.Add(RightEdge);
		Normals.Add(SplineUp);
		UV0.Add(FVector2D(1.0f, VCoord));
		Tangents.Add(FProcMeshTangent(SplineTangent, false));

		// Comprobacion de estructura elevada (Viaducto / Flyover sobre pilares)
		const ERoadElevationType Elevation = GetElevationTypeAtDistance(CurrentDistance);
		if (Elevation == ERoadElevationType::ViaductoPuente)
		{
			// Generar pilares de hormigon cada ViaductPillarSpacing
			if ((CurrentDistance - LastPillarDistance) >= ViaductPillarSpacing && CenterLocation.Z > (ViaductDeckThickness + 50.0f))
			{
				LastPillarDistance = CurrentDistance;

				const float PillarHalfWidth = 100.0f; // Pilar de 2.0m x 2.0m
				const FVector PillarTopCenter = CenterLocation - (SplineUp * ViaductDeckThickness);
				const float GroundZ = 0.0f; // Nivel del suelo en espacio local
				const float PillarHeight = PillarTopCenter.Z - GroundZ;

				if (PillarHeight > 100.0f)
				{
					const int32 BaseIdx = StructureVertices.Num();

					// 4 vertices en la base
					const FVector B0 = FVector(PillarTopCenter.X - PillarHalfWidth, PillarTopCenter.Y - PillarHalfWidth, GroundZ);
					const FVector B1 = FVector(PillarTopCenter.X + PillarHalfWidth, PillarTopCenter.Y - PillarHalfWidth, GroundZ);
					const FVector B2 = FVector(PillarTopCenter.X + PillarHalfWidth, PillarTopCenter.Y + PillarHalfWidth, GroundZ);
					const FVector B3 = FVector(PillarTopCenter.X - PillarHalfWidth, PillarTopCenter.Y + PillarHalfWidth, GroundZ);

					// 4 vertices en la coronacion del pilar
					const FVector T0 = FVector(B0.X, B0.Y, PillarTopCenter.Z);
					const FVector T1 = FVector(B1.X, B1.Y, PillarTopCenter.Z);
					const FVector T2 = FVector(B2.X, B2.Y, PillarTopCenter.Z);
					const FVector T3 = FVector(B3.X, B3.Y, PillarTopCenter.Z);

					// Cara 1: Frontal (-Y)
					StructureVertices.Add(B0); StructureVertices.Add(B1); StructureVertices.Add(T1); StructureVertices.Add(T0);
					for (int32 k = 0; k < 4; ++k) { StructureNormals.Add(-FVector::ForwardVector); StructureTangents.Add(FProcMeshTangent(FVector::RightVector, false)); }
					StructureUV0.Add(FVector2D(0, 0)); StructureUV0.Add(FVector2D(1, 0)); StructureUV0.Add(FVector2D(1, PillarHeight / 200.0f)); StructureUV0.Add(FVector2D(0, PillarHeight / 200.0f));
					StructureTriangles.Add(BaseIdx + 0); StructureTriangles.Add(BaseIdx + 1); StructureTriangles.Add(BaseIdx + 2);
					StructureTriangles.Add(BaseIdx + 0); StructureTriangles.Add(BaseIdx + 2); StructureTriangles.Add(BaseIdx + 3);

					// Cara 2: Derecha (+X)
					const int32 C2 = StructureVertices.Num();
					StructureVertices.Add(B1); StructureVertices.Add(B2); StructureVertices.Add(T2); StructureVertices.Add(T1);
					for (int32 k = 0; k < 4; ++k) { StructureNormals.Add(FVector::RightVector); StructureTangents.Add(FProcMeshTangent(FVector::ForwardVector, false)); }
					StructureUV0.Add(FVector2D(0, 0)); StructureUV0.Add(FVector2D(1, 0)); StructureUV0.Add(FVector2D(1, PillarHeight / 200.0f)); StructureUV0.Add(FVector2D(0, PillarHeight / 200.0f));
					StructureTriangles.Add(C2 + 0); StructureTriangles.Add(C2 + 1); StructureTriangles.Add(C2 + 2);
					StructureTriangles.Add(C2 + 0); StructureTriangles.Add(C2 + 2); StructureTriangles.Add(C2 + 3);

					// Cara 3: Trasera (+Y)
					const int32 C3 = StructureVertices.Num();
					StructureVertices.Add(B2); StructureVertices.Add(B3); StructureVertices.Add(T3); StructureVertices.Add(T2);
					for (int32 k = 0; k < 4; ++k) { StructureNormals.Add(FVector::ForwardVector); StructureTangents.Add(FProcMeshTangent(-FVector::RightVector, false)); }
					StructureUV0.Add(FVector2D(0, 0)); StructureUV0.Add(FVector2D(1, 0)); StructureUV0.Add(FVector2D(1, PillarHeight / 200.0f)); StructureUV0.Add(FVector2D(0, PillarHeight / 200.0f));
					StructureTriangles.Add(C3 + 0); StructureTriangles.Add(C3 + 1); StructureTriangles.Add(C3 + 2);
					StructureTriangles.Add(C3 + 0); StructureTriangles.Add(C3 + 2); StructureTriangles.Add(C3 + 3);

					// Cara 4: Izquierda (-X)
					const int32 C4 = StructureVertices.Num();
					StructureVertices.Add(B3); StructureVertices.Add(B0); StructureVertices.Add(T0); StructureVertices.Add(T3);
					for (int32 k = 0; k < 4; ++k) { StructureNormals.Add(-FVector::RightVector); StructureTangents.Add(FProcMeshTangent(-FVector::ForwardVector, false)); }
					StructureUV0.Add(FVector2D(0, 0)); StructureUV0.Add(FVector2D(1, 0)); StructureUV0.Add(FVector2D(1, PillarHeight / 200.0f)); StructureUV0.Add(FVector2D(0, PillarHeight / 200.0f));
					StructureTriangles.Add(C4 + 0); StructureTriangles.Add(C4 + 1); StructureTriangles.Add(C4 + 2);
					StructureTriangles.Add(C4 + 0); StructureTriangles.Add(C4 + 2); StructureTriangles.Add(C4 + 3);
				}
			}
		}
	}

	// Triangulacion de la calzada (2 franjas de quads: izquierda y derecha)
	for (int32 i = 0; i < NumSteps; ++i)
	{
		const int32 RowA = i * 3;
		const int32 RowB = (i + 1) * 3;

		// Quad Izquierdo (LeftEdge -> Center)
		Triangles.Add(RowA + 0);
		Triangles.Add(RowB + 0);
		Triangles.Add(RowA + 1);

		Triangles.Add(RowA + 1);
		Triangles.Add(RowB + 0);
		Triangles.Add(RowB + 1);

		// Quad Derecho (Center -> RightEdge con cuna)
		Triangles.Add(RowA + 1);
		Triangles.Add(RowB + 1);
		Triangles.Add(RowA + 2);

		Triangles.Add(RowA + 2);
		Triangles.Add(RowB + 1);
		Triangles.Add(RowB + 2);
	}

	TargetMesh->ClearAllMeshSections();

	// Seccion 0: Calzada y Firme Asfaltico
	TargetMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	TargetMesh->SetMeshSectionCastsShadow(0, true);

	// Seccion 1: Pilares y Estructura de Viaducto (si existen)
	if (StructureVertices.Num() > 0)
	{
		TargetMesh->CreateMeshSection(1, StructureVertices, StructureTriangles, StructureNormals, StructureUV0, TArray<FColor>(), StructureTangents, true);
		TargetMesh->SetMeshSectionCastsShadow(1, true);
	}
}
