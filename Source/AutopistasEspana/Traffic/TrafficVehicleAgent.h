#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Traffic/TrafficTypes.h"
#include "TrafficVehicleAgent.generated.h"

class UStaticMeshComponent;
class URoadSplineComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriverConditionChanged, ATrafficVehicleAgent*, Vehicle, ECondicionPsicologica, NewCondition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriverHeadlightFlash, ATrafficVehicleAgent*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriverHonkHorn, ATrafficVehicleAgent*, Vehicle);

/**
 * Agente de Vehiculo de Trafico.
 * Controla la navegacion por spline, el modelo car-following IDM adaptativo,
 * el modelo de cambio de carril MOBIL (con agresividad variable segun humor),
 * la acumulacion de frustracion en atascos/obras viales y la furia al volante (road rage).
 */
UCLASS()
class AUTOPISTASESPANA_API ATrafficVehicleAgent : public AActor
{
	GENERATED_BODY()
	
public:	
	ATrafficVehicleAgent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// --- Visuales y Propiedades Fisicas ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	UStaticMeshComponent* VehicleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Properties")
	EVehicleCategory VehicleCategory = EVehicleCategory::Turismo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Properties")
	EIncidentState IncidentState = EIncidentState::Normal;

	// --- Modelo Psicologico y Humor del Conductor ---

	/** Estado animico actual segun el nivel de frustracion */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology")
	ECondicionPsicologica CondicionPsicologica = ECondicionPsicologica::Calmado;

	/** Estado animico compatible con el subsistema DGT / Pegasus */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology")
	EDriverMood DriverMood = EDriverMood::Tranquilo;

	/** Nivel de frustracion acumulado (0.0% a 100.0%) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology")
	float FrustrationPercent = 0.0f;

	/** Tiempo de espera acumulado en atasco / congestion (segundos) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology")
	float WaitTimeInJam = 0.0f;

	/** Umbral de paciencia individual de este conductor antes de irritarse rapidamente */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float IndividualPatienceTolerance = 15.0f;

	/** Parametros configurables de respuesta psicologica */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	FDriverPsychologyParams PsychologyParams;

	// --- Efectos de Furia al Volante (Road Rage) ---

	/** Multiplicador de riesgo de colision por alcance (x1 en normal, x5 en furia) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology|Road Rage")
	float RearEndCollisionRiskMultiplier = 1.0f;

	/** Indica si el vehiculo esta haciendo rafagas de luces al coche delantero */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology|Road Rage")
	bool bFlashingHeadlights = false;

	/** Estado actual de luz larga en la rafaga intermitente */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology|Road Rage")
	bool bHeadlightsHighBeam = false;

	/** Indica si el vehiculo esta tocando el claxon/bocina por desesperacion */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology|Road Rage")
	bool bHonkingHorn = false;

	// --- Delegados para Audio / Efectos FX ---

	UPROPERTY(BlueprintAssignable, Category = "Driver Psychology|Events")
	FOnDriverConditionChanged OnDriverConditionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Driver Psychology|Events")
	FOnDriverHeadlightFlash OnDriverHeadlightFlash;

	UPROPERTY(BlueprintAssignable, Category = "Driver Psychology|Events")
	FOnDriverHonkHorn OnDriverHonkHorn;

	// --- Parametros IDM y Dimensiones ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Properties")
	FIDMParameters IDMParams;

	/** Parametros IDM base para restaurar al relajarse */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle Properties")
	FIDMParameters BaseIDMParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Dimensions")
	float VehicleLengthCm = 420.0f; // 4.2m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Dimensions")
	float VehicleWidthCm = 180.0f;  // 1.8m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Dimensions")
	float VehicleMassKg = 1400.0f;

	// --- Estado Cinematico ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float CurrentSpeedCmS = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float CurrentSpeedKmh = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float DistanceAlongSpline = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float CurrentLaneOffsetCm = 175.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	int32 CurrentLaneIndex = 0; // 0 = Carril derecho, 1 = Carril izquierdo

	// Referencia a la carretera actual
	UPROPERTY(BlueprintReadWrite, Category = "Navigation")
	TWeakObjectPtr<URoadSplineComponent> CurrentRoadSpline;

	// --- Metodos de Trafico y Psicologia ---

	/** Deteccion del vehiculo precedente u obstaculo de obras en el mismo carril */
	UFUNCTION(BlueprintCallable, Category = "Traffic AI|IDM")
	bool FindLeadVehicle(float ScanDistanceCm, float& OutDistanceToLeadCm, float& OutRelativeSpeedCmS, ATrafficVehicleAgent*& OutLeadVehicle);

	/** Calculo de aceleracion IDM segun distancia y velocidad del vehiculo predecesor */
	UFUNCTION(BlueprintCallable, Category = "Traffic AI|IDM")
	float CalculateIDMAcceleration(float DistanceToLeadCm, float RelativeSpeedCmS) const;

	/** Evaluacion de cambio de carril MOBIL con cortes de obra y agresividad psicologica */
	UFUNCTION(BlueprintCallable, Category = "Traffic AI|MOBIL")
	bool EvaluateMOBILLaneChange(bool bWantsToOvertake);

	/** Disparar una rafaga de luces al vehiculo de delante */
	UFUNCTION(BlueprintCallable, Category = "Driver Psychology")
	void TriggerHeadlightFlash();

	/** Tocar el claxon/bocina en atasco */
	UFUNCTION(BlueprintCallable, Category = "Driver Psychology")
	void HonkHorn();

	/** Resetear estado de frustracion a modo Zen/Calmado */
	UFUNCTION(BlueprintCallable, Category = "Driver Psychology")
	void ResetPsychology();

	/** Disparador de siniestro vial / choque por alcance */
	UFUNCTION(BlueprintCallable, Category = "Incident")
	void TriggerAccidentCollision(FVector ImpactDirection);

	/** Disparador de averia repentina en arcen */
	UFUNCTION(BlueprintCallable, Category = "Incident")
	void TriggerMechanicalBreakdown();

private:
	// Progreso de cambio de carril (interpolacion lateral suave)
	bool bIsChangingLanes = false;
	float TargetLaneOffsetCm = 0.0f;
	float LaneChangeProgress = 0.0f;
	float LaneChangeDuration = 2.0f;

	// Temporizadores internos de efectos
	float HeadlightFlashTimer = 0.0f;
	float HornCooldownTimer = 0.0f;
	float OvertakeCheckTimer = 0.0f;

	/** Actualiza el medidor de frustracion y las consecuencias de conducta al volante */
	void UpdateDriverPsychology(float DeltaTime, float DistanceToLeadCm);
};
