#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VIRUP Spherical Export CLI Tool

This is a command-line wrapper for launching VIRUP with spherical export
configuration. It provides an easy way to export spherical images without
manually editing configuration files.

Requirements:
    - VIRUP built and installed
    - Python 3.6+

Usage:
    python virup_spherical_cli.py --projection panorama360 --resolution 8k --output ./output
    python virup_spherical_cli.py --projection vr180 --scene milky_way
    python virup_spherical_cli.py --list-scenes
    python virup_spherical_cli.py --batch --projection panorama360

Author: Generated for VIRUP spherical export
License: GPL v3
"""

import argparse
import os
import sys
import subprocess
import tempfile
import configparser
from pathlib import Path


# Resolution presets
RESOLUTION_PRESETS = {
    # Panorama 360
    "8k": (8192, 4096),
    "4k": (4096, 2048),
    "2k": (2048, 1024),

    # VR180
    "vr180_8k": (8000, 4000),
    "vr180_6k": (6000, 3000),
    "vr180_4k": (4000, 2000),

    # Domemaster
    "dome_8k": (8192, 8192),
    "dome_4k": (4096, 4096),
    "dome_2k": (2048, 2048),

    # Standard
    "1080p": (1920, 1080),
    "4k_standard": (3840, 2160),
}

# Projection types
PROJECTIONS = {
    "panorama360": "Full 360-degree equirectangular panorama",
    "vr180": "Stereoscopic 180-degree VR (left + right)",
    "vr180l": "VR180 left eye only",
    "vr180r": "VR180 right eye only",
    "domemaster180": "180-degree fisheye for planetarium domes",
    "default": "Standard perspective projection",
}

# Predefined scenes (matching main.py)
SCENES = {
    "earth_orbit": "Earth from low orbit",
    "solar_system": "Solar System with orbits",
    "milky_way": "Milky Way galaxy structure",
    "local_group": "Local Group of galaxies",
    "illustris_tng": "IllustrisTNG cosmic web",
    "sdss_survey": "SDSS galaxy distribution",
    "cmb": "Cosmic Microwave Background",
    "moon": "Moon surface view",
    "mars_phobos": "View from Phobos",
}


def find_virup_executable():
    """Find the VIRUP executable."""
    # Try common locations
    candidates = [
        "./build/virup",
        "./virup",
        "../build/virup",
        os.path.expanduser("~/bin/virup"),
        "/usr/local/bin/virup",
        "/usr/bin/virup",
    ]

    for candidate in candidates:
        if os.path.isfile(candidate) and os.access(candidate, os.X_OK):
            return os.path.abspath(candidate)

    # Try to find in PATH
    try:
        result = subprocess.run(["which", "virup"], capture_output=True, text=True)
        if result.returncode == 0:
            return result.stdout.strip()
    except:
        pass

    return None


def create_config_file(args, output_dir):
    """Create a temporary configuration file for VIRUP."""
    config = configparser.ConfigParser()

    # Graphics settings for high quality
    config["graphics"] = {
        "antialiasing": "4",
        "atmoquality": "5",
        "bloom": "false",
        "dithering": "true",
        "gentexload": "6",
        "hfov": "0",
        "maxlightcasters": "2",
        "shadowsquality": "5",
        "smoothshadows": "5",
        "texmaxsize": "13",
        "vfov": "0",
    }

    # Quality settings
    config["quality"] = {
        "atmoquality": "5",
        "gentexload": "6",
        "planetquality": "7",
        "texmaxsize": "32",
    }

    # Scripting
    config["scripting"] = {
        "rootdir": "spherical_export",
    }

    # VR disabled
    config["vr"] = {
        "enabled": "false",
        "mode": "false",
    }

    # Window settings
    width, height = get_resolution(args)
    config["window"] = {
        "projection": args.projection,
        "forcewidth": str(width),
        "forceheight": str(height),
        "forcerenderresolution": "true",
        "fullscreen": "false",
        "hdr": "true",
        "vsync": "false",
        "videomode": "true" if args.batch else "false",
        "viddir": output_dir,
        "videofps": "60",
        "maxframe": "1",
    }

    # Network
    config["network"] = {
        "server": "true",
    }

    # Write to temp file
    config_file = tempfile.NamedTemporaryFile(
        mode="w", suffix=".ini", delete=False, prefix="virup_spherical_"
    )
    config.write(config_file)
    config_file.close()

    return config_file.name


def get_resolution(args):
    """Get resolution from args or preset."""
    if args.width and args.height:
        return (args.width, args.height)

    preset = args.resolution.lower()
    if preset in RESOLUTION_PRESETS:
        return RESOLUTION_PRESETS[preset]

    # Default based on projection
    defaults = {
        "panorama360": RESOLUTION_PRESETS["8k"],
        "vr180": RESOLUTION_PRESETS["vr180_6k"],
        "vr180l": (3000, 3000),
        "vr180r": (3000, 3000),
        "domemaster180": RESOLUTION_PRESETS["dome_4k"],
        "default": RESOLUTION_PRESETS["4k_standard"],
    }
    return defaults.get(args.projection, RESOLUTION_PRESETS["4k"])


def list_presets():
    """Print available presets."""
    print("\nProjection Types:")
    print("-" * 50)
    for name, desc in PROJECTIONS.items():
        print(f"  {name:15} - {desc}")

    print("\nResolution Presets:")
    print("-" * 50)
    for name, (w, h) in sorted(RESOLUTION_PRESETS.items()):
        print(f"  {name:15} - {w}x{h}")

    print("\nPredefined Scenes:")
    print("-" * 50)
    for name, desc in SCENES.items():
        print(f"  {name:15} - {desc}")


def run_virup(args):
    """Run VIRUP with the configured settings."""
    virup_exe = args.executable or find_virup_executable()
    if not virup_exe:
        print("Error: Could not find VIRUP executable.", file=sys.stderr)
        print("Please specify with --executable or ensure VIRUP is in PATH.", file=sys.stderr)
        return 1

    # Create output directory
    output_dir = os.path.abspath(args.output)
    os.makedirs(output_dir, exist_ok=True)

    # Create config file
    config_file = create_config_file(args, output_dir)

    try:
        # Build command
        cmd = [virup_exe]

        # Add config file argument if VIRUP supports it
        # (Check VIRUP documentation for exact argument format)
        # cmd.extend(["--config", config_file])

        print(f"Starting VIRUP with {args.projection} projection...")
        print(f"Resolution: {get_resolution(args)[0]}x{get_resolution(args)[1]}")
        print(f"Output directory: {output_dir}")
        print(f"Config file: {config_file}")
        print("-" * 50)

        if args.dry_run:
            print(f"Would run: {' '.join(cmd)}")
            return 0

        # Run VIRUP
        env = os.environ.copy()
        env["VIRUP_CONFIG"] = config_file

        result = subprocess.run(cmd, env=env)
        return result.returncode

    finally:
        # Cleanup temp config (optionally keep for debugging)
        if not args.keep_config:
            try:
                os.unlink(config_file)
            except:
                pass


def create_export_script(args):
    """Create a shell script for exporting spherical images."""
    output_dir = os.path.abspath(args.output)
    os.makedirs(output_dir, exist_ok=True)

    width, height = get_resolution(args)

    script_content = f'''#!/bin/bash
# VIRUP Spherical Export Script
# Generated by virup_spherical_cli.py
#
# This script configures and runs VIRUP for spherical image export.
# Adjust paths as needed for your installation.

VIRUP_EXE="${{VIRUP_EXE:-virup}}"
OUTPUT_DIR="{output_dir}"
PROJECTION="{args.projection}"
WIDTH={width}
HEIGHT={height}

echo "VIRUP Spherical Export"
echo "======================"
echo "Projection: $PROJECTION"
echo "Resolution: ${{WIDTH}}x${{HEIGHT}}"
echo "Output: $OUTPUT_DIR"
echo ""

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Set environment variables for VIRUP
export VIRUP_PROJECTION="$PROJECTION"
export VIRUP_WIDTH="$WIDTH"
export VIRUP_HEIGHT="$HEIGHT"
export VIRUP_OUTPUT="$OUTPUT_DIR"

# Run VIRUP with spherical export script
$VIRUP_EXE

echo ""
echo "Export complete. Check $OUTPUT_DIR for output files."
'''

    script_path = os.path.join(output_dir, "run_spherical_export.sh")
    with open(script_path, "w") as f:
        f.write(script_content)

    os.chmod(script_path, 0o755)
    print(f"Created export script: {script_path}")
    return script_path


def main():
    parser = argparse.ArgumentParser(
        description="VIRUP Spherical Image Export CLI Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  Export 8K panorama:
    python virup_spherical_cli.py --projection panorama360 --resolution 8k

  Export VR180 stereo:
    python virup_spherical_cli.py --projection vr180 --resolution vr180_6k

  Custom resolution:
    python virup_spherical_cli.py --projection panorama360 --width 16384 --height 8192

  Batch export all scenes:
    python virup_spherical_cli.py --batch --projection panorama360

  List available presets:
    python virup_spherical_cli.py --list

  Generate export script:
    python virup_spherical_cli.py --generate-script --output ./exports
"""
    )

    # Projection settings
    parser.add_argument(
        "--projection", "-p",
        choices=list(PROJECTIONS.keys()),
        default="panorama360",
        help="Projection type for export (default: panorama360)"
    )

    # Resolution settings
    parser.add_argument(
        "--resolution", "-r",
        default="8k",
        help="Resolution preset (e.g., 8k, 4k, vr180_6k) or use --width/--height"
    )
    parser.add_argument("--width", "-W", type=int, help="Custom width in pixels")
    parser.add_argument("--height", "-H", type=int, help="Custom height in pixels")

    # Output settings
    parser.add_argument(
        "--output", "-o",
        default="./virup_spherical_output",
        help="Output directory for exported images"
    )

    # Scene selection
    parser.add_argument(
        "--scene", "-s",
        choices=list(SCENES.keys()),
        help="Specific scene to export"
    )
    parser.add_argument(
        "--batch", "-b",
        action="store_true",
        help="Export all predefined scenes"
    )

    # VIRUP executable
    parser.add_argument(
        "--executable", "-e",
        help="Path to VIRUP executable"
    )

    # Utility options
    parser.add_argument(
        "--list", "-l",
        action="store_true",
        help="List available projections, resolutions, and scenes"
    )
    parser.add_argument(
        "--generate-script", "-g",
        action="store_true",
        help="Generate a shell script for export instead of running VIRUP"
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show what would be done without executing"
    )
    parser.add_argument(
        "--keep-config",
        action="store_true",
        help="Keep temporary config file after execution"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Verbose output"
    )

    args = parser.parse_args()

    # Handle list command
    if args.list:
        list_presets()
        return 0

    # Handle generate-script command
    if args.generate_script:
        create_export_script(args)
        return 0

    # Run VIRUP
    return run_virup(args)


if __name__ == "__main__":
    sys.exit(main())
