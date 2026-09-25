#include "UI/AutopistasMainHUD.h"
#include "UI/AutopistasHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "AutopistasEspana.h"

AAutopistasMainHUD::AAutopistasMainHUD()
{
	ActiveHUDWidget = nullptr;
}

void AAutopistasMainHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		ActiveHUDWidget = CreateWidget<UAutopistasHUDWidget>(GetWorld(), HUDWidgetClass);
		if (ActiveHUDWidget)
		{
			ActiveHUDWidget->AddToViewport(0);
			UE_LOG(LogAutopistas, Log, TEXT("UAutopistasHUDWidget anadido con exito al Viewport."));
		}
	}
}

void AAutopistasMainHUD::RefreshHUDMetrics(const FHUDSimulationData& Data)
{
	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->OnSimulationDataUpdated(Data);
	}
}
