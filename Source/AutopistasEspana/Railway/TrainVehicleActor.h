#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Railway/RailwayTypes.h"
#include "TrainVehicleActor.generated.h"

class UStaticMeshComponent;
class URailwayTrackComponent;
class ALevelCrossingActor;
class ATrafficVehicleAgent;

/**
 * Actor de Convoy Ferroviario (Tren de Mercancias / Cercanias / Autopista Ferroviaria).
 * Circula de forma guiada sobre el spline de URailwayTrackComponent, interactua con
 * los pasos a nivel para accionar semibarreras y senales rojas, y ejecuta la retirada
 * activa de vehiculos pesados (camiones) y turismos de las autovias para descongestionar la red viaria.
 */
UCLASS()
class AUTOPISTASESPANA_API ATrainVehicleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATrainVehicleActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// --- COMPONENTES VISUALES ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TrainRoot;

	/** Malla de la locomotora tractora (Euro 4000 / Serie 333 / 252) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Rolling Stock")
	UStaticMeshComponent* LocomotiveMesh;

	/** Mallas de los vagones acoplados en cola */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Rolling Stock")
	TArray<UStaticMeshComponent*> WagonMeshes;

	// --- CONFIGURACION DEL TREN ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Configuration")
	ETrainType TrainType = ETrainType::MercanciasContenedores;

	/** Numero de vagones portacontenedores / plataformas rebajadas tipo Modalohr / portacoches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Configuration")
	int32 WagonCount = 5;

	/** Longitud de cada vagon en centimetros (14 metros tipico) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Configuration")
	float WagonLengthCm = 1400.0f;

	/** Separacion de enganche entre vagones (1.5 metros) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Configuration")
	float WagonCouplingSpacingCm = 150.0f;

	/** Masa total del convoy ferroviario en toneladas metricas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Physics")
	float TrainMassTons = 1200.0f;

	// --- DINAMICA Y CINEMATICA SOBRE VIA ---

	/** Referencia al spline de la via ferrea actual */
	UPROPERTY(BlueprintReadWrite, Category = "Navigation")
	TWeakObjectPtr<URailwayTrackComponent> CurrentTrackSpline;

	/** Posicion longitudinal sobre el spline en centimetros */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float DistanceAlongSpline = 0.0f;

	/** Velocidad actual en cm/s */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float CurrentSpeedCmS = 0.0f;

	/** Velocidad actual en km/h */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinematics")
	float CurrentSpeedKmh = 0.0f;

	/** Velocidad de crucero consignada (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinematics")
	float TargetCruiseSpeedKmh = 100.0f;

	/** Velocidad maxima autorizada (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinematics")
	float MaxSpeedKmh = 120.0f;

	/** Tasa de aceleracion del convoy (cm/s^2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Physics")
	float AccelerationCmS2 = 60.0f;

	/** Tasa de deceleracion por freno de servicio (cm/s^2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Physics")
	float ServiceBrakingDecelerationCmS2 = 90.0f;

	// --- REGISTRO DE CARGA Y RETIRADA DE VEHICULOS (AUTOPISTA FERROVIARIA) ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Intermodal Logistics")
	FRailwayCargoManifest CargoManifest;

	/** Radio de barrido para transferir camiones de la autovia al tren (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intermodal Logistics")
	float HighwaySweepRadiusCm = 6000.0f; // 60 metros alrededor del tren

	/** Habilita la descongestion automatica continua de vehiculos en autovias paralelas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intermodal Logistics")
	bool bAutoDecongestHighway = true;

	// --- FUNCIONES Y METODOS PUBLICOS ---

	/** Retira un vehiculo especifico (camion o coche) de la calzada e ingresa su carga en el tren */
	UFUNCTION(BlueprintCallable, Category = "Intermodal Logistics")
	bool RemoveHighwayVehicle(ATrafficVehicleAgent* RoadVehicle, FString& OutLog);

	/** Efectua un barrido sobre la autovia adyacente y transfiere vehiculos pesados/ligeros al tren */
	UFUNCTION(BlueprintCallable, Category = "Intermodal Logistics")
	int32 DecongestParallelHighway(float SearchRadiusCm);

	/** Rescata y retira vehiculos averiados o atrapados en pasos a nivel proximos */
	UFUNCTION(BlueprintCallable, Category = "Railway Safety")
	void ClearBlockedVehiclesAtUpcomingCrossing();

	/** Acciona la senal acustica de advertencia (bocina / silbato bitono) */
	UFUNCTION(BlueprintCallable, Category = "Railway Audio")
	void SoundTrainHorn();

	/** Obtiene la longitud total del convoy (locomotora + vagones acoplados) */
	UFUNCTION(BlueprintPure, Category = "Train Geometry")
	float GetTotalTrainLengthCm() const;

private:
	/** Actualiza el avance cinemático de la locomotora y la articulacion de los vagones */
	void UpdateTrainKinematics(float DeltaTime);

	/** Supervisa y comunica por adelantado a los pasos a nivel en la trayectoria */
	void MonitorLevelCrossings(float DeltaTime);

	/** Rutina periodica de descongestion intermodal */
	void TickHighwayDecongestion(float DeltaTime);

	/** Instancia procedural o crea los componentes de vagones en cascada */
	void InitializeWagons();

	float DecongestionTimer = 0.0f;
	float DecongestionInterval = 3.0f; // Cada 3 segundos evalua retirada de trafico rodado

	/** Pasos a nivel actualmente bajo supervision del tren */
	UPROPERTY()
	TArray<TWeakObjectPtr<ALevelCrossingActor>> MonitoredCrossings;
};
