#include "Core/AutopistasGameModeBase.h"
#include "Camera/AutopistasTopDownCamera.h"
#include "Core/AutopistasPlayerController.h"
#include "UI/AutopistasMainHUD.h"

AAutopistasGameModeBase::AAutopistasGameModeBase()
{
	DefaultPawnClass = AAutopistasTopDownCamera::StaticClass();
	PlayerControllerClass = AAutopistasPlayerController::StaticClass();
	HUDClass = AAutopistasMainHUD::StaticClass();
}
