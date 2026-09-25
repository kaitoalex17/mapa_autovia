#pragma once

#include "CoreMinimal.h"
#include "UITypes.generated.h"

/** Herramienta de construccion o gestion activa en la interfaz */
UENUM(BlueprintType)
enum class EActiveBuildTool : uint8
{
	Seleccionar          UMETA(DisplayName = "Inspeccionar / Cursor"),
	Carretera_90         UMETA(DisplayName = "Carretera Convencional (90 km/h)"),
	Autovia_2x2          UMETA(DisplayName = "Autovia 2x2 (120 km/h)"),
	Autopista_3x3        UMETA(DisplayName = "Autopista 3x3 (120 km/h)"),
	RotondaGlorieta      UMETA(DisplayName = "Glorieta / Rotonda"),
	PuenteViaducto       UMETA(DisplayName = "Viaducto / Puente"),
	TunelMontana         UMETA(DisplayName = "Tunel Subterraneo"),
	Peaje                UMETA(DisplayName = "Peaje / Tronco de Autovia"),
	PasoANivel           UMETA(DisplayName = "Paso a Nivel ADIF Clase C"),
	LineasTransporte     UMETA(DisplayName = "Lineas de Autobuses / Fletes"),
	ControlesDGT         UMETA(DisplayName = "Operativo DGT / Conos / Alcoholemia"),
	HelicopteroPegasus   UMETA(DisplayName = "Patrulla Aerea Pegasus"),
	Demoler              UMETA(DisplayName = "Demolicion / Excavadora")
};

/** Selector modular de carriles estilo Highways & Co. (1L a 4L) */
UENUM(BlueprintType)
enum class ENumLanesBuild : uint8
{
	Lane_1L              UMETA(DisplayName = "1 Carril (3.50 m) - Ramal / Via Servicio"),
	Lanes_2L             UMETA(DisplayName = "2 Carriles (7.00 m) - Autovia Estandar"),
	Lanes_3L             UMETA(DisplayName = "3 Carriles (10.50 m) - Autopista Troncal"),
	Lanes_4L             UMETA(DisplayName = "4 Carriles (14.00 m) - Macro-Autovia Perimetral")
};

/** Selector de cota y rasante vertical (Highways & Co. Elevation Steps) */
UENUM(BlueprintType)
enum class ERoadElevationStep : uint8
{
	Tunel_NivelMenos1    UMETA(DisplayName = "Nivel -1: Tunel Subterraneo (-6.0 m / -600 UU)"),
	Suelo_RasanteCero    UMETA(DisplayName = "Nivel 0: Cota Rasante Terreno (0.0 m / 0 UU)"),
	Puente_NivelMas1     UMETA(DisplayName = "Nivel +1: Viaducto / Puente (+6.0 m / +600 UU)"),
	PasoAlto_NivelMas2   UMETA(DisplayName = "Nivel +2: Flyover Distinto Nivel (+12.0 m / +1200 UU)")
};

/** Modo de trazado y geometria CAD */
UENUM(BlueprintType)
enum class ERoadDrawMode : uint8
{
	Recta                UMETA(DisplayName = "Alineacion Recta Ortogonal"),
	CurvaBezier          UMETA(DisplayName = "Curva Suave / Radio Fijo"),
	SplineLibre          UMETA(DisplayName = "Spline Continuo Libre"),
	Glorieta             UMETA(DisplayName = "Glorieta Giratoria Tangencial")
};

/** Sentido de circulacion */
UENUM(BlueprintType)
enum class ETrafficDirectionMode : uint8
{
	SentidoUnico         UMETA(DisplayName = "Sentido Unico (One-Way)"),
	DobleSentido         UMETA(DisplayName = "Doble Sentido con Mediana (Two-Way)")
};


