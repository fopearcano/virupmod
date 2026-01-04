#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Spherical Image Export Script for VIRUP

This script exports high-resolution spherical/panoramic images from the VIRUP
space visualization software. It supports multiple projection types including:
- PANORAMA360: Full 360-degree equirectangular panorama
- VR180: Stereoscopic 180-degree VR format (left + right eye)
- VR180L/VR180R: Single eye VR180
- DOMEMASTER180: Fisheye projection for planetarium domes

Usage:
    1. Place this script in virup/data/scripts/spherical_export/
    2. Configure settings in config_spherical.ini (or use defaults)
    3. Run VIRUP with scripting/rootdir=spherical_export
    4. Use keyboard controls to export scenes

Keyboard Controls (when running):
    F5  - Export current view as spherical image
    F6  - Export all predefined scenes
    1-9 - Jump to scene 1-9
    +/- - Cycle through projections

Author: Generated for VIRUP spherical export
License: GPL v3
"""

from PythonQt.QtCore import Qt, QDateTime, QDate, QTime, QSettings
from PythonQt.libplanet import Vector3
from math import cos, sin, pi, atan2, asin
import os

# ============================================================================
# CONFIGURATION
# ============================================================================

class SphericalExportConfig:
    """Configuration for spherical image export."""

    def __init__(self):
        # Output settings
        self.output_dir = QSettings().value("window/viddir", "/tmp/virup_spherical")
        self.file_prefix = "virup_spherical"
        self.file_format = "png"  # png, jpg, exr (HDR)

        # Resolution presets for different projections
        self.resolutions = {
            "panorama360": (8192, 4096),  # 8K equirectangular
            "vr180": (6000, 3000),        # Standard VR180 (combined L+R)
            "vr180l": (3000, 3000),       # Single eye
            "vr180r": (3000, 3000),       # Single eye
            "domemaster180": (4096, 4096), # Square dome master
            "default": (3840, 2160),       # 4K standard
        }

        # Default projection type
        self.default_projection = "panorama360"

        # Camera settings
        self.camera_yaw_offset = 0.0      # Horizontal rotation offset (radians)
        self.camera_pitch_offset = 0.0    # Vertical rotation offset (radians)

        # Tone mapping defaults
        self.default_exposure = 0.3
        self.default_dynamic_range = 10000.0

        # Export quality settings
        self.force_max_quality = True     # Force maximum LOD quality
        self.wait_frames = 5              # Frames to wait for scene to settle


# ============================================================================
# SCENE DEFINITIONS
# ============================================================================

class SphericalScene:
    """Defines a scene for spherical export."""

    def __init__(self, name, cosmo_position=None, scale=None,
                 planet_system=None, planet_target=None, planet_position=None,
                 visibilities=None, exposure=0.3, yaw=0.0, pitch=0.0,
                 simulation_time=None, description=""):
        self.name = name
        self.description = description
        self.cosmo_position = cosmo_position or Vector3(0, 0, 0)
        self.scale = scale or 1.0
        self.planet_system = planet_system  # e.g., "Solar System"
        self.planet_target = planet_target  # e.g., "Earth"
        self.planet_position = planet_position or Vector3(0, 0, 0)
        self.visibilities = visibilities or {}
        self.exposure = exposure
        self.yaw = yaw
        self.pitch = pitch
        self.simulation_time = simulation_time


# Predefined scenes for spherical export
SPHERICAL_SCENES = [
    # Earth from space
    SphericalScene(
        name="earth_orbit",
        description="Earth from low orbit",
        planet_system="Solar System",
        planet_target="Earth",
        scale=15000000,
        visibilities={"Gaia": 1.0, "Hipparcos": 1.0},
        exposure=0.3,
    ),

    # Solar System overview
    SphericalScene(
        name="solar_system",
        description="Solar System with orbits",
        planet_system="Solar System",
        planet_target="Sun",
        scale=5.65e+12,
        visibilities={"Gaia": 1.0, "Hipparcos": 1.0, "Orbits": 1.0, "PlanetsLabels": 1.0},
        exposure=0.3,
    ),

    # Milky Way galaxy view
    SphericalScene(
        name="milky_way",
        description="Milky Way galaxy structure",
        cosmo_position=Vector3(-0.43, -8.24, -0.81),
        scale=6.171e+20,
        visibilities={"Volumetric AGORA": 1.0, "Andromeda": 2.0, "M33": 2.0},
        exposure=0.3,
    ),

    # Local Group
    SphericalScene(
        name="local_group",
        description="Local Group of galaxies",
        cosmo_position=Vector3(-0.43, -8.24, -0.81),
        scale=3.04e+22,
        visibilities={"Volumetric AGORA": 1.0, "Andromeda": 2.0, "M33": 2.0, "LG Dwarves": 5.0},
        exposure=0.3,
    ),

    # Illustris TNG cosmological simulation
    SphericalScene(
        name="illustris_tng",
        description="IllustrisTNG cosmic web",
        cosmo_position=Vector3(-0.43, -8.24, -0.81),
        scale=2.0e+24,
        visibilities={"IllustrisTNG": 1.0},
        exposure=0.3,
    ),

    # SDSS galaxy survey
    SphericalScene(
        name="sdss_survey",
        description="SDSS galaxy distribution",
        cosmo_position=Vector3(-0.43, -8.24, -0.81),
        scale=2.0e+26,
        visibilities={"SDSS": 1.0},
        exposure=0.3,
    ),

    # CMB (Cosmic Microwave Background)
    SphericalScene(
        name="cmb",
        description="Cosmic Microwave Background",
        cosmo_position=Vector3(-0.43, -8.24, -0.81),
        scale=4.0e+26,
        visibilities={"SDSS": 1.0, "CMB": 1.0},
        exposure=0.3,
    ),

    # Moon surface
    SphericalScene(
        name="moon",
        description="Moon surface view",
        planet_system="Solar System",
        planet_target="Moon",
        scale=4000000,
        visibilities={"Gaia": 1.0, "Hipparcos": 1.0},
        exposure=0.3,
    ),

    # Mars and Phobos
    SphericalScene(
        name="mars_phobos",
        description="View from Phobos",
        planet_system="Solar System",
        planet_target="Phobos",
        scale=30000,
        visibilities={"Gaia": 1.0, "Hipparcos": 1.0},
        exposure=0.3,
    ),
]


# ============================================================================
# SPHERICAL EXPORT ENGINE
# ============================================================================

class SphericalExporter:
    """Main class for exporting spherical images from VIRUP."""

    def __init__(self):
        self.config = SphericalExportConfig()
        self.current_scene_index = 0
        self.export_counter = 0
        self.wait_counter = 0
        self.pending_export = False
        self.batch_export_mode = False
        self.batch_scene_index = 0

        # Available projections for cycling
        self.projections = ["panorama360", "vr180", "vr180l", "vr180r", "domemaster180", "default"]
        self.current_projection_index = 0

    def get_current_projection(self):
        """Get the currently selected projection type."""
        return self.projections[self.current_projection_index]

    def cycle_projection(self, forward=True):
        """Cycle through available projection types."""
        if forward:
            self.current_projection_index = (self.current_projection_index + 1) % len(self.projections)
        else:
            self.current_projection_index = (self.current_projection_index - 1) % len(self.projections)

        projection = self.get_current_projection()
        self.apply_projection(projection)
        print(f"Projection changed to: {projection}")
        return projection

    def apply_projection(self, projection):
        """Apply projection settings to VIRUP."""
        # Set projection type
        VIRUP.projection = projection

        # Set resolution based on projection
        if projection in self.config.resolutions:
            width, height = self.config.resolutions[projection]
        else:
            width, height = self.config.resolutions["default"]

        # Update QSettings for resolution
        settings = QSettings()
        settings.setValue("window/forcewidth", width)
        settings.setValue("window/forceheight", height)
        settings.setValue("window/forcerenderresolution", True)

        print(f"Applied projection: {projection} at {width}x{height}")

    def apply_scene(self, scene):
        """Apply a SphericalScene to the VIRUP universe."""
        # Set cosmological position
        if scene.cosmo_position is not None:
            Universe.cosmoPosition = scene.cosmo_position

        # Set scale
        if scene.scale is not None:
            Universe.scale = scene.scale

        # Set planetary system target if specified
        if scene.planet_system and scene.planet_target:
            if Universe.planetarySystemLoaded:
                if Universe.planetarySystemName == scene.planet_system:
                    Universe.planetTarget = scene.planet_target
                    if scene.planet_position:
                        Universe.planetPosition = scene.planet_position

        # Set camera orientation
        Universe.camYaw = scene.yaw + self.config.camera_yaw_offset
        Universe.camPitch = scene.pitch + self.config.camera_pitch_offset

        # Set tone mapping
        ToneMappingModel.exposure = scene.exposure

        # Apply visibilities
        for name, visibility in scene.visibilities.items():
            Universe.setVisibility(name, visibility)

        # Set simulation time if specified
        if scene.simulation_time is not None and scene.simulation_time.isValid():
            Universe.simulationTime = scene.simulation_time

        print(f"Applied scene: {scene.name} - {scene.description}")

    def generate_filename(self, scene_name, projection):
        """Generate a unique filename for the export."""
        from datetime import datetime
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"{self.config.file_prefix}_{scene_name}_{projection}_{timestamp}.{self.config.file_format}"
        return os.path.join(self.config.output_dir, filename)

    def export_current_view(self):
        """Export the current view as a spherical image."""
        projection = self.get_current_projection()

        # Ensure output directory exists
        try:
            os.makedirs(self.config.output_dir, exist_ok=True)
        except Exception as e:
            print(f"Warning: Could not create output directory: {e}")

        # Generate filename
        if self.current_scene_index < len(SPHERICAL_SCENES):
            scene_name = SPHERICAL_SCENES[self.current_scene_index].name
        else:
            scene_name = f"custom_{self.export_counter}"

        filepath = self.generate_filename(scene_name, projection)

        # Take screenshot using VIRUP's built-in function
        VIRUP.takeScreenshot(filepath)

        self.export_counter += 1
        print(f"Exported: {filepath}")
        return filepath

    def schedule_export(self):
        """Schedule an export after waiting for the scene to settle."""
        self.wait_counter = self.config.wait_frames
        self.pending_export = True
        print(f"Export scheduled, waiting {self.config.wait_frames} frames...")

    def start_batch_export(self):
        """Start exporting all predefined scenes."""
        self.batch_export_mode = True
        self.batch_scene_index = 0
        print("Starting batch export of all scenes...")
        self._process_next_batch_scene()

    def _process_next_batch_scene(self):
        """Process the next scene in batch export."""
        if self.batch_scene_index < len(SPHERICAL_SCENES):
            scene = SPHERICAL_SCENES[self.batch_scene_index]
            self.current_scene_index = self.batch_scene_index
            self.apply_scene(scene)
            self.schedule_export()
        else:
            self.batch_export_mode = False
            print("Batch export complete!")

    def update(self):
        """Called every frame to process pending exports."""
        if self.pending_export:
            if self.wait_counter > 0:
                self.wait_counter -= 1
            else:
                self.pending_export = False
                self.export_current_view()

                # Continue batch export if active
                if self.batch_export_mode:
                    self.batch_scene_index += 1
                    self._process_next_batch_scene()

    def goto_scene(self, index):
        """Jump to a specific scene by index."""
        if 0 <= index < len(SPHERICAL_SCENES):
            self.current_scene_index = index
            self.apply_scene(SPHERICAL_SCENES[index])
            print(f"Jumped to scene {index + 1}: {SPHERICAL_SCENES[index].name}")
        else:
            print(f"Invalid scene index: {index + 1}")

    def next_scene(self):
        """Go to the next scene."""
        self.current_scene_index = (self.current_scene_index + 1) % len(SPHERICAL_SCENES)
        self.apply_scene(SPHERICAL_SCENES[self.current_scene_index])

    def prev_scene(self):
        """Go to the previous scene."""
        self.current_scene_index = (self.current_scene_index - 1) % len(SPHERICAL_SCENES)
        self.apply_scene(SPHERICAL_SCENES[self.current_scene_index])


# ============================================================================
# MULTI-ANGLE CUBEMAP EXPORT
# ============================================================================

class CubemapExporter:
    """
    Export 6 cubemap faces for manual assembly into equirectangular.

    This is useful when the built-in panorama360 projection is not available
    or when higher quality is needed through external stitching tools.
    """

    FACE_ROTATIONS = {
        "front":  (0.0, 0.0),           # +Z
        "back":   (pi, 0.0),            # -Z
        "left":   (-pi/2, 0.0),         # -X
        "right":  (pi/2, 0.0),          # +X
        "top":    (0.0, pi/2),          # +Y
        "bottom": (0.0, -pi/2),         # -Y
    }

    def __init__(self, exporter):
        self.exporter = exporter
        self.current_face = 0
        self.faces = list(self.FACE_ROTATIONS.keys())
        self.base_yaw = 0.0
        self.base_pitch = 0.0
        self.export_in_progress = False
        self.wait_counter = 0

    def start_cubemap_export(self):
        """Start exporting all 6 cubemap faces."""
        # Store original camera orientation
        self.base_yaw = Universe.camYaw
        self.base_pitch = Universe.camPitch

        # Set to default projection for cubemap faces
        self.exporter.apply_projection("default")

        # Set 90 degree FOV for proper cubemap (if possible through settings)
        settings = QSettings()
        settings.setValue("graphics/hfov", 90)
        settings.setValue("graphics/vfov", 90)

        self.current_face = 0
        self.export_in_progress = True
        print("Starting cubemap export...")
        self._set_face_rotation()

    def _set_face_rotation(self):
        """Set camera rotation for current face."""
        if self.current_face < len(self.faces):
            face_name = self.faces[self.current_face]
            yaw_offset, pitch_offset = self.FACE_ROTATIONS[face_name]
            Universe.camYaw = self.base_yaw + yaw_offset
            Universe.camPitch = self.base_pitch + pitch_offset
            self.wait_counter = self.exporter.config.wait_frames
            print(f"Set rotation for face: {face_name}")

    def update(self):
        """Called every frame to process cubemap export."""
        if not self.export_in_progress:
            return

        if self.wait_counter > 0:
            self.wait_counter -= 1
            return

        # Export current face
        face_name = self.faces[self.current_face]
        filepath = self._generate_cubemap_filename(face_name)
        VIRUP.takeScreenshot(filepath)
        print(f"Exported cubemap face: {face_name} -> {filepath}")

        # Move to next face
        self.current_face += 1
        if self.current_face < len(self.faces):
            self._set_face_rotation()
        else:
            # Restore original orientation
            Universe.camYaw = self.base_yaw
            Universe.camPitch = self.base_pitch
            self.export_in_progress = False
            print("Cubemap export complete!")
            print("Use external tools (e.g., Hugin, PTGui) to stitch into equirectangular.")

    def _generate_cubemap_filename(self, face_name):
        """Generate filename for a cubemap face."""
        from datetime import datetime
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"cubemap_{face_name}_{timestamp}.{self.exporter.config.file_format}"
        return os.path.join(self.exporter.config.output_dir, filename)


# ============================================================================
# GLOBAL STATE AND CALLBACKS
# ============================================================================

# Global exporter instance
exporter = None
cubemap_exporter = None


def initScene():
    """Called when the script is loaded."""
    global exporter, cubemap_exporter

    exporter = SphericalExporter()
    cubemap_exporter = CubemapExporter(exporter)

    # Apply initial settings
    exporter.apply_projection(exporter.config.default_projection)

    # Set initial tone mapping
    ToneMappingModel.exposure = exporter.config.default_exposure
    ToneMappingModel.dynamicrange = exporter.config.default_dynamic_range

    print("=" * 60)
    print("VIRUP Spherical Image Export Script Loaded")
    print("=" * 60)
    print("Controls:")
    print("  F5      - Export current view")
    print("  F6      - Export all predefined scenes (batch)")
    print("  F7      - Export cubemap (6 faces)")
    print("  1-9     - Jump to scene 1-9")
    print("  +/-     - Cycle projection type")
    print(f"\nCurrent projection: {exporter.get_current_projection()}")
    print(f"Output directory: {exporter.config.output_dir}")
    print("=" * 60)


def updateScene():
    """Called every frame."""
    global exporter, cubemap_exporter

    if exporter:
        exporter.update()

    if cubemap_exporter:
        cubemap_exporter.update()


def keyPressEvent(e):
    """Handle keyboard input."""
    global exporter, cubemap_exporter

    if not exporter:
        return

    # F5 - Export current view
    if e.key() == Qt.Key_F5:
        exporter.schedule_export()

    # F6 - Batch export all scenes
    elif e.key() == Qt.Key_F6:
        exporter.start_batch_export()

    # F7 - Cubemap export
    elif e.key() == Qt.Key_F7:
        cubemap_exporter.start_cubemap_export()

    # Number keys 1-9 - Jump to scene
    elif e.key() >= Qt.Key_1 and e.key() <= Qt.Key_9:
        scene_index = e.key() - Qt.Key_1
        exporter.goto_scene(scene_index)

    # Plus/Minus - Cycle projections
    elif e.key() == Qt.Key_Plus or e.key() == Qt.Key_Equal:
        exporter.cycle_projection(forward=True)
    elif e.key() == Qt.Key_Minus:
        exporter.cycle_projection(forward=False)

    # Page Up/Down - Next/Prev scene
    elif e.key() == Qt.Key_PageDown:
        exporter.next_scene()
    elif e.key() == Qt.Key_PageUp:
        exporter.prev_scene()


# ============================================================================
# UTILITY FUNCTIONS FOR EXTERNAL USE
# ============================================================================

def export_scene(scene_name, projection="panorama360", output_path=None):
    """
    Export a specific scene as a spherical image.

    Args:
        scene_name: Name of predefined scene or custom scene object
        projection: Projection type (panorama360, vr180, etc.)
        output_path: Optional custom output path

    Returns:
        Path to the exported file
    """
    global exporter

    if exporter is None:
        exporter = SphericalExporter()

    # Find scene by name if string
    if isinstance(scene_name, str):
        scene = None
        for s in SPHERICAL_SCENES:
            if s.name == scene_name:
                scene = s
                break
        if scene is None:
            raise ValueError(f"Unknown scene: {scene_name}")
    else:
        scene = scene_name

    # Apply projection
    exporter.apply_projection(projection)

    # Apply scene
    exporter.apply_scene(scene)

    # Wait a bit for scene to settle (in batch mode)
    # In interactive mode, use schedule_export instead

    if output_path:
        VIRUP.takeScreenshot(output_path)
        return output_path
    else:
        return exporter.export_current_view()


def set_projection(projection):
    """Set the current projection type."""
    global exporter
    if exporter:
        exporter.apply_projection(projection)


def get_scene_list():
    """Get list of available predefined scenes."""
    return [(i, s.name, s.description) for i, s in enumerate(SPHERICAL_SCENES)]


def create_custom_scene(name, **kwargs):
    """Create a custom scene for export."""
    return SphericalScene(name=name, **kwargs)
