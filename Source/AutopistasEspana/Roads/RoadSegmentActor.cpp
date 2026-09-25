#include "Roads/RoadSegmentActor.h"
#include "Roads/RoadSplineComponent.h"
#include "Roads/RoadNetworkSubsystem.h"
#include "Economy/EconomySubsystem.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "AutopistasEspana.h"

ARoadSegmentActor::ARoadSegmentActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RoadSpline = CreateDefaultSubobject<URoadSplineComponent>(TEXT("RoadSpline"));
	RootComponent = RoadSpline;

	RoadMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RoadMesh"));
	RoadMesh->SetupAttachment(RootComponent);
	RoadMesh->bUseAsyncCooking = true;

	// Componente de instancias para balizamiento con conos reflectantes SM_Cono_Obra_75
	ConesMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ConesMeshComponent"));
	ConesMeshComponent->SetupAttachment(RootComponent);
	ConesMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	ConesMeshComponent->bCastDynamicShadow = true;

	// Intentar cargar la malla del cono de obra
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMeshAsset(TEXT("/Game/Meshes/RoadProps/SM_Cono_Obra_75.SM_Cono_Obra_75"));
	if (ConeMeshAsset.Succeeded())
	{
		ConeStaticMesh = ConeMeshAsset.Object;
		ConesMeshComponent->SetStaticMesh(ConeStaticMesh);
	}

	// Maquinaria de obra provisional
	MachineryMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MachineryMeshComponent"));
	MachineryMeshComponent->SetupAttachment(RootComponent);
	MachineryMeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
	MachineryMeshComponent->SetVisibility(false);
}

void ARoadSegmentActor::BeginPlay()
{
	Super::BeginPlay();

	// Asignar malla de conos si no estaba asignada
	if (ConeStaticMesh && ConesMeshComponent && ConesMeshComponent->GetStaticMesh() == nullptr)
	{
		ConesMeshComponent->SetStaticMesh(ConeStaticMesh);
	}

	// Registrar automaticamente el tramo en la red global
	if (UWorld* World = GetWorld())
	{
		if (URoadNetworkSubsystem* NetworkSubsystem = World->GetSubsystem<URoadNetworkSubsystem>())
		{
			NetworkSubsystem->RegisterRoadSegment(RoadSpline);
		}

		// Comprobar ajustes de impacto de obra en el subsistema economico
		if (UEconomySubsystem* EconomySubsystem = World->GetSubsystem<UEconomySubsystem>())
		{
			if (EconomySubsystem->IsConstructionImpactEnabled() && CurrentConstructionPhase != ERoadConstructionPhase::AbiertaAlTrafico)
			{
				StartConstruction(false);
			}
			else
			{
				CompleteConstruction();
			}
		}
		else
		{
			CompleteConstruction();
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

	ClearConstructionCones();
	Super::EndPlay(EndPlayReason);
}

void ARoadSegmentActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Si el tramo esta en obras, avanzar temporizadores
	if (IsUnderConstruction())
	{
		float SpeedMultiplier = 1.0f;
		if (UWorld* World = GetWorld())
		{
			if (UEconomySubsystem* Economy = World->GetSubsystem<UEconomySubsystem>())
			{
				// Si el jugador desactiva las obras en tiempo real, abrir de inmediato
				if (!Economy->IsConstructionImpactEnabled())
				{
					CompleteConstruction();
					return;
				}
				SpeedMultiplier = Economy->GetConstructionSpeedMultiplier();
			}
		}

		CurrentPhaseTimer += DeltaTime * SpeedMultiplier;

		// Calcular progreso global ponderado
		const float PhaseNormProgress = FMath::Clamp(CurrentPhaseTimer / FMath::Max(0.1f, CurrentPhaseDuration), 0.0f, 1.0f);
		if (CurrentConstructionPhase == ERoadConstructionPhase::MovimientoTierras)
		{
			TotalConstructionProgress = PhaseNormProgress * 0.30f;
		}
		else if (CurrentConstructionPhase == ERoadConstructionPhase::ExtendidoAsfaltado)
		{
			TotalConstructionProgress = 0.30f + (PhaseNormProgress * 0.50f);
		}
		else if (CurrentConstructionPhase == ERoadConstructionPhase::PinturaYBalizamiento)
		{
			TotalConstructionProgress = 0.80f + (PhaseNormProgress * 0.20f);
		}

		// Transicion automatica a la siguiente fase
		if (CurrentPhaseTimer >= CurrentPhaseDuration)
		{
			AdvanceConstructionPhase();
		}
	}
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
		if (IsUnderConstruction())
		{
			SpawnConstructionCones();
		}
	}
}

