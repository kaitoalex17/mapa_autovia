#include "Environment/WeatherSubsystem.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "AutopistasEspana.h"

void UWeatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Inicializar parametros por defecto en clima Soleado
	CurrentWeather = EWeatherCondition::Soleado;
	TargetWeather = EWeatherCondition::Soleado;
	CurrentGripFactors = FRoadSurfaceGripFactors::GetDefaultGripForWeather(EWeatherCondition::Soleado);
	SourceGripFactors = CurrentGripFactors;
	TargetGripFactors = CurrentGripFactors;

	// Determinar periodo inicial segun la hora de arranque (08:00 = Punta de la manana)
	CurrentPeriod = DetermineRushHourPeriod(CurrentHour);
	LastNotifiedMinute = FMath::FloorToInt(FMath::Fmod(CurrentHour * 60.0f, 60.0f));

	// Intentar enlazar con la luz solar presente en la escena
	if (bAutoFindSunLight)
	{
		AutoDetectSunLight();
	}

	UE_LOG(LogAutopistas, Log, TEXT("UWeatherSubsystem inicializado con exito. Hora inicial: %.2fh, Clima: Soleado, Periodo: %d"),
		CurrentHour, static_cast<int32>(CurrentPeriod));
}

void UWeatherSubsystem::Deinitialize()
{
	SunDirectionalLight.Reset();
	Super::Deinitialize();
}

void UWeatherSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Progresion del reloj de 24 horas
	if (bTimeProgressionEnabled)
	{
		UpdateTimeProgression(DeltaTime);
	}

	// 2. Evaluacion de franjas horarias y horas punta
	UpdateRushHourState();

	// 3. Transicion suave de climatologia y firme
	if (bIsWeatherTransitioning)
	{
		UpdateWeatherTransition(DeltaTime);
	}

	// 4. Actualizacion fisica de la rotacion y luz cenital del sol
	UpdateSunLighting();
}

/* =========================================================================
 *  SECCION 1: CICLO DE 24 HORAS
 * ========================================================================= */

void UWeatherSubsystem::SetTimeOfDay(float NewHour)
{
	CurrentHour = FMath::Fmod(FMath::Max(0.0f, NewHour), 24.0f);
	UpdateRushHourState();
	UpdateSunLighting();

	int32 H, M, S;
	GetCurrentTimeHMS(H, M, S);
	LastNotifiedMinute = M;
	OnTimeOfDayChanged.Broadcast(CurrentHour, H, M);
}

void UWeatherSubsystem::GetCurrentTimeHMS(int32& OutHours, int32& OutMinutes, int32& OutSeconds) const
{
	OutHours = FMath::FloorToInt(CurrentHour) % 24;
	const float MinutesDecimal = (CurrentHour - FMath::FloorToFloat(CurrentHour)) * 60.0f;
	OutMinutes = FMath::FloorToInt(MinutesDecimal) % 60;
	OutSeconds = FMath::FloorToInt((MinutesDecimal - FMath::FloorToFloat(MinutesDecimal)) * 60.0f) % 60;
}

FString UWeatherSubsystem::GetFormattedTimeString() const
{
	int32 H, M, S;
	GetCurrentTimeHMS(H, M, S);
	return FString::Printf(TEXT("%02d:%02d"), H, M);
}

void UWeatherSubsystem::UpdateTimeProgression(float DeltaTime)
{
	// Horas de juego que avanzan por cada segundo de tiempo real
	const float HoursPerSecond = 24.0f / FMath::Max(1.0f, DayDurationRealSeconds);
	CurrentHour += HoursPerSecond * TimeProgressionMultiplier * DeltaTime;

	if (CurrentHour >= 24.0f)
	{
		CurrentHour = FMath::Fmod(CurrentHour, 24.0f);
	}

	// Notificar a la interfaz y oyentes cada vez que cambia el minuto
	int32 H, M, S;
	GetCurrentTimeHMS(H, M, S);
	if (M != LastNotifiedMinute)
	{
		LastNotifiedMinute = M;
		OnTimeOfDayChanged.Broadcast(CurrentHour, H, M);
	}
}

