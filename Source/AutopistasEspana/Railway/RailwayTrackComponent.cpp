#include "Railway/RailwayTrackComponent.h"
#include "ProceduralMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

URailwayTrackComponent::URailwayTrackComponent()
{
	bDrawDebug = true;
	SetClosedLoop(false);

	// Por defecto inicializamos con perfil reglamentario de Red Convencional ADIF (Ancho Iberico 1.668 mm)
	GaugeType = ERailwayTrackGauge::Iberico1668;
	TrackConfig.GaugeWidthCm = 166.8f;
	TrackConfig.BallastTopWidthCm = 340.0f;
	TrackConfig.BallastBaseWidthCm = 460.0f;
	TrackConfig.BallastHeightCm = 38.0f;
	TrackConfig.RailHeadWidthCm = 7.5f;
	TrackConfig.RailHeightCm = 17.2f;
	TrackConfig.SleeperSpacingCm = 60.0f;
	TrackConfig.SleeperLengthCm = 260.0f;
	TrackConfig.SleeperWidthCm = 30.0f;
	TrackConfig.SleeperHeightCm = 22.0f;
	SpeedLimitKmh = 140.0f;
}

FVector URailwayTrackComponent::GetRailPositionAtDistance(float DistanceAlongSpline, bool bRightRail, ESplineCoordinateSpace::Type Space) const
{
	const FVector CenterPos = GetLocationAtDistanceAlongSpline(DistanceAlongSpline, Space);
	const FVector Tangent = GetTangentAtDistanceAlongSpline(DistanceAlongSpline, Space).GetSafeNormal();
	const FVector Up = GetUpVectorAtDistanceAlongSpline(DistanceAlongSpline, Space);
	const FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

	const float RailOffset = (TrackConfig.GaugeWidthCm * 0.5f) * (bRightRail ? 1.0f : -1.0f);
	return CenterPos + (Right * RailOffset) + (Up * TrackConfig.RailHeightCm);
}

FRotator URailwayTrackComponent::GetTrackRotationAtDistance(float DistanceAlongSpline, ESplineCoordinateSpace::Type Space) const
{
	const FVector Tangent = GetTangentAtDistanceAlongSpline(DistanceAlongSpline, Space).GetSafeNormal();
	const FVector Up = GetUpVectorAtDistanceAlongSpline(DistanceAlongSpline, Space);
	return UKismetMathLibrary::MakeRotFromXZ(Tangent, Up);
}

float URailwayTrackComponent::CalculateCurvatureCant(float DistanceAlongSpline) const
{
	// Calculo de peralte teorico segun curvatura y velocidad maxima
	const float SampleDelta = 200.0f;
	const FVector TangentA = GetTangentAtDistanceAlongSpline(FMath::Max(0.0f, DistanceAlongSpline - SampleDelta), ESplineCoordinateSpace::World).GetSafeNormal();
	const FVector TangentB = GetTangentAtDistanceAlongSpline(FMath::Min(GetSplineLength(), DistanceAlongSpline + SampleDelta), ESplineCoordinateSpace::World).GetSafeNormal();

	const float DotValue = FMath::Clamp(FVector::DotProduct(TangentA, TangentB), -1.0f, 1.0f);
	const float AngleRad = FMath::Acos(DotValue);
	
	// Si hay curvatura apreciable en el radio de via
	if (AngleRad > 0.001f)
	{
		const float ApproximateRadiusCm = (SampleDelta * 2.0f) / AngleRad;
		const float SpeedMS = (SpeedLimitKmh * 1000.0f) / 3600.0f;
		// Peralte teorico h = (v^2 * s) / (g * R) en mm
		const float CantMm = (FMath::Square(SpeedMS) * (TrackConfig.GaugeWidthCm * 10.0f)) / (9.81f * (ApproximateRadiusCm * 0.01f));
		return FMath::Clamp(CantMm, 0.0f, 160.0f); // Maximo 160 mm segun norma ADIF
	}

	return 0.0f;
}

