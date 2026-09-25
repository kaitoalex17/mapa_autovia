#pragma once

#include "CoreMinimal.h"
#include "EnvironmentTypes.generated.h"

/**
 * Tipologias de Climatologia Dinamica segun la geografia vial de Espana.
 * Abarca desde la aridez y sol de la meseta/sur hasta temporales extremos
 * mediterraneos (DANA), puertos de montana cantabricos/pirinaicos y bancos de niebla.
 */
UENUM(BlueprintType)
enum class EWeatherCondition : uint8
{
	Soleado        UMETA(DisplayName = "Soleado / Despejado (Seco)"),
	LluviaDANA     UMETA(DisplayName = "Lluvia Torrencial (DANA / Gota Fria)"),
	NievePuertos   UMETA(DisplayName = "Temporal de Nieve en Puertos de Montana"),
	NieblaDensa    UMETA(DisplayName = "Niebla Densa / Visibilidad Reducida")
};

/**
 * Periodos del ciclo de 24 horas y comportamiento del trafico en la red espanola.
 * Modela el flujo sociolaboral: horas punta pendulares, mediodia y logistica pesada nocturna.
 */
UENUM(BlueprintType)
enum class ERushHourPeriod : uint8
{
	Valle             UMETA(DisplayName = "Horario Valle Estandar (Trafico Regular)"),
	PuntaManana       UMETA(DisplayName = "Hora Punta Matinal (07:30 - 09:30)"),
	ValleMediodia     UMETA(DisplayName = "Hora Almuerzo / Escolar (14:00 - 15:30)"),
	PuntaTarde        UMETA(DisplayName = "Hora Punta de Tarde (18:00 - 20:30)"),
	NocheLogistica    UMETA(DisplayName = "Noche Logistica / Camiones Pesados (22:00 - 06:00)")
};

/**
 * Factores de agarre del firme, pelicula de agua y modificadores de friccion del asfalto.
 * Alimenta la cinematica de vehiculos (IDM), distancia de frenado y el motor de accidentes.
 */
USTRUCT(BlueprintType)
struct FRoadSurfaceGripFactors
{
	GENERATED_BODY()

	/** Coeficiente de friccion longitudinal y lateral del asfalto (mu): 1.0 = seco optimo, 0.55 = mojado DANA, 0.25 = nieve/hielo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grip")
	float FrictionCoefficient = 1.0f;

	/** Multiplicador de distancia de parada/frenado (1.0 = normal, 1.85 = DANA, 3.6 = nieve) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grip")
	float BrakingDistanceMultiplier = 1.0f;

	/** Multiplicador de susceptibilidad a perdida de traccion por aquaplaning (0.0 en seco, hasta 4.5+ en tormenta severa) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aquaplaning")
	float AquaplaningRiskMultiplier = 0.0f;

	/** Velocidad critica a partir de la cual el neumatico pierde contacto hidrodinamico con el firme (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aquaplaning")
	float AquaplaningSpeedThresholdKmh = 999.0f;

	/** Espesor estimado de la pelicula de agua o nieve sobre la calzada (milimetros) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Surface")
	float SurfaceWaterFilmThicknessMm = 0.0f;

	/** Factor de riesgo de accidente segun climatologia (K_clima de la formula DGT del GDD) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accident Engine")
	float WeatherAccidentRiskMultiplier = 1.0f;

	/** Limite de velocidad maximo aconsejado por la DGT (PMV) segun estado de la via (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Regulation")
	float RecommendedMaxSpeedKmh = 120.0f;

	/** Distancia visual atmosferica efectiva en metros (niebla, cortina de agua o nieve) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float VisibilityMeters = 10000.0f;

	/** Devuelve la configuracion de parametros estandar para un clima dado */
	static FRoadSurfaceGripFactors GetDefaultGripForWeather(EWeatherCondition Weather)
	{
		FRoadSurfaceGripFactors Factors;
		switch (Weather)
		{
		case EWeatherCondition::Soleado:
			Factors.FrictionCoefficient = 1.0f;
			Factors.BrakingDistanceMultiplier = 1.0f;
			Factors.AquaplaningRiskMultiplier = 0.0f;
			Factors.AquaplaningSpeedThresholdKmh = 999.0f;
			Factors.SurfaceWaterFilmThicknessMm = 0.0f;
			Factors.WeatherAccidentRiskMultiplier = 1.0f; // K_clima seco = 1.0
			Factors.RecommendedMaxSpeedKmh = 120.0f;
			Factors.VisibilityMeters = 10000.0f;
			break;

		case EWeatherCondition::LluviaDANA:
			// Tormenta violenta: caida del 45% de agarre, encharcamiento critico (>6mm) y riesgo severo de aquaplaning a >75 km/h
			Factors.FrictionCoefficient = 0.55f;
			Factors.BrakingDistanceMultiplier = 1.85f;
			Factors.AquaplaningRiskMultiplier = 4.5f;
			Factors.AquaplaningSpeedThresholdKmh = 75.0f;
			Factors.SurfaceWaterFilmThicknessMm = 6.5f;
			Factors.WeatherAccidentRiskMultiplier = 2.5f; // K_clima lluvia = 2.5 segun GDD 2.1
			Factors.RecommendedMaxSpeedKmh = 80.0f;
			Factors.VisibilityMeters = 350.0f;
			break;

		case EWeatherCondition::NievePuertos:
			// Temporal de frio y nieve en cotas altas: calzada helada, minima adherencia y obligacion de cadenas
			Factors.FrictionCoefficient = 0.25f;
			Factors.BrakingDistanceMultiplier = 3.6f;
			Factors.AquaplaningRiskMultiplier = 0.5f;
			Factors.AquaplaningSpeedThresholdKmh = 50.0f;
			Factors.SurfaceWaterFilmThicknessMm = 12.0f;
			Factors.WeatherAccidentRiskMultiplier = 6.0f; // K_clima nieve = 6.0 segun GDD 2.1
			Factors.RecommendedMaxSpeedKmh = 40.0f;
			Factors.VisibilityMeters = 150.0f;
			break;

		case EWeatherCondition::NieblaDensa:
			// Banco de niebla orografica: asfalto humedo pero peligro critico por perdida de visibilidad (<50m)
			Factors.FrictionCoefficient = 0.85f;
			Factors.BrakingDistanceMultiplier = 1.25f;
			Factors.AquaplaningRiskMultiplier = 0.8f;
			Factors.AquaplaningSpeedThresholdKmh = 95.0f;
			Factors.SurfaceWaterFilmThicknessMm = 1.0f;
			Factors.WeatherAccidentRiskMultiplier = 5.0f; // Choques en cadena por falta de anticipacion
			Factors.RecommendedMaxSpeedKmh = 60.0f;
			Factors.VisibilityMeters = 45.0f;
			break;
		}
		return Factors;
	}

