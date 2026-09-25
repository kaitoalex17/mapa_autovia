/**
 * ============================================================================
 * AUTOPISTAS DE ESPAÑA - PROTOTIPO JUGABLE HTML5 / CANVAS 2.5D
 * Centro de Control DGT & Simulación Vial en Tiempo Real
 * ============================================================================
 * Desarrollado para Prototipado Rápido e Interactivo.
 * Implementa IDM (Intelligent Driver Model), MOBIL (Lane Changing),
 * Splines de trazado vial, Helicóptero Pegasus radar, Obras en vivo y
 * Sistema de Psicología y Furia al Volante (Road Rage).
 */

// Polyfill para CanvasRenderingContext2D.roundRect en navegadores antiguos
if (typeof CanvasRenderingContext2D !== 'undefined' && !CanvasRenderingContext2D.prototype.roundRect) {
  CanvasRenderingContext2D.prototype.roundRect = function (x, y, w, h, r = 0) {
    if (typeof r === 'number') r = [r, r, r, r];
    const tl = r[0] || 0, tr = r[1] || 0, br = r[2] || 0, bl = r[3] || 0;
    this.moveTo(x + tl, y);
    this.lineTo(x + w - tr, y);
    this.quadraticCurveTo(x + w, y, x + w, y + tr);
    this.lineTo(x + w, y + h - br);
    this.quadraticCurveTo(x + w, y + h, x + w - br, y + h);
    this.lineTo(x + bl, y + h);
    this.quadraticCurveTo(x, y + h, x, y + h - bl);
    this.lineTo(x, y + tl);
    this.quadraticCurveTo(x, y, x + tl, y);
    this.closePath();
    return this;
  };
}

// ============================================================================
// 1. SISTEMA DE INTERNACIONALIZACIÓN Y TEXTOS (I18N)
// ============================================================================
const I18N = {
  es: {
    GAME_TITLE: "Autopistas de España",
    SUB_TITLE: "Centro de Gestión de Tráfico DGT",
    HUD_BUDGET: "Presupuesto",
    HUD_CONGESTION: "Congestión",
    HUD_INCIDENTS: "Siniestros",
    HUD_ROAD_RAGE: "Furia al Volante",
    PANEL_SETTINGS: "Control de Simulación",
    TOGGLE_WORKS: "Obras en Vivo (5s)",
    TOGGLE_WORKS_SUB: "Conos y obreros antes de abrir",
    TOGGLE_RAGE: "Psicología y Furia",
    TOGGLE_RAGE_SUB: "Bocinas, emoticonos y estrés",
    TOGGLE_PEGASUS: "Patrulla Pegasus",
    TOGGLE_PEGASUS_SUB: "Helicóptero con radar láser",
    TOGGLE_WEATHER: "Clima DANA Lluvia",
    TOGGLE_WEATHER_SUB: "Asfalto mojado y menor frenada",
    ACTION_CAUSE_JAM: "Provocar Retención / Conos",
    ACTION_SPAWN_WAVE: "Generar Ola de Tráfico (+15)",
    ACTION_LOAD_SCENARIO: "Reiniciar Escenario A-4 / M-40",
    ACTION_CLEAR_MAP: "Limpiar Red Viaria",
    PEGASUS_RADAR_TITLE: "DGT PEGASUS MX-15",
    TOOL_INSPECT: "Inspeccionar",
    TOOL_CONVENCIONAL: "Carretera 90",
    TOOL_AUTOVIA: "Autovía 2x2",
    TOOL_AUTOVIA_3X3: "Autopista 3x3",
    TOOL_ENLACE: "Enlace / Ramal",
    TOOL_TOLL: "Peaje Troncal",
    TOOL_RAILWAY: "Paso a Nivel",
    TOOL_TOW: "Grúa 112",
    TOOL_POLICE: "Control DGT",
    TOOL_DEMOLISH: "Demoler",
    HELP_TITLE: "Guía Rápida de Juego y Controles",
    HELP_NAV_TITLE: "🎮 Navegación de Cámara",
    HELP_BUILD_TITLE: "🏗️ Construcción de Carreteras",
    HELP_TRAFFIC_TITLE: "🚗 Tráfico y Psicología",
    HELP_PEGASUS_TITLE: "🚁 Helicóptero Pegasus DGT",
    TOAST_ROAD_OPEN: "¡Obra finalizada! Tramo abierto al tráfico.",
    TOAST_JAM_TRIGGERED: "¡Retención provocada! Conos desplegados en calzada.",
    TOAST_FINE_ISSUED: "Multa Pegasus: {amount} € por {reason} ({speed} km/h)",
    TOAST_TOLL_PAID: "Peaje recaudado: +3.50 €",
    TOAST_TOW_ARRIVED: "Grúa 112 enviada. Retirando vehículo accidentado...",
    TOAST_TRAIN_CROSSING: "¡Aviso ADIF! Tren aproximándose al Paso a Nivel.",
    TOAST_DANA_ON: "Aviso DGT: Alerta DANA activada. Precaución en calzada.",
    TOAST_DANA_OFF: "Aviso DGT: Temporal finalizado. Vía seca.",
    STATUS_PATROL_ACTIVE: "PATRULLA ACTIVA",
    STATUS_PATROL_OFF: "EN BASE AÉREA",
    REASON_SPEEDING: "Exceso de velocidad",
    REASON_TAILGATING: "Conducción temeraria / Acoso",
    INSPECT_TITLE: "Vehículo Inspeccionado",
    INSPECT_TYPE: "Tipo",
    INSPECT_SPEED: "Velocidad",
    INSPECT_LIMIT: "Límite vía",
    INSPECT_MOOD: "Humor conductor",
    INSPECT_FRUSTRATION: "Frustración",
    INSPECT_JAM_TIME: "Tiempo en Atasco",
    INSPECT_STATUS: "Estado",
    INSPECT_ACTION_FINE: "Sancionar DGT (200 €)",
    INSPECT_ACTION_TOW: "Enviar Grúa 112"
  },
  en: {
    GAME_TITLE: "Highways of Spain",
    SUB_TITLE: "DGT Traffic Management Center",
    HUD_BUDGET: "Budget",
    HUD_CONGESTION: "Congestion",
    HUD_INCIDENTS: "Active Incidents",
    HUD_ROAD_RAGE: "Road Rage",
    PANEL_SETTINGS: "Simulation Controls",
    TOGGLE_WORKS: "Live Roadworks (5s)",
    TOGGLE_WORKS_SUB: "Cones and crew before opening",
    TOGGLE_RAGE: "Psychology & Rage",
    TOGGLE_RAGE_SUB: "Horns, emojis and driver stress",
    TOGGLE_PEGASUS: "Pegasus Patrol",
    TOGGLE_PEGASUS_SUB: "Helicopter laser radar patrol",
    TOGGLE_WEATHER: "DANA Heavy Rain",
    TOGGLE_WEATHER_SUB: "Wet asphalt & lower braking grip",
    ACTION_CAUSE_JAM: "Trigger Traffic Jam / Cones",
    ACTION_SPAWN_WAVE: "Spawn Traffic Wave (+15)",
    ACTION_LOAD_SCENARIO: "Reload A-4 / M-40 Corridor",
    ACTION_CLEAR_MAP: "Clear Road Network",
    PEGASUS_RADAR_TITLE: "DGT PEGASUS MX-15",
    TOOL_INSPECT: "Inspect",
    TOOL_CONVENCIONAL: "Road 90 km/h",
    TOOL_AUTOVIA: "Highway 2x2",
    TOOL_AUTOVIA_3X3: "Highway 3x3",
    TOOL_ENLACE: "Ramp / Link",
    TOOL_TOLL: "Toll Booth",
    TOOL_RAILWAY: "Level Crossing",
    TOOL_TOW: "Tow Truck 112",
    TOOL_POLICE: "Highway Patrol Check",
    TOOL_DEMOLISH: "Demolish",
    HELP_TITLE: "Quick Start Guide & Controls",
    HELP_NAV_TITLE: "🎮 Camera Navigation",
    HELP_BUILD_TITLE: "🏗️ Road Construction",
    HELP_TRAFFIC_TITLE: "🚗 Traffic & Psychology",
    HELP_PEGASUS_TITLE: "🚁 Pegasus DGT Helicopter",
    TOAST_ROAD_OPEN: "Roadworks complete! New road open to traffic.",
    TOAST_JAM_TRIGGERED: "Traffic jam created! Cones blocking lane.",
    TOAST_FINE_ISSUED: "Pegasus Fine: {amount} € for {reason} ({speed} km/h)",
    TOAST_TOLL_PAID: "Toll collected: +3.50 €",
    TOAST_TOW_ARRIVED: "112 Tow truck dispatched. Clearing collision...",
    TOAST_TRAIN_CROSSING: "ADIF Alert! Train approaching level crossing.",
    TOAST_DANA_ON: "DGT Warning: Heavy rainstorm active. Reduced traction.",
    TOAST_DANA_OFF: "DGT Warning: Storm cleared. Dry pavement.",
    STATUS_PATROL_ACTIVE: "ACTIVE PATROL",
    STATUS_PATROL_OFF: "AT AIRBASE",
    REASON_SPEEDING: "Speeding violation",
    REASON_TAILGATING: "Reckless tailgating",
    INSPECT_TITLE: "Inspected Vehicle",
    INSPECT_TYPE: "Type",
    INSPECT_SPEED: "Speed",
    INSPECT_LIMIT: "Road limit",
    INSPECT_MOOD: "Driver mood",
    INSPECT_FRUSTRATION: "Frustration",
    INSPECT_JAM_TIME: "Time in Jam",
    INSPECT_STATUS: "Status",
    INSPECT_ACTION_FINE: "Issue DGT Fine (200 €)",
    INSPECT_ACTION_TOW: "Dispatch 112 Tow Truck"
  }
};

let currentLang = 'es';

function t(key, params = {}) {
  let str = I18N[currentLang][key] || I18N['es'][key] || key;
  for (const [k, v] of Object.entries(params)) {
    str = str.replace(`{${k}}`, v);
  }
  return str;
}

function updateUITranslations() {
  document.querySelectorAll('[data-i18n]').forEach(el => {
    const key = el.getAttribute('data-i18n');
    el.textContent = t(key);
  });
  document.getElementById('lang-code').textContent = currentLang.toUpperCase();
  document.getElementById('lang-flag').textContent = currentLang === 'es' ? '🇪🇸' : '🇬🇧';
}

// ============================================================================
// 2. SISTEMA DE AUDIO PROCEDURAL (WEB AUDIO API SINTETIZADOR)
// ============================================================================
class SoundEngine {
  constructor() {
    this.ctx = null;
    this.enabled = true;
  }

  init() {
    if (!this.ctx) {
      const AudioCtx = window.AudioContext || window.webkitAudioContext;
      if (AudioCtx) {
        this.ctx = new AudioCtx();
      }
    }
    if (this.ctx && this.ctx.state === 'suspended') {
      this.ctx.resume();
    }
  }

  playClick() {
    if (!this.enabled) return;
    this.init();
    if (!this.ctx) return;
    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(600, this.ctx.currentTime);
    osc.frequency.exponentialRampToValueAtTime(1200, this.ctx.currentTime + 0.05);
    gain.gain.setValueAtTime(0.08, this.ctx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.001, this.ctx.currentTime + 0.05);
    osc.connect(gain);
    gain.connect(this.ctx.destination);
    osc.start();
    osc.stop(this.ctx.currentTime + 0.05);
  }

