#pragma once

#include "CoreMinimal.h"
#include "RailwayTypes.generated.h"

/**
 * Tipos de convoy ferroviario que operan en la red de 'Autopistas de Espana'.
 * Cumple con la clasificacion tipica de ADIF / Renfe y servicios intermodales.
 */
UENUM(BlueprintType)
enum class ETrainType : uint8
{
	/** Tren de mercancias pesadas, contenedores y transporte intermodal (Autopista Ferroviaria) */
	MercanciasContenedores    UMETA(DisplayName = "Mercancias y Contenedores (Autopista Ferroviaria)"),

	/** Tren de cercanias de pasajeros y media distancia interurbana */
	CercaniasPasajeros        UMETA(DisplayName = "Cercanias y Pasajeros Interurbanos"),

	/** Tren de Alta Velocidad (AVE) de largo recorrido */
	AltaVelocidadAVE          UMETA(DisplayName = "Alta Velocidad Espanola (AVE)"),

	/** Locomotora de maniobras, mantenimiento de infraestructura (ADIF) y auxilio/rescate */
	MantenimientoAuxilio      UMETA(DisplayName = "Locomotora de Maniobras y Rescate")
};

/**
 * Estados del ciclo de seguridad y enclavamiento de un Paso a Nivel.
 * Regulado por el Reglamento de Circulacion Ferroviaria (RCF) y normativa de ADIF.
 */
UENUM(BlueprintType)
enum class ELevelCrossingState : uint8
{
	/** Paso libre para el trafico rodado de la carretera; barreras alzadas a 90 grados y luces apagadas */
	Abierto                   UMETA(DisplayName = "Abierto (Paso libre a vehiculos)"),

	/** Tren en zona de aproximacion; inicio de senales opticas rojas alternantes y aviso acustico */
	AvisoTrenAproximandose    UMETA(DisplayName = "Aviso (Senales luminosas y acusticas activadas)"),

	/** Las semibarreras motorizadas descienden de 90 a 0 grados para bloquear la calzada */
	BajandoBarreras           UMETA(DisplayName = "Bajando Semibarreras"),

	/** Paso a nivel enclavado y completamente cerrado; exclusion total del trafico rodado durante el paso del tren */
	Cerrado                   UMETA(DisplayName = "Cerrado Total (Paso del tren)"),

	/** El tren ha rebasado el paso a nivel; las semibarreras ascienden de 0 a 90 grados */
	SubiendoBarreras          UMETA(DisplayName = "Subiendo Semibarreras"),

	/** Estado de fallo en circuito de via, obstaculo detectado o corte electrico */
	AveriaEmergencia          UMETA(DisplayName = "Averia / Emergencia")
};

/**
 * Clasificacion reglamentaria de pasos a nivel segun el nivel de proteccion activa.
 */
UENUM(BlueprintType)
enum class ELevelCrossingClass : uint8
{
	/** Senalizacion fija pasiva: Cruz de San Andres y Stop (baja intensidad de trafico) */
	ClaseA_Pasivo             UMETA(DisplayName = "Clase A: Senales Pasivas (Cruz de San Andres)"),

	/** Proteccion automatica luminosa y acustica sin barreras */
	ClaseB_SennalesLuminosas  UMETA(DisplayName = "Clase B: Proteccion Luminosa y Acustica"),

	/** Proteccion automatica con semibarreras abatibles (SLA) y senales luminosas rojas */
	ClaseC_Semibarreras       UMETA(DisplayName = "Clase C: Semibarreras Abatibles (SLA)"),

	/** Barreras completas con enclavamiento centralizado */
	ClaseD_BarrerasCompletas  UMETA(DisplayName = "Clase D: Barreras Dobles Completas")
};

/**
 * Ancho de via (Gauge) ferroviario.
 */
UENUM(BlueprintType)
enum class ERailwayTrackGauge : uint8
{
	/** Ancho Iberico tradicional ADIF (1.668 mm) */
	Iberico1668               UMETA(DisplayName = "Ancho Iberico (1.668 mm)"),

