#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/UITypes.h"
#include "AutopistasHUDWidget.generated.h"

/**
 * Widget Principal de la Interfaz Grafica (HUD).
 * Muestra la barra superior de estado (Finanzas, Hora, Congestion, Accidentes, Humor de conductores)
 * y la barra inferior de herramientas viales de construccion y gestion DGT.
 */
UCLASS()
class AUTOPISTASESPANA_API UAutopistasHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Actualizar datos globales del HUD
	UFUNCTION(BlueprintImplementableEvent, Category = "Autopistas UI")
	void OnSimulationDataUpdated(const FHUDSimulationData& NewData);

	// Notificar cambio de herramienta de construccion activa
	UFUNCTION(BlueprintCallable, Category = "Autopistas UI")
	void SelectBuildTool(EActiveBuildTool NewTool);

	// Notificar seleccion de numero de carriles (1L, 2L, 3L, 4L)
	UFUNCTION(BlueprintCallable, Category = "Autopistas UI|Dock")
	void SelectLanes(ENumLanesBuild NewLanes);

	// Notificar seleccion de cota / nivel de rasante (-1 Tunel, 0 Rasante, +1 Puente, +2 Flyover)
	UFUNCTION(BlueprintCallable, Category = "Autopistas UI|Dock")
	void SelectElevation(ERoadElevationStep NewElevation);

	// Notificar seleccion de modo de trazado CAD (Recta, Curva, Spline Libre, Glorieta)
	UFUNCTION(BlueprintCallable, Category = "Autopistas UI|Dock")
	void SelectDrawMode(ERoadDrawMode NewMode);

	// Notificar seleccion de sentido de circulacion (Unidireccional / Bidireccional)
	UFUNCTION(BlueprintCallable, Category = "Autopistas UI|Dock")
	void SelectDirection(ETrafficDirectionMode NewDirection);

	// Notificar aparicion de siniestro o alerta de trafico en pantalla
	UFUNCTION(BlueprintImplementableEvent, Category = "Autopistas UI")
	void ShowIncidentToastAlert(const FString& IncidentMessage, bool bIsEmergency);

	// Evento para alternar la velocidad de simulacion (Pausa, 1x, 2x, 4x)
	UFUNCTION(BlueprintCallable, Category = "Autopistas UI")
	void SetSimulationSpeed(float NewSpeed);

	// Getters
	UFUNCTION(BlueprintPure, Category = "Autopistas UI")
	EActiveBuildTool GetCurrentTool() const { return CurrentTool; }

	UFUNCTION(BlueprintPure, Category = "Autopistas UI|Dock")
	ENumLanesBuild GetCurrentLanes() const { return CurrentLanes; }

	UFUNCTION(BlueprintPure, Category = "Autopistas UI|Dock")
	ERoadElevationStep GetCurrentElevation() const { return CurrentElevation; }

	UFUNCTION(BlueprintPure, Category = "Autopistas UI|Dock")
	ERoadDrawMode GetCurrentDrawMode() const { return CurrentDrawMode; }

	UFUNCTION(BlueprintPure, Category = "Autopistas UI|Dock")
	ETrafficDirectionMode GetCurrentDirection() const { return CurrentDirection; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Autopistas UI")
	EActiveBuildTool CurrentTool = EActiveBuildTool::Seleccionar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Autopistas UI|Dock")
	ENumLanesBuild CurrentLanes = ENumLanesBuild::Lanes_2L;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Autopistas UI|Dock")
	ERoadElevationStep CurrentElevation = ERoadElevationStep::Suelo_RasanteCero;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Autopistas UI|Dock")
	ERoadDrawMode CurrentDrawMode = ERoadDrawMode::CurvaBezier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Autopistas UI|Dock")
	ETrafficDirectionMode CurrentDirection = ETrafficDirectionMode::DobleSentido;
};