  playHorn() {
    if (!this.enabled) return;
    this.init();
    if (!this.ctx) return;
    // Doble tono clásico de bocina europea (420Hz y 510Hz)
    const now = this.ctx.currentTime;
    [420, 510].forEach(freq => {
      const osc = this.ctx.createOscillator();
      const gain = this.ctx.createGain();
      osc.type = 'sawtooth';
      osc.frequency.setValueAtTime(freq, now);
      gain.gain.setValueAtTime(0.04, now);
      gain.gain.exponentialRampToValueAtTime(0.001, now + 0.35);
      osc.connect(gain);
      gain.connect(this.ctx.destination);
      osc.start(now);
      osc.stop(now + 0.35);
    });
  }

  playCash() {
    if (!this.enabled) return;
    this.init();
    if (!this.ctx) return;
    // Chime brillante de multa registrada / caja registradora
    const now = this.ctx.currentTime;
    [987.77, 1318.51, 1975.53].forEach((freq, idx) => {
      const osc = this.ctx.createOscillator();
      const gain = this.ctx.createGain();
      osc.type = 'triangle';
      osc.frequency.setValueAtTime(freq, now + idx * 0.08);
      gain.gain.setValueAtTime(0.07, now + idx * 0.08);
      gain.gain.exponentialRampToValueAtTime(0.001, now + idx * 0.08 + 0.25);
      osc.connect(gain);
      gain.connect(this.ctx.destination);
      osc.start(now + idx * 0.08);
      osc.stop(now + idx * 0.08 + 0.25);
    });
  }

  playChime() {
    if (!this.enabled) return;
    this.init();
    if (!this.ctx) return;
    const now = this.ctx.currentTime;
    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(523.25, now); // C5
    osc.frequency.exponentialRampToValueAtTime(659.25, now + 0.15); // E5
    gain.gain.setValueAtTime(0.09, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.3);
    osc.connect(gain);
    gain.connect(this.ctx.destination);
    osc.start(now);
    osc.stop(now + 0.3);
  }
}

const sounds = new SoundEngine();

// ============================================================================
// 3. ESTADO GLOBAL DE LA SIMULACIÓN
// ============================================================================
const GameState = {
  budget: 2500000,
  congestion: 12,
  incidents: 0,
  roadRagePercent: 4,
  timeSpeed: 1, // 0: Pausa, 1: Normal, 2: Rápida, 4: Ultra
  
  // Toggles de simulación
  worksEnabled: true,
  rageEnabled: true,
  pegasusEnabled: true,
  weatherDANA: false,

  // Herramienta activa
  activeTool: 'inspect', // inspect, convencional, autovia, enlace, police_check, demolish

  // Estadísticas Pegasus
  pegasusInfractions: 0,
  pegasusCollected: 0,

  // Trazado en curso (Drag & Drop)
  isDrawing: false,
  drawStart: null,
  drawCurrent: null,

  // Vehículo seleccionado para inspección
  selectedVehicle: null
};

// ============================================================================
// 4. SISTEMA DE CÁMARA (VISTA ISOMÉTRICA / CENITAL 2.5D)
// ============================================================================
class Camera {
  constructor(canvas) {
    this.canvas = canvas;
    this.x = 800; // Coordenada X del mundo centrada en autovía
    this.y = 500;
    this.targetX = 800;
    this.targetY = 500;
    this.zoom = 1.0;
    this.targetZoom = 1.0;
    this.minZoom = 0.35;
    this.maxZoom = 2.8;

    this.isDragging = false;
    this.dragStartX = 0;
    this.dragStartY = 0;
    this.camStartX = 0;
    this.camStartY = 0;

    this.keys = {};
  }

  screenToWorld(screenX, screenY) {
    const cx = this.canvas.width / 2;
    const cy = this.canvas.height / 2;
    const wx = (screenX - cx) / this.zoom + this.x;
    const wy = (screenY - cy) / this.zoom + this.y;
    return { x: wx, y: wy };
  }

  worldToScreen(worldX, worldY) {
    const cx = this.canvas.width / 2;
    const cy = this.canvas.height / 2;
    const sx = (worldX - this.x) * this.zoom + cx;
    const sy = (worldY - this.y) * this.zoom + cy;
    return { x: sx, y: sy };
  }

  update(dt) {
    // Teclado WASD / Flechas
    const panSpeed = 650 * (1 / this.zoom);
    if (this.keys['KeyW'] || this.keys['ArrowUp']) this.targetY -= panSpeed * dt;
    if (this.keys['KeyS'] || this.keys['ArrowDown']) this.targetY += panSpeed * dt;
    if (this.keys['KeyA'] || this.keys['ArrowLeft']) this.targetX -= panSpeed * dt;
    if (this.keys['KeyD'] || this.keys['ArrowRight']) this.targetX += panSpeed * dt;

    // Suavizado Lerp
    this.x += (this.targetX - this.x) * Math.min(1, dt * 10);
    this.y += (this.targetY - this.y) * Math.min(1, dt * 10);
    this.zoom += (this.targetZoom - this.zoom) * Math.min(1, dt * 12);
  }

  centerOn(x, y, zoom = null) {
    this.targetX = x;
    this.targetY = y;
    if (zoom) this.targetZoom = Math.max(this.minZoom, Math.min(this.maxZoom, zoom));
  }
}

// ============================================================================
// 5. MODELADO DE CARRETERAS Y SPLINES
// ============================================================================
class RoadSegment {
  constructor(id, type, p0, p1, p2, p3) {
    this.id = id;
    this.type = type; // 'convencional', 'autovia', 'enlace'
    this.p0 = p0; // Puntos de control Bézier Cúbica
    this.p1 = p1;
    this.p2 = p2;
    this.p3 = p3;

    // Propiedades según tipo
    if (type === 'autovia_3x3') {
      this.speedLimit = 120;
      this.width = 92;
      this.lanes = 6; // 3 por sentido
      this.hasMedian = true;
    } else if (type === 'autovia') {
      this.speedLimit = 120;
      this.width = 64;
      this.lanes = 4; // 2 por sentido
      this.hasMedian = true;
    } else if (type === 'toll') {
      this.speedLimit = 40;
      this.width = 72;
      this.lanes = 4;
      this.hasMedian = true;
      this.isTollBooth = true;
      this.paidCars = new Set();
    } else if (type === 'railway') {
      this.speedLimit = 60;
      this.width = 36;
      this.lanes = 2; // calzada que cruza las vías
      this.hasMedian = false;
      this.isRailway = true;
      this.barrierLowered = false;
      this.trainTimer = 5.0; // tiempo hasta el próximo tren
    } else if (type === 'convencional') {
      this.speedLimit = 90;
      this.width = 36;
      this.lanes = 2; // 1 por sentido
      this.hasMedian = false;
    } else { // enlace
      this.speedLimit = 80;
      this.width = 24;
      this.lanes = 1; // 1 sentido
      this.hasMedian = false;
    }

    // Impacto de construcción (Obras en tiempo real)
    this.isUnderConstruction = GameState.worksEnabled;
    this.constructionProgress = 0; // 0.0 a 1.0
    this.constructionDuration = 5.0; // 5 segundos

    // Precalcular muestras del spline para interpolación rápida
    this.samples = [];
    this.length = 0;
    this.sampleSpline(60);

    // Obstáculo temporal (Conos provocados por atasco de prueba)
    this.blockedLanes = {}; // ej { laneIndex: true }
  }

  // Ecuación paramétrica Bézier Cúbica: B(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t) t^2 P2 + t^3 P3
  getPointAt(t) {
    const it = 1 - t;
    const it2 = it * it;
    const it3 = it2 * it;
    const t2 = t * t;
    const t3 = t2 * t;

    const x = it3 * this.p0.x + 3 * it2 * t * this.p1.x + 3 * it * t2 * this.p2.x + t3 * this.p3.x;
    const y = it3 * this.p0.y + 3 * it2 * t * this.p1.y + 3 * it * t2 * this.p2.y + t3 * this.p3.y;
    return { x, y };
  }

  getTangentAt(t) {
    const it = 1 - t;
    const dx = 3 * it * it * (this.p1.x - this.p0.x) + 6 * it * t * (this.p2.x - this.p1.x) + 3 * t * t * (this.p3.x - this.p2.x);
    const dy = 3 * it * it * (this.p1.y - this.p0.y) + 6 * it * t * (this.p2.y - this.p1.y) + 3 * t * t * (this.p3.y - this.p2.y);
    const len = Math.hypot(dx, dy) || 1;
    return { x: dx / len, y: dy / len, angle: Math.atan2(dy, dx) };
  }

  sampleSpline(numSamples) {
    this.samples = [];
    this.length = 0;
    let prev = this.getPointAt(0);
    this.samples.push({ t: 0, pt: prev, dist: 0 });

    for (let i = 1; i <= numSamples; i++) {
      const t = i / numSamples;
      const pt = this.getPointAt(t);
      const d = Math.hypot(pt.x - prev.x, pt.y - prev.y);
      this.length += d;
      this.samples.push({ t, pt, dist: this.length });
      prev = pt;
    }
  }

  getPointAtDistance(dist) {
    if (dist <= 0) return { pt: this.p0, t: 0, tangent: this.getTangentAt(0) };
    if (dist >= this.length) return { pt: this.p3, t: 1, tangent: this.getTangentAt(1) };

    for (let i = 0; i < this.samples.length - 1; i++) {
      const s0 = this.samples[i];
      const s1 = this.samples[i + 1];
      if (dist >= s0.dist && dist <= s1.dist) {
        const segDist = s1.dist - s0.dist;
        const localT = segDist > 0 ? (dist - s0.dist) / segDist : 0;
        const t = s0.t + localT * (s1.t - s0.t);
        return {
          pt: {
            x: s0.pt.x + localT * (s1.pt.x - s0.pt.x),
            y: s0.pt.y + localT * (s1.pt.y - s0.pt.y)
          },
          t,
          tangent: this.getTangentAt(t)
        };
      }
    }
    return { pt: this.p3, t: 1, tangent: this.getTangentAt(1) };
  }

  update(dt) {
    if (this.isUnderConstruction) {
      this.constructionProgress += dt / this.constructionDuration;
      if (this.constructionProgress >= 1.0) {
        this.constructionProgress = 1.0;
        this.isUnderConstruction = false;
        sounds.playChime();
        showToast(t('TOAST_ROAD_OPEN'), 'toast-works', '🚧');
      }
    }

    // Lógica de tren en paso a nivel
    if (this.isRailway) {
      this.trainTimer -= dt;
      if (this.trainTimer <= 0) {
        this.barrierLowered = !this.barrierLowered;
        this.trainTimer = this.barrierLowered ? 7.0 : 18.0; // 7s bajada para pasar el tren, 18s subida
        if (this.barrierLowered) {
          sounds.playHorn();
          showToast(t('TOAST_TRAIN_CROSSING'), 'toast-accident', '🚆');
        }
      }
    }
  }

