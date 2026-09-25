#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Environment/EnvironmentTypes.h"
#include "WeatherSubsystem.generated.h"

class ADirectionalLight;
class UDirectionalLightComponent;

/**
 * Subsistema Global de Climatologia Dinamica y Ciclo Dia/Noche para 'Autopistas de Espana'.
 * 
 * Responsabilidades Principales:
 * 1. Ciclo de 24 horas continuo y deteccion de franjas de trafico reales en Espana:
 *    - Horas punta de manana (07:30 a 09:30) y tarde (18:00 a 20:30).
 *    - Periodo nocturno de transporte de mercancias pesadas (22:00 a 06:00).
 * 2. Rotacion astronomica y control cenital de la luz solar (Directional Light):
 *    - Calculo de acimut y elevacion para la peninsula iberica (~40°N).
 *    - Modulacion de intensidad y temperatura de color (amanecer dorado, cenit diurno y luz lunar).
 * 3. Gestion del agarre del firme y alimentacion al motor de accidentes:
 *    - Coeficientes de friccion dinamicos segun climatologia (Soleado, DANA, Nieve, Niebla).
 *    - Modelo hidrodinamico de aquaplaning para vehiculos a alta velocidad sobre asfalto inundado.
 *    - Factor K_clima y evaluacion de probabilidad de siniestralidad vial segun normativa DGT.
 */
