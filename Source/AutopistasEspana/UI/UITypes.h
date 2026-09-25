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
};
