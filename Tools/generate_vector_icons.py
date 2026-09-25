import os

ICONS_DIR = r"c:\app\game_autopista\Content\Textures\UI\Icons"
os.makedirs(ICONS_DIR, exist_ok=True)

icons = {
    "icon_cursor_inspect.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none" stroke="currentColor">
  <!-- Precision Reticle Telemetry -->
  <circle cx="32" cy="32" r="26" stroke="#00c0f3" stroke-width="2" stroke-dasharray="8 4" opacity="0.4"/>
  <circle cx="32" cy="32" r="16" stroke="#00c0f3" stroke-width="2.5"/>
  <circle cx="32" cy="32" r="4" fill="#00c0f3"/>
  <line x1="32" y1="4" x2="32" y2="12" stroke="#00c0f3" stroke-width="3" stroke-linecap="round"/>
  <line x1="32" y1="52" x2="32" y2="60" stroke="#00c0f3" stroke-width="3" stroke-linecap="round"/>
  <line x1="4" y1="32" x2="12" y2="32" stroke="#00c0f3" stroke-width="3" stroke-linecap="round"/>
  <line x1="52" y1="32" x2="60" y2="32" stroke="#00c0f3" stroke-width="3" stroke-linecap="round"/>
  <!-- Corner brackets -->
  <path d="M14 20 L14 14 L20 14" stroke="#38bdf8" stroke-width="2.5" fill="none"/>
  <path d="M50 20 L50 14 L44 14" stroke="#38bdf8" stroke-width="2.5" fill="none"/>
  <path d="M14 44 L14 50 L20 50" stroke="#38bdf8" stroke-width="2.5" fill="none"/>
  <path d="M50 44 L50 50 L44 50" stroke="#38bdf8" stroke-width="2.5" fill="none"/>
</svg>''',

    "icon_road_convencional.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Calzada Convencional 90 km/h -->
  <rect x="14" y="6" width="36" height="52" rx="4" fill="#1e293b" stroke="#64748b" stroke-width="2"/>
  <!-- Lineas blancas de borde -->
  <line x1="18" y1="6" x2="18" y2="58" stroke="#f8fafc" stroke-width="2"/>
  <line x1="46" y1="6" x2="46" y2="58" stroke="#f8fafc" stroke-width="2"/>
  <!-- Eje central discontinuo -->
  <line x1="32" y1="10" x2="32" y2="20" stroke="#f8fafc" stroke-width="2.5" stroke-linecap="round"/>
  <line x1="32" y1="28" x2="32" y2="38" stroke="#f8fafc" stroke-width="2.5" stroke-linecap="round"/>
  <line x1="32" y1="46" x2="32" y2="56" stroke="#f8fafc" stroke-width="2.5" stroke-linecap="round"/>
  <!-- Flechas bidireccionales -->
  <path d="M25 40 L25 24 M22 28 L25 24 L28 28" stroke="#38bdf8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
  <path d="M39 24 L39 40 M36 36 L39 40 L42 36" stroke="#f59e0b" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
</svg>''',

    "icon_autovia_2x2.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Calzada Doble 2x2 con Mediana -->
  <rect x="8" y="6" width="48" height="52" rx="4" fill="#1e293b" stroke="#475569" stroke-width="2"/>
  <!-- Mediana central y bionda -->
  <rect x="29" y="6" width="6" height="52" fill="#0f172a"/>
  <line x1="32" y1="6" x2="32" y2="58" stroke="#94a3b8" stroke-width="2"/>
  <!-- Lineas continuas de arcen -->
  <line x1="12" y1="6" x2="12" y2="58" stroke="#f8fafc" stroke-width="2"/>
  <line x1="52" y1="6" x2="52" y2="58" stroke="#f8fafc" stroke-width="2"/>
  <!-- Separadores de carril discontinuos -->
  <line x1="20" y1="12" x2="20" y2="22" stroke="#94a3b8" stroke-width="1.8" stroke-linecap="round"/>
  <line x1="20" y1="30" x2="20" y2="40" stroke="#94a3b8" stroke-width="1.8" stroke-linecap="round"/>
  <line x1="20" y1="48" x2="20" y2="56" stroke="#94a3b8" stroke-width="1.8" stroke-linecap="round"/>
  <line x1="44" y1="12" x2="44" y2="22" stroke="#94a3b8" stroke-width="1.8" stroke-linecap="round"/>
  <line x1="44" y1="30" x2="44" y2="40" stroke="#94a3b8" stroke-width="1.8" stroke-linecap="round"/>
  <line x1="44" y1="48" x2="44" y2="56" stroke="#94a3b8" stroke-width="1.8" stroke-linecap="round"/>
</svg>''',

    "icon_autovia_3x3.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Autopista de Gran Capacidad 3x3 -->
  <rect x="4" y="6" width="56" height="52" rx="4" fill="#0f172a" stroke="#334155" stroke-width="2"/>
  <!-- Mediana central ancha -->
  <rect x="28" y="6" width="8" height="52" fill="#020617"/>
  <line x1="31" y1="6" x2="31" y2="58" stroke="#64748b" stroke-width="1.5"/>
  <line x1="33" y1="6" x2="33" y2="58" stroke="#64748b" stroke-width="1.5"/>
  <!-- Discontinuas calzada izquierda (3 carriles) -->
  <line x1="12" y1="10" x2="12" y2="22" stroke="#cbd5e1" stroke-width="1.5"/>
  <line x1="12" y1="32" x2="12" y2="44" stroke="#cbd5e1" stroke-width="1.5"/>
  <line x1="20" y1="18" x2="20" y2="30" stroke="#cbd5e1" stroke-width="1.5"/>
  <line x1="20" y1="40" x2="20" y2="52" stroke="#cbd5e1" stroke-width="1.5"/>
  <!-- Discontinuas calzada derecha (3 carriles) -->
  <line x1="44" y1="10" x2="44" y2="22" stroke="#cbd5e1" stroke-width="1.5"/>
  <line x1="44" y1="32" x2="44" y2="44" stroke="#cbd5e1" stroke-width="1.5"/>
  <line x1="52" y1="18" x2="52" y2="30" stroke="#cbd5e1" stroke-width="1.5"/>
  <line x1="52" y1="40" x2="52" y2="52" stroke="#cbd5e1" stroke-width="1.5"/>
  <!-- Distintivo 3x3 -->
  <text x="32" y="36" fill="#38bdf8" font-size="9" font-family="monospace" font-weight="900" text-anchor="middle">3x3</text>
</svg>''',

    "icon_enlace_ramal.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Ramal de Enlace e Incorporacion Tangencial -->
  <path d="M16 6 L16 58" stroke="#475569" stroke-width="12" stroke-linecap="round"/>
  <path d="M16 6 L16 58" stroke="#0f172a" stroke-width="8"/>
  <path d="M50 56 C46 36, 32 28, 16 22" stroke="#475569" stroke-width="10" stroke-linecap="round"/>
  <path d="M50 56 C46 36, 32 28, 16 22" stroke="#1e293b" stroke-width="6"/>
  <!-- Cuña de aceleracion de pintura blanca -->
  <path d="M18 20 L24 23 L20 28 Z" fill="#ffffff"/>
  <path d="M46 50 L42 42 L38 46 Z" fill="#00c0f3"/>
</svg>''',

    "icon_rotonda_glorieta.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Glorieta Giratoria Multicarril -->
  <circle cx="32" cy="32" r="22" stroke="#334155" stroke-width="12"/>
  <circle cx="32" cy="32" r="22" stroke="#1e293b" stroke-width="8"/>
  <circle cx="32" cy="32" r="14" fill="#0f172a" stroke="#64748b" stroke-width="2"/>
  <!-- Isleta central ajardinada -->
  <circle cx="32" cy="32" r="9" fill="#065f46" stroke="#059669" stroke-width="1.5"/>
  <!-- Accesos radiales -->
  <line x1="32" y1="4" x2="32" y2="16" stroke="#1e293b" stroke-width="8"/>
  <line x1="32" y1="48" x2="32" y2="60" stroke="#1e293b" stroke-width="8"/>
  <line x1="4" y1="32" x2="16" y2="32" stroke="#1e293b" stroke-width="8"/>
  <line x1="48" y1="32" x2="60" y2="32" stroke="#1e293b" stroke-width="8"/>
</svg>''',

    "icon_puente_viaducto.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Viaducto Elevado sobre Pilares de Hormigon -->
  <path d="M6 46 C16 46, 20 54, 32 54 C44 54, 48 46, 58 46" stroke="#0369a1" stroke-width="3" fill="none"/>
  <!-- Pilares estructurales -->
  <rect x="18" y="24" width="6" height="30" fill="#94a3b8" rx="1"/>
  <rect x="40" y="24" width="6" height="30" fill="#94a3b8" rx="1"/>
  <line x1="16" y1="54" x2="26" y2="54" stroke="#475569" stroke-width="3"/>
  <line x1="38" y1="54" x2="48" y2="54" stroke="#475569" stroke-width="3"/>
  <!-- Tablero del viaducto -->
  <rect x="6" y="20" width="52" height="7" rx="2" fill="#cbd5e1" stroke="#475569" stroke-width="1.5"/>
  <!-- Quitamiedos superior -->
  <line x1="6" y1="16" x2="58" y2="16" stroke="#38bdf8" stroke-width="2"/>
</svg>''',

    "icon_tunel_montana.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Boca de Tunel de Montana en Herradura -->
  <path d="M10 56 L10 32 C10 18, 54 18, 54 32 L54 56 Z" fill="#0f172a" stroke="#64748b" stroke-width="3"/>
  <!-- Boveda de hormigon interior -->
  <path d="M18 56 L18 34 C18 24, 46 24, 46 34 L46 56 Z" fill="#020617"/>
  <!-- Macizo rocoso superior -->
  <path d="M4 36 L16 12 L32 8 L48 14 L60 36" stroke="#78716c" stroke-width="4" fill="none" stroke-linecap="round"/>
  <!-- Calzada entrando con linea reflectante -->
  <line x1="32" y1="42" x2="32" y2="56" stroke="#facc15" stroke-width="2.5" stroke-dasharray="3 3"/>
</svg>''',

    "icon_peaje_troncal.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Estacion de Peaje Troncal con Barreras -->
  <rect x="10" y="14" width="44" height="8" rx="2" fill="#0284c7" stroke="#38bdf8" stroke-width="1.5"/>
  <!-- Columnas y cabinas -->
  <rect x="14" y="22" width="6" height="24" fill="#e2e8f0" stroke="#64748b" stroke-width="1"/>
  <rect x="29" y="22" width="6" height="24" fill="#e2e8f0" stroke="#64748b" stroke-width="1"/>
  <rect x="44" y="22" width="6" height="24" fill="#e2e8f0" stroke="#64748b" stroke-width="1"/>
  <!-- Barrera abatible rayada amarilla/roja -->
  <line x1="20" y1="36" x2="29" y2="36" stroke="#ef4444" stroke-width="3" stroke-linecap="round"/>
  <line x1="35" y1="36" x2="44" y2="36" stroke="#22c55e" stroke-width="3" stroke-linecap="round"/>
  <!-- Tarjeta y simbolo € -->
  <circle cx="32" cy="52" r="8" fill="#10b981"/>
  <text x="32" y="56" fill="#ffffff" font-size="11" font-family="sans-serif" font-weight="900" text-anchor="middle">€</text>
</svg>''',

    "icon_paso_nivel.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Cruz de San Andres Oficial ADIF -->
  <line x1="14" y1="12" x2="50" y2="48" stroke="#ef4444" stroke-width="7" stroke-linecap="square"/>
  <line x1="50" y1="12" x2="14" y2="48" stroke="#ef4444" stroke-width="7" stroke-linecap="square"/>
  <line x1="14" y1="12" x2="50" y2="48" stroke="#ffffff" stroke-width="3" stroke-linecap="square"/>
  <line x1="50" y1="12" x2="14" y2="48" stroke="#ffffff" stroke-width="3" stroke-linecap="square"/>
  <!-- Foco luminoso rojo de detencion -->
  <circle cx="32" cy="54" r="6" fill="#dc2626" stroke="#ffffff" stroke-width="1.5"/>
</svg>''',

    "icon_grua_112.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Grua de Asistencia y Rescate 112 -->
  <rect x="8" y="32" width="22" height="16" rx="2" fill="#d97706" stroke="#f59e0b" stroke-width="1.5"/>
  <rect x="30" y="24" width="22" height="24" rx="2" fill="#b45309" stroke="#f59e0b" stroke-width="1.5"/>
  <rect x="36" y="28" width="12" height="8" rx="1" fill="#0f172a"/>
  <!-- Brazo hidraulico con cable -->
  <path d="M12 32 L24 16 L28 18" stroke="#facc15" stroke-width="3" stroke-linecap="round"/>
  <line x1="24" y1="16" x2="24" y2="28" stroke="#e2e8f0" stroke-width="1.5"/>
  <!-- Ruedas -->
  <circle cx="16" cy="48" r="5" fill="#1e293b" stroke="#cbd5e1" stroke-width="1.5"/>
  <circle cx="44" cy="48" r="5" fill="#1e293b" stroke="#cbd5e1" stroke-width="1.5"/>
  <!-- Rotativo ambar superior -->
  <polygon points="41,20 47,20 45,16 43,16" fill="#facc15"/>
</svg>''',

    "icon_guardia_civil.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Emblema Tecnico de la Agrupacion de Trafico -->
  <path d="M32 8 L52 14 L52 34 C52 46, 32 58, 32 58 C32 58, 12 46, 12 34 L12 14 Z" fill="#064e3b" stroke="#10b981" stroke-width="2.5"/>
  <!-- Espada y Fasces cruzadas estilizadas -->
  <line x1="22" y1="20" x2="42" y2="44" stroke="#facc15" stroke-width="2.5" stroke-linecap="round"/>
  <line x1="42" y1="20" x2="22" y2="44" stroke="#facc15" stroke-width="2.5" stroke-linecap="round"/>
  <circle cx="32" cy="32" r="5" fill="#047857" stroke="#facc15" stroke-width="1.5"/>
</svg>''',

    "icon_helicoptero_pegasus.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Helicoptero DGT Pegasus MX-15 -->
  <!-- Rotor principal -->
  <line x1="8" y1="16" x2="56" y2="16" stroke="#94a3b8" stroke-width="2" stroke-linecap="round"/>
  <circle cx="32" cy="16" r="3" fill="#38bdf8"/>
  <!-- Fuselaje aerodinamico -->
  <path d="M22 22 L40 22 C46 22, 48 26, 44 32 L36 38 L20 38 C16 38, 14 32, 18 24 Z" fill="#004b87" stroke="#38bdf8" stroke-width="1.5"/>
  <!-- Cola y rotor posterior -->
  <line x1="18" y1="28" x2="6" y2="24" stroke="#004b87" stroke-width="3"/>
  <line x1="6" y1="20" x2="6" y2="28" stroke="#94a3b8" stroke-width="2"/>
  <!-- Patines de aterrizaje -->
  <line x1="18" y1="44" x2="44" y2="44" stroke="#64748b" stroke-width="2"/>
  <line x1="24" y1="38" x2="22" y2="44" stroke="#64748b" stroke-width="2"/>
  <line x1="38" y1="38" x2="40" y2="44" stroke="#64748b" stroke-width="2"/>
  <!-- Camara optica frontal MX-15 -->
  <circle cx="44" cy="36" r="3" fill="#00c0f3"/>
</svg>''',

    "icon_demoler.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Piqueta de Demolicion y Obra Civil -->
  <path d="M12 52 L36 28" stroke="#94a3b8" stroke-width="5" stroke-linecap="round"/>
  <path d="M30 18 C34 12, 46 8, 52 14 C58 20, 54 32, 48 36 L38 26 Z" fill="#dc2626" stroke="#f87171" stroke-width="2"/>
  <!-- Lineas de fractura / impacto -->
  <line x1="8" y1="56" x2="16" y2="60" stroke="#fca5a5" stroke-width="2"/>
  <line x1="4" y1="48" x2="8" y2="54" stroke="#fca5a5" stroke-width="2"/>
</svg>''',

    "icon_weather_dana.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Borrasca DANA Torrencial con Isobaras -->
  <path d="M18 36 C14 36, 10 32, 10 26 C10 20, 16 16, 22 16 C24 10, 34 8, 42 14 C48 12, 54 16, 54 24 C58 26, 58 34, 52 36 Z" fill="#334155" stroke="#94a3b8" stroke-width="2"/>
  <!-- Precipitación vectorial inclinada -->
  <line x1="20" y1="42" x2="16" y2="54" stroke="#38bdf8" stroke-width="2.5" stroke-linecap="round"/>
  <line x1="30" y1="42" x2="26" y2="54" stroke="#38bdf8" stroke-width="2.5" stroke-linecap="round"/>
  <line x1="40" y1="42" x2="36" y2="54" stroke="#38bdf8" stroke-width="2.5" stroke-linecap="round"/>
  <line x1="50" y1="42" x2="46" y2="54" stroke="#38bdf8" stroke-width="2.5" stroke-linecap="round"/>
</svg>''',

    "icon_driver_zen.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Estado Conductor: Calma / Zen (Pulso Estable) -->
  <circle cx="32" cy="32" r="26" fill="#064e3b" stroke="#10b981" stroke-width="2.5"/>
  <path d="M14 32 L22 32 L26 24 L30 38 L34 28 L38 34 L42 32 L50 32" stroke="#34d399" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"/>
</svg>''',

    "icon_driver_impatient.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Estado Conductor: Impaciente (Presion Elevada) -->
  <circle cx="32" cy="32" r="26" fill="#78350f" stroke="#f59e0b" stroke-width="2.5"/>
  <path d="M18 42 A 18 18 0 1 1 46 42" stroke="#fcd34d" stroke-width="3" stroke-linecap="round" fill="none"/>
  <line x1="32" y1="32" x2="42" y2="24" stroke="#fbbf24" stroke-width="3.5" stroke-linecap="round"/>
  <circle cx="32" cy="32" r="4" fill="#fbbf24"/>
</svg>''',

    "icon_driver_rage.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Estado Conductor: Furia al Volante (Alerta Maxima) -->
  <circle cx="32" cy="32" r="26" fill="#7f1d1d" stroke="#ef4444" stroke-width="2.5"/>
  <polygon points="32,14 50,46 14,46" fill="#b91c1c" stroke="#f87171" stroke-width="2"/>
  <line x1="32" y1="24" x2="32" y2="36" stroke="#ffffff" stroke-width="3" stroke-linecap="round"/>
  <circle cx="32" cy="41" r="2" fill="#ffffff"/>
</svg>''',

    # Highways & Co. Dock Icons: Lane Selectors
    "icon_lanes_1.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <rect x="8" y="10" width="48" height="44" rx="4" fill="#1e293b" stroke="#475569" stroke-width="2"/>
  <rect x="22" y="14" width="20" height="36" fill="#334155" stroke="#64748b" stroke-width="1.5"/>
  <line x1="22" y1="14" x2="22" y2="50" stroke="#f8fafc" stroke-width="2"/>
  <line x1="42" y1="14" x2="42" y2="50" stroke="#f8fafc" stroke-width="2"/>
  <text x="32" y="38" font-family="Arial,sans-serif" font-size="16" font-weight="bold" fill="#38bdf8" text-anchor="middle">1L</text>
  <text x="32" y="58" font-family="Arial,sans-serif" font-size="8" fill="#94a3b8" text-anchor="middle">3.5m</text>
</svg>''',

    "icon_lanes_2.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <rect x="6" y="10" width="52" height="44" rx="4" fill="#1e293b" stroke="#475569" stroke-width="2"/>
  <rect x="14" y="14" width="36" height="36" fill="#334155" stroke="#64748b" stroke-width="1.5"/>
  <line x1="14" y1="14" x2="14" y2="50" stroke="#f8fafc" stroke-width="2"/>
  <line x1="50" y1="14" x2="50" y2="50" stroke="#f8fafc" stroke-width="2"/>
  <line x1="32" y1="18" x2="32" y2="26" stroke="#94a3b8" stroke-width="1.5"/>
  <line x1="32" y1="34" x2="32" y2="42" stroke="#94a3b8" stroke-width="1.5"/>
  <text x="32" y="38" font-family="Arial,sans-serif" font-size="16" font-weight="bold" fill="#38bdf8" text-anchor="middle">2L</text>
  <text x="32" y="58" font-family="Arial,sans-serif" font-size="8" fill="#94a3b8" text-anchor="middle">7.0m</text>
</svg>''',

    "icon_lanes_3.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <rect x="4" y="10" width="56" height="44" rx="4" fill="#0f172a" stroke="#334155" stroke-width="2"/>
  <rect x="10" y="14" width="44" height="36" fill="#1e293b" stroke="#475569" stroke-width="1.5"/>
  <line x1="10" y1="14" x2="10" y2="50" stroke="#f8fafc" stroke-width="2"/>
  <line x1="54" y1="14" x2="54" y2="50" stroke="#f8fafc" stroke-width="2"/>
  <line x1="25" y1="18" x2="25" y2="46" stroke="#94a3b8" stroke-width="1.5" stroke-dasharray="4 4"/>
  <line x1="39" y1="18" x2="39" y2="46" stroke="#94a3b8" stroke-width="1.5" stroke-dasharray="4 4"/>
  <text x="32" y="38" font-family="Arial,sans-serif" font-size="16" font-weight="bold" fill="#38bdf8" text-anchor="middle">3L</text>
  <text x="32" y="58" font-family="Arial,sans-serif" font-size="8" fill="#94a3b8" text-anchor="middle">10.5m</text>
</svg>''',

    "icon_lanes_4.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <rect x="2" y="10" width="60" height="44" rx="4" fill="#0f172a" stroke="#334155" stroke-width="2"/>
  <rect x="6" y="14" width="52" height="36" fill="#1e293b" stroke="#475569" stroke-width="1.5"/>
  <line x1="6" y1="14" x2="6" y2="50" stroke="#f8fafc" stroke-width="2"/>
  <line x1="58" y1="14" x2="58" y2="50" stroke="#f8fafc" stroke-width="2"/>
  <line x1="19" y1="18" x2="19" y2="46" stroke="#94a3b8" stroke-width="1.2" stroke-dasharray="3 3"/>
  <line x1="32" y1="18" x2="32" y2="46" stroke="#94a3b8" stroke-width="1.2" stroke-dasharray="3 3"/>
  <line x1="45" y1="18" x2="45" y2="46" stroke="#94a3b8" stroke-width="1.2" stroke-dasharray="3 3"/>
  <text x="32" y="38" font-family="Arial,sans-serif" font-size="16" font-weight="bold" fill="#38bdf8" text-anchor="middle">4L</text>
  <text x="32" y="58" font-family="Arial,sans-serif" font-size="8" fill="#94a3b8" text-anchor="middle">14.0m</text>
</svg>''',

    # Highways & Co. Elevation Level Selectors
    "icon_elevation_tunnel.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Nivel -1: Tunel Subterraneo (-6m) -->
  <line x1="4" y1="20" x2="60" y2="20" stroke="#64748b" stroke-width="3"/>
  <path d="M16 20 C16 44, 48 44, 48 20" stroke="#38bdf8" stroke-width="3" fill="#0f172a"/>
  <rect x="22" y="32" width="20" height="12" fill="#1e293b" stroke="#94a3b8" stroke-width="1.5"/>
  <text x="32" y="42" font-family="Arial,sans-serif" font-size="10" font-weight="bold" fill="#38bdf8" text-anchor="middle">-1</text>
  <text x="32" y="58" font-family="Arial,sans-serif" font-size="8" fill="#64748b" text-anchor="middle">-6.0m</text>
</svg>''',

    "icon_elevation_ground.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Nivel 0: Cota Rasante Terreno (0m) -->
  <line x1="4" y1="36" x2="60" y2="36" stroke="#10b981" stroke-width="3"/>
  <rect x="18" y="28" width="28" height="8" rx="2" fill="#1e293b" stroke="#f8fafc" stroke-width="2"/>
  <line x1="32" y1="28" x2="32" y2="36" stroke="#fcd34d" stroke-width="1.5"/>
  <text x="32" y="22" font-family="Arial,sans-serif" font-size="11" font-weight="bold" fill="#10b981" text-anchor="middle">0</text>
  <text x="32" y="52" font-family="Arial,sans-serif" font-size="8" fill="#64748b" text-anchor="middle">0.0m</text>
</svg>''',

    "icon_elevation_bridge.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Nivel +1: Viaducto / Puente (+6m) -->
  <line x1="4" y1="48" x2="60" y2="48" stroke="#64748b" stroke-width="2.5"/>
  <!-- Tablero elevado -->
  <rect x="10" y="22" width="44" height="8" rx="2" fill="#334155" stroke="#38bdf8" stroke-width="2"/>
  <!-- Pilares de hormigon -->
  <rect x="20" y="30" width="6" height="18" fill="#475569" stroke="#94a3b8" stroke-width="1"/>
  <rect x="38" y="30" width="6" height="18" fill="#475569" stroke="#94a3b8" stroke-width="1"/>
  <text x="32" y="16" font-family="Arial,sans-serif" font-size="11" font-weight="bold" fill="#38bdf8" text-anchor="middle">+1</text>
  <text x="32" y="58" font-family="Arial,sans-serif" font-size="8" fill="#64748b" text-anchor="middle">+6.0m</text>
</svg>''',

    "icon_elevation_highbridge.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Nivel +2: Paso a Distinto Nivel Superior (+12m) -->
  <line x1="4" y1="52" x2="60" y2="52" stroke="#64748b" stroke-width="2"/>
  <!-- Nivel intermedio -->
  <line x1="16" y1="36" x2="48" y2="36" stroke="#94a3b8" stroke-width="2" stroke-dasharray="4 2"/>
  <!-- Tablero alto -->
  <rect x="8" y="14" width="48" height="8" rx="2" fill="#1e293b" stroke="#f59e0b" stroke-width="2"/>
  <!-- Columnas esbeltas -->
  <line x1="18" y1="22" x2="18" y2="52" stroke="#cbd5e1" stroke-width="2.5"/>
  <line x1="46" y1="22" x2="46" y2="52" stroke="#cbd5e1" stroke-width="2.5"/>
  <text x="32" y="10" font-family="Arial,sans-serif" font-size="10" font-weight="bold" fill="#f59e0b" text-anchor="middle">+2</text>
  <text x="32" y="60" font-family="Arial,sans-serif" font-size="8" fill="#64748b" text-anchor="middle">+12.0m</text>
</svg>''',

    # Highways & Co. Drawing Modes & Alignment
    "icon_mode_straight.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Modo Alineacion Recta CAD -->
  <line x1="12" y1="52" x2="52" y2="12" stroke="#38bdf8" stroke-width="4" stroke-linecap="round"/>
  <circle cx="12" cy="52" r="5" fill="#0284c7" stroke="#ffffff" stroke-width="2"/>
  <circle cx="52" cy="12" r="5" fill="#0284c7" stroke="#ffffff" stroke-width="2"/>
  <!-- Regla graduada CAD -->
  <line x1="22" y1="38" x2="26" y2="42" stroke="#f8fafc" stroke-width="1.5"/>
  <line x1="32" y1="28" x2="36" y2="32" stroke="#f8fafc" stroke-width="1.5"/>
  <line x1="42" y1="18" x2="46" y2="22" stroke="#f8fafc" stroke-width="1.5"/>
</svg>''',

    "icon_mode_free.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Modo Curva Bezier / Spline Suave -->
  <path d="M10 48 C20 16, 44 16, 54 48" stroke="#38bdf8" stroke-width="4" stroke-linecap="round" fill="none"/>
  <!-- Tangentes de control Bezier -->
  <line x1="10" y1="48" x2="22" y2="16" stroke="#94a3b8" stroke-width="1.5" stroke-dasharray="3 3"/>
  <line x1="54" y1="48" x2="42" y2="16" stroke="#94a3b8" stroke-width="1.5" stroke-dasharray="3 3"/>
  <circle cx="10" cy="48" r="4" fill="#0284c7" stroke="#ffffff" stroke-width="1.5"/>
  <circle cx="54" cy="48" r="4" fill="#0284c7" stroke="#ffffff" stroke-width="1.5"/>
  <rect x="19" y="13" width="6" height="6" fill="#f59e0b" stroke="#ffffff" stroke-width="1"/>
  <rect x="39" y="13" width="6" height="6" fill="#f59e0b" stroke="#ffffff" stroke-width="1"/>
</svg>''',

    "icon_dir_oneway.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Sentido Unico (One-Way) -->
  <rect x="8" y="12" width="48" height="40" rx="4" fill="#1e293b" stroke="#475569" stroke-width="2"/>
  <path d="M32 44 L32 20 M22 28 L32 18 L42 28" stroke="#10b981" stroke-width="4" stroke-linecap="round" stroke-linejoin="round"/>
</svg>''',

    "icon_dir_twoway.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" fill="none">
  <!-- Doble Sentido (Two-Way) -->
  <rect x="8" y="12" width="48" height="40" rx="4" fill="#1e293b" stroke="#475569" stroke-width="2"/>
  <line x1="32" y1="12" x2="32" y2="52" stroke="#94a3b8" stroke-width="1.5" stroke-dasharray="4 3"/>
  <path d="M22 38 L22 22 M16 26 L22 20 L28 26" stroke="#38bdf8" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"/>
  <path d="M42 26 L42 42 M36 38 L42 44 L48 38" stroke="#f59e0b" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"/>
</svg>'''
}

for name, content in icons.items():
    path = os.path.join(ICONS_DIR, name)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"Generated: {name}")

print(f"Successfully created {len(icons)} technical SVG icons without any emojis.")
