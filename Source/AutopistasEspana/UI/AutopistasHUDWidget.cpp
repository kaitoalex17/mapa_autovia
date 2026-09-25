#include "UI/AutopistasHUDWidget.h"
#include "Kismet/GameplayStatics.h"
#include "AutopistasEspana.h"

void UAutopistasHUDWidget::SelectBuildTool(EActiveBuildTool NewTool)
{
	CurrentTool = NewTool;
	UE_LOG(LogAutopistas, Log, TEXT("Herramienta activa seleccionada: %d"), static_cast<int32>(NewTool));
}

void UAutopistasHUDWidget::SelectLanes(ENumLanesBuild NewLanes)
{
	CurrentLanes = NewLanes;
	UE_LOG(LogAutopistas, Log, TEXT("Dock Carriles seleccionado: %d"), static_cast<int32>(NewLanes));
}

void UAutopistasHUDWidget::SelectElevation(ERoadElevationStep NewElevation)
{
	CurrentElevation = NewElevation;
	UE_LOG(LogAutopistas, Log, TEXT("Dock Cota/Rasante seleccionada: %d"), static_cast<int32>(NewElevation));
}

void UAutopistasHUDWidget::SelectDrawMode(ERoadDrawMode NewMode)
{
	CurrentDrawMode = NewMode;
	UE_LOG(LogAutopistas, Log, TEXT("Dock Modo Trazado seleccionado: %d"), static_cast<int32>(NewMode));
}

void UAutopistasHUDWidget::SelectDirection(ETrafficDirectionMode NewDirection)
{
	CurrentDirection = NewDirection;
	UE_LOG(LogAutopistas, Log, TEXT("Dock Sentido seleccionado: %d"), static_cast<int32>(NewDirection));
}

void UAutopistasHUDWidget::SetSimulationSpeed(float NewSpeed)
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), FMath::Clamp(NewSpeed, 0.0f, 4.0f));
	UE_LOG(LogAutopistas, Log, TEXT("Velocidad de simulacion ajustada a: %.1fx"), NewSpeed);
}

