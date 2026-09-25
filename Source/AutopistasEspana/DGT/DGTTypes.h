#pragma once

#include "CoreMinimal.h"
#include "Traffic/TrafficTypes.h"
#include "DGTTypes.generated.h"

class URoadSplineComponent;
class AActor;

/**
 * Tipos de Dispositivos y Operativos de Control de la DGT y Guardia Civil
 */
UENUM(BlueprintType)
enum class EDGTControlType : uint8
{
	Alcoholemia       UMETA(DisplayName = "Control Preventivo de Alcoholemia y Drogas"),
	PesajeCamiones    UMETA(DisplayName = "Control de Pesaje, MMA y Tacografo de Camiones"),
	RadarFijo         UMETA(DisplayName = "Cabina de Radar Fijo / Cinemometro en Portico"),
	Pegasus           UMETA(DisplayName = "Vigilancia Aerea con Helicoptero Pegasus")
};

/**
 * Tipos de Infracciones de Trafico tipificadas segun la Ley de Seguridad Vial
 */
UENUM(BlueprintType)
enum class EInfractionType : uint8
{
	ExcesoVelocidad       UMETA(DisplayName = "Exceso de Velocidad (>150 km/h o superior al limite)"),
	AcosoTrasero          UMETA(DisplayName = "Acoso Trasero / Distancia de Seguridad Insuficiente"),
	AlcoholemiaPositiva   UMETA(DisplayName = "Tasa Positiva en Aire Espirado / Sustancias Psicotropicas"),
	SobrecargaPesaje      UMETA(DisplayName = "Exceso de Masa Maxima Autorizada (MMA)"),
	ConduccionTemeraria   UMETA(DisplayName = "Conduccion Temeraria con Peligro Manifiesto"),
	UsoMovilDistraccion   UMETA(DisplayName = "Uso Manual de Telefonia Movil durante la Marcha")
};

/**
 * Estados operativos y de mision de la unidad aerea Pegasus
 */
UENUM(BlueprintType)
enum class EPegasusFlightState : uint8
{
	Patrullando       UMETA(DisplayName = "Patrullando Autovia (Cota Cenital 300m)"),
	FijandoObjetivo   UMETA(DisplayName = "Fijando Vehiculo Infractor (Laser Tracking)"),
	Sancionando       UMETA(DisplayName = "Emitiendo Sancion Electronica y Telemetria"),
	RegresandoBase    UMETA(DisplayName = "Regresando a Helisuperficie")
};

/**
 * Expediente y registro individual de sancion DGT
 */
USTRUCT(BlueprintType)
struct FDGTInfractionRecord
{
	GENERATED_BODY()

	// Identificador unico del boletin de denuncia
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	FGuid InfractionId;

	// Tipologia de la infraccion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	EInfractionType InfractionType = EInfractionType::ExcesoVelocidad;

	// Medio o dispositivo captador de la infraccion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	EDGTControlType DetectedBy = EDGTControlType::Pegasus;

	// Descripcion o matricula del vehiculo denunciado
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	FString VehicleDescription = TEXT("Turismo");

	// Categoria vehicular
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	EVehicleCategory VehicleCategory = EVehicleCategory::Turismo;

	// Velocidad constatada en el momento de la captacion (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	float DetectedSpeedKmh = 0.0f;

	// Limite generico o especifico del tramo (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	float SpeedLimitKmh = 120.0f;

	// Distancia al vehiculo precedente en metros (relevante para acoso trasero)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	float DistanceToPredecessorMeters = 0.0f;

	// Cuantia de la sancion economica en Euros (€)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	int64 FineAmountEuros = 600;

	// Detraccion de puntos del permiso de conducir
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	int32 PointsDeducted = 6;

	// Coordenadas mundiales donde se cometio la infraccion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	FVector InfractionLocation = FVector::ZeroVector;

	// Marca temporal de la sancion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	FDateTime Timestamp = FDateTime::UtcNow();

	// Estado de cobro de la multa
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Infraction")
	bool bPaid = true;

	FDGTInfractionRecord()
	{
		InfractionId = FGuid::NewGuid();
		Timestamp = FDateTime::UtcNow();
	}
};

/**
 * Informacion y parametros de configuracion de un Punto de Control en Carretera
 */
USTRUCT(BlueprintType)
struct FDGTControlPoint
{
	GENERATED_BODY()

	// ID unico del dispositivo en la red
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	int32 ControlId = 0;

	// Tipo de dispositivo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	EDGTControlType ControlType = EDGTControlType::Alcoholemia;

	// Posicion geografica en el mundo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	FVector WorldLocation = FVector::ZeroVector;

	// Orientacion del operativo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	FRotator WorldRotation = FRotator::ZeroRotator;

	// Spline de carretera donde se ubica el control
	UPROPERTY(BlueprintReadOnly, Category = "DGT Checkpoint")
	TWeakObjectPtr<URoadSplineComponent> RoadSpline;

	// Distancia sobre el eje del spline (en centimetros)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	float DistanceAlongSpline = 0.0f;

	// Longitud de conificacion y balizamiento en metros (taper de transicion)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	float ConeTaperLengthMeters = 150.0f;

	// Numero de conos de balizamiento desplegados
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	int32 NumConesDeployed = 15;

	// Velocidad reducida obligatoria en la zona de balizamiento (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	float TargetSpeedLimitKmh = 40.0f;

	// Coeficiente de reduccion de siniestralidad (ej. 0.70 = -70% de accidentes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	float AccidentReductionFactor = 0.70f;

	// Coste diario de mantenimiento de la patrulla y logistica (€)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	int32 DailyOperationalCostEuros = 450;

	// Indicador de si el control esta en funcionamiento activo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DGT Checkpoint")
	bool bActive = true;

	// Actores auxiliares generados en el mundo (conos, patrulla, furgoneta, radar)
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SpawnedVisualActors;
};

/**
 * Estadisticas y balance global de intervenciones DGT
 */
USTRUCT(BlueprintType)
struct FDGTSummaryStats
{
	GENERATED_BODY()

	// Total de denuncias emitidas
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DGT Stats")
	int32 TotalInfractions = 0;

	// Total recaudado acumulado (€)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DGT Stats")
	int64 TotalRevenueCollectedEuros = 0;

	// Total de puntos detraidos
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DGT Stats")
	int32 TotalPointsWithdrawn = 0;

	// Desglose por tipo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DGT Stats")
	int32 SpeedingInfractions = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DGT Stats")
	int32 TailgatingInfractions = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DGT Stats")
	int32 AlcoholInfractions = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DGT Stats")
	int32 OverloadInfractions = 0;

	// Porcentaje global ponderado de reduccion de siniestralidad en la red
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DGT Stats")
	float GlobalAccidentReductionPercentage = 0.0f;
};
