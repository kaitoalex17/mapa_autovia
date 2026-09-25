#include "Camera/AutopistasTopDownCamera.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"

AAutopistasTopDownCamera::AAutopistasTopDownCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false; // Sin colision de camara para vista cenital limpia
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	// Inclinacion a 87.5 grados: Casi totalmente cenital (2D feel), pero con suficiente angulo para apreciar viaductos y perfiles
	SpringArm->SetRelativeRotation(FRotator(-87.5f, 0.0f, 0.0f));
	SpringArm->TargetArmLength = 8000.0f; // Zoom medio inicial
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 12.0f;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false;

	TargetArmLength = SpringArm->TargetArmLength;
}

void AAutopistasTopDownCamera::BeginPlay()
{
	Super::BeginPlay();
	TargetArmLength = SpringArm->TargetArmLength;
}

void AAutopistasTopDownCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Suavizado de Zoom
	if (!FMath::IsNearlyEqual(SpringArm->TargetArmLength, TargetArmLength, 1.0f))
	{
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetArmLength, DeltaTime, ZoomInterpSpeed);
	}

	// 2. Desplazamiento proporcional a la altura de la camara
	if (!TargetMovementDirection.IsNearlyZero())
	{
		// A mayor altura, nos movemos mas rapido para mantener la sensacion de velocidad en el mapa
		const float ZoomFactor = FMath::GetMappedRangeValueClamped(
			FVector2D(MinZoomArmLength, MaxZoomArmLength),
			FVector2D(1.0f, 8.0f),
			SpringArm->TargetArmLength
		);

		const float CurrentSpeed = MoveSpeedBase * ZoomFactor * (bIsFastMoveActive ? MoveSpeedFastMultiplier : 1.0f);
		FVector DeltaLocation = TargetMovementDirection.GetSafeNormal() * CurrentSpeed * DeltaTime;

		FVector NewLocation = GetActorLocation() + DeltaLocation;

		// Confinar dentro de los limites del mapa
		NewLocation.X = FMath::Clamp(NewLocation.X, WorldMinBounds.X, WorldMaxBounds.X);
		NewLocation.Y = FMath::Clamp(NewLocation.Y, WorldMinBounds.Y, WorldMaxBounds.Y);

		SetActorLocation(NewLocation);
	}

	// Resetear vector de direccion en cada tick tras consumir
	TargetMovementDirection = FVector::ZeroVector;
}

void AAutopistasTopDownCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AAutopistasTopDownCamera::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AAutopistasTopDownCamera::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Zoom"), this, &AAutopistasTopDownCamera::Zoom);
}

void AAutopistasTopDownCamera::MoveForward(float Value)
{
	if (FMath::Abs(Value) > 0.01f)
	{
		TargetMovementDirection.X += Value;
	}
}

void AAutopistasTopDownCamera::MoveRight(float Value)
{
	if (FMath::Abs(Value) > 0.01f)
	{
		TargetMovementDirection.Y += Value;
	}
}

void AAutopistasTopDownCamera::Zoom(float Value)
{
	if (FMath::Abs(Value) > 0.01f)
	{
		// Value negativo aleja la camara, positivo acerca
		TargetArmLength = FMath::Clamp(TargetArmLength - (Value * ZoomSpeed), MinZoomArmLength, MaxZoomArmLength);
	}
}

void AAutopistasTopDownCamera::SetFastMove(bool bFast)
{
	bIsFastMoveActive = bFast;
}
