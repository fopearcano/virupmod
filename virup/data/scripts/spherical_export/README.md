# VIRUP Spherical Image Export

This module provides tools for exporting high-resolution spherical/panoramic images from the VIRUP space visualization software without using VR hardware.

## Features

- **Multiple projection types**: PANORAMA360, VR180, DOMEMASTER180
- **Predefined astronomical scenes**: Earth orbit, Solar System, Milky Way, cosmic web, etc.
- **Batch export**: Export multiple scenes automatically
- **Cubemap export**: Generate 6 faces for external stitching
- **High resolution support**: Up to 8K (8192x4096) or higher
- **CLI tool**: Command-line interface for easy scripting

## Quick Start

### Method 1: Using VIRUP's Python Scripting

1. Copy the `spherical_export` folder to your VIRUP data directory:
   ```bash
   cp -r virup/data/scripts/spherical_export ~/.config/virup/scripts/
   ```

2. Edit your VIRUP config to use the spherical export script:
   ```ini
   [scripting]
   rootdir=spherical_export

   [window]
   projection=panorama360
   forcewidth=8192
   forceheight=4096
   ```

3. Launch VIRUP and use keyboard controls:
   - **F5**: Export current view
   - **F6**: Export all predefined scenes
   - **F7**: Export cubemap (6 faces)
   - **1-9**: Jump to scene 1-9
   - **+/-**: Cycle projection type
   - **PgUp/PgDown**: Previous/Next scene

### Method 2: Using the CLI Tool

```bash
# List available presets
python virup_spherical_cli.py --list

# Export 8K panorama
python virup_spherical_cli.py --projection panorama360 --resolution 8k --output ./output

# Export VR180 stereo
python virup_spherical_cli.py --projection vr180 --resolution vr180_6k

# Batch export all scenes
python virup_spherical_cli.py --batch --projection panorama360 --output ./batch_output

# Generate a shell script for later use
python virup_spherical_cli.py --generate-script --output ./exports
```

## Projection Types

| Projection | Description | Typical Resolution |
|------------|-------------|-------------------|
| `panorama360` | Full 360° equirectangular panorama | 8192x4096 (8K) |
| `vr180` | Stereoscopic 180° VR (left+right eye) | 6000x3000 |
| `vr180l` | VR180 left eye only | 3000x3000 |
| `vr180r` | VR180 right eye only | 3000x3000 |
| `domemaster180` | 180° fisheye for planetarium domes | 4096x4096 |
| `default` | Standard perspective view | 3840x2160 |

## Predefined Scenes

| Scene | Description |
|-------|-------------|
| `earth_orbit` | Earth from low orbit with stars |
| `solar_system` | Solar System with orbital paths |
| `milky_way` | Milky Way galaxy structure |
| `local_group` | Local Group of galaxies |
| `illustris_tng` | IllustrisTNG cosmic web simulation |
| `sdss_survey` | SDSS galaxy distribution |
| `cmb` | Cosmic Microwave Background |
| `moon` | Moon surface view |
| `mars_phobos` | View from Phobos (Mars moon) |

## Configuration

### Resolution Presets

Edit `config_spherical.ini` to change resolution:

```ini
[window]
# 8K Panorama (recommended for 360°)
forcewidth=8192
forceheight=4096

# 4K Panorama
# forcewidth=4096
# forceheight=2048

# VR180 standard
# forcewidth=6000
# forceheight=3000
```

### Quality Settings

For highest quality exports:

```ini
[graphics]
antialiasing=4
atmoquality=5
texmaxsize=13

[quality]
gentexload=6
planetquality=7
texmaxsize=32
```

### Output Directory

```ini
[window]
viddir=/path/to/output/directory
```

## Custom Scenes

You can define custom scenes in `main.py`:

```python
from main import SphericalScene, SPHERICAL_SCENES

# Add a custom scene
custom_scene = SphericalScene(
    name="custom_view",
    description="My custom view",
    cosmo_position=Vector3(-0.43, -8.24, -0.81),
    scale=6.171e+20,
    visibilities={
        "Volumetric AGORA": 1.0,
        "Gaia": 1.0,
    },
    exposure=0.3,
    yaw=0.5,       # Camera horizontal rotation (radians)
    pitch=0.1,     # Camera vertical rotation (radians)
)

SPHERICAL_SCENES.append(custom_scene)
```

## Cubemap Export

For manual stitching or when you need more control:

1. Press **F7** to export 6 cubemap faces
2. Use external tools like [Hugin](http://hugin.sourceforge.net/) or [PTGui](https://www.ptgui.com/) to stitch them into an equirectangular image

The 6 faces are:
- `front` (+Z): Forward direction
- `back` (-Z): Backward direction
- `left` (-X): Left direction
- `right` (+X): Right direction
- `top` (+Y): Up direction
- `bottom` (-Y): Down direction

## Output Formats

- **PNG**: Lossless, best for archiving
- **JPG**: Lossy, smaller file size
- **EXR**: HDR format (requires HDR viewer)

## Hardware Requirements

For 8K export:
- GPU with at least 8GB VRAM
- 16GB+ RAM recommended
- SSD for faster file writing

## Troubleshooting

### Black or corrupted images
- Increase `wait_frames` in the config to give the scene more time to load
- Check that your VIRUP data files are properly configured

### Out of memory
- Reduce resolution (try 4K instead of 8K)
- Lower texture quality settings
- Close other GPU-intensive applications

### Slow export
- Disable anti-aliasing for faster previews
- Use lower quality settings during testing
- Consider using video mode for frame sequences

## API Reference

### SphericalExporter

Main export class with methods:
- `export_current_view()`: Export current view immediately
- `schedule_export()`: Schedule export after scene settles
- `start_batch_export()`: Export all predefined scenes
- `apply_projection(projection)`: Change projection type
- `apply_scene(scene)`: Apply a scene configuration

### Utility Functions

```python
# Export a specific scene
export_scene("milky_way", projection="panorama360", output_path="./milky_way.png")

# Set projection
set_projection("vr180")

# Get list of available scenes
scenes = get_scene_list()

# Create a custom scene
scene = create_custom_scene(
    name="custom",
    cosmo_position=Vector3(0, 0, 0),
    scale=1e20,
    visibilities={"SDSS": 1.0}
)
```

## License

This software is part of VIRUP and is licensed under the GNU General Public License v3.0.