  // Retorna la posición transversal de un carril (offset perpendicular al eje)
  getLaneOffset(laneIndex) {
    if (this.type === 'autovia_3x3') {
      switch (laneIndex) {
        case 0: return -32; // Carril derecho sentido A
        case 1: return -20; // Carril central sentido A
        case 2: return -8;  // Carril izquierdo sentido A
        case 3: return 8;   // Carril izquierdo sentido B
        case 4: return 20;  // Carril central sentido B
        case 5: return 32;  // Carril derecho sentido B
        default: return -20;
      }
    } else if (this.type === 'autovia' || this.type === 'toll') {
      switch (laneIndex) {
        case 0: return -18; // Carril derecho (sentido A)
        case 1: return -6;  // Carril izquierdo (adelantamiento sentido A)
        case 2: return 6;   // Carril izquierdo (adelantamiento sentido B)
        case 3: return 18;  // Carril derecho (sentido B)
        default: return -12;
      }
    } else if (this.type === 'convencional' || this.type === 'railway') {
      return laneIndex === 0 ? -9 : 9;
    } else {
      return 0;
    }
  }
}

// ============================================================================
// 6. SIMULACIÓN DE TRÁFICO REALISTA: MODELOS IDM Y MOBIL
// ============================================================================
class Vehicle {
  constructor(id, type, road, laneIndex, distance = 0) {
    this.id = id;
    this.type = type; // 'turismo', 'camion', 'furgoneta', 'guardia_civil', 'grua'
    this.road = road;
    this.laneIndex = laneIndex;
    this.distance = distance; // Distancia recorrida en metros a lo largo del spline
    this.v = 0; // Velocidad actual en m/s
    this.acc = 0;
    this.jamTime = 0.0;
    this.isTowed = false;
    this.isAccident = false;
    this.isBrokenDown = false;

    // Configuración física según tipo
    this.initPhysics();

    // Psicología y Humor del Conductor
    this.frustration = 0.0; // 0.0 a 100.0%
    this.mood = 'zen'; // 'zen', 'impatient', 'road_rage'
    this.honkTimer = 0;
    this.isHonking = false;
    this.headlightFlash = 0;

    // Infracción detectada por Pegasus
    this.isTargetedByPegasus = false;
    this.sanctionCooldown = 0;

    // Posición espacial y rotación en mundo
    this.x = 0;
    this.y = 0;
    this.angle = 0;
    this.lateralOffset = this.road.getLaneOffset(this.laneIndex);
    this.targetLateralOffset = this.lateralOffset;

    // Dirección de circulación en el spline (sentido +1 o -1)
    if (this.road.type === 'autovia_3x3') {
      this.direction = (laneIndex <= 2) ? 1 : -1;
    } else if (this.road.type === 'autovia' || this.road.type === 'toll') {
      this.direction = (laneIndex <= 1) ? 1 : -1;
    } else if (this.road.type === 'convencional' || this.road.type === 'railway') {
      this.direction = (laneIndex === 0) ? 1 : -1;
    } else {
      this.direction = 1;
    }

    if (this.direction === -1 && this.distance === 0) {
      this.distance = this.road.length;
    }

    // Inicializar velocidad con variación realista
    this.v = (this.v0 * (0.85 + Math.random() * 0.25)) / 3.6;
  }

  initPhysics() {
    switch (this.type) {
      case 'camion':
        this.length = 32;
        this.width = 9;
        this.v0 = 90; // km/h velocidad deseada
        this.a = 0.8; // Aceleración máx m/s^2
        this.b = 1.2; // Frenada confortable
        this.s0 = 4.0; // Distancia mínima en parada
        this.T = 1.8; // Espacio temporal de seguimiento
        this.color = '#38bdf8';
        this.bodyColor = '#0284c7';
        break;
      case 'furgoneta':
        this.length = 18;
        this.width = 7.5;
        this.v0 = 105;
        this.a = 1.4;
        this.b = 1.6;
        this.s0 = 2.5;
        this.T = 1.4;
        this.color = '#f1f5f9';
        this.bodyColor = '#cbd5e1';
        break;
      case 'guardia_civil':
        this.length = 16;
        this.width = 7.5;
        this.v0 = 120;
        this.a = 2.0;
        this.b = 2.4;
        this.s0 = 2.2;
        this.T = 1.2;
        this.color = '#005930';
        this.bodyColor = '#ffffff';
        this.isPolice = true;
        this.sirenFlash = 0;
        break;
      case 'turismo':
      default:
        this.length = 14;
        this.width = 7;
        // Velocidad deseada entre 110 y 135 km/h
        this.v0 = 110 + Math.random() * 25;
        this.a = 1.6 + Math.random() * 0.4;
        this.b = 1.8;
        this.s0 = 2.0;
        this.T = 1.2;
        const palette = ['#ef4444', '#3b82f6', '#10b981', '#f59e0b', '#64748b', '#e2e8f0', '#0f172a'];
        this.color = palette[Math.floor(Math.random() * palette.length)];
        this.bodyColor = this.color;
        break;
    }
  }

  // Cálculo IDM (Intelligent Driver Model):
  // acc = a * [ 1 - (v / v0)^4 - (s* / s)^2 ]
  // s* = s0 + v*T + (v * delta_v) / (2 * sqrt(a*b))
  calculateIDMAcceleration(frontVehicle, frontDist) {
    const v = Math.max(0, this.v);
    const v0_ms = (this.v0 / 3.6);

    // Ajuste por lluvia DANA (asfalto resbaladizo)
    const adhesionFactor = GameState.weatherDANA ? 0.65 : 1.0;
    const effA = this.a * adhesionFactor;
    const effB = this.b * adhesionFactor;

    // Comportamiento agresivo si tiene Furia al Volante
    let effS0 = this.s0;
    let effT = this.T;
    if (GameState.rageEnabled && this.mood === 'road_rage') {
      effS0 = Math.max(0.8, this.s0 * 0.4); // Pegado al parachoques
      effT = Math.max(0.4, this.T * 0.5);
    } else if (GameState.rageEnabled && this.mood === 'impatient') {
      effS0 = this.s0 * 0.7;
      effT = this.T * 0.8;
    }

    if (!frontVehicle) {
      // Vía libre: acelerar hacia velocidad deseada
      return effA * (1 - Math.pow(v / v0_ms, 4));
    }

    const deltaV = v - frontVehicle.v;
    const sStar = effS0 + Math.max(0, v * effT + (v * deltaV) / (2 * Math.sqrt(effA * effB)));
    const s = Math.max(0.5, frontDist);

    return effA * (1 - Math.pow(v / v0_ms, 4) - Math.pow(sStar / s, 2));
  }

  // Cambio de carril MOBIL simplificado para Autovía
  checkMOBILLaneChange(vehiclesOnRoad, dt) {
    if (this.road.type !== 'autovia' || this.road.lanes < 2) return;
    if (this.road.isUnderConstruction) return;

    // Sólo consideramos cambios entre carril derecho e izquierdo del mismo sentido
    let targetLane = -1;
    if (this.direction === 1) {
      // Sentido A: carril 0 (derecha) y carril 1 (adelantar)
      if (this.laneIndex === 0) targetLane = 1;
      else if (this.laneIndex === 1) targetLane = 0;
    } else {
      // Sentido B: carril 3 (derecha) y carril 2 (adelantar)
      if (this.laneIndex === 3) targetLane = 2;
      else if (this.laneIndex === 2) targetLane = 3;
    }

    if (targetLane === -1) return;
    if (this.road.blockedLanes[targetLane]) return; // Carril cortado

    // Buscar líderes y seguidores en carril actual y carril destino
    const currentLeader = this.findLeaderInLane(vehiclesOnRoad, this.laneIndex);
    const targetLeader = this.findLeaderInLane(vehiclesOnRoad, targetLane);
    const targetFollower = this.findFollowerInLane(vehiclesOnRoad, targetLane);

    // Criterio de seguridad: el nuevo seguidor no debe tener que clavar frenos
    if (targetFollower) {
      const followDist = Math.abs(this.distance - targetFollower.distance);
      if (followDist < this.s0 * 3) return; // Muy pegado para meterse con seguridad
    }

    // Regla de circular por la derecha:
    // Si estamos en el carril izquierdo (1 o 2), volver a la derecha si hay espacio suficiente
    const isLeftLane = (this.laneIndex === 1 || this.laneIndex === 2);
    if (isLeftLane) {
      if (!targetLeader || (Math.abs(targetLeader.distance - this.distance) > 60)) {
        this.laneIndex = targetLane;
        this.targetLateralOffset = this.road.getLaneOffset(targetLane);
        return;
      }
    } else {
      // Estamos en la derecha: ¿nos compensa adelantar por la izquierda?
      if (currentLeader) {
        const leadDist = Math.abs(currentLeader.distance - this.distance);
        const isLeaderSlow = currentLeader.v < this.v * 0.85;
        const wantsToOvertake = (leadDist < this.v * 2.5 && isLeaderSlow) || (this.mood === 'road_rage');
        if (wantsToOvertake) {
          this.laneIndex = targetLane;
          this.targetLateralOffset = this.road.getLaneOffset(targetLane);
        }
      }
    }
  }

  findLeaderInLane(vehicles, lane) {
    let bestDist = Infinity;
    let leader = null;
    for (const other of vehicles) {
      if (other === this || other.road !== this.road || other.laneIndex !== lane) continue;
      const d = (other.distance - this.distance) * this.direction;
      if (d > 0 && d < bestDist) {
        bestDist = d;
        leader = other;
      }
    }
    return leader;
  }

  findFollowerInLane(vehicles, lane) {
    let bestDist = Infinity;
    let follower = null;
    for (const other of vehicles) {
      if (other === this || other.road !== this.road || other.laneIndex !== lane) continue;
      const d = (this.distance - other.distance) * this.direction;
      if (d > 0 && d < bestDist) {
        bestDist = d;
        follower = other;
      }
    }
    return follower;
  }