void ARoadSegmentActor::SetRoadCategory(ERoadCategory NewCategory)
{
	if (RoadSpline)
	{
		RoadSpline->RoadCategory = NewCategory;

		switch (NewCategory)
		{
		case ERoadCategory::CaminoRural:
			RoadSpline->CrossSection.NumLanesDirection = 1;
			RoadSpline->CrossSection.LaneWidth = 300.0f;
			RoadSpline->CrossSection.OuterShoulderWidth = 50.0f;
			RoadSpline->CrossSection.InnerShoulderWidth = 0.0f;
			RoadSpline->CrossSection.MedianWidth = 0.0f;
			RoadSpline->CrossSection.SpeedLimitKmh = 40.0f;
			RoadSpline->CrossSection.bHasGuardrail = false;
			break;

		case ERoadCategory::Convencional_90:
			RoadSpline->CrossSection.NumLanesDirection = 1;
			RoadSpline->CrossSection.LaneWidth = 350.0f;
			RoadSpline->CrossSection.OuterShoulderWidth = 150.0f;
			RoadSpline->CrossSection.InnerShoulderWidth = 0.0f;
			RoadSpline->CrossSection.MedianWidth = 0.0f;
			RoadSpline->CrossSection.SpeedLimitKmh = 90.0f;
			RoadSpline->CrossSection.bHasGuardrail = true;
			break;

		case ERoadCategory::Autovia_120:
			RoadSpline->CrossSection.NumLanesDirection = 2;
			RoadSpline->CrossSection.LaneWidth = 350.0f;
			RoadSpline->CrossSection.OuterShoulderWidth = 250.0f;
			RoadSpline->CrossSection.InnerShoulderWidth = 100.0f;
			RoadSpline->CrossSection.MedianWidth = 200.0f;
			RoadSpline->CrossSection.SpeedLimitKmh = 120.0f;
			RoadSpline->CrossSection.bHasGuardrail = true;
			break;

		case ERoadCategory::Autopista_3x3:
			RoadSpline->CrossSection.NumLanesDirection = 3;
			RoadSpline->CrossSection.LaneWidth = 350.0f;
			RoadSpline->CrossSection.OuterShoulderWidth = 250.0f;
			RoadSpline->CrossSection.InnerShoulderWidth = 150.0f;
			RoadSpline->CrossSection.MedianWidth = 300.0f;
			RoadSpline->CrossSection.SpeedLimitKmh = 120.0f;
			RoadSpline->CrossSection.bHasGuardrail = true;
			break;

		case ERoadCategory::RamalEnlace:
			RoadSpline->CrossSection.NumLanesDirection = 1;
			RoadSpline->CrossSection.LaneWidth = 400.0f;
			RoadSpline->CrossSection.OuterShoulderWidth = 150.0f;
			RoadSpline->CrossSection.InnerShoulderWidth = 50.0f;
			RoadSpline->CrossSection.MedianWidth = 0.0f;
			RoadSpline->CrossSection.SpeedLimitKmh = 60.0f;
			RoadSpline->CrossSection.bHasGuardrail = true;
			break;

		default:
			break;
		}

		RebuildRoadGeometry();
		if (IsUnderConstruction())
		{
			SpawnConstructionCones();
		}
	}
}

void ARoadSegmentActor::StartConstruction(bool bInstant)
{
	if (bInstant)
	{
		CompleteConstruction();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UEconomySubsystem* Economy = World->GetSubsystem<UEconomySubsystem>())
		{
			if (!Economy->IsConstructionImpactEnabled())
			{
				CompleteConstruction();
				return;
			}

			const float TotalDuration = Economy->CalculateConstructionDurationSeconds(
				RoadSpline ? RoadSpline->RoadCategory : ERoadCategory::Autovia_120,
				RoadSpline ? RoadSpline->GetSplineLength() : 10000.0f);

			ConstructionParams.Phase1DurationSeconds = TotalDuration * 0.30f;
			ConstructionParams.Phase2DurationSeconds = TotalDuration * 0.50f;
			ConstructionParams.Phase3DurationSeconds = TotalDuration * 0.20f;
		}
	}

	SetConstructionPhase(ERoadConstructionPhase::MovimientoTierras);
}

