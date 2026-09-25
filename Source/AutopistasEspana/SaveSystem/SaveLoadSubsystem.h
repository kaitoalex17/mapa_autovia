#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSystem/AutopistasSaveGame.h"
#include "SaveLoadSubsystem.generated.h"

/**
 * Subsistema Global de Guardado y Carga de Partidas (Save / Load).
 * Controla el guardado rapido (F5), carga rapida (F9), autoguardado rotativo y serializacion binaria.
 */
UCLASS()
class AUTOPISTASESPANA_API USaveLoadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Guardar partida en ranura especifica
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool SaveGame(const FString& SlotName);

	// Cargar partida desde ranura especifica
	UFUNCTION(BlueprintCallable, Category = "Save System")
	UAutopistasSaveGame* LoadGame(const FString& SlotName);

	// Guardado Rapido (F5)
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool QuickSave();

	// Carga Rapida (F9)
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool QuickLoad();

	// Comprobar si existe una ranura
	UFUNCTION(BlueprintPure, Category = "Save System")
	bool DoesSaveExist(const FString& SlotName) const;

	const FString QuickSaveSlotName = TEXT("QuickSave_Slot");
};