	/** Ancho Internacional / Estandar UIC Alta Velocidad (1.435 mm) */
	Internacional1435         UMETA(DisplayName = "Ancho Internacional UIC (1.435 mm)"),

	/** Ancho Metrico de via estrecha FEVE / RAM (1.000 mm) */
	Metrico1000               UMETA(DisplayName = "Ancho Metrico (1.000 mm)")
};

/**
 * Parametros geometricos y fisicos de la superestructura ferroviaria.
 */
USTRUCT(BlueprintType)
struct FRailwayTrackConfig
{
	GENERATED_BODY()

	/** Ancho de via entre cabezas de carril en centimetros (166.8 cm por defecto para Espana) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track Geometry")
	float GaugeWidthCm = 166.8f;

	/** Anchura de la coronacion superior del prisma de balasto (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ballast Profile")
	float BallastTopWidthCm = 340.0f;

	/** Anchura de la base inferior del prisma de balasto (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ballast Profile")
	float BallastBaseWidthCm = 460.0f;

	/** Espesor / Altura del prisma de balasto bajo las traviesas (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ballast Profile")
	float BallastHeightCm = 38.0f;

	/** Ancho de la cabeza del carril de acero (Perfil UIC 60: aprox 7.2 cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Profile")
	float RailHeadWidthCm = 7.5f;

	/** Altura del carril de acero desde patin hasta cabeza (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Profile")
	float RailHeightCm = 17.2f;

	/** Distancia entre ejes de traviesas consecutivas (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleepers Profile")
	float SleeperSpacingCm = 60.0f;

	/** Longitud total de cada traviesa de hormigon monobloque (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleepers Profile")
	float SleeperLengthCm = 260.0f;

	/** Anchura de la traviesa en sentido del avance (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleepers Profile")
	float SleeperWidthCm = 30.0f;

	/** Altura / canto de la traviesa de hormigon (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleepers Profile")
	float SleeperHeightCm = 22.0f;
};

/**
 * Parametros de temporizacion y seguridad del paso a nivel.
 */
USTRUCT(BlueprintType)
struct FLevelCrossingSettings
{
	GENERATED_BODY()

	/** Tiempo de preaviso acustico y optico antes de empezar a bajar las semibarreras (segundos) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float PreWarningDurationSeconds = 6.0f;

	/** Tiempo que tarda el motor de la semibarrera en bajar o subir completamente (segundos) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BarrierMovementDurationSeconds = 4.0f;

	/** Margen de seguridad con barreras cerradas antes de que el tren entre al paso (segundos) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float ClearanceSafetyMarginSeconds = 5.0f;

	/** Frecuencia de parpadeo alternante de los focos rojos LED (Hz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signaling")
	float RedLightBlinkFrequencyHz = 1.5f;

	/** Distancia de frenado y parada obligatoria para vehiculos rodados ante la barrera (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Traffic")
	float RoadStoppingDistanceCm = 450.0f;

	/** Distancia previa en la via a la que el sensor de via detecta el tren aproximandose (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track Sensors")
	float TrainDetectionDistanceCm = 30000.0f; // 300 metros

	/** Distancia tras el paso a nivel a la que se considera librada la via (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track Sensors")
	float TrainClearanceDistanceCm = 4500.0f; // 45 metros
};

/**
 * Informacion de carga o vehiculos retirados por la Autopista Ferroviaria.
 */
USTRUCT(BlueprintType)
struct FRailwayCargoManifest
{
	GENERATED_BODY()

	/** Numero total de camiones articulados retirados de la autovia */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cargo Manifest")
	int32 SemiTrucksCarried = 0;

	/** Numero total de turismos o furgonetas transportados */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cargo Manifest")
	int32 LightVehiclesCarried = 0;

	/** Tonelaje total de carga retirada de la red viaria */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cargo Manifest")
	float TotalCargoTons = 0.0f;

	/** Reduccion estimada de emisiones CO2 (kg) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cargo Manifest")
	float EstimatedCO2SavedKg = 0.0f;

	/** Ingresos generados por canon intermodal de mercancias (€) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cargo Manifest")
	int64 IntermodalRevenueEuros = 0;
};