void URailwayTrackComponent::GenerateTrackMesh(UProceduralMeshComponent* TargetMesh)
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

	TargetMesh->ClearAllMeshSections();

	// Seccion 0: Prisma de balasto de piedra machacada
	BuildBallastMeshSection(TargetMesh, 0);

	// Seccion 1: Carriles de acero continuo soldado (UIC 60)
	BuildSteelRailsMeshSection(TargetMesh, 1);

	// Seccion 2: Traviesas de hormigon monobloque
	BuildSleepersMeshSection(TargetMesh, 2);
}

void URailwayTrackComponent::BuildBallastMeshSection(UProceduralMeshComponent* TargetMesh, int32 SectionIndex)
{
	const float TotalSplineLength = GetSplineLength();
	const int32 NumSteps = FMath::Max(2, FMath::CeilToInt(TotalSplineLength / SegmentStepLength));
	const float ActualStepLength = TotalSplineLength / static_cast<float>(NumSteps);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;

	const float HalfTopWidth = TrackConfig.BallastTopWidthCm * 0.5f;
	const float HalfBaseWidth = TrackConfig.BallastBaseWidthCm * 0.5f;
	const float BallastHeight = TrackConfig.BallastHeightCm;

	// Generar secciones transversales del prisma trapezoidal
	for (int32 i = 0; i <= NumSteps; ++i)
	{
		const float CurrentDist = i * ActualStepLength;
		const FVector Center = GetLocationAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::Local);
		const FVector SplineTangent = GetTangentAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::Local).GetSafeNormal();
		const FVector SplineUp = GetUpVectorAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::Local);
		const FVector SplineRight = FVector::CrossProduct(SplineTangent, SplineUp).GetSafeNormal();

		const float VCoord = CurrentDist / 300.0f; // Repeticion de textura cada 3 metros

		// V0: Borde inferior izquierdo de la banqueta de balasto
		const FVector V0 = Center - (SplineRight * HalfBaseWidth) - (SplineUp * BallastHeight);
		// V1: Arista superior izquierda (coronacion)
		const FVector V1 = Center - (SplineRight * HalfTopWidth);
		// V2: Arista superior derecha (coronacion)
		const FVector V2 = Center + (SplineRight * HalfTopWidth);
		// V3: Borde inferior derecho de la banqueta de balasto
		const FVector V3 = Center + (SplineRight * HalfBaseWidth) - (SplineUp * BallastHeight);

		Vertices.Add(V0);
		Vertices.Add(V1);
		Vertices.Add(V2);
		Vertices.Add(V3);

		// Normales aproximadas por arista
		Normals.Add((-SplineRight + SplineUp).GetSafeNormal());
		Normals.Add(SplineUp);
		Normals.Add(SplineUp);
		Normals.Add((SplineRight + SplineUp).GetSafeNormal());

		UV0.Add(FVector2D(0.0f, VCoord));
		UV0.Add(FVector2D(0.35f, VCoord));
		UV0.Add(FVector2D(0.65f, VCoord));
		UV0.Add(FVector2D(1.0f, VCoord));

		const FProcMeshTangent Tan(SplineTangent, false);
		Tangents.Add(Tan);
		Tangents.Add(Tan);
		Tangents.Add(Tan);
		Tangents.Add(Tan);
	}

	// Triangulacion de los 3 planos longitudinales: Talud Izquierdo, Coronacion Central, Talud Derecho
	for (int32 i = 0; i < NumSteps; ++i)
	{
		const int32 RowA = i * 4;
		const int32 RowB = (i + 1) * 4;

		// 1. Talud izquierdo (V0 -> V1)
		Triangles.Add(RowA + 0);
		Triangles.Add(RowB + 0);
		Triangles.Add(RowA + 1);

		Triangles.Add(RowA + 1);
		Triangles.Add(RowB + 0);
		Triangles.Add(RowB + 1);

		// 2. Coronacion superior horizontal (V1 -> V2)
		Triangles.Add(RowA + 1);
		Triangles.Add(RowB + 1);
		Triangles.Add(RowA + 2);

		Triangles.Add(RowA + 2);
		Triangles.Add(RowB + 1);
		Triangles.Add(RowB + 2);

		// 3. Talud derecho (V2 -> V3)
		Triangles.Add(RowA + 2);
		Triangles.Add(RowB + 2);
		Triangles.Add(RowA + 3);

		Triangles.Add(RowA + 3);
		Triangles.Add(RowB + 2);
		Triangles.Add(RowB + 3);
	}

	TargetMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	TargetMesh->SetMeshSectionCastsShadow(SectionIndex, true);
}