UCLASS()
class AUTOPISTASESPANA_API UWeatherSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	//~ Begin FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UWeatherSubsystem, STATGROUP_Tickables); }
	virtual bool IsTickable() const override { return !IsTemplate(); }
	//~ End FTickableGameObject Interface

	/* =========================================================================
	 *  SECCION 1: CICLO DE 24 HORAS Y HORARIOS LABORALES / LOGISTICOS
	 * ========================================================================= */

	/** Hora actual del dia en formato decimal [0.0f, 24.0f) (ej. 8.5 = 08:30) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Time Cycle")
	float CurrentHour = 8.0f;

	/** Duracion de un dia completo (24 horas de juego) en segundos de tiempo real (por defecto 1200s = 20 minutos) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Time Cycle", meta = (ClampMin = "30.0", UIMin = "30.0"))
	float DayDurationRealSeconds = 1200.0f;

	/** Multiplicador de velocidad del tiempo (1.0 = normal, 2.0 = rapido, 0.0 = pausado) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Time Cycle", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float TimeProgressionMultiplier = 1.0f;

	/** Indica si el tiempo progresa de forma automatica cada tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Time Cycle")
	bool bTimeProgressionEnabled = true;

	/** Ajusta manualmente la hora del dia */
	UFUNCTION(BlueprintCallable, Category = "Weather|Time Cycle")
	void SetTimeOfDay(float NewHour);

	/** Obtiene la hora actual en formato decimal */
	UFUNCTION(BlueprintPure, Category = "Weather|Time Cycle")
	float GetCurrentTimeOfDayHours() const { return CurrentHour; }

	/** Obtiene la hora, minutos y segundos enteros del reloj */
	UFUNCTION(BlueprintPure, Category = "Weather|Time Cycle")
	void GetCurrentTimeHMS(int32& OutHours, int32& OutMinutes, int32& OutSeconds) const;

	/** Devuelve la hora formateada como texto (ej. "08:30") */
	UFUNCTION(BlueprintPure, Category = "Weather|Time Cycle")
	FString GetFormattedTimeString() const;

	/** Activa o desactiva la progresion temporal */
	UFUNCTION(BlueprintCallable, Category = "Weather|Time Cycle")
	void SetTimeProgressionEnabled(bool bEnabled) { bTimeProgressionEnabled = bEnabled; }

	/** Establece el multiplicador de velocidad del tiempo */
	UFUNCTION(BlueprintCallable, Category = "Weather|Time Cycle")
	void SetTimeProgressionSpeed(float Multiplier) { TimeProgressionMultiplier = FMath::Max(0.0f, Multiplier); }

	/** Establece la duracion de un ciclo completo de 24h en segundos de tiempo real */
	UFUNCTION(BlueprintCallable, Category = "Weather|Time Cycle")
	void SetDayDurationSeconds(float DurationSeconds) { DayDurationRealSeconds = FMath::Max(30.0f, DurationSeconds); }

	/* =========================================================================
	 *  SECCION 2: FRANJAS DE TRAFICO Y HORAS PUNTA ESPANOLAS
	 * ========================================================================= */

	/** Devuelve el periodo actual del trafico (Valle, PuntaManana, ValleMediodia, PuntaTarde, NocheLogistica) */
	UFUNCTION(BlueprintPure, Category = "Weather|Traffic Schedule")
	ERushHourPeriod GetCurrentRushHourPeriod() const { return CurrentPeriod; }

	/** Comprueba si estamos en cualquier hora punta (manana 07:30-09:30 o tarde 18:00-20:30) */
	UFUNCTION(BlueprintPure, Category = "Weather|Traffic Schedule")
	bool IsRushHour() const;

	/** Comprueba especificamente la hora punta matinal de acceso laboral (07:30 - 09:30) */
	UFUNCTION(BlueprintPure, Category = "Weather|Traffic Schedule")
	bool IsMorningRushHour() const;

	/** Comprueba la hora punta de retorno vespertino y comercial (18:00 - 20:30) */
	UFUNCTION(BlueprintPure, Category = "Weather|Traffic Schedule")
	bool IsEveningRushHour() const;

	/** Comprueba si estamos en la franja nocturna de logistica pesada y camiones de gran tonelaje (22:00 - 06:00) */
	UFUNCTION(BlueprintPure, Category = "Weather|Traffic Schedule")
	bool IsNightTruckPeriod() const;

	/** Multiplicador demografico para la generacion de turismos y motos en base a la hora */
	UFUNCTION(BlueprintPure, Category = "Weather|Traffic Schedule")
	float GetPassengerCarSpawnMultiplier() const;

	/** Multiplicador demografico para la generacion de camiones pesados y trailers en base a la hora */
	UFUNCTION(BlueprintPure, Category = "Weather|Traffic Schedule")
	float GetHeavyTruckSpawnMultiplier() const;

	/** Devuelve un paquete con todos los datos horarios y factores demograficos */
	UFUNCTION(BlueprintPure, Category = "Weather|Traffic Schedule")
	FTimeOfDayInfo GetTimeOfDayInfo() const;

	/* =========================================================================
	 *  SECCION 3: ILUMINACION CENITAL SOLAR Y CIELO
	 * ========================================================================= */

	/** Actor de luz direccional (Sol/Luna) controlado por el subsistema */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Lighting")
	TWeakObjectPtr<ADirectionalLight> SunDirectionalLight;

	/** Si es true, el subsistema busca automaticamente el primer DirectionalLight del mapa */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Lighting")
	bool bAutoFindSunLight = true;

	/** Intensidad luminica solar maxima en el cenit diurno (Lux o escala UE) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Lighting")
	float MaxSunIntensity = 75000.0f;

	/** Intensidad minima de luz lunar en plena noche */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Lighting")
	float NightMoonIntensity = 0.25f;

	/** Registra manualmente la luz solar a controlar */
	UFUNCTION(BlueprintCallable, Category = "Weather|Lighting")
	void RegisterSunLight(ADirectionalLight* InSunLight);

	/** Busca activamente en el mundo la luz solar direccional */
	UFUNCTION(BlueprintCallable, Category = "Weather|Lighting")
	void AutoDetectSunLight();

	/** Calcula la rotacion fisica del sol segun la hora para la latitud de Espana (~40°N) */
	UFUNCTION(BlueprintPure, Category = "Weather|Lighting")
	FRotator CalculateSunRotation(float Hour24) const;

	/** Calcula la intensidad luminosa correspondiente a la hora */
	UFUNCTION(BlueprintPure, Category = "Weather|Lighting")
	float CalculateSunIntensity(float Hour24) const;

	/** Calcula el color/temperatura de la luz solar (calido en amanecer/atardecer, neutro a mediodia, azulado de noche) */
	UFUNCTION(BlueprintPure, Category = "Weather|Lighting")
	FLinearColor CalculateSunColor(float Hour24) const;

	/** Indica si el sol se encuentra por encima de la linea del horizonte */
	UFUNCTION(BlueprintPure, Category = "Weather|Lighting")
	bool IsSunAboveHorizon() const;

	/* =========================================================================
	 *  SECCION 4: CLIMATOLOGIA Y MODIFICADORES DE AGARRE DEL FIRME
	 * ========================================================================= */

	/** Condicion climatologica actual */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather|Conditions")
	EWeatherCondition CurrentWeather = EWeatherCondition::Soleado;

	/** Condicion objetivo hacia la que se transiciona */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather|Conditions")
	EWeatherCondition TargetWeather = EWeatherCondition::Soleado;

	/** Estado actual de parametros de agarre y pelicula de calzada */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather|Grip")
	FRoadSurfaceGripFactors CurrentGripFactors;

	/** Cambia el clima activo con transicion suave de friccion y visibilidad */
	UFUNCTION(BlueprintCallable, Category = "Weather|Conditions")
	void SetWeatherCondition(EWeatherCondition NewWeather, float TransitionTimeSeconds = 5.0f);

	/** Obtiene el clima actual */
	UFUNCTION(BlueprintPure, Category = "Weather|Conditions")
	EWeatherCondition GetCurrentWeatherCondition() const { return CurrentWeather; }

	/** Obtiene los factores de agarre actuales */
	UFUNCTION(BlueprintPure, Category = "Weather|Grip")
	FRoadSurfaceGripFactors GetCurrentGripFactors() const { return CurrentGripFactors; }

	/** Coeficiente de friccion actual (mu) del pavimento */
	UFUNCTION(BlueprintPure, Category = "Weather|Grip")
	float GetCurrentFrictionCoefficient() const { return CurrentGripFactors.FrictionCoefficient; }

	/** Multiplicador de distancia de frenado */
	UFUNCTION(BlueprintPure, Category = "Weather|Grip")
	float GetBrakingDistanceMultiplier() const { return CurrentGripFactors.BrakingDistanceMultiplier; }

	/** Multiplicador de riesgo de aquaplaning */
	UFUNCTION(BlueprintPure, Category = "Weather|Grip")
	float GetAquaplaningRiskMultiplier() const { return CurrentGripFactors.AquaplaningRiskMultiplier; }

	/** Espesor de la pelicula de agua en mm */
	UFUNCTION(BlueprintPure, Category = "Weather|Grip")
	float GetWaterFilmThicknessMm() const { return CurrentGripFactors.SurfaceWaterFilmThicknessMm; }

	/** Factor de riesgo de accidente segun clima (K_clima) */
	UFUNCTION(BlueprintPure, Category = "Weather|Grip")
	float GetWeatherAccidentRiskMultiplier() const { return CurrentGripFactors.WeatherAccidentRiskMultiplier; }

	/** Velocidad maxima aconsejada para paneles PMV */
	UFUNCTION(BlueprintPure, Category = "Weather|Grip")
	float GetRecommendedSpeedLimitKmh() const { return CurrentGripFactors.RecommendedMaxSpeedKmh; }

	/** Visibilidad atmosferica en metros */
	UFUNCTION(BlueprintPure, Category = "Weather|Grip")
	float GetAtmosphericVisibilityMeters() const { return CurrentGripFactors.VisibilityMeters; }

	/* =========================================================================
	 *  SECCION 5: ALIMENTACION AL MOTOR DE ACCIDENTES Y FISICA DE AQUAPLANING
	 * ========================================================================= */

	/**
	 * Calcula el coeficiente de friccion efectivo (mu_efectivo) para un vehiculo en movimiento,
	 * teniendo en cuenta la velocidad, la suspension hidrodinamica de las ruedas (aquaplaning)
	 * y el desgaste superficial del asfalto (0.0 = asfalto drenante nuevo, 1.0 = roderas y baches).
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|Accident Simulation")
	float CalculateEffectiveFriction(float VehicleSpeedKmh, float AsphaltWear01 = 0.0f) const;

	/**
	 * Calcula el indice de riesgo de aquaplaning inmediato (0.0 = sin riesgo, >1.0 = desprendimiento hidrodinamico).
	 * Aplica la fisica de Horne modificada con el espesor de la lamina de agua de la tormenta DANA.
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|Accident Simulation")
	float CalculateAquaplaningRisk(float VehicleSpeedKmh, float AsphaltWear01 = 0.0f) const;

	/**
	 * Implementacion completa de la formula oficial de siniestralidad DGT del GDD (Seccion 2.1):
	 * P_accidente = P_base * K_velocidad * K_clima * K_geometria * K_asfalto + FactorAquaplaning
	 * 
	 * @param VehicleSpeedKmh Velocidad actual del vehiculo
	 * @param SpeedLimitKmh Limite reglamentario del tramo
	 * @param CurveRadiusMeters Radio de curvatura del spline (0 o grande = recta)
	 * @param AsphaltWear01 Nivel de degradacion del firme (0.0 nuevo a 1.0 degradado)
	 * @return Multiplicador de probabilidad de accidente para el evaluador estocastico
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|Accident Simulation")
	float CalculateAccidentProbabilityMultiplier(float VehicleSpeedKmh, float SpeedLimitKmh, float CurveRadiusMeters, float AsphaltWear01 = 0.0f) const;

	/**
	 * Evaluacion estocastica rapida de perdida de control por aquaplaning en charco o curva mojada.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|Accident Simulation")
	bool EvaluateAquaplaningLossOfControl(float VehicleSpeedKmh, float AsphaltWear01 = 0.0f) const;

	/**
	 * Calcula la distancia teorica de parada de emergencia en metros para un vehiculo dado su velocidad y peso.
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|Accident Simulation")
	float CalculateSafeBrakingDistanceMeters(float SpeedKmh, float VehicleMassKg = 1400.0f) const;

	/* =========================================================================
	 *  SECCION 6: DELEGADOS Y EVENTOS BLUEPRINT
	 * ========================================================================= */

	/** Notificacion cuando cambia la condicion meteorologica o finaliza su transicion */
	UPROPERTY(BlueprintAssignable, Category = "Weather|Events")
	FOnWeatherConditionChanged OnWeatherChanged;

	/** Notificacion con cada cambio de minuto en el reloj del mundo */
	UPROPERTY(BlueprintAssignable, Category = "Weather|Events")
	FOnTimeOfDayUpdated OnTimeOfDayChanged;

	/** Notificacion al entrar o salir de una hora punta o franja logistica */
	UPROPERTY(BlueprintAssignable, Category = "Weather|Events")
	FOnRushHourPeriodChanged OnRushHourPeriodChanged;

	/** Alerta DGT cuando la velocidad de los vehiculos entra en zona critica de aquaplaning */
	UPROPERTY(BlueprintAssignable, Category = "Weather|Events")
	FOnAquaplaningAlert OnAquaplaningAlert;

private:
	// Periodo horario anterior para detectar transiciones
	ERushHourPeriod CurrentPeriod = ERushHourPeriod::PuntaManana;

	// Minuto entero anterior para evitar emitir OnTimeOfDayChanged en cada tick
	int32 LastNotifiedMinute = -1;

	// Variables para transicion suave de climatologia
	bool bIsWeatherTransitioning = false;
	float WeatherTransitionDuration = 5.0f;
	float WeatherTransitionElapsedTime = 0.0f;
	FRoadSurfaceGripFactors SourceGripFactors;
	FRoadSurfaceGripFactors TargetGripFactors;

	// Metodos internos
	void UpdateTimeProgression(float DeltaTime);
	void UpdateRushHourState();
	void UpdateSunLighting();
	void UpdateWeatherTransition(float DeltaTime);
	ERushHourPeriod DetermineRushHourPeriod(float Hour) const;
};
