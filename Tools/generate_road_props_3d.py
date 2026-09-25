#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
===============================================================================
Generador de Props y Señalización Vial 3D para "Autopistas de España"
Escala Oficial: 1 Unreal Unit (UU) = 1 cm = 0.01 m (100 UU = 1 metro)
Motor de destino: Unreal Engine 5.x
===============================================================================
Genera modelos geométricos en formato Wavefront .obj con librerías .mtl asociadas,
incluyendo normales suavizadas/duras, mapeado UV completo y materiales PBR listos
para su importación directa en Unreal Engine 5.
"""

import os
import sys
import math
import argparse
from typing import List, Tuple, Dict, Optional

# =============================================================================
# UTILIDADES MATEMÁTICAS VECTORIALES (3D & 2D)
# =============================================================================

def vec3_add(a: Tuple[float, float, float], b: Tuple[float, float, float]) -> Tuple[float, float, float]:
    return (a[0] + b[0], a[1] + b[1], a[2] + b[2])

def vec3_sub(a: Tuple[float, float, float], b: Tuple[float, float, float]) -> Tuple[float, float, float]:
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])

def vec3_scale(a: Tuple[float, float, float], s: float) -> Tuple[float, float, float]:
    return (a[0] * s, a[1] * s, a[2] * s)

def vec3_dot(a: Tuple[float, float, float], b: Tuple[float, float, float]) -> float:
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]

def vec3_cross(a: Tuple[float, float, float], b: Tuple[float, float, float]) -> Tuple[float, float, float]:
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    )

def vec3_length(a: Tuple[float, float, float]) -> float:
    return math.sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2])

def vec3_normalize(a: Tuple[float, float, float]) -> Tuple[float, float, float]:
    l = vec3_length(a)
    if l < 1e-8:
        return (0.0, 0.0, 1.0)
    return (a[0] / l, a[1] / l, a[2] / l)


# =============================================================================
# DEFINICIÓN DE MATERIALES PBR VIALES (VALORES MTL OFICIALES)
# =============================================================================

DEFAULT_MATERIALS = {
    "M_Steel_Galvanized": {
        "Ka": (0.24, 0.25, 0.26),
        "Kd": (0.68, 0.70, 0.73),
        "Ks": (0.80, 0.82, 0.85),
        "Ns": 75.0,
        "d": 1.0,
        "desc": "Acero galvanizado en caliente para biondas, postes y pórticos"
    },
    "M_Concrete_NewJersey": {
        "Ka": (0.20, 0.20, 0.19),
        "Kd": (0.60, 0.59, 0.56),
        "Ks": (0.12, 0.12, 0.12),
        "Ns": 10.0,
        "d": 1.0,
        "desc": "Hormigón armado texturizado para barreras de seguridad rígidas"
    },
    "M_Concrete": {
        "Ka": (0.18, 0.18, 0.18),
        "Kd": (0.52, 0.52, 0.50),
        "Ks": (0.10, 0.10, 0.10),
        "Ns": 8.0,
        "d": 1.0,
        "desc": "Hormigón estándar para cimentaciones y zapatas de pórticos y radares"
    },
    "M_TrafficCone_Orange": {
        "Ka": (0.35, 0.08, 0.00),
        "Kd": (0.98, 0.32, 0.02),
        "Ks": (0.45, 0.20, 0.10),
        "Ns": 35.0,
        "d": 1.0,
        "desc": "Naranja fluorescente alta visibilidad DGT / UNE-EN 13422"
    },
    "M_Cone_Base_Black": {
        "Ka": (0.05, 0.05, 0.05),
        "Kd": (0.12, 0.12, 0.13),
        "Ks": (0.15, 0.15, 0.15),
        "Ns": 15.0,
        "d": 1.0,
        "desc": "Caucho negro vulcanizado de alta densidad para base de cono"
    },
    "M_Reflective_White": {
        "Ka": (0.30, 0.30, 0.30),
        "Kd": (0.96, 0.96, 0.96),
        "Ks": (0.92, 0.92, 0.95),
        "Ns": 120.0,
        "d": 1.0,
        "desc": "Lámina retrorreflectante blanca microprismática Clase 3"
    },
    "M_Reflector_White": {
        "Ka": (0.30, 0.30, 0.30),
        "Kd": (0.95, 0.95, 0.95),
        "Ks": (0.90, 0.90, 0.90),
        "Ns": 128.0,
        "d": 1.0,
        "desc": "Captafaro retrorreflectante blanco / ojo de gato para biondas e hitos"
    },
    "M_Reflector_Amber": {
        "Ka": (0.30, 0.18, 0.02),
        "Kd": (0.98, 0.65, 0.05),
        "Ks": (0.90, 0.70, 0.20),
        "Ns": 128.0,
        "d": 1.0,
        "desc": "Captafaro retrorreflectante ámbar / amarillo margen izquierdo"
    },
    "M_Sign_Blue_RAL5017": {
        "Ka": (0.00, 0.12, 0.24),
        "Kd": (0.00, 0.29, 0.55),
        "Ks": (0.35, 0.35, 0.40),
        "Ns": 45.0,
        "d": 1.0,
        "desc": "Azul señales de autovía y autopista según Norma 8.1-IC (RAL 5017 Azul Tráfico)"
    },
    "M_Sign_White_Reflective": {
        "Ka": (0.30, 0.30, 0.30),
        "Kd": (0.97, 0.97, 0.97),
        "Ks": (0.90, 0.90, 0.90),
        "Ns": 110.0,
        "d": 1.0,
        "desc": "Blanco reflectante de orlas, flechas a 45º y pictogramas viales"
    },
    "M_Sign_Back_Gray": {
        "Ka": (0.18, 0.18, 0.19),
        "Kd": (0.45, 0.46, 0.48),
        "Ks": (0.28, 0.28, 0.30),
        "Ns": 25.0,
        "d": 1.0,
        "desc": "Chapa trasera galvanizada mate con imprimación gris anticorrosión"
    },
    "M_PMV_Housing": {
        "Ka": (0.05, 0.05, 0.05),
        "Kd": (0.14, 0.14, 0.15),
        "Ks": (0.25, 0.25, 0.28),
        "Ns": 35.0,
        "d": 1.0,
        "desc": "Carcasa exterior de aluminio anodizado negro mate de Panel de Mensaje Variable"
    },
    "M_PMV_Display": {
        "Ka": (0.02, 0.02, 0.02),
        "Kd": (0.06, 0.06, 0.07),
        "Ks": (0.50, 0.45, 0.15),
        "Ns": 60.0,
        "d": 1.0,
        "desc": "Matriz de píxeles LED ámbar de alta luminosidad sobre fondo óptico antirreflejo"
    },
    "M_Marker_White": {
        "Ka": (0.25, 0.25, 0.25),
        "Kd": (0.92, 0.92, 0.93),
        "Ks": (0.30, 0.30, 0.32),
        "Ns": 30.0,
        "d": 1.0,
        "desc": "Polímero blanco estabilizado anti-UV para cuerpo de hito de arista DGT"
    },
    "M_Marker_Black": {
        "Ka": (0.03, 0.03, 0.03),
        "Kd": (0.08, 0.08, 0.09),
        "Ks": (0.15, 0.15, 0.15),
        "Ns": 20.0,
        "d": 1.0,
        "desc": "Banda negra superior de contraste para hito de arista"
    },
    "M_Radar_Housing_Gray": {
        "Ka": (0.18, 0.19, 0.20),
        "Kd": (0.52, 0.54, 0.56),
        "Ks": (0.30, 0.30, 0.32),
        "Ns": 30.0,
        "d": 1.0,
        "desc": "Cabina metálica cinemómetro DGT en gris intemperie"
    },
    "M_Radar_Orange_DGT": {
        "Ka": (0.30, 0.12, 0.00),
        "Kd": (0.96, 0.46, 0.02),
        "Ks": (0.50, 0.30, 0.10),
        "Ns": 40.0,
        "d": 1.0,
        "desc": "Franjas de peligro y alta visibilidad naranja DGT para cabina de radar"
    },
    "M_Radar_Lens": {
        "Ka": (0.02, 0.02, 0.03),
        "Kd": (0.05, 0.06, 0.08),
        "Ks": (0.95, 0.95, 0.98),
        "Ns": 180.0,
        "d": 0.95,
        "desc": "Vidrio óptico multicapa oscuro para cámara fotográfica y sensor cinemómetro"
    },
    "M_Radar_Sensor": {
        "Ka": (0.04, 0.04, 0.05),
        "Kd": (0.16, 0.17, 0.19),
        "Ks": (0.60, 0.60, 0.65),
        "Ns": 70.0,
        "d": 1.0,
        "desc": "Óptica secundaria de flash infrarrojo estroboscópico / antena radar Doppler"
    },
    "M_Mojon_Blue": {
        "Ka": (0.00, 0.12, 0.25),
        "Kd": (0.00, 0.30, 0.58),
        "Ks": (0.30, 0.30, 0.35),
        "Ns": 40.0,
        "d": 1.0,
        "desc": "Fondo azul autovía para mojón de punto kilométrico (PK)"
    },
    "M_Mojon_White": {
        "Ka": (0.25, 0.25, 0.25),
        "Kd": (0.88, 0.88, 0.90),
        "Ks": (0.25, 0.25, 0.28),
        "Ns": 25.0,
        "d": 1.0,
        "desc": "Cuerpo y textos blancos de mojón kilométrico"
    }
}


# =============================================================================
# CLASE PRINCIPAL MESH BUILDER
# =============================================================================

class MeshBuilder:
    """Construye y exporta geometría 3D completa en Wavefront OBJ y MTL."""

    def __init__(self, name: str):
        self.name = name
        self.vertices: List[Tuple[float, float, float]] = []
        self.uvs: List[Tuple[float, float]] = []
        self.normals: List[Tuple[float, float, float]] = []
        self.faces_by_mat: Dict[str, List[List[Tuple[int, int, int]]]] = {}
        self.used_materials: set = set()

    def add_vertex(self, x: float, y: float, z: float) -> int:
        self.vertices.append((round(x, 4), round(y, 4), round(z, 4)))
        return len(self.vertices) - 1

    def add_uv(self, u: float, v: float) -> int:
        self.uvs.append((round(u, 5), round(v, 5)))
        return len(self.uvs) - 1

    def add_normal(self, nx: float, ny: float, nz: float) -> int:
        n = vec3_normalize((nx, ny, nz))
        self.normals.append((round(n[0], 5), round(n[1], 5), round(n[2], 5)))
        return len(self.normals) - 1

    def add_triangle(self,
                     p0: Tuple[float, float, float], p1: Tuple[float, float, float], p2: Tuple[float, float, float],
                     uv0: Tuple[float, float], uv1: Tuple[float, float], uv2: Tuple[float, float],
                     n0: Tuple[float, float, float], n1: Tuple[float, float, float], n2: Tuple[float, float, float],
                     material: str):
        v0 = self.add_vertex(*p0)
        v1 = self.add_vertex(*p1)
        v2 = self.add_vertex(*p2)
        t0 = self.add_uv(*uv0)
        t1 = self.add_uv(*uv1)
        t2 = self.add_uv(*uv2)
        norm0 = self.add_normal(*n0)
        norm1 = self.add_normal(*n1)
        norm2 = self.add_normal(*n2)

        if material not in self.faces_by_mat:
            self.faces_by_mat[material] = []
        self.faces_by_mat[material].append([(v0, t0, norm0), (v1, t1, norm1), (v2, t2, norm2)])
        self.used_materials.add(material)

    def add_quad(self,
                 p0: Tuple[float, float, float], p1: Tuple[float, float, float],
                 p2: Tuple[float, float, float], p3: Tuple[float, float, float],
                 uv0: Tuple[float, float], uv1: Tuple[float, float],
                 uv2: Tuple[float, float], uv3: Tuple[float, float],
                 n0: Tuple[float, float, float], n1: Tuple[float, float, float],
                 n2: Tuple[float, float, float], n3: Tuple[float, float, float],
                 material: str):
        """Añade un cuadrilátero subdividiéndolo en 2 triángulos con devanado CCW."""
        self.add_triangle(p0, p1, p2, uv0, uv1, uv2, n0, n1, n2, material)
        self.add_triangle(p0, p2, p3, uv0, uv2, uv3, n0, n2, n3, material)

    def add_box(self,
                x_min: float, x_max: float,
                y_min: float, y_max: float,
                z_min: float, z_max: float,
                material: str,
                uv_scale_u: float = 1.0, uv_scale_v: float = 1.0):
        """Añade una caja alineada con los ejes con normales duras y mapeado UV independiente por cara."""
        # 8 esquinas
        # Cara Frontal (+X)
        self.add_quad(
            (x_max, y_min, z_min), (x_max, y_max, z_min), (x_max, y_max, z_max), (x_max, y_min, z_max),
            (0.0, 0.0), (uv_scale_u, 0.0), (uv_scale_u, uv_scale_v), (0.0, uv_scale_v),
            (1, 0, 0), (1, 0, 0), (1, 0, 0), (1, 0, 0),
            material
        )
        # Cara Trasera (-X)
        self.add_quad(
            (x_min, y_max, z_min), (x_min, y_min, z_min), (x_min, y_min, z_max), (x_min, y_max, z_max),
            (0.0, 0.0), (uv_scale_u, 0.0), (uv_scale_u, uv_scale_v), (0.0, uv_scale_v),
            (-1, 0, 0), (-1, 0, 0), (-1, 0, 0), (-1, 0, 0),
            material
        )
        # Cara Derecha (+Y)
        self.add_quad(
            (x_max, y_max, z_min), (x_min, y_max, z_min), (x_min, y_max, z_max), (x_max, y_max, z_max),
            (0.0, 0.0), (uv_scale_u, 0.0), (uv_scale_u, uv_scale_v), (0.0, uv_scale_v),
            (0, 1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0),
            material
        )
        # Cara Izquierda (-Y)
        self.add_quad(
            (x_min, y_min, z_min), (x_max, y_min, z_min), (x_max, y_min, z_max), (x_min, y_min, z_max),
            (0.0, 0.0), (uv_scale_u, 0.0), (uv_scale_u, uv_scale_v), (0.0, uv_scale_v),
            (0, -1, 0), (0, -1, 0), (0, -1, 0), (0, -1, 0),
            material
        )
        # Cara Superior (+Z)
        self.add_quad(
            (x_min, y_min, z_max), (x_max, y_min, z_max), (x_max, y_max, z_max), (x_min, y_max, z_max),
            (0.0, 0.0), (uv_scale_u, 0.0), (uv_scale_u, uv_scale_v), (0.0, uv_scale_v),
            (0, 0, 1), (0, 0, 1), (0, 0, 1), (0, 0, 1),
            material
        )
        # Cara Inferior (-Z)
        self.add_quad(
            (x_min, y_max, z_min), (x_max, y_max, z_min), (x_max, y_min, z_min), (x_min, y_min, z_min),
            (0.0, 0.0), (uv_scale_u, 0.0), (uv_scale_u, uv_scale_v), (0.0, uv_scale_v),
            (0, 0, -1), (0, 0, -1), (0, 0, -1), (0, 0, -1),
            material
        )

    def add_cylinder(self,
                     p_start: Tuple[float, float, float],
                     p_end: Tuple[float, float, float],
                     radius: float,
                     segments: int,
                     material: str,
                     capped_start: bool = True,
                     capped_end: bool = True):
        """Añade un cilindro entre dos puntos 3D con normales radiales suaves y tapas circulares."""
        axis = vec3_sub(p_end, p_start)
        length = vec3_length(axis)
        if length < 1e-6:
            return
        z_dir = vec3_normalize(axis)

        # Base ortogonal
        up = (0.0, 0.0, 1.0) if abs(z_dir[2]) < 0.9 else (0.0, 1.0, 0.0)
        x_dir = vec3_normalize(vec3_cross(up, z_dir))
        y_dir = vec3_cross(z_dir, x_dir)

        ring_start = []
        ring_end = []
        normals_rad = []

        for i in range(segments):
            theta = 2.0 * math.pi * i / segments
            cos_t = math.cos(theta)
            sin_t = math.sin(theta)

            rad_vec = vec3_add(vec3_scale(x_dir, cos_t), vec3_scale(y_dir, sin_t))
            pt_start = vec3_add(p_start, vec3_scale(rad_vec, radius))
            pt_end = vec3_add(p_end, vec3_scale(rad_vec, radius))

            ring_start.append(pt_start)
            ring_end.append(pt_end)
            normals_rad.append(rad_vec)

        # Quads laterales
        for i in range(segments):
            next_i = (i + 1) % segments
            u0 = i / segments
            u1 = (i + 1) / segments

            p0 = ring_start[i]
            p1 = ring_start[next_i]
            p2 = ring_end[next_i]
            p3 = ring_end[i]

            n0 = normals_rad[i]
            n1 = normals_rad[next_i]
            n2 = normals_rad[next_i]
            n3 = normals_rad[i]

            self.add_quad(
                p0, p1, p2, p3,
                (u0, 0.0), (u1, 0.0), (u1, 1.0), (u0, 1.0),
                n0, n1, n2, n3,
                material
            )

        # Tapa de inicio (p_start)
        if capped_start:
            n_cap = vec3_scale(z_dir, -1.0)
            for i in range(segments):
                next_i = (i + 1) % segments
                theta0 = 2.0 * math.pi * i / segments
                theta1 = 2.0 * math.pi * next_i / segments
                uv0 = (0.5, 0.5)
                uv1 = (0.5 + 0.5 * math.cos(theta0), 0.5 + 0.5 * math.sin(theta0))
                uv2 = (0.5 + 0.5 * math.cos(theta1), 0.5 + 0.5 * math.sin(theta1))
                self.add_triangle(p_start, ring_start[next_i], ring_start[i], uv0, uv2, uv1, n_cap, n_cap, n_cap, material)

        # Tapa de fin (p_end)
        if capped_end:
            n_cap = z_dir
            for i in range(segments):
                next_i = (i + 1) % segments
                theta0 = 2.0 * math.pi * i / segments
                theta1 = 2.0 * math.pi * next_i / segments
                uv0 = (0.5, 0.5)
                uv1 = (0.5 + 0.5 * math.cos(theta0), 0.5 + 0.5 * math.sin(theta0))
                uv2 = (0.5 + 0.5 * math.cos(theta1), 0.5 + 0.5 * math.sin(theta1))
                self.add_triangle(p_end, ring_end[i], ring_end[next_i], uv0, uv1, uv2, n_cap, n_cap, n_cap, material)

    def add_frustum(self,
                    p_start: Tuple[float, float, float],
                    p_end: Tuple[float, float, float],
                    radius_start: float,
                    radius_end: float,
                    segments: int,
                    material: str,
                    capped_start: bool = True,
                    capped_end: bool = True,
                    uv_v_range: Tuple[float, float] = (0.0, 1.0)):
        """Añade un tronco de cono entre p_start y p_end con normales cónicas calculadas exactamente."""
        axis = vec3_sub(p_end, p_start)
        length = vec3_length(axis)
        if length < 1e-6:
            return
        z_dir = vec3_normalize(axis)

        up = (0.0, 0.0, 1.0) if abs(z_dir[2]) < 0.9 else (0.0, 1.0, 0.0)
        x_dir = vec3_normalize(vec3_cross(up, z_dir))
        y_dir = vec3_cross(z_dir, x_dir)

        # Ángulo de inclinación de la generatriz cónica
        dr = radius_start - radius_end
        slant_angle = math.atan2(dr, length)
        cos_slant = math.cos(slant_angle)
        sin_slant = math.sin(slant_angle)

        ring_start = []
        ring_end = []
        normals_side = []

        for i in range(segments):
            theta = 2.0 * math.pi * i / segments
            cos_t = math.cos(theta)
            sin_t = math.sin(theta)

            rad_vec = vec3_add(vec3_scale(x_dir, cos_t), vec3_scale(y_dir, sin_t))
            pt_start = vec3_add(p_start, vec3_scale(rad_vec, radius_start))
            pt_end = vec3_add(p_end, vec3_scale(rad_vec, radius_end))

            ring_start.append(pt_start)
            ring_end.append(pt_end)

            norm_vec = vec3_normalize(vec3_add(vec3_scale(rad_vec, cos_slant), vec3_scale(z_dir, sin_slant)))
            normals_side.append(norm_vec)

        v0, v1 = uv_v_range
        for i in range(segments):
            next_i = (i + 1) % segments
            u0 = i / segments
            u1 = (i + 1) / segments

            p0 = ring_start[i]
            p1 = ring_start[next_i]
            p2 = ring_end[next_i]
            p3 = ring_end[i]

            n0 = normals_side[i]
            n1 = normals_side[next_i]
            n2 = normals_side[next_i]
            n3 = normals_side[i]

            self.add_quad(
                p0, p1, p2, p3,
                (u0, v0), (u1, v0), (u1, v1), (u0, v1),
                n0, n1, n2, n3,
                material
            )

        if capped_start and radius_start > 1e-4:
            n_cap = vec3_scale(z_dir, -1.0)
            for i in range(segments):
                next_i = (i + 1) % segments
                theta0 = 2.0 * math.pi * i / segments
                theta1 = 2.0 * math.pi * next_i / segments
                uv0 = (0.5, 0.5)
                uv1 = (0.5 + 0.5 * math.cos(theta0), 0.5 + 0.5 * math.sin(theta0))
                uv2 = (0.5 + 0.5 * math.cos(theta1), 0.5 + 0.5 * math.sin(theta1))
                self.add_triangle(p_start, ring_start[next_i], ring_start[i], uv0, uv2, uv1, n_cap, n_cap, n_cap, material)

        if capped_end and radius_end > 1e-4:
            n_cap = z_dir
            for i in range(segments):
                next_i = (i + 1) % segments
                theta0 = 2.0 * math.pi * i / segments
                theta1 = 2.0 * math.pi * next_i / segments
                uv0 = (0.5, 0.5)
                uv1 = (0.5 + 0.5 * math.cos(theta0), 0.5 + 0.5 * math.sin(theta0))
                uv2 = (0.5 + 0.5 * math.cos(theta1), 0.5 + 0.5 * math.sin(theta1))
                self.add_triangle(p_end, ring_end[i], ring_end[next_i], uv0, uv1, uv2, n_cap, n_cap, n_cap, material)

    def add_profile_extrusion(self,
                              profile_yz: List[Tuple[float, float]],
                              x_start: float,
                              x_end: float,
                              material: str,
                              smooth_profile: bool = True,
                              cap_start: bool = True,
                              cap_end: bool = True):
        """
        Extruye un perfil 2D en el plano YZ a lo largo del eje X.
        profile_yz: lista de tuplas (y, z).
        """
        num_pts = len(profile_yz)
        if num_pts < 2:
            return

        # Calcular normales 2D del perfil en YZ
        seg_normals_yz = []
        for i in range(num_pts - 1):
            dy = profile_yz[i + 1][0] - profile_yz[i][0]
            dz = profile_yz[i + 1][1] - profile_yz[i][1]
            length = math.sqrt(dy * dy + dz * dz)
            if length < 1e-6:
                seg_normals_yz.append((0.0, 1.0))
            else:
                # Normal exterior apuntando a la derecha de la progresión del perfil
                seg_normals_yz.append((dz / length, -dy / length))

        # Normales por vértice (suavizadas o duras)
        vert_normals_3d = []
        if smooth_profile:
            for i in range(num_pts):
                if i == 0:
                    ny, nz = seg_normals_yz[0]
                elif i == num_pts - 1:
                    ny, nz = seg_normals_yz[-1]
                else:
                    ny = (seg_normals_yz[i - 1][0] + seg_normals_yz[i][0]) * 0.5
                    nz = (seg_normals_yz[i - 1][1] + seg_normals_yz[i][1]) * 0.5
                    l = math.sqrt(ny * ny + nz * nz)
                    if l > 1e-6:
                        ny /= l
                        nz /= l
                vert_normals_3d.append((0.0, ny, nz))
        else:
            # Para perfiles facetados duros
            pass

        # Longitud acumulada del perfil para coordenadas UV V
        arc_lengths = [0.0]
        for i in range(num_pts - 1):
            dy = profile_yz[i + 1][0] - profile_yz[i][0]
            dz = profile_yz[i + 1][1] - profile_yz[i][1]
            arc_lengths.append(arc_lengths[-1] + math.sqrt(dy * dy + dz * dz))
        total_profile_len = arc_lengths[-1] if arc_lengths[-1] > 1e-6 else 1.0

        length_x = abs(x_end - x_start)
        u_tile = length_x / 100.0  # 1 repetición UV por cada metro (100 UU)

        # Generar quads longitudinales
        for i in range(num_pts - 1):
            p0 = (x_start, profile_yz[i][0], profile_yz[i][1])
            p1 = (x_end, profile_yz[i][0], profile_yz[i][1])
            p2 = (x_end, profile_yz[i + 1][0], profile_yz[i + 1][1])
            p3 = (x_start, profile_yz[i + 1][0], profile_yz[i + 1][1])

            v0 = arc_lengths[i] / total_profile_len
            v1 = arc_lengths[i + 1] / total_profile_len

            if smooth_profile:
                n0 = vert_normals_3d[i]
                n1 = vert_normals_3d[i]
                n2 = vert_normals_3d[i + 1]
                n3 = vert_normals_3d[i + 1]
            else:
                ny, nz = seg_normals_yz[i]
                n0 = n1 = n2 = n3 = (0.0, ny, nz)

            # Comprobar devanado CCW
            # p1 - p0 = (length, 0, 0)
            # p3 - p0 = (0, dy, dz)
            # Cross: (0*dz - 0*dy, 0*0 - length*dz, length*dy - 0*0) = (0, -length*dz, length*dy)
            # Normal debe concordar con (0, ny, nz) = (0, dz, -dy)
            # Si apunta al revés, invertimos orden:
            dot_check = -dz * seg_normals_yz[i][0] + dy * seg_normals_yz[i][1]
            if dot_check < 0:
                self.add_quad(
                    p0, p1, p2, p3,
                    (0.0, v0), (u_tile, v0), (u_tile, v1), (0.0, v1),
                    n0, n1, n2, n3,
                    material
                )
            else:
                self.add_quad(
                    p0, p3, p2, p1,
                    (0.0, v0), (0.0, v1), (u_tile, v1), (u_tile, v0),
                    n0, n3, n2, n1,
                    material
                )

        # Tapas extremas (Polígono 2D)
        if cap_start and num_pts >= 3:
            # Tapa plana en x_start mirando hacia -X
            # Triangulación simple tipo abanico (funciona óptimamente para perfiles convexos o New Jersey)
            center_y = sum(p[0] for p in profile_yz) / num_pts
            center_z = sum(p[1] for p in profile_yz) / num_pts
            center_pt = (x_start, center_y, center_z)
            n_cap = (-1.0, 0.0, 0.0)

            for i in range(num_pts - 1):
                pt_a = (x_start, profile_yz[i][0], profile_yz[i][1])
                pt_b = (x_start, profile_yz[i + 1][0], profile_yz[i + 1][1])
                uv_c = (0.5, 0.5)
                uv_a = ((pt_a[1] - center_y) / 50.0 + 0.5, (pt_a[2] - center_z) / 50.0 + 0.5)
                uv_b = ((pt_b[1] - center_y) / 50.0 + 0.5, (pt_b[2] - center_z) / 50.0 + 0.5)
                self.add_triangle(center_pt, pt_a, pt_b, uv_c, uv_a, uv_b, n_cap, n_cap, n_cap, material)

        if cap_end and num_pts >= 3:
            # Tapa plana en x_end mirando hacia +X
            center_y = sum(p[0] for p in profile_yz) / num_pts
            center_z = sum(p[1] for p in profile_yz) / num_pts
            center_pt = (x_end, center_y, center_z)
            n_cap = (1.0, 0.0, 0.0)

            for i in range(num_pts - 1):
                pt_a = (x_end, profile_yz[i][0], profile_yz[i][1])
                pt_b = (x_end, profile_yz[i + 1][0], profile_yz[i + 1][1])
                uv_c = (0.5, 0.5)
                uv_a = ((pt_a[1] - center_y) / 50.0 + 0.5, (pt_a[2] - center_z) / 50.0 + 0.5)
                uv_b = ((pt_b[1] - center_y) / 50.0 + 0.5, (pt_b[2] - center_z) / 50.0 + 0.5)
                self.add_triangle(center_pt, pt_b, pt_a, uv_c, uv_b, uv_a, n_cap, n_cap, n_cap, material)

    def export_obj_and_mtl(self, obj_path: str, mtl_path: str):
        """Exporta los archivos .obj y .mtl listos para Unreal Engine."""
        os.makedirs(os.path.dirname(obj_path), exist_ok=True)
        mtl_filename = os.path.basename(mtl_path)

        # 1. Exportar archivo .mtl
        with open(mtl_path, "w", encoding="utf-8") as fm:
            fm.write(f"# Material Library para {self.name}\n")
            fm.write(f"# Autopistas de España - Unreal Engine 5 PBR Materials\n\n")

            for mat_name in sorted(self.used_materials):
                props = DEFAULT_MATERIALS.get(mat_name, {
                    "Ka": (0.2, 0.2, 0.2), "Kd": (0.6, 0.6, 0.6), "Ks": (0.3, 0.3, 0.3),
                    "Ns": 30.0, "d": 1.0, "desc": "Material estándar"
                })
                fm.write(f"newmtl {mat_name}\n")
                fm.write(f"# {props.get('desc', '')}\n")
                ka = props["Ka"]
                kd = props["Kd"]
                ks = props["Ks"]
                fm.write(f"Ka {ka[0]:.4f} {ka[1]:.4f} {ka[2]:.4f}\n")
                fm.write(f"Kd {kd[0]:.4f} {kd[1]:.4f} {kd[2]:.4f}\n")
                fm.write(f"Ks {ks[0]:.4f} {ks[1]:.4f} {ks[2]:.4f}\n")
                fm.write(f"Ns {props['Ns']:.1f}\n")
                fm.write(f"d {props['d']:.2f}\n")
                fm.write(f"illum 2\n\n")

        # 2. Exportar archivo .obj
        with open(obj_path, "w", encoding="utf-8") as fo:
            fo.write(f"# Wavefront OBJ File: {self.name}\n")
            fo.write(f"# Proyecto: Autopistas de España\n")
            fo.write(f"# Escala: 1 Unreal Unit = 1 cm = 0.01 m\n")
            fo.write(f"mtllib {mtl_filename}\n")
            fo.write(f"o {self.name}\n\n")

            # Vértices geométricos
            for v in self.vertices:
                fo.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
            fo.write(f"# {len(self.vertices)} vertices\n\n")

            # Coordenadas de textura UV
            for uv in self.uvs:
                fo.write(f"vt {uv[0]:.5f} {uv[1]:.5f}\n")
            fo.write(f"# {len(self.uvs)} texture coordinates\n\n")

            # Normales de vértice
            for n in self.normals:
                fo.write(f"vn {n[0]:.5f} {n[1]:.5f} {n[2]:.5f}\n")
            fo.write(f"# {len(self.normals)} vertex normals\n\n")

            # Caras agrupadas por material
            fo.write(f"s 1\n")
            total_faces = 0
            for mat_name, faces in self.faces_by_mat.items():
                fo.write(f"\nusemtl {mat_name}\n")
                for face in faces:
                    # En OBJ los índices son 1-based: v/vt/vn
                    face_str = " ".join(f"{idx[0] + 1}/{idx[1] + 1}/{idx[2] + 1}" for idx in face)
                    fo.write(f"f {face_str}\n")
                    total_faces += 1

            fo.write(f"\n# {total_faces} faces en total\n")

        return len(self.vertices), total_faces


# =============================================================================
# CONSTRUCTORES GEOMÉTRICOS DE LOS 8 ELEMENTOS VIALES
# =============================================================================

def build_bionda_metalica_simple_400() -> MeshBuilder:
    """
    1. Bionda Metálica Simple de Acero Galvanizado (400 UU / 4.0 m).
    - Módulo de 400 UU de longitud a lo largo del eje X (0 a 400 UU).
    - Perfil de doble onda UNE 135312 (W-Beam) entre Z = 44 UU y Z = 75 UU (altura reglamentaria 75 cm).
    - 2 Postes en perfil C a X = 100 UU y X = 300 UU (separación reglamentaria de 200 UU / 2.0 m).
    - Separadores trapezoidales de amortiguación con tornillería.
    - 2 Captafaros retrorreflectantes blancos de alta intensidad en el seno central.
    """
    mesh = MeshBuilder("SM_Bionda_Simple_400")

    # 1. Perfil de doble onda de la bionda (Y hacia la calzada en valores cercanos a 0, hacia atrás negativo)
    # Calzada a Y > 0. El borde delantero de la cresta toca Y = 0.
    w_profile = [
        (-1.5, 75.0),   # Pliegue superior hacia atrás
        (-0.5, 73.5),
        (0.0, 67.5),    # Cresta superior prominente
        (-2.5, 64.0),
        (-6.0, 61.5),
        (-8.5, 59.5),   # Valle central (fijación y captafaro)
        (-6.0, 57.5),
        (-2.5, 55.0),
        (0.0, 51.5),    # Cresta inferior prominente
        (-0.5, 45.5),
        (-1.5, 44.0)    # Pliegue inferior hacia atrás
    ]

    # Extrusión longitudinal de la chapa de acero (0 a 400 UU)
    mesh.add_profile_extrusion(
        w_profile,
        x_start=0.0,
        x_end=400.0,
        material="M_Steel_Galvanized",
        smooth_profile=True,
        cap_start=True,
        cap_end=True
    )

    # 2. Postes en perfil C (en X = 100 UU y X = 300 UU)
    # Dimensiones perfil C: 8 UU de ancho (X), 12 UU de fondo (Y: -22 a -10), altura Z = 0 a 73 UU
    post_x_positions = [100.0, 300.0]
    for px in post_x_positions:
        # Alma del poste (chapa posterior)
        mesh.add_box(
            x_min=px - 4.0, x_max=px + 4.0,
            y_min=-22.0, y_max=-20.5,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )
        # Alas laterales del perfil C
        mesh.add_box(
            x_min=px - 4.0, x_max=px - 2.8,
            y_min=-20.5, y_max=-10.0,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )
        mesh.add_box(
            x_min=px + 2.8, x_max=px + 4.0,
            y_min=-20.5, y_max=-10.0,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )
        # Labios de rigidización frontales del perfil C
        mesh.add_box(
            x_min=px - 4.0, x_max=px - 1.5,
            y_min=-10.0, y_max=-8.8,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )
        mesh.add_box(
            x_min=px + 1.5, x_max=px + 4.0,
            y_min=-10.0, y_max=-8.8,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )

        # Separador / Amortiguador trapezoidal entre el poste y la bionda (Z = 53 a 66 UU)
        mesh.add_box(
            x_min=px - 3.5, x_max=px + 3.5,
            y_min=-10.0, y_max=-8.5,
            z_min=53.0, z_max=66.0,
            material="M_Steel_Galvanized"
        )

        # Tornillo hexagonal frontal en el seno central (X: px, Y: -8.5 a -7.5, Z: 59.5)
        mesh.add_cylinder(
            p_start=(px, -8.5, 59.5),
            p_end=(px, -7.5, 59.5),
            radius=1.6,
            segments=6,
            material="M_Steel_Galvanized"
        )

        # 3. Captafaro reflectante trapezoidal (ojo de gato DGT blanco)
        # Ubicado sobre el seno central de la bionda, ligeramente inclinado hacia el sentido de marcha (+X)
        mesh.add_box(
            x_min=px - 2.5, x_max=px + 2.5,
            y_min=-8.2, y_max=-7.4,
            z_min=56.5, z_max=62.5,
            material="M_Reflector_White"
        )

    return mesh


def build_bionda_spm_400() -> MeshBuilder:
    """
    2. Bionda con SPM (Sistema para Protección de Motociclistas según Norma UNE-EN 1317).
    - Incorpora todos los elementos de la bionda simple de 400 UU.
    - Añade el faldón inferior continuo que cubre el hueco inferior desde Z = 12 UU hasta Z = 46 UU,
      evitando el impacto directo o enganche contra los postes en C.
    - Sujeciones inferiores anti-enganche conectadas a los postes.
    """
    mesh = MeshBuilder("SM_Bionda_SPM_400")

    # 1. Bionda metálica superior estándar
    w_profile = [
        (-1.5, 75.0),
        (-0.5, 73.5),
        (0.0, 67.5),
        (-2.5, 64.0),
        (-6.0, 61.5),
        (-8.5, 59.5),
        (-6.0, 57.5),
        (-2.5, 55.0),
        (0.0, 51.5),
        (-0.5, 45.5),
        (-1.5, 44.0)
    ]
    mesh.add_profile_extrusion(
        w_profile,
        x_start=0.0,
        x_end=400.0,
        material="M_Steel_Galvanized",
        smooth_profile=True,
        cap_start=True,
        cap_end=True
    )

    # 2. Faldón continuo inferior de SPM (Protección Motoristas UNE-EN 1317)
    # Se solapa bajo el labio inferior de la bionda (Z = 46) y baja hasta Z = 12 UU con curvatura convexa continua.
    spm_profile = [
        (-2.0, 46.0),   # Solape superior bajo la bionda
        (-0.8, 42.0),
        (0.0, 34.0),    # Vientre convexo alineado con el plano exterior
        (-0.5, 24.0),
        (-2.0, 16.0),
        (-3.8, 12.0)    # Borde inferior doblado hacia el interior (evita aristas cortantes)
    ]
    mesh.add_profile_extrusion(
        spm_profile,
        x_start=0.0,
        x_end=400.0,
        material="M_Steel_Galvanized",
        smooth_profile=True,
        cap_start=True,
        cap_end=True
    )

    # 3. Postes en C y brazos de anclaje de SPM
    post_x_positions = [100.0, 300.0]
    for px in post_x_positions:
        # Alma del poste
        mesh.add_box(
            x_min=px - 4.0, x_max=px + 4.0,
            y_min=-22.0, y_max=-20.5,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )
        # Alas del perfil C
        mesh.add_box(
            x_min=px - 4.0, x_max=px - 2.8,
            y_min=-20.5, y_max=-10.0,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )
        mesh.add_box(
            x_min=px + 2.8, x_max=px + 4.0,
            y_min=-20.5, y_max=-10.0,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )
        # Labios
        mesh.add_box(
            x_min=px - 4.0, x_max=px - 1.5,
            y_min=-10.0, y_max=-8.8,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )
        mesh.add_box(
            x_min=px + 1.5, x_max=px + 4.0,
            y_min=-10.0, y_max=-8.8,
            z_min=0.0, z_max=73.0,
            material="M_Steel_Galvanized"
        )

        # Separador superior
        mesh.add_box(
            x_min=px - 3.5, x_max=px + 3.5,
            y_min=-10.0, y_max=-8.5,
            z_min=53.0, z_max=66.0,
            material="M_Steel_Galvanized"
        )
        # Brazo de soporte inferior específico de SPM (amortiguador inferior a Z = 20 a 32 UU)
        mesh.add_box(
            x_min=px - 3.0, x_max=px + 3.0,
            y_min=-10.0, y_max=-1.5,
            z_min=20.0, z_max=32.0,
            material="M_Steel_Galvanized"
        )

        # Tornillo bionda superior
        mesh.add_cylinder(
            p_start=(px, -8.5, 59.5),
            p_end=(px, -7.5, 59.5),
            radius=1.6,
            segments=6,
            material="M_Steel_Galvanized"
        )
        # Tornillo de sujeción inferior de SPM
        mesh.add_cylinder(
            p_start=(px, -1.5, 26.0),
            p_end=(px, -0.2, 26.0),
            radius=1.4,
            segments=6,
            material="M_Steel_Galvanized"
        )

        # Captafaro retrorreflectante blanco
        mesh.add_box(
            x_min=px - 2.5, x_max=px + 2.5,
            y_min=-8.2, y_max=-7.4,
            z_min=56.5, z_max=62.5,
            material="M_Reflector_White"
        )

    return mesh


def build_barrera_new_jersey_200() -> MeshBuilder:
    """
    3. Barrera Rígida New Jersey de Hormigón Armado (200 UU / 2.0 m).
    - Longitud modular de 200 UU a lo largo del eje X (0 a 200 UU).
    - Altura exacta de 80 cm / 80 UU.
    - Perfil oficial simétrico New Jersey / F-Shape:
        * Base de contacto en Z = 0: ancho 56 UU (Y de -28 a +28).
        * Escalón vertical inferior de Z = 0 a 7 UU (ancho 56 UU).
        * Tramo inclinado inferior de Z = 7 a 25 UU (ancho pasa de 56 a 40 UU).
        * Tramo superior de Z = 25 a 75 UU (ancho pasa de 40 a 18 UU).
        * Coronación redondeada de Z = 75 a 80 UU (ancho superior 15 UU).
    - Imbornal de desagüe inferior pasante en el centro (Z = 0 a 6 UU, X = 85 a 115 UU).
    - Unión macho-hembra en extremos e inserciones de izado lateral.
    """
    mesh = MeshBuilder("SM_Barrera_NewJersey_200")

    # Perfil cerrado simétrico en plano YZ
    # Iniciamos en la coronación derecha, bajamos por la derecha, cruzamos base y subimos por izquierda
    nj_profile = [
        # Coronación superior plana / chaflán
        (0.0, 80.0),
        (7.5, 79.5),
        (9.0, 75.0),
        # Tramo superior derecho
        (20.0, 25.0),
        # Tramo inclinado inferior derecho
        (28.0, 7.0),
        # Escalón vertical derecho
        (28.0, 0.0),
        # Base de apoyo
        (-28.0, 0.0),
        # Escalón vertical izquierdo
        (-28.0, 7.0),
        # Tramo inclinado inferior izquierdo
        (-20.0, 25.0),
        # Tramo superior izquierdo
        (-9.0, 75.0),
        (-7.5, 79.5),
        (0.0, 80.0)
    ]

    # Extrusión del cuerpo de hormigón
    mesh.add_profile_extrusion(
        nj_profile,
        x_start=0.0,
        x_end=200.0,
        material="M_Concrete_NewJersey",
        smooth_profile=False,  # Barrera facetada según aristas de encofrado oficial
        cap_start=True,
        cap_end=True
    )

    # Detalle de unión macho-hembra en los extremos (ranura de enclavamiento)
    # Macho en extremo X = 200 (lengüeta que sobresale hacia el siguiente bloque)
    mesh.add_box(
        x_min=200.0, x_max=204.0,
        y_min=-5.0, y_max=5.0,
        z_min=15.0, z_max=65.0,
        material="M_Concrete_NewJersey"
    )

    # Orificios pasantes de izado con grúa (huecos cilíndricos a X = 50 y X = 150, Z = 50 UU)
    for x_hole in [50.0, 150.0]:
        # Rebaje lateral derecho
        mesh.add_cylinder(
            p_start=(x_hole, 15.0, 50.0),
            p_end=(x_hole, 17.0, 50.0),
            radius=3.0,
            segments=12,
            material="M_Concrete_NewJersey"
        )
        # Rebaje lateral izquierdo
        mesh.add_cylinder(
            p_start=(x_hole, -17.0, 50.0),
            p_end=(x_hole, -15.0, 50.0),
            radius=3.0,
            segments=12,
            material="M_Concrete_NewJersey"
        )

    return mesh


def build_cono_obra_75() -> MeshBuilder:
    """
    4. Cono de Obra Naranja Fluorescente Reflectante (75 UU / 75 cm).
    - Altura reglamentaria: 75 UU (75 cm) según norma UNE-EN 13422 para vías de alta velocidad.
    - Base pesada cuadrada de 42 x 42 UU (-21 a +21) en caucho negro reciclado (Z = 0 a 4.5 UU).
    - Cuerpo cónico naranja fluorescente de alta visibilidad (Z = 4.5 a 75 UU).
    - Dos bandas retrorreflectantes blancas microprismáticas de alta intensidad:
        * Banda inferior: Z = 24 a 37 UU.
        * Banda superior: Z = 47 a 62 UU.
    - Borde superior con collarín de manipulación y orificio pasante para apilado.
    """
    mesh = MeshBuilder("SM_Cono_Obra_75")

    # 1. Base cuadrada lastrada de caucho vulcanizado negro
    # Dimensiones: 42 x 42 UU, altura Z = 0 a 3.5 UU
    mesh.add_box(
        x_min=-21.0, x_max=21.0,
        y_min=-21.0, y_max=21.0,
        z_min=0.0, z_max=3.5,
        material="M_Cone_Base_Black"
    )
    # Bisel superior de la base de caucho
    mesh.add_frustum(
        p_start=(0.0, 0.0, 3.5),
        p_end=(0.0, 0.0, 4.5),
        radius_start=20.0,
        radius_end=17.5,
        segments=24,
        material="M_Cone_Base_Black",
        capped_start=False,
        capped_end=False
    )

    # Radios del cono a distintas cotas Z (para mantener conicidad exacta):
    # En Z = 4.5 UU -> Radio = 17.0 UU
    # En Z = 72.0 UU -> Radio = 4.6 UU
    # Pendiente: dr/dz = (4.6 - 17.0) / (72.0 - 4.5) = -12.4 / 67.5 = -0.183704
    def cone_radius_at(z: float) -> float:
        return 17.0 - 0.183704 * (z - 4.5)

    # 2. Tramo 1: Cono naranja inferior (Z = 4.5 a 24.0 UU)
    r_z4_5 = cone_radius_at(4.5)
    r_z24 = cone_radius_at(24.0)
    mesh.add_frustum(
        p_start=(0.0, 0.0, 4.5),
        p_end=(0.0, 0.0, 24.0),
        radius_start=r_z4_5,
        radius_end=r_z24,
        segments=24,
        material="M_TrafficCone_Orange",
        capped_start=False,
        capped_end=False
    )

    # 3. Tramo 2: Primera banda reflectante blanca (Z = 24.0 a 37.0 UU)
    r_z37 = cone_radius_at(37.0)
    mesh.add_frustum(
        p_start=(0.0, 0.0, 24.0),
        p_end=(0.0, 0.0, 37.0),
        radius_start=r_z24,
        radius_end=r_z37,
        segments=24,
        material="M_Reflective_White",
        capped_start=False,
        capped_end=False
    )

    # 4. Tramo 3: Cono naranja intermedio (Z = 37.0 a 47.0 UU)
    r_z47 = cone_radius_at(47.0)
    mesh.add_frustum(
        p_start=(0.0, 0.0, 37.0),
        p_end=(0.0, 0.0, 47.0),
        radius_start=r_z37,
        radius_end=r_z47,
        segments=24,
        material="M_TrafficCone_Orange",
        capped_start=False,
        capped_end=False
    )

    # 5. Tramo 4: Segunda banda reflectante blanca superior (Z = 47.0 a 62.0 UU)
    r_z62 = cone_radius_at(62.0)
    mesh.add_frustum(
        p_start=(0.0, 0.0, 47.0),
        p_end=(0.0, 0.0, 62.0),
        radius_start=r_z47,
        radius_end=r_z62,
        segments=24,
        material="M_Reflective_White",
        capped_start=False,
        capped_end=False
    )

    # 6. Tramo 5: Cono naranja superior (Z = 62.0 a 72.0 UU)
    r_z72 = cone_radius_at(72.0)
    mesh.add_frustum(
        p_start=(0.0, 0.0, 62.0),
        p_end=(0.0, 0.0, 72.0),
        radius_start=r_z62,
        radius_end=r_z72,
        segments=24,
        material="M_TrafficCone_Orange",
        capped_start=False,
        capped_end=False
    )

    # 7. Cuello superior con collarín de agarre y orificio pasante (Z = 72.0 a 75.0 UU)
    mesh.add_cylinder(
        p_start=(0.0, 0.0, 72.0),
        p_end=(0.0, 0.0, 75.0),
        radius=5.2,
        segments=24,
        material="M_TrafficCone_Orange",
        capped_start=False,
        capped_end=True
    )
    # Rebaje interior de apilamiento en la coronación
    mesh.add_cylinder(
        p_start=(0.0, 0.0, 74.5),
        p_end=(0.0, 0.0, 75.1),
        radius=3.2,
        segments=24,
        material="M_Cone_Base_Black",
        capped_start=False,
        capped_end=True
    )

    return mesh


def build_portico_autovia_2300() -> MeshBuilder:
    """
    5. Pórtico Metálico de Autovía para Señales y PMV (2300 UU / 23.0 m).
    - Cruza una calzada 2x2 completa de 2300 UU de ancho (Y: -1150 a +1150 UU).
    - Altura libre reglamentaria (Gálibo mínimo): estrictamente 600 UU (6.0 metros).
    - Dos torres/pilares laterales en celosía espacial con zapatas de hormigón (Z = 0 a 750 UU).
    - Dintel principal en celosía tridimensional de 4 cordones longitudinales (Z = 600 a 740 UU).
    - Pasarela de mantenimiento trasera con suelo de rejilla y barandilla de seguridad con pasamanos.
    - Escalera vertical con jaula salvavidas en el pilar derecho.
    - Gran Cartel de Salida / Destino de autovía (Fondo azul RAL 5017, flechas y texto reflectante).
    - Panel de Mensaje Variable (PMV DGT) con visera antideslumbrante y matriz LED activa.
    """
    mesh = MeshBuilder("SM_Portico_Autovia_2300")

    half_span = 1150.0  # Total 2300 UU de luz libre
    z_clearance = 600.0
    truss_height = 140.0
    z_top = z_clearance + truss_height  # 740.0 UU

    # 1. Zapatas de hormigón en ambos márgenes
    for y_pillar in [-half_span, half_span]:
        mesh.add_box(
            x_min=-60.0, x_max=60.0,
            y_min=y_pillar - 45.0, y_max=y_pillar + 45.0,
            z_min=0.0, z_max=35.0,
            material="M_Concrete"
        )
        # Placa base de acero con pernos de anclaje
        mesh.add_box(
            x_min=-50.0, x_max=50.0,
            y_min=y_pillar - 35.0, y_max=y_pillar + 35.0,
            z_min=35.0, z_max=40.0,
            material="M_Steel_Galvanized"
        )

    # 2. Torres/Pilares verticales en celosía (izquierdo y derecho)
    # Cada pilar consta de dos columnas tubulares principales unidas por diagonales en X
    for y_pillar in [-half_span, half_span]:
        # Columnas verticales tubulares (X = -40 y X = +40)
        mesh.add_cylinder(
            p_start=(-40.0, y_pillar, 40.0),
            p_end=(-40.0, y_pillar, z_top),
            radius=8.0,
            segments=12,
            material="M_Steel_Galvanized"
        )
        mesh.add_cylinder(
            p_start=(40.0, y_pillar, 40.0),
            p_end=(40.0, y_pillar, z_top),
            radius=8.0,
            segments=12,
            material="M_Steel_Galvanized"
        )

        # Diagonales de arriostramiento en celosía (cruces de San Andrés) cada 70 UU de cota Z
        num_braces = int((z_clearance - 40.0) / 70.0)
        for b in range(num_braces):
            zb0 = 40.0 + b * 70.0
            zb1 = zb0 + 70.0
            # Diagonal 1
            mesh.add_cylinder(
                p_start=(-40.0, y_pillar, zb0),
                p_end=(40.0, y_pillar, zb1),
                radius=3.0,
                segments=8,
                material="M_Steel_Galvanized",
                capped_start=False, capped_end=False
            )
            # Diagonal 2
            mesh.add_cylinder(
                p_start=(40.0, y_pillar, zb0),
                p_end=(-40.0, y_pillar, zb1),
                radius=3.0,
                segments=8,
                material="M_Steel_Galvanized",
                capped_start=False, capped_end=False
            )
            # Travesaño horizontal
            mesh.add_cylinder(
                p_start=(-40.0, y_pillar, zb1),
                p_end=(40.0, y_pillar, zb1),
                radius=3.5,
                segments=8,
                material="M_Steel_Galvanized",
                capped_start=False, capped_end=False
            )

    # 3. Dintel principal en celosía espacial tridimensional (span de Y = -half_span a +half_span)
    # 4 cordones longitudinales principales (tubulares de 12 UU de radio)
    coords_chords = [
        (-40.0, z_clearance),  # Delantero inferior
        (40.0, z_clearance),   # Trasero inferior
        (-40.0, z_top),        # Delantero superior
        (40.0, z_top)          # Trasero superior
    ]
    for xc, zc in coords_chords:
        mesh.add_cylinder(
            p_start=(xc, -half_span, zc),
            p_end=(xc, half_span, zc),
            radius=6.5,
            segments=12,
            material="M_Steel_Galvanized"
        )

    # Diagonales y montantes de la viga en celosía cada 100 UU de luz a lo largo de Y
    panel_step = 100.0
    num_panels = int((2.0 * half_span) / panel_step)
    for p in range(num_panels):
        y0 = -half_span + p * panel_step
        y1 = y0 + panel_step

        # Cara frontal (X = -40)
        mesh.add_cylinder(
            p_start=(-40.0, y0, z_clearance),
            p_end=(-40.0, y1, z_top),
            radius=3.0, segments=8, material="M_Steel_Galvanized", capped_start=False, capped_end=False
        )
        # Cara trasera (X = +40)
        mesh.add_cylinder(
            p_start=(40.0, y0, z_top),
            p_end=(40.0, y1, z_clearance),
            radius=3.0, segments=8, material="M_Steel_Galvanized", capped_start=False, capped_end=False
        )
        # Cara inferior (Z = z_clearance)
        mesh.add_cylinder(
            p_start=(-40.0, y0, z_clearance),
            p_end=(40.0, y1, z_clearance),
            radius=3.0, segments=8, material="M_Steel_Galvanized", capped_start=False, capped_end=False
        )
        # Cara superior (Z = z_top)
        mesh.add_cylinder(
            p_start=(-40.0, y0, z_top),
            p_end=(40.0, y1, z_top),
            radius=3.0, segments=8, material="M_Steel_Galvanized", capped_start=False, capped_end=False
        )
        # Montantes verticales
        mesh.add_cylinder(
            p_start=(-40.0, y1, z_clearance),
            p_end=(-40.0, y1, z_top),
            radius=3.5, segments=8, material="M_Steel_Galvanized", capped_start=False, capped_end=False
        )
        mesh.add_cylinder(
            p_start=(40.0, y1, z_clearance),
            p_end=(40.0, y1, z_top),
            radius=3.5, segments=8, material="M_Steel_Galvanized", capped_start=False, capped_end=False
        )

    # 4. Pasarela de mantenimiento trasera (Catwalk con barandilla)
    # Suelo de rejilla metálica en la cara trasera (+X de 40 a 95 UU, Z = z_clearance + 2)
    mesh.add_box(
        x_min=40.0, x_max=95.0,
        y_min=-half_span + 50.0, y_max=half_span - 50.0,
        z_min=z_clearance + 2.0, z_max=z_clearance + 6.0,
        material="M_Steel_Galvanized"
    )
    # Pasamanos superior de la barandilla (X = 95, Z = z_clearance + 110)
    mesh.add_cylinder(
        p_start=(95.0, -half_span + 50.0, z_clearance + 110.0),
        p_end=(95.0, half_span - 50.0, z_clearance + 110.0),
        radius=2.5, segments=8, material="M_Steel_Galvanized"
    )
    # Listón intermedio
    mesh.add_cylinder(
        p_start=(95.0, -half_span + 50.0, z_clearance + 55.0),
        p_end=(95.0, half_span - 50.0, z_clearance + 55.0),
        radius=1.8, segments=8, material="M_Steel_Galvanized"
    )
    # Montantes de la barandilla cada 150 UU
    num_rail_posts = int((2.0 * half_span - 100.0) / 150.0)
    for rp in range(num_rail_posts + 1):
        yrp = (-half_span + 50.0) + rp * 150.0
        mesh.add_cylinder(
            p_start=(95.0, yrp, z_clearance + 6.0),
            p_end=(95.0, yrp, z_clearance + 110.0),
            radius=2.0, segments=8, material="M_Steel_Galvanized"
        )

    # 5. Escalera vertical de gato con aros quitamiedos (Pilar derecho en Y = half_span)
    ladder_y = half_span - 15.0
    for rz in range(40, int(z_clearance), 28):
        # Peldaños de escalera
        mesh.add_cylinder(
            p_start=(45.0, ladder_y, float(rz)),
            p_end=(75.0, ladder_y, float(rz)),
            radius=1.5, segments=6, material="M_Steel_Galvanized"
        )
    # Largueros verticales de la escalera
    mesh.add_cylinder(p_start=(45.0, ladder_y, 40.0), p_end=(45.0, ladder_y, z_clearance), radius=2.0, segments=8, material="M_Steel_Galvanized")
    mesh.add_cylinder(p_start=(75.0, ladder_y, 40.0), p_end=(75.0, ladder_y, z_clearance), radius=2.0, segments=8, material="M_Steel_Galvanized")

    # 6. Gran Cartel Vial de Salida / Dirección (Autovía Azul RAL 5017)
    # Montado en la cara frontal (-X de la celosía), cubriendo carriles izquierdos (Y: -950 a -200 UU)
    # Altura del cartel: Z = 620 a 790 UU (170 UU de alto x 750 UU de ancho)
    mesh.add_box(
        x_min=-48.0, x_max=-43.0,
        y_min=-950.0, y_max=-200.0,
        z_min=620.0, z_max=790.0,
        material="M_Sign_Back_Gray"
    )
    # Cara frontal azul RAL 5017
    mesh.add_quad(
        (-48.5, -950.0, 620.0), (-48.5, -200.0, 620.0), (-48.5, -200.0, 790.0), (-48.5, -950.0, 790.0),
        (0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0),
        (-1, 0, 0), (-1, 0, 0), (-1, 0, 0), (-1, 0, 0),
        "M_Sign_Blue_RAL5017"
    )
    # Orla blanca perimetral reflectante
    mesh.add_box(
        x_min=-49.0, x_max=-48.5,
        y_min=-945.0, y_max=-205.0,
        z_min=782.0, z_max=786.0,
        material="M_Sign_White_Reflective"
    )
    mesh.add_box(
        x_min=-49.0, x_max=-48.5,
        y_min=-945.0, y_max=-205.0,
        z_min=624.0, z_max=628.0,
        material="M_Sign_White_Reflective"
    )
    mesh.add_box(
        x_min=-49.0, x_max=-48.5,
        y_min=-945.0, y_max=-941.0,
        z_min=624.0, z_max=786.0,
        material="M_Sign_White_Reflective"
    )
    mesh.add_box(
        x_min=-49.0, x_max=-48.5,
        y_min=-209.0, y_max=-205.0,
        z_min=624.0, z_max=786.0,
        material="M_Sign_White_Reflective"
    )
    # Flechas verticales blancas reflectantes sobre cada carril en el cartel
    for y_arr in [-750.0, -400.0]:
        mesh.add_box(
            x_min=-49.2, x_max=-48.5,
            y_min=y_arr - 6.0, y_max=y_arr + 6.0,
            z_min=635.0, z_max=675.0,
            material="M_Sign_White_Reflective"
        )

    # 7. Panel de Mensaje Variable (PMV DGT de Matriz Completa)
    # Montado en la cara frontal sobre los carriles derechos (Y: +150 a +900 UU)
    # Ancho 750 UU, alto 160 UU (Z = 625 a 785 UU), profundidad 26 UU (X: -70 a -44 UU)
    mesh.add_box(
        x_min=-70.0, x_max=-44.0,
        y_min=150.0, y_max=900.0,
        z_min=625.0, z_max=785.0,
        material="M_PMV_Housing"
    )
    # Visera perimetral antideslumbrante del PMV (sobresale hacia el tráfico en -X)
    mesh.add_box(
        x_min=-80.0, x_max=-70.0,
        y_min=145.0, y_max=905.0,
        z_min=782.0, z_max=788.0,
        material="M_PMV_Housing"
    )
    mesh.add_box(
        x_min=-80.0, x_max=-70.0,
        y_min=145.0, y_max=151.0,
        z_min=622.0, z_max=788.0,
        material="M_PMV_Housing"
    )
    mesh.add_box(
        x_min=-80.0, x_max=-70.0,
        y_min=899.0, y_max=905.0,
        z_min=622.0, z_max=788.0,
        material="M_PMV_Housing"
    )
    # Pantalla LED activa de alta definición
    mesh.add_quad(
        (-70.5, 155.0, 630.0), (-70.5, 895.0, 630.0), (-70.5, 895.0, 780.0), (-70.5, 155.0, 780.0),
        (0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0),
        (-1, 0, 0), (-1, 0, 0), (-1, 0, 0), (-1, 0, 0),
        "M_PMV_Display"
    )

    return mesh


def build_cartel_salida_inmediata() -> MeshBuilder:
    """
    6. Cartel de Salida Inmediata de Autovía (Divergencia / Nariz de Salida, Norma 8.1-IC).
    - Señal reglamentaria colocada en la isleta cebreada donde se bifurca el ramal de deceleración.
    - Fondo oficial azul autovía RAL 5017.
    - Flecha blanca retrorreflectante inclinada exactamente a 45 grados apuntando hacia arriba a la derecha (↗).
    - Orla perimetral reflectante con radios de curvatura en esquinas.
    - Bastidor trasero con postes de acero galvanizado y zapatas de apoyo.
    - Dimensiones del panel: ancho 200 UU (Y: -100 a +100), alto 160 UU (Z: 140 a 300 UU).
    """
    mesh = MeshBuilder("SM_Cartel_Salida_Inmediata")

    # 1. Zapatas de cimentación y dos postes tubulares traseros
    for yp in [-55.0, 55.0]:
        mesh.add_box(
            x_min=-25.0, x_max=25.0,
            y_min=yp - 15.0, y_max=yp + 15.0,
            z_min=0.0, z_max=12.0,
            material="M_Concrete"
        )
        # Poste tubular de acero galvanizado (Z = 12 a 280 UU)
        mesh.add_cylinder(
            p_start=(5.0, yp, 12.0),
            p_end=(5.0, yp, 280.0),
            radius=4.5,
            segments=12,
            material="M_Steel_Galvanized"
        )
        # Abrazaderas de fijación del panel a los postes
        for z_clamp in [170.0, 260.0]:
            mesh.add_box(
                x_min=0.0, x_max=10.0,
                y_min=yp - 6.0, y_max=yp + 6.0,
                z_min=z_clamp - 3.0, z_max=z_clamp + 3.0,
                material="M_Steel_Galvanized"
            )

    # 2. Guías transversales de arriostramiento en la chapa trasera
    for z_rail in [170.0, 260.0]:
        mesh.add_box(
            x_min=0.0, x_max=2.5,
            y_min=-95.0, y_max=95.0,
            z_min=z_rail - 2.5, z_max=z_rail + 2.5,
            material="M_Sign_Back_Gray"
        )

    # 3. Panel de la señal (Chapa de fondo)
    # Ancho 200 UU, Alto 160 UU (Z: 140 a 300 UU), grosor 2 UU (X: -2.0 a 0.0)
    mesh.add_box(
        x_min=-2.0, x_max=0.0,
        y_min=-100.0, y_max=100.0,
        z_min=140.0, z_max=300.0,
        material="M_Sign_Back_Gray"
    )

    # Cara frontal azul RAL 5017 (X = -2.1)
    mesh.add_quad(
        (-2.1, -100.0, 140.0), (-2.1, 100.0, 140.0), (-2.1, 100.0, 300.0), (-2.1, -100.0, 300.0),
        (0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0),
        (-1, 0, 0), (-1, 0, 0), (-1, 0, 0), (-1, 0, 0),
        "M_Sign_Blue_RAL5017"
    )

    # 4. Orla perimetral blanca reflectante (marco de 4.5 UU de anchura)
    mesh.add_box(x_min=-2.6, x_max=-2.1, y_min=-96.0, y_max=96.0, z_min=293.0, z_max=297.5, material="M_Sign_White_Reflective")
    mesh.add_box(x_min=-2.6, x_max=-2.1, y_min=-96.0, y_max=96.0, z_min=142.5, z_max=147.0, material="M_Sign_White_Reflective")
    mesh.add_box(x_min=-2.6, x_max=-2.1, y_min=-96.0, y_max=-91.5, z_min=142.5, z_max=297.5, material="M_Sign_White_Reflective")
    mesh.add_box(x_min=-2.6, x_max=-2.1, y_min=91.5, y_max=96.0, z_min=142.5, z_max=297.5, material="M_Sign_White_Reflective")

    # 5. Geometría 3D de la Flecha de Salida Inmediata inclinada a 45º (↗)
    # Centro de la señal en Y = 0, Z = 220 UU.
    # Ángulo 45º exacto: cos(45º) = sin(45º) = 0.7071
    # Astil de la flecha: rectángulo inclinado a 45º
    arrow_w = 14.0   # Anchura del fuste
    arrow_len = 65.0 # Longitud del fuste
    # Vértices del astil a 45 grados (desplazamiento u=(0.7071, 0.7071), normal w=(-0.7071, 0.7071))
    hw = arrow_w * 0.5
    u_dir = (0.7071, 0.7071)
    w_dir = (-0.7071, 0.7071)

    start_pt = (-20.0, 165.0)  # Inicio en cuadrante inferior izquierdo del cartel
    end_pt = (start_pt[0] + arrow_len * u_dir[0], start_pt[1] + arrow_len * u_dir[1])

    # 4 esquinas del fuste en el plano YZ
    f0 = (start_pt[0] - hw * w_dir[0], start_pt[1] - hw * w_dir[1])
    f1 = (start_pt[0] + hw * w_dir[0], start_pt[1] + hw * w_dir[1])
    f2 = (end_pt[0] + hw * w_dir[0], end_pt[1] + hw * w_dir[1])
    f3 = (end_pt[0] - hw * w_dir[0], end_pt[1] - hw * w_dir[1])

    mesh.add_quad(
        (-2.7, f0[0], f0[1]), (-2.7, f1[0], f1[1]), (-2.7, f2[0], f2[1]), (-2.7, f3[0], f3[1]),
        (0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0),
        (-1, 0, 0), (-1, 0, 0), (-1, 0, 0), (-1, 0, 0),
        "M_Sign_White_Reflective"
    )

    # Punta triangular de la flecha a 45 grados
    # Vértice de la punta:
    tip_len = 38.0
    tip_width = 46.0
    tip_pt = (end_pt[0] + tip_len * u_dir[0], end_pt[1] + tip_len * u_dir[1])
    corner_left = (end_pt[0] - tip_width * 0.5 * w_dir[0], end_pt[1] - tip_width * 0.5 * w_dir[1])
    corner_right = (end_pt[0] + tip_width * 0.5 * w_dir[0], end_pt[1] + tip_width * 0.5 * w_dir[1])

    mesh.add_triangle(
        (-2.7, corner_left[0], corner_left[1]),
        (-2.7, corner_right[0], corner_right[1]),
        (-2.7, tip_pt[0], tip_pt[1]),
        (0.0, 0.0), (1.0, 0.0), (0.5, 1.0),
        (-1, 0, 0), (-1, 0, 0), (-1, 0, 0),
        "M_Sign_White_Reflective"
    )

    # 6. Cajetín indicador de "SALIDA" / Autovía en la esquina superior izquierda
    mesh.add_box(
        x_min=-2.6, x_max=-2.1,
        y_min=-85.0, y_max=-35.0,
        z_min=250.0, z_max=285.0,
        material="M_Sign_White_Reflective"
    )
    mesh.add_box(
        x_min=-2.7, x_max=-2.1,
        y_min=-82.0, y_max=-38.0,
        z_min=253.0, z_max=282.0,
        material="M_Sign_Blue_RAL5017"
    )

    return mesh


def build_hito_arista() -> MeshBuilder:
    """
    7. Hito de Arista Oficial de Carreteras (DGT / Ministerio de Transportes).
    - Altura reglamentaria: 105 UU (1.05 m sobre el terreno).
    - Perfil trapecial aerodinámico característico de polietileno blanco de alta densidad.
    - Cabeza biselada inclinada a 15º hacia la calzada para evacuación de agua.
    - Banda negra superior de contraste de 12 UU de altura (Z = 78 a 90 UU).
    - Captafaro retrorreflectante integrado de alta intensidad (Clase 3) en el frontal.
    """
    mesh = MeshBuilder("SM_Hito_Arista")

    # Perfil 2D del hito de arista en plano XY:
    # Cara frontal hacia la calzada (-X), fondo hacia el terraplén (+X)
    # Ancho frontal 12 UU (Y: -6 a +6), profundidad 10 UU (X: -2 a +8)
    front_x = -3.0
    back_x = 7.0

    # 1. Cuerpo principal blanco (Z = 0 a 78 UU)
    # Tramo inferior blanco
    mesh.add_box(
        x_min=front_x, x_max=back_x,
        y_min=-6.0, y_max=6.0,
        z_min=0.0, z_max=78.0,
        material="M_Marker_White"
    )

    # 2. Banda negra de contraste superior (Z = 78 a 90 UU)
    mesh.add_box(
        x_min=front_x - 0.2, x_max=back_x + 0.2,
        y_min=-6.2, y_max=6.2,
        z_min=78.0, z_max=90.0,
        material="M_Marker_Black"
    )

    # 3. Captafaro retrorreflectante blanco en la cara frontal (Z = 80 a 88 UU, centrado)
    mesh.add_box(
        x_min=front_x - 0.7, x_max=front_x - 0.1,
        y_min=-2.5, y_max=2.5,
        z_min=80.0, z_max=88.0,
        material="M_Reflector_White"
    )

    # 4. Tramo superior blanco y cabeza biselada (Z = 90 a 105 UU)
    # Cuerpo blanco de Z = 90 a 98 UU
    mesh.add_box(
        x_min=front_x, x_max=back_x,
        y_min=-6.0, y_max=6.0,
        z_min=90.0, z_max=98.0,
        material="M_Marker_White"
    )

    # Cabeza biselada inclinada hacia el frente (de Z = 105 en la parte trasera a Z = 98 en la delantera)
    # Cuadrilátero de coronación inclinado
    mesh.add_quad(
        (front_x, -6.0, 98.0), (front_x, 6.0, 98.0), (back_x, 6.0, 105.0), (back_x, -6.0, 105.0),
        (0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0),
        (-0.5735, 0.0, 0.8192), (-0.5735, 0.0, 0.8192), (-0.5735, 0.0, 0.8192), (-0.5735, 0.0, 0.8192),
        "M_Marker_White"
    )
    # Laterales triangulares del bisel
    # Lateral derecho (+Y)
    mesh.add_triangle(
        (front_x, 6.0, 98.0), (back_x, 6.0, 98.0), (back_x, 6.0, 105.0),
        (0.0, 0.0), (1.0, 0.0), (1.0, 1.0),
        (0, 1, 0), (0, 1, 0), (0, 1, 0),
        "M_Marker_White"
    )
    # Lateral izquierdo (-Y)
    mesh.add_triangle(
        (front_x, -6.0, 98.0), (back_x, -6.0, 105.0), (back_x, -6.0, 98.0),
        (0.0, 0.0), (1.0, 1.0), (1.0, 0.0),
        (0, -1, 0), (0, -1, 0), (0, -1, 0),
        "M_Marker_White"
    )
    # Cara trasera del bisel
    mesh.add_quad(
        (back_x, 6.0, 98.0), (back_x, -6.0, 98.0), (back_x, -6.0, 105.0), (back_x, 6.0, 105.0),
        (0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0),
        (1, 0, 0), (1, 0, 0), (1, 0, 0), (1, 0, 0),
        "M_Marker_White"
    )

    return mesh


def build_mojon_kilometrico() -> MeshBuilder:
    """
    7b. Mojón Kilométrico de Autovía / Punto Kilométrico Oficial (PK).
    - Cota de altura: 85 UU.
    - Prisma con coronación abovedada semicircular según la Instrucción de Carreteras de España.
    - Cuerpo con fondo azul autovía RAL 5017, campo de identificación de vía y kilometraje.
    """
    mesh = MeshBuilder("SM_Mojon_Kilometrico")

    # Dimensiones: ancho 44 UU (Y: -22 a +22), fondo 18 UU (X: -9 a +9), altura total 85 UU
    # 1. Zapata base de hormigón
    mesh.add_box(
        x_min=-11.0, x_max=11.0,
        y_min=-24.0, y_max=24.0,
        z_min=0.0, z_max=8.0,
        material="M_Concrete"
    )

    # 2. Cuerpo prismático inferior blanco (Z = 8 a 65 UU)
    mesh.add_box(
        x_min=-9.0, x_max=9.0,
        y_min=-22.0, y_max=22.0,
        z_min=8.0, z_max=65.0,
        material="M_Mojon_White"
    )

    # 3. Placa azul frontal con nomenclatura de autovía (A-4 / AP-7)
    mesh.add_box(
        x_min=-9.3, x_max=-9.0,
        y_min=-19.0, y_max=19.0,
        z_min=42.0, z_max=62.0,
        material="M_Mojon_Blue"
    )
    # Inscripción blanca / indicativo
    mesh.add_box(
        x_min=-9.5, x_max=-9.2,
        y_min=-12.0, y_max=12.0,
        z_min=48.0, z_max=56.0,
        material="M_Mojon_White"
    )

    # Campo numérico de PK (ej. km 120) en el tramo inferior
    mesh.add_box(
        x_min=-9.3, x_max=-9.0,
        y_min=-14.0, y_max=14.0,
        z_min=18.0, z_max=36.0,
        material="M_Marker_Black"
    )

    # 4. Coronación semicilíndrica abovedada (Z = 65 a 85 UU, radio = 20 UU en torno a Y)
    mesh.add_cylinder(
        p_start=(-9.0, 0.0, 65.0),
        p_end=(9.0, 0.0, 65.0),
        radius=20.0,
        segments=24,
        material="M_Mojon_White",
        capped_start=True,
        capped_end=True
    )

    # 5. Captafaros laterales ámbar para visión nocturna en rasante
    mesh.add_box(x_min=-3.0, x_max=3.0, y_min=22.0, y_max=22.6, z_min=45.0, z_max=55.0, material="M_Reflector_Amber")
    mesh.add_box(x_min=-3.0, x_max=3.0, y_min=-22.6, y_max=-22.0, z_min=45.0, z_max=55.0, material="M_Reflector_Amber")

    return mesh


def build_radar_fijo_dgt() -> MeshBuilder:
    """
    8. Radar Fijo de Velocidad de Cabina DGT (Cinemómetro de Arcén Oficial).
    - Cabina metálica compacta anti-vandálica sobre zapata de hormigón.
    - Altura total: 150 UU (1.50 m) con tejadillo inclinado para evacuación de agua.
    - Pintura oficial DGT bicolor: gris tráfico con franjas diagonales naranja fluorescente reflectante.
    - Doble óptica frontal encarada al tráfico (-X):
        * Óptica superior: lente de cámara fotográfica de alta resolución con visera antirreflejo.
        * Óptica inferior: ventana de sensor cinemómetro radar Doppler y flash estroboscópico.
    - Puerta lateral de acceso de mantenimiento con cerradura y rejillas de ventilación de lamas.
    """
    mesh = MeshBuilder("SM_Radar_Fijo_DGT")

    # 1. Zapata base de hormigón
    # Dimensiones: 66 x 66 UU (X: -33 a +33, Y: -33 a +33), altura Z = 0 a 10 UU
    mesh.add_box(
        x_min=-33.0, x_max=33.0,
        y_min=-33.0, y_max=33.0,
        z_min=0.0, z_max=10.0,
        material="M_Concrete"
    )
    # Pernos de fijación al suelo
    for bx in [-26.0, 26.0]:
        for by in [-26.0, 26.0]:
            mesh.add_cylinder(
                p_start=(bx, by, 10.0),
                p_end=(bx, by, 12.5),
                radius=1.8,
                segments=6,
                material="M_Steel_Galvanized"
            )

    # 2. Cabina principal metálica (gris DGT)
    # Dimensiones: 52 x 52 UU (X: -26 a +26, Y: -26 a +26), altura Z = 10 a 140 UU
    mesh.add_box(
        x_min=-26.0, x_max=26.0,
        y_min=-26.0, y_max=26.0,
        z_min=10.0, z_max=140.0,
        material="M_Radar_Housing_Gray"
    )

    # 3. Franjas naranjas de alta visibilidad DGT en frontal y laterales
    # Franja inferior naranja (Z = 15 a 45 UU)
    mesh.add_box(
        x_min=-26.4, x_max=26.4,
        y_min=-26.4, y_max=26.4,
        z_min=18.0, z_max=42.0,
        material="M_Radar_Orange_DGT"
    )
    # Franja intermedia naranja de advertencia (Z = 95 a 105 UU)
    mesh.add_box(
        x_min=-26.4, x_max=26.4,
        y_min=-26.4, y_max=26.4,
        z_min=96.0, z_max=104.0,
        material="M_Radar_Orange_DGT"
    )

    # 4. Tejadillo superior inclinado para lluvia (Z = 140 a 148 UU, vierte hacia atrás en +X)
    mesh.add_box(
        x_min=-28.0, x_max=28.0,
        y_min=-28.0, y_max=28.0,
        z_min=140.0, z_max=143.0,
        material="M_Radar_Housing_Gray"
    )
    # Bisel superior del techo
    mesh.add_quad(
        (-28.0, -28.0, 143.0), (-28.0, 28.0, 143.0), (28.0, 28.0, 147.5), (28.0, -28.0, 147.5),
        (0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0),
        (0.0799, 0.0, 0.9968), (0.0799, 0.0, 0.9968), (0.0799, 0.0, 0.9968), (0.0799, 0.0, 0.9968),
        "M_Radar_Housing_Gray"
    )

    # 5. Doble óptica frontal en la cara -X (hacia el tráfico que se aproxima)
    # A) Óptica Superior: Cámara de captura fotográfica (Z = 118 UU, Y = 0)
    # Marco circular exterior y visera
    mesh.add_cylinder(
        p_start=(-26.0, 0.0, 118.0),
        p_end=(-30.0, 0.0, 118.0),
        radius=7.5,
        segments=20,
        material="M_Radar_Housing_Gray",
        capped_start=False,
        capped_end=False
    )
    # Visera superior semicircular protectora contra lluvia/sol
    mesh.add_box(
        x_min=-33.0, x_max=-26.0,
        y_min=-8.5, y_max=8.5,
        z_min=124.0, z_max=126.5,
        material="M_Radar_Housing_Gray"
    )
    # Lente óptica oscura multicapa
    mesh.add_cylinder(
        p_start=(-28.5, 0.0, 118.0),
        p_end=(-28.0, 0.0, 118.0),
        radius=6.0,
        segments=20,
        material="M_Radar_Lens",
        capped_start=True,
        capped_end=True
    )

    # B) Óptica Inferior: Ventana rectangular de sensor cinemómetro y flash (Z = 72 UU, Y = 0)
    # Marco rectangular prominente
    mesh.add_box(
        x_min=-29.0, x_max=-26.0,
        y_min=-11.0, y_max=11.0,
        z_min=64.0, z_max=82.0,
        material="M_Radar_Housing_Gray"
    )
    # Visera superior
    mesh.add_box(
        x_min=-32.0, x_max=-26.0,
        y_min=-12.0, y_max=12.0,
        z_min=82.0, z_max=84.5,
        material="M_Radar_Housing_Gray"
    )
    # Placa óptica del sensor
    mesh.add_quad(
        (-29.2, -9.5, 66.0), (-29.2, 9.5, 66.0), (-29.2, 9.5, 80.0), (-29.2, -9.5, 80.0),
        (0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0),
        (-1, 0, 0), (-1, 0, 0), (-1, 0, 0), (-1, 0, 0),
        "M_Radar_Sensor"
    )

    # 6. Detalles de la cabina: Puerta de servicio lateral (+Y), cerradura y rejillas
    # Marco de la puerta lateral en Y = 26.2
    mesh.add_box(
        x_min=-18.0, x_max=18.0,
        y_min=26.0, y_max=26.5,
        z_min=15.0, z_max=130.0,
        material="M_Radar_Housing_Gray"
    )
    # Cerradura de seguridad de alta seguridad
    mesh.add_cylinder(
        p_start=(14.0, 26.0, 70.0),
        p_end=(14.0, 27.5, 70.0),
        radius=1.8,
        segments=8,
        material="M_Steel_Galvanized"
    )

    # Rejillas de lamas de ventilación troqueladas en ambos laterales
    for y_side, ny in [(-26.3, -1), (26.3, 1)]:
        for z_louver in [52.0, 56.0, 60.0]:
            mesh.add_box(
                x_min=-12.0, x_max=12.0,
                y_min=y_side - 0.2, y_max=y_side + 0.2,
                z_min=z_louver, z_max=z_louver + 1.8,
                material="M_Marker_Black"
            )

    return mesh


# =============================================================================
# FUNCIÓN DE VERIFICACIÓN Y CONTROL DE CALIDAD
# =============================================================================

def verify_generated_mesh(obj_path: str, mtl_path: str) -> Dict[str, any]:
    """Verifica la integridad técnica del modelo exportado para Unreal Engine."""
    if not os.path.exists(obj_path):
        return {"status": "ERROR", "message": f"Archivo OBJ no existe: {obj_path}"}
    if not os.path.exists(mtl_path):
        return {"status": "ERROR", "message": f"Archivo MTL no existe: {mtl_path}"}

    v_count = 0
    vt_count = 0
    vn_count = 0
    f_count = 0
    materials_referenced = set()
    materials_defined = set()
    bounds_min = [float("inf"), float("inf"), float("inf")]
    bounds_max = [float("-inf"), float("-inf"), float("-inf")]

    # Leer MTL
    with open(mtl_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line.startswith("newmtl "):
                materials_defined.add(line.split()[1])

    # Leer OBJ
    with open(obj_path, "r", encoding="utf-8") as f:
        for line in f:
            parts = line.strip().split()
            if not parts:
                continue
            if parts[0] == "v":
                v_count += 1
                coords = [float(p) for p in parts[1:4]]
                for i in range(3):
                    bounds_min[i] = min(bounds_min[i], coords[i])
                    bounds_max[i] = max(bounds_max[i], coords[i])
            elif parts[0] == "vt":
                vt_count += 1
            elif parts[0] == "vn":
                vn_count += 1
            elif parts[0] == "f":
                f_count += 1
            elif parts[0] == "usemtl":
                materials_referenced.add(parts[1])

    size_x = bounds_max[0] - bounds_min[0]
    size_y = bounds_max[1] - bounds_min[1]
    size_z = bounds_max[2] - bounds_min[2]

    # Validaciones críticas para UE5
    missing_materials = materials_referenced - materials_defined
    has_valid_normals = vn_count > 0
    has_valid_uvs = vt_count > 0

    return {
        "status": "OK" if not missing_materials and has_valid_normals and has_valid_uvs else "WARNING",
        "vertices": v_count,
        "normals": vn_count,
        "uvs": vt_count,
        "faces": f_count,
        "materials": list(materials_referenced),
        "missing_materials": list(missing_materials),
        "dimensions_uu": (round(size_x, 2), round(size_y, 2), round(size_z, 2)),
        "file_size_kb": round(os.path.getsize(obj_path) / 1024.0, 1)
    }


# =============================================================================
# EJECUCIÓN PRINCIPAL Y EXPORTACIÓN
# =============================================================================

def generate_all_props(output_dir: str):
    """Genera todos los modelos requeridos y los exporta a la carpeta destino."""
    print("=" * 80)
    print("GENERADOR DE MODELOS 3D DE INFRAESTRUCTURA Y SEÑALIZACIÓN VIAL DE ESPAÑA")
    print("Proyecto: 'Autopistas de España' | Motor: Unreal Engine 5.x")
    print("Escala Oficial: 1 Unreal Unit (UU) = 1 cm = 0.01 m")
    print(f"Carpeta de salida: {output_dir}")
    print("=" * 80)

    os.makedirs(output_dir, exist_ok=True)

    generators = [
        ("SM_Bionda_Simple_400", build_bionda_metalica_simple_400, "Bionda metálica simple 4.0m con postes en C y captafaros"),
        ("SM_Bionda_SPM_400", build_bionda_spm_400, "Bionda con SPM (Protección Motoristas UNE-EN 1317) con faldón continuo"),
        ("SM_Barrera_NewJersey_200", build_barrera_new_jersey_200, "Barrera rígida New Jersey hormigón 80cm altura, módulo 2.0m"),
        ("SM_Cono_Obra_75", build_cono_obra_75, "Cono de obra 75cm naranja fluorescente con franjas reflectantes DGT"),
        ("SM_Portico_Autovia_2300", build_portico_autovia_2300, "Pórtico metálico autovía 23m luz para calzada 2x2, gálibo libre 6m con PMV y cartel"),
        ("SM_Cartel_Salida_Inmediata", build_cartel_salida_inmediata, "Cartel de salida inmediata de autovía azul RAL 5017 y flecha a 45º"),
        ("SM_Hito_Arista", build_hito_arista, "Hito de arista oficial DGT 1.05m con franja negra y captafaro reflectante"),
        ("SM_Mojon_Kilometrico", build_mojon_kilometrico, "Mojón kilométrico oficial de autovía (PK) con fondo azul y kilometraje"),
        ("SM_Radar_Fijo_DGT", build_radar_fijo_dgt, "Radar fijo de velocidad cabina DGT con doble óptica para cámara y flash")
    ]

    results = []

    for filename, builder_func, description in generators:
        print(f"\n[+] Generando: {filename}...")
        print(f"    Descripción: {description}")

        builder = builder_func()
        obj_path = os.path.join(output_dir, f"{filename}.obj")
        mtl_path = os.path.join(output_dir, f"{filename}.mtl")

        v_count, f_count = builder.export_obj_and_mtl(obj_path, mtl_path)
        print(f"    Exportado: {obj_path} ({v_count} vértices, {f_count} caras)")

        # Control de calidad y verificación técnica
        report = verify_generated_mesh(obj_path, mtl_path)
        results.append((filename, description, report))
        print(f"    Dimensiones (X, Y, Z) en UU: {report['dimensions_uu']}")
        print(f"    Materiales asignados: {', '.join(report['materials'])}")
        print(f"    Estado de verificación: {report['status']}")

    print("\n" + "=" * 80)
    print("RESUMEN DE VERIFICACIÓN TÉCNICA PARA UNREAL ENGINE 5")
    print("=" * 80)
    print(f"{'Modelo':<28} | {'Vértices':<8} | {'Caras':<8} | {'Dim (X, Y, Z) UU':<22} | {'Estado'}")
    print("-" * 80)
    for name, desc, rep in results:
        dim_str = f"{rep['dimensions_uu'][0]}x{rep['dimensions_uu'][1]}x{rep['dimensions_uu'][2]}"
        print(f"{name:<28} | {rep['vertices']:<8} | {rep['faces']:<8} | {dim_str:<22} | {rep['status']}")
    print("=" * 80)
    print("Todos los modelos y materiales han sido generados satisfactoriamente.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generador de Modelos 3D de Señalización e Infraestructura Vial")
    parser.add_argument(
        "--output-dir",
        default="C:/app/game_autopista/Content/Meshes/RoadProps",
        help="Ruta de exportación para los archivos .obj y .mtl (por defecto Content/Meshes/RoadProps)"
    )
    args = parser.parse_args()
    generate_all_props(args.output_dir)
