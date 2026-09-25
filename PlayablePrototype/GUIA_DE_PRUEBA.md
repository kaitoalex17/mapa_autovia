# 🛣️ Guía de Prueba Interactiva: Prototipo Jugable "Autopistas de España"
## Centro de Gestión de Tráfico y Simulación Vial 2.5D

¡Bienvenido al primer prototipo jugable autónomo de **Autopistas de España**! Este entorno ha sido diseñado para permitirte experimentar de forma directa las mecánicas centrales del juego (físicas de tráfico IDM/MOBIL, trazado de carreteras por splines, psicología de los conductores y vigilancia aérea con el helicóptero radar Pegasus de la DGT) antes de su compilación final en Unreal Engine 5.

---

## 🚀 Paso 1: Cómo Ejecutar el Prototipo

Tienes dos formas inmediatas de abrirlo:
1. **Acceso directo (Recomendado):** Haz doble clic en el archivo [`ejecutar_prototipo.bat`](file:///C:/app/game_autopista/ejecutar_prototipo.bat) situado en la raíz del proyecto (`C:\app\game_autopista\ejecutar_prototipo.bat`).
2. **Navegador web:** Abre directamente [`PlayablePrototype/index.html`](file:///C:/app/game_autopista/PlayablePrototype/index.html) con Google Chrome, Microsoft Edge, Firefox u Opera.

> [!NOTE]
> No requiere instalación de Node.js, servidores locales ni extensiones. Es 100% autónomo y utiliza HTML5 Canvas acelerado por hardware y la Web Audio API para los efectos sonoros.

---

## 🎮 Paso 2: Controles de Cámara y Navegación

El simulador cuenta con una cámara cenital/isométrica fluida con amortiguación inercial:

| Acción | Control |
| :--- | :--- |
| **Desplazar cámara** | Teclas `W`, `A`, `S`, `D` o **Flechas de dirección**. |
| **Arrastre táctico** | Mantén pulsado el **Botón Central** (rueda) o **Botón Derecho** del ratón y arrastra. |
| **Zoom continuo** | Gira la **Rueda del Ratón** (rango suave de $0.35\times$ a $2.8\times$). |
| **Centrar en corredor** | Pulsa la tecla `C` o haz clic en el botón flotante `🎯` en la esquina inferior derecha. |
| **Pausa / Velocidad** | Pulsa `Espacio` para pausar, o los botones `1x`, `2x`, `4x` en la barra superior. |

---

## 🏗️ Paso 3: Construcción de Carreteras con Splines

Puedes ampliar la red vial trazando nuevas vías que se integran dinámicamente:

1. **Selecciona un tipo de vía en el Dock Inferior:**
   - `🛣️ Carretera 90 (Tecla 1)`: Vía convencional de 1 carril por sentido (límite $90\text{ km/h}$).
   - `🛤️ Autovía 2x2 (Tecla 2)`: Doble calzada separada por mediana con guardarraíles bionda de acero (límite $120\text{ km/h}$).
   - `↗️ Enlace / Ramal (Tecla 3)`: Vía de 1 carril para incorporaciones o salidas (límite $80\text{ km/h}$).
2. **Trazar en el mapa:** Haz **clic izquierdo** en el punto inicial, **arrastra** hacia el destino deseado y **suelta**. El trazador calculará automáticamente una curva Bézier suave, respetando la continuidad del trazado.
3. **Mecánica de Obras en Vivo (5 segundos):**
   - Con el toggle `Obras en Vivo (5s)` activado en el panel izquierdo, el tramo recién construido se baliza con **conos reflectantes de obra**, cartel indicador y obreros.
   - Durante esos 5 segundos el tramo no admite tráfico hasta que concluye el asfaltado. Una vez finalizado, suena un aviso acústico y queda inaugurado.

---

## 🚗 Paso 4: Simulación de Tráfico, Flota y Físicas IDM/MOBIL

El simulador implementa los modelos científicos oficiales de tráfico:
- **IDM (Intelligent Driver Model):** Los vehículos aceleran suavemente hacia su velocidad deseada, guardan la distancia de seguridad prescrita ($s_0$ y $T$) y frenan progresivamente ante obstáculos.
- **MOBIL (Lane Changing):** En la Autovía 2x2, los coches evalúan si circular por la izquierda para adelantar a un vehículo pesado y **vuelven inmediatamente a la derecha** cuando está libre (norma de la DGT española).
- **Tipología de Vehículos:**
  - 🚗 **Turismos compactos:** Ágiles, con velocidades deseadas entre $110$ y $135\text{ km/h}$.
  - 🚛 **Camiones articulados (Tráilers):** Gran masa, aceleración pausada y velocidad máxima restringida a $90\text{ km/h}$.
  - 🚐 **Furgonetas de reparto:** Circulación a $105\text{ km/h}$.
  - 🚓 **Patrullas de la Guardia Civil:** Decoradas con la librea oficial verde y blanca, con rotativos azules estroboscópicos en el techo.

---

## 🚨 Paso 5: Provocar Atascos y Observar la Psicología del Conductor

Para verificar cómo reaccionan los conductores ante imprevistos:

1. Haz clic en el botón rojo **`🚨 Provocar Retención / Conos`** en el panel lateral izquierdo.
2. Se colocará un obstáculo de mantenimiento en uno de los carriles principales de la autovía.
3. **Efecto Embudo en Cadena:**
   - Los coches que se aproximan clavan frenos, provocando la onda de frenado en acordeón típica de los atascos fantasma.
   - Observa cómo sube el indicador de **Congestión Global** en el HUD superior.
4. **Evolución del Humor y Furia al Volante (*Road Rage*):**
   - Cada conductor tiene un medidor individual de frustración ($0 - 100\%$).
   - `🙂 Zen (0 - 30%)`: Circulación tranquila.
   - `😐 Impaciente (30 - 70%)`: El conductor pega el coche al parachoques delantero y busca huecos ansiosamente.
   - `😡 Furia al Volante (> 70%)`: Se activa el icono de enfado, el conductor **toca la bocina (bocinazo audible sintetizado)**, da ráfagas y realiza maniobras temerarias.
5. Puedes pulsar de nuevo el botón para retirar los conos y ver cómo la vía recupera la fluidez.

---

## 🚁 Paso 6: Vigilancia Aérea con el Helicóptero Radar Pegasus

En el cielo de la autovía patrulla el helicóptero radar **Pegasus MX-15** de la DGT:

1. **Haz de Radar Láser:** Proyecta un cono cian sobre la calzada con anillos de pulso concéntrico.
2. **Detección Automática de Infractores:**
   - Escanea la velocidad de todos los vehículos que atraviesan el cono.
   - Si un turismo circula a más de $120\text{ km/h}$ o un conductor furioso realiza acoso pegado al parachoques, el radar fija la mira (**RADAR LOCK en rojo**).
3. **Emisión de Multa Electrónica:**
   - Tras 1.2 segundos de confirmación, emite un sonido de caja registradora / captura fotográfica.
   - Aparece un texto flotante verde sobre el coche infractor (`+300 €` o `+600 €`).
   - El dinero se ingresa inmediatamente en tu **Presupuesto Municipal (€)** y se anota en el panel derecho `DGT PEGASUS MX-15`.

---

## 🌦️ Paso 7: Toggles y Modificadores en Tiempo Real

En el panel lateral izquierdo puedes activar o desactivar los subsistemas al vuelo:
- **`Obras en Vivo (5s)`**: Alterna entre construcción instantánea o realista con conos.
- **`Psicología y Furia`**: Activa o silencia el sistema de estrés, emoticonos y bocinas de los conductores.
- **`Patrulla Pegasus`**: Envía al helicóptero a la base o lo mantiene patrullando la red.
- **`Clima DANA Lluvia`**: Despliega una tormenta con cortina de lluvia diagonal, oscurece el asfalto simulando calzada mojada y **reduce el coeficiente de adherencia de los frenos a un 65%**, aumentando el riesgo de alcances traseros.

---

## 🌐 Paso 8: Selector de Idioma (Español / English)

En la barra superior derecha encontrarás el botón **`🇪🇸 ES`**:
- Al hacer clic, cambiará instantáneamente a **`🇬🇧 EN`**.
- Todos los textos, botones, avisos 112, registros de multas de Pegasus y menús de ayuda se traducirán en tiempo real sin recargar la página ni perder el estado de la simulación.

---

## ⌨️ Resumen de Atajos de Teclado

| Tecla | Función |
| :--- | :--- |
| `Q` | Herramienta de Inspección de Vehículos |
| `1` | Carretera Convencional (90 km/h) |
| `2` | Autovía 2x2 (120 km/h) |
| `3` | Enlace / Ramal (80 km/h) |
| `4` | Control Estático de la Guardia Civil |
| `5` | Demoler tramo vial |
| `W`, `A`, `S`, `D` | Desplazamiento de cámara |
| `C` | Centrar cámara en el nudo vial principal |
| `Espacio` | Pausar / Reanudar simulación |

---

¡Disfruta probando este primer prototipo jugable! Todos tus comentarios servirán para calibrar el comportamiento de las físicas y la interfaz en Unreal Engine 5.
