#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AutopistasGameModeBase.generated.h"

/**
 * Modo de Juego Principal de Autopistas de Espana.
 * Vincula la camara cenital y el controlador de raton por defecto.
 */
UCLASS()
class AUTOPISTASESPANA_API AAutopistasGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAutopistasGameModeBase();

	virtual void BeginPlay() override;

	// Configuracion por defecto del modo de juego para obras viales
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode Settings|Construction")
	bool bDefaultEnableConstructionImpact = true;

	// Configuracion por defecto para frustracion de conductores
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode Settings|Traffic Psychology")
	bool bDefaultEnableDriverFrustration = true;

	// Cambiar los ajustes en tiempo de ejecucion a traves del GameMode
	UFUNCTION(BlueprintCallable, Category = "GameMode Settings")
	void SetSimulationSettings(bool bEnableConstruction, bool bEnableFrustration);
};
