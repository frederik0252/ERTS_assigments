#!/usr/bin/env python3
"""Build the Exercise 7 bare-metal application with Vitis 2026.1.

An existing generated workspace is updated in place by default so this script
never silently removes user changes.  Pass ``--rebuild`` to move the existing
workspace to a timestamped backup and generate a clean one.
"""

import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import os
import re
import shlex
import shutil

import vitis


SCRIPT_DIR = Path(__file__).resolve().parent
ASSIGNMENT_DIR = SCRIPT_DIR.parents[1]
HARDWARE_DIR = ASSIGNMENT_DIR / "hardware"
HLS_CONFIG_DIR = ASSIGNMENT_DIR / "exercise7_neural_net" / "hls"
HLS_CONFIG = HLS_CONFIG_DIR / "neuralNet_config.h"
XSA = HARDWARE_DIR / "Ex7_2026_1.xsa"
WORKSPACE = HARDWARE_DIR / "Ex7_vitis"
PLATFORM_NAME = "ex7_platform"
DOMAIN_NAME = "standalone_ps7_cortexa9_0"
APP_NAME = "neuralNetTest"
APP_OPTIMIZATION = "-O2"


def parse_arguments():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--rebuild",
        action="store_true",
        help=(
            "move an existing generated workspace to a timestamped backup "
            "before creating a clean workspace"
        ),
    )
    return parser.parse_args()


def checked_build(component, label):
    status = component.build()
    if status != 0:
        raise RuntimeError(f"{label} build failed with status {status}")


def backup_workspace():
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    backup = WORKSPACE.with_name(f"{WORKSPACE.name}.backup-{timestamp}")
    suffix = 1
    while backup.exists():
        backup = WORKSPACE.with_name(
            f"{WORKSPACE.name}.backup-{timestamp}-{suffix}"
        )
        suffix += 1
    shutil.move(str(WORKSPACE), str(backup))
    print(f"WORKSPACE_BACKUP={backup}")


def existing_component(client, name):
    try:
        return client.get_component(name=name)
    except Exception as error:
        raise RuntimeError(
            f"Existing workspace {WORKSPACE} does not contain the expected "
            f"component {name!r}. Inspect it or rerun with --rebuild; it has "
            "not been deleted."
        ) from error


def configure_application(app):
    app.set_app_config(
        key="USER_COMPILE_OPTIMIZATION_LEVEL", values=APP_OPTIMIZATION
    )
    configured = app.get_app_config(key="USER_COMPILE_OPTIMIZATION_LEVEL")
    if isinstance(configured, str):
        configured = [configured]
    # The native API may return either -O2 or a quoted '"-O2"' value.
    # Normalize its serialization, then independently inspect actual compile
    # commands after the build; do not weaken the effective-option check.
    configured_tokens = [token for value in configured for token in shlex.split(value)]
    if configured_tokens != [APP_OPTIMIZATION]:
        raise RuntimeError(
            "Vitis did not retain the requested application optimization: "
            f"expected {APP_OPTIMIZATION!r}, received {configured!r}"
        )
    print(f"APP_C_OPTIMIZATION={APP_OPTIMIZATION}")


def preserve_changed_sources(source_groups):
    """Keep copies of generated-workspace sources before refreshing them."""
    backup_dir = None
    destination_dir = WORKSPACE / APP_NAME / "src"
    for source_dir, filenames in source_groups:
        for filename in filenames:
            source = source_dir / filename
            destination = destination_dir / filename
            if (
                not destination.is_file()
                or destination.read_bytes() == source.read_bytes()
            ):
                continue
            if backup_dir is None:
                timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
                backup_root = WORKSPACE / APP_NAME / "source_backups"
                backup_dir = backup_root / timestamp
                suffix = 1
                while backup_dir.exists():
                    backup_dir = backup_root / f"{timestamp}-{suffix}"
                    suffix += 1
                backup_dir.mkdir(parents=True, exist_ok=False)
            shutil.copy2(destination, backup_dir / filename)
            print(f"SOURCE_BACKUP={backup_dir / filename}")


def import_application_sources(app, platform_source_dir):
    source_groups = (
        (SCRIPT_DIR, ("neuralNetTest.c",)),
        (HLS_CONFIG_DIR, (HLS_CONFIG.name,)),
        (ASSIGNMENT_DIR / "common", ("zynq_timer.h",)),
        (platform_source_dir, ("platform.c", "platform.h")),
    )
    preserve_changed_sources(source_groups)
    app.import_files(
        from_loc=str(SCRIPT_DIR),
        files=list(source_groups[0][1]),
        dest_dir_in_cmp="src",
    )
    app.import_files(
        from_loc=str(HLS_CONFIG_DIR),
        files=list(source_groups[1][1]),
        dest_dir_in_cmp="src",
    )
    app.import_files(
        from_loc=str(ASSIGNMENT_DIR / "common"),
        files=list(source_groups[2][1]),
        dest_dir_in_cmp="src",
    )
    app.import_files(
        from_loc=str(platform_source_dir),
        files=list(source_groups[3][1]),
        dest_dir_in_cmp="src",
    )