  update(dt, vehiclesOnRoad) {
    // Si la carretera está en obras, detenerse antes de entrar o reducir a 10 km/h
    if (this.road.isUnderConstruction) {
      this.v = Math.max(0, this.v - 3.0 * dt);
      return;
    }

    // Comprobar si el carril actual está bloqueado por conos de prueba
    let frontVehicle = this.findLeaderInLane(vehiclesOnRoad, this.laneIndex);
    let frontDist = frontVehicle ? Math.abs(frontVehicle.distance - this.distance) - (frontVehicle.length / 2 + this.length / 2) : Infinity;

    // Obstáculo de conos en carril
    if (this.road.blockedLanes[this.laneIndex]) {
      const obstacleDist = Math.abs(this.road.length * 0.5 - this.distance);
      if (obstacleDist < frontDist && (this.road.length * 0.5 - this.distance) * this.direction > 0) {
        frontVehicle = { v: 0, length: 4 };
        frontDist = obstacleDist;
      }
    }

    // Parada ante barreras bajadas en Paso a Nivel ADIF
    if (this.road.isRailway && this.road.barrierLowered) {
      const barrierDist = Math.abs(this.road.length * 0.5 - this.distance);
      if (barrierDist < frontDist && (this.road.length * 0.5 - this.distance) * this.direction > 0) {
        frontVehicle = { v: 0, length: 3 };
        frontDist = Math.max(1, barrierDist - 6);
      }
    }

    // Parada y abono de tarifa en Peaje Troncal
    if (this.road.isTollBooth) {
      const tollDist = Math.abs(this.road.length * 0.5 - this.distance);
      if (tollDist < frontDist && (this.road.length * 0.5 - this.distance) * this.direction > 0) {
        if (!this.road.paidCars.has(this.id) && tollDist < 10) {
          this.road.paidCars.add(this.id);
          GameState.budget += 3.50;
          sounds.playCash();
          showToast(t('TOAST_TOLL_PAID'), 'toast-fine', '💳');
          floatingTexts.push({ text: "+3.50 €", x: this.x, y: this.y - 15, life: 2.0, alpha: 1.0 });
        } else if (!this.road.paidCars.has(this.id)) {
          frontVehicle = { v: 0, length: 2 };
          frontDist = Math.max(1, tollDist - 4);
        }
      }
    }

    // Evaluar cambio de carril MOBIL
    this.checkMOBILLaneChange(vehiclesOnRoad, dt);

    // Calcular aceleración IDM
    this.acc = this.calculateIDMAcceleration(frontVehicle, frontDist);
    this.v = Math.max(0, this.v + this.acc * dt);

    // Actualizar distancia en el trazado
    this.distance += this.v * dt * this.direction;

    // Medición de tiempo en atasco
    if (this.v * 3.6 < 12) {
      this.jamTime = (this.jamTime || 0) + dt;
    } else {
      this.jamTime = Math.max(0, (this.jamTime || 0) - dt * 0.5);
    }

    // Actualizar Psicología / Frustración
    if (GameState.rageEnabled) {
      const v_kmh = this.v * 3.6;
      if (v_kmh < 15) {
        // En atasco o parado: la frustración sube rápidamente
        this.frustration = Math.min(100, this.frustration + 9.5 * dt);
      } else if (v_kmh > this.v0 * 0.8) {
        // Circulación libre: alivio de estrés
        this.frustration = Math.max(0, this.frustration - 6.0 * dt);
      }

      // Estados de humor
      if (this.frustration < 30) {
        this.mood = 'zen';
        this.isHonking = false;
      } else if (this.frustration < 70) {
        this.mood = 'impatient';
        this.isHonking = false;
      } else {
        this.mood = 'road_rage';
        // En furia al volante: tocar claxon esporádicamente
        this.honkTimer -= dt;
        if (this.honkTimer <= 0) {
          this.isHonking = true;
          this.honkTimer = 2.5 + Math.random() * 2.0;
          sounds.playHorn();
        } else if (this.honkTimer < 2.0) {
          this.isHonking = false;
        }
      }
    } else {
      this.frustration = 0;
      this.mood = 'zen';
      this.isHonking = false;
    }

    // Suavizado del cambio lateral de carril
    this.lateralOffset += (this.targetLateralOffset - this.lateralOffset) * Math.min(1, dt * 5);

    // Luces de patrulla
    if (this.isPolice) {
      this.sirenFlash = (this.sirenFlash + dt * 10) % 2;
    }

    // Calcular posición X, Y y rotación en el mundo a través del spline
    const sample = this.road.getPointAtDistance(this.distance);
    const tangent = sample.tangent;
    this.angle = tangent.angle + (this.direction === -1 ? Math.PI : 0);

    // Desplazamiento perpendicular para situarse en su carril
    const perpX = -Math.sin(tangent.angle) * this.lateralOffset;
    const perpY = Math.cos(tangent.angle) * this.lateralOffset;

    this.x = sample.pt.x + perpX;
    this.y = sample.pt.y + perpY;
  }
}

// ============================================================================
// 7. HELICÓPTERO RADAR DGT PEGASUS
// ============================================================================
class PegasusHelicopter {
  constructor() {
    this.x = 800;
    this.y = 500;
    this.altitude = 300;
    this.targetX = 800;
    this.targetY = 500;
    this.speed = 140; // km/h
    this.rotorAngle = 0;
    this.scanRadius = 140; // Radio del cono láser en la calzada
    this.scanPulse = 0;
    this.trackedVehicle = null;
    this.trackTimer = 0;
    this.active = true;
  }

  update(dt, vehicles, roads) {
    if (!this.active || !GameState.pegasusEnabled) return;

    this.rotorAngle += dt * 35;
    this.scanPulse = (this.scanPulse + dt * 2.5) % 1;

    // Patrulla orbital suave alrededor de los tramos principales
    if (roads.length > 0) {
      const time = Date.now() * 0.0004;
      const refRoad = roads[0];
      const midPt = refRoad.getPointAt(0.5);
      this.targetX = midPt.x + Math.cos(time) * 320;
      this.targetY = midPt.y + Math.sin(time) * 180;
    }

    // Movimiento hacia objetivo
    const dx = this.targetX - this.x;
    const dy = this.targetY - this.y;
    const dist = Math.hypot(dx, dy);
    if (dist > 10) {
      const step = (this.speed * 0.8) * dt;
      this.x += (dx / dist) * Math.min(step, dist);
      this.y += (dy / dist) * Math.min(step, dist);
    }

    // Escanear vehículos bajo el cono de radar
    let candidate = null;
    let worstSpeed = 0;

    for (const v of vehicles) {
      const d = Math.hypot(v.x - this.x, v.y - this.y);
      if (d < this.scanRadius) {
        const speedKmh = v.v * 3.6;
        const limitKmh = v.road.speedLimit;

        // Infracción: exceso de velocidad o furia al volante peligrosa
        const isSpeeding = speedKmh > limitKmh + 6;
        const isReckless = (v.mood === 'road_rage');

        if ((isSpeeding || isReckless) && speedKmh > worstSpeed) {
          worstSpeed = speedKmh;
          candidate = v;
        }
      }
    }

    // Lógica de fijación de objetivo (Lock-on) y emisión de multa
    if (candidate) {
      this.trackedVehicle = candidate;
      this.trackTimer += dt;

      // Mantener fijación 1.2 segundos para certificar la infracción
      if (this.trackTimer >= 1.2) {
        this.issueSanction(candidate);
        this.trackTimer = 0;
      }
    } else {
      this.trackedVehicle = null;
      this.trackTimer = 0;
    }
  }

  issueSanction(vehicle) {
    if (vehicle.sanctionCooldown > 0) return;
    vehicle.sanctionCooldown = 8.0; // Cooldown para no multar dos veces seguidas

    const speedKmh = Math.round(vehicle.v * 3.6);
    const limit = vehicle.road.speedLimit;
    let fineAmount = 300;
    let reason = t('REASON_SPEEDING');

    if (vehicle.mood === 'road_rage') {
      fineAmount = 600;
      reason = t('REASON_TAILGATING');
    } else if (speedKmh > limit + 30) {
      fineAmount = 600;
    } else {
      fineAmount = 100;
    }

    // Ingresar al presupuesto
    GameState.budget += fineAmount;
    GameState.pegasusInfractions++;
    GameState.pegasusCollected += fineAmount;

    sounds.playCash();

    // Actualizar log en el panel de Pegasus
    addPegasusLogEntry(fineAmount, reason, speedKmh);

    // Notificación Toast
    showToast(t('TOAST_FINE_ISSUED', { amount: fineAmount, reason, speed: speedKmh }), 'toast-fine', '📸');

    // Partícula visual flotante de multa en mundo
    spawnFloatingFine(vehicle.x, vehicle.y, `+${fineAmount} €`);
  }
}

// ============================================================================
// 8. EFECTOS VISUALES, PARTÍCULAS Y CLIMA DANA
// ============================================================================
const floatingTexts = [];
function spawnFloatingFine(x, y, text) {
  floatingTexts.push({ x, y, text, alpha: 1.0, life: 2.0 });
}

const rainParticles = [];
function initRain(width, height) {
  rainParticles.length = 0;
  for (let i = 0; i < 240; i++) {
    rainParticles.push({
      x: Math.random() * width,
      y: Math.random() * height,
      len: 12 + Math.random() * 10,
      speed: 400 + Math.random() * 300
    });
  }
}

// ============================================================================
// 9. GESTOR PRINCIPAL DEL JUEGO (MOTOR)
// ============================================================================
class GameEngine {
  constructor() {
    this.canvas = document.getElementById('gameCanvas');
    this.ctx = this.canvas.getContext('2d');
    this.camera = new Camera(this.canvas);
    this.pegasus = new PegasusHelicopter();

    this.roads = [];
    this.vehicles = [];
    this.nextRoadId = 1;
    this.nextVehicleId = 1;

    this.lastTime = performance.now();
    this.spawnTimer = 0;

    // Fondo DGT
    this.bgImage = new Image();
    this.bgImageLoaded = false;
    this.bgImage.onload = () => { this.bgImageLoaded = true; };
    this.bgImage.src = 'MainMenu_DGT_Background.jpg';

    this.resizeCanvas();
    window.addEventListener('resize', () => this.resizeCanvas());
    this.bindEvents();
    this.bindUI();

    // Iniciar con un escenario realista de autovía española pre-cargado
    this.loadDefaultScenario();
  }

  resizeCanvas() {
    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight;
    initRain(this.canvas.width, this.canvas.height);
  }

  loadDefaultScenario() {
    this.roads = [];
    this.vehicles = [];
    floatingTexts.length = 0;

    // Corredor Autovía A-4 / M-40 principal de 1.4 km con curvas suaves
    // Segmento 1: Autovía recta principal
    const r1 = new RoadSegment(
      this.nextRoadId++,
      'autovia',
      { x: 100, y: 500 },
      { x: 500, y: 500 },
      { x: 900, y: 480 },
      { x: 1400, y: 480 }
    );
    r1.isUnderConstruction = false;
    this.roads.push(r1);

    // Segmento 2: Prolongación con curva suave hacia el noreste
    const r2 = new RoadSegment(
      this.nextRoadId++,
      'autovia',
      { x: 1400, y: 480 },
      { x: 1800, y: 480 },
      { x: 2100, y: 600 },
      { x: 2500, y: 750 }
    );
    r2.isUnderConstruction = false;
    this.roads.push(r2);

    // Segmento 3: Carretera convencional N-IV que cruza e intercepta
    const r3 = new RoadSegment(
      this.nextRoadId++,
      'convencional',
      { x: 750, y: 150 },
      { x: 780, y: 350 },
      { x: 820, y: 650 },
      { x: 860, y: 900 }
    );
    r3.isUnderConstruction = false;
    this.roads.push(r3);

    // Segmento 4: Ramal de enlace para incorporación a autovía
    const r4 = new RoadSegment(
      this.nextRoadId++,
      'enlace',
      { x: 790, y: 380 },
      { x: 850, y: 420 },
      { x: 950, y: 480 },
      { x: 1100, y: 480 }
    );
    r4.isUnderConstruction = false;
    this.roads.push(r4);

    // Generar flota inicial de vehículos en movimiento
    this.spawnVehicleFleet(20);

    // Centrar cámara en el nudo de comunicaciones
    this.camera.centerOn(950, 490, 0.95);
  }

  spawnVehicleFleet(count) {
    const types = ['turismo', 'turismo', 'camion', 'furgoneta', 'guardia_civil'];
    for (let i = 0; i < count; i++) {
      const road = this.roads[Math.floor(Math.random() * this.roads.length)];
      if (road.isUnderConstruction) continue;

      const lane = Math.floor(Math.random() * road.lanes);
      const dist = Math.random() * (road.length * 0.9);
      const type = types[Math.floor(Math.random() * types.length)];

      const v = new Vehicle(this.nextVehicleId++, type, road, lane, dist);
      this.vehicles.push(v);
    }
  }

