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

/** Estado psicologico y humor del conductor */
UENUM(BlueprintType)
enum class EDriverMood : uint8
{
	Tranquilo        UMETA(DisplayName = "Tranquilo (Zen)"),
	Impaciente       UMETA(DisplayName = "Impaciente"),
	Irritado         UMETA(DisplayName = "Irritado (Uso de Claxon)"),
	FuriaAlVolante   UMETA(DisplayName = "Furia al Volante (Road Rage / Temerario)")
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