void URailwayTrackComponent::BuildSteelRailsMeshSection(UProceduralMeshComponent* TargetMesh, int32 SectionIndex)
{
	const float TotalSplineLength = GetSplineLength();
	const int32 NumSteps = FMath::Max(2, FMath::CeilToInt(TotalSplineLength / SegmentStepLength));
	const float ActualStepLength = TotalSplineLength / static_cast<float>(NumSteps);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;

	const float HalfGauge = TrackConfig.GaugeWidthCm * 0.5f;
	const float HalfRailHead = TrackConfig.RailHeadWidthCm * 0.5f;
	const float RailHeight = TrackConfig.RailHeightCm;

	// Generamos el carril izquierdo y derecho mediante extrusion simultanea
	// Cada carril tiene 4 vertices por anillo de seccion transversal:
	// 0: Arriba Izquierda, 1: Arriba Derecha, 2: Abajo Derecha, 3: Abajo Izquierda
	for (int32 i = 0; i <= NumSteps; ++i)
	{
		const float CurrentDist = i * ActualStepLength;
		const FVector Center = GetLocationAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::Local);
		const FVector SplineTangent = GetTangentAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::Local).GetSafeNormal();
		const FVector SplineUp = GetUpVectorAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::Local);
		const FVector SplineRight = FVector::CrossProduct(SplineTangent, SplineUp).GetSafeNormal();

		const float VCoord = CurrentDist / 150.0f;
		const FProcMeshTangent Tan(SplineTangent, false);

		// --- CARRIL IZQUIERDO ---
		const FVector LeftRailCenter = Center - (SplineRight * HalfGauge);
		const FVector L_TopLeft = LeftRailCenter - (SplineRight * HalfRailHead) + (SplineUp * RailHeight);
		const FVector L_TopRight = LeftRailCenter + (SplineRight * HalfRailHead) + (SplineUp * RailHeight);
		const FVector L_BotRight = LeftRailCenter + (SplineRight * HalfRailHead);
		const FVector L_BotLeft = LeftRailCenter - (SplineRight * HalfRailHead);

		Vertices.Add(L_TopLeft);
		Vertices.Add(L_TopRight);
		Vertices.Add(L_BotRight);
		Vertices.Add(L_BotLeft);

		Normals.Add(SplineUp);
		Normals.Add(SplineUp);
		Normals.Add(SplineRight);
		Normals.Add(-SplineRight);

		UV0.Add(FVector2D(0.0f, VCoord));
		UV0.Add(FVector2D(0.5f, VCoord));
		UV0.Add(FVector2D(0.75f, VCoord));
		UV0.Add(FVector2D(1.0f, VCoord));

		Tangents.Add(Tan);
		Tangents.Add(Tan);
		Tangents.Add(Tan);
		Tangents.Add(Tan);

		// --- CARRIL DERECHO ---
		const FVector RightRailCenter = Center + (SplineRight * HalfGauge);
		const FVector R_TopLeft = RightRailCenter - (SplineRight * HalfRailHead) + (SplineUp * RailHeight);
		const FVector R_TopRight = RightRailCenter + (SplineRight * HalfRailHead) + (SplineUp * RailHeight);
		const FVector R_BotRight = RightRailCenter + (SplineRight * HalfRailHead);
		const FVector R_BotLeft = RightRailCenter - (SplineRight * HalfRailHead);

		Vertices.Add(R_TopLeft);
		Vertices.Add(R_TopRight);
		Vertices.Add(R_BotRight);
		Vertices.Add(R_BotLeft);

		Normals.Add(SplineUp);
		Normals.Add(SplineUp);
		Normals.Add(SplineRight);
		Normals.Add(-SplineRight);

		UV0.Add(FVector2D(0.0f, VCoord));
		UV0.Add(FVector2D(0.5f, VCoord));
		UV0.Add(FVector2D(0.75f, VCoord));
		UV0.Add(FVector2D(1.0f, VCoord));

		Tangents.Add(Tan);
		Tangents.Add(Tan);
		Tangents.Add(Tan);
		Tangents.Add(Tan);
	}

	// Triangulacion de los dos carriles (8 vertices por paso transversal)
	for (int32 i = 0; i < NumSteps; ++i)
	{
		const int32 BaseA = i * 8;
		const int32 BaseB = (i + 1) * 8;

		// --- Triangulos Carril Izquierdo (Indices 0..3) ---
		// Cara superior (0 -> 1)
		Triangles.Add(BaseA + 0);
		Triangles.Add(BaseB + 0);
		Triangles.Add(BaseA + 1);

		Triangles.Add(BaseA + 1);
		Triangles.Add(BaseB + 0);
		Triangles.Add(BaseB + 1);

		// Flanco derecho (1 -> 2)
		Triangles.Add(BaseA + 1);
		Triangles.Add(BaseB + 1);
		Triangles.Add(BaseA + 2);

		Triangles.Add(BaseA + 2);
		Triangles.Add(BaseB + 1);
		Triangles.Add(BaseB + 2);

		// Flanco izquierdo (3 -> 0)
		Triangles.Add(BaseA + 3);
		Triangles.Add(BaseA + 0);
		Triangles.Add(BaseB + 3);

		Triangles.Add(BaseB + 3);
		Triangles.Add(BaseA + 0);
		Triangles.Add(BaseB + 0);

		// --- Triangulos Carril Derecho (Indices 4..7) ---
		// Cara superior (4 -> 5)
		Triangles.Add(BaseA + 4);
		Triangles.Add(BaseB + 4);
		Triangles.Add(BaseA + 5);

		Triangles.Add(BaseA + 5);
		Triangles.Add(BaseB + 4);
		Triangles.Add(BaseB + 5);

		// Flanco derecho (5 -> 6)
		Triangles.Add(BaseA + 5);
		Triangles.Add(BaseB + 5);
		Triangles.Add(BaseA + 6);

		Triangles.Add(BaseA + 6);
		Triangles.Add(BaseB + 5);
		Triangles.Add(BaseB + 6);

		// Flanco izquierdo (7 -> 4)
		Triangles.Add(BaseA + 7);
		Triangles.Add(BaseA + 4);
		Triangles.Add(BaseB + 7);

		Triangles.Add(BaseB + 7);
		Triangles.Add(BaseA + 4);
		Triangles.Add(BaseB + 4);
	}

	TargetMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	TargetMesh->SetMeshSectionCastsShadow(SectionIndex, true);
}

