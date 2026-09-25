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
};
