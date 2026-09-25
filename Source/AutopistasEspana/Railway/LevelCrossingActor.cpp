#include "Railway/LevelCrossingActor.h"
#include "Railway/RailwayTrackComponent.h"
#include "Railway/TrainVehicleActor.h"
#include "Roads/RoadSplineComponent.h"
#include "Traffic/TrafficVehicleAgent.h"
#include "Traffic/TrafficSimulationSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "AutopistasEspana.h"

ALevelCrossingActor::ALevelCrossingActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Pavimento de caucho/hormigon STRAIL entre raíles
	CrossingPavementMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CrossingPavementMesh"));
	CrossingPavementMesh->SetupAttachment(RootComponent);
	CrossingPavementMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// --- SEMIBARRERA IZQUIERDA (Acceso calzada sentido 1) ---
	BarrierMastLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrierMastLeft"));
	BarrierMastLeft->SetupAttachment(RootComponent);
	BarrierMastLeft->SetRelativeLocation(FVector(-280.0f, -450.0f, 0.0f));

	BarrierPivotLeft = CreateDefaultSubobject<USceneComponent>(TEXT("BarrierPivotLeft"));
	BarrierPivotLeft->SetupAttachment(BarrierMastLeft);
	BarrierPivotLeft->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f)); // Altura del eje de giro

	BarrierArmLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrierArmLeft"));
	BarrierArmLeft->SetupAttachment(BarrierPivotLeft);
	BarrierArmLeft->SetRelativeLocation(FVector(0.0f, 200.0f, 0.0f)); // Brazo horizontal de 4 metros hacia el carril
	BarrierArmLeft->SetRelativeScale3D(FVector(0.12f, 4.0f, 0.2f));

	RedLightLeftA = CreateDefaultSubobject<UPointLightComponent>(TEXT("RedLightLeftA"));
	RedLightLeftA->SetupAttachment(BarrierMastLeft);
	RedLightLeftA->SetRelativeLocation(FVector(0.0f, -25.0f, 220.0f));
	RedLightLeftA->SetLightColor(FLinearColor::Red);
	RedLightLeftA->SetIntensity(0.0f);
	RedLightLeftA->SetAttenuationRadius(1800.0f);

	RedLightLeftB = CreateDefaultSubobject<UPointLightComponent>(TEXT("RedLightLeftB"));
	RedLightLeftB->SetupAttachment(BarrierMastLeft);
	RedLightLeftB->SetRelativeLocation(FVector(0.0f, 25.0f, 220.0f));
	RedLightLeftB->SetLightColor(FLinearColor::Red);
	RedLightLeftB->SetIntensity(0.0f);
	RedLightLeftB->SetAttenuationRadius(1800.0f);

	// --- SEMIBARRERA DERECHA (Acceso calzada sentido 2) ---
	BarrierMastRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrierMastRight"));
	BarrierMastRight->SetupAttachment(RootComponent);
	BarrierMastRight->SetRelativeLocation(FVector(280.0f, 450.0f, 0.0f));
	BarrierMastRight->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	BarrierPivotRight = CreateDefaultSubobject<USceneComponent>(TEXT("BarrierPivotRight"));
	BarrierPivotRight->SetupAttachment(BarrierMastRight);
	BarrierPivotRight->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));

	BarrierArmRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrierArmRight"));
	BarrierArmRight->SetupAttachment(BarrierPivotRight);
	BarrierArmRight->SetRelativeLocation(FVector(0.0f, 200.0f, 0.0f));
	BarrierArmRight->SetRelativeScale3D(FVector(0.12f, 4.0f, 0.2f));

	RedLightRightA = CreateDefaultSubobject<UPointLightComponent>(TEXT("RedLightRightA"));
	RedLightRightA->SetupAttachment(BarrierMastRight);
	RedLightRightA->SetRelativeLocation(FVector(0.0f, -25.0f, 220.0f));
	RedLightRightA->SetLightColor(FLinearColor::Red);
	RedLightRightA->SetIntensity(0.0f);
	RedLightRightA->SetAttenuationRadius(1800.0f);

	RedLightRightB = CreateDefaultSubobject<UPointLightComponent>(TEXT("RedLightRightB"));
	RedLightRightB->SetupAttachment(BarrierMastRight);
	RedLightRightB->SetRelativeLocation(FVector(0.0f, 25.0f, 220.0f));
	RedLightRightB->SetLightColor(FLinearColor::Red);
	RedLightRightB->SetIntensity(0.0f);
	RedLightRightB->SetAttenuationRadius(1800.0f);

	// --- CAJAS DE SEGURIDAD Y BLOQUEO ---
	RoadTrafficBlockerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RoadTrafficBlockerBox"));
	RoadTrafficBlockerBox->SetupAttachment(RootComponent);
	RoadTrafficBlockerBox->SetBoxExtent(FVector(150.0f, 500.0f, 180.0f));
	RoadTrafficBlockerBox->SetCollisionProfileName(TEXT("BlockAll"));
	RoadTrafficBlockerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Desactivado por defecto mientras este abierto

	TrackDangerZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TrackDangerZoneBox"));
	TrackDangerZoneBox->SetupAttachment(RootComponent);
	TrackDangerZoneBox->SetBoxExtent(FVector(250.0f, 450.0f, 150.0f));
	TrackDangerZoneBox->SetCollisionProfileName(TEXT("Trigger"));
	TrackDangerZoneBox->SetGenerateOverlapEvents(true);

	// Estado inicial
	CurrentState = ELevelCrossingState::Abierto;
	CurrentBarrierAngle = 90.0f; // Abiertas en vertical
	TargetBarrierAngle = 90.0f;
}

