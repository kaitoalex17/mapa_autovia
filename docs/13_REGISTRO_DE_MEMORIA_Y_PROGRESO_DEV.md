# 13 - Registro de Memoria y Progreso de Desarrollo (Living Dev Log)
## Proyecto "Autopistas de España"

Este documento actúa como la **memoria técnica persistente** del proyecto para el desarrollador senior y los subagentes, asegurando que ningún detalle o subsistema se pierda a lo largo del ciclo de vida del juego.

---

## 📊 Matriz de Estado de Módulos (Checklist Técnico)

| Módulo / Subsistema | Clases C++ Principales | Estado | Verificación |
| :--- | :--- | :--- | :--- |
| **Cámara Cenital 2.5D** | `AAutopistasTopDownCamera` | ✅ Completado | Paneo suave, zoom adaptativo 12m-650m, límites $8\times 8\text{ km}$. |
| **Controlador y Modos** | `AAutopistasPlayerController`, `AAutopistasGameModeBase` | ✅ Completado | Cursor visible, proyección $Z=0$, HUD vinculado. |
| **Generador de Carreteras** | `URoadSplineComponent`, `ARoadSegmentActor`, `RoadTypes.h` | ✅ Completado | Secciones Norma 3.1-IC, viaductos $Z>3\text{m}$, túneles $Z<-3\text{m}$. |
| **Red Vial en Memoria** | `URoadNetworkSubsystem` | ✅ Completado | Registro automático, demolición y cálculo de km totales. |
| **Motor de Tráfico** | `ATrafficVehicleAgent`, `UTrafficSimulationSubsystem`, `TrafficTypes.h` | ✅ Completado | IDM para aceleración/frenado, MOBIL (circular por la derecha). |
| **Físicas y Siniestros** | `ATrafficVehicleAgent::TriggerAccidentCollision` | ✅ Completado | Choques físicos, desprendimiento de piezas y bloqueo de calzada. |
| **Psicología Conductor** | `ECondicionPsicologica`, `EDriverMood`, `ATrafficVehicleAgent` | ✅ Perfeccionado | WaitTimeInJam, IDM adaptativo, ráfagas de luces, claxon, tailgating crítico (0.2s) y riesgo colisión x5 en Furia al Volante (Doc 15). |
| **Impacto Obras Temporales** | `ARoadSegmentActor`, `RoadTypes.h`, `UEconomySubsystem` | ✅ Implementado | Toggle bEnableConstructionImpact (Modo Realista vs Sandbox), 3 fases constructivas (Desmonte 40km/h, Asfaltado 40km/h, Pintura 60km/h), balizamiento ISM con conos SM_Cono_Obra_75 y estrechamiento de calzada (Doc 15). |
| **Mundo Procedural** | `AProceduralWorldGenerator`, `WorldGenTypes.h` | ✅ Completado | Relieve de Meseta/Cantábrico/Levante, ciudades, polígonos y matriz O-D. |
| **Interfaz Gráfica (HUD)** | `AAutopistasMainHUD`, `UAutopistasHUDWidget`, `UITypes.h` | ✅ Completado | Barra superior de finanzas, dock inferior y alertas Toast. |
| **Economía y Fondos (€)** | `UEconomySubsystem` | ✅ Completado | Balance en Euros, costes por metro, peajes y multas. |
| **Sistema Guardar/Cargar**| `UAutopistasSaveGame`, `USaveLoadSubsystem` | ✅ Completado | Slots, metadatos, serialización binaria y QuickSave (F5/F9). |
| **Localización Bilingüe** | `ULocalizationSubsystem`, `ELanguageCode` | ✅ Perfeccionado | Cambio en caliente (ES/EN), delegado reactivo `OnLanguageChanged`, persistencia `GConfig`, diccionarios completos (Doc 16). |
| **Modelos 3D de Vehículos**| 23 archivos `.obj` y `.mtl` en `Content/Meshes/Vehicles/` | ✅ Completado | 7 arquetipos oficiales + piezas desprendibles a escala $1\text{ UU} = 1\text{ cm}$. |
| **Props Viales Españoles**| 9 archivos `.obj` y `.mtl` en `Content/Meshes/RoadProps/` | ✅ Completado | Biondas SPM (UNE-EN 1317), New Jersey, radar DGT, pórticos y conos. |
| **Operativos DGT y Pegasus** | `UDGTControlSubsystem`, `APegasusHelicopterActor`, `DGTTypes.h` | ✅ Completado | Cota cenital 300m, patrulla, cinemómetro láser, >150 km/h, acoso trasero, controles calzada, balizamiento conos y reducción siniestralidad. |
| **Arte de la UI** | `MainMenu_DGT_Background.jpg` en `Content/Textures/UI/` | ✅ Completado | Plano del Centro de Control de Tráfico DGT en alta resolución. |
| **Repositorio Git Remoto**| GitHub `kaitoalex17/mapa_autovia.git` | ✅ Sincronizado | Rama `main` actualizada con todos los módulos. |

---

## 📅 Historial de Decisiones de Diseño Clave
1. **Escala 1:1 Local con Compresión Territorial:** Garantiza que las medidas de calzadas ($3.5\text{ m}$) y vehículos ($4.2\text{ m}$) sean idénticas a la realidad sin exigir mapas vacíos imposibles de renderizar.
2. **Exclusión Temporal de Audio:** Todo el esfuerzo se centra en la jugabilidad, mallas, físicas y tráfico visual; el diseño sonoro se integrará en etapas posteriores de pulido comercial.
3. **Internacionalización Nativa:** Todas las cadenas de texto del juego cuentan con clave unívoca y doble diccionario (ES / EN).