  // ==========================================================================
  // EVENTOS DE ENTRADA Y CONTROLES
  // ==========================================================================
  bindEvents() {
    window.addEventListener('keydown', (e) => {
      this.camera.keys[e.code] = true;

      // Atajos de herramientas numéricos (1 a 9) y Q
      if (e.code === 'KeyQ') selectTool('inspect');
      if (e.code === 'Digit1') selectTool('convencional');
      if (e.code === 'Digit2') selectTool('autovia');
      if (e.code === 'Digit3') selectTool('autovia_3x3');
      if (e.code === 'Digit4') selectTool('enlace');
      if (e.code === 'Digit5') selectTool('toll');
      if (e.code === 'Digit6') selectTool('railway');
      if (e.code === 'Digit7') selectTool('tow_truck');
      if (e.code === 'Digit8') selectTool('police_check');
      if (e.code === 'Digit9') selectTool('demolish');

      // Centrar cámara con C
      if (e.code === 'KeyC') this.camera.centerOn(950, 490, 0.95);

      // Pausa con Espacio
      if (e.code === 'Space') {
        e.preventDefault();
        setSimulationSpeed(GameState.timeSpeed === 0 ? 1 : 0);
      }
    });

    window.addEventListener('keyup', (e) => {
      this.camera.keys[e.code] = false;
    });

    // Zoom con rueda del ratón hacia el cursor
    this.canvas.addEventListener('wheel', (e) => {
      e.preventDefault();
      const zoomFactor = e.deltaY < 0 ? 1.15 : 0.87;
      const mouse = this.camera.screenToWorld(e.clientX, e.clientY);
      const newZoom = Math.max(this.camera.minZoom, Math.min(this.camera.maxZoom, this.camera.targetZoom * zoomFactor));

      this.camera.targetZoom = newZoom;
      this.camera.targetX += (mouse.x - this.camera.targetX) * (1 - 1 / zoomFactor) * 0.5;
      this.camera.targetY += (mouse.y - this.camera.targetY) * (1 - 1 / zoomFactor) * 0.5;
    }, { passive: false });

    // Ratón: Arrastre de mapa o Trazado de Carretera
    this.canvas.addEventListener('mousedown', (e) => {
      sounds.init();
      const worldPos = this.camera.screenToWorld(e.clientX, e.clientY);

      // Botón central o botón derecho: Desplazar cámara
      if (e.button === 1 || e.button === 2) {
        this.camera.isDragging = true;
        this.camera.dragStartX = e.clientX;
        this.camera.dragStartY = e.clientY;
        this.camera.camStartX = this.camera.targetX;
        this.camera.camStartY = this.camera.targetY;
        return;
      }

      // Botón izquierdo: interactuar según herramienta
      if (e.button === 0) {
        if (GameState.activeTool === 'inspect') {
          this.inspectAt(worldPos);
        } else if (['convencional', 'autovia', 'autovia_3x3', 'enlace', 'toll', 'railway'].includes(GameState.activeTool)) {
          GameState.isDrawing = true;
          const snapped = this.getSnapPoint(worldPos);
          GameState.drawStart = snapped;
          GameState.drawCurrent = snapped;
        } else if (GameState.activeTool === 'tow_truck') {
          this.dispatchTowTruck(worldPos);
        } else if (GameState.activeTool === 'police_check') {
          this.deployPoliceCheckpoint(worldPos);
        } else if (GameState.activeTool === 'demolish') {
          this.demolishAt(worldPos);
        }
      }
    });

    window.addEventListener('mousemove', (e) => {
      if (this.camera.isDragging) {
        const dx = (e.clientX - this.camera.dragStartX) / this.camera.zoom;
        const dy = (e.clientY - this.camera.dragStartY) / this.camera.zoom;
        this.camera.targetX = this.camera.camStartX - dx;
        this.camera.targetY = this.camera.camStartY - dy;
        return;
      }

      if (GameState.isDrawing) {
        const worldPos = this.camera.screenToWorld(e.clientX, e.clientY);
        GameState.drawCurrent = this.getSnapPoint(worldPos);
      }
    });

    window.addEventListener('mouseup', (e) => {
      if (e.button === 1 || e.button === 2) {
        this.camera.isDragging = false;
      }

      if (e.button === 0 && GameState.isDrawing) {
        GameState.isDrawing = false;
        if (GameState.drawStart && GameState.drawCurrent) {
          const d = Math.hypot(GameState.drawCurrent.x - GameState.drawStart.x, GameState.drawCurrent.y - GameState.drawStart.y);
          if (d > 40) {
            this.buildRoadSegment(GameState.drawStart, GameState.drawCurrent, GameState.activeTool);
          }
        }
        GameState.drawStart = null;
        GameState.drawCurrent = null;
      }
    });

    this.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
  }

  getSnapPoint(pt) {
    const snapDist = 28;
    for (const r of this.roads) {
      if (Math.hypot(r.p0.x - pt.x, r.p0.y - pt.y) < snapDist) return { x: r.p0.x, y: r.p0.y };
      if (Math.hypot(r.p3.x - pt.x, r.p3.y - pt.y) < snapDist) return { x: r.p3.x, y: r.p3.y };
    }
    return pt;
  }

  buildRoadSegment(pStart, pEnd, toolType) {
    const dx = pEnd.x - pStart.x;
    const dy = pEnd.y - pStart.y;
    const p1 = { x: pStart.x + dx * 0.33, y: pStart.y + dy * 0.33 };
    const p2 = { x: pStart.x + dx * 0.66, y: pStart.y + dy * 0.66 };

    const segment = new RoadSegment(this.nextRoadId++, toolType, pStart, p1, p2, pEnd);

    // Deducción de coste de obra (€)
    const costPerMeter = toolType === 'autovia_3x3' ? 1200 : toolType === 'autovia' ? 850 : toolType === 'toll' ? 1500 : toolType === 'railway' ? 950 : 450;
    const totalCost = Math.round(segment.length * costPerMeter);
    GameState.budget = Math.max(0, GameState.budget - totalCost);

    sounds.playClick();
    this.roads.push(segment);

    if (GameState.worksEnabled) {
      showToast(`Obras iniciadas en nuevo tramo (-${totalCost.toLocaleString()} €)`, 'toast-works', '🚧');
    } else {
      showToast(`Tramo completado (-${totalCost.toLocaleString()} €)`, 'toast-fine', '🛣️');
    }
  }

  deployPoliceCheckpoint(pt) {
    for (const r of this.roads) {
      for (const sample of r.samples) {
        if (Math.hypot(sample.pt.x - pt.x, sample.pt.y - pt.y) < 35) {
          r.blockedLanes[0] = !r.blockedLanes[0];
          sounds.playHorn();
          showToast("Control Guardia Civil desplegado en calzada.", 'toast-accident', '🚓');
          return;
        }
      }
    }
  }

  dispatchTowTruck(pt) {
    if (this.roads.length === 0) return;
    let target = null;
    let minDist = 120;
    for (const v of this.vehicles) {
      const d = Math.hypot(v.x - pt.x, v.y - pt.y);
      if (d < minDist) {
        minDist = d;
        target = v;
      }
    }
    if (!target) {
      target = this.vehicles.find(v => v.mood === 'road_rage' || v.isBrokenDown || v.isAccident);
    }
    if (target) {
      target.isTowed = true;
      sounds.playChime();
      showToast(t('TOAST_TOW_ARRIVED'), 'toast-fine', '🚚');
      target.road.blockedLanes = {};
      setTimeout(() => {
        this.vehicles = this.vehicles.filter(v => v !== target);
        showToast("Vehículo retirado al depósito. Vía despejada.", 'toast-works', '✅');
      }, 2500);
    } else {
      showToast("No se detectan incidentes en la zona seleccionada.", 'toast-works', 'ℹ️');
    }
  }

  demolishAt(pt) {
    for (let i = this.roads.length - 1; i >= 0; i--) {
      const r = this.roads[i];
      for (const sample of r.samples) {
        if (Math.hypot(sample.pt.x - pt.x, sample.pt.y - pt.y) < 30) {
          this.vehicles = this.vehicles.filter(v => v.road !== r);
          this.roads.splice(i, 1);
          sounds.playClick();
          showToast("Tramo demolido.", 'toast-accident', '🚜');
          return;
        }
      }
    }
  }

  inspectAt(pt) {
    let closest = null;
    let minDist = 40;
    for (const v of this.vehicles) {
      const d = Math.hypot(v.x - pt.x, v.y - pt.y);
      if (d < minDist) {
        minDist = d;
        closest = v;
      }
    }
    GameState.selectedVehicle = closest;
    const insp = document.getElementById('vehicle-inspector');
    if (closest && insp) {
      sounds.playClick();
      insp.classList.remove('hidden');
      this.updateInspectorCard(closest);
    } else if (insp) {
      insp.classList.add('hidden');
    }
  }

  updateInspectorCard(v) {
    if (!v) return;
    const spd = Math.round(v.v * 3.6);
    const typeLabel = v.type === 'camion' ? 'CAMIÓN ARTICULADO' : v.type === 'furgoneta' ? 'FURGONETA REPARTO' : v.type === 'guardia_civil' ? 'GUARDIA CIVIL TRÁFICO' : 'TURISMO COMPACTO';
    document.getElementById('insp-type').textContent = typeLabel;
    document.getElementById('insp-icon').textContent = v.type === 'camion' ? '🚛' : v.type === 'furgoneta' ? '🚐' : v.type === 'guardia_civil' ? '🚓' : '🚗';
    document.getElementById('insp-plate').textContent = `${(1000 + v.id * 17) % 9000 + 1000}-DGT`;
    document.getElementById('insp-speed').innerHTML = `${spd} km/h <span class="insp-sub">/ ${v.road ? v.road.speedLimit : 120}</span>`;

    const moodEmoji = v.mood === 'road_rage' ? '😡' : v.mood === 'impatient' ? '😐' : '🙂';
    const moodText = v.mood === 'road_rage' ? t('HUD_ROAD_RAGE') : v.mood === 'impatient' ? 'Impaciente' : 'Tranquilo';
    document.getElementById('insp-mood-emoji').textContent = moodEmoji;
    document.getElementById('insp-mood-text').textContent = moodText;

    const frust = Math.round(v.frustration);
    document.getElementById('insp-frustration-val').textContent = `${frust}%`;
    document.getElementById('insp-frustration-fill').style.width = `${frust}%`;
    document.getElementById('insp-jam-time').textContent = `${(v.jamTime || 0).toFixed(1)} s`;
    document.getElementById('insp-incident-status').textContent = v.isTowed ? 'En grúa' : (spd < 5 ? 'Detenido en retención' : 'Circulando con normalidad');
  }