/* =========================================================================
 *  SECCION 2: FRANJAS DE TRAFICO Y HORAS PUNTA ESPANOLAS
 * ========================================================================= */

ERushHourPeriod UWeatherSubsystem::DetermineRushHourPeriod(float Hour) const
{
	// 07:30 - 09:30: Hora punta matinal (acceso laboral a ciudades e industrias)
	if (Hour >= 7.5f && Hour < 9.5f)
	{
		return ERushHourPeriod::PuntaManana;
	}
	// 14:00 - 15:30: Salida escolar y hora de almuerzo
	else if (Hour >= 14.0f && Hour < 15.5f)
	{
		return ERushHourPeriod::ValleMediodia;
	}
	// 18:00 - 20:30: Hora punta de tarde (retorno a dormitorios y centros comerciales)
	else if (Hour >= 18.0f && Hour < 20.5f)
	{
		return ERushHourPeriod::PuntaTarde;
	}
	// 22:00 - 06:00: Noche logistica pesada (camiones de mercancías nocturnas)
	else if (Hour >= 22.0f || Hour < 6.0f)
	{
		return ERushHourPeriod::NocheLogistica;
	}

	return ERushHourPeriod::Valle;
}

void UWeatherSubsystem::UpdateRushHourState()
{
	const ERushHourPeriod NewPeriod = DetermineRushHourPeriod(CurrentHour);
	if (NewPeriod != CurrentPeriod)
	{
		const ERushHourPeriod Previous = CurrentPeriod;
		CurrentPeriod = NewPeriod;
		OnRushHourPeriodChanged.Broadcast(CurrentPeriod, Previous);

		UE_LOG(LogAutopistas, Log, TEXT("Transicion de periodo horario a: %d (Hora: %.2f)"),
			static_cast<int32>(CurrentPeriod), CurrentHour);
	}
}

bool UWeatherSubsystem::IsRushHour() const
{
	return IsMorningRushHour() || IsEveningRushHour();
}

bool UWeatherSubsystem::IsMorningRushHour() const
{
	return CurrentHour >= 7.5f && CurrentHour < 9.5f;
}

bool UWeatherSubsystem::IsEveningRushHour() const
{
	return CurrentHour >= 18.0f && CurrentHour < 20.5f;
}

bool UWeatherSubsystem::IsNightTruckPeriod() const
{
	return CurrentHour >= 22.0f || CurrentHour < 6.0f;
}

float UWeatherSubsystem::GetPassengerCarSpawnMultiplier() const
{
	switch (CurrentPeriod)
	{
	case ERushHourPeriod::PuntaManana:
		// Pico masivo de vehiculos ligeros (trabajadores)
		return 2.0f;

	case ERushHourPeriod::ValleMediodia:
		// Repunte por comidas y colegios
		return 1.35f;

	case ERushHourPeriod::PuntaTarde:
		// Retorno residencial saturado
		return 1.85f;

	case ERushHourPeriod::NocheLogistica:
		// Desplome del 80% en turismos segun GDD
		return 0.20f;

	case ERushHourPeriod::Valle:
	default:
		return 1.0f;
	}
}

float UWeatherSubsystem::GetHeavyTruckSpawnMultiplier() const
{
	switch (CurrentPeriod)
	{
	case ERushHourPeriod::NocheLogistica:
		// Explosion de trafico de mercancias nocturnas de largo recorrido (+250% a +350%)
		return 3.2f;

	case ERushHourPeriod::PuntaManana:
		// Camiones pesados evitan horas punta matinales o tienen restricciones de circulacion
		return 0.4f;

	case ERushHourPeriod::PuntaTarde:
		return 0.6f;

	case ERushHourPeriod::ValleMediodia:
		return 0.9f;

	case ERushHourPeriod::Valle:
	default:
		return 1.0f;
	}
}