void ALevelCrossingActor::BeginPlay()
{
	Super::BeginPlay();

	BuildCrossingPavement();

	// Iniciar con barreras abiertas y luces apagadas
	BarrierPivotLeft->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	BarrierPivotRight->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	RoadTrafficBlockerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALevelCrossingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Maquina de estados del ciclo de enclavamiento ferroviario
	switch (CurrentState)
	{
	case ELevelCrossingState::Abierto:
		TargetBarrierAngle = 90.0f;
		RoadTrafficBlockerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case ELevelCrossingState::AvisoTrenAproximandose:
		RoadTrafficBlockerBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		PhaseTimer += DeltaTime;
		// Tras agotar los segundos reglamentarios de preaviso acustico/optico, descienden las barreras
		if (PhaseTimer >= Settings.PreWarningDurationSeconds)
		{
			SetCrossingState(ELevelCrossingState::BajandoBarreras);
		}
		break;

	case ELevelCrossingState::BajandoBarreras:
		TargetBarrierAngle = 0.0f; // Bajar a la horizontal
		if (CurrentBarrierAngle <= 1.0f)
		{
			CurrentBarrierAngle = 0.0f;
			SetCrossingState(ELevelCrossingState::Cerrado);
		}
		break;

	case ELevelCrossingState::Cerrado:
		TargetBarrierAngle = 0.0f;
		RoadTrafficBlockerBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;

	case ELevelCrossingState::SubiendoBarreras:
		TargetBarrierAngle = 90.0f; // Subir a la vertical
		if (CurrentBarrierAngle >= 89.0f)
		{
			CurrentBarrierAngle = 90.0f;
			SetCrossingState(ELevelCrossingState::Abierto);
		}
		break;

	case ELevelCrossingState::AveriaEmergencia:
		// En averia se mantienen bajadas por seguridad a prueba de fallos (fail-safe)
		TargetBarrierAngle = 0.0f;
		RoadTrafficBlockerBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}

	// 2. Animacion mecanica suave de las semibarreras abatibles
	UpdateBarrierRotation(DeltaTime);

	// 3. Focos rojos de senalizacion luminosa
	UpdateRedLightSignaling(DeltaTime);

	// 4. Detencion y gestion de vehiculos rodados ante el paso a nivel
	ManageRoadTrafficStopping(DeltaTime);
}

void ALevelCrossingActor::UpdateBarrierRotation(float DeltaTime)
{
	const float MovementRate = 90.0f / FMath::Max(1.0f, Settings.BarrierMovementDurationSeconds);
	CurrentBarrierAngle = FMath::FInterpConstantTo(CurrentBarrierAngle, TargetBarrierAngle, DeltaTime, MovementRate);

	// Aplicamos la rotacion en el eje Z/Roll del pivote
	BarrierPivotLeft->SetRelativeRotation(FRotator(0.0f, 0.0f, CurrentBarrierAngle));
	BarrierPivotRight->SetRelativeRotation(FRotator(0.0f, 0.0f, CurrentBarrierAngle));
}

