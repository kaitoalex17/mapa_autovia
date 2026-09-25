#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LocalizationSubsystem.generated.h"

/** 
 * Idiomas oficiales soportados en Autopistas de Espana 
 */
UENUM(BlueprintType)
enum class ELanguageCode : uint8
{
	Spanish    UMETA(DisplayName = "Español (ES)"),
	English    UMETA(DisplayName = "English (EN)")
};

/** Alias de retrocompatibilidad con el codigo existente */
using EGameLanguage = ELanguageCode;

/** Delegado dynamic multicast para refrescar widgets y paneles PMV en tiempo real */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChangedDelegate, ELanguageCode, NewLanguage);

/**
 * Subsistema Global de Localizacion Bilingue en Tiempo Real (ES / EN).
 * Permite alternar instantaneamente entre Espanol e Ingles sin reiniciar la partida,
 * notificando a todos los widgets de la interfaz, paneles de mensaje variable (PMV),
 * menus de construccion, sistemas de emergencias 112 y alertas DGT.
 */
UCLASS()
class AUTOPISTASESPANA_API ULocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Cambia el idioma activo de forma instantanea y notifica a los suscriptores */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetLanguage(ELanguageCode NewLanguage);

	/** Alias de retrocompatibilidad */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetCurrentLanguage(ELanguageCode NewLanguage) { SetLanguage(NewLanguage); }

	/** Cambia el idioma especificando codigo ISO ("ES", "EN", "es", "en") */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	bool SetLanguageByCode(const FString& InLanguageCode);

	/** Devuelve el idioma activo */
	UFUNCTION(BlueprintPure, Category = "Localization")
	ELanguageCode GetLanguage() const { return CurrentLanguage; }

	/** Alias de retrocompatibilidad */
	UFUNCTION(BlueprintPure, Category = "Localization")
	ELanguageCode GetCurrentLanguage() const { return CurrentLanguage; }

	/** Devuelve el codigo de dos letras del idioma activo ("ES" o "EN") */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FString GetLanguageCodeString() const;

	/** Devuelve el nombre visible en UI del idioma activo ("Español (ES)" o "English (EN)") */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FString GetLanguageDisplayName() const;

	/** Comprueba si el idioma indicado es el actualmente activo */
	UFUNCTION(BlueprintPure, Category = "Localization")
	bool IsLanguage(ELanguageCode Language) const { return CurrentLanguage == Language; }

	/** Obtiene el texto traducido para una clave en el idioma activo */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLocalizedString(const FString& Key) const;

	/** Alias de conveniencia para GetLocalizedString */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLocalizedText(const FString& Key) const { return GetLocalizedString(Key); }

	/** Obtiene el texto traducido con un valor por defecto si la clave no existe */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLocalizedStringWithFallback(const FString& Key, const FString& DefaultFallback) const;

	/** Obtiene el texto para un idioma especifico sin alterar el idioma activo del subsistema */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLocalizedTextForLanguage(const FString& Key, ELanguageCode TargetLanguage) const;

	/** Formatea una cadena traducida sustituyendo tokens con nombre {Token} */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	FText GetFormattedLocalizedString(const FString& Key, const TMap<FString, FString>& Arguments) const;

	/** Formatea una cadena traducida sustituyendo un unico parametro {0} */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	FText GetFormattedLocalizedString1P(const FString& Key, const FString& Param0) const;

	/** Formatea una cadena traducida sustituyendo dos parametros {0} y {1} */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	FText GetFormattedLocalizedString2P(const FString& Key, const FString& Param0, const FString& Param1) const;

	/** Verifica si una clave existe en los diccionarios */
	UFUNCTION(BlueprintPure, Category = "Localization")
	bool HasTranslationKey(const FString& Key) const;

	/** Devuelve el numero de claves registradas para el idioma dado */
	UFUNCTION(BlueprintPure, Category = "Localization")
	int32 GetTranslationCount(ELanguageCode Language) const;

	/** Devuelve la lista completa de claves de texto registradas */
	UFUNCTION(BlueprintPure, Category = "Localization")
	TArray<FString> GetAllTranslationKeys() const;

	/** Registra o sobrescribe dinamicamente una cadena en caliente */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void RegisterCustomString(const FString& Key, const FString& SpanishText, const FString& EnglishText);

	/** Helper estatico C++ para obtener el subsistema desde cualquier UObject en el World */
	static ULocalizationSubsystem* Get(const UObject* WorldContextObject);

	/** Helper estatico Blueprint para consultar textos desde cualquier Widget o Nodo */
	UFUNCTION(BlueprintPure, Category = "Localization", meta = (WorldContext = "WorldContextObject"))
	static FText GetGameLocalizedString(const UObject* WorldContextObject, const FString& Key);

	/** Delegado disparado inmediatamente al cambiar de idioma */
	UPROPERTY(BlueprintAssignable, Category = "Localization")
	FOnLanguageChangedDelegate OnLanguageChanged;

private:
	UPROPERTY()
	ELanguageCode CurrentLanguage = ELanguageCode::Spanish;

	/** Diccionario de cadenas en Espanol (ES) */
	TMap<FString, FString> SpanishDictionary;

	/** Diccionario de cadenas en Ingles (EN) */
	TMap<FString, FString> EnglishDictionary;

	/** Carga e inicializa el vocabulario bilingue integral */
	void LoadDictionaries();

	/** Carga la preferencia de idioma persistente desde GGameUserSettingsIni */
	void LoadLanguagePreference();

	/** Guarda la preferencia de idioma en GGameUserSettingsIni */
	void SaveLanguagePreference();
};
