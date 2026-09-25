#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AutopistasTopDownCamera.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

/**
 * Camara cenital (Top-Down 2.5D) realista para Autopistas de Espana.
 * Controla el desplazamiento suave por el mapa (WASD / Borde de pantalla) y zoom continuo con rueda de raton.
 */
UCLASS()
class AUTOPISTASESPANA_API AAutopistasTopDownCamera : public APawn
{
	GENERATED_BODY()

public:
	AAutopistasTopDownCamera();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Componentes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* TopDownCamera;

	// Parametros de Movimiento
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Movement")
	float MoveSpeedBase = 3500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Movement")
	float MoveSpeedFastMultiplier = 2.5f;

	// Parametros de Zoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Zoom")
	float MinZoomArmLength = 1200.0f; // Vista cercana (detalles de coches y biondas)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Zoom")
	float MaxZoomArmLength = 65000.0f; // Vista estrategica comarcal

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Zoom")
	float ZoomSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Zoom")
	float ZoomInterpSpeed = 8.0f;

	// Limites de Mundo (Centimetros / UU)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Bounds")
	FVector2D WorldMinBounds = FVector2D(-400000.0f, -400000.0f); // -4 km

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Bounds")
	FVector2D WorldMaxBounds = FVector2D(400000.0f, 400000.0f);   // +4 km (8x8 km inicial)

	// Funciones de Entrada
	UFUNCTION(BlueprintCallable, Category = "Camera Input")
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = "Camera Input")
	void MoveRight(float Value);

	UFUNCTION(BlueprintCallable, Category = "Camera Input")
	void Zoom(float Value);

	UFUNCTION(BlueprintCallable, Category = "Camera Input")
	void SetFastMove(bool bFast);

private:
	FVector TargetMovementDirection;
	float TargetArmLength;
	bool bIsFastMoveActive = false;
};
