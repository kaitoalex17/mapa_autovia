#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Roads/RoadTypes.h"
#include "EconomySubsystem.generated.h"

/**
 * Subsistema Global de Economia y Presupuestos (€).
 * Gestiona el balance de fondos, costes de construccion por metro, mantenimiento del firme,
 * recaudacion de peajes (Via-T), sanciones de trafico (radares) y penalizaciones por siniestralidad.
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

	// Balance mensual proyectado
	UFUNCTION(BlueprintPure, Category = "Economy")
	int32 GetProjectedMonthlyBalance() const { return MonthlyNetIncome; }

private:
	int32 MonthlyNetIncome = 45000;
};