def record_compile_configuration(vitis_root):
    databases = (
        WORKSPACE / APP_NAME / "compile_commands.json",
        WORKSPACE / APP_NAME / "build" / "compile_commands.json",
    )
    database = next((path for path in databases if path.is_file()), None)
    if database is None:
        raise FileNotFoundError(
            "Application build produced no compile_commands.json; cannot "
            "verify the requested optimization flags"
        )

    entries = json.loads(database.read_text(encoding="utf-8"))
    commands = []
    for entry in entries:
        source = Path(entry.get("file", ""))
        if source.name != "neuralNetTest.c":
            continue
        command = entry.get("command")
        if command is None:
            command = shlex.join(entry.get("arguments", []))
        commands.append(command)

    if not commands:
        raise RuntimeError(
            f"No neuralNetTest.c compilation was recorded in {database}"
        )

    optimization_pattern = re.compile(r"^-O(?:[0-3]|s)$")
    for command in commands:
        optimization_flags = [
            token
            for token in shlex.split(command)
            if optimization_pattern.fullmatch(token)
        ]
        if optimization_flags != [APP_OPTIMIZATION]:
            raise RuntimeError(
                "Unexpected optimization flags for neuralNetTest.c: "
                f"{optimization_flags!r}; expected [{APP_OPTIMIZATION!r}]"
            )

    manifest = WORKSPACE / APP_NAME / "build" / "build_configuration.txt"
    manifest.write_text(
        "\n".join(
            [
                "ERTS Assignment 2 Exercise 7 application build",
                f"recorded_utc={datetime.now(timezone.utc).isoformat()}",
                f"vitis_root={vitis_root}",
                f"hardware_xsa={XSA}",
                f"shared_dimensions={HLS_CONFIG}",
                f"requested_optimization={APP_OPTIMIZATION}",
                f"compile_database={database}",
                *(f"compile_command={command}" for command in commands),
                "",
            ]
        ),
        encoding="utf-8",
    )
    print(f"BUILD_CONFIGURATION={manifest}")


def main():
    arguments = parse_arguments()
    if not XSA.is_file():
        raise FileNotFoundError(f"Hardware platform not found: {XSA}")
    if not HLS_CONFIG.is_file():
        raise FileNotFoundError(f"Shared HLS dimensions not found: {HLS_CONFIG}")

    vitis_environment = os.environ.get("XILINX_VITIS")
    if not vitis_environment:
        raise RuntimeError(
            "XILINX_VITIS is not set; source the Vitis 2026.1 settings first"
        )
    vitis_root = Path(vitis_environment).resolve()
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

    if arguments.rebuild and WORKSPACE.exists():
        backup_workspace()
    reuse_workspace = WORKSPACE.exists()

    client = vitis.create_client()
    try:
        client.set_workspace(str(WORKSPACE))
        if reuse_workspace:
            print(f"REUSING_WORKSPACE={WORKSPACE}")
            platform = existing_component(client, PLATFORM_NAME)
            platform.update_hw(hw_design=str(XSA))
        else:
            platform = client.create_platform_component(
                name=PLATFORM_NAME,
                hw_design=str(XSA),
                os="standalone",
                cpu="ps7_cortexa9_0",
                domain_name=DOMAIN_NAME,
            )
        checked_build(platform, "platform")

        platform_xpfm = client.find_platform_in_repos(PLATFORM_NAME)
        if reuse_workspace:
            app = existing_component(client, APP_NAME)
        else:
            app = client.create_app_component(
                name=APP_NAME,
                platform=platform_xpfm,
                domain=DOMAIN_NAME,
                template="empty_application",
            )
        import_application_sources(app, platform_source_dir)
        configure_application(app)
        checked_build(app, "application")
    finally:
        vitis.dispose()

    elf = WORKSPACE / APP_NAME / "build" / f"{APP_NAME}.elf"
    if not elf.is_file():
        raise FileNotFoundError(f"Build completed without the expected ELF: {elf}")
    record_compile_configuration(vitis_root)
    print(f"NATIVE_ELF={elf}")


if __name__ == "__main__":
    main()
