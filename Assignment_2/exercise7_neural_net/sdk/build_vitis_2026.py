#!/usr/bin/env python3
"""Build the Exercise 7 bare-metal application with Vitis 2026.1."""

from pathlib import Path
import os
import shutil

import vitis


SCRIPT_DIR = Path(__file__).resolve().parent
ASSIGNMENT_DIR = SCRIPT_DIR.parents[1]
HARDWARE_DIR = ASSIGNMENT_DIR / "hardware"
XSA = HARDWARE_DIR / "Ex7_2026_1.xsa"
WORKSPACE = HARDWARE_DIR / "Ex7_vitis"
PLATFORM_NAME = "ex7_platform"
DOMAIN_NAME = "standalone_ps7_cortexa9_0"
APP_NAME = "neuralNetTest"


def checked_build(component, label):
    status = component.build()
    if status != 0:
        raise RuntimeError(f"{label} build failed with status {status}")


def main():
    if not XSA.is_file():
        raise FileNotFoundError(f"Hardware platform not found: {XSA}")

    vitis_root = Path(os.environ["XILINX_VITIS"]).resolve()
    platform_source_dir = (
        vitis_root.parent
        / "data"
        / "embeddedsw"
        / "lib"
        / "sw_apps"
        / "hello_world"
        / "src"
    )
    for filename in ("platform.c", "platform.h"):
        if not (platform_source_dir / filename).is_file():
            raise FileNotFoundError(platform_source_dir / filename)

    # This directory is generated exclusively by this script. Recreating it
    # avoids stale BSP metadata when the XSA changes.
    if WORKSPACE.exists():
        shutil.rmtree(WORKSPACE)

    client = vitis.create_client()
    try:
        client.set_workspace(str(WORKSPACE))
        platform = client.create_platform_component(
            name=PLATFORM_NAME,
            hw_design=str(XSA),
            os="standalone",
            cpu="ps7_cortexa9_0",
            domain_name=DOMAIN_NAME,
        )
        checked_build(platform, "platform")

        platform_xpfm = client.find_platform_in_repos(PLATFORM_NAME)
        app = client.create_app_component(
            name=APP_NAME,
            platform=platform_xpfm,
            domain=DOMAIN_NAME,
            template="empty_application",
        )
        app.import_files(
            from_loc=str(SCRIPT_DIR),
            files=["neuralNetTest.c"],
            dest_dir_in_cmp="src",
        )
        app.import_files(
            from_loc=str(ASSIGNMENT_DIR / "common"),
            files=["zynq_timer.h"],
            dest_dir_in_cmp="src",
        )
        app.import_files(
            from_loc=str(platform_source_dir),
            files=["platform.c", "platform.h"],
            dest_dir_in_cmp="src",
        )
        checked_build(app, "application")
    finally:
        vitis.dispose()

    elf = WORKSPACE / APP_NAME / "build" / f"{APP_NAME}.elf"
    if not elf.is_file():
        raise FileNotFoundError(f"Build completed without the expected ELF: {elf}")
    print(f"NATIVE_ELF={elf}")


if __name__ == "__main__":
    main()
