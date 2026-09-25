#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DGT/DGTTypes.h"
#include "PegasusHelicopterActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class URoadSplineComponent;
class ATrafficVehicleAgent;
class UDGTControlSubsystem;
class UEconomySubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPegasusInfractionDetected, const FDGTInfractionRecord&, InfractionRecord);

/**
 * Actor del Helicoptero de Vigilancia Aerea 'Pegasus' de la DGT.
 * Vuela a 300m de cota cenital patrullando autovias y carreteras nacionales.
 * Equipado con camara giroestabilizada y cinemometro radar laser aerotransportado (MX-15).
 * Detecta y fija automaticamente vehiculos con exceso de velocidad (>150 km/h) o acoso trasero,
 * emitiendo sanciones firmes e ingresando fondos en las arcas publicas.
 */
UCLASS()
class AUTOPISTASESPANA_API APegasusHelicopterActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APegasusHelicopterActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// Componentes visuales y estructurales
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HelicopterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MainRotorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TailRotorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CameraGimbalMesh;

	// Parametros de Vuelo y Cota Cenital
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Flight")
	float FlightAltitudeMeters = 300.0f; // Cota reglamentaria DGT: 300 metros

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Flight")
	float CruiseSpeedKmh = 190.0f; // Velocidad de crucero (~190 km/h)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Flight")
	float MainRotorRpmSpeed = 2160.0f; // Grados por segundo de giro del rotor principal

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Flight")
	float TailRotorRpmSpeed = 4320.0f; // Grados por segundo del rotor de cola

	// Estado y Mision
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pegasus|State")
	EPegasusFlightState FlightState = EPegasusFlightState::Patrullando;

	// Parametros de Deteccion y Radar Laser
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Radar")
	float DetectionRadiusMeters = 1000.0f; // Radio del cono de cobertura en tierra (1 km)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Radar")
	float SpeedingThresholdKmh = 150.0f; // Umbral de infraccion muy grave (>150 km/h)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Radar")
	float TailgatingSafetyDistanceMeters = 8.0f; // Distancia minima de seguridad critica (8 metros)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Radar")
	float TailgatingMinSpeedKmh = 80.0f; // Velocidad minima para computar acoso trasero

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Radar")
	float LaserLockRequiredSeconds = 2.5f; // Tiempo de fijacion continua para expedir multa

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pegasus|Radar")
	float CurrentLockTime = 0.0f;

	// Visualizacion Laser y HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pegasus|Visuals")
	bool bDrawLaserDebugBeam = true;

	// Carretera asignada para patrulla
	UPROPERTY(BlueprintReadWrite, Category = "Pegasus|Patrol")
	TWeakObjectPtr<URoadSplineComponent> CurrentPatrolRoad;

	// Objetivo fijado
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pegasus|Target")
	TWeakObjectPtr<ATrafficVehicleAgent> CurrentTargetVehicle;

	// Delegado de evento de sancion
	UPROPERTY(BlueprintAssignable, Category = "Pegasus|Events")
	FOnPegasusInfractionDetected OnInfractionDetected;

	// Funciones de control de mision
	UFUNCTION(BlueprintCallable, Category = "Pegasus|Mission")
	void AssignRoadToPatrol(URoadSplineComponent* RoadSpline);

	UFUNCTION(BlueprintCallable, Category = "Pegasus|Mission")
	void SetZenitalAltitude(float InAltitudeMeters);

	UFUNCTION(BlueprintCallable, Category = "Pegasus|Targeting")
	void ManualLockVehicle(ATrafficVehicleAgent* TargetVehicle);

	UFUNCTION(BlueprintCallable, Category = "Pegasus|Targeting")
	void ClearCurrentTarget();

	UFUNCTION(BlueprintCallable, Category = "Pegasus|Targeting")
	bool IssueElectronicFine(EInfractionType OverrideType = EInfractionType::ExcesoVelocidad);

	UFUNCTION(BlueprintPure, Category = "Pegasus|Targeting")
	float GetLockProgressPercent() const;

	UFUNCTION(BlueprintPure, Category = "Pegasus|Targeting")
	FVector GetCameraGimbalWorldLocation() const;

private:
	// Progreso del vuelo a lo largo del spline
	float CurrentDistanceAlongSpline = 0.0f;
	bool bPatrolForward = true;

	// Temporizador de estado sancionando
	float PostSanctionCooldown = 0.0f;

	// Busqueda automatica de infractores en el cono de radar
	void ScanForInfractors(float DeltaTime);

	// Evaluacion de acoso trasero para un vehiculo dado
	bool EvaluateTailgating(ATrafficVehicleAgent* SubjectVehicle, float& OutPredecessorDistanceMeters) const;

	// Actualizacion cinematica del vuelo del helicoptero
	void UpdateFlightPath(float DeltaTime);

	// Orientacion de la torreta MX-15 hacia el vehiculo
	void UpdateCameraGimbal(float DeltaTime);

	// Dibujado del haz laser infrarrojo/visible del radar
	void RenderLaserRadarBeam();

	// Efecto disuasorio sobre el trafico proximo
	void ApplyDissuasiveEffect();
};