void ARoadSegmentActor::SetConstructionPhase(ERoadConstructionPhase NewPhase)
{
	CurrentConstructionPhase = NewPhase;
	CurrentPhaseTimer = 0.0f;

	switch (NewPhase)
	{
	case ERoadConstructionPhase::MovimientoTierras:
		CurrentPhaseDuration = ConstructionParams.Phase1DurationSeconds;
		TotalConstructionProgress = 0.0f;
		break;

	case ERoadConstructionPhase::ExtendidoAsfaltado:
		CurrentPhaseDuration = ConstructionParams.Phase2DurationSeconds;
		TotalConstructionProgress = 0.30f;
		break;

	case ERoadConstructionPhase::PinturaYBalizamiento:
		CurrentPhaseDuration = ConstructionParams.Phase3DurationSeconds;
		TotalConstructionProgress = 0.80f;
		break;

	case ERoadConstructionPhase::AbiertaAlTrafico:
		CompleteConstruction();
		return;

	default:
		break;
	}

	UpdateConstructionVisuals();
	SpawnConstructionCones();

	OnConstructionPhaseChanged.Broadcast(this, NewPhase);

	UE_LOG(LogAutopistas, Log, TEXT("Tramo [%s] avanza a Fase de Obra: %d (Velocidad max: %.0f km/h, Carriles cortados: %d)"),
		*GetName(), static_cast<int32>(NewPhase), GetEffectiveSpeedLimitKmh(), GetClosedLanesCount());
}

void ARoadSegmentActor::AdvanceConstructionPhase()
{
	if (CurrentConstructionPhase == ERoadConstructionPhase::MovimientoTierras)
	{
		SetConstructionPhase(ERoadConstructionPhase::ExtendidoAsfaltado);
	}
	else if (CurrentConstructionPhase == ERoadConstructionPhase::ExtendidoAsfaltado)
	{
		SetConstructionPhase(ERoadConstructionPhase::PinturaYBalizamiento);
	}
	else if (CurrentConstructionPhase == ERoadConstructionPhase::PinturaYBalizamiento)
	{
		CompleteConstruction();
	}
}

void ARoadSegmentActor::CompleteConstruction()
{
	CurrentConstructionPhase = ERoadConstructionPhase::AbiertaAlTrafico;
	TotalConstructionProgress = 1.0f;
	CurrentPhaseTimer = 0.0f;

	ClearConstructionCones();

	if (MachineryMeshComponent)
	{
		MachineryMeshComponent->SetVisibility(false);
	}

	RebuildRoadGeometry();
	OnConstructionCompleted.Broadcast(this);

	UE_LOG(LogAutopistas, Log, TEXT("Tramo [%s]: OBRAS COMPLETADAS. Vía abierta al 100%% al tráfico con velocidad máxima de %.0f km/h."),
		*GetName(), GetEffectiveSpeedLimitKmh());
}

float ARoadSegmentActor::GetEffectiveSpeedLimitKmh() const
{
	switch (CurrentConstructionPhase)
	{
	case ERoadConstructionPhase::MovimientoTierras:
		return ConstructionParams.Phase1SpeedLimitKmh; // 40 km/h en desmonte

	case ERoadConstructionPhase::ExtendidoAsfaltado:
		return ConstructionParams.Phase2SpeedLimitKmh; // 40 km/h en asfaltado

	case ERoadConstructionPhase::PinturaYBalizamiento:
		return ConstructionParams.Phase3SpeedLimitKmh; // 60 km/h en pintura

	case ERoadConstructionPhase::AbiertaAlTrafico:
	default:
		return RoadSpline ? RoadSpline->CrossSection.SpeedLimitKmh : 120.0f;
	}
}

int32 ARoadSegmentActor::GetClosedLanesCount() const
{
	if (!IsUnderConstruction() || !RoadSpline)
	{
		return 0;
	}

	const int32 TotalLanes = RoadSpline->CrossSection.NumLanesDirection;
	if (TotalLanes <= 1)
	{
		// En carreteras convencionales de un carril por sentido no se corta totalmente el carril
		// sino que se reduce drasticamente la velocidad a 40 km/h con paso alternado
		return 0;
	}

	// En autovias 2x2 y autopistas 3x3 se corta como minimo 1 carril (el carril izquierdo o central)
	return FMath::Clamp(ConstructionParams.ClosedLanesCount, 1, TotalLanes - 1);
}

int32 ARoadSegmentActor::GetEffectiveOpenLanesCount() const
{
	if (!RoadSpline)
	{
		return 1;
	}

	const int32 TotalLanes = RoadSpline->CrossSection.NumLanesDirection;
	return FMath::Max(1, TotalLanes - GetClosedLanesCount());
}

bool ARoadSegmentActor::IsLaneClosed(int32 LaneIndex) const
{
	if (!IsUnderConstruction() || !RoadSpline)
	{
		return false;
	}

	const int32 TotalLanes = RoadSpline->CrossSection.NumLanesDirection;
	if (TotalLanes <= 1)
	{
		return false;
	}

	const int32 ClosedCount = GetClosedLanesCount();
	// Los carriles se cierran desde la izquierda (adelantamiento) hacia la derecha
	return LaneIndex >= (TotalLanes - ClosedCount);
}