/** Datos en tiempo real para refrescar la interfaz (HUD) */
USTRUCT(BlueprintType)
struct FHUDSimulationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Data")
	int64 CashEuros = 5000000; // Fondos en Euros

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Data")
	int32 MonthlyIncomeEuros = 45000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Data")
	float CongestionPercent = 12.0f; // 0% a 100%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Data")
	int32 ActiveAccidents = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Data")
	float RoadRagePercent = 4.0f; // % de conductores en furia al volante

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Data")
	int32 TimeHours = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Data")
	int32 TimeMinutes = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Data")
	float SimulationSpeed = 1.0f; // 0x (Pausa), 1x, 2x, 4x

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Data")
	FString CurrentWeather = TEXT("Soleado / Despejado");

	// Obtiene la ruta al asset vectorial SVG correspondiente segun la herramienta activa
	static FString GetToolVectorIconPath(EActiveBuildTool Tool)
	{
		switch (Tool)
		{
		case EActiveBuildTool::Seleccionar:
			return TEXT("/Game/Textures/UI/Icons/icon_cursor_inspect.svg");
		case EActiveBuildTool::Carretera_90:
			return TEXT("/Game/Textures/UI/Icons/icon_road_convencional.svg");
		case EActiveBuildTool::Autovia_2x2:
			return TEXT("/Game/Textures/UI/Icons/icon_autovia_2x2.svg");
		case EActiveBuildTool::Autopista_3x3:
			return TEXT("/Game/Textures/UI/Icons/icon_autovia_3x3.svg");
		case EActiveBuildTool::RotondaGlorieta:
			return TEXT("/Game/Textures/UI/Icons/icon_rotonda_glorieta.svg");
		case EActiveBuildTool::PuenteViaducto:
			return TEXT("/Game/Textures/UI/Icons/icon_puente_viaducto.svg");
		case EActiveBuildTool::TunelMontana:
			return TEXT("/Game/Textures/UI/Icons/icon_tunel_montana.svg");
		case EActiveBuildTool::Peaje:
			return TEXT("/Game/Textures/UI/Icons/icon_peaje_troncal.svg");
		case EActiveBuildTool::PasoANivel:
			return TEXT("/Game/Textures/UI/Icons/icon_paso_nivel.svg");
		case EActiveBuildTool::LineasTransporte:
			return TEXT("/Game/Textures/UI/Icons/icon_grua_112.svg");
		case EActiveBuildTool::ControlesDGT:
			return TEXT("/Game/Textures/UI/Icons/icon_guardia_civil.svg");
		case EActiveBuildTool::HelicopteroPegasus:
			return TEXT("/Game/Textures/UI/Icons/icon_helicoptero_pegasus.svg");
		case EActiveBuildTool::Demoler:
			return TEXT("/Game/Textures/UI/Icons/icon_demoler.svg");
		default:
			return TEXT("/Game/Textures/UI/Icons/icon_cursor_inspect.svg");
		}
	}

	// Obtiene la ruta al asset vectorial SVG para el selector de carriles (1L - 4L)
	static FString GetLanesVectorIconPath(ENumLanesBuild Lanes)
	{
		switch (Lanes)
		{
		case ENumLanesBuild::Lane_1L:
			return TEXT("/Game/Textures/UI/Icons/icon_lanes_1.svg");
		case ENumLanesBuild::Lanes_2L:
			return TEXT("/Game/Textures/UI/Icons/icon_lanes_2.svg");
		case ENumLanesBuild::Lanes_3L:
			return TEXT("/Game/Textures/UI/Icons/icon_lanes_3.svg");
		case ENumLanesBuild::Lanes_4L:
			return TEXT("/Game/Textures/UI/Icons/icon_lanes_4.svg");
		default:
			return TEXT("/Game/Textures/UI/Icons/icon_lanes_2.svg");
		}
	}

	// Obtiene la ruta al asset vectorial SVG para el selector de cota / elevacion
	static FString GetElevationVectorIconPath(ERoadElevationStep Elevation)
	{
		switch (Elevation)
		{
		case ERoadElevationStep::Tunel_NivelMenos1:
			return TEXT("/Game/Textures/UI/Icons/icon_elevation_tunnel.svg");
		case ERoadElevationStep::Suelo_RasanteCero:
			return TEXT("/Game/Textures/UI/Icons/icon_elevation_ground.svg");
		case ERoadElevationStep::Puente_NivelMas1:
			return TEXT("/Game/Textures/UI/Icons/icon_elevation_bridge.svg");
		case ERoadElevationStep::PasoAlto_NivelMas2:
			return TEXT("/Game/Textures/UI/Icons/icon_elevation_highbridge.svg");
		default:
			return TEXT("/Game/Textures/UI/Icons/icon_elevation_ground.svg");
		}
	}

	// Obtiene la ruta al asset vectorial SVG para el modo de trazado CAD
	static FString GetDrawModeVectorIconPath(ERoadDrawMode Mode)
	{
		switch (Mode)
		{
		case ERoadDrawMode::Recta:
			return TEXT("/Game/Textures/UI/Icons/icon_mode_straight.svg");
		case ERoadDrawMode::CurvaBezier:
		case ERoadDrawMode::SplineLibre:
			return TEXT("/Game/Textures/UI/Icons/icon_mode_free.svg");
		case ERoadDrawMode::Glorieta:
			return TEXT("/Game/Textures/UI/Icons/icon_rotonda_glorieta.svg");
		default:
			return TEXT("/Game/Textures/UI/Icons/icon_mode_straight.svg");
		}
	}

	// Obtiene la ruta al asset vectorial SVG para el sentido de circulacion
	static FString GetDirectionVectorIconPath(ETrafficDirectionMode Direction)
	{
		switch (Direction)
		{
		case ETrafficDirectionMode::SentidoUnico:
			return TEXT("/Game/Textures/UI/Icons/icon_dir_oneway.svg");
		case ETrafficDirectionMode::DobleSentido:
			return TEXT("/Game/Textures/UI/Icons/icon_dir_twoway.svg");
		default:
			return TEXT("/Game/Textures/UI/Icons/icon_dir_twoway.svg");
		}
	}
};

