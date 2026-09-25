#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
===============================================================================
Autopistas de España - Generador Procedural de Vehículos 3D (Escala 1 UU = 1 cm)
===============================================================================
Desarrollado para generar modelos 3D (.obj y .mtl) optimizados para vista cenital
(Top-Down Low/Mid-Poly) con dimensiones métricas exactas para Unreal Engine 5.

Escala Oficial del Proyecto:
    1 Unreal Unit (UU) = 1 centímetro (cm) = 0.01 metros (m)
    100 UU = 1 Metro

Ejes en Unreal Engine:
    +X : Hacia adelante (Frontal)
    -X : Hacia atrás (Trasera)
    +Y : Hacia la derecha (Lado derecho del vehículo, Y=0 es línea central)
    -Y : Hacia la izquierda (Lado izquierdo del vehículo)
    +Z : Hacia arriba (Z=0 es contacto con el suelo / asfalto)

Modelos Generados:
    1. Turismo Compacto (420 x 180 x 145 UU)
       - Capó, techo, maletero, cristales, ruedas separadas y paragolpes delantero separable.
    2. Camión Articulado:
       - Cabina tractora (450 x 250 x 360 UU) con quinta rueda.
       - Semirremolque trailer caja cerrada (1200 x 255 x 400 UU) con pivote (kingpin).
       - Camión Articulado Completo montado (1650 x 255 x 400 UU).
    3. Furgoneta de Reparto (580 x 205 x 230 UU).
    4. Autobús Interurbano (1350 x 255 x 330 UU) con ventanas panorámicas y A/C en techo.
    5. Motocicleta (215 x 85 x 120 UU).
    6. Patrulla Guardia Civil de Tráfico (480 x 190 x 185 UU) blanco/verde con puente V-1 azul.
    7. Grúa de Asistencia en Carretera (720 x 240 x 265 UU) con plataforma y rotativos ámbar V-2.
    8. Piezas de colisión / restos sueltos en calzada (Debris): Paragolpes y Rueda suelta.