void ARoadSegmentActor::SpawnConstructionCones()
{
	ClearConstructionCones();

	if (!IsUnderConstruction() || !RoadSpline || !ConesMeshComponent)
	{
		return;
	}

	if (ConeStaticMesh && ConesMeshComponent->GetStaticMesh() == nullptr)
	{
		ConesMeshComponent->SetStaticMesh(ConeStaticMesh);
	}

	const float SplineLength = RoadSpline->GetSplineLength();
	if (SplineLength < 200.0f)
	{
		return;
	}

	const float Spacing = FMath::Max(300.0f, ConstructionParams.ConeSpacingCm); // Cada 6-7m
	const int32 NumLanes = RoadSpline->CrossSection.NumLanesDirection;
	const float LaneW = RoadSpline->CrossSection.LaneWidth;

	// Desplazamiento lateral de los conos: a lo largo de la linea divisoria del carril cerrado
	// En autovia 2x2: Carril 0 esta a +LaneW/2 (+175cm), carril 1 a -LaneW/2 (-175cm).
	// La linea divisoria de conos se ubica exactamente en offset 0.0f (o en el borde del carril cerrado).
	float DividerOffsetCm = 0.0f;
	if (NumLanes > 1)
	{
		DividerOffsetCm = 0.0f; // Eje divisorio entre carril abierto y cerrado
	}
	else
	{
		// En convencional de 1 carril, se colocan los conos en el arcen exterior (+175cm)
		DividerOffsetCm = LaneW * 0.5f + 30.0f;
	}

	// Cuña de balizamiento de entrada (taper de 30 metros al inicio)
	const float TaperLength = FMath::Min(3000.0f, SplineLength * 0.25f);

	for (float Dist = 0.0f; Dist <= SplineLength; Dist += Spacing)
	{
		const FVector SplinePos = RoadSpline->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
		const FVector SplineTangent = RoadSpline->GetTangentAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World).GetSafeNormal();
		const FVector SplineUp = RoadSpline->GetUpVectorAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
		const FVector SplineRight = FVector::CrossProduct(SplineTangent, SplineUp).GetSafeNormal();

		float LateralOffset = DividerOffsetCm;

		// Taper / cuña diagonal en los primeros metros
		if (Dist < TaperLength)
		{
			const float TaperAlpha = Dist / TaperLength;
			const float StartOffset = (NumLanes > 1) ? -(LaneW * 0.5f) : (LaneW + 100.0f);
			LateralOffset = FMath::Lerp(StartOffset, DividerOffsetCm, TaperAlpha);
		}

		const FVector ConeLocation = SplinePos + (SplineRight * LateralOffset);
		const FRotator ConeRotation = UKismetMathLibrary::MakeRotFromXZ(SplineTangent, SplineUp);

		FTransform ConeTransform;
		ConeTransform.SetLocation(ConeLocation);
		ConeTransform.SetRotation(ConeRotation.Quaternion());
		ConeTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

		ConesMeshComponent->AddInstance(ConeTransform);
	}
}

void ARoadSegmentActor::ClearConstructionCones()
{
	if (ConesMeshComponent)
	{
		ConesMeshComponent->ClearInstances();
	}
}

void ARoadSegmentActor::UpdateConstructionVisuals()
{
	if (!MachineryMeshComponent || !RoadSpline)
	{
		return;
	}

	if (CurrentConstructionPhase == ERoadConstructionPhase::MovimientoTierras ||
		CurrentConstructionPhase == ERoadConstructionPhase::ExtendidoAsfaltado)
	{
		// Posicionar la maquinaria en el carril cerrado a mitad de tramo
		const float MidDist = RoadSpline->GetSplineLength() * 0.5f;
		const FVector MidPos = RoadSpline->GetLocationAtDistanceAlongSpline(MidDist, ESplineCoordinateSpace::World);
		const FVector Tangent = RoadSpline->GetTangentAtDistanceAlongSpline(MidDist, ESplineCoordinateSpace::World).GetSafeNormal();
		const FVector Up = RoadSpline->GetUpVectorAtDistanceAlongSpline(MidDist, ESplineCoordinateSpace::World);
		const FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

		const float ClosedLaneOffset = -RoadSpline->CrossSection.LaneWidth * 0.5f;
		const FVector MachineryLoc = MidPos + (Right * ClosedLaneOffset);
		const FRotator MachineryRot = UKismetMathLibrary::MakeRotFromXZ(Tangent, Up);

		MachineryMeshComponent->SetWorldLocationAndRotation(MachineryLoc, MachineryRot);
		MachineryMeshComponent->SetVisibility(true);
	}
	else
	{
		// En Fase 3 (Pintura) la maquinaria pesada ya ha sido retirada a base
		MachineryMeshComponent->SetVisibility(false);
	}
}
