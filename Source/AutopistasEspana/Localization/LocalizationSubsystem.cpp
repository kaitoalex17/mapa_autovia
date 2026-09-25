#include "Localization/LocalizationSubsystem.h"
#include "AutopistasEspana.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Misc/ConfigCacheIni.h"

void ULocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadDictionaries();
	LoadLanguagePreference();

	UE_LOG(LogAutopistas, Log, TEXT("ULocalizationSubsystem inicializado con exito. Claves cargadas: ES=%d, EN=%d. Idioma activo: %s"),
		SpanishDictionary.Num(), EnglishDictionary.Num(), *GetLanguageDisplayName());
}

void ULocalizationSubsystem::Deinitialize()
{
	SpanishDictionary.Empty();
	EnglishDictionary.Empty();
	Super::Deinitialize();
}

void ULocalizationSubsystem::SetLanguage(ELanguageCode NewLanguage)
{
	if (CurrentLanguage != NewLanguage)
	{
		CurrentLanguage = NewLanguage;
		SaveLanguagePreference();
		OnLanguageChanged.Broadcast(NewLanguage);

		UE_LOG(LogAutopistas, Log, TEXT("Idioma de Autopistas de Espana cambiado dinamicamente a: %s (%s)"),
			*GetLanguageDisplayName(), *GetLanguageCodeString());
	}
}

bool ULocalizationSubsystem::SetLanguageByCode(const FString& InLanguageCode)
{
	FString Code = InLanguageCode.TrimStartAndEnd().ToUpper();

	if (Code == TEXT("ES") || Code == TEXT("SPANISH") || Code == TEXT("ESPANOL") || Code == TEXT("ESPAÑOL"))
	{
		SetLanguage(ELanguageCode::Spanish);
		return true;
	}
	else if (Code == TEXT("EN") || Code == TEXT("ENGLISH") || Code == TEXT("INGLES") || Code == TEXT("INGLÉS"))
	{
		SetLanguage(ELanguageCode::English);
		return true;
	}

	UE_LOG(LogAutopistas, Warning, TEXT("ULocalizationSubsystem: Codigo de idioma no reconocido '%s'"), *InLanguageCode);
	return false;
}

FString ULocalizationSubsystem::GetLanguageCodeString() const
{
	return (CurrentLanguage == ELanguageCode::Spanish) ? TEXT("ES") : TEXT("EN");
}

FString ULocalizationSubsystem::GetLanguageDisplayName() const
{
	return (CurrentLanguage == ELanguageCode::Spanish) ? TEXT("Español (ES)") : TEXT("English (EN)");
}

FText ULocalizationSubsystem::GetLocalizedString(const FString& Key) const
{
	const TMap<FString, FString>& TargetDict = (CurrentLanguage == ELanguageCode::Spanish) ? SpanishDictionary : EnglishDictionary;

	// 1. Busqueda en diccionario del idioma activo
	if (const FString* Found = TargetDict.Find(Key))
	{
		return FText::FromString(*Found);
	}

	// 2. Fallback al diccionario Espanol si falta en Ingles
	if (const FString* FallbackEs = SpanishDictionary.Find(Key))
	{
		return FText::FromString(*FallbackEs);
	}

	// 3. Fallback al diccionario Ingles si falta en Espanol
	if (const FString* FallbackEn = EnglishDictionary.Find(Key))
	{
		return FText::FromString(*FallbackEn);
	}

	// 4. Clave sin traduccion
	return FText::FromString(Key);
}

FText ULocalizationSubsystem::GetLocalizedStringWithFallback(const FString& Key, const FString& DefaultFallback) const
{
	const TMap<FString, FString>& TargetDict = (CurrentLanguage == ELanguageCode::Spanish) ? SpanishDictionary : EnglishDictionary;

	if (const FString* Found = TargetDict.Find(Key))
	{
		return FText::FromString(*Found);
	}
	if (const FString* FallbackEs = SpanishDictionary.Find(Key))
	{
		return FText::FromString(*FallbackEs);
	}
	if (const FString* FallbackEn = EnglishDictionary.Find(Key))
	{
		return FText::FromString(*FallbackEn);
	}

	return FText::FromString(DefaultFallback);
}

FText ULocalizationSubsystem::GetLocalizedTextForLanguage(const FString& Key, ELanguageCode TargetLanguage) const
{
	const TMap<FString, FString>& TargetDict = (TargetLanguage == ELanguageCode::Spanish) ? SpanishDictionary : EnglishDictionary;

	if (const FString* Found = TargetDict.Find(Key))
	{
		return FText::FromString(*Found);
	}

	const TMap<FString, FString>& FallbackDict = (TargetLanguage == ELanguageCode::Spanish) ? EnglishDictionary : SpanishDictionary;
	if (const FString* Fallback = FallbackDict.Find(Key))
	{
		return FText::FromString(*Fallback);
	}

	return FText::FromString(Key);
}

FText ULocalizationSubsystem::GetFormattedLocalizedString(const FString& Key, const TMap<FString, FString>& Arguments) const
{
	FString FormattedString = GetLocalizedString(Key).ToString();

	for (const auto& Pair : Arguments)
	{
		const FString Token = FString::Printf(TEXT("{%s}"), *Pair.Key);
		FormattedString = FormattedString.Replace(*Token, *Pair.Value);
	}

	return FText::FromString(FormattedString);
}

FText ULocalizationSubsystem::GetFormattedLocalizedString1P(const FString& Key, const FString& Param0) const
{
	FString FormattedString = GetLocalizedString(Key).ToString();
	FormattedString = FormattedString.Replace(TEXT("{0}"), *Param0);
	return FText::FromString(FormattedString);
}