  // ==========================================================================
  // BINDING DE ELEMENTOS DE LA INTERFAZ HTML (HUD)
  // ==========================================================================
  bindUI() {
    // Velocidad de simulación
    const setSpeed = (spd, btnId) => {
      GameState.timeSpeed = spd;
      document.querySelectorAll('.time-btn').forEach(b => b.classList.remove('active'));
      document.getElementById(btnId).classList.add('active');
      sounds.playClick();
    };

    document.getElementById('btn-pause').onclick = () => setSpeed(0, 'btn-pause');
    document.getElementById('btn-speed-1').onclick = () => setSpeed(1, 'btn-speed-1');
    document.getElementById('btn-speed-2').onclick = () => setSpeed(2, 'btn-speed-2');
    document.getElementById('btn-speed-4').onclick = () => setSpeed(4, 'btn-speed-4');

    // Selector de idioma ES / EN
    document.getElementById('btn-lang').onclick = () => {
      currentLang = currentLang === 'es' ? 'en' : 'es';
      updateUITranslations();
      sounds.playClick();
    };

    // Audio mute / unmute
    const btnAudio = document.getElementById('btn-audio');
    btnAudio.onclick = () => {
      sounds.enabled = !sounds.enabled;
      btnAudio.textContent = sounds.enabled ? '🔊' : '🔇';
      sounds.playClick();
    };

    // Modal de Ayuda
    const helpModal = document.getElementById('help-modal');
    document.getElementById('btn-help').onclick = () => helpModal.classList.add('open');
    document.getElementById('btn-close-help').onclick = () => helpModal.classList.remove('open');
    helpModal.onclick = (e) => { if (e.target === helpModal) helpModal.classList.remove('open'); };

    // Toggles en panel lateral
    document.getElementById('toggle-works').onchange = (e) => {
      GameState.worksEnabled = e.target.checked;
      sounds.playClick();
    };

    document.getElementById('toggle-rage').onchange = (e) => {
      GameState.rageEnabled = e.target.checked;
      sounds.playClick();
    };

    document.getElementById('toggle-pegasus').onchange = (e) => {
      GameState.pegasusEnabled = e.target.checked;
      document.getElementById('pegasus-status').textContent = GameState.pegasusEnabled ? t('STATUS_PATROL_ACTIVE') : t('STATUS_PATROL_OFF');
      document.getElementById('pegasus-status').style.color = GameState.pegasusEnabled ? '#34d399' : '#94a3b8';
      sounds.playClick();
    };

    document.getElementById('toggle-weather').onchange = (e) => {
      GameState.weatherDANA = e.target.checked;
      sounds.playClick();
      showToast(GameState.weatherDANA ? t('TOAST_DANA_ON') : t('TOAST_DANA_OFF'), 'toast-accident', '🌧️');
    };

    // Botones de acción rápida
    document.getElementById('btn-trigger-jam').onclick = () => {
      if (this.roads.length > 0) {
        // Bloquear temporalmente el carril derecho de la autovía principal para generar retención
        const autovia = this.roads.find(r => r.type === 'autovia') || this.roads[0];
        autovia.blockedLanes[0] = !autovia.blockedLanes[0];
        sounds.playHorn();
        showToast(t('TOAST_JAM_TRIGGERED'), 'toast-accident', '🚨');
      }
    };

    document.getElementById('btn-spawn-fleet').onclick = () => {
      this.spawnVehicleFleet(15);
      sounds.playClick();
    };

    document.getElementById('btn-load-scenario').onclick = () => {
      this.loadDefaultScenario();
      sounds.playClick();
    };

    document.getElementById('btn-clear-map').onclick = () => {
      this.roads = [];
      this.vehicles = [];
      sounds.playClick();
      showToast("Mapa despejado. Modo Sandbox libre activado.", 'toast-works', '🗑️');
    };

    // Botones de cámara
    document.getElementById('btn-zoom-in').onclick = () => {
      this.camera.targetZoom = Math.min(this.camera.maxZoom, this.camera.targetZoom * 1.25);
      sounds.playClick();
    };
    document.getElementById('btn-zoom-out').onclick = () => {
      this.camera.targetZoom = Math.max(this.camera.minZoom, this.camera.targetZoom * 0.8);
      sounds.playClick();
    };
    document.getElementById('btn-center-cam').onclick = () => {
      this.camera.centerOn(950, 490, 0.95);
      sounds.playClick();
    };
    document.getElementById('btn-center-incident').onclick = () => {
      // Centrar en el vehículo más enfadado o en el tramo con conos
      const madCar = this.vehicles.find(v => v.mood === 'road_rage');
      if (madCar) {
        this.camera.centerOn(madCar.x, madCar.y, 1.4);
      } else {
        this.camera.centerOn(950, 490, 1.0);
      }
      sounds.playClick();
    };

    // Botones del Dock Inferior
    document.querySelectorAll('.dock-btn').forEach(btn => {
      btn.onclick = () => {
        selectTool(btn.getAttribute('data-tool'));
        sounds.playClick();
      };
    });

    // Acciones de la Tarjeta de Inspección de Vehículo
    const btnCloseInsp = document.getElementById('btn-close-inspector');
    if (btnCloseInsp) {
      btnCloseInsp.onclick = () => {
        document.getElementById('vehicle-inspector')?.classList.add('hidden');
        GameState.selectedVehicle = null;
        sounds.playClick();
      };
    }

    const btnInspFine = document.getElementById('btn-insp-fine');
    if (btnInspFine) {
      btnInspFine.onclick = () => {
        if (GameState.selectedVehicle) {
          GameState.budget += 200;
          GameState.pegasusInfractions++;
          GameState.pegasusCollected += 200;
          sounds.playCash();
          showToast("Boletín de sanción DGT emitido (+200 €)", 'toast-fine', '⚡');
          floatingTexts.push({ text: "+200 €", x: GameState.selectedVehicle.x, y: GameState.selectedVehicle.y - 20, life: 2.5, alpha: 1.0 });
          GameState.selectedVehicle.frustration = Math.max(0, GameState.selectedVehicle.frustration - 35);
          GameState.selectedVehicle.mood = 'zen';
          addPegasusLogEntry(200, "Sanción DGT manual", Math.round(GameState.selectedVehicle.v * 3.6));
        }
      };
    }

    const btnInspTow = document.getElementById('btn-insp-tow');
    if (btnInspTow) {
      btnInspTow.onclick = () => {
        if (GameState.selectedVehicle) {
          this.dispatchTowTruck({ x: GameState.selectedVehicle.x, y: GameState.selectedVehicle.y });
        }
      };
    }
  }

  // ==========================================================================
  // BUCLE DE ACTUALIZACIÓN (UPDATE)
  // ==========================================================================
  update(dt) {
    this.camera.update(dt);

    // Actualizar tarjeta de inspección activa en tiempo real
    if (GameState.selectedVehicle) {
      if (!this.vehicles.includes(GameState.selectedVehicle)) {
        document.getElementById('vehicle-inspector')?.classList.add('hidden');
        GameState.selectedVehicle = null;
      } else {
        this.updateInspectorCard(GameState.selectedVehicle);
      }
    }

    if (GameState.timeSpeed > 0) {
      const simDt = dt * GameState.timeSpeed;

      // Actualizar carreteras (obras en curso)
      for (const r of this.roads) {
        r.update(simDt);
      }

      // Spawner continuo de vehículos para mantener tráfico vivo
      this.spawnTimer += simDt;
      if (this.spawnTimer > 2.0 && this.vehicles.length < 50 && this.roads.length > 0) {
        this.spawnTimer = 0;
        const availableRoads = this.roads.filter(r => !r.isUnderConstruction);
        if (availableRoads.length > 0) {
          const road = availableRoads[Math.floor(Math.random() * availableRoads.length)];
          const lane = Math.floor(Math.random() * road.lanes);
          const types = ['turismo', 'turismo', 'camion', 'furgoneta'];
          const type = types[Math.floor(Math.random() * types.length)];
          const v = new Vehicle(this.nextVehicleId++, type, road, lane, 0);
          this.vehicles.push(v);
        }
      }

      // Actualizar vehículos
      for (let i = this.vehicles.length - 1; i >= 0; i--) {
        const v = this.vehicles[i];
        v.update(simDt, this.vehicles);

        if (v.sanctionCooldown > 0) v.sanctionCooldown -= simDt;

        // Si el vehículo supera el final de la vía, respawnear al inicio o eliminar
        if (v.distance > v.road.length + 30 || v.distance < -30) {
          if (v.road.type === 'autovia') {
            v.distance = (v.direction === 1) ? 0 : v.road.length;
          } else {
            this.vehicles.splice(i, 1);
          }
        }
      }

      // Actualizar helicóptero Pegasus
      this.pegasus.update(simDt, this.vehicles, this.roads);

      // Actualizar partículas flotantes
      for (let i = floatingTexts.length - 1; i >= 0; i--) {
        const ft = floatingTexts[i];
        ft.y -= 25 * simDt;
        ft.life -= simDt;
        ft.alpha = Math.max(0, ft.life / 2.0);
        if (ft.life <= 0) floatingTexts.splice(i, 1);
      }

      // Métricas y estadísticas globales
      this.calculateMetrics();
    }

    // Actualizar lluvia DANA
    if (GameState.weatherDANA) {
      for (const p of rainParticles) {
        p.y += p.speed * dt;
        p.x -= p.speed * 0.25 * dt;
        if (p.y > this.canvas.height) {
          p.y = -20;
          p.x = Math.random() * (this.canvas.width + 100);
        }
      }
    }
  }

  calculateMetrics() {
    if (this.vehicles.length === 0) {
      GameState.congestion = 0;
      GameState.roadRagePercent = 0;
      GameState.incidents = 0;
      this.updateHUDValues();
      return;
    }

    let slowVehicles = 0;
    let furiousVehicles = 0;
    let activeObstacles = 0;

    for (const v of this.vehicles) {
      const v_kmh = v.v * 3.6;
      if (v_kmh < 25) slowVehicles++;
      if (v.mood === 'road_rage') furiousVehicles++;
    }

    for (const r of this.roads) {
      if (Object.keys(r.blockedLanes).length > 0) activeObstacles++;
      if (r.isUnderConstruction) activeObstacles++;
    }

    GameState.congestion = Math.round((slowVehicles / this.vehicles.length) * 100);
    GameState.roadRagePercent = Math.round((furiousVehicles / this.vehicles.length) * 100);
    GameState.incidents = activeObstacles;

    this.updateHUDValues();
  }

  updateHUDValues() {
    document.getElementById('hud-budget').textContent = `${GameState.budget.toLocaleString()} €`;

    const congEl = document.getElementById('hud-congestion');
    congEl.textContent = `${GameState.congestion}%`;
    congEl.className = 'stat-value ' + (GameState.congestion > 50 ? 'congestion-high' : GameState.congestion > 25 ? 'congestion-med' : 'congestion-low');

    document.getElementById('hud-incidents').textContent = `${GameState.incidents} ` + (GameState.incidents === 1 ? 'Activo' : 'Activos');

    document.getElementById('hud-rage').textContent = `${GameState.roadRagePercent}%`;
    document.getElementById('hud-rage-icon').textContent = GameState.roadRagePercent > 40 ? '😡' : GameState.roadRagePercent > 15 ? '😐' : '🙂';

    document.getElementById('pegasus-infractions').textContent = `${GameState.pegasusInfractions} sancionados`;
    document.getElementById('pegasus-collected').textContent = `+${GameState.pegasusCollected.toLocaleString()} €`;
  }

