#include "SaveSystem/SaveLoadSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "AutopistasEspana.h"

bool USaveLoadSubsystem::SaveGame(const FString& SlotName)
{
	UAutopistasSaveGame* SaveGameObject = Cast<UAutopistasSaveGame>(UGameplayStatics::CreateSaveGameObject(UAutopistasSaveGame::StaticClass()));
	if (!SaveGameObject)
	{
		return false;
	}

	SaveGameObject->SaveSlotName = SlotName;
	SaveGameObject->Timestamp = FDateTime::Now();

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);
	if (bSuccess)
	{
		UE_LOG(LogAutopistas, Log, TEXT("Partida guardada con exito en ranura: %s"), *SlotName);
	}
	else
	{
		UE_LOG(LogAutopistas, Error, TEXT("Error al guardar la partida en ranura: %s"), *SlotName);
	}

	return bSuccess;
}

UAutopistasSaveGame* USaveLoadSubsystem::LoadGame(const FString& SlotName)
{
	if (!DoesSaveExist(SlotName))
	{
		UE_LOG(LogAutopistas, Warning, TEXT("No se encontro el archivo de guardado: %s"), *SlotName);
		return nullptr;
	}

	UAutopistasSaveGame* LoadedGame = Cast<UAutopistasSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (LoadedGame)
	{
		UE_LOG(LogAutopistas, Log, TEXT("Partida cargada con exito desde ranura: %s"), *SlotName);
	}

	return LoadedGame;
}

bool USaveLoadSubsystem::QuickSave()
{
	return SaveGame(QuickSaveSlotName);
}

bool USaveLoadSubsystem::QuickLoad()
{
	return LoadGame(QuickSaveSlotName) != nullptr;
}

bool USaveLoadSubsystem::DoesSaveExist(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}
