#include "Core/AutopistasPlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AAutopistasPlayerController::AAutopistasPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AAutopistasPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Configurar modo de entrada Juego + UI para permitir clicks en HUD y en el mundo simultaneamente
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void AAutopistasPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindAction(TEXT("PrimaryAction"), IE_Pressed, this, &AAutopistasPlayerController::OnPrimaryClickPressed);
		InputComponent->BindAction(TEXT("PrimaryAction"), IE_Released, this, &AAutopistasPlayerController::OnPrimaryClickReleased);
		InputComponent->BindAction(TEXT("SecondaryAction"), IE_Pressed, this, &AAutopistasPlayerController::OnSecondaryClickPressed);
	}
}

bool AAutopistasPlayerController::GetMouseWorldPosition(FVector& OutWorldPosition)
{
	FHitResult HitResult;
	// Trazar linea desde la posicion del cursor en pantalla hacia el mundo
	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		OutWorldPosition = HitResult.ImpactPoint;
		return true;
	}

	// Si no impacta contra nada (ej. vacio sin colision), proyectar matematicamente sobre el plano Z = 0
	FVector WorldOrigin, WorldDirection;
	if (DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		if (!FMath::IsNearlyZero(WorldDirection.Z))
		{
			float T = -WorldOrigin.Z / WorldDirection.Z;
			if (T >= 0.0f)
			{
				OutWorldPosition = WorldOrigin + WorldDirection * T;
				return true;
			}
		}
	}

	return false;
}

void AAutopistasPlayerController::OnPrimaryClickPressed()
{
	FVector ClickLocation;
	if (GetMouseWorldPosition(ClickLocation))
	{
		// Punto de anclaje para trazado vial (se conectara con el RoadNetworkSubsystem)
	}
}

void AAutopistasPlayerController::OnPrimaryClickReleased()
{
	// Finalizar tramo vial trazado
}

void AAutopistasPlayerController::OnSecondaryClickPressed()
{
	// Cancelar herramienta activa o modo demolicion
}