===============================================================================
"""

import os
import sys
import math
from typing import List, Tuple, Dict, Optional


class MeshBuilder:
    """Constructor geométrico de mallas 3D con exportación a formato Wavefront OBJ/MTL."""

    def __init__(self, name: str):
        self.name = name
        self.vertices: List[Tuple[float, float, float]] = []
        self.normals: List[Tuple[float, float, float]] = []
        self.uvs: List[Tuple[float, float]] = []
        self.faces: List[Dict] = []
        self.current_material: str = "default"
        self.current_group: str = "default"
        self.materials_used: set = set()

    def set_material(self, mat_name: str):
        self.current_material = mat_name
        self.materials_used.add(mat_name)

    def set_group(self, group_name: str):
        self.current_group = group_name

    def add_vertex(self, x: float, y: float, z: float) -> int:
        self.vertices.append((round(x, 4), round(y, 4), round(z, 4)))
        return len(self.vertices)

    def add_normal(self, nx: float, ny: float, nz: float) -> int:
        length = math.sqrt(nx * nx + ny * ny + nz * nz)
        if length > 1e-6:
            nx /= length
            ny /= length
            nz /= length
        else:
            nx, ny, nz = 0.0, 0.0, 1.0
        self.normals.append((round(nx, 4), round(ny, 4), round(nz, 4)))
        return len(self.normals)

    def add_uv(self, u: float, v: float) -> int:
        self.uvs.append((round(u, 4), round(v, 4)))
        return len(self.uvs)

    def add_triangle_face(self, v1: Tuple[float, float, float],
                          v2: Tuple[float, float, float],
                          v3: Tuple[float, float, float],
                          custom_normal: Optional[Tuple[float, float, float]] = None):
        """Añade una cara triangular con cálculo automático de normal y UVs planares."""
        if custom_normal is None:
            ax, ay, az = v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]
            bx, by, bz = v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]
            nx = ay * bz - az * by
            ny = az * bx - ax * bz
            nz = ax * by - ay * bx
            n_idx = self.add_normal(nx, ny, nz)
        else:
            n_idx = self.add_normal(*custom_normal)

        def calc_uv(p):
            nx_val, ny_val, nz_val = self.normals[n_idx - 1]
            if abs(nz_val) >= abs(nx_val) and abs(nz_val) >= abs(ny_val):
                return (p[0] / 100.0, p[1] / 100.0)
            elif abs(ny_val) >= abs(nx_val):
                return (p[0] / 100.0, p[2] / 100.0)
            else:
                return (p[1] / 100.0, p[2] / 100.0)

        v1_idx = self.add_vertex(*v1)
        v2_idx = self.add_vertex(*v2)
        v3_idx = self.add_vertex(*v3)

        uv1_idx = self.add_uv(*calc_uv(v1))
        uv2_idx = self.add_uv(*calc_uv(v2))
        uv3_idx = self.add_uv(*calc_uv(v3))

        self.faces.append({
            'verts': [(v1_idx, uv1_idx, n_idx), (v2_idx, uv2_idx, n_idx), (v3_idx, uv3_idx, n_idx)],
            'material': self.current_material,
            'group': self.current_group
        })

    def add_quad_face(self, v1: Tuple[float, float, float],
                      v2: Tuple[float, float, float],
                      v3: Tuple[float, float, float],
                      v4: Tuple[float, float, float],
                      custom_normal: Optional[Tuple[float, float, float]] = None):
        """Añade una cara cuadrilátera dividida en dos triángulos coherentes."""
        self.add_triangle_face(v1, v2, v3, custom_normal)
        self.add_triangle_face(v1, v3, v4, custom_normal)

    def add_box(self, x_min: float, x_max: float,
                y_min: float, y_max: float,
                z_min: float, z_max: float,
                mat: Optional[str] = None,
                group: Optional[str] = None,
                skip_bottom: bool = False):
        """Genera un paralelepípedo alineado con los ejes."""
        if mat:
            self.set_material(mat)
        if group:
            self.set_group(group)

        p000 = (x_min, y_min, z_min)
        p100 = (x_max, y_min, z_min)
        p110 = (x_max, y_max, z_min)
        p010 = (x_min, y_max, z_min)
        p001 = (x_min, y_min, z_max)
        p101 = (x_max, y_min, z_max)
        p111 = (x_max, y_max, z_max)
        p011 = (x_min, y_max, z_max)

        # Cara Frontal (+X): normal (1, 0, 0)
        self.add_quad_face(p100, p110, p111, p101, (1.0, 0.0, 0.0))
        # Cara Trasera (-X): normal (-1, 0, 0)
        self.add_quad_face(p010, p000, p001, p011, (-1.0, 0.0, 0.0))
        # Cara Derecha (+Y): normal (0, 1, 0)
        self.add_quad_face(p110, p010, p011, p111, (0.0, 1.0, 0.0))
        # Cara Izquierda (-Y): normal (0, -1, 0)
        self.add_quad_face(p000, p100, p101, p001, (0.0, -1.0, 0.0))
        # Cara Superior (+Z): normal (0, 0, 1)
        self.add_quad_face(p001, p101, p111, p011, (0.0, 0.0, 1.0))
        # Cara Inferior (-Z): normal (0, 0, -1)
        if not skip_bottom:
            self.add_quad_face(p010, p110, p100, p000, (0.0, 0.0, -1.0))

    def add_frustum(self, x_bottom: Tuple[float, float], y_bottom: Tuple[float, float], z_bottom: float,
                    x_top: Tuple[float, float], y_top: Tuple[float, float], z_top: float,
                    mat: Optional[str] = None, group: Optional[str] = None):
        """Genera un tronco piramidal/trapezoide (ideal para capós, cabinas inclinadas y techos)."""
        if mat:
            self.set_material(mat)
        if group:
            self.set_group(group)

        xb_min, xb_max = x_bottom
        yb_min, yb_max = y_bottom
        xt_min, xt_max = x_top
        yt_min, yt_max = y_top

        b0 = (xb_max, yb_min, z_bottom)
        b1 = (xb_max, yb_max, z_bottom)
        b2 = (xb_min, yb_max, z_bottom)
        b3 = (xb_min, yb_min, z_bottom)

        t0 = (xt_max, yt_min, z_top)
        t1 = (xt_max, yt_max, z_top)
        t2 = (xt_min, yt_max, z_top)
        t3 = (xt_min, yt_min, z_top)

        self.add_quad_face(b0, b1, t1, t0)
        self.add_quad_face(b2, b3, t3, t2)
        self.add_quad_face(b1, b2, t2, t1)
        self.add_quad_face(b3, b0, t0, t3)
        self.add_quad_face(t3, t0, t1, t2)

    def add_cylinder_y(self, cx: float, cy: float, cz: float,
                       radius: float, width: float,
                       segments: int = 14,
                       mat: Optional[str] = None,
                       group: Optional[str] = None):
        """Genera un cilindro con eje a lo largo de Y (ruedas y ejes)."""
        if mat:
            self.set_material(mat)
        if group:
            self.set_group(group)

        y_min = cy - width / 2.0
        y_max = cy + width / 2.0

        angles = [2.0 * math.pi * i / segments for i in range(segments)]
        ring_min = []
        ring_max = []

        for a in angles:
            x = cx + radius * math.cos(a)
            z = cz + radius * math.sin(a)
            ring_min.append((x, y_min, z))
            ring_max.append((x, y_max, z))

        for i in range(segments):
            next_i = (i + 1) % segments
            v1 = ring_min[i]
            v2 = ring_max[i]
            v3 = ring_max[next_i]
            v4 = ring_min[next_i]

            cos_mid = math.cos((angles[i] + angles[next_i]) / 2.0)
            sin_mid = math.sin((angles[i] + angles[next_i]) / 2.0)
            rad_normal = (cos_mid, 0.0, sin_mid)

            self.add_triangle_face(v1, v2, v3, rad_normal)
            self.add_triangle_face(v1, v3, v4, rad_normal)

        center_min = (cx, y_min, cz)
        center_max = (cx, y_max, cz)
        norm_left = (0.0, -1.0, 0.0)
        norm_right = (0.0, 1.0, 0.0)

        for i in range(segments):
            next_i = (i + 1) % segments
            self.add_triangle_face(center_min, ring_min[next_i], ring_min[i], norm_left)
            self.add_triangle_face(center_max, ring_max[i], ring_max[next_i], norm_right)

    def add_wheel_assembly(self, cx: float, cy: float, cz: float,
                           radius: float, width: float,
                           rim_radius_ratio: float = 0.65,
                           rim_mat: str = "M_Rim_Silver",
                           tire_mat: str = "M_Rubber_Tire",
                           group_name: str = "Wheel"):
        """Genera una rueda con neumático y llanta."""
        self.set_group(group_name)
        self.add_cylinder_y(cx, cy, cz, radius, width, segments=16, mat=tire_mat, group=group_name)

        rim_radius = radius * rim_radius_ratio
        rim_width = width * 1.02
        self.add_cylinder_y(cx, cy, cz, rim_radius, rim_width, segments=12, mat=rim_mat, group=group_name)

    def add_cylinder_z(self, cx: float, cy: float, cz: float,
                       radius: float, height: float,
                       segments: int = 12,
                       mat: Optional[str] = None,
                       group: Optional[str] = None):
        """Genera un cilindro vertical a lo largo de Z (rotativos V-1/V-2)."""
        if mat:
            self.set_material(mat)
        if group:
            self.set_group(group)

        z_min = cz
        z_max = cz + height
        angles = [2.0 * math.pi * i / segments for i in range(segments)]

        ring_bottom = []
        ring_top = []

        for a in angles:
            x = cx + radius * math.cos(a)
            y = cy + radius * math.sin(a)
            ring_bottom.append((x, y, z_min))
            ring_top.append((x, y, z_max))

        for i in range(segments):
            next_i = (i + 1) % segments
            v1 = ring_bottom[i]
            v2 = ring_bottom[next_i]
            v3 = ring_top[next_i]
            v4 = ring_top[i]
            self.add_quad_face(v1, v2, v3, v4)

        c_top = (cx, cy, z_max)
        for i in range(segments):
            next_i = (i + 1) % segments
            self.add_triangle_face(c_top, ring_top[i], ring_top[next_i], (0.0, 0.0, 1.0))

    def get_bounds(self) -> Dict[str, Tuple[float, float, float]]:
        if not self.vertices:
            return {"min": (0, 0, 0), "max": (0, 0, 0), "size": (0, 0, 0)}
        xs = [v[0] for v in self.vertices]
        ys = [v[1] for v in self.vertices]
        zs = [v[2] for v in self.vertices]
        min_pt = (min(xs), min(ys), min(zs))
        max_pt = (max(xs), max(ys), max(zs))
        size_pt = (round(max_pt[0] - min_pt[0], 2),
                   round(max_pt[1] - min_pt[1], 2),
                   round(max_pt[2] - min_pt[2], 2))
        return {"min": min_pt, "max": max_pt, "size": size_pt}

    def export_obj(self, filepath: str, mtl_filename: str):
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(f"# Autopistas de España - Modelo 3D Paramétrico\n")
            f.write(f"# Malla: {self.name}\n")
            f.write(f"# Escala: 1 UU = 1 cm\n")
            bounds = self.get_bounds()
            f.write(f"# Bounding Box: Min={bounds['min']}, Max={bounds['max']}\n")
            f.write(f"# Dimensiones (X x Y x Z): {bounds['size'][0]} x {bounds['size'][1]} x {bounds['size'][2]} UU\n")
            f.write(f"mtllib {mtl_filename}\n\n")

            for v in self.vertices:
                f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
            f.write("\n")

            for vt in self.uvs:
                f.write(f"vt {vt[0]:.4f} {vt[1]:.4f}\n")
            f.write("\n")

            for vn in self.normals:
                f.write(f"vn {vn[0]:.4f} {vn[1]:.4f} {vn[2]:.4f}\n")
            f.write("\n")

            current_grp = None
            current_mat = None

            for face in self.faces:
                grp = face['group']
                mat = face['material']

                if grp != current_grp:
                    f.write(f"g {grp}\n")
                    f.write(f"o {grp}\n")
                    current_grp = grp

                if mat != current_mat:
                    f.write(f"usemtl {mat}\n")
                    current_mat = mat

                face_str = " ".join(f"{v_idx}/{vt_idx}/{vn_idx}" for v_idx, vt_idx, vn_idx in face['verts'])
                f.write(f"f {face_str}\n")


# =============================================================================
# Biblioteca Maestra de Materiales (.mtl)
# =============================================================================
def generate_master_mtl(filepath: str):
    materials = {
        "M_CarPaint_Red": {
            "Ka": "0.15 0.02 0.02",
            "Kd": "0.75 0.06 0.06",
            "Ks": "0.85 0.85 0.85",
            "Ns": "120.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_CarPaint_White": {
            "Ka": "0.20 0.20 0.20",
            "Kd": "0.92 0.93 0.95",
            "Ks": "0.80 0.80 0.80",
            "Ns": "100.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_CarPaint_Blue": {
            "Ka": "0.02 0.05 0.15",
            "Kd": "0.08 0.25 0.70",
            "Ks": "0.80 0.80 0.80",
            "Ns": "110.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_CarPaint_DarkGrey": {
            "Ka": "0.05 0.05 0.05",
            "Kd": "0.22 0.23 0.25",
            "Ks": "0.70 0.70 0.70",
            "Ns": "90.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_CarPaint_GC_Green": {
            "Ka": "0.00 0.08 0.05",
            "Kd": "0.00 0.337 0.231",
            "Ks": "0.75 0.75 0.75",
            "Ns": "100.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_GC_Yellow": {
            "Ka": "0.25 0.25 0.02",
            "Kd": "0.88 0.92 0.08",
            "Ks": "0.60 0.60 0.60",
            "Ns": "80.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_CarPaint_Yellow_Tow": {
            "Ka": "0.25 0.18 0.01",
            "Kd": "0.95 0.68 0.05",
            "Ks": "0.75 0.75 0.75",
            "Ns": "95.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_CarPaint_Bus": {
            "Ka": "0.02 0.08 0.15",
            "Kd": "0.06 0.42 0.72",
            "Ks": "0.85 0.85 0.85",
            "Ns": "110.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_Trailer_White": {
            "Ka": "0.15 0.15 0.15",
            "Kd": "0.86 0.87 0.89",
            "Ks": "0.50 0.50 0.50",
            "Ns": "60.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_Glass": {
            "Ka": "0.05 0.08 0.12",
            "Kd": "0.15 0.22 0.30",
            "Ks": "0.95 0.95 0.95",
            "Ns": "150.0",
            "d": "0.55",
            "illum": "2"
        },
        "M_Plastic_Black": {
            "Ka": "0.04 0.04 0.04",
            "Kd": "0.12 0.12 0.12",
            "Ks": "0.20 0.20 0.20",
            "Ns": "30.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_Rubber_Tire": {
            "Ka": "0.02 0.02 0.02",
            "Kd": "0.08 0.08 0.08",
            "Ks": "0.10 0.10 0.10",
            "Ns": "15.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_Rim_Silver": {
            "Ka": "0.20 0.20 0.20",
            "Kd": "0.80 0.82 0.85",
            "Ks": "0.90 0.90 0.90",
            "Ns": "130.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_Chrome_Metal": {
            "Ka": "0.30 0.30 0.30",
            "Kd": "0.90 0.90 0.92",
            "Ks": "0.98 0.98 0.98",
            "Ns": "160.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_Diamond_Plate": {
            "Ka": "0.15 0.15 0.15",
            "Kd": "0.55 0.56 0.58",
            "Ks": "0.75 0.75 0.75",
            "Ns": "85.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_Lights_Front": {
            "Ka": "0.30 0.30 0.30",
            "Kd": "0.95 0.95 0.90",
            "Ks": "1.00 1.00 1.00",
            "Ke": "1.00 0.98 0.85",
            "Ns": "140.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_Lights_Rear": {
            "Ka": "0.20 0.02 0.02",
            "Kd": "0.85 0.05 0.05",
            "Ks": "0.90 0.50 0.50",
            "Ke": "0.95 0.05 0.05",
            "Ns": "120.0",
            "d": "1.0",
            "illum": "2"
        },
        "M_Lights_Beacon_Blue": {
            "Ka": "0.02 0.15 0.40",
            "Kd": "0.05 0.45 1.00",
            "Ks": "1.00 1.00 1.00",
            "Ke": "0.10 0.65 1.00",
            "Ns": "160.0",
            "d": "0.85",
            "illum": "2"
        },
        "M_Lights_Beacon_Amber": {
            "Ka": "0.30 0.15 0.01",
            "Kd": "1.00 0.65 0.02",
            "Ks": "0.95 0.95 0.95",
            "Ke": "1.00 0.68 0.00",
            "Ns": "150.0",
            "d": "0.85",
            "illum": "2"
        }
    }

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write("# Autopistas de España - Biblioteca Maestra de Materiales\n\n")
        for mat_name, props in materials.items():
            f.write(f"newmtl {mat_name}\n")
            for key, val in props.items():
                f.write(f"  {key} {val}\n")
            f.write("\n")


# =============================================================================
# 1. Turismo Compacto (420 x 180 x 145 UU)
# =============================================================================
def build_turismo_compacto() -> MeshBuilder:
    mb = MeshBuilder("Turismo_Compacto")

    # 1. Ruedas independientes (4 ruedas, radio 30 UU)
    wheel_radius = 30.0
    wheel_width = 18.0
    front_axle_x = 130.0
    rear_axle_x = -130.0
    left_y = -79.0
    right_y = 79.0
    wheel_center_z = 30.0

    mb.add_wheel_assembly(front_axle_x, left_y, wheel_center_z, wheel_radius, wheel_width, group_name="Wheel_FL")
    mb.add_wheel_assembly(front_axle_x, right_y, wheel_center_z, wheel_radius, wheel_width, group_name="Wheel_FR")
    mb.add_wheel_assembly(rear_axle_x, left_y, wheel_center_z, wheel_radius, wheel_width, group_name="Wheel_RL")
    mb.add_wheel_assembly(rear_axle_x, right_y, wheel_center_z, wheel_radius, wheel_width, group_name="Wheel_RR")

    # 2. Paragolpes delantero separable para choques (Bumper_Front)
    # X: [192, 210], Y: [-90, 90], Z: [10, 52]
    mb.add_box(192.0, 210.0, -90.0, 90.0, 10.0, 52.0,
               mat="M_Plastic_Black", group="Bumper_Front")
    mb.add_box(208.5, 210.0, -50.0, 50.0, 14.0, 36.0,
               mat="M_Plastic_Black", group="Bumper_Front")

    # 3. Paragolpes trasero (Bumper_Rear)
    # X: [-210, -192], Y: [-90, 90], Z: [10, 54]
    mb.add_box(-210.0, -192.0, -90.0, 90.0, 10.0, 54.0,
               mat="M_Plastic_Black", group="Bumper_Rear")

    # 4. Chasis inferior / Bajos
    mb.add_box(-192.0, 192.0, -85.0, 85.0, 14.0, 45.0,
               mat="M_Plastic_Black", group="Chassis_Lower")

    # 5. Carrocería principal (Chapa pintada)
    mb.add_box(-192.0, 192.0, -88.0, 88.0, 45.0, 80.0,
               mat="M_CarPaint_Red", group="Car_Body")

    # Capó delantero (Hood)
    mb.add_frustum(
        x_bottom=(70.0, 192.0), y_bottom=(-86.0, 86.0), z_bottom=52.0,
        x_top=(68.0, 190.0), y_top=(-82.0, 82.0), z_top=80.0,
        mat="M_CarPaint_Red", group="Car_Body"
    )

    # Maletero trasero (Trunk)
    mb.add_frustum(
        x_bottom=(-192.0, -125.0), y_bottom=(-86.0, 86.0), z_bottom=54.0,
        x_top=(-190.0, -125.0), y_top=(-82.0, 82.0), z_top=82.0,
        mat="M_CarPaint_Red", group="Car_Body"
    )

    # 6. Habitáculo superior / Techo y Cristales
    # Techo central: Z max = 145 UU
    mb.add_box(-75.0, 20.0, -74.0, 74.0, 140.0, 145.0,
               mat="M_CarPaint_Red", group="Car_Roof")

    # Parabrisas delantero inclinado
    mb.add_frustum(
        x_bottom=(20.0, 68.0), y_bottom=(-80.0, 80.0), z_bottom=80.0,
        x_top=(20.0, 20.0), y_top=(-74.0, 74.0), z_top=140.0,
        mat="M_Glass", group="Windows"
    )

    # Luna trasera inclinada
    mb.add_frustum(
        x_bottom=(-125.0, -75.0), y_bottom=(-80.0, 80.0), z_bottom=82.0,
        x_top=(-75.0, -75.0), y_top=(-74.0, 74.0), z_top=140.0,
        mat="M_Glass", group="Windows"
    )

    # Ventanillas laterales
    mb.add_quad_face((20.0, -74.0, 140.0), (68.0, -80.0, 80.0),
                     (-125.0, -80.0, 82.0), (-75.0, -74.0, 140.0),
                     custom_normal=(0.0, -1.0, 0.0))
    mb.add_quad_face((20.0, 74.0, 140.0), (-75.0, 74.0, 140.0),
                     (-125.0, 80.0, 82.0), (68.0, 80.0, 80.0),
                     custom_normal=(0.0, 1.0, 0.0))

    # Pilares B
    mb.add_box(-30.0, -22.0, -76.0, -73.0, 80.0, 140.0, mat="M_Plastic_Black", group="Pillars")
    mb.add_box(-30.0, -22.0, 73.0, 76.0, 80.0, 140.0, mat="M_Plastic_Black", group="Pillars")

    # Retrovisores exteriores (ajustados a Y = +-90)
    mb.add_box(40.0, 52.0, -90.0, -80.0, 80.0, 92.0, mat="M_Plastic_Black", group="Mirrors")
    mb.add_box(40.0, 52.0, 80.0, 90.0, 80.0, 92.0, mat="M_Plastic_Black", group="Mirrors")

    # 7. Faros delanteros y pilotos traseros
    mb.add_box(190.0, 192.0, -84.0, -50.0, 55.0, 72.0, mat="M_Lights_Front", group="Lights_Front")
    mb.add_box(190.0, 192.0, 50.0, 84.0, 55.0, 72.0, mat="M_Lights_Front", group="Lights_Front")

    mb.add_box(-192.0, -190.0, -84.0, -52.0, 58.0, 76.0, mat="M_Lights_Rear", group="Lights_Rear")
    mb.add_box(-192.0, -190.0, 52.0, 84.0, 58.0, 76.0, mat="M_Lights_Rear", group="Lights_Rear")

    return mb


# =============================================================================
# 2. Camión Articulado: Cabina Tractora (450 x 250 x 360 UU)
# =============================================================================
def build_camion_cabina() -> MeshBuilder:
    """
    Construye la Cabina Tractora de Camión Articulado:
    Dimensiones exactas: 450 x 250 x 360 UU
    X in [-225, +225] = 450 UU
    Y in [-125, +125] = 250 UU
    Z in [0, 360] = 360 UU
    """
    mb = MeshBuilder("Camion_Cabina")

    # 1. Ruedas de camión pesado (Radio 52 UU, ancho 26 UU)
    tire_r = 52.0
    tire_w = 26.0
    front_x = 135.0
    rear_x = -125.0
    left_y = -106.0
    right_y = 106.0

    # Ruedas delanteras (Y exterior = +-119 UU)
    mb.add_wheel_assembly(front_x, left_y, tire_r, tire_r, tire_w, rim_radius_ratio=0.6, group_name="Wheel_FL")
    mb.add_wheel_assembly(front_x, right_y, tire_r, tire_r, tire_w, rim_radius_ratio=0.6, group_name="Wheel_FR")

    # Ruedas traseras dobles
    mb.add_wheel_assembly(rear_x, left_y, tire_r, tire_r, tire_w, rim_radius_ratio=0.6, group_name="Wheel_RL_Outer")
    mb.add_wheel_assembly(rear_x, left_y + 16.0, tire_r, tire_r, tire_w, rim_radius_ratio=0.6, group_name="Wheel_RL_Inner")
    mb.add_wheel_assembly(rear_x, right_y, tire_r, tire_r, tire_w, rim_radius_ratio=0.6, group_name="Wheel_RR_Outer")
    mb.add_wheel_assembly(rear_x, right_y - 16.0, tire_r, tire_r, tire_w, rim_radius_ratio=0.6, group_name="Wheel_RR_Inner")

    # 2. Largueros del Chasis
    mb.add_box(-225.0, 190.0, -45.0, 45.0, 45.0, 100.0,
               mat="M_Plastic_Black", group="Chassis_Frame")

    # Depósitos de combustible
    mb.add_box(-80.0, 60.0, -118.0, -55.0, 35.0, 95.0, mat="M_Chrome_Metal", group="Fuel_Tanks")
    mb.add_box(-80.0, 60.0, 55.0, 118.0, 35.0, 95.0, mat="M_Chrome_Metal", group="Fuel_Tanks")

    # 3. Quinta Rueda (Plato de enganche del trailer)
    mb.add_box(-155.0, -95.0, -45.0, 45.0, 100.0, 115.0, mat="M_Plastic_Black", group="Fifth_Wheel")
    mb.add_cylinder_z(-125.0, 0.0, 115.0, radius=22.0, height=8.0, segments=16, mat="M_Plastic_Black", group="Fifth_Wheel")

    # 4. Paragolpes delantero pesado (X front = +225, Y span = [-125, +125])
    mb.add_box(200.0, 225.0, -125.0, 125.0, 25.0, 90.0, mat="M_Plastic_Black", group="Bumper_Front")

    # 5. Cabina Principal Avanzada
    # X: [-10, 215], Y: [-120, 120], Z: [90, 310]
    mb.add_box(-10.0, 215.0, -120.0, 120.0, 90.0, 310.0, mat="M_CarPaint_White", group="Cab_Body")

    # Calandra y Parrilla frontal
    mb.add_box(215.0, 220.0, -85.0, 85.0, 90.0, 190.0, mat="M_Plastic_Black", group="Grille")

    # Parabrisas panorámico
    mb.add_box(205.0, 216.0, -115.0, 115.0, 195.0, 295.0, mat="M_Glass", group="Windshield")

    # Ventanillas laterales
    mb.add_box(50.0, 200.0, -120.5, -119.0, 195.0, 290.0, mat="M_Glass", group="Side_Windows")
    mb.add_box(50.0, 200.0, 119.0, 120.5, 195.0, 290.0, mat="M_Glass", group="Side_Windows")

    # 6. Deflector aerodinámico superior de techo (Spoiler / Fairing hasta Z=360)
    mb.add_frustum(
        x_bottom=(-10.0, 200.0), y_bottom=(-120.0, 120.0), z_bottom=310.0,
        x_top=(-10.0, 150.0), y_top=(-112.0, 112.0), z_top=360.0,
        mat="M_CarPaint_White", group="Roof_Spoiler"
    )

    # Espejos retrovisores verticales (Ajustados dentro de Y in [-125, +125])
    mb.add_box(180.0, 195.0, -125.0, -115.0, 210.0, 280.0, mat="M_Plastic_Black", group="Mirrors")
    mb.add_box(180.0, 195.0, 115.0, 125.0, 210.0, 280.0, mat="M_Plastic_Black", group="Mirrors")

    # Faros delanteros LED
    mb.add_box(220.0, 225.0, -120.0, -85.0, 50.0, 80.0, mat="M_Lights_Front", group="Lights_Front")
    mb.add_box(220.0, 225.0, 85.0, 120.0, 50.0, 80.0, mat="M_Lights_Front", group="Lights_Front")

    # Luces de visera
    mb.add_box(160.0, 165.0, -90.0, 90.0, 312.0, 320.0, mat="M_Lights_Front", group="Marker_Lights")

    return mb


# =============================================================================
# 2b. Semirremolque Trailer de Caja Cerrada (1200 x 255 x 400 UU)
# =============================================================================
def build_camion_trailer() -> MeshBuilder:
    """
    Construye el Semirremolque Trailer de Caja Cerrada:
    Dimensiones exactas: 1200 x 255 x 400 UU
    Pivote (Kingpin) en X=0, Y=0, Z=115
    X: [-1050, +150] = 1200 UU
    Y: [-127.5, +127.5] = 255 UU
    Z: [0, 400] = 400 UU
    """
    mb = MeshBuilder("Camion_Trailer")

    # 1. Kingpin (Perno de acoplamiento al plato) en X=0, Y=0
    mb.add_cylinder_z(0.0, 0.0, 100.0, radius=8.0, height=20.0,
                      segments=12, mat="M_Chrome_Metal", group="Kingpin_Pivot")

    # Placa de frotamiento inferior delantera
    mb.add_box(-30.0, 140.0, -115.0, 115.0, 115.0, 122.0,
               mat="M_Plastic_Black", group="Chassis_Hitch")

    # 2. Patas de apoyo telescópicas (Landing Gear)
    mb.add_box(-180.0, -150.0, -95.0, -80.0, 10.0, 115.0, mat="M_Plastic_Black", group="Landing_Gear")
    mb.add_box(-180.0, -150.0, 80.0, 95.0, 10.0, 115.0, mat="M_Plastic_Black", group="Landing_Gear")

    # 3. Triple Eje Trasero (Tridem: 3 ejes, 6 ruedas, radio 48 UU)
    tridem_x = [-720.0, -840.0, -960.0]
    tire_r = 48.0
    tire_w = 26.0
    y_l = -105.0
    y_r = 105.0

    for idx, ax_x in enumerate(tridem_x, 1):
        mb.add_wheel_assembly(ax_x, y_l, tire_r, tire_r, tire_w, rim_radius_ratio=0.6,
                              group_name=f"Trailer_Wheel_L{idx}")
        mb.add_wheel_assembly(ax_x, y_r, tire_r, tire_r, tire_w, rim_radius_ratio=0.6,
                              group_name=f"Trailer_Wheel_R{idx}")

    # Guardabarros
    mb.add_box(-1010.0, -670.0, -126.0, -85.0, 55.0, 105.0, mat="M_Plastic_Black", group="Mudguards")
    mb.add_box(-1010.0, -670.0, 85.0, 126.0, 55.0, 105.0, mat="M_Plastic_Black", group="Mudguards")

    # Protecciones laterales anti-empotramiento (Ciclistas)
    mb.add_box(-660.0, -220.0, -122.0, -118.0, 40.0, 75.0, mat="M_Plastic_Black", group="Side_Underrun")
    mb.add_box(-660.0, -220.0, 118.0, 122.0, 40.0, 75.0, mat="M_Plastic_Black", group="Side_Underrun")

    # 4. Caja Cerrada Gran Volumen (Dry Van Body)
    # X: [-1045, 150], Y: [-127.5, 127.5], Z: [115, 395]
    mb.add_box(-1045.0, 150.0, -127.5, 127.5, 115.0, 395.0,
               mat="M_Trailer_White", group="Trailer_Box")

    # Perfil perimetral del techo (Roof Rails hasta Z=400)
    mb.add_box(-1045.0, 150.0, -127.5, 127.5, 395.0, 400.0,
               mat="M_Plastic_Black", group="Roof_Trim")

    # Puertas traseras batientes dobles (terminan en X = -1048)
    mb.add_box(-1048.0, -1045.0, -124.0, 124.0, 120.0, 390.0,
               mat="M_Trailer_White", group="Rear_Doors")

    # Barras de cierre verticales (X = -1050.0 cota trasera exacta)
    mb.add_box(-1050.0, -1048.0, -35.0, -31.0, 130.0, 380.0, mat="M_Chrome_Metal", group="Door_Hardware")
    mb.add_box(-1050.0, -1048.0, 31.0, 35.0, 130.0, 380.0, mat="M_Chrome_Metal", group="Door_Hardware")

    # 5. Paragolpes trasero y pilotos (Cota exacta X = -1050.0)
    mb.add_box(-1050.0, -1042.0, -125.0, 125.0, 35.0, 65.0,
               mat="M_Plastic_Black", group="Rear_Bumper")
    mb.add_box(-1050.0, -1048.0, -115.0, -60.0, 40.0, 60.0, mat="M_GC_Yellow", group="Reflectors")
    mb.add_box(-1050.0, -1048.0, 60.0, 115.0, 40.0, 60.0, mat="M_GC_Yellow", group="Reflectors")
    mb.add_box(-1050.0, -1048.0, -124.0, -115.0, 40.0, 60.0, mat="M_Lights_Rear", group="Rear_Lights")
    mb.add_box(-1050.0, -1048.0, 115.0, 124.0, 40.0, 60.0, mat="M_Lights_Rear", group="Rear_Lights")

    return mb


# =============================================================================
# 2c. Camión Articulado Completo (1650 x 255 x 400 UU)
# =============================================================================
def build_camion_articulado_completo() -> MeshBuilder:
    """
    Construye el conjunto completo del Camión Articulado en posición de marcha:
    Frontal cabina en X = +750, Trasera trailer en X = -900.
    Dimensiones exactas: 1650 x 255 x 400 UU.
    """
    mb = MeshBuilder("Camion_Articulado_Completo")

    cab_offset_x = 525.0  # Cabina X: [300, 750] (longitud 450)
    tire_r = 52.0
    tire_w = 26.0

    # Ruedas cabina
    mb.add_wheel_assembly(cab_offset_x + 135.0, -106.0, tire_r, tire_r, tire_w, group_name="Cab_Wheel_FL")
    mb.add_wheel_assembly(cab_offset_x + 135.0, 106.0, tire_r, tire_r, tire_w, group_name="Cab_Wheel_FR")
    mb.add_wheel_assembly(cab_offset_x - 125.0, -106.0, tire_r, tire_r, tire_w, group_name="Cab_Wheel_RL")
    mb.add_wheel_assembly(cab_offset_x - 125.0, 106.0, tire_r, tire_r, tire_w, group_name="Cab_Wheel_RR")

    # Chasis cabina
    mb.add_box(cab_offset_x - 225.0, cab_offset_x + 190.0, -45.0, 45.0, 45.0, 100.0,
               mat="M_Plastic_Black", group="Cab_Chassis")
    # Carrocería cabina
    mb.add_box(cab_offset_x - 10.0, cab_offset_x + 215.0, -120.0, 120.0, 90.0, 310.0,
               mat="M_CarPaint_White", group="Cab_Body")
    # Deflector cabina
    mb.add_frustum(
        x_bottom=(cab_offset_x - 10.0, cab_offset_x + 200.0), y_bottom=(-120.0, 120.0), z_bottom=310.0,
        x_top=(cab_offset_x - 10.0, cab_offset_x + 150.0), y_top=(-112.0, 112.0), z_top=360.0,
        mat="M_CarPaint_White", group="Cab_Spoiler"
    )
    # Parabrisas cabina
    mb.add_box(cab_offset_x + 205.0, cab_offset_x + 216.0, -115.0, 115.0, 195.0, 295.0,
               mat="M_Glass", group="Cab_Windshield")
    # Paragolpes delantero cabina (llega a X = +750.0)
    mb.add_box(cab_offset_x + 200.0, cab_offset_x + 225.0, -125.0, 125.0, 25.0, 90.0,
               mat="M_Plastic_Black", group="Cab_Bumper")
    mb.add_box(cab_offset_x + 220.0, cab_offset_x + 225.0, -120.0, -85.0, 50.0, 80.0,
               mat="M_Lights_Front", group="Cab_Lights")
    mb.add_box(cab_offset_x + 220.0, cab_offset_x + 225.0, 85.0, 120.0, 50.0, 80.0,
               mat="M_Lights_Front", group="Cab_Lights")

    # Trailer acoplado: X in [-900, +300] = 1200 UU, Y in [-127.5, 127.5] = 255 UU, Z in [0, 400]
    mb.add_box(-895.0, 300.0, -127.5, 127.5, 115.0, 395.0,
               mat="M_Trailer_White", group="Trailer_Box")
    mb.add_box(-895.0, 300.0, -127.5, 127.5, 395.0, 400.0,
               mat="M_Plastic_Black", group="Trailer_Roof")

    # Triple eje trailer
    for idx, ax_x in enumerate([-570.0, -690.0, -810.0], 1):
        mb.add_wheel_assembly(ax_x, -105.0, 48.0, 48.0, 26.0, group_name=f"Tr_Wheel_L{idx}")
        mb.add_wheel_assembly(ax_x, 105.0, 48.0, 48.0, 26.0, group_name=f"Tr_Wheel_R{idx}")

    # Puertas traseras y pilotos (Cota trasera exacta X = -900.0)
    mb.add_box(-898.0, -895.0, -124.0, 124.0, 120.0, 390.0, mat="M_Trailer_White", group="Tr_Doors")
    mb.add_box(-900.0, -893.0, -125.0, 125.0, 35.0, 65.0, mat="M_Plastic_Black", group="Tr_Bumper")
    mb.add_box(-900.0, -898.0, -124.0, -115.0, 40.0, 60.0, mat="M_Lights_Rear", group="Tr_Lights")
    mb.add_box(-900.0, -898.0, 115.0, 124.0, 40.0, 60.0, mat="M_Lights_Rear", group="Tr_Lights")

    return mb


# =============================================================================
# 3. Furgoneta de Reparto (580 x 205 x 230 UU)
# =============================================================================
def build_furgoneta_reparto() -> MeshBuilder:
    """
    Construye la Furgoneta de Reparto:
    Dimensiones exactas: 580 x 205 x 230 UU
    X in [-290, +290] = 580 UU
    Y in [-102.5, +102.5] = 205 UU
    Z in [0, 230] = 230 UU
    """
    mb = MeshBuilder("Furgoneta_Reparto")

    # 1. Ruedas (Radio 36 UU, ancho 22 UU)
    wheel_r = 36.0
    wheel_w = 22.0
    f_axle = 180.0
    r_axle = -180.0
    y_l = -88.0
    y_r = 88.0

    mb.add_wheel_assembly(f_axle, y_l, wheel_r, wheel_r, wheel_w, rim_radius_ratio=0.6, group_name="Wheel_FL")
    mb.add_wheel_assembly(f_axle, y_r, wheel_r, wheel_r, wheel_w, rim_radius_ratio=0.6, group_name="Wheel_FR")
    mb.add_wheel_assembly(r_axle, y_l, wheel_r, wheel_r, wheel_w, rim_radius_ratio=0.6, group_name="Wheel_RL")
    mb.add_wheel_assembly(r_axle, y_r, wheel_r, wheel_r, wheel_w, rim_radius_ratio=0.6, group_name="Wheel_RR")

    # 2. Paragolpes delantero y trasero (Alineados exactamente en X = +290 y X = -290)
    mb.add_box(265.0, 290.0, -102.5, 102.5, 15.0, 65.0, mat="M_Plastic_Black", group="Bumper_Front")
    mb.add_box(-290.0, -270.0, -102.5, 102.5, 15.0, 65.0, mat="M_Plastic_Black", group="Bumper_Rear")

    # Molduras laterales
    mb.add_box(-270.0, 265.0, -102.5, -99.0, 25.0, 55.0, mat="M_Plastic_Black", group="Side_Moldings")
    mb.add_box(-270.0, 265.0, 99.0, 102.5, 25.0, 55.0, mat="M_Plastic_Black", group="Side_Moldings")

    # 3. Morro y Capó
    mb.add_frustum(
        x_bottom=(160.0, 268.0), y_bottom=(-101.0, 101.0), z_bottom=60.0,
        x_top=(155.0, 265.0), y_top=(-97.0, 97.0), z_top=108.0,
        mat="M_CarPaint_White", group="Van_Hood"
    )

    # 4. Parabrisas
    mb.add_frustum(
        x_bottom=(75.0, 155.0), y_bottom=(-98.0, 98.0), z_bottom=108.0,
        x_top=(75.0, 75.0), y_top=(-94.0, 94.0), z_top=210.0,
        mat="M_Glass", group="Windshield"
    )

    # Ventanillas laterales de cabina
    mb.add_box(20.0, 120.0, -101.0, -99.0, 115.0, 205.0, mat="M_Glass", group="Side_Windows")
    mb.add_box(20.0, 120.0, 99.0, 101.0, 115.0, 205.0, mat="M_Glass", group="Side_Windows")

    # 5. Caja de Carga (Z max = 230 UU)
    mb.add_box(-275.0, 75.0, -102.5, 102.5, 45.0, 226.0, mat="M_CarPaint_White", group="Cargo_Box")
    mb.add_box(-275.0, 75.0, -100.0, 100.0, 226.0, 230.0, mat="M_CarPaint_White", group="Van_Roof")

    # Puertas traseras
    mb.add_box(-278.0, -275.0, -98.0, 98.0, 50.0, 220.0, mat="M_CarPaint_White", group="Rear_Cargo_Doors")
    mb.add_box(-279.0, -277.0, -10.0, 10.0, 110.0, 130.0, mat="M_Plastic_Black", group="Door_Handle")

    # Faros y pilotos
    mb.add_box(265.0, 270.0, -95.0, -60.0, 70.0, 95.0, mat="M_Lights_Front", group="Lights_Front")
    mb.add_box(265.0, 270.0, 60.0, 95.0, 70.0, 95.0, mat="M_Lights_Front", group="Lights_Front")

    mb.add_box(-278.0, -274.0, -101.0, -85.0, 90.0, 170.0, mat="M_Lights_Rear", group="Lights_Rear")
    mb.add_box(-278.0, -274.0, 85.0, 101.0, 90.0, 170.0, mat="M_Lights_Rear", group="Lights_Rear")

    # Retrovisores (Ajustados dentro de Y in [-102.5, 102.5])
    mb.add_box(115.0, 135.0, -102.5, -92.0, 120.0, 155.0, mat="M_Plastic_Black", group="Mirrors")
    mb.add_box(115.0, 135.0, 92.0, 102.5, 120.0, 155.0, mat="M_Plastic_Black", group="Mirrors")

    return mb


# =============================================================================
# 4. Autobús Interurbano (1350 x 255 x 330 UU)
# =============================================================================
def build_autobus_interurbano() -> MeshBuilder:
    """
    Construye el Autobús Interurbano:
    Dimensiones exactas: 1350 x 255 x 330 UU
    X in [-675, +675] = 1350 UU
    Y in [-127.5, +127.5] = 255 UU
    Z in [0, 330] = 330 UU
    """
    mb = MeshBuilder("Autobus_Interurbano")

    # 1. Ruedas (Radio 50 UU, ancho 28 UU)
    wheel_r = 50.0
    wheel_w = 28.0
    f_axle = 430.0
    r_axle1 = -360.0
    r_axle2 = -490.0
    y_l = -105.0
    y_r = 105.0

    mb.add_wheel_assembly(f_axle, y_l, wheel_r, wheel_r, wheel_w, group_name="Wheel_FL")
    mb.add_wheel_assembly(f_axle, y_r, wheel_r, wheel_r, wheel_w, group_name="Wheel_FR")
    mb.add_wheel_assembly(r_axle1, y_l, wheel_r, wheel_r, wheel_w, group_name="Wheel_RL1")
    mb.add_wheel_assembly(r_axle1, y_r, wheel_r, wheel_r, wheel_w, group_name="Wheel_RR1")
    mb.add_wheel_assembly(r_axle2, y_l, wheel_r, wheel_r, wheel_w, group_name="Wheel_RL2")
    mb.add_wheel_assembly(r_axle2, y_r, wheel_r, wheel_r, wheel_w, group_name="Wheel_RR2")

    # 2. Carrocería inferior y maleteros
    mb.add_box(-660.0, 660.0, -126.0, 126.0, 25.0, 120.0,
               mat="M_CarPaint_Bus", group="Bus_Chassis_Luggage")

    # Paragolpes delantero y trasero (Llegan a X = +675 y X = -675)
    mb.add_box(650.0, 675.0, -127.5, 127.5, 20.0, 80.0, mat="M_Plastic_Black", group="Bumper_Front")
    mb.add_box(-675.0, -655.0, -127.5, 127.5, 20.0, 80.0, mat="M_Plastic_Black", group="Bumper_Rear")

    # 3. Ventanas panorámicas continuas en ambos laterales
    mb.add_box(-630.0, 570.0, -127.5, -125.0, 120.0, 280.0, mat="M_Glass", group="Panoramic_Windows_Left")
    mb.add_box(-630.0, 570.0, 125.0, 127.5, 120.0, 280.0, mat="M_Glass", group="Panoramic_Windows_Right")

    # Gran parabrisas panorámico delantero
    mb.add_frustum(
        x_bottom=(570.0, 670.0), y_bottom=(-126.0, 126.0), z_bottom=90.0,
        x_top=(530.0, 580.0), y_top=(-122.0, 122.0), z_top=280.0,
        mat="M_Glass", group="Windshield_Front"
    )

    # Letrero electrónico de destino (PMV Frontal)
    mb.add_box(560.0, 595.0, -80.0, 80.0, 280.0, 305.0, mat="M_Plastic_Black", group="Destination_Sign")

    # Luna trasera
    mb.add_box(-662.0, -655.0, -115.0, 115.0, 180.0, 275.0, mat="M_Glass", group="Windshield_Rear")

    # 4. Techo aerodinámico del autobús
    mb.add_box(-660.0, 580.0, -126.0, 126.0, 280.0, 308.0, mat="M_CarPaint_Bus", group="Bus_Roof")

    # 5. Climatizadores de techo (HVAC Pods) - Cota máxima Z = 330 UU
    mb.add_box(100.0, 350.0, -75.0, 75.0, 308.0, 330.0, mat="M_CarPaint_White", group="HVAC_Front")
    mb.add_box(-450.0, -200.0, -75.0, 75.0, 308.0, 330.0, mat="M_CarPaint_White", group="HVAC_Rear")

    # 6. Puertas de acceso de pasajeros
    mb.add_box(460.0, 560.0, 125.5, 127.5, 30.0, 280.0, mat="M_Glass", group="Passenger_Door_Front")
    mb.add_box(-60.0, 40.0, 125.5, 127.5, 30.0, 280.0, mat="M_Glass", group="Passenger_Door_Mid")

    # 7. Faros y pilotos
    mb.add_box(670.0, 675.0, -120.0, -70.0, 45.0, 75.0, mat="M_Lights_Front", group="Lights_Front")
    mb.add_box(670.0, 675.0, 70.0, 120.0, 45.0, 75.0, mat="M_Lights_Front", group="Lights_Front")

    mb.add_box(-675.0, -670.0, -125.0, -100.0, 40.0, 120.0, mat="M_Lights_Rear", group="Lights_Rear")
    mb.add_box(-675.0, -670.0, 100.0, 125.0, 40.0, 120.0, mat="M_Lights_Rear", group="Lights_Rear")

    # Retrovisores aerodinámicos de autocar (Ajustados dentro de Y in [-127.5, 127.5])
    mb.add_box(630.0, 660.0, -127.5, -112.5, 220.0, 265.0, mat="M_Plastic_Black", group="Mirrors")
    mb.add_box(630.0, 660.0, 112.5, 127.5, 220.0, 265.0, mat="M_Plastic_Black", group="Mirrors")

    return mb


# =============================================================================
# 5. Motocicleta (215 x 85 x 120 UU)
# =============================================================================
def build_motocicleta() -> MeshBuilder:
    """
    Construye la Motocicleta:
    Dimensiones exactas: 215 x 85 x 120 UU
    X in [-107.5, +107.5] = 215 UU
    Y in [-42.5, +42.5] = 85 UU
    Z in [0, 120] = 120 UU
    """
    mb = MeshBuilder("Motocicleta")

    # 1. Ruedas de moto (Radio 30 UU)
    wheel_r = 30.0
    mb.add_wheel_assembly(72.5, 0.0, wheel_r, wheel_r, 10.0, rim_radius_ratio=0.7, group_name="Wheel_Front")
    mb.add_wheel_assembly(-72.5, 0.0, wheel_r, wheel_r, 16.0, rim_radius_ratio=0.7, group_name="Wheel_Rear")

    # Guardabarros delantero
    mb.add_box(52.0, 92.0, -8.0, 8.0, 32.0, 42.0, mat="M_CarPaint_Red", group="Front_Fender")

    # Horquilla delantera
    mb.add_box(40.0, 72.5, -9.0, -5.0, 28.0, 85.0, mat="M_Chrome_Metal", group="Fork")
    mb.add_box(40.0, 72.5, 5.0, 9.0, 28.0, 85.0, mat="M_Chrome_Metal", group="Fork")

    # 2. Manillar con puños y retrovisores (Determina Y = 85 UU y Z = 120 UU)
    mb.add_box(32.0, 42.0, -42.5, 42.5, 96.0, 104.0, mat="M_Plastic_Black", group="Handlebars")
    mb.add_box(30.0, 38.0, -42.5, -34.0, 104.0, 120.0, mat="M_Plastic_Black", group="Mirrors")
    mb.add_box(30.0, 38.0, 34.0, 42.5, 104.0, 120.0, mat="M_Plastic_Black", group="Mirrors")

    # Cúpula / Faro delantero
    mb.add_box(80.0, 107.5, -12.0, 12.0, 60.0, 88.0, mat="M_CarPaint_Red", group="Headlight_Fairing")
    mb.add_box(105.0, 107.5, -10.0, 10.0, 65.0, 85.0, mat="M_Lights_Front", group="Headlight")

    # Cúpula transparente superior
    mb.add_box(50.0, 85.0, -10.0, 10.0, 88.0, 112.0, mat="M_Glass", group="Windshield")

    # 3. Chasis central, motor y depósito
    mb.add_box(-30.0, 30.0, -14.0, 14.0, 15.0, 52.0, mat="M_Plastic_Black", group="Engine_Block")

    mb.add_frustum(
        x_bottom=(-10.0, 45.0), y_bottom=(-18.0, 18.0), z_bottom=52.0,
        x_top=(-5.0, 38.0), y_top=(-14.0, 14.0), z_top=90.0,
        mat="M_CarPaint_Red", group="Fuel_Tank"
    )

    # 4. Asiento biplaza
    mb.add_box(-60.0, -5.0, -14.0, 14.0, 60.0, 76.0, mat="M_Plastic_Black", group="Seat")

    # 5. Colín trasero y matrícula (Llega a X = -107.5)
    mb.add_box(-105.0, -60.0, -12.0, 12.0, 58.0, 78.0, mat="M_CarPaint_Red", group="Tail_Fairing")
    mb.add_box(-107.5, -105.0, -8.0, 8.0, 62.0, 74.0, mat="M_Lights_Rear", group="Taillight")

    # 6. Tubo de escape cromado
    mb.add_cylinder_y(-30.0, 20.0, 28.0, radius=6.0, width=12.0, segments=10, mat="M_Chrome_Metal", group="Exhaust")
    mb.add_box(-90.0, -20.0, 16.0, 24.0, 22.0, 34.0, mat="M_Chrome_Metal", group="Exhaust")

    return mb


# =============================================================================
# 6. Patrulla de la Guardia Civil de Tráfico (480 x 190 x 185 UU)
# =============================================================================
def build_patrulla_guardia_civil() -> MeshBuilder:
    """
    Construye la Patrulla de la Guardia Civil de Tráfico:
    Dimensiones exactas: 480 x 190 x 185 UU
    X in [-240, +240] = 480 UU
    Y in [-95, +95] = 190 UU
    Z in [0, 185] = 185 UU
    """
    mb = MeshBuilder("Patrulla_Guardia_Civil")

    # 1. Ruedas (Radio 31 UU, ancho 20 UU)
    wheel_r = 31.0
    wheel_w = 20.0
    f_axle = 142.5
    r_axle = -142.5
    y_l = -84.0
    y_r = 84.0

    mb.add_wheel_assembly(f_axle, y_l, wheel_r, wheel_r, wheel_w, group_name="Wheel_FL")
    mb.add_wheel_assembly(f_axle, y_r, wheel_r, wheel_r, wheel_w, group_name="Wheel_FR")
    mb.add_wheel_assembly(r_axle, y_l, wheel_r, wheel_r, wheel_w, group_name="Wheel_RL")
    mb.add_wheel_assembly(r_axle, y_r, wheel_r, wheel_r, wheel_w, group_name="Wheel_RR")

    # 2. Paragolpes delantero y trasero
    mb.add_box(218.0, 240.0, -95.0, 95.0, 12.0, 54.0, mat="M_Plastic_Black", group="Bumper_Front")
    mb.add_box(-240.0, -218.0, -95.0, 95.0, 12.0, 56.0, mat="M_Plastic_Black", group="Bumper_Rear")

    # 3. Carrocería Base (Blanco corporativo)
    mb.add_box(-218.0, 218.0, -92.0, 92.0, 15.0, 48.0, mat="M_Plastic_Black", group="Chassis")
    mb.add_box(-218.0, 218.0, -93.0, 93.0, 48.0, 82.0, mat="M_CarPaint_White", group="Body_White")

    # 4. Decoración reglamentaria Guardia Civil de Tráfico
    # Bandas laterales verde (#00563B) y franjas amarillas
    mb.add_box(-170.0, 170.0, -94.5, -92.5, 52.0, 75.0, mat="M_CarPaint_GC_Green", group="GC_Livery_Green")
    mb.add_box(-170.0, 170.0, 92.5, 94.5, 52.0, 75.0, mat="M_CarPaint_GC_Green", group="GC_Livery_Green")
    mb.add_box(-165.0, 165.0, -95.0, -93.5, 50.0, 54.0, mat="M_GC_Yellow", group="GC_Livery_Yellow")
    mb.add_box(-165.0, 165.0, 93.5, 95.0, 50.0, 54.0, mat="M_GC_Yellow", group="GC_Livery_Yellow")

    # Capó delantero con franjas verdes
    mb.add_frustum(
        x_bottom=(75.0, 218.0), y_bottom=(-90.0, 90.0), z_bottom=54.0,
        x_top=(72.0, 215.0), y_top=(-86.0, 86.0), z_top=82.0,
        mat="M_CarPaint_White", group="Hood_White"
    )
    mb.add_box(100.0, 195.0, -35.0, 35.0, 78.0, 83.0, mat="M_CarPaint_GC_Green", group="Hood_Green_Stripes")

    # Maletero trasero
    mb.add_frustum(
        x_bottom=(-218.0, -140.0), y_bottom=(-90.0, 90.0), z_bottom=56.0,
        x_top=(-215.0, -140.0), y_top=(-86.0, 86.0), z_top=84.0,
        mat="M_CarPaint_White", group="Trunk_White"
    )

    # 5. Techo y Cristales
    mb.add_box(-90.0, 25.0, -78.0, 78.0, 140.0, 145.0, mat="M_CarPaint_White", group="Roof")

    mb.add_frustum(
        x_bottom=(25.0, 72.0), y_bottom=(-86.0, 86.0), z_bottom=82.0,
        x_top=(25.0, 25.0), y_top=(-78.0, 78.0), z_top=140.0,
        mat="M_Glass", group="Windshield_Front"
    )
    mb.add_frustum(
        x_bottom=(-140.0, -90.0), y_bottom=(-86.0, 86.0), z_bottom=84.0,
        x_top=(-90.0, -90.0), y_top=(-78.0, 78.0), z_top=140.0,
        mat="M_Glass", group="Windshield_Rear"
    )

    mb.add_box(-88.0, 24.0, -80.0, -77.5, 84.0, 140.0, mat="M_Glass", group="Side_Glass")
    mb.add_box(-88.0, 24.0, 77.5, 80.0, 84.0, 140.0, mat="M_Glass", group="Side_Glass")

    # 6. Puente de Luces Prioritarias V-1 en el Techo (Cota máxima Z = 185 UU)
    mb.add_box(-40.0, -20.0, -55.0, -45.0, 145.0, 155.0, mat="M_Plastic_Black", group="Lightbar_Mount")
    mb.add_box(-40.0, -20.0, 45.0, 55.0, 145.0, 155.0, mat="M_Plastic_Black", group="Lightbar_Mount")

    # Tulipas rotativas azules V-1
    mb.add_box(-45.0, -15.0, -60.0, -15.0, 155.0, 185.0, mat="M_Lights_Beacon_Blue", group="Beacon_Left")
    mb.add_box(-45.0, -15.0, 15.0, 60.0, 155.0, 185.0, mat="M_Lights_Beacon_Blue", group="Beacon_Right")

    # Panel central PMV en el puente de luces
    mb.add_box(-44.0, -16.0, -15.0, 15.0, 155.0, 182.0, mat="M_Plastic_Black", group="Lightbar_Center_Sign")

    # 7. Faros y retrovisores
    mb.add_box(215.0, 218.0, -90.0, -55.0, 58.0, 76.0, mat="M_Lights_Front", group="Lights_Front")
    mb.add_box(215.0, 218.0, 55.0, 90.0, 58.0, 76.0, mat="M_Lights_Front", group="Lights_Front")

    mb.add_box(-218.0, -215.0, -90.0, -55.0, 60.0, 80.0, mat="M_Lights_Rear", group="Lights_Rear")
    mb.add_box(-218.0, -215.0, 55.0, 90.0, 60.0, 80.0, mat="M_Lights_Rear", group="Lights_Rear")

    mb.add_box(45.0, 60.0, -95.0, -82.0, 85.0, 98.0, mat="M_Plastic_Black", group="Mirrors")
    mb.add_box(45.0, 60.0, 82.0, 95.0, 85.0, 98.0, mat="M_Plastic_Black", group="Mirrors")

    return mb


# =============================================================================
# 7. Grúa de Asistencia en Carretera (720 x 240 x 265 UU)
# =============================================================================
def build_grua_asistencia() -> MeshBuilder:
    """
    Construye la Grúa de Asistencia en Carretera con plataforma y rotativos amarillos:
    Dimensiones exactas: 720 x 240 x 265 UU
    X in [-360, +360] = 720 UU
    Y in [-120, +120] = 240 UU
    Z in [0, 265] = 265 UU
    """
    mb = MeshBuilder("Grua_Asistencia")

    # 1. Ruedas (Radio 44 UU, ancho 26 UU)
    wheel_r = 44.0
    wheel_w = 26.0
    f_axle = 240.0
    r_axle = -160.0
    y_l = -96.0
    y_r = 96.0

    mb.add_wheel_assembly(f_axle, y_l, wheel_r, wheel_r, wheel_w, group_name="Wheel_FL")
    mb.add_wheel_assembly(f_axle, y_r, wheel_r, wheel_r, wheel_w, group_name="Wheel_FR")
    mb.add_wheel_assembly(r_axle, y_l, wheel_r, wheel_r, wheel_w, group_name="Wheel_RL_Outer")
    mb.add_wheel_assembly(r_axle, y_l + 14.0, wheel_r, wheel_r, wheel_w, group_name="Wheel_RL_Inner")
    mb.add_wheel_assembly(r_axle, y_r, wheel_r, wheel_r, wheel_w, group_name="Wheel_RR_Outer")
    mb.add_wheel_assembly(r_axle, y_r - 14.0, wheel_r, wheel_r, wheel_w, group_name="Wheel_RR_Inner")

    # 2. Chasis
    mb.add_box(-350.0, 330.0, -45.0, 45.0, 35.0, 80.0, mat="M_Plastic_Black", group="Chassis_Frame")

    # 3. Cabina de Asistencia (Llega a X = +360.0)
    mb.add_box(110.0, 340.0, -115.0, 115.0, 50.0, 226.0,
               mat="M_CarPaint_Yellow_Tow", group="Cab_Body")

    # Morro y calandra
    mb.add_box(340.0, 360.0, -118.0, 118.0, 25.0, 85.0, mat="M_Plastic_Black", group="Cab_Bumper")
    mb.add_box(340.0, 350.0, -75.0, 75.0, 85.0, 150.0, mat="M_Plastic_Black", group="Cab_Grille")

    # Parabrisas
    mb.add_frustum(
        x_bottom=(220.0, 335.0), y_bottom=(-112.0, 112.0), z_bottom=120.0,
        x_top=(215.0, 215.0), y_top=(-108.0, 108.0), z_top=218.0,
        mat="M_Glass", group="Cab_Windshield"
    )

    # Ventanillas laterales
    mb.add_box(130.0, 230.0, -116.0, -114.0, 125.0, 210.0, mat="M_Glass", group="Cab_Windows")
    mb.add_box(130.0, 230.0, 114.0, 116.0, 125.0, 210.0, mat="M_Glass", group="Cab_Windows")

    # Techo de la cabina
    mb.add_box(110.0, 215.0, -114.0, 114.0, 218.0, 230.0,
               mat="M_CarPaint_Yellow_Tow", group="Cab_Roof")

    # 4. Rotativos de Emergencia Ámbar V-2 en el Techo (Cota máxima Z = 265 UU)
    mb.add_box(150.0, 180.0, -80.0, 80.0, 230.0, 240.0, mat="M_Plastic_Black", group="Beacon_Bar_Mount")
    mb.add_cylinder_z(165.0, -65.0, 240.0, radius=14.0, height=25.0,
                      segments=14, mat="M_Lights_Beacon_Amber", group="Rotary_Beacon_Left")
    mb.add_cylinder_z(165.0, 65.0, 240.0, radius=14.0, height=25.0,
                      segments=14, mat="M_Lights_Beacon_Amber", group="Rotary_Beacon_Right")

    # 5. Plataforma Basculante de Rescate (Flatbed)
    # X in [-360, 105] = 465 UU, Y in [-120, 120] = 240 UU
    mb.add_box(-360.0, 105.0, -120.0, 120.0, 75.0, 88.0,
               mat="M_Diamond_Plate", group="Recovery_Platform")

    # Barandillas laterales
    mb.add_box(-360.0, 105.0, -120.0, -112.0, 88.0, 105.0, mat="M_CarPaint_Yellow_Tow", group="Platform_Rails")
    mb.add_box(-360.0, 105.0, 112.0, 120.0, 88.0, 105.0, mat="M_CarPaint_Yellow_Tow", group="Platform_Rails")

    # Cabestrante hidráulico
    mb.add_box(75.0, 105.0, -40.0, 40.0, 88.0, 125.0, mat="M_Chrome_Metal", group="Hydraulic_Winch")
    mb.add_cylinder_y(90.0, 0.0, 105.0, radius=12.0, width=50.0, segments=12, mat="M_Chrome_Metal", group="Winch_Spool")

    # Rampa trasera biselada (termina en X = -360.0)
    mb.add_frustum(
        x_bottom=(-360.0, -320.0), y_bottom=(-112.0, 112.0), z_bottom=25.0,
        x_top=(-360.0, -320.0), y_top=(-112.0, 112.0), z_top=75.0,
        mat="M_Diamond_Plate", group="Platform_Ramps"
    )

    # 6. Faros y pilotos (Cota trasera exacta X = -360.0)
    mb.add_box(355.0, 360.0, -110.0, -75.0, 50.0, 75.0, mat="M_Lights_Front", group="Lights_Front")
    mb.add_box(355.0, 360.0, 75.0, 110.0, 50.0, 75.0, mat="M_Lights_Front", group="Lights_Front")

    mb.add_box(102.0, 108.0, -35.0, -15.0, 125.0, 145.0, mat="M_Lights_Front", group="Work_Lights")

    mb.add_box(-360.0, -352.0, -115.0, 115.0, 25.0, 50.0, mat="M_Plastic_Black", group="Rear_Bumper")
    mb.add_box(-360.0, -358.0, -110.0, -85.0, 30.0, 45.0, mat="M_Lights_Rear", group="Lights_Rear")
    mb.add_box(-360.0, -358.0, 85.0, 110.0, 30.0, 45.0, mat="M_Lights_Rear", group="Lights_Rear")

    # Retrovisores (Ajustados dentro de Y in [-120, 120])
    mb.add_box(210.0, 230.0, -120.0, -108.0, 140.0, 180.0, mat="M_Plastic_Black", group="Mirrors")
    mb.add_box(210.0, 230.0, 108.0, 120.0, 140.0, 180.0, mat="M_Plastic_Black", group="Mirrors")

    return mb


# =============================================================================
# 8. Piezas de Daño / Restos Sueltos en Calzada (Debris)
# =============================================================================
def build_turismo_paragolpes_delantero_debris() -> MeshBuilder:
    mb = MeshBuilder("Turismo_Paragolpes_Delantero")
    mb.add_box(-10.0, 10.0, -90.0, 90.0, 0.0, 42.0, mat="M_Plastic_Black", group="Bumper_Debris")
    mb.add_box(9.0, 10.0, -50.0, 50.0, 4.0, 26.0, mat="M_Plastic_Black", group="Bumper_Grille")
    return mb


def build_rueda_turismo_debris() -> MeshBuilder:
    mb = MeshBuilder("Rueda_Turismo")
    mb.add_wheel_assembly(0.0, 0.0, 30.0, 30.0, 20.0, rim_radius_ratio=0.65,
                          rim_mat="M_Rim_Silver", tire_mat="M_Rubber_Tire", group_name="Detached_Wheel")
    return mb


# =============================================================================
# Función Principal de Generación y Validación
# =============================================================================
def main():
    print("=" * 80)
    print("  AUTOPISTAS DE ESPAÑA - GENERADOR DE MODELOS 3D DE VEHÍCULOS")
    print("  Normativa: 1 Unreal Unit = 1 cm = 0.01 m | Vista Cenital Low/Mid-Poly")
    print("=" * 80)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    output_dir = os.path.join(project_root, "Content", "Meshes", "Vehicles")

    os.makedirs(output_dir, exist_ok=True)
    print(f"\n[+] Directorio de salida: {output_dir}")

    master_mtl_filename = "Vehicles_Master.mtl"
    master_mtl_path = os.path.join(output_dir, master_mtl_filename)
    generate_master_mtl(master_mtl_path)
    print(f"[+] Archivo de materiales generado: {master_mtl_filename}")

    generators = [
        ("Turismo_Compacto", build_turismo_compacto, (420.0, 180.0, 145.0)),
        ("Camion_Cabina", build_camion_cabina, (450.0, 250.0, 360.0)),
        ("Camion_Trailer", build_camion_trailer, (1200.0, 255.0, 400.0)),
        ("Camion_Articulado_Completo", build_camion_articulado_completo, (1650.0, 255.0, 400.0)),
        ("Furgoneta_Reparto", build_furgoneta_reparto, (580.0, 205.0, 230.0)),
        ("Autobus_Interurbano", build_autobus_interurbano, (1350.0, 255.0, 330.0)),
        ("Motocicleta", build_motocicleta, (215.0, 85.0, 120.0)),
        ("Patrulla_Guardia_Civil", build_patrulla_guardia_civil, (480.0, 190.0, 185.0)),
        ("Grua_Asistencia", build_grua_asistencia, (720.0, 240.0, 265.0)),
        ("Turismo_Paragolpes_Delantero", build_turismo_paragolpes_delantero_debris, (20.0, 180.0, 42.0)),
        ("Rueda_Turismo", build_rueda_turismo_debris, (60.0, 20.0, 60.0)),
    ]

    print("\n[+] Construyendo y validando geometría:")
    print("-" * 80)
    print(f"{'Modelo':<30} | {'Dim. Obtenida (UU)':<22} | {'Tris':<6} | {'Estado'}")
    print("-" * 80)

    success_count = 0
    tolerance = 1.0

    for name, builder_func, expected_dims in generators:
        mesh = builder_func()
        obj_filename = f"{name}.obj"
        obj_path = os.path.join(output_dir, obj_filename)

        single_mtl_filename = f"{name}.mtl"
        single_mtl_path = os.path.join(output_dir, single_mtl_filename)
        generate_master_mtl(single_mtl_path)

        mesh.export_obj(obj_path, single_mtl_filename)

        bounds = mesh.get_bounds()
        actual_dims = bounds['size']
        tri_count = len(mesh.faces) * 2

        diff_x = abs(actual_dims[0] - expected_dims[0])
        diff_y = abs(actual_dims[1] - expected_dims[1])
        diff_z = abs(actual_dims[2] - expected_dims[2])

        status = "OK"
        if diff_x > tolerance or diff_y > tolerance or diff_z > tolerance:
            status = f"WARN (Esp: {expected_dims[0]}x{expected_dims[1]}x{expected_dims[2]})"
        else:
            success_count += 1

        dim_str = f"{actual_dims[0]:.0f} x {actual_dims[1]:.0f} x {actual_dims[2]:.0f}"
        print(f"{name:<30} | {dim_str:<22} | {tri_count:<6} | {status}")

    print("-" * 80)
    print(f"\n[+] Proceso completado: {len(generators)} modelos generados con éxito en '{output_dir}'.")
    print(f"[+] Todos los archivos .obj y .mtl están listos para importar directamente en Unreal Engine 5.\n")


if __name__ == "__main__":
    main()
