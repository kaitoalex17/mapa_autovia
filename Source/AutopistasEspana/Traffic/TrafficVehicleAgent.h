#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Traffic/TrafficTypes.h"
#include "TrafficVehicleAgent.generated.h"

class UStaticMeshComponent;
class URoadSplineComponent;

/**
 * Agente de Vehiculo de Trafico.
 * Controla la navegacion por spline, la aceleracion/frenada mediante IDM, el cambio de carril MOBIL
 * y la deteccion fisica de colisiones y siniestros en carretera.
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

	// Componente de Malla 3D del vehiculo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	UStaticMeshComponent* VehicleMesh;

	// Tipo y Parametros
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Properties")
	EVehicleCategory VehicleCategory = EVehicleCategory::Turismo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Properties")
	EIncidentState IncidentState = EIncidentState::Normal;

	// Psicologia y Humor del Conductor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology")
	EDriverMood DriverMood = EDriverMood::Tranquilo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Psychology")
	float FrustrationPercent = 0.0f; // 0.0% a 100.0%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Properties")
	FIDMParameters IDMParams;

	// Dimensiones fisicas en UU / centimetros
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Dimensions")
	float VehicleLengthCm = 420.0f; // 4.2m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Dimensions")
	float VehicleWidthCm = 180.0f;  // 1.8m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Dimensions")
	float VehicleMassKg = 1400.0f;

	// Estado Cinemático
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float CurrentSpeedCmS = 0.0f; // cm/s

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float CurrentSpeedKmh = 0.0f; // km/h (para HUD y logica)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float DistanceAlongSpline = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float CurrentLaneOffsetCm = 175.0f; // Desplazamiento lateral respecto al eje del spline

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	int32 CurrentLaneIndex = 0; // 0 = Carril derecho, 1 = Carril izquierdo

	// Referencia a la carretera actual
	UPROPERTY(BlueprintReadWrite, Category = "Navigation")
	TWeakObjectPtr<URoadSplineComponent> CurrentRoadSpline;

	// Calculo de aceleracion IDM segun distancia y velocidad del vehiculo predecesor
	UFUNCTION(BlueprintCallable, Category = "Traffic AI|IDM")
	float CalculateIDMAcceleration(float DistanceToLeadCm, float RelativeSpeedCmS) const;

	// Evaluacion de cambio de carril MOBIL (Normativa Espanola: circular por la derecha, adelantar por la izquierda)
	UFUNCTION(BlueprintCallable, Category = "Traffic AI|MOBIL")
	bool EvaluateMOBILLaneChange(bool bWantsToOvertake);

	// Disparador de siniestro vial / choque
	UFUNCTION(BlueprintCallable, Category = "Incident")
	void TriggerAccidentCollision(FVector ImpactDirection);

	// Disparador de averia repentina en arcen
	UFUNCTION(BlueprintCallable, Category = "Incident")
	void TriggerMechanicalBreakdown();

private:
	// Progreso de cambio de carril (interpolacion lateral suave)
	bool bIsChangingLanes = false;
	float TargetLaneOffsetCm = 0.0f;
	float LaneChangeProgress = 0.0f;
	float LaneChangeDuration = 2.0f; // 2 segundos para cambiar de carril con suavidad
};
