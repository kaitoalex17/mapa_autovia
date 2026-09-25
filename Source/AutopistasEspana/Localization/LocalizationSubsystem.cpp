#include "Localization/LocalizationSubsystem.h"
#include "AutopistasEspana.h"

void ULocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadDictionaries();
	UE_LOG(LogAutopistas, Log, TEXT("ULocalizationSubsystem inicializado con exito. Idioma por defecto: Espanol"));
}

void ULocalizationSubsystem::SetCurrentLanguage(EGameLanguage NewLanguage)
{
	if (CurrentLanguage != NewLanguage)
	{
		CurrentLanguage = NewLanguage;
		OnLanguageChanged.Broadcast(NewLanguage);
		UE_LOG(LogAutopistas, Log, TEXT("Idioma del juego cambiado a: %s"), (NewLanguage == EGameLanguage::Spanish) ? TEXT("Espanol") : TEXT("English"));
	}
}

FText ULocalizationSubsystem::GetLocalizedString(const FString& Key) const
{
	const TMap<FString, FString>& TargetDict = (CurrentLanguage == EGameLanguage::Spanish) ? SpanishDictionary : EnglishDictionary;

	if (const FString* Found = TargetDict.Find(Key))
	{
		return FText::FromString(*Found);
	}

	// Fallback en Espanol si falta en Ingles
	if (const FString* Fallback = SpanishDictionary.Find(Key))
	{
		return FText::FromString(*Fallback);
	}

	return FText::FromString(Key);
}