void URailwayTrackComponent::BuildSleepersMeshSection(UProceduralMeshComponent* TargetMesh, int32 SectionIndex)
{
	const float TotalSplineLength = GetSplineLength();
	const float Spacing = FMath::Max(30.0f, TrackConfig.SleeperSpacingCm);
	const int32 NumSleepers = FMath::FloorToInt(TotalSplineLength / Spacing);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;

	const float HalfLength = TrackConfig.SleeperLengthCm * 0.5f;
	const float HalfWidth = TrackConfig.SleeperWidthCm * 0.5f;
	const float HalfHeight = TrackConfig.SleeperHeightCm * 0.5f;

	for (int32 s = 0; s < NumSleepers; ++s)
	{
		const float CurrentDist = (s * Spacing) + (Spacing * 0.5f);
		if (CurrentDist > TotalSplineLength)
		{
			break;
		}

		const FVector Center = GetLocationAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::Local);
		const FVector SplineTangent = GetTangentAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::Local).GetSafeNormal();
		const FVector SplineUp = GetUpVectorAtDistanceAlongSpline(CurrentDist, ESplineCoordinateSpace::Local);
		const FVector SplineRight = FVector::CrossProduct(SplineTangent, SplineUp).GetSafeNormal();

		const FProcMeshTangent Tan(SplineTangent, false);
		const int32 VBase = Vertices.Num();

		// 8 Vertices del prisma rectangular de la traviesa
		// Top face: 0, 1, 2, 3
		const FVector T_FL = Center - (SplineRight * HalfLength) + (SplineTangent * HalfWidth) + (SplineUp * HalfHeight);
		const FVector T_FR = Center + (SplineRight * HalfLength) + (SplineTangent * HalfWidth) + (SplineUp * HalfHeight);
		const FVector T_BR = Center + (SplineRight * HalfLength) - (SplineTangent * HalfWidth) + (SplineUp * HalfHeight);
		const FVector T_BL = Center - (SplineRight * HalfLength) - (SplineTangent * HalfWidth) + (SplineUp * HalfHeight);

		// Bottom face: 4, 5, 6, 7 (semienterrada en el balasto)
		const FVector B_FL = Center - (SplineRight * HalfLength) + (SplineTangent * HalfWidth) - (SplineUp * HalfHeight);
		const FVector B_FR = Center + (SplineRight * HalfLength) + (SplineTangent * HalfWidth) - (SplineUp * HalfHeight);
		const FVector B_BR = Center + (SplineRight * HalfLength) - (SplineTangent * HalfWidth) - (SplineUp * HalfHeight);
		const FVector B_BL = Center - (SplineRight * HalfLength) - (SplineTangent * HalfWidth) - (SplineUp * HalfHeight);

		Vertices.Add(T_FL); Vertices.Add(T_FR); Vertices.Add(T_BR); Vertices.Add(T_BL);
		Vertices.Add(B_FL); Vertices.Add(B_FR); Vertices.Add(B_BR); Vertices.Add(B_BL);

		Normals.Add(SplineUp); Normals.Add(SplineUp); Normals.Add(SplineUp); Normals.Add(SplineUp);
		Normals.Add(-SplineUp); Normals.Add(-SplineUp); Normals.Add(-SplineUp); Normals.Add(-SplineUp);

		UV0.Add(FVector2D(0.0f, 0.0f)); UV0.Add(FVector2D(1.0f, 0.0f));
		UV0.Add(FVector2D(1.0f, 1.0f)); UV0.Add(FVector2D(0.0f, 1.0f));
		UV0.Add(FVector2D(0.0f, 0.0f)); UV0.Add(FVector2D(1.0f, 0.0f));
		UV0.Add(FVector2D(1.0f, 1.0f)); UV0.Add(FVector2D(0.0f, 1.0f));

		for (int32 k = 0; k < 8; ++k)
		{
			Tangents.Add(Tan);
		}

		// Cara superior (Top: 0, 1, 2, 3)
		Triangles.Add(VBase + 0); Triangles.Add(VBase + 1); Triangles.Add(VBase + 2);
		Triangles.Add(VBase + 0); Triangles.Add(VBase + 2); Triangles.Add(VBase + 3);

		// Cara frontal (Forward: 0, 1, 5, 4)
		Triangles.Add(VBase + 0); Triangles.Add(VBase + 4); Triangles.Add(VBase + 1);
		Triangles.Add(VBase + 1); Triangles.Add(VBase + 4); Triangles.Add(VBase + 5);

		// Cara trasera (Backward: 2, 3, 7, 6)
		Triangles.Add(VBase + 2); Triangles.Add(VBase + 6); Triangles.Add(VBase + 3);
		Triangles.Add(VBase + 3); Triangles.Add(VBase + 6); Triangles.Add(VBase + 7);

		// Cara lateral derecha (Right: 1, 2, 6, 5)
		Triangles.Add(VBase + 1); Triangles.Add(VBase + 5); Triangles.Add(VBase + 2);
		Triangles.Add(VBase + 2); Triangles.Add(VBase + 5); Triangles.Add(VBase + 6);

		// Cara lateral izquierda (Left: 3, 0, 4, 7)
		Triangles.Add(VBase + 3); Triangles.Add(VBase + 7); Triangles.Add(VBase + 0);
		Triangles.Add(VBase + 0); Triangles.Add(VBase + 7); Triangles.Add(VBase + 4);
	}

	TargetMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	TargetMesh->SetMeshSectionCastsShadow(SectionIndex, true);
}
