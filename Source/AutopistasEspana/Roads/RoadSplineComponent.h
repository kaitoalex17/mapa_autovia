#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Roads/RoadTypes.h"
#include "RoadSplineComponent.generated.h"

class UProceduralMeshComponent;

/**
 * Componente de Spline Vial.
 * Modela el trazado geometrico de carreteras, enlaces a nivel de carril,
 * cunas de aceleracion/deceleracion (Norma 8.1-IC), y genera proceduralmente
 * la malla del firme, tableros de viaducto y pilares de soporte para pasos a distinto nivel.
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

	// Forzado de viaducto continuo (utilizado en pasos a distinto nivel / flyovers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation|Structures")
	bool bIsViaductFlyover = false;

	// Separacion longitudinal entre pilares de viaducto (en UU / cm, ej. 2500 UU = 25m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation|Structures")
	float ViaductPillarSpacing = 2500.0f;

	// Espesor del tablero de viaducto (canto de hormigon en cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation|Structures")
	float ViaductDeckThickness = 80.0f;

	// --- Cuna de Aceleracion / Deceleracion (Norma 8.1-IC) ---

	// Activa la generacion de cuna de aceleracion o deceleracion en la malla procedural
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation|Taper")
	bool bHasAccelerationTaper = false;

	// Longitud de la cuna de aceleracion (Norma 8.1-IC: 15000 UU = 150 metros)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation|Taper")
	float AccelerationTaperLength = 15000.0f;

	// Ubicacion de la cuna: true = al final del tramo (merge de incorporacion), false = al inicio (diverge de salida)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation|Taper")
	bool bTaperAtEnd = true;

	// Ancho maximo del carril en la cuna (350 cm = 3.50m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Generation|Taper")
	float TaperLaneWidth = 350.0f;

	// --- Generacion de Malla y Geometria ---

	// Regenera la malla procedural completa a lo largo del spline (firme, tablero y pilares)
	UFUNCTION(BlueprintCallable, Category = "Road Generation")
	void GenerateRoadMesh(UProceduralMeshComponent* TargetMesh);

	// Detecta si un punto del spline esta elevado formando un puente o viaducto
	UFUNCTION(BlueprintCallable, Category = "Road Generation")
	ERoadElevationType GetElevationTypeAtDistance(float DistanceAlongSpline) const;

	// Obtiene los anchos laterales de calzada a una distancia dada, considerando cunas si estan activas
	UFUNCTION(BlueprintPure, Category = "Road Generation")
	float GetCarriagewayWidthAtDistance(float DistanceAlongSpline, float& OutLeftWidth, float& OutRightWidth) const;

	// Obtiene la distancia lateral desde el eje del spline al centro de un carril especifico
	UFUNCTION(BlueprintPure, Category = "Road Generation")
	float GetLaneCenterOffset(int32 LaneIndex) const;

	// --- Conectividad a Nivel de Carril (Norma 8.1-IC) ---

	/**
	 * Conecta el inicio del spline (punto 0) a un carril especifico de otra carretera.
	 * Ajusta posicion y tangente del primer punto para alineacion tangencial perfecta.
	 * @param TargetRoad Carretera principal a la que conectarse.
	 * @param TargetLaneIndex Indice de carril de la carretera principal (0 = carril derecho).
	 * @param TargetDistanceAlongSpline Distancia a lo largo del spline destino (-1 para buscar el punto mas proximo).
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Connections")
	void ConnectStartToLane(URoadSplineComponent* TargetRoad, int32 TargetLaneIndex, float TargetDistanceAlongSpline = -1.0f);

	/**
	 * Conecta el final del spline (ultimo punto) a un carril especifico de otra carretera.
	 * Ajusta posicion y tangente del ultimo punto para una incorporacion tangencial fluida.
	 * @param TargetRoad Carretera principal a la que conectarse.
	 * @param TargetLaneIndex Indice de carril de la carretera principal (0 = carril derecho).
	 * @param TargetDistanceAlongSpline Distancia a lo largo del spline destino (-1 para buscar el punto mas proximo).
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Connections")
	void ConnectEndToLane(URoadSplineComponent* TargetRoad, int32 TargetLaneIndex, float TargetDistanceAlongSpline = -1.0f);

	/**
	 * Conecta un extremo del spline (inicio o fin) al carril de otra carretera.
	 * @param bConnectStart True para inicio, False para fin.
	 * @param TargetRoad Carretera destino.
	 * @param TargetLaneIndex Indice del carril destino.
	 * @param TargetDistanceAlongSpline Distancia a lo largo del spline destino (-1 para mas cercana).
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Connections")
	void ConnectSplineEndToRoadLane(bool bConnectStart, URoadSplineComponent* TargetRoad, int32 TargetLaneIndex, float TargetDistanceAlongSpline = -1.0f);

	/**
	 * Configura y activa una cuna de aceleracion reglamentaria de 150m (Norma 8.1-IC).
	 * @param bEnable Activar o desactivar cuna.
	 * @param Length Longitud en centimetros (por defecto 15000 cm = 150m).
	 * @param bAtEnd True para final del tramo (merge), false para inicio (diverge).
	 */
	UFUNCTION(BlueprintCallable, Category = "Road Spline|Taper")
	void EnableAccelerationTaper(bool bEnable = true, float Length = 15000.0f, bool bAtEnd = true);
};
