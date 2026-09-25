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

float UEconomySubsystem::CalculateConstructionDurationSeconds(ERoadCategory Category, float LengthCm) const
{
	if (!bEnableConstructionImpact)
	{
		return 0.0f; // Si esta desactivado el impacto de obras, duracion cero
	}

	const float LengthMeters = LengthCm / 100.0f;
	float BaseSecondsPerHundredMeters = 3.0f;

	switch (Category)
	{
	case ERoadCategory::CaminoRural:
		BaseSecondsPerHundredMeters = 1.5f;
		break;
	case ERoadCategory::Convencional_90:
		BaseSecondsPerHundredMeters = 2.5f;
		break;
	case ERoadCategory::Autovia_120:
		BaseSecondsPerHundredMeters = 4.0f;
		break;
	case ERoadCategory::Autopista_3x3:
		BaseSecondsPerHundredMeters = 5.5f;
		break;
	default:
		BaseSecondsPerHundredMeters = 3.0f;
		break;
	}

	// Duracion minima garantizada de 10 segundos para percibir las 3 fases
	const float Calculated = (LengthMeters / 100.0f) * BaseSecondsPerHundredMeters;
	const float SafeDuration = FMath::Clamp(Calculated, 10.0f, 120.0f);
	
	const float EffectiveMultiplier = FMath::Max(0.1f, ConstructionSpeedMultiplier);
	return SafeDuration / EffectiveMultiplier;
}

void UEconomySubsystem::SetEnableConstructionImpact(bool bEnable)
{
	if (bEnableConstructionImpact != bEnable)
	{
		bEnableConstructionImpact = bEnable;
		UE_LOG(LogAutopistas, Log, TEXT("UEconomySubsystem: Impacto de Obras cambiado a: %s"), bEnable ? TEXT("ACTIVADO (Con Fases)") : TEXT("DESACTIVADO (Sandbox Instantaneo)"));
		OnConstructionImpactToggled.Broadcast(bEnable);
	}
}

void UEconomySubsystem::SetEnableDriverFrustration(bool bEnable)
{
	if (bEnableDriverFrustration != bEnable)
	{
		bEnableDriverFrustration = bEnable;
		UE_LOG(LogAutopistas, Log, TEXT("UEconomySubsystem: Psicologia y Furia de Conductores cambiada a: %s"), bEnable ? TEXT("ACTIVADO") : TEXT("DESACTIVADO"));
		OnDriverFrustrationToggled.Broadcast(bEnable);
	}
}

void UEconomySubsystem::SetConstructionSpeedMultiplier(float InMultiplier)
{
	ConstructionSpeedMultiplier = FMath::Clamp(InMultiplier, 0.1f, 10.0f);
	UE_LOG(LogAutopistas, Log, TEXT("UEconomySubsystem: Multiplicador de velocidad de obras: %.2fx"), ConstructionSpeedMultiplier);
}

