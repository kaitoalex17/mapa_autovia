#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Roads/RoadTypes.h"
#include "RoadSplineComponent.generated.h"

class UProceduralMeshComponent;

/**
 * Componente de Spline Vial.
 * Modela el trazado de carreteras y genera proceduralmente la malla de asfalto, marcas viales y viaductos.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AUTOPISTASESPANA_API URoadSplineComponent : public USplineComponent
{
	GENERATED_BODY()

public:
	URoadSplineComponent();

	// Configuracion de la seccion transversal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Properties")
	ERoadCategory RoadCategory = ERoadCategory::Autovia_120;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Properties")
	FRoadCrossSection CrossSection;

	// Paso de muestreo a lo largo del spline para generar la malla (en centimetros / UU)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation")
	float SegmentStepLength = 200.0f; // Muestreo cada 2.0 metros para curvas suaves

	// Altura umbral para convertir la via en viaducto con pilares
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation|Structures")
	float ViaductHeightThreshold = 300.0f; // Si Z > 3.0m, es viaducto

	// Regenera la malla procedural completa a lo largo del spline
	UFUNCTION(BlueprintCallable, Category = "Road Generation")
	void GenerateRoadMesh(UProceduralMeshComponent* TargetMesh);

	// Detecta si un punto del spline esta elevado formando un puente
	UFUNCTION(BlueprintCallable, Category = "Road Generation")
	ERoadElevationType GetElevationTypeAtDistance(float DistanceAlongSpline) const;
};