FTimeOfDayInfo UWeatherSubsystem::GetTimeOfDayInfo() const
{
	FTimeOfDayInfo Info;
	Info.CurrentHour24 = CurrentHour;
	GetCurrentTimeHMS(Info.Hours, Info.Minutes, Info.Seconds);
	Info.RushHourPeriod = CurrentPeriod;
	Info.bIsRushHour = IsRushHour();
	Info.bIsNightTruckPeriod = IsNightTruckPeriod();
	Info.PassengerCarSpawnMultiplier = GetPassengerCarSpawnMultiplier();
	Info.HeavyTruckSpawnMultiplier = GetHeavyTruckSpawnMultiplier();
	Info.FormattedTime = GetFormattedTimeString();
	return Info;
}

/* =========================================================================
 *  SECCION 3: ILUMINACION CENITAL SOLAR Y CIELO
 * ========================================================================= */

void UWeatherSubsystem::RegisterSunLight(ADirectionalLight* InSunLight)
{
	SunDirectionalLight = InSunLight;
	UpdateSunLighting();
}

void UWeatherSubsystem::AutoDetectSunLight()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		SunDirectionalLight = *It;
		UE_LOG(LogAutopistas, Log, TEXT("UWeatherSubsystem: Luz solar detectada automaticamente: %s"), *It->GetName());
		break;
	}

	UpdateSunLighting();
}

FRotator UWeatherSubsystem::CalculateSunRotation(float Hour24) const
{
	// En Espana (latitud media ~40°N), el mediodia solar ocurre aproximadamente a las 13:30 CEST.
	// Angulo horario H respecto al mediodia solar: cada hora representa 15 grados.
	const float SolarNoonHour = 13.5f;
	const float HourAngleDeg = (Hour24 - SolarNoonHour) * 15.0f;
	const float HourAngleRad = FMath::DegreesToRadians(HourAngleDeg);

	// Declinacion solar representativa (verano/primavera ~16°) y latitud de la peninsula (~40°)
	const float LatRad = FMath::DegreesToRadians(40.0f);
	const float DecRad = FMath::DegreesToRadians(16.0f);

	// Formula astronomica de elevacion solar: sin(alpha) = sin(dec)*sin(lat) + cos(dec)*cos(lat)*cos(H)
	const float SinElevation = (FMath::Sin(DecRad) * FMath::Sin(LatRad)) +
	                           (FMath::Cos(DecRad) * FMath::Cos(LatRad) * FMath::Cos(HourAngleRad));
	const float ElevationRad = FMath::Asin(FMath::Clamp(SinElevation, -1.0f, 1.0f));
	const float ElevationDeg = FMath::RadiansToDegrees(ElevationRad);

	// Formula astronomica de acimut solar
	// cos(azimuth) = (sin(elevation)*sin(lat) - sin(dec)) / (cos(elevation)*cos(lat))
	const float CosElev = FMath::Max(0.001f, FMath::Cos(ElevationRad));
	const float CosAzimuth = (SinElevation * FMath::Sin(LatRad) - FMath::Sin(DecRad)) / (CosElev * FMath::Cos(LatRad));
	float AzimuthDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAzimuth, -1.0f, 1.0f)));

	// Si es antes del mediodia, el acimut esta al este (180 - Azimuth); despues al oeste (180 + Azimuth)
	if (HourAngleDeg < 0.0f)
	{
		AzimuthDeg = 180.0f - AzimuthDeg; // Este
	}
	else
	{
		AzimuthDeg = 180.0f + AzimuthDeg; // Oeste
	}

	// En Unreal Engine, la luz direccional emite a lo largo del vector Forward (+X).
	// Para iluminar desde el sol hacia el suelo:
	// Pitch: angulo negativo hacia abajo (-Elevation)
	// Yaw: direccion opuesta al acimut solar (Azimuth + 180)
	const float SunPitch = -ElevationDeg;
	const float SunYaw = FMath::Fmod(AzimuthDeg + 180.0f, 360.0f);

	return FRotator(SunPitch, SunYaw, 0.0f);
}

