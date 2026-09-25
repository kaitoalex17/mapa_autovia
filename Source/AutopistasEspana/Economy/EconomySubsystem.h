#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Roads/RoadTypes.h"
#include "EconomySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConstructionImpactToggled, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriverFrustrationToggled, bool, bEnabled);

/**
 * Subsistema Global de Economia y Presupuestos (€).
 * Gestiona el balance de fondos, costes de construccion por metro, mantenimiento del firme,
 * recaudacion de peajes (Via-T), sanciones de trafico (radares) y penalizaciones por siniestralidad.
 * Ademas, centraliza los ajustes de simulacion (Impacto de Obras y Psicologia de Conductores).
 */
UCLASS()
class AUTOPISTASESPANA_API UEconomySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Balance actual en Euros
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int64 CurrentCashEuros = 5000000; // 5 Millones iniciales

	// Costes de construccion por metro lineal (€/m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Costs")
	int32 CostPerMeterConvencional = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Costs")
	int32 CostPerMeterAutovia2x2 = 600;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Costs")
	int32 CostPerMeterAutopista3x3 = 1100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Costs")
	int32 CostPerMeterViaducto = 3500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction Costs")
	int32 CostPerMeterTunel = 8000;

	// Coste de conservacion de carreteras (€/km mensual)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maintenance")
	int32 MaintenanceCostPerKmMonth = 450;

	// --- AJUSTES DE SIMULACION CONMUTABLES (MODO REALISTA VS SANDBOX) ---

	/** Si esta activado, la construccion tiene fases temporales de obra con cortes y reduccion de velocidad. Si se desactiva, las carreteras se abren instantaneamente. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings|Construction")
	bool bEnableConstructionImpact = true;

	/** Si esta activado, los conductores acumulan estres y furia al volante en atascos y retenciones por obras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings|Traffic Psychology")
	bool bEnableDriverFrustration = true;

	/** Multiplicador de velocidad de ejecucion de las obras viales (1.0 = normal, 2.0 = rapido) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings|Construction")
	float ConstructionSpeedMultiplier = 1.0f;

	// Eventos notificadores para la UI y subsistemas
	UPROPERTY(BlueprintAssignable, Category = "Simulation Events")
	FOnConstructionImpactToggled OnConstructionImpactToggled;

	UPROPERTY(BlueprintAssignable, Category = "Simulation Events")
	FOnDriverFrustrationToggled OnDriverFrustrationToggled;

	// Comprobar si hay fondos suficientes
	UFUNCTION(BlueprintPure, Category = "Economy")
	bool CanAfford(int64 Amount) const { return CurrentCashEuros >= Amount; }

	// Deducir fondos por obra o servicio de emergencia
	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool DeductFunds(int64 Amount);

	// Ingresar fondos (peajes, subvenciones o multas)
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddFunds(int64 Amount);

	// Calcular presupuesto requerido para un tramo vial segun longitud y tipologia
	UFUNCTION(BlueprintPure, Category = "Economy")
	int64 CalculateRoadSegmentCost(ERoadCategory Category, float LengthCm, bool bIsViaduct, bool bIsTunnel) const;

	// Calcular duracion estimada de las obras segun longitud y categoria
	UFUNCTION(BlueprintPure, Category = "Economy|Construction")
	float CalculateConstructionDurationSeconds(ERoadCategory Category, float LengthCm) const;

	// Configurar toggle de impacto de obras
	UFUNCTION(BlueprintCallable, Category = "Simulation Settings|Construction")
	void SetEnableConstructionImpact(bool bEnable);

	UFUNCTION(BlueprintPure, Category = "Simulation Settings|Construction")
	bool IsConstructionImpactEnabled() const { return bEnableConstructionImpact; }

	// Configurar toggle de psicologia y furia de conductores
	UFUNCTION(BlueprintCallable, Category = "Simulation Settings|Traffic Psychology")
	void SetEnableDriverFrustration(bool bEnable);

	UFUNCTION(BlueprintPure, Category = "Simulation Settings|Traffic Psychology")
	bool IsDriverFrustrationEnabled() const { return bEnableDriverFrustration; }

	// Configurar multiplicador de velocidad de obras
	UFUNCTION(BlueprintCallable, Category = "Simulation Settings|Construction")
	void SetConstructionSpeedMultiplier(float InMultiplier);

	UFUNCTION(BlueprintPure, Category = "Simulation Settings|Construction")
	float GetConstructionSpeedMultiplier() const { return ConstructionSpeedMultiplier; }

	// Balance mensual proyectado
	UFUNCTION(BlueprintPure, Category = "Economy")
	int32 GetProjectedMonthlyBalance() const { return MonthlyNetIncome; }

private:
	int32 MonthlyNetIncome = 45000;
};
