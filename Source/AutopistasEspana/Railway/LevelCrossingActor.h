#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Railway/RailwayTypes.h"
#include "LevelCrossingActor.generated.h"

class UProceduralMeshComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UBoxComponent;
class URailwayTrackComponent;
class URoadSplineComponent;
class ATrainVehicleActor;
class ATrafficVehicleAgent;

/**
 * Actor de Paso a Nivel para la interseccion entre autovias/carreteras y lineas de ferrocarril.
 * Incluye semibarreras abatibles motorizadas, senalizacion optica y acustica reglamentaria (ADIF / DGT),
 * y sistema de enclavamiento para detener el trafico rodado cuando se aproxima un tren.
 */
UCLASS()
class AUTOPISTASESPANA_API ALevelCrossingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelCrossingActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// --- COMPONENTES VISUALES Y MECANICOS ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	/** Pavimento especial de caucho/hormigon (sistema STRAIL) para cruce de calzada sobre los raíles */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Pavement")
	UProceduralMeshComponent* CrossingPavementMesh;

	/** Mastil de la semibarrera izquierda (sentido carretera) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Barriers")
	UStaticMeshComponent* BarrierMastLeft;

	/** Punto de giro motorizado de la semibarrera izquierda */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Barriers")
	USceneComponent* BarrierPivotLeft;

	/** Brazo abatible reflectante rojo/blanco izquierdo */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Barriers")
	UStaticMeshComponent* BarrierArmLeft;

	/** Mastil de la semibarrera derecha */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Barriers")
	UStaticMeshComponent* BarrierMastRight;

	/** Punto de giro motorizado de la semibarrera derecha */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Barriers")
	USceneComponent* BarrierPivotRight;

	/** Brazo abatible reflectante rojo/blanco derecho */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Barriers")
	UStaticMeshComponent* BarrierArmRight;

	// --- SENALIZACION LUMINOSA (FOCOS ROJOS ALTERNANTES) ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Lights")
	UPointLightComponent* RedLightLeftA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Lights")
	UPointLightComponent* RedLightLeftB;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Lights")
	UPointLightComponent* RedLightRightA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Lights")
	UPointLightComponent* RedLightRightB;

	// --- VOLUMENES DE SEGURIDAD Y BLOQUEO DE TRAFICO ---

	/** Volumen fisico que bloquea e impide el avance de vehiculos rodados ante la barrera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Safety")
	UBoxComponent* RoadTrafficBlockerBox;

	/** Zona de peligro sobre las vias para detectar vehiculos atrapados entre barreras */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Safety")
	UBoxComponent* TrackDangerZoneBox;

	// --- CONFIGURACION Y ESTADO ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Crossing")
	ELevelCrossingState CurrentState = ELevelCrossingState::Abierto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Crossing")
	ELevelCrossingClass CrossingClass = ELevelCrossingClass::ClaseC_Semibarreras;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Crossing")
	FLevelCrossingSettings Settings;

	/** Angulo actual de las semibarreras (90.0 = Vertical/Abierto, 0.0 = Horizontal/Cerrado) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Crossing|Mechanics")
	float CurrentBarrierAngle = 90.0f;

	/** Angulo objetivo segun el estado del enclavamiento */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Crossing|Mechanics")
	float TargetBarrierAngle = 90.0f;

	/** Referencia a la via ferrea con la que intersecta */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Crossing|Network")
	TWeakObjectPtr<URailwayTrackComponent> AssociatedTrack;

	/** Kilometro / distancia en la via ferrea (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Crossing|Network")
	float CrossingDistanceOnTrack = 0.0f;

	/** Referencia a la carretera con la que intersecta */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Crossing|Network")
	TWeakObjectPtr<URoadSplineComponent> AssociatedRoad;

	/** Kilometro / distancia en la carretera (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Crossing|Network")
	float CrossingDistanceOnRoad = 0.0f;

	// --- FUNCIONES DE CONTROL FERROVIARIO Y TRAFICO ---

	/** Indica si el trafico rodado esta actualmente bloqueado o detenido ante la barrera */
	UFUNCTION(BlueprintPure, Category = "Level Crossing")
	bool IsRoadTrafficBlocked() const { return CurrentState != ELevelCrossingState::Abierto; }

	/** Notificacion enviada por el tren cuando entra en el circuito de via de aproximacion */
	UFUNCTION(BlueprintCallable, Category = "Level Crossing")
	void NotifyTrainApproaching(ATrainVehicleActor* Train, float DistanceToCrossingCm);

	/** Notificacion enviada por el tren cuando su cabeza entra en el cruce */
	UFUNCTION(BlueprintCallable, Category = "Level Crossing")
	void NotifyTrainEnteredCrossing(ATrainVehicleActor* Train);

	/** Notificacion enviada por la cola del tren cuando libra completamente el paso a nivel */
	UFUNCTION(BlueprintCallable, Category = "Level Crossing")
	void NotifyTrainClearedCrossing(ATrainVehicleActor* Train);

	/** Cambia de forma forzada o programada el estado del paso a nivel */
	UFUNCTION(BlueprintCallable, Category = "Level Crossing")
	void SetCrossingState(ELevelCrossingState NewState);

	/** Comprueba si hay algun vehiculo averiado o atrapado en la zona de peligro de las vias */
	UFUNCTION(BlueprintPure, Category = "Level Crossing|Safety")
	bool HasTrappedVehicleOnTracks() const;

	/** Obtiene la lista de vehiculos actualmente atrapados en las vias para su rescate o retirada */
	UFUNCTION(BlueprintCallable, Category = "Level Crossing|Safety")
	TArray<ATrafficVehicleAgent*> GetTrappedVehicles() const;

private:
	/** Actualiza la interpolacion angular de las semibarreras abatibles */
	void UpdateBarrierRotation(float DeltaTime);

	/** Actualiza el parpadeo alternante de los focos rojos reglamentarios */
	void UpdateRedLightSignaling(float DeltaTime);

	/** Gestiona la parada obligatoria de vehiculos de la carretera ante la linea de detencion */
	void ManageRoadTrafficStopping(float DeltaTime);

	/** Genera proceduralmente la losa de cruce (placas STRAIL) a nivel entre raíles */
	void BuildCrossingPavement();

	/** Temporizador interno para fases de aviso previo y enclavamiento */
	float PhaseTimer = 0.0f;

	/** Temporizador para la oscilacion del parpadeo luminoso */
	float BlinkTimer = 0.0f;

	/** Estado logico alternante de los focos A y B */
	bool bBlinkToggle = false;

	/** Tren que tiene actualmente reservado o bloqueado el paso */
	TWeakObjectPtr<ATrainVehicleActor> ActiveTrain;

	/** Lista de vehiculos de carretera que han sido detenidos ante la barrera */
	UPROPERTY()
	TArray<TWeakObjectPtr<ATrafficVehicleAgent>> StoppedRoadVehicles;
};