float UWeatherSubsystem::CalculateSunIntensity(float Hour24) const
{
	const FRotator Rot = CalculateSunRotation(Hour24);
	const float Elevation = -Rot.Pitch;

	if (Elevation <= 0.0f)
	{
		// De noche se mantiene una sutil luz lunar
		return NightMoonIntensity;
	}

	// Curva suave de amanecer al cenit diurno
	const float ElevationAlpha = FMath::Clamp(Elevation / 65.0f, 0.0f, 1.0f);
	const float DaylightIntensity = FMath::Lerp(500.0f, MaxSunIntensity, FMath::Pow(ElevationAlpha, 0.75f));

	// La lluvia DANA y la niebla densa atenua fuertemente la radiacion solar directa
	float WeatherOcclusion = 1.0f;
	switch (CurrentWeather)
	{
	case EWeatherCondition::LluviaDANA:
		WeatherOcclusion = 0.25f; // Cielo oscuro plomizo
		break;
	case EWeatherCondition::NievePuertos:
		WeatherOcclusion = 0.45f; // Neblina de nieve
		break;
	case EWeatherCondition::NieblaDensa:
		WeatherOcclusion = 0.20f; // Bloqueo de luz directa
		break;
	case EWeatherCondition::Soleado:
	default:
		WeatherOcclusion = 1.0f;
		break;
	}

	return DaylightIntensity * WeatherOcclusion;
}

FLinearColor UWeatherSubsystem::CalculateSunColor(float Hour24) const
{
	const FRotator Rot = CalculateSunRotation(Hour24);
	const float Elevation = -Rot.Pitch;

	if (Elevation <= 0.0f)
	{
		// Tonalidad nocturna: azul plata frio (luz de luna)
		return FLinearColor(0.25f, 0.35f, 0.65f, 1.0f);
	}

	// Amanecer y atardecer dorados (Golden Hour en la meseta espanola)
	if (Elevation < 12.0f)
	{
		const float DawnDuskAlpha = FMath::Clamp(Elevation / 12.0f, 0.0f, 1.0f);
		const FLinearColor GoldenColor(1.0f, 0.55f, 0.25f, 1.0f);
		const FLinearColor MorningWarm(1.0f, 0.85f, 0.65f, 1.0f);
		return FMath::Lerp(GoldenColor, MorningWarm, DawnDuskAlpha);
	}

	// Luz cenital blanca-calida natural de mediodia
	return FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
}

bool UWeatherSubsystem::IsSunAboveHorizon() const
{
	const FRotator Rot = CalculateSunRotation(CurrentHour);
	return (-Rot.Pitch) > 0.0f;
}

void UWeatherSubsystem::UpdateSunLighting()
{
	if (!SunDirectionalLight.IsValid() && bAutoFindSunLight)
	{
		AutoDetectSunLight();
	}

	if (!SunDirectionalLight.IsValid())
	{
		return;
	}

	// 1. Rota la luz solar cenital
	const FRotator NewSunRotation = CalculateSunRotation(CurrentHour);
	SunDirectionalLight->SetActorRotation(NewSunRotation);

	// 2. Modula la intensidad y el color
	UDirectionalLightComponent* LightComp = SunDirectionalLight->GetComponentByClass<UDirectionalLightComponent>();
	if (LightComp)
	{
		LightComp->SetIntensity(CalculateSunIntensity(CurrentHour));
		LightComp->SetLightColor(CalculateSunColor(CurrentHour));
	}
}

/* =========================================================================
 *  SECCION 4: CLIMATOLOGIA Y AGARRE DEL FIRME
 * ========================================================================= */

void UWeatherSubsystem::SetWeatherCondition(EWeatherCondition NewWeather, float TransitionTimeSeconds)
{
	if (NewWeather == CurrentWeather && !bIsWeatherTransitioning)
	{
		return;
	}

	TargetWeather = NewWeather;
	SourceGripFactors = CurrentGripFactors;
	TargetGripFactors = FRoadSurfaceGripFactors::GetDefaultGripForWeather(NewWeather);

	WeatherTransitionDuration = FMath::Max(0.5f, TransitionTimeSeconds);
	WeatherTransitionElapsedTime = 0.0f;
	bIsWeatherTransitioning = true;

	UE_LOG(LogAutopistas, Log, TEXT("Iniciando transicion meteorologica hacia: %d en %.1f segundos"),
		static_cast<int32>(NewWeather), WeatherTransitionDuration);
}

