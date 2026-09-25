#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DGT/DGTTypes.h"
#include "DGTControlSubsystem.generated.h"

class URoadSplineComponent;
class ATrafficVehicleAgent;
class APegasusHelicopterActor;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDGTInfractionIssued, const FDGTInfractionRecord&, InfractionRecord);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnControlPointDeployed, const FDGTControlPoint&, ControlPoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnControlPointRemoved, int32, ControlId);

/**
 * Subsistema Global de Operativos DGT y Seguridad Vial.
 * Gestiona los puntos de control en carretera (Alcoholemia, Pesaje de Camiones, Radares Fijos),
 * el balizamiento mediante estrechamientos conicos de conos de obra,
 * el registro centralizado de sanciones e ingresos (€) para el presupuesto publico,
 * y el calculo matematico de reduccion de siniestralidad vial en la red.
 */
UCLASS()
class AUTOPISTASESPANA_API UDGTControlSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Desplegar un nuevo punto de control en calzada
	UFUNCTION(BlueprintCallable, Category = "DGT|Operations")
	int32 DeployControlPoint(EDGTControlType ControlType, URoadSplineComponent* Road, float DistanceAlongSpline, float TaperLengthMeters = 150.0f, int32 ConesCount = 15);

	// Levantar y retirar un punto de control
	UFUNCTION(BlueprintCallable, Category = "DGT|Operations")
	bool RemoveControlPoint(int32 ControlId);

	// Retirar todos los puntos de control activos
	UFUNCTION(BlueprintCallable, Category = "DGT|Operations")
	void ClearAllControlPoints();

	// Obtener lista de controles activos
	UFUNCTION(BlueprintPure, Category = "DGT|Operations")
	const TArray<FDGTControlPoint>& GetActiveControlPoints() const { return ActiveControlPoints; }

	// Consultar control por su ID
	UFUNCTION(BlueprintCallable, Category = "DGT|Operations")
	bool GetControlPointById(int32 ControlId, FDGTControlPoint& OutControlPoint) const;

	// Contabilizar operativos en calzada
	UFUNCTION(BlueprintPure, Category = "DGT|Operations")
	int32 GetActiveCheckpointsCount() const { return ActiveControlPoints.Num(); }

	// Spawner y Despliegue de patrulla aerea Pegasus
	UFUNCTION(BlueprintCallable, Category = "DGT|Pegasus")
	APegasusHelicopterActor* SpawnPegasusPatrol(URoadSplineComponent* InitialRoad = nullptr, FVector CustomLocation = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "DGT|Pegasus")
	void RegisterPegasus(APegasusHelicopterActor* Helicopter);

	UFUNCTION(BlueprintCallable, Category = "DGT|Pegasus")
	void UnregisterPegasus(APegasusHelicopterActor* Helicopter);

	UFUNCTION(BlueprintPure, Category = "DGT|Pegasus")
	int32 GetActivePegasusCount() const { return ActivePegasusHelicopters.Num(); }

	// Registro de infracciones y gestion economica
	UFUNCTION(BlueprintCallable, Category = "DGT|Infractions")
	void RecordInfraction(const FDGTInfractionRecord& Record);

	UFUNCTION(BlueprintPure, Category = "DGT|Infractions")
	int64 GetTotalRevenueCollectedEuros() const { return OperationalStats.TotalRevenueCollectedEuros; }

	UFUNCTION(BlueprintPure, Category = "DGT|Infractions")
	const FDGTSummaryStats& GetOperationalStats() const { return OperationalStats; }

	UFUNCTION(BlueprintPure, Category = "DGT|Infractions")
	const TArray<FDGTInfractionRecord>& GetInfractionHistory() const { return InfractionHistory; }

	// Calculo de reduccion de siniestralidad y accidentalidad
	UFUNCTION(BlueprintPure, Category = "DGT|Safety")
	float CalculateGlobalAccidentReductionPercent() const;

	UFUNCTION(BlueprintPure, Category = "DGT|Safety")
	float GetAccidentReductionAtLocation(const FVector& Location) const;

	// Delegados de eventos
	UPROPERTY(BlueprintAssignable, Category = "DGT|Events")
	FOnDGTInfractionIssued OnInfractionIssued;

	UPROPERTY(BlueprintAssignable, Category = "DGT|Events")
	FOnControlPointDeployed OnControlPointDeployed;

	UPROPERTY(BlueprintAssignable, Category = "DGT|Events")
	FOnControlPointRemoved OnControlPointRemoved;

private:
	// Puntos de control activos en la red
	UPROPERTY()
	TArray<FDGTControlPoint> ActiveControlPoints;

	// Historico acumulado de expedientes sancionadores
	UPROPERTY()
	TArray<FDGTInfractionRecord> InfractionHistory;

	// Metricas globales
	UPROPERTY()
	FDGTSummaryStats OperationalStats;

	// Lista de helicopteros Pegasus en vuelo
	UPROPERTY()
	TArray<TWeakObjectPtr<APegasusHelicopterActor>> ActivePegasusHelicopters;

	// Contador autoincremental de ID de controles
	int32 NextControlId = 1001;

	// Handle de temporizador para chequeo periodico de calzada
	FTimerHandle CheckpointTimerHandle;

	// Ciclo periodico de inspeccion de vehiculos en puntos de control
	void ProcessCheckpointInspections();

	// Generacion geometrica de conos y balizamiento para estrechamiento de carril
	void SpawnBalizamientoCones(FDGTControlPoint& ControlPoint);

	// Limpieza de actores visuales de un control
	void DestroyControlVisualProps(FDGTControlPoint& ControlPoint);
};
