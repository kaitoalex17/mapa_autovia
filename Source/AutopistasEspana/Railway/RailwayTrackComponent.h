#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Railway/RailwayTypes.h"
#include "RailwayTrackComponent.generated.h"

class UProceduralMeshComponent;

/**
 * Componente de Spline Ferroviario para 'Autopistas de Espana'.
 * Modela el trazado de lineas de ferrocarril y genera proceduralmente
 * la superestructura de via: prisma de balasto, traviesas de hormigon y carriles de acero (UIC 60).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AUTOPISTASESPANA_API URailwayTrackComponent : public USplineComponent
{
	GENERATED_BODY()

public:
	URailwayTrackComponent();

	/** Configuracion de ancho de via (Iberico 1668mm, UIC 1435mm o Metrico) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railway Properties")
	ERailwayTrackGauge GaugeType = ERailwayTrackGauge::Iberico1668;

	/** Parametros geometricos de la superestructura de via */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railway Properties")
	FRailwayTrackConfig TrackConfig;

	/** Paso de muestreo a lo largo del spline para generar la malla (centimetros) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation")
	float SegmentStepLength = 150.0f;

	/** Velocidad maxima de diseno del tramo ferroviario (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railway Properties")
	float SpeedLimitKmh = 140.0f;

	/** Indica si el tramo dispone de catenaria y electrificacion a 25 kV o 3 kV CC */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railway Properties")
	bool bElectrifiedTrack = true;

	/** Genera proceduralmente la malla completa: Seccion 0 = Balasto, Seccion 1 = Raíles de Acero, Seccion 2 = Traviesas */
	UFUNCTION(BlueprintCallable, Category = "Railway Generation")
	void GenerateTrackMesh(UProceduralMeshComponent* TargetMesh);

	/** Obtiene la coordenada 3D exacta del carril izquierdo o derecho a una distancia determinada */
	UFUNCTION(BlueprintPure, Category = "Railway Geometry")
	FVector GetRailPositionAtDistance(float DistanceAlongSpline, bool bRightRail, ESplineCoordinateSpace::Type Space = ESplineCoordinateSpace::World) const;

	/** Obtiene la orientacion y rotacion de la via a una distancia dada */
	UFUNCTION(BlueprintPure, Category = "Railway Geometry")
	FRotator GetTrackRotationAtDistance(float DistanceAlongSpline, ESplineCoordinateSpace::Type Space = ESplineCoordinateSpace::World) const;

	/** Comprueba si el punto se encuentra en una curva pronunciada para calcular peralte reglamentario */
	UFUNCTION(BlueprintPure, Category = "Railway Geometry")
	float CalculateCurvatureCant(float DistanceAlongSpline) const;

private:
	/** Genera el prisma trapezoidal de balasto de piedra machacada */
	void BuildBallastMeshSection(UProceduralMeshComponent* TargetMesh, int32 SectionIndex);

	/** Genera los dos perfiles de carril de acero continuo soldado */
	void BuildSteelRailsMeshSection(UProceduralMeshComponent* TargetMesh, int32 SectionIndex);

	/** Genera las traviesas monobloque de hormigon a intervalos regulares */
	void BuildSleepersMeshSection(UProceduralMeshComponent* TargetMesh, int32 SectionIndex);
};