void ALevelCrossingActor::UpdateRedLightSignaling(float DeltaTime)
{
	if (CurrentState == ELevelCrossingState::Abierto)
	{
		// Si el paso esta abierto, los focos rojos permanecen completamente apagados
		RedLightLeftA->SetIntensity(0.0f);
		RedLightLeftB->SetIntensity(0.0f);
		RedLightRightA->SetIntensity(0.0f);
		RedLightRightB->SetIntensity(0.0f);
		return;
	}

	// Si hay tren aproximandose o barreras cerradas, alternar parpadeo a la frecuencia especificada
	BlinkTimer += DeltaTime;
	const float HalfPeriod = 0.5f / FMath::Max(0.5f, Settings.RedLightBlinkFrequencyHz);
	if (BlinkTimer >= HalfPeriod)
	{
		BlinkTimer = 0.0f;
		bBlinkToggle = !bBlinkToggle;
	}

	const float BrightnessOn = 8000.0f; // Candela/lumens intensa visible de dia
	const float BrightnessOff = 0.0f;

	const float IntensityA = bBlinkToggle ? BrightnessOn : BrightnessOff;
	const float IntensityB = bBlinkToggle ? BrightnessOff : BrightnessOn;

	RedLightLeftA->SetIntensity(IntensityA);
	RedLightLeftB->SetIntensity(IntensityB);
	RedLightRightA->SetIntensity(IntensityA);
	RedLightRightB->SetIntensity(IntensityB);
}