  // ==========================================================================
  // RENDERIZADO VISUAL 2.5D EN CANVAS
  // ==========================================================================
  render() {
    const ctx = this.ctx;
    const w = this.canvas.width;
    const h = this.canvas.height;

    ctx.clearRect(0, 0, w, h);

    // 1. DIBUJAR FONDO CARTOGRÁFICO DGT / TERRENO
    if (this.bgImageLoaded) {
      ctx.drawImage(this.bgImage, 0, 0, w, h);
      ctx.fillStyle = 'rgba(11, 17, 24, 0.78)';
      ctx.fillRect(0, 0, w, h);
    } else {
      ctx.fillStyle = '#0f1722';
      ctx.fillRect(0, 0, w, h);
    }

    // Guardar contexto para aplicar la transformación de la cámara
    ctx.save();
    ctx.translate(w / 2, h / 2);
    ctx.scale(this.camera.zoom, this.camera.zoom);
    ctx.translate(-this.camera.x, -this.camera.y);

    // 2. CUADRÍCULA DE COORDENADAS DGT ETRS89
    this.renderCoordinateGrid(ctx);

    // 3. RENDERIZADO DE CARRETERAS (Asfalto, Marcas Viales 8.2-IC, Mediana)
    for (const r of this.roads) {
      this.renderRoad(ctx, r);
    }

    // 4. PREVISUALIZACIÓN DE TRAZADO EN CURSO
    if (GameState.isDrawing && GameState.drawStart && GameState.drawCurrent) {
      this.renderDrawPreview(ctx);
    }

    // 5. RENDERIZADO DE VEHÍCULOS (Con sombreado 2.5D, intermitentes y luces)
    for (const v of this.vehicles) {
      this.renderVehicle(ctx, v);
    }

    // 6. RENDERIZADO DEL CONO DE RADAR LÁSER DEL HELICÓPTERO PEGASUS
    if (GameState.pegasusEnabled && this.pegasus.active) {
      this.renderPegasusRadarCone(ctx);
    }

    // 7. RENDERIZADO DEL HELICÓPTERO PEGASUS EN VUELO
    if (GameState.pegasusEnabled && this.pegasus.active) {
      this.renderPegasusSprite(ctx);
    }

    // 8. TEXTOS FLOTANTES DE SANCIONES Y MULTAS (€)
    for (const ft of floatingTexts) {
      ctx.save();
      ctx.font = 'bold 15px monospace';
      ctx.fillStyle = `rgba(52, 211, 153, ${ft.alpha})`;
      ctx.shadowColor = 'rgba(0,0,0,0.8)';
      ctx.shadowBlur = 4;
      ctx.fillText(ft.text, ft.x - 25, ft.y);
      ctx.restore();
    }

    ctx.restore();

    // 9. LLUVIA DANA EN ESPACIO DE PANTALLA
    if (GameState.weatherDANA) {
      this.renderRain(ctx, w, h);
    }
  }

  renderCoordinateGrid(ctx) {
    ctx.save();
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.035)';
    ctx.lineWidth = 1;
    const step = 200;
    const left = this.camera.x - 1200;
    const right = this.camera.x + 1200;
    const top = this.camera.y - 800;
    const bottom = this.camera.y + 800;

