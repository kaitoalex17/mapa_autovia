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

### 3.1. Catálogo Completo de Vías e Infraestructuras en el Dock:
1. `🛣️ Carretera 90 (Tecla 1)`: Vía convencional de 1 carril por sentido (límite $90\text{ km/h}$).
2. `🛤️ Autovía 2x2 (Tecla 2)`: Doble calzada con mediana y biondas bivalva metálicas (límite $120\text{ km/h}$).
3. `🛣️ Autopista 3x3 (Tecla 3)`: Vía de gran capacidad de 3 carriles por sentido para tramos de alta demanda tipo M-40 / A-4 (límite $120\text{ km/h}$).
4. `↗️ Enlace / Ramal (Tecla 4)`: Vía de 1 carril para incorporaciones y trenzados (límite $80\text{ km/h}$).
5. `💳 Peaje Troncal (Tecla 5)`: Estación de peaje con marquesina azul. Cada coche que cruza se detiene brevemente y abona **+3.50 €** que suma a tu presupuesto.
6. `🚆 Paso a Nivel ADIF (Tecla 6)`: Cruce viario con vía férrea. Cada 18 segundos se aproximan trenes de mercancías; las semibarreras rojas y blancas bajan, las luces parpadean y el tráfico se detiene en seco hasta que pasa el convoy.
7. `🚚 Grúa 112 (Tecla 7)`: Despacha una grúa de asistencia en carretera para retirar vehículos averiados, colisionados o bloqueados, despejando el carril.
8. `🚓 Control DGT (Tecla 8)`: Despliega una patrulla de la Guardia Civil y conos reflectantes para canalizar el tráfico a 40 km/h.
9. `🚜 Demoler (Tecla 9)`: Elimina tramos obsoletos o mal trazados.

---

## 🔍 Paso 4: Ficha de Inspección de Vehículo y Psicología en Vivo (`Tecla Q`)

Al seleccionar la herramienta **🔍 Inspeccionar** y hacer clic sobre cualquier vehículo en circulación, se abrirá la **Tarjeta de Inspección de la DGT**:
- **Ficha Técnica:** Modelo exacto (Turismo, Camión, Furgoneta, Guardia Civil), matrícula oficial y velocímetro en tiempo real frente al límite de la vía.
- **Termómetro de Estrés:** Barra de frustración del conductor ($0 - 100\%$) y tiempo acumulado en atasco.
- **Acciones Tácticas Directas del Jugador:**
  - `⚡ Sancionar DGT (200 €)`: Emite un boletín electrónico inmediato que calma al conductor rebelde e ingresa 200 € en tus arcas.
  - `🚚 Enviar Grúa 112`: Envía una grúa de plataforma para remolcar el vehículo si está provocando retención.

---

## ⌨️ Resumen Completo de Atajos de Teclado

| Tecla | Función |
| :--- | :--- |
| `Q` | Herramienta de Inspección de Vehículos y Conductores |
| `1` | Carretera Convencional (90 km/h) |
| `2` | Autovía 2x2 (120 km/h) |
| `3` | Autopista 3x3 de Gran Capacidad (120 km/h) |
| `4` | Enlace / Ramal de Incorporación (80 km/h) |
| `5` | Peaje Troncal (+3.50 € por vehículo) |
| `6` | Paso a Nivel ADIF con Barreras y Tren |
| `7` | Despachar Grúa de Asistencia 112 |
| `8` | Control Preventivo DGT / Guardia Civil |
| `9` | Demoler tramo vial |
| `W`, `A`, `S`, `D` | Desplazamiento panorámico de cámara |
| `C` | Centrar cámara en el nudo vial principal |
| `Espacio` | Pausar / Reanudar simulación |

---

¡Disfruta probando este prototipo jugable! Todos tus comentarios servirán para calibrar el comportamiento de las físicas y la interfaz en Unreal Engine 5.