void ALevelCrossingActor::ManageRoadTrafficStopping(float DeltaTime)
{
	// Si el paso a nivel esta libre, liberar los vehiculos retenidos
	if (CurrentState == ELevelCrossingState::Abierto)
	{
		if (StoppedRoadVehicles.Num() > 0)
		{
			StoppedRoadVehicles.Empty();
			UE_LOG(LogAutopistas, Log, TEXT("Paso a nivel reabierto: trafico rodado reanudando la marcha."));
		}
		return;
	}

	// Si el paso esta en aviso, bajando o cerrado, detener vehiculos que se aproximen
	if (!AssociatedRoad.IsValid())
	{
		return;
	}

	// Buscar en el subsistema de trafico vehiculos en la carretera asociada
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	RoadTrafficBlockerBox->GetOverlappingActors(OverlappingActors, ATrafficVehicleAgent::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		ATrafficVehicleAgent* Vehicle = Cast<ATrafficVehicleAgent>(Actor);
		if (Vehicle && Vehicle->IncidentState == EIncidentState::Normal)
		{
			// Frenada forzosa en la linea de parada
			Vehicle->CurrentSpeedCmS = 0.0f;
			Vehicle->CurrentSpeedKmh = 0.0f;
			if (!StoppedRoadVehicles.Contains(Vehicle))
			{
				StoppedRoadVehicles.Add(Vehicle);
			}
		}
	}

	// Deteccion proactiva por proximidad sobre el spline de carretera
	const FVector CrossingWorldPos = GetActorLocation();
	const float DetectionRadius = Settings.RoadStoppingDistanceCm + 1500.0f; // ~20 metros

	TArray<FOverlapResult> Overlaps;
	FCollisionShape QuerySphere = FCollisionShape::MakeSphere(DetectionRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (World->OverlapMultiByChannel(Overlaps, CrossingWorldPos, FQuat::Identity, ECC_WorldDynamic, QuerySphere, QueryParams))
	{
		for (const FOverlapResult& Hit : Overlaps)
		{
			ATrafficVehicleAgent* Agent = Cast<ATrafficVehicleAgent>(Hit.GetActor());
			if (Agent && Agent->IncidentState == EIncidentState::Normal)
			{
				const float DistToCrossing = FVector::Dist(Agent->GetActorLocation(), CrossingWorldPos);
				if (DistToCrossing <= Settings.RoadStoppingDistanceCm)
				{
					// Detenido completamente ante el paso
					Agent->CurrentSpeedCmS = 0.0f;
					Agent->CurrentSpeedKmh = 0.0f;
					if (!StoppedRoadVehicles.Contains(Agent))
					{
						StoppedRoadVehicles.Add(Agent);
					}
				}
				else
				{
					// Desaceleracion gradual aproximandose a la barrera
					Agent->CurrentSpeedCmS = FMath::FInterpTo(Agent->CurrentSpeedCmS, 0.0f, DeltaTime, 3.0f);
					Agent->CurrentSpeedKmh = Agent->CurrentSpeedCmS * (3600.0f / 100000.0f);
				}
			}
		}
	}
}

void ALevelCrossingActor::NotifyTrainApproaching(ATrainVehicleActor* Train, float DistanceToCrossingCm)
{
	ActiveTrain = Train;

	if (CurrentState == ELevelCrossingState::Abierto)
	{
		PhaseTimer = 0.0f;
		SetCrossingState(ELevelCrossingState::AvisoTrenAproximandose);
		UE_LOG(LogAutopistas, Warning, TEXT("Paso a Nivel: Tren aproximandose a %.1f metros. Activando senales rojas y campana."), DistanceToCrossingCm * 0.01f);
	}
}

void ALevelCrossingActor::NotifyTrainEnteredCrossing(ATrainVehicleActor* Train)
{
	ActiveTrain = Train;
	SetCrossingState(ELevelCrossingState::Cerrado);
	CurrentBarrierAngle = 0.0f;
	TargetBarrierAngle = 0.0f;
	UE_LOG(LogAutopistas, Log, TEXT("Paso a Nivel: Tren ocupando el cruce ferroviario."));
}

void ALevelCrossingActor::NotifyTrainClearedCrossing(ATrainVehicleActor* Train)
{
	if (ActiveTrain == Train || !ActiveTrain.IsValid())
	{
		ActiveTrain = nullptr;
		SetCrossingState(ELevelCrossingState::SubiendoBarreras);
		UE_LOG(LogAutopistas, Log, TEXT("Paso a Nivel: Tren ha librado el circuito de via. Alzando semibarreras."));
	}
}

void ALevelCrossingActor::SetCrossingState(ELevelCrossingState NewState)
{
	CurrentState = NewState;

	if (NewState == ELevelCrossingState::BajandoBarreras || NewState == ELevelCrossingState::Cerrado || NewState == ELevelCrossingState::AveriaEmergencia)
	{
		TargetBarrierAngle = 0.0f;
		RoadTrafficBlockerBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else if (NewState == ELevelCrossingState::SubiendoBarreras || NewState == ELevelCrossingState::Abierto)
	{
		TargetBarrierAngle = 90.0f;
	}
}

bool ALevelCrossingActor::HasTrappedVehicleOnTracks() const
{
	return GetTrappedVehicles().Num() > 0;
}

TArray<ATrafficVehicleAgent*> ALevelCrossingActor::GetTrappedVehicles() const
{
	TArray<ATrafficVehicleAgent*> TrappedList;
	TArray<AActor*> Overlaps;
	TrackDangerZoneBox->GetOverlappingActors(Overlaps, ATrafficVehicleAgent::StaticClass());

	for (AActor* Actor : Overlaps)
	{
		if (ATrafficVehicleAgent* Vehicle = Cast<ATrafficVehicleAgent>(Actor))
		{
			TrappedList.Add(Vehicle);
		}
	}

	return TrappedList;
}

void ALevelCrossingActor::BuildCrossingPavement()
{
	// Generar una losa de hormigon/caucho de 8x10 metros a nivel de la rasante para vehiculos rodados
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;

	const float HalfW = 400.0f; // 8 metros de calzada
	const float HalfL = 300.0f; // 6 metros a lo largo de las vias
	const float HeightZ = 18.0f; // A la altura exacta de la cabeza del carril

	// Losa superior
	Vertices.Add(FVector(-HalfL, -HalfW, HeightZ)); // 0
	Vertices.Add(FVector( HalfL, -HalfW, HeightZ)); // 1
	Vertices.Add(FVector( HalfL,  HalfW, HeightZ)); // 2
	Vertices.Add(FVector(-HalfL,  HalfW, HeightZ)); // 3

	Normals.Add(FVector::UpVector);
	Normals.Add(FVector::UpVector);
	Normals.Add(FVector::UpVector);
	Normals.Add(FVector::UpVector);

	UV0.Add(FVector2D(0.0f, 0.0f));
	UV0.Add(FVector2D(1.0f, 0.0f));
	UV0.Add(FVector2D(1.0f, 1.0f));
	UV0.Add(FVector2D(0.0f, 1.0f));

	Triangles.Add(0); Triangles.Add(1); Triangles.Add(2);
	Triangles.Add(0); Triangles.Add(2); Triangles.Add(3);

	CrossingPavementMesh->ClearAllMeshSections();
	CrossingPavementMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}
