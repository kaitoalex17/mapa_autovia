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

	// Notificar aparicion de siniestro o alerta de trafico en pantalla
	UFUNCTION(BlueprintImplementableEvent, Category = "Autopistas UI")
	void ShowIncidentToastAlert(const FString& IncidentMessage, bool bIsEmergency);

	// Evento para alternar la velocidad de simulacion (Pausa, 1x, 2x, 4x)
	UFUNCTION(BlueprintCallable, Category = "Autopistas UI")
	void SetSimulationSpeed(float NewSpeed);

	// Obtener la herramienta activa actualmente
	UFUNCTION(BlueprintPure, Category = "Autopistas UI")
	EActiveBuildTool GetCurrentTool() const { return CurrentTool; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Autopistas UI")
	EActiveBuildTool CurrentTool = EActiveBuildTool::Seleccionar;
};
