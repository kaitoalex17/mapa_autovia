#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Roads/RoadTypes.h"
#include "RoadNetworkSubsystem.generated.h"

class URoadSplineComponent;

/**
 * Subsistema Global de la Red de Carreteras.
 * Registra y gestiona todos los tramos de carretera, calcula conexiones topologicas entre vias y sirve de soporte para el enrutamiento de vehiculos.
 */
UCLASS()
class AUTOPISTASESPANA_API URoadNetworkSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Registrar un nuevo segmento vial en el grafo del mundo
	UFUNCTION(BlueprintCallable, Category = "Road Network")
	void RegisterRoadSegment(URoadSplineComponent* RoadSegment);

	// Eliminar un segmento vial (demolicion)
	UFUNCTION(BlueprintCallable, Category = "Road Network")
	void UnregisterRoadSegment(URoadSplineComponent* RoadSegment);

	// Obtener el numero de tramos registrados
	UFUNCTION(BlueprintCallable, Category = "Road Network")
	int32 GetTotalRoadSegmentsCount() const { return RegisteredRoads.Num(); }

	// Calcular longitud total de la red en kilometros
	UFUNCTION(BlueprintCallable, Category = "Road Network")
	float GetTotalNetworkLengthKm() const;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<URoadSplineComponent>> RegisteredRoads;
};
