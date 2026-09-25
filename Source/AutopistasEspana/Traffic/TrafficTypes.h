#pragma once

#include "CoreMinimal.h"
#include "TrafficTypes.generated.h"

/** Categorias de vehiculos en el simulador */
UENUM(BlueprintType)
enum class EVehicleCategory : uint8
{
	Turismo          UMETA(DisplayName = "Turismo Compacto"),
	Motocicleta      UMETA(DisplayName = "Motocicleta"),
	Furgoneta        UMETA(DisplayName = "Furgoneta de Reparto"),
	Autobus          UMETA(DisplayName = "Autobus Interurbano"),
	CamionTrailer    UMETA(DisplayName = "Camion Articulado con Trailer"),
	GuardiaCivil     UMETA(DisplayName = "Patrulla Guardia Civil de Trafico"),
	GruaAsistencia   UMETA(DisplayName = "Grua de Asistencia en Carretera")
};

/** Estado operativo y de siniestralidad del vehiculo */
UENUM(BlueprintType)
enum class EIncidentState : uint8
{
	Normal           UMETA(DisplayName = "Circulando Normalmente"),
	FrenadaEmergencia UMETA(DisplayName = "Frenada Fuerte / Evitacion"),
	AveriadoArcen    UMETA(DisplayName = "Averiado en Arcen con Warning"),
	Colisionado      UMETA(DisplayName = "Colisionado (Carril Bloqueado)"),
	EnRemolque       UMETA(DisplayName = "Siendo Remolcado por Grua")
};

/** Condición psicológica y temperamento del conductor en carretera */
UENUM(BlueprintType)
enum class ECondicionPsicologica : uint8
{
	Calmado          UMETA(DisplayName = "Calmado (Zen - Respeto Escrupuloso)"),
	Impaciente       UMETA(DisplayName = "Impaciente (Distancia Reducida)"),
	Estresado        UMETA(DisplayName = "Estresado (Ráfagas, Bocina y Tensión)"),
	FuriaAlVolante   UMETA(DisplayName = "Furia al Volante (Road Rage / Temerario)")
};

/** Estado psicologico y humor del conductor */
UENUM(BlueprintType)
enum class EDriverMood : uint8
{
	Tranquilo        UMETA(DisplayName = "Tranquilo (Zen)"),
	Impaciente       UMETA(DisplayName = "Impaciente"),
	Irritado         UMETA(DisplayName = "Irritado (Uso de Claxon)"),
	FuriaAlVolante   UMETA(DisplayName = "Furia al Volante (Road Rage / Temerario)")
};

/** Parametros configurables del modelo de psicologia y estres del conductor */
USTRUCT(BlueprintType)
struct FDriverPsychologyParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float PatienceThresholdSeconds = 15.0f; // Tiempo de paciencia antes de degradacion acelerada

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float JamStressRatePerSec = 4.5f; // Tasa de incremento de frustracion por segundo detenido en atasco

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float SlowTrafficStressRatePerSec = 1.8f; // Tasa de acumulacion al circular a baja velocidad (< 45% v0)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float FreeFlowReliefRatePerSec = 2.5f; // Tasa de alivio/recuperacion al circular fluido

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float CalmSafeTimeHeadway = 1.4f; // Distancia temporal modo Calmado (segundos)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float ImpatientSafeTimeHeadway = 0.9f; // Modo Impaciente

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float StressedSafeTimeHeadway = 0.5f; // Modo Estresado

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float RageSafeTimeHeadway = 0.2f; // Modo Furia: Acoso trasero / tailgating critico (0.2s)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float CalmJamDistanceCm = 250.0f; // 2.5 metros

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float RageJamDistanceCm = 40.0f; // 40 centimetros pegado a la chapa en furia al volante

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Psychology")
	float RearEndCollisionMultiplier = 5.0f; // Multiplicador x5 del riesgo de colision por alcance en furia
};

/** Parametros del Modelo de Conductor Inteligente (IDM) */
USTRUCT(BlueprintType)
struct FIDMParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IDM")
	float DesiredSpeedKmh = 120.0f; // Velocidad deseada v0 (km/h)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IDM")
	float SafeTimeHeadwaySeconds = 1.4f; // Tiempo de seguridad T (segundos)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IDM")
	float MinimumJamDistanceCm = 250.0f; // Distancia minima detenido en atasco s0 (2.5 metros)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IDM")
	float MaxAccelerationCmS2 = 280.0f; // Aceleracion confortable a (2.8 m/s^2)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IDM")
	float DesiredDecelerationCmS2 = 350.0f; // Deceleracion confortable b (3.5 m/s^2)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IDM")
	float EmergencyBrakingCmS2 = 750.0f; // Frenada maxima de emergencia (7.5 m/s^2)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IDM")
	float AccelerationExponent = 4.0f; // Exponente delta de aceleracion
};
