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
	FVector WorldPos = GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	
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

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;

	// Anchuras
	const float HalfCarriagewayWidth = CrossSection.GetOneWayCarriagewayWidth();
	const float HalfMedian = CrossSection.MedianWidth * 0.5f;
	const float HalfTotalWidth = HalfCarriagewayWidth + HalfMedian;

	const int32 NumSteps = FMath::Max(2, FMath::CeilToInt(TotalSplineLength / SegmentStepLength));
	const float ActualStepLength = TotalSplineLength / static_cast<float>(NumSteps);

	// Generar secciones de vertices a lo largo del spline
	for (int32 i = 0; i <= NumSteps; ++i)
	{
		const float CurrentDistance = i * ActualStepLength;
		const FVector CenterLocation = GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::Local);
		const FVector SplineTangent = GetTangentAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::Local).GetSafeNormal();
		const FVector SplineUp = GetUpVectorAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::Local);
		const FVector SplineRight = FVector::CrossProduct(SplineTangent, SplineUp).GetSafeNormal();

		// Coordenada V de textura (cada 4 metros se repite la textura de asfalto y linea continua/discontinua)
		const float VCoord = CurrentDistance / 400.0f;

		// Vertice 0: Borde exterior izquierdo (arcén izquierdo)
		FVector LeftEdge = CenterLocation - (SplineRight * HalfTotalWidth);
		Vertices.Add(LeftEdge);
		Normals.Add(SplineUp);
		UV0.Add(FVector2D(0.0f, VCoord));
		Tangents.Add(FProcMeshTangent(SplineTangent, false));

		// Vertice 1: Centro de calzada (o mediana)
		Vertices.Add(CenterLocation);
		Normals.Add(SplineUp);
		UV0.Add(FVector2D(0.5f, VCoord));
		Tangents.Add(FProcMeshTangent(SplineTangent, false));

		// Vertice 2: Borde exterior derecho (arcén derecho)
		FVector RightEdge = CenterLocation + (SplineRight * HalfTotalWidth);
		Vertices.Add(RightEdge);
		Normals.Add(SplineUp);
		UV0.Add(FVector2D(1.0f, VCoord));
		Tangents.Add(FProcMeshTangent(SplineTangent, false));
	}

	// Construir triangulos (2 quads por segmento: izquierdo y derecho)
	// Cada fila tiene 3 vertices: [i*3 + 0] = Left, [i*3 + 1] = Center, [i*3 + 2] = Right
	for (int32 i = 0; i < NumSteps; ++i)
	{
		const int32 RowA = i * 3;
		const int32 RowB = (i + 1) * 3;

		// Quad Izquierdo (Left -> Center)
		// Triangulo 1
		Triangles.Add(RowA + 0);
		Triangles.Add(RowB + 0);
		Triangles.Add(RowA + 1);

		// Triangulo 2
		Triangles.Add(RowA + 1);
		Triangles.Add(RowB + 0);
		Triangles.Add(RowB + 1);

		// Quad Derecho (Center -> Right)
		// Triangulo 1
		Triangles.Add(RowA + 1);
		Triangles.Add(RowB + 1);
		Triangles.Add(RowA + 2);

		// Triangulo 2
		Triangles.Add(RowA + 2);
		Triangles.Add(RowB + 1);
		Triangles.Add(RowB + 2);
	}

	TargetMesh->ClearAllMeshSections();
	TargetMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	TargetMesh->SetMeshSectionCastsShadow(0, true);
}
