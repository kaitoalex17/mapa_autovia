#include "Economy/EconomySubsystem.h"
#include "AutopistasEspana.h"

void UEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogAutopistas, Log, TEXT("UEconomySubsystem inicializado. Fondos iniciales: %lld Euros."), CurrentCashEuros);
}

bool UEconomySubsystem::DeductFunds(int64 Amount)
{
	if (CanAfford(Amount))
	{
		CurrentCashEuros -= Amount;
		UE_LOG(LogAutopistas, Log, TEXT("Gastados %lld Euros. Saldo restante: %lld Euros."), Amount, CurrentCashEuros);
		return true;
	}

	UE_LOG(LogAutopistas, Warning, TEXT("FONDOS INSUFICIENTES: Se requieren %lld Euros, pero solo hay %lld Euros."), Amount, CurrentCashEuros);
	return false;
}

void UEconomySubsystem::AddFunds(int64 Amount)
{
	CurrentCashEuros += Amount;
	UE_LOG(LogAutopistas, Log, TEXT("Ingresados %lld Euros. Nuevo saldo: %lld Euros."), Amount, CurrentCashEuros);
}

int64 UEconomySubsystem::CalculateRoadSegmentCost(ERoadCategory Category, float LengthCm, bool bIsViaduct, bool bIsTunnel) const
{
	const float LengthMeters = LengthCm / 100.0f; // 100 UU = 1 m
	int32 CostPerMeter = CostPerMeterConvencional;

	if (bIsTunnel)
	{
		CostPerMeter = CostPerMeterTunel;
	}
	else if (bIsViaduct)
	{
		CostPerMeter = CostPerMeterViaducto;
	}
	else
	{
		switch (Category)
		{
		case ERoadCategory::Convencional_90:
			CostPerMeter = CostPerMeterConvencional;
			break;
		case ERoadCategory::Autovia_120:
			CostPerMeter = CostPerMeterAutovia2x2;
			break;
		case ERoadCategory::Autopista_3x3:
			CostPerMeter = CostPerMeterAutopista3x3;
			break;
		default:
			CostPerMeter = CostPerMeterConvencional;
			break;
		}
	}

	return static_cast<int64>(LengthMeters * CostPerMeter);
}
