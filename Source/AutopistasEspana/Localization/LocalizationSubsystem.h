#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LocalizationSubsystem.generated.h"

/** Idiomas soportados en el juego */
UENUM(BlueprintType)
enum class EGameLanguage : uint8
{
	Spanish    UMETA(DisplayName = "Español (ES)"),
	English    UMETA(DisplayName = "English (EN)")
};

/**
 * Subsistema Global de Localizacion Bilingue (ES / EN).
 * Permite cambiar de idioma en tiempo real sin reiniciar el juego y traduce dinamicamente
 * textos de la interfaz, alertas del 112, paneles PMV, descripciones tecnicas y tutoriales.
 */
UCLASS()
class AUTOPISTASESPANA_API ULocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Cambiar idioma activo del juego
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetCurrentLanguage(EGameLanguage NewLanguage);

	// Obtener idioma activo
	UFUNCTION(BlueprintPure, Category = "Localization")
	EGameLanguage GetCurrentLanguage() const { return CurrentLanguage; }

	// Obtener texto traducido por clave
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLocalizedString(const FString& Key) const;

	// Delegate para notificar a la UI cuando cambie el idioma
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChangedDelegate, EGameLanguage, NewLanguage);
	UPROPERTY(BlueprintAssignable, Category = "Localization")
	FOnLanguageChangedDelegate OnLanguageChanged;

private:
	UPROPERTY()
	EGameLanguage CurrentLanguage = EGameLanguage::Spanish;

	// Diccionarios en memoria
	TMap<FString, FString> SpanishDictionary;
	TMap<FString, FString> EnglishDictionary;

	void LoadDictionaries();
};
