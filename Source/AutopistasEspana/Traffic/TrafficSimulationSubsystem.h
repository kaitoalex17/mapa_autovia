#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Traffic/TrafficTypes.h"
#include "TrafficSimulationSubsystem.generated.h"

class ATrafficVehicleAgent;
class URoadSplineComponent;

/**
 * Subsistema Global de Simulacion de Trafico.
 * Gestiona el ciclo de vida de los vehiculos, emparejamiento con el vehiculo predecesor para IDM,
 * deteccion de colisiones/accidentes masivos y calculo de metricas de fluidez de la red.
 */
UCLASS()
class AUTOPISTASESPANA_API UTrafficSimulationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Registrar vehiculo en la simulación
	UFUNCTION(BlueprintCallable, Category = "Traffic Simulation")
	void RegisterVehicle(ATrafficVehicleAgent* Vehicle);

	// Dar de baja vehiculo
	UFUNCTION(BlueprintCallable, Category = "Traffic Simulation")
	void UnregisterVehicle(ATrafficVehicleAgent* Vehicle);

	// Spawner de vehiculo en una carretera determinada
	UFUNCTION(BlueprintCallable, Category = "Traffic Simulation")
	ATrafficVehicleAgent* SpawnVehicleOnRoad(TSubclassOf<ATrafficVehicleAgent> VehicleClass, URoadSplineComponent* Road, float StartDistance, int32 LaneIndex);

	// Provocar un accidente aleatorio en la red para pruebas de gestion de atascos
	UFUNCTION(BlueprintCallable, Category = "Traffic Simulation|Testing")
	void TriggerRandomAccident();

	// Metricas de Trafico en tiempo real para el HUD
	UFUNCTION(BlueprintCallable, Category = "Traffic Metrics")
	int32 GetTotalVehiclesCount() const { return ActiveVehicles.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Traffic Metrics")
	int32 GetActiveAccidentsCount() const;

	UFUNCTION(BlueprintCallable, Category = "Traffic Metrics")
	float GetAverageNetworkSpeedKmh() const;

	UFUNCTION(BlueprintCallable, Category = "Traffic Metrics")
	float GetCongestionPercentage() const;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<ATrafficVehicleAgent>> ActiveVehicles;
};