void UWeatherSubsystem::UpdateWeatherTransition(float DeltaTime)
{
	WeatherTransitionElapsedTime += DeltaTime;
	const float Alpha = FMath::Clamp(WeatherTransitionElapsedTime / WeatherTransitionDuration, 0.0f, 1.0f);

	CurrentGripFactors = FRoadSurfaceGripFactors::Interpolate(SourceGripFactors, TargetGripFactors, Alpha);

	if (Alpha >= 1.0f)
	{
		bIsWeatherTransitioning = false;
		CurrentWeather = TargetWeather;
		CurrentGripFactors = TargetGripFactors;

		OnWeatherChanged.Broadcast(CurrentWeather, CurrentGripFactors);

		UE_LOG(LogAutopistas, Log, TEXT("Transicion meteorologica completada a: %d. Friccion firme: %.2f, Visibilidad: %.0fm"),
			static_cast<int32>(CurrentWeather), CurrentGripFactors.FrictionCoefficient, CurrentGripFactors.VisibilityMeters);
	}
}

/* =========================================================================
 *  SECCION 5: MOTOR DE ACCIDENTES Y MODIFICADORES DE FRICCION (AQUAPLANING)
 * ========================================================================= */

float UWeatherSubsystem::CalculateAquaplaningRisk(float VehicleSpeedKmh, float AsphaltWear01) const
{
	// Si la calzada esta seca o la pelicula de agua es infima (< 1.5mm), el riesgo es nulo
	if (CurrentGripFactors.SurfaceWaterFilmThicknessMm < 1.5f || CurrentGripFactors.AquaplaningRiskMultiplier <= 0.0f)
	{
		return 0.0f;
	}

	// Velocidad critica umbral de flotabilidad del neumatico
	// El asfalto desgastado reduce la capacidad de evacuacion de las roderas en un 20%
	const float EffectiveThresholdKmh = CurrentGripFactors.AquaplaningSpeedThresholdKmh * (1.0f - (AsphaltWear01 * 0.20f));

	if (VehicleSpeedKmh <= EffectiveThresholdKmh)
	{
		return 0.0f;
	}

	// Exceso de velocidad por encima de la velocidad de desprendimiento
	const float SpeedDelta = VehicleSpeedKmh - EffectiveThresholdKmh;

	// Ley de presion dinamica de fluido: el empuje de agua crece cuadraticamente con la velocidad
	const float DynamicHydroPressureFactor = FMath::Square(SpeedDelta / 30.0f);

	// Espesor de la lamina de agua por encima del dibujo del neumatico
	const float WaterDepthFactor = FMath::Clamp((CurrentGripFactors.SurfaceWaterFilmThicknessMm - 1.5f) / 4.0f, 0.1f, 3.0f);

	// Penalizacion por desgaste del firme
	const float WearFactor = 1.0f + (AsphaltWear01 * 1.5f);

	const float TotalRisk = CurrentGripFactors.AquaplaningRiskMultiplier * DynamicHydroPressureFactor * WaterDepthFactor * WearFactor;

	// Si el riesgo es critico (> 2.0) emitir alerta de aquaplaning para paneles PMV y DGT
	if (TotalRisk > 2.0f)
	{
		const_cast<UWeatherSubsystem*>(this)->OnAquaplaningAlert.Broadcast(
			CurrentGripFactors.SurfaceWaterFilmThicknessMm,
			EffectiveThresholdKmh,
			TotalRisk);
	}

	return TotalRisk;
}

float UWeatherSubsystem::CalculateEffectiveFriction(float VehicleSpeedKmh, float AsphaltWear01) const
{
	// 1. Coeficiente base segun meteorologia
	float Mu = CurrentGripFactors.FrictionCoefficient;

	// 2. Penalizacion por envejecimiento y pulido de aridos en asfalto desgastado (hasta -35%)
	const float WearDegradation = FMath::Clamp(AsphaltWear01 * 0.35f, 0.0f, 0.35f);
	Mu *= (1.0f - WearDegradation);

	// 3. Perdida drastica de agarre por hidroplaneo
	const float AquaplaningRisk = CalculateAquaplaningRisk(VehicleSpeedKmh, AsphaltWear01);
	if (AquaplaningRisk > 0.0f)
	{
		// El vehiculo flota sobre la cuña de agua: el coeficiente cae asintoticamente a casi cero
		const float HydroplaningLiftOffRatio = 1.0f / (1.0f + (AquaplaningRisk * 0.85f));
		Mu *= HydroplaningLiftOffRatio;
	}

	return FMath::Clamp(Mu, 0.05f, 1.0f);
}