    for (let x = Math.floor(left / step) * step; x < right; x += step) {
      ctx.beginPath();
      ctx.moveTo(x, top);
      ctx.lineTo(x, bottom);
      ctx.stroke();
    }
    for (let y = Math.floor(top / step) * step; y < bottom; y += step) {
      ctx.beginPath();
      ctx.moveTo(left, y);
      ctx.lineTo(right, y);
      ctx.stroke();
    }
    ctx.restore();
  }

  renderRoad(ctx, road) {
    ctx.save();

    // Dibujar base de asfalto mediante trazo spline
    ctx.beginPath();
    ctx.moveTo(road.p0.x, road.p0.y);
    ctx.bezierCurveTo(road.p1.x, road.p1.y, road.p2.x, road.p2.y, road.p3.x, road.p3.y);

    // Color del asfalto (más oscuro con brillo si llueve DANA)
    ctx.strokeStyle = GameState.weatherDANA ? '#16191f' : '#222731';
    ctx.lineWidth = road.width;
    ctx.lineCap = 'round';
    ctx.lineJoin = 'round';
    ctx.stroke();

    // Líneas continuas exteriores de borde (Norma 8.2-IC)
    ctx.strokeStyle = 'rgba(240, 240, 240, 0.85)';
    ctx.lineWidth = 1.5;
    ctx.stroke();

    // Dibujar detalles internos por muestras
    if (road.type === 'autovia') {
      // Mediana central separadora con guardarraíles bionda
      ctx.save();
      ctx.strokeStyle = '#0f172a';
      ctx.lineWidth = 6;
      ctx.stroke();

      // Bionda metálica reflectante
      ctx.strokeStyle = '#94a3b8';
      ctx.lineWidth = 2;
      ctx.stroke();
      ctx.restore();

      // Líneas discontinuas de separación de carril en ambos sentidos
      ctx.save();
      ctx.setLineDash([12, 18]);
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.6)';
      ctx.lineWidth = 1.5;

      [-12, 12].forEach(offset => {
        ctx.beginPath();
        for (let i = 0; i < road.samples.length; i++) {
          const s = road.samples[i];
          const tang = road.getTangentAt(s.t);
          const px = s.pt.x - Math.sin(tang.angle) * offset;
          const py = s.pt.y + Math.cos(tang.angle) * offset;
          if (i === 0) ctx.moveTo(px, py);
          else ctx.lineTo(px, py);
        }
        ctx.stroke();
      });
      ctx.restore();

    } else if (road.type === 'autovia_3x3') {
      // Autopista 3x3 de Gran Capacidad
      ctx.save();
      ctx.strokeStyle = '#0f172a';
      ctx.lineWidth = 7;
      ctx.stroke();
      ctx.strokeStyle = '#94a3b8';
      ctx.lineWidth = 2.5;
      ctx.stroke();
      ctx.restore();

      ctx.save();
      ctx.setLineDash([12, 18]);
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.6)';
      ctx.lineWidth = 1.5;

      [-26, -14, 14, 26].forEach(offset => {
        ctx.beginPath();
        for (let i = 0; i < road.samples.length; i++) {
          const s = road.samples[i];
          const tang = road.getTangentAt(s.t);
          const px = s.pt.x - Math.sin(tang.angle) * offset;
          const py = s.pt.y + Math.cos(tang.angle) * offset;
          if (i === 0) ctx.moveTo(px, py);
          else ctx.lineTo(px, py);
        }
        ctx.stroke();
      });
      ctx.restore();

    } else if (road.type === 'toll') {
      // Peaje Troncal con marquesina
      const mid = road.getPointAt(0.5);
      const tang = road.getTangentAt(0.5);
      ctx.save();
      ctx.translate(mid.x, mid.y);
      ctx.rotate(tang.angle);
      ctx.fillStyle = '#0284c7';
      ctx.fillRect(-22, -road.width / 2 - 8, 44, road.width + 16);
      ctx.strokeStyle = '#38bdf8';
      ctx.lineWidth = 2;
      ctx.strokeRect(-22, -road.width / 2 - 8, 44, road.width + 16);
      ctx.font = 'bold 9px sans-serif';
      ctx.fillStyle = '#fff';
      ctx.textAlign = 'center';
      ctx.fillText('PEAJE 3.50 €', 0, -road.width / 2 - 12);
      ctx.restore();

    } else if (road.type === 'railway') {
      // Paso a Nivel y Vía Férrea transversal
      const mid = road.getPointAt(0.5);
      const tang = road.getTangentAt(0.5);
      ctx.save();
      ctx.translate(mid.x, mid.y);
      ctx.rotate(tang.angle + Math.PI / 2);

      // Balasto
      ctx.fillStyle = '#475569';
      ctx.fillRect(-100, -10, 200, 20);

      // Traviesas de madera
      ctx.fillStyle = '#78350f';
      for (let tx = -95; tx <= 95; tx += 14) {
        ctx.fillRect(tx, -9, 7, 18);
      }

      // Raíles
      ctx.strokeStyle = '#e2e8f0';
      ctx.lineWidth = 2.5;
      ctx.beginPath();
      ctx.moveTo(-100, -5); ctx.lineTo(100, -5);
      ctx.moveTo(-100, 5); ctx.lineTo(100, 5);
      ctx.stroke();

      // Semibarreras abatibles si está bajada
      if (road.barrierLowered) {
        ctx.strokeStyle = '#ef4444';
        ctx.lineWidth = 4;
        ctx.setLineDash([8, 8]);
        ctx.beginPath();
        ctx.moveTo(-35, -14); ctx.lineTo(35, -14);
        ctx.stroke();
        ctx.setLineDash([]);

        const flash = Math.sin(Date.now() * 0.015) > 0;
        ctx.fillStyle = flash ? '#ef4444' : '#7f1d1d';
        ctx.beginPath();
        ctx.arc(-35, -14, 4, 0, Math.PI * 2);
        ctx.arc(35, -14, 4, 0, Math.PI * 2);
        ctx.fill();

        // Tren de mercancías cruzando
        const trainTime = Date.now() * 0.12;
        const trainX = ((trainTime % 700) - 350);
        ctx.save();
        ctx.translate(trainX, 0);
        ctx.fillStyle = '#dc2626';
        ctx.fillRect(-35, -7, 32, 14);
        ctx.fillStyle = '#fef08a';
        ctx.fillRect(-6, -4, 4, 8);
        const vagColors = ['#2563eb', '#16a34a', '#d97706', '#9333ea'];
        vagColors.forEach((col, idx) => {
          ctx.fillStyle = col;
          ctx.fillRect(-70 - idx * 34, -6, 28, 12);
        });
        ctx.restore();
      }
      ctx.restore();

    } else if (road.type === 'convencional') {
      ctx.save();
      ctx.setLineDash([8, 14]);
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.65)';
      ctx.lineWidth = 1.8;
      ctx.stroke();
      ctx.restore();
    }

    // Dibujar elementos de Obras en tiempo real si está en construcción
    if (road.isUnderConstruction) {
      this.renderConstructionSite(ctx, road);
    }

    // Dibujar conos de corte si hay carriles bloqueados (test atascos)
    for (const [laneStr, isBlocked] of Object.entries(road.blockedLanes)) {
      if (isBlocked) {
        const lane = parseInt(laneStr, 10);
        const midSample = road.getPointAtDistance(road.length * 0.5);
        const tang = midSample.tangent;
        const offset = road.getLaneOffset(lane);
        const cx = midSample.pt.x - Math.sin(tang.angle) * offset;
        const cy = midSample.pt.y + Math.cos(tang.angle) * offset;

        // Furgoneta de mantenimiento o balizamiento
        ctx.fillStyle = '#f97316';
        ctx.beginPath();
        ctx.arc(cx, cy, 7, 0, Math.PI * 2);
        ctx.fill();

        // Destello ámbar de emergencia
        const flash = Math.sin(Date.now() * 0.01) > 0;
        ctx.fillStyle = flash ? '#facc15' : '#ef4444';
        ctx.beginPath();
        ctx.arc(cx, cy, 3, 0, Math.PI * 2);
        ctx.fill();
      }
    }

    ctx.restore();
  }

  renderConstructionSite(ctx, road) {
    ctx.save();
    // Hilera de conos reflectantes naranjas a lo largo de la calzada
    const coneSpacing = 40;
    const numCones = Math.floor(road.length / coneSpacing);

    for (let i = 1; i < numCones; i++) {
      const sample = road.getPointAtDistance(i * coneSpacing);
      const tang = sample.tangent;
      [-14, 14].forEach(offset => {
        const cx = sample.pt.x - Math.sin(tang.angle) * offset;
        const cy = sample.pt.y + Math.cos(tang.angle) * offset;

        // Cono naranja
        ctx.fillStyle = '#ea580c';
        ctx.beginPath();
        ctx.arc(cx, cy, 4, 0, Math.PI * 2);
        ctx.fill();
        // Franja blanca reflectante
        ctx.fillStyle = '#ffffff';
        ctx.beginPath();
        ctx.arc(cx, cy, 2, 0, Math.PI * 2);
        ctx.fill();
      });
    }

    // Cartel y barra de progreso de obra en el centro del tramo
    const mid = road.getPointAt(0.5);
    const pct = Math.round(road.constructionProgress * 100);
    const timeLeft = Math.max(0, (road.constructionDuration * (1 - road.constructionProgress))).toFixed(1);

    ctx.fillStyle = 'rgba(15, 23, 42, 0.9)';
    ctx.strokeStyle = '#f59e0b';
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    ctx.roundRect(mid.x - 70, mid.y - 28, 140, 36, 6);
    ctx.fill();
    ctx.stroke();

    ctx.font = 'bold 10px sans-serif';
    ctx.fillStyle = '#f59e0b';
    ctx.textAlign = 'center';
    ctx.fillText(`🚧 OBRAS: ${timeLeft}s (${pct}%)`, mid.x, mid.y - 12);

    // Barra de progreso interior
    ctx.fillStyle = 'rgba(255,255,255,0.15)';
    ctx.fillRect(mid.x - 60, mid.y - 4, 120, 6);
    ctx.fillStyle = '#10b981';
    ctx.fillRect(mid.x - 60, mid.y - 4, 120 * road.constructionProgress, 6);

    ctx.restore();
  }

  renderDrawPreview(ctx) {
    ctx.save();
    ctx.beginPath();
    ctx.moveTo(GameState.drawStart.x, GameState.drawStart.y);
    ctx.lineTo(GameState.drawCurrent.x, GameState.drawCurrent.y);
    ctx.setLineDash([8, 8]);
    ctx.strokeStyle = '#00c0f3';
    ctx.lineWidth = 32;
    ctx.stroke();

    const d = Math.round(Math.hypot(GameState.drawCurrent.x - GameState.drawStart.x, GameState.drawCurrent.y - GameState.drawStart.y));
    const costPerMeter = GameState.activeTool === 'autovia_3x3' ? 1200 : GameState.activeTool === 'autovia' ? 850 : GameState.activeTool === 'toll' ? 1500 : GameState.activeTool === 'railway' ? 950 : 450;
    const cost = d * costPerMeter;
    const toolLabel = GameState.activeTool === 'autovia_3x3' ? 'Autopista 3x3' : GameState.activeTool === 'autovia' ? 'Autovía 2x2' : GameState.activeTool === 'toll' ? 'Peaje' : GameState.activeTool === 'railway' ? 'Paso a Nivel' : 'Vía';

    ctx.font = 'bold 13px monospace';
    ctx.fillStyle = '#fff';
    ctx.shadowColor = '#000';
    ctx.shadowBlur = 4;
    ctx.fillText(`${toolLabel}: ${d} m | ${cost.toLocaleString()} €`, GameState.drawCurrent.x + 15, GameState.drawCurrent.y - 15);
    ctx.restore();
  }

  renderVehicle(ctx, v) {
    ctx.save();
    ctx.translate(v.x, v.y);
    ctx.rotate(v.angle);

    // 1. Sombra arrojada 2.5D
    ctx.fillStyle = 'rgba(0, 0, 0, 0.45)';
    ctx.beginPath();
    ctx.roundRect(-v.length / 2 + 2, -v.width / 2 + 3, v.length, v.width, 2);
    ctx.fill();

    // 2. Chasis del vehículo
    ctx.fillStyle = v.color;
    ctx.beginPath();
    ctx.roundRect(-v.length / 2, -v.width / 2, v.length, v.width, 2.5);
    ctx.fill();

    // 3. Luna delantera y trasera
    ctx.fillStyle = '#1e293b';
    ctx.fillRect(-v.length * 0.1, -v.width * 0.4, v.length * 0.35, v.width * 0.8);

    // 4. Luces delanteras (faros)
    ctx.fillStyle = '#fef08a';
    ctx.fillRect(v.length / 2 - 1, -v.width / 2 + 1, 1.5, 2);
    ctx.fillRect(v.length / 2 - 1, v.width / 2 - 3, 1.5, 2);

    // 5. Luces traseras de frenado
    const isBraking = v.acc < -0.4;
    ctx.fillStyle = isBraking ? '#ef4444' : '#991b1b';
    ctx.fillRect(-v.length / 2, -v.width / 2 + 1, 1.5, 2);
    ctx.fillRect(-v.length / 2, v.width / 2 - 3, 1.5, 2);

    // 6. Rotativos azules de la Guardia Civil
    if (v.isPolice) {
      const flashLeft = Math.floor(v.sirenFlash) === 0;
      ctx.fillStyle = flashLeft ? '#00e5ff' : '#002b80';
      ctx.fillRect(-2, -v.width / 2 + 1, 4, 2.5);
      ctx.fillStyle = !flashLeft ? '#00e5ff' : '#002b80';
      ctx.fillRect(-2, v.width / 2 - 3.5, 4, 2.5);
    }

    ctx.restore();

    // 7. EMOTICONO Y PSICOLOGÍA DEL CONDUCTOR (Bocina y estado de furia)
    if (GameState.rageEnabled && (v.mood === 'road_rage' || v.mood === 'impatient')) {
      ctx.save();
      const emoji = v.mood === 'road_rage' ? '😡' : '😐';
      ctx.font = '16px sans-serif';
      ctx.textAlign = 'center';
      ctx.shadowColor = 'rgba(0,0,0,0.7)';
      ctx.shadowBlur = 4;
      ctx.fillText(emoji, v.x, v.y - 18);

      // Onda acústica si está pitando con el claxon
      if (v.isHonking) {
        ctx.strokeStyle = 'rgba(239, 68, 68, 0.7)';
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.arc(v.x, v.y, 22, 0, Math.PI * 2);
        ctx.stroke();
      }
      ctx.restore();
    }
  }

  renderPegasusRadarCone(ctx) {
    const peg = this.pegasus;
    ctx.save();

    // Gradiente radial del haz cónico sobre la calzada
    const grad = ctx.createRadialGradient(peg.x, peg.y, 10, peg.x, peg.y, peg.scanRadius);
    grad.addColorStop(0, 'rgba(0, 240, 255, 0.35)');
    grad.addColorStop(0.7, 'rgba(0, 192, 243, 0.15)');
    grad.addColorStop(1, 'rgba(0, 192, 243, 0.0)');

    ctx.fillStyle = grad;
    ctx.beginPath();
    ctx.arc(peg.x, peg.y, peg.scanRadius, 0, Math.PI * 2);
    ctx.fill();

    // Anillos concéntricos de pulso radar
    ctx.strokeStyle = 'rgba(0, 240, 255, 0.4)';
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    ctx.arc(peg.x, peg.y, peg.scanRadius * peg.scanPulse, 0, Math.PI * 2);
    ctx.stroke();

    // Retícula de mira fija si está siguiendo a un infractor
    if (peg.trackedVehicle) {
      const tv = peg.trackedVehicle;
      ctx.strokeStyle = '#ef4444';
      ctx.lineWidth = 2;
      ctx.strokeRect(tv.x - 18, tv.y - 14, 36, 28);

      ctx.fillStyle = '#ef4444';
      ctx.font = 'bold 11px monospace';
      ctx.fillText(`RADAR LOCK [${Math.round(tv.v * 3.6)} km/h]`, tv.x - 30, tv.y - 20);
    }

    ctx.restore();
  }

  renderPegasusSprite(ctx) {
    const peg = this.pegasus;
    ctx.save();
    ctx.translate(peg.x, peg.y);

    // Sombra del helicóptero arrojada en el suelo
    ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
    ctx.beginPath();
    ctx.ellipse(30, 45, 24, 12, 0, 0, Math.PI * 2);
    ctx.fill();

    // Fuselaje del Pegasus (Eurocopter EC135 azul y amarillo DGT)
    ctx.fillStyle = '#004b87';
    ctx.beginPath();
    ctx.roundRect(-24, -10, 48, 20, 8);
    ctx.fill();

    // Franja amarilla reflectante
    ctx.fillStyle = '#facc15';
    ctx.fillRect(-18, -3, 36, 6);

    // Cabina acristalada
    ctx.fillStyle = '#38bdf8';
    ctx.beginPath();
    ctx.arc(14, 0, 7, -Math.PI / 2, Math.PI / 2);
    ctx.fill();

    // Rótulo DGT
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 8px sans-serif';
    ctx.fillText('DGT', -8, 3);

    // Aspas del rotor girando
    ctx.save();
    ctx.rotate(peg.rotorAngle);
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.7)';
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.moveTo(-45, 0);
    ctx.lineTo(45, 0);
    ctx.stroke();
    ctx.restore();

    ctx.restore();
  }

  renderRain(ctx, w, h) {
    ctx.save();
    ctx.strokeStyle = 'rgba(186, 230, 253, 0.45)';
    ctx.lineWidth = 1.2;
    ctx.beginPath();
    for (const p of rainParticles) {
      ctx.moveTo(p.x, p.y);
      ctx.lineTo(p.x - p.len * 0.25, p.y + p.len);
    }
    ctx.stroke();
    ctx.restore();
  }

  // ==========================================================================
  // LOOP PRINCIPAL (REQUEST ANIMATION FRAME)
  // ==========================================================================
  start() {
    const loop = (currentTime) => {
      const dt = Math.min(0.1, (currentTime - this.lastTime) / 1000);
      this.lastTime = currentTime;

      this.update(dt);
      this.render();

      requestAnimationFrame(loop);
    };
    requestAnimationFrame(loop);
  }
}

// ============================================================================
// 10. FUNCIONES AUXILIARES GLOBALES
// ============================================================================
function selectTool(toolName) {
  GameState.activeTool = toolName;
  document.querySelectorAll('.dock-btn').forEach(btn => {
    btn.classList.toggle('active', btn.getAttribute('data-tool') === toolName);
  });
}

function showToast(message, typeClass = 'toast-works', icon = 'ℹ️') {
  const container = document.getElementById('toast-container');
  const toast = document.createElement('div');
  toast.className = `toast ${typeClass}`;
  toast.innerHTML = `<span class="toast-icon">${icon}</span><div class="toast-text">${message}</div>`;
  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateX(100%)';
    toast.style.transition = 'all 0.3s ease';
    setTimeout(() => toast.remove(), 300);
  }, 4500);
}

function addPegasusLogEntry(amount, reason, speed) {
  const log = document.getElementById('pegasus-log');
  const entry = document.createElement('div');
  entry.className = 'fine-entry';
  entry.innerHTML = `<span><strong>+${amount} €</strong> ${reason}</span><span style="font-weight:700;">${speed} km/h</span>`;
  log.prepend(entry);

  if (log.children.length > 5) {
    log.removeChild(log.lastChild);
  }
}

// Inicialización automática tras cargar DOM
window.addEventListener('DOMContentLoaded', () => {
  updateUITranslations();
  const engine = new GameEngine();
  engine.start();
});