FText ULocalizationSubsystem::GetFormattedLocalizedString2P(const FString& Key, const FString& Param0, const FString& Param1) const
{
	FString FormattedString = GetLocalizedString(Key).ToString();
	FormattedString = FormattedString.Replace(TEXT("{0}"), *Param0);
	FormattedString = FormattedString.Replace(TEXT("{1}"), *Param1);
	return FText::FromString(FormattedString);
}

bool ULocalizationSubsystem::HasTranslationKey(const FString& Key) const
{
	return SpanishDictionary.Contains(Key) || EnglishDictionary.Contains(Key);
}

int32 ULocalizationSubsystem::GetTranslationCount(ELanguageCode Language) const
{
	return (Language == ELanguageCode::Spanish) ? SpanishDictionary.Num() : EnglishDictionary.Num();
}

TArray<FString> ULocalizationSubsystem::GetAllTranslationKeys() const
{
	TSet<FString> KeySet;
	TArray<FString> Keys;
	SpanishDictionary.GetKeys(Keys);
	KeySet.Append(Keys);
	Keys.Empty();
	EnglishDictionary.GetKeys(Keys);
	KeySet.Append(Keys);
	return KeySet.Array();
}

void ULocalizationSubsystem::RegisterCustomString(const FString& Key, const FString& SpanishText, const FString& EnglishText)
{
	SpanishDictionary.Add(Key, SpanishText);
	EnglishDictionary.Add(Key, EnglishText);
}

ULocalizationSubsystem* ULocalizationSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<ULocalizationSubsystem>() : nullptr;
}

FText ULocalizationSubsystem::GetGameLocalizedString(const UObject* WorldContextObject, const FString& Key)
{
	if (ULocalizationSubsystem* Subsystem = Get(WorldContextObject))
	{
		return Subsystem->GetLocalizedString(Key);
	}
	return FText::FromString(Key);
}

void ULocalizationSubsystem::LoadLanguagePreference()
{
	if (!GConfig)
	{
		return;
	}

	FString SavedLanguageCode;
	if (GConfig->GetString(TEXT("Localization"), TEXT("Language"), SavedLanguageCode, GGameUserSettingsIni))
	{
		if (!SavedLanguageCode.IsEmpty())
		{
			SetLanguageByCode(SavedLanguageCode);
		}
	}
}

