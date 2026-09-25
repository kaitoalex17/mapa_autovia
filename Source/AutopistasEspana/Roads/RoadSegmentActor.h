#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Roads/RoadTypes.h"
#include "RoadSegmentActor.generated.h"

class URoadSplineComponent;
class UProceduralMeshComponent;

/**
 * Actor que representa un Tramo de Carretera en el Mundo.
 * Contiene el Spline geometrico y la Malla Procedural del firme, conectandose de forma automatica
 * al subsistema global de red vial (URoadNetworkSubsystem).
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Components")
	URoadSplineComponent* RoadSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Components")
	UProceduralMeshComponent* RoadMesh;

	// Regenerar la geometria del tramo
	UFUNCTION(BlueprintCallable, Category = "Road Building")
	void RebuildRoadGeometry();

	// Anadir un nuevo punto de trazado en coordenadas de mundo
	UFUNCTION(BlueprintCallable, Category = "Road Building")
	void AddSplinePointAtWorldLocation(const FVector& WorldLocation);

	// Configurar la tipologia de via (Convencional, Autovia 2x2, etc.)
	UFUNCTION(BlueprintCallable, Category = "Road Building")
	void SetRoadCategory(ERoadCategory NewCategory);
};