	/** Interpolacion lineal suave entre dos perfiles de agarre durante transiciones meteorologicas */
	static FRoadSurfaceGripFactors Interpolate(const FRoadSurfaceGripFactors& A, const FRoadSurfaceGripFactors& B, float Alpha)
	{
		const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		FRoadSurfaceGripFactors Result;
		Result.FrictionCoefficient = FMath::Lerp(A.FrictionCoefficient, B.FrictionCoefficient, ClampedAlpha);
		Result.BrakingDistanceMultiplier = FMath::Lerp(A.BrakingDistanceMultiplier, B.BrakingDistanceMultiplier, ClampedAlpha);
		Result.AquaplaningRiskMultiplier = FMath::Lerp(A.AquaplaningRiskMultiplier, B.AquaplaningRiskMultiplier, ClampedAlpha);
		Result.AquaplaningSpeedThresholdKmh = FMath::Lerp(A.AquaplaningSpeedThresholdKmh, B.AquaplaningSpeedThresholdKmh, ClampedAlpha);
		Result.SurfaceWaterFilmThicknessMm = FMath::Lerp(A.SurfaceWaterFilmThicknessMm, B.SurfaceWaterFilmThicknessMm, ClampedAlpha);
		Result.WeatherAccidentRiskMultiplier = FMath::Lerp(A.WeatherAccidentRiskMultiplier, B.WeatherAccidentRiskMultiplier, ClampedAlpha);
		Result.RecommendedMaxSpeedKmh = FMath::Lerp(A.RecommendedMaxSpeedKmh, B.RecommendedMaxSpeedKmh, ClampedAlpha);
		Result.VisibilityMeters = FMath::Lerp(A.VisibilityMeters, B.VisibilityMeters, ClampedAlpha);
		return Result;
	}
};

/**
 * Resumen del estado temporal y demografico del trafico en un instante del ciclo 24h.
 */
USTRUCT(BlueprintType)
struct FTimeOfDayInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	float CurrentHour24 = 8.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	int32 Hours = 8;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	int32 Minutes = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	int32 Seconds = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	ERushHourPeriod RushHourPeriod = ERushHourPeriod::PuntaManana;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	bool bIsRushHour = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	bool bIsNightTruckPeriod = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic Demographics")
	float PassengerCarSpawnMultiplier = 1.8f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic Demographics")
	float HeavyTruckSpawnMultiplier = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	FString FormattedTime = TEXT("08:00");
};

// Delegados dinamicos para notificacion a UI, sistema de sonido, iluminacion y simulacion
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherConditionChanged, EWeatherCondition, NewWeather, const FRoadSurfaceGripFactors&, GripFactors);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTimeOfDayUpdated, float, CurrentHour24, int32, Hours, int32, Minutes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRushHourPeriodChanged, ERushHourPeriod, NewPeriod, ERushHourPeriod, PreviousPeriod);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAquaplaningAlert, float, WaterFilmThicknessMm, float, CriticalSpeedKmh, float, RiskFactor);
