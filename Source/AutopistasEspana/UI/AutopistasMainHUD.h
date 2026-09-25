#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/UITypes.h"
#include "AutopistasMainHUD.generated.h"

class UAutopistasHUDWidget;

/**
 * Gestor Principal del HUD (AHUD).
 * Crea el widget UMG en el Viewport del jugador y transmite las metricas de la simulacion a la interfaz grafica.
 */
UCLASS()
class AUTOPISTASESPANA_API AAutopistasMainHUD : public AHUD
{
	GENERATED_BODY()

public:
	AAutopistasMainHUD();

protected:
	virtual void BeginPlay() override;

public:
	// Clase del Widget de HUD (asignable desde Blueprint o C++)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration")
	TSubclassOf<UAutopistasHUDWidget> HUDWidgetClass;

	// Instancia activa del Widget
	UPROPERTY(BlueprintReadOnly, Category = "UI Configuration")
	UAutopistasHUDWidget* ActiveHUDWidget;

	// Actualizar metricas en pantalla
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RefreshHUDMetrics(const FHUDSimulationData& Data);
};