void ULocalizationSubsystem::LoadDictionaries()
{
	// DICCIONARIO ESPAÑOL
	SpanishDictionary.Add(TEXT("GAME_TITLE"), TEXT("Autopistas de España"));
	SpanishDictionary.Add(TEXT("MENU_CONTINUE"), TEXT("Continuar"));
	SpanishDictionary.Add(TEXT("MENU_NEW_GAME"), TEXT("Nueva Partida"));
	SpanishDictionary.Add(TEXT("MENU_LOAD_GAME"), TEXT("Cargar Partida"));
	SpanishDictionary.Add(TEXT("MENU_SETTINGS"), TEXT("Ajustes"));
	SpanishDictionary.Add(TEXT("MENU_EXIT"), TEXT("Salir al Escritorio"));

	SpanishDictionary.Add(TEXT("TOOL_SELECT"), TEXT("Inspeccionar"));
	SpanishDictionary.Add(TEXT("TOOL_ROAD_90"), TEXT("Carretera Convencional (90 km/h)"));
	SpanishDictionary.Add(TEXT("TOOL_HIGHWAY_2X2"), TEXT("Autovía 2x2 (120 km/h)"));
	SpanishDictionary.Add(TEXT("TOOL_HIGHWAY_3X3"), TEXT("Autopista 3x3 (120 km/h)"));
	SpanishDictionary.Add(TEXT("TOOL_ROUNDABOUT"), TEXT("Glorieta / Rotonda"));
	SpanishDictionary.Add(TEXT("TOOL_VIADUCT"), TEXT("Viaducto / Puente"));
	SpanishDictionary.Add(TEXT("TOOL_TUNNEL"), TEXT("Túnel de Montaña"));
	SpanishDictionary.Add(TEXT("TOOL_BUS_LINES"), TEXT("Líneas de Transporte"));
	SpanishDictionary.Add(TEXT("TOOL_DGT_CONTROLS"), TEXT("Operativo DGT / Conos"));
	SpanishDictionary.Add(TEXT("TOOL_PEGASUS"), TEXT("Patrulla Aérea Pegasus"));
	SpanishDictionary.Add(TEXT("TOOL_DEMOLISH"), TEXT("Demolición"));

	SpanishDictionary.Add(TEXT("HUD_BUDGET"), TEXT("Presupuesto"));
	SpanishDictionary.Add(TEXT("HUD_CONGESTION"), TEXT("Congestión"));
	SpanishDictionary.Add(TEXT("HUD_ACCIDENTS"), TEXT("Accidentes Activos"));
	SpanishDictionary.Add(TEXT("HUD_ROAD_RAGE"), TEXT("Furia al Volante"));
	SpanishDictionary.Add(TEXT("ALERT_ACCIDENT"), TEXT("¡ALERTA 112: Accidente en calzada! Grúa y Guardia Civil en camino."));
	SpanishDictionary.Add(TEXT("ALERT_WEATHER_RAIN"), TEXT("AVISO DGT: Lluvia intensa. Riesgo de aquaplaning."));
	SpanishDictionary.Add(TEXT("ALERT_ROAD_JAM"), TEXT("RETENCIÓN: Vía colapsada. Los conductores están perdiendo los nervios."));

	// ENGLISH DICTIONARY
	EnglishDictionary.Add(TEXT("GAME_TITLE"), TEXT("Highways of Spain"));
	EnglishDictionary.Add(TEXT("MENU_CONTINUE"), TEXT("Continue"));
	EnglishDictionary.Add(TEXT("MENU_NEW_GAME"), TEXT("New Game"));
	EnglishDictionary.Add(TEXT("MENU_LOAD_GAME"), TEXT("Load Game"));
	EnglishDictionary.Add(TEXT("MENU_SETTINGS"), TEXT("Settings"));
	EnglishDictionary.Add(TEXT("MENU_EXIT"), TEXT("Exit to Desktop"));

	EnglishDictionary.Add(TEXT("TOOL_SELECT"), TEXT("Inspect"));
	EnglishDictionary.Add(TEXT("TOOL_ROAD_90"), TEXT("Conventional Road (90 km/h)"));
	EnglishDictionary.Add(TEXT("TOOL_HIGHWAY_2X2"), TEXT("Dual Carriageway 2x2 (120 km/h)"));
	EnglishDictionary.Add(TEXT("TOOL_HIGHWAY_3X3"), TEXT("Motorway 3x3 (120 km/h)"));
	EnglishDictionary.Add(TEXT("TOOL_ROUNDABOUT"), TEXT("Roundabout"));
	EnglishDictionary.Add(TEXT("TOOL_VIADUCT"), TEXT("Viaduct / Bridge"));
	EnglishDictionary.Add(TEXT("TOOL_TUNNEL"), TEXT("Mountain Tunnel"));
	EnglishDictionary.Add(TEXT("TOOL_BUS_LINES"), TEXT("Transit Lines"));
	EnglishDictionary.Add(TEXT("TOOL_DGT_CONTROLS"), TEXT("DGT Police Checkpoint"));
	EnglishDictionary.Add(TEXT("TOOL_PEGASUS"), TEXT("Pegasus Helicopter Patrol"));
	EnglishDictionary.Add(TEXT("TOOL_DEMOLISH"), TEXT("Demolition"));

	EnglishDictionary.Add(TEXT("HUD_BUDGET"), TEXT("Budget"));
	EnglishDictionary.Add(TEXT("HUD_CONGESTION"), TEXT("Congestion"));
	EnglishDictionary.Add(TEXT("HUD_ACCIDENTS"), TEXT("Active Incidents"));
	EnglishDictionary.Add(TEXT("HUD_ROAD_RAGE"), TEXT("Road Rage"));
	EnglishDictionary.Add(TEXT("ALERT_ACCIDENT"), TEXT("112 EMERGENCY: Traffic collision! Tow truck and Highway Patrol dispatched."));
	EnglishDictionary.Add(TEXT("ALERT_WEATHER_RAIN"), TEXT("WEATHER WARNING: Heavy rain detected. Aquaplaning hazard."));
	EnglishDictionary.Add(TEXT("ALERT_ROAD_JAM"), TEXT("TRAFFIC JAM: Severe gridlock. Drivers are getting frustrated."));
}
