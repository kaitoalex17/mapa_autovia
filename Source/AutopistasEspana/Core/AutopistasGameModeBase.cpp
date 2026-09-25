#include "Core/AutopistasGameModeBase.h"
#include "Camera/AutopistasTopDownCamera.h"
#include "Core/AutopistasPlayerController.h"
#include "UI/AutopistasMainHUD.h"
#include "Economy/EconomySubsystem.h"
#include "AutopistasEspana.h"

AAutopistasGameModeBase::AAutopistasGameModeBase()
{
	DefaultPawnClass = AAutopistasTopDownCamera::StaticClass();
	PlayerControllerClass = AAutopistasPlayerController::StaticClass();
	HUDClass = AAutopistasMainHUD::StaticClass();
}

void AAutopistasGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UEconomySubsystem* EconomySubsystem = World->GetSubsystem<UEconomySubsystem>())
		{
			EconomySubsystem->SetEnableConstructionImpact(bDefaultEnableConstructionImpact);
			EconomySubsystem->SetEnableDriverFrustration(bDefaultEnableDriverFrustration);
			UE_LOG(LogAutopistas, Log, TEXT("AAutopistasGameModeBase: Inicializados ajustes de simulación (Obras: %d, Frustración: %d)."),
				bDefaultEnableConstructionImpact, bDefaultEnableDriverFrustration);
		}
	}
}

void AAutopistasGameModeBase::SetSimulationSettings(bool bEnableConstruction, bool bEnableFrustration)
{
	bDefaultEnableConstructionImpact = bEnableConstruction;
	bDefaultEnableDriverFrustration = bEnableFrustration;

	if (UWorld* World = GetWorld())
	{
		if (UEconomySubsystem* EconomySubsystem = World->GetSubsystem<UEconomySubsystem>())
		{
			EconomySubsystem->SetEnableConstructionImpact(bEnableConstruction);
			EconomySubsystem->SetEnableDriverFrustration(bEnableFrustration);
		}
	}
}
