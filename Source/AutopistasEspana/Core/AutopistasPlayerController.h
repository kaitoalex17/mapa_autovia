#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AutopistasPlayerController.generated.h"

/**
 * Controlador de Jugador para vista cenital estrategica.
 * Muestra el cursor del raton permanentemente, gestiona clics en el terreno y delega a las herramientas de construccion.
 */
UCLASS()
class AUTOPISTASESPANA_API AAutopistasPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAutopistasPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	// Obtener la posicion del raton proyectada sobre el plano del terreno (Z=0 o suelo con raycast)
	UFUNCTION(BlueprintCallable, Category = "Autopistas|Interaction")
	bool GetMouseWorldPosition(FVector& OutWorldPosition);

	// Acciones de Raton
	void OnPrimaryClickPressed();
	void OnPrimaryClickReleased();
	void OnSecondaryClickPressed();
};