float UWeatherSubsystem::CalculateAccidentProbabilityMultiplier(float VehicleSpeedKmh, float SpeedLimitKmh, float CurveRadiusMeters, float AsphaltWear01) const
{
	// Formula oficial de siniestralidad DGT (GDD Seccion 2.1):
	// P_accidente = P_base * K_velocidad * K_clima * K_geometria * K_asfalto

	// 1. K_velocidad: El exceso respecto al limite reglamentario escala cubicamente
	const float SafeSpeedLimit = FMath::Max(30.0f, SpeedLimitKmh);
	const float SpeedRatio = VehicleSpeedKmh / SafeSpeedLimit;
	float K_velocidad = 1.0f;
	if (SpeedRatio > 1.0f)
	{
		K_velocidad = FMath::Pow(SpeedRatio, 3.0f);
	}
	else
	{
		// A velocidades prudentes por debajo del limite, el riesgo decae
		K_velocidad = FMath::Max(0.5f, FMath::Pow(SpeedRatio, 1.5f));
	}

	// 2. K_clima: Seco=1.0, Lluvia DANA=2.5, Nieve=6.0, Niebla=5.0
	const float K_clima = CurrentGripFactors.WeatherAccidentRiskMultiplier;

	// 3. K_geometria: Penalizacion de curvas cerradas sin clotoide segun Norma 3.1-IC
	float K_geometria = 1.0f;
	if (CurveRadiusMeters > 0.0f && CurveRadiusMeters < 500.0f)
	{
		// Para 120 km/h el radio minimo deseable es 700m; radios < 300m disparan la fuerza centrifuga
		K_geometria = FMath::Max(1.0f, 450.0f / CurveRadiusMeters);
	}

	// 4. K_asfalto: De 1.0 (nuevo) a 3.2 (con baches, roderas y baja macrotextura)
	const float K_asfalto = 1.0f + (FMath::Clamp(AsphaltWear01, 0.0f, 1.0f) * 2.2f);

	// 5. Adicion directa del riesgo de aquaplaning violento
	const float AquaplaningAdder = CalculateAquaplaningRisk(VehicleSpeedKmh, AsphaltWear01) * 3.5f;

	const float TotalMultiplier = (K_velocidad * K_clima * K_geometria * K_asfalto) + AquaplaningAdder;

	return FMath::Clamp(TotalMultiplier, 0.1f, 100.0f);
}

bool UWeatherSubsystem::EvaluateAquaplaningLossOfControl(float VehicleSpeedKmh, float AsphaltWear01) const
{
	const float Risk = CalculateAquaplaningRisk(VehicleSpeedKmh, AsphaltWear01);
	if (Risk <= 0.2f)
	{
		return false;
	}

	// Probabilidad estocastica de trompo / salida de via en charco
	const float LossOfControlChancePercent = FMath::Clamp(Risk * 12.0f, 0.0f, 85.0f);
	const float DiceRoll = FMath::FRandRange(0.0f, 100.0f);

	return DiceRoll < LossOfControlChancePercent;
}

float UWeatherSubsystem::CalculateSafeBrakingDistanceMeters(float SpeedKmh, float VehicleMassKg) const
{
	// d = v^2 / (2 * mu * g)
	const float SpeedMs = SpeedKmh * (1000.0f / 3600.0f);
	const float GravityMs2 = 9.81f;
	const float MuEffective = CalculateEffectiveFriction(SpeedKmh, 0.0f);

	const float BaseBrakingDist = (SpeedMs * SpeedMs) / (2.0f * MuEffective * GravityMs2);

	// Se aplica el multiplicador de distancia de parada segun climatologia (nieve/hielo, lluvia torrencial)
	return BaseBrakingDist * CurrentGripFactors.BrakingDistanceMultiplier;
}
