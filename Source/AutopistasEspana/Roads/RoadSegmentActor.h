#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Roads/RoadTypes.h"
#include "RoadSegmentActor.generated.h"

class URoadSplineComponent;
class UProceduralMeshComponent;
class UInstancedStaticMeshComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConstructionPhaseChanged, ARoadSegmentActor*, RoadSegment, ERoadConstructionPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConstructionCompleted, ARoadSegmentActor*, RoadSegment);

/**
 * Actor que representa un Tramo de Carretera en el Mundo.
 * Soporta modelado geometrico procedimental por spline, fases progresivas de construccion
 * con balizamiento reflectante (SM_Cono_Obra_75), reduccion de velocidad reglamentaria segun fase,
 * corte de carriles y conexion directa con el subsistema de red vial y economia.
 */
UCLASS()
class AUTOPISTASESPANA_API ARoadSegmentActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ARoadSegmentActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

	// --- Componentes ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Components")
	URoadSplineComponent* RoadSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Components")
	UProceduralMeshComponent* RoadMesh;

	/** Instancias de conos reflectantes de obra (SM_Cono_Obra_75) a lo largo del tramo */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Construction|Props")
	UInstancedStaticMeshComponent* ConesMeshComponent;

	/** Malla de maquinaria pesada provisional durante la obra (retroexcavadora / extendedora) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Construction|Props")
	UStaticMeshComponent* MachineryMeshComponent;

	// Malla estática opcional para conos (editable en editor / BP)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Construction|Props")
	UStaticMesh* ConeStaticMesh;

	// --- Estado de Obras e Impacto Constructivo ---

	/** Fase actual de construcción o estado operativo */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Construction")
	ERoadConstructionPhase CurrentConstructionPhase = ERoadConstructionPhase::AbiertaAlTrafico;

	/** Parametros configurables de tiempos, balizamiento y velocidad de obras */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Construction")
	FRoadConstructionParams ConstructionParams;

	/** Progreso normalizado de la obra completa (0.0 = inicio, 1.0 = finalizada) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Construction")
	float TotalConstructionProgress = 1.0f;

	/** Temporizador de la fase actual (segundos transcurridos) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Construction")
	float CurrentPhaseTimer = 0.0f;

	/** Duración asignada a la fase actual (segundos) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Construction")
	float CurrentPhaseDuration = 10.0f;

	// --- Eventos y Delegados ---

	UPROPERTY(BlueprintAssignable, Category = "Road Construction|Events")
	FOnConstructionPhaseChanged OnConstructionPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Road Construction|Events")
	FOnConstructionCompleted OnConstructionCompleted;

	// --- Métodos de Construcción y Geometría ---

	/** Regenerar la geometria del tramo y actualizar balizamiento */
	UFUNCTION(BlueprintCallable, Category = "Road Building")
	void RebuildRoadGeometry();

	/** Anadir un nuevo punto de trazado en coordenadas de mundo */
	UFUNCTION(BlueprintCallable, Category = "Road Building")
	void AddSplinePointAtWorldLocation(const FVector& WorldLocation);

	/** Configurar la tipologia de via (Convencional, Autovia 2x2, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Road Building")
	void SetRoadCategory(ERoadCategory NewCategory);

	/** Iniciar el proceso de obra (con fases progresivas o instantaneo si bInstant es true) */
	UFUNCTION(BlueprintCallable, Category = "Road Construction")
	void StartConstruction(bool bInstant = false);

	/** Avanzar manualmente o por temporizador a la siguiente fase de obra */
	UFUNCTION(BlueprintCallable, Category = "Road Construction")
	void AdvanceConstructionPhase();

	/** Establecer directamente una fase de obra determinada */
	UFUNCTION(BlueprintCallable, Category = "Road Construction")
	void SetConstructionPhase(ERoadConstructionPhase NewPhase);

	/** Finalizar completamente las obras y abrir la carretera al tráfico al 100% */
	UFUNCTION(BlueprintCallable, Category = "Road Construction")
	void CompleteConstruction();

	// --- Consultas de Restricciones Viales ---

	/** Indica si el tramo se encuentra actualmente en fase de obras */
	UFUNCTION(BlueprintPure, Category = "Road Construction")
	bool IsUnderConstruction() const { return CurrentConstructionPhase != ERoadConstructionPhase::AbiertaAlTrafico; }

	/** Velocidad máxima efectiva permitida en este tramo (reducida durante obras) */
	UFUNCTION(BlueprintPure, Category = "Road Construction|Restrictions")
	float GetEffectiveSpeedLimitKmh() const;

	/** Número de carriles cortados actualmente por la presencia de obras */
	UFUNCTION(BlueprintPure, Category = "Road Construction|Restrictions")
	int32 GetClosedLanesCount() const;

	/** Número de carriles abiertos y transitables */
	UFUNCTION(BlueprintPure, Category = "Road Construction|Restrictions")
	int32 GetEffectiveOpenLanesCount() const;

	/** Comprueba si un carril específico (0 = derecho, 1 = izquierdo...) está cerrado al tráfico */
	UFUNCTION(BlueprintPure, Category = "Road Construction|Restrictions")
	bool IsLaneClosed(int32 LaneIndex) const;

	// --- Visuales y Balizamiento ---

	/** Genera los conos de balizamiento reflectantes a lo largo del carril cortado */
	UFUNCTION(BlueprintCallable, Category = "Road Construction|Props")
	void SpawnConstructionCones();

	/** Elimina todas las instancias de conos del tramo */
	UFUNCTION(BlueprintCallable, Category = "Road Construction|Props")
	void ClearConstructionCones();

	/** Actualiza las mallas y visibilidad de maquinaria según la fase activa */
	UFUNCTION(BlueprintCallable, Category = "Road Construction|Props")
	void UpdateConstructionVisuals();
};