void ULocalizationSubsystem::SaveLanguagePreference()
{
	if (!GConfig)
	{
		return;
	}

	GConfig->SetString(TEXT("Localization"), TEXT("Language"), *GetLanguageCodeString(), GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void ULocalizationSubsystem::LoadDictionaries()
{
	SpanishDictionary.Empty();
	EnglishDictionary.Empty();

	// =========================================================================
	// 1. MENÚ PRINCIPAL, GUARDADO Y NAVEGACIÓN UI
	// =========================================================================
	SpanishDictionary.Add(TEXT("GAME_TITLE"), TEXT("Autopistas de España"));
	EnglishDictionary.Add(TEXT("GAME_TITLE"), TEXT("Highways of Spain"));

	SpanishDictionary.Add(TEXT("GAME_SUBTITLE"), TEXT("Simulador de Infraestructuras y Tráfico"));
	EnglishDictionary.Add(TEXT("GAME_SUBTITLE"), TEXT("Infrastructure & Traffic Simulator"));

	SpanishDictionary.Add(TEXT("MENU_PLAY"), TEXT("Jugar"));
	EnglishDictionary.Add(TEXT("MENU_PLAY"), TEXT("Play"));

	SpanishDictionary.Add(TEXT("MENU_NEW_GAME"), TEXT("Nueva Partida"));
	EnglishDictionary.Add(TEXT("MENU_NEW_GAME"), TEXT("New Game"));

	SpanishDictionary.Add(TEXT("MENU_CONTINUE"), TEXT("Continuar"));
	EnglishDictionary.Add(TEXT("MENU_CONTINUE"), TEXT("Continue"));

	SpanishDictionary.Add(TEXT("MENU_LOAD_GAME"), TEXT("Cargar Partida"));
	EnglishDictionary.Add(TEXT("MENU_LOAD_GAME"), TEXT("Load Game"));

	SpanishDictionary.Add(TEXT("MENU_SAVE_GAME"), TEXT("Guardar"));
	EnglishDictionary.Add(TEXT("MENU_SAVE_GAME"), TEXT("Save"));

	SpanishDictionary.Add(TEXT("MENU_SAVE_GAME_TITLE"), TEXT("Guardar Partida"));
	EnglishDictionary.Add(TEXT("MENU_SAVE_GAME_TITLE"), TEXT("Save Game"));

	SpanishDictionary.Add(TEXT("MENU_QUICK_SAVE"), TEXT("Guardado Rápido (F5)"));
	EnglishDictionary.Add(TEXT("MENU_QUICK_SAVE"), TEXT("Quick Save (F5)"));

	SpanishDictionary.Add(TEXT("MENU_QUICK_LOAD"), TEXT("Carga Rápida (F9)"));
	EnglishDictionary.Add(TEXT("MENU_QUICK_LOAD"), TEXT("Quick Load (F9)"));

	SpanishDictionary.Add(TEXT("MENU_AUTO_SAVE"), TEXT("Autoguardado"));
	EnglishDictionary.Add(TEXT("MENU_AUTO_SAVE"), TEXT("Autosave"));

	SpanishDictionary.Add(TEXT("MENU_SETTINGS"), TEXT("Opciones"));
	EnglishDictionary.Add(TEXT("MENU_SETTINGS"), TEXT("Settings"));

	SpanishDictionary.Add(TEXT("MENU_OPTIONS"), TEXT("Ajustes"));
	EnglishDictionary.Add(TEXT("MENU_OPTIONS"), TEXT("Options"));

	SpanishDictionary.Add(TEXT("MENU_QUIT"), TEXT("Salir"));
	EnglishDictionary.Add(TEXT("MENU_QUIT"), TEXT("Quit"));

	SpanishDictionary.Add(TEXT("MENU_EXIT"), TEXT("Salir al Escritorio"));
	EnglishDictionary.Add(TEXT("MENU_EXIT"), TEXT("Exit to Desktop"));

	SpanishDictionary.Add(TEXT("MENU_BACK_MAIN"), TEXT("Salir al Menú Principal"));
	EnglishDictionary.Add(TEXT("MENU_BACK_MAIN"), TEXT("Exit to Main Menu"));

	SpanishDictionary.Add(TEXT("MENU_RESUME"), TEXT("Reanudar"));
	EnglishDictionary.Add(TEXT("MENU_RESUME"), TEXT("Resume"));

	SpanishDictionary.Add(TEXT("MENU_PAUSE"), TEXT("Pausa"));
	EnglishDictionary.Add(TEXT("MENU_PAUSE"), TEXT("Pause"));

	SpanishDictionary.Add(TEXT("SAVE_SLOT_EMPTY"), TEXT("Ranura Vacía"));
	EnglishDictionary.Add(TEXT("SAVE_SLOT_EMPTY"), TEXT("Empty Slot"));

	SpanishDictionary.Add(TEXT("SAVE_SLOT_NAME"), TEXT("Ranura {0}"));
	EnglishDictionary.Add(TEXT("SAVE_SLOT_NAME"), TEXT("Slot {0}"));

	SpanishDictionary.Add(TEXT("SAVE_SUCCESS"), TEXT("Partida guardada correctamente en {0}"));
	EnglishDictionary.Add(TEXT("SAVE_SUCCESS"), TEXT("Game saved successfully to {0}"));

	SpanishDictionary.Add(TEXT("LOAD_SUCCESS"), TEXT("Partida cargada con éxito desde {0}"));
	EnglishDictionary.Add(TEXT("LOAD_SUCCESS"), TEXT("Game loaded successfully from {0}"));

	SpanishDictionary.Add(TEXT("SAVE_ERROR"), TEXT("Error al guardar la partida en disco"));
	EnglishDictionary.Add(TEXT("SAVE_ERROR"), TEXT("Error saving game to storage"));

	SpanishDictionary.Add(TEXT("LOAD_ERROR"), TEXT("Error al cargar la partida seleccionada"));
	EnglishDictionary.Add(TEXT("LOAD_ERROR"), TEXT("Error loading selected save file"));

	SpanishDictionary.Add(TEXT("CONFIRM_OVERWRITE"), TEXT("¿Desea sobrescribir la ranura de guardado seleccionada?"));
	EnglishDictionary.Add(TEXT("CONFIRM_OVERWRITE"), TEXT("Do you want to overwrite the selected save slot?"));

	SpanishDictionary.Add(TEXT("CONFIRM_QUIT"), TEXT("¿Seguro que desea salir? Se perderán los progresos no guardados."));
	EnglishDictionary.Add(TEXT("CONFIRM_QUIT"), TEXT("Are you sure you want to quit? Unsaved progress will be lost."));

	SpanishDictionary.Add(TEXT("CONFIRM_YES"), TEXT("Sí"));
	EnglishDictionary.Add(TEXT("CONFIRM_YES"), TEXT("Yes"));

	SpanishDictionary.Add(TEXT("CONFIRM_NO"), TEXT("No"));
	EnglishDictionary.Add(TEXT("CONFIRM_NO"), TEXT("No"));

	SpanishDictionary.Add(TEXT("CONFIRM_CANCEL"), TEXT("Cancelar"));
	EnglishDictionary.Add(TEXT("CONFIRM_CANCEL"), TEXT("Cancel"));

	// =========================================================================
	// 2. MODOS DE CONSTRUCCIÓN E INFRAESTRUCTURA VIAL
	// =========================================================================
	SpanishDictionary.Add(TEXT("TOOL_SELECT"), TEXT("Inspeccionar / Cursor"));
	EnglishDictionary.Add(TEXT("TOOL_SELECT"), TEXT("Inspect / Cursor"));

	SpanishDictionary.Add(TEXT("BUILD_SINGLE_LANE"), TEXT("Calzada única (90 km/h)"));
	EnglishDictionary.Add(TEXT("BUILD_SINGLE_LANE"), TEXT("1-Lane Road (90 km/h)"));

	SpanishDictionary.Add(TEXT("TOOL_ROAD_1L"), TEXT("Calzada única (90 km/h)"));
	EnglishDictionary.Add(TEXT("TOOL_ROAD_1L"), TEXT("1-Lane Road (90 km/h)"));

	SpanishDictionary.Add(TEXT("TOOL_ROAD_90"), TEXT("Carretera Convencional (90 km/h)"));
	EnglishDictionary.Add(TEXT("TOOL_ROAD_90"), TEXT("Conventional Road (90 km/h)"));

	SpanishDictionary.Add(TEXT("BUILD_HIGHWAY_2X2"), TEXT("Autovía 2x2 (120 km/h)"));
	EnglishDictionary.Add(TEXT("BUILD_HIGHWAY_2X2"), TEXT("2x2 Highway (120 km/h)"));

	SpanishDictionary.Add(TEXT("TOOL_HIGHWAY_2X2"), TEXT("Autovía 2x2 (120 km/h)"));
	EnglishDictionary.Add(TEXT("TOOL_HIGHWAY_2X2"), TEXT("2x2 Highway (120 km/h)"));

	SpanishDictionary.Add(TEXT("BUILD_HIGHWAY_3X3"), TEXT("Autopista 3x3 (120 km/h)"));
	EnglishDictionary.Add(TEXT("BUILD_HIGHWAY_3X3"), TEXT("3x3 Highway (120 km/h)"));

	SpanishDictionary.Add(TEXT("TOOL_HIGHWAY_3X3"), TEXT("Autopista 3x3 (120 km/h)"));
	EnglishDictionary.Add(TEXT("TOOL_HIGHWAY_3X3"), TEXT("3x3 Highway (120 km/h)"));

	SpanishDictionary.Add(TEXT("BUILD_ROUNDABOUT"), TEXT("Enlace / Glorieta"));
	EnglishDictionary.Add(TEXT("BUILD_ROUNDABOUT"), TEXT("Roundabout"));

	SpanishDictionary.Add(TEXT("TOOL_ROUNDABOUT"), TEXT("Enlace / Glorieta"));
	EnglishDictionary.Add(TEXT("TOOL_ROUNDABOUT"), TEXT("Roundabout"));

	SpanishDictionary.Add(TEXT("BUILD_BRIDGE"), TEXT("Puente"));
	EnglishDictionary.Add(TEXT("BUILD_BRIDGE"), TEXT("Bridge"));

	SpanishDictionary.Add(TEXT("TOOL_BRIDGE"), TEXT("Puente / Viaducto"));
	EnglishDictionary.Add(TEXT("TOOL_BRIDGE"), TEXT("Bridge / Viaduct"));

	SpanishDictionary.Add(TEXT("TOOL_VIADUCT"), TEXT("Viaducto / Puente"));
	EnglishDictionary.Add(TEXT("TOOL_VIADUCT"), TEXT("Viaduct / Bridge"));

	SpanishDictionary.Add(TEXT("BUILD_TUNNEL"), TEXT("Túnel"));
	EnglishDictionary.Add(TEXT("BUILD_TUNNEL"), TEXT("Tunnel"));

	SpanishDictionary.Add(TEXT("TOOL_TUNNEL"), TEXT("Túnel de Montaña"));
	EnglishDictionary.Add(TEXT("TOOL_TUNNEL"), TEXT("Mountain Tunnel"));

	SpanishDictionary.Add(TEXT("BUILD_TOLL"), TEXT("Peaje"));
	EnglishDictionary.Add(TEXT("BUILD_TOLL"), TEXT("Toll"));

	SpanishDictionary.Add(TEXT("TOOL_TOLL"), TEXT("Peaje (Vía-T / Manual)"));
	EnglishDictionary.Add(TEXT("TOOL_TOLL"), TEXT("Toll Plaza (Electronic / Manual)"));

	SpanishDictionary.Add(TEXT("BUILD_LEVEL_CROSSING"), TEXT("Paso a Nivel"));
	EnglishDictionary.Add(TEXT("BUILD_LEVEL_CROSSING"), TEXT("Level Crossing"));

	SpanishDictionary.Add(TEXT("TOOL_LEVEL_CROSSING"), TEXT("Paso a Nivel (ADIF PN Clase C)"));
	EnglishDictionary.Add(TEXT("TOOL_LEVEL_CROSSING"), TEXT("Level Crossing (ADIF Class C)"));

	SpanishDictionary.Add(TEXT("TOOL_RAILWAY_TRACK"), TEXT("Vía Férrea (Ancho Ibérico)"));
	EnglishDictionary.Add(TEXT("TOOL_RAILWAY_TRACK"), TEXT("Railway Track (Iberian Gauge)"));

	SpanishDictionary.Add(TEXT("TOOL_BUS_LINES"), TEXT("Líneas de Transporte"));
	EnglishDictionary.Add(TEXT("TOOL_BUS_LINES"), TEXT("Transit Lines"));

	SpanishDictionary.Add(TEXT("TOOL_DGT_CONTROLS"), TEXT("Operativo DGT / Conos"));
	EnglishDictionary.Add(TEXT("TOOL_DGT_CONTROLS"), TEXT("DGT Police Checkpoint"));

	SpanishDictionary.Add(TEXT("TOOL_PEGASUS"), TEXT("Patrulla Aérea Pegasus"));
	EnglishDictionary.Add(TEXT("TOOL_PEGASUS"), TEXT("Pegasus Helicopter Patrol"));

	SpanishDictionary.Add(TEXT("TOOL_DEMOLISH"), TEXT("Demolición / Excavadora"));
	EnglishDictionary.Add(TEXT("TOOL_DEMOLISH"), TEXT("Demolition / Bulldozer"));

	SpanishDictionary.Add(TEXT("BUILD_COST_PREVIEW"), TEXT("Coste estimado: {0} €"));
	EnglishDictionary.Add(TEXT("BUILD_COST_PREVIEW"), TEXT("Estimated cost: €{0}"));

	SpanishDictionary.Add(TEXT("BUILD_LENGTH_PREVIEW"), TEXT("Longitud: {0} m"));
	EnglishDictionary.Add(TEXT("BUILD_LENGTH_PREVIEW"), TEXT("Length: {0} m"));

	SpanishDictionary.Add(TEXT("BUILD_SLOPE_TOO_STEEP"), TEXT("Pendiente excesiva (> 8%): Requiere viaducto o túnel"));
	EnglishDictionary.Add(TEXT("BUILD_SLOPE_TOO_STEEP"), TEXT("Slope too steep (> 8%): Requires viaduct or tunnel"));

	SpanishDictionary.Add(TEXT("BUILD_BLOCKED"), TEXT("Trazado obstruido por terreno o edificación"));
	EnglishDictionary.Add(TEXT("BUILD_BLOCKED"), TEXT("Route obstructed by terrain or structure"));

	SpanishDictionary.Add(TEXT("BUILD_CONFIRM"), TEXT("Construir tramo"));
	EnglishDictionary.Add(TEXT("BUILD_CONFIRM"), TEXT("Construct segment"));

	SpanishDictionary.Add(TEXT("BUILD_CANCEL"), TEXT("Cancelar trazado"));
	EnglishDictionary.Add(TEXT("BUILD_CANCEL"), TEXT("Cancel route"));

	// =========================================================================
	// 3. MECÁNICAS CONMUTABLES (OPCIONES Y AJUSTES)
	// =========================================================================
	SpanishDictionary.Add(TEXT("SETTING_ROADWORKS_TITLE"), TEXT("Impacto de Obras"));
	EnglishDictionary.Add(TEXT("SETTING_ROADWORKS_TITLE"), TEXT("Roadworks Impact"));

	SpanishDictionary.Add(TEXT("SETTING_ROADWORKS_ENABLED"), TEXT("Impacto de Obras: Activado"));
	EnglishDictionary.Add(TEXT("SETTING_ROADWORKS_ENABLED"), TEXT("Roadworks Impact: Enabled"));

	SpanishDictionary.Add(TEXT("SETTING_ROADWORKS_DISABLED"), TEXT("Impacto de Obras: Desactivado"));
	EnglishDictionary.Add(TEXT("SETTING_ROADWORKS_DISABLED"), TEXT("Roadworks Impact: Disabled"));

	SpanishDictionary.Add(TEXT("SETTING_ROADWORKS_DESC"), TEXT("Al activarse, las obras requieren fases de maquinaria, cortan carriles con conos y reducen la velocidad del tráfico."));
	EnglishDictionary.Add(TEXT("SETTING_ROADWORKS_DESC"), TEXT("When enabled, construction requires machinery phases, closes lanes with cones, and reduces traffic speed."));

	SpanishDictionary.Add(TEXT("SETTING_ROAD_RAGE_TITLE"), TEXT("Furia y Psicología Vial"));
	EnglishDictionary.Add(TEXT("SETTING_ROAD_RAGE_TITLE"), TEXT("Road Rage & Driver Psychology"));

	SpanishDictionary.Add(TEXT("SETTING_ROAD_RAGE_ENABLED"), TEXT("Furia y Psicología Vial: Activada"));
	EnglishDictionary.Add(TEXT("SETTING_ROAD_RAGE_ENABLED"), TEXT("Road Rage & Driver Psychology: Enabled"));

	SpanishDictionary.Add(TEXT("SETTING_ROAD_RAGE_DISABLED"), TEXT("Furia y Psicología Vial: Desactivada"));
	EnglishDictionary.Add(TEXT("SETTING_ROAD_RAGE_DISABLED"), TEXT("Road Rage & Driver Psychology: Disabled"));

	SpanishDictionary.Add(TEXT("SETTING_ROAD_RAGE_DESC"), TEXT("Al activarse, las retenciones generan frustración en los conductores, bocinazos y maniobras temerarias."));
	EnglishDictionary.Add(TEXT("SETTING_ROAD_RAGE_DESC"), TEXT("When enabled, traffic delays cause driver frustration, horn honking, and reckless overtaking maneuvers."));

	SpanishDictionary.Add(TEXT("SETTING_WEATHER_TITLE"), TEXT("Temporal y Dinámica Climática"));
	EnglishDictionary.Add(TEXT("SETTING_WEATHER_TITLE"), TEXT("Severe Storms & Weather Dynamics"));

	SpanishDictionary.Add(TEXT("SETTING_WEATHER_ENABLED"), TEXT("Meteorología Dinámica: Activada"));
	EnglishDictionary.Add(TEXT("SETTING_WEATHER_ENABLED"), TEXT("Dynamic Weather: Enabled"));

	SpanishDictionary.Add(TEXT("SETTING_WEATHER_DISABLED"), TEXT("Meteorología Dinámica: Desactivada"));
	EnglishDictionary.Add(TEXT("SETTING_WEATHER_DISABLED"), TEXT("Dynamic Weather: Disabled"));

	SpanishDictionary.Add(TEXT("SETTING_LANGUAGE_TITLE"), TEXT("Idioma / Language"));
	EnglishDictionary.Add(TEXT("SETTING_LANGUAGE_TITLE"), TEXT("Language / Idioma"));

	SpanishDictionary.Add(TEXT("SETTING_GRAPHICS_QUALITY"), TEXT("Calidad Gráfica"));
	EnglishDictionary.Add(TEXT("SETTING_GRAPHICS_QUALITY"), TEXT("Graphics Quality"));

	// =========================================================================
	// 4. ALERTAS DGT Y ESTADO DE CALZADA
	// =========================================================================
	SpanishDictionary.Add(TEXT("ALERT_PEGASUS_PATROL"), TEXT("Patrulla Pegasus en servicio"));
	EnglishDictionary.Add(TEXT("ALERT_PEGASUS_PATROL"), TEXT("DGT Pegasus on patrol"));

	SpanishDictionary.Add(TEXT("ALERT_PEGASUS_PATROL_DESC"), TEXT("Helicóptero Pegasus de la DGT vigilando el corredor: cinemómetro radar activo contra excesos de velocidad."));
	EnglishDictionary.Add(TEXT("ALERT_PEGASUS_PATROL_DESC"), TEXT("DGT Pegasus helicopter patrolling highway: radar active tracking speeding and safety violations."));

	SpanishDictionary.Add(TEXT("ALERT_ALCOHOL_CHECKPOINT"), TEXT("Control preventivo de alcoholemia"));
	EnglishDictionary.Add(TEXT("ALERT_ALCOHOL_CHECKPOINT"), TEXT("Alcohol checkpoint"));

	SpanishDictionary.Add(TEXT("ALERT_ALCOHOL_CHECKPOINT_DESC"), TEXT("Guardia Civil de Tráfico desplegada en calzada con conos reflectantes y reducción a 1 carril."));
	EnglishDictionary.Add(TEXT("ALERT_ALCOHOL_CHECKPOINT_DESC"), TEXT("Traffic Police deployed on carriageway with reflective cones and taper to 1 slow lane."));

	SpanishDictionary.Add(TEXT("ALERT_WEIGHT_CHECKPOINT"), TEXT("Control de pesaje y tacógrafo"));
	EnglishDictionary.Add(TEXT("ALERT_WEIGHT_CHECKPOINT"), TEXT("Weigh station checkpoint"));

	SpanishDictionary.Add(TEXT("ALERT_TRAFFIC_JAM"), TEXT("Tráfico denso / Retención"));
	EnglishDictionary.Add(TEXT("ALERT_TRAFFIC_JAM"), TEXT("Traffic congestion"));

	SpanishDictionary.Add(TEXT("ALERT_ROAD_JAM"), TEXT("Tráfico denso / Retención: Vía colapsada, los conductores pierden los nervios."));
	EnglishDictionary.Add(TEXT("ALERT_ROAD_JAM"), TEXT("Traffic congestion: Severe gridlock, drivers are getting frustrated."));

	SpanishDictionary.Add(TEXT("ALERT_DANA_AQUAPLANING"), TEXT("Temporal DANA / Aquaplaning"));
	EnglishDictionary.Add(TEXT("ALERT_DANA_AQUAPLANING"), TEXT("Severe storm / Aquaplaning"));

	SpanishDictionary.Add(TEXT("ALERT_WEATHER_RAIN"), TEXT("Temporal DANA / Aquaplaning: Precipitación torrencial. Reduzca velocidad a 80 km/h."));
	EnglishDictionary.Add(TEXT("ALERT_WEATHER_RAIN"), TEXT("Severe storm / Aquaplaning: Torrential rain on highway. Reduce speed to 80 km/h."));

	SpanishDictionary.Add(TEXT("ALERT_WEATHER_SNOW"), TEXT("Temporal de nieve: Firme deslizante"));
	EnglishDictionary.Add(TEXT("ALERT_WEATHER_SNOW"), TEXT("Snowstorm: Slippery road surface"));

	SpanishDictionary.Add(TEXT("ALERT_WEATHER_FOG"), TEXT("Niebla densa: Visibilidad reducida"));
	EnglishDictionary.Add(TEXT("ALERT_WEATHER_FOG"), TEXT("Dense fog: Reduced visibility hazard"));

	SpanishDictionary.Add(TEXT("ALERT_ACCIDENT"), TEXT("¡ALERTA 112: Accidente en calzada! Grúa y Guardia Civil en camino."));
	EnglishDictionary.Add(TEXT("ALERT_ACCIDENT"), TEXT("112 EMERGENCY: Traffic collision! Tow truck and Highway Patrol dispatched."));

	SpanishDictionary.Add(TEXT("ALERT_ACCIDENT_EMERGENCY"), TEXT("¡ALERTA 112: Accidente grave en calzada!"));
	EnglishDictionary.Add(TEXT("ALERT_ACCIDENT_EMERGENCY"), TEXT("112 EMERGENCY: Severe highway crash!"));

	SpanishDictionary.Add(TEXT("ALERT_LEVEL_CROSSING_DOWN"), TEXT("Paso a Nivel cerrado: Tren aproximándose"));
	EnglishDictionary.Add(TEXT("ALERT_LEVEL_CROSSING_DOWN"), TEXT("Level Crossing closed: Approaching train"));

	SpanishDictionary.Add(TEXT("ALERT_ROADWORKS_ACTIVE"), TEXT("Obras en calzada: Precaución maquinistas"));
	EnglishDictionary.Add(TEXT("ALERT_ROADWORKS_ACTIVE"), TEXT("Roadworks ahead: Heavy equipment caution"));

	// Paneles de Mensaje Variable (PMV)
	SpanishDictionary.Add(TEXT("PMV_SPEED_LIMIT"), TEXT("DGT: RESPETE LOS LIMITES DE VELOCIDAD"));
	EnglishDictionary.Add(TEXT("PMV_SPEED_LIMIT"), TEXT("DGT: OBEY POSTED SPEED LIMITS"));

	SpanishDictionary.Add(TEXT("PMV_SEATBELT"), TEXT("DGT: EL CINTURON SALVA VIDAS"));
	EnglishDictionary.Add(TEXT("PMV_SEATBELT"), TEXT("DGT: SEATBELTS SAVE LIVES"));

	SpanishDictionary.Add(TEXT("PMV_FOG"), TEXT("DGT: NIEBLA MODERE VELOCIDAD"));
	EnglishDictionary.Add(TEXT("PMV_FOG"), TEXT("DGT: FOG AHEAD SLOW DOWN"));

	SpanishDictionary.Add(TEXT("PMV_RAIN"), TEXT("DGT: LLUVIA PELIGRO AQUAPLANING"));
	EnglishDictionary.Add(TEXT("PMV_RAIN"), TEXT("DGT: RAIN AQUAPLANING HAZARD"));

	SpanishDictionary.Add(TEXT("PMV_JAM"), TEXT("DGT: RETENCION PROXIMOS KILOMETROS"));
	EnglishDictionary.Add(TEXT("PMV_JAM"), TEXT("DGT: TRAFFIC JAM NEXT MILES"));

	// =========================================================================
	// 5. PRESUPUESTO, FINANZAS Y ECONOMÍA
	// =========================================================================
	SpanishDictionary.Add(TEXT("ECONOMY_FUNDS_AVAILABLE"), TEXT("€ Fondos disponibles"));
	EnglishDictionary.Add(TEXT("ECONOMY_FUNDS_AVAILABLE"), TEXT("€ Available funds"));

	SpanishDictionary.Add(TEXT("ECONOMY_BALANCE"), TEXT("Saldo en Tesorería"));
	EnglishDictionary.Add(TEXT("ECONOMY_BALANCE"), TEXT("Treasury Balance"));

	SpanishDictionary.Add(TEXT("ECONOMY_DAILY_MAINTENANCE"), TEXT("Mantenimiento diario"));
	EnglishDictionary.Add(TEXT("ECONOMY_DAILY_MAINTENANCE"), TEXT("Daily maintenance"));

	SpanishDictionary.Add(TEXT("ECONOMY_MONTHLY_MAINTENANCE"), TEXT("Mantenimiento mensual de calzadas"));
	EnglishDictionary.Add(TEXT("ECONOMY_MONTHLY_MAINTENANCE"), TEXT("Monthly road maintenance"));

	SpanishDictionary.Add(TEXT("ECONOMY_TOLL_REVENUE"), TEXT("Recaudación peajes"));
	EnglishDictionary.Add(TEXT("ECONOMY_TOLL_REVENUE"), TEXT("Toll revenue"));

	SpanishDictionary.Add(TEXT("ECONOMY_DGT_FINES"), TEXT("Sanciones DGT"));
	EnglishDictionary.Add(TEXT("ECONOMY_DGT_FINES"), TEXT("DGT traffic fines"));

	SpanishDictionary.Add(TEXT("ECONOMY_RAILWAY_CANON"), TEXT("Cánones de autopista ferroviaria"));
	EnglishDictionary.Add(TEXT("ECONOMY_RAILWAY_CANON"), TEXT("Railway intermodal transit fees"));

	SpanishDictionary.Add(TEXT("ECONOMY_NET_INCOME"), TEXT("Balance neto"));
	EnglishDictionary.Add(TEXT("ECONOMY_NET_INCOME"), TEXT("Net income"));

	SpanishDictionary.Add(TEXT("ECONOMY_BANKRUPT_WARNING"), TEXT("¡ALERTA: Fondos insuficientes para mantener la infraestructura!"));
	EnglishDictionary.Add(TEXT("ECONOMY_BANKRUPT_WARNING"), TEXT("WARNING: Insufficient funds to maintain infrastructure!"));

	SpanishDictionary.Add(TEXT("ECONOMY_COST_PER_METER"), TEXT("Coste por metro lineal: {0} €/m"));
	EnglishDictionary.Add(TEXT("ECONOMY_COST_PER_METER"), TEXT("Cost per meter: €{0}/m"));

	SpanishDictionary.Add(TEXT("ECONOMY_INSUFFICIENT_FUNDS"), TEXT("Fondos insuficientes para realizar esta obra."));
	EnglishDictionary.Add(TEXT("ECONOMY_INSUFFICIENT_FUNDS"), TEXT("Insufficient funds for this construction work."));

	// =========================================================================
	// 6. HUD Y MÉTRICAS EN TIEMPO REAL
	// =========================================================================
	SpanishDictionary.Add(TEXT("HUD_BUDGET"), TEXT("Presupuesto"));
	EnglishDictionary.Add(TEXT("HUD_BUDGET"), TEXT("Budget"));

	SpanishDictionary.Add(TEXT("HUD_CONGESTION"), TEXT("Congestión"));
	EnglishDictionary.Add(TEXT("HUD_CONGESTION"), TEXT("Congestion"));

	SpanishDictionary.Add(TEXT("HUD_ACCIDENTS"), TEXT("Accidentes Activos"));
	EnglishDictionary.Add(TEXT("HUD_ACCIDENTS"), TEXT("Active Incidents"));

	SpanishDictionary.Add(TEXT("HUD_ROAD_RAGE"), TEXT("Furia al Volante"));
	EnglishDictionary.Add(TEXT("HUD_ROAD_RAGE"), TEXT("Road Rage"));

	SpanishDictionary.Add(TEXT("HUD_SPEED"), TEXT("Velocidad de Simulación"));
	EnglishDictionary.Add(TEXT("HUD_SPEED"), TEXT("Simulation Speed"));

	SpanishDictionary.Add(TEXT("HUD_SPEED_PAUSE"), TEXT("Pausa (0x)"));
	EnglishDictionary.Add(TEXT("HUD_SPEED_PAUSE"), TEXT("Pause (0x)"));

	SpanishDictionary.Add(TEXT("HUD_SPEED_1X"), TEXT("Normal (1x)"));
	EnglishDictionary.Add(TEXT("HUD_SPEED_1X"), TEXT("Normal (1x)"));

	SpanishDictionary.Add(TEXT("HUD_SPEED_2X"), TEXT("Rápido (2x)"));
	EnglishDictionary.Add(TEXT("HUD_SPEED_2X"), TEXT("Fast (2x)"));

	SpanishDictionary.Add(TEXT("HUD_SPEED_4X"), TEXT("Muy Rápido (4x)"));
	EnglishDictionary.Add(TEXT("HUD_SPEED_4X"), TEXT("Very Fast (4x)"));

	SpanishDictionary.Add(TEXT("HUD_WEATHER"), TEXT("Meteorología"));
	EnglishDictionary.Add(TEXT("HUD_WEATHER"), TEXT("Weather"));

	SpanishDictionary.Add(TEXT("HUD_TIME"), TEXT("Hora del Día"));
	EnglishDictionary.Add(TEXT("HUD_TIME"), TEXT("Time of Day"));

	SpanishDictionary.Add(TEXT("HUD_TOTAL_KM"), TEXT("Red viaria: {0} km"));
	EnglishDictionary.Add(TEXT("HUD_TOTAL_KM"), TEXT("Road network: {0} km"));

	SpanishDictionary.Add(TEXT("HUD_VEHICLES_ACTIVE"), TEXT("Vehículos: {0}"));
	EnglishDictionary.Add(TEXT("HUD_VEHICLES_ACTIVE"), TEXT("Vehicles: {0}"));

	// =========================================================================
	// 7. PSICOLOGÍA VIAL Y ESTADO DE CONDUCTORES
	// =========================================================================
	SpanishDictionary.Add(TEXT("DRIVER_MOOD_CALM"), TEXT("Tranquilo"));
	EnglishDictionary.Add(TEXT("DRIVER_MOOD_CALM"), TEXT("Calm"));

	SpanishDictionary.Add(TEXT("DRIVER_MOOD_IMPATIENT"), TEXT("Impaciente"));
	EnglishDictionary.Add(TEXT("DRIVER_MOOD_IMPATIENT"), TEXT("Impatient"));

	SpanishDictionary.Add(TEXT("DRIVER_MOOD_IRRITATED"), TEXT("Irritado"));
	EnglishDictionary.Add(TEXT("DRIVER_MOOD_IRRITATED"), TEXT("Irritated"));

	SpanishDictionary.Add(TEXT("DRIVER_MOOD_ROAD_RAGE"), TEXT("Furia al Volante"));
	EnglishDictionary.Add(TEXT("DRIVER_MOOD_ROAD_RAGE"), TEXT("Road Rage"));

	SpanishDictionary.Add(TEXT("DRIVER_FRUSTRATION_INDEX"), TEXT("Índice de Frustración: {0}%"));
	EnglishDictionary.Add(TEXT("DRIVER_FRUSTRATION_INDEX"), TEXT("Frustration Index: {0}%"));

	// =========================================================================
	// 8. CLIMATOLOGÍA Y ENTORNO
	// =========================================================================
	SpanishDictionary.Add(TEXT("WEATHER_SUNNY"), TEXT("Soleado / Despejado"));
	EnglishDictionary.Add(TEXT("WEATHER_SUNNY"), TEXT("Sunny / Clear"));

	SpanishDictionary.Add(TEXT("WEATHER_OVERCAST"), TEXT("Nublado"));
	EnglishDictionary.Add(TEXT("WEATHER_OVERCAST"), TEXT("Overcast"));

	SpanishDictionary.Add(TEXT("WEATHER_RAIN_LIGHT"), TEXT("Llovizna suave"));
	EnglishDictionary.Add(TEXT("WEATHER_RAIN_LIGHT"), TEXT("Light Rain"));

	SpanishDictionary.Add(TEXT("WEATHER_RAIN_TORRENTIAL"), TEXT("Temporal DANA / Lluvia torrencial"));
	EnglishDictionary.Add(TEXT("WEATHER_RAIN_TORRENTIAL"), TEXT("Severe Storm / Torrential Rain"));

	SpanishDictionary.Add(TEXT("WEATHER_FOG"), TEXT("Niebla densa"));
	EnglishDictionary.Add(TEXT("WEATHER_FOG"), TEXT("Dense Fog"));

	SpanishDictionary.Add(TEXT("WEATHER_SNOW"), TEXT("Nevada intensa"));
	EnglishDictionary.Add(TEXT("WEATHER_SNOW"), TEXT("Heavy Snow"));

	// =========================================================================
	// 9. FERROCARRIL Y DESCONGESTIÓN MODAL
	// =========================================================================
	SpanishDictionary.Add(TEXT("RAILWAY_TRAIN_FREIGHT"), TEXT("Autopista Ferroviaria (Mercancías)"));
	EnglishDictionary.Add(TEXT("RAILWAY_TRAIN_FREIGHT"), TEXT("Rolling Highway (Freight Train)"));

	SpanishDictionary.Add(TEXT("RAILWAY_TRAIN_PASSENGER"), TEXT("Tren de Cercanías"));
	EnglishDictionary.Add(TEXT("RAILWAY_TRAIN_PASSENGER"), TEXT("Commuter Passenger Train"));

	SpanishDictionary.Add(TEXT("RAILWAY_TRAIN_AVE"), TEXT("Alta Velocidad (AVE)"));
	EnglishDictionary.Add(TEXT("RAILWAY_TRAIN_AVE"), TEXT("High-Speed Rail (AVE)"));

	SpanishDictionary.Add(TEXT("RAILWAY_CROSSING_OPEN"), TEXT("Paso a Nivel: Abierto al tráfico"));
	EnglishDictionary.Add(TEXT("RAILWAY_CROSSING_OPEN"), TEXT("Level Crossing: Open to traffic"));

	SpanishDictionary.Add(TEXT("RAILWAY_CROSSING_WARNING"), TEXT("Paso a Nivel: Tren aproximándose"));
	EnglishDictionary.Add(TEXT("RAILWAY_CROSSING_WARNING"), TEXT("Level Crossing: Approaching train"));

	SpanishDictionary.Add(TEXT("RAILWAY_CROSSING_CLOSED"), TEXT("Paso a Nivel: Cerrado (Barreras bajadas)"));
	EnglishDictionary.Add(TEXT("RAILWAY_CROSSING_CLOSED"), TEXT("Level Crossing: Closed (Barriers lowered)"));
}
