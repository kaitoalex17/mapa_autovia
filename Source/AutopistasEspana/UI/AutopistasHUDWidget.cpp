#include "UI/AutopistasHUDWidget.h"
#include "Kismet/GameplayStatics.h"
#include "AutopistasEspana.h"

void UAutopistasHUDWidget::SelectBuildTool(EActiveBuildTool NewTool)
{
	CurrentTool = NewTool;
	UE_LOG(LogAutopistas, Log, TEXT("Herramienta activa seleccionada: %d"), static_cast<int32>(NewTool));
}

void UAutopistasHUDWidget::SetSimulationSpeed(float NewSpeed)
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), FMath::Clamp(NewSpeed, 0.0f, 4.0f));
	UE_LOG(LogAutopistas, Log, TEXT("Velocidad de simulacion ajustada a: %.1fx"), NewSpeed);
}
