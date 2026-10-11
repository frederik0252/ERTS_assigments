#!/usr/bin/env python3
"""Build native Ex3/4/5 applications and prove heap/stack placement in PS OCM.

Run from a Vitis 2026.1 environment:
    vitis -s Assignment_2/hardware/build_ex345_vitis.py

The existing generated workspace is renamed before recreation. Canonical code
stays outside that workspace. This script does not program or reset the board.
"""

from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess

import vitis


HARDWARE_DIR = Path(__file__).resolve().parent
ASSIGNMENT_DIR = HARDWARE_DIR.parent
XSA = HARDWARE_DIR / "Ex345_2026_1.xsa"
WORKSPACE = HARDWARE_DIR / "Ex345_vitis"
LINKER_TEMPLATE = HARDWARE_DIR / "ex345_ocm.ld"
PLATFORM_NAME = "ex345_platform"
DOMAIN_NAME = "standalone_ps7_cortexa9_0"
APPS = (
    ("ex3_console", "exercise3_console"),
    ("ex4_matrix", "exercise4_matrix"),
    ("ex5_matrix", "exercise5_matrix_ip"),
)
COMMON_FILES = (
    "console.h", "matrix.h", "matrix.c", "matrix_uart.c",
    "zynq_peripherals.h", "zynq_timer.h",
)
OCM_BEGIN = 0x1000
OCM_END = 0x30000


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def checked_build(component, label):
    status = component.build()
    if status != 0:
        raise RuntimeError(f"{label} build failed with status {status}")


def preserve_workspace():
    if not WORKSPACE.exists():
        return None
    if WORKSPACE.is_symlink() or not WORKSPACE.is_dir():
        raise RuntimeError(f"Expected a real generated workspace directory: {WORKSPACE}")
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    backup = HARDWARE_DIR / f"Ex345_vitis.backup-{stamp}"
    WORKSPACE.rename(backup)
    print(f"PRESERVED_WORKSPACE={backup}")
    return str(backup)


def find_readelf(vitis_root):
    candidates = (
        vitis_root / "gnu/aarch32/lin/gcc-arm-none-eabi/bin/arm-none-eabi-readelf",
        vitis_root.parent / "gnu/aarch32/lin/gcc-arm-none-eabi/x86_64-oesdk-linux/usr/bin/arm-xilinx-eabi/arm-xilinx-eabi-readelf",
    )
    for candidate in candidates:
        if candidate.is_file():
            return str(candidate)
    for name in ("arm-none-eabi-readelf", "arm-xilinx-eabi-readelf", "readelf"):
        candidate = shutil.which(name)
        if candidate:
            return candidate
    raise FileNotFoundError("No ELF inspection tool found in the native toolchain")


def inspect_command(readelf, option, elf, output):
    result = subprocess.run(
        [readelf, "-W", option, str(elf)], check=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
    )
    output.write_text(result.stdout, encoding="utf-8")
    return result.stdout


def parse_sections(report):
    sections = {}
    pattern = re.compile(
        r"^\s*\[\s*\d+\]\s+(\S+)\s+(\S+)\s+"
        r"([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\b",
        re.MULTILINE,
    )
    for match in pattern.finditer(report):
        sections[match[1]] = {
            "type": match[2],
            "address": int(match[3], 16),
            "size": int(match[5], 16),
        }
    return sections


def verify_compile_options(app_dir):
    candidates = (
        app_dir / "build/compile_commands.json",
        app_dir / "compile_commands.json",
        app_dir / "src/compile_commands.json",
    )
    command_file = next((path for path in candidates if path.is_file()), None)
    if command_file is None:
        raise FileNotFoundError(f"Missing compiler-command evidence in {app_dir}")
    entries = json.loads(command_file.read_text(encoding="utf-8"))
    required = {"main.c", "matrix.c", "matrix_uart.c", "platform.c"}
    checked = []
    for entry in entries:
        filename = Path(entry["file"]).name
        if filename not in required:
            continue
        arguments = entry.get("arguments") or shlex.split(entry["command"])
        optimizations = [arg for arg in arguments if re.fullmatch(r"-O(?:[0-3sgz]|fast)", arg)]
        if not optimizations or optimizations[-1] != "-O2":
            raise RuntimeError(f"{filename} was not compiled with effective -O2: {arguments}")
        checked.append({"file": entry["file"], "arguments": arguments})
    if {Path(item["file"]).name for item in checked} != required:
        raise RuntimeError(f"Compiler database does not cover all expected application sources: {command_file}")
    return {"database": str(command_file), "verified_commands": checked}


def verify_application(app_name, readelf):
    app_dir = WORKSPACE / app_name
    build_dir = app_dir / "build"
    elf = build_dir / f"{app_name}.elf"
    if not elf.is_file():
        raise FileNotFoundError(f"Build completed without the expected ELF: {elf}")
    header = inspect_command(readelf, "-h", elf, build_dir / "elf_header.txt")
    if not re.search(r"Machine:\s+ARM\s*$", header, re.MULTILINE):
        raise RuntimeError(f"Application is not an ARM ELF: {elf}")
    section_text = inspect_command(readelf, "-S", elf, build_dir / "elf_sections.txt")
    inspect_command(readelf, "-s", elf, build_dir / "elf_symbols.txt")
    sections = parse_sections(section_text)
    allocations = {}
    for name in (".heap", ".stack"):
        if name not in sections:
            raise RuntimeError(f"Required section {name} is missing from {elf}")
        section = sections[name]
        start = section["address"]
        end = start + section["size"]
        if section["size"] <= 0 or not OCM_BEGIN <= start < end <= OCM_END:
            raise RuntimeError(f"{elf}: {name} is outside reserved-safe low OCM: {section}")
        allocations[name] = {**section, "end_exclusive": end}
    if allocations[".heap"]["end_exclusive"] > allocations[".stack"]["address"]:
        raise RuntimeError(f"Heap and stack overlap in {elf}")
    if sections.get(".drvcfg_sec", {}).get("size", 0) <= 0:
        raise RuntimeError(f"SDT driver configuration section was lost from {elf}")
    linker_copy = app_dir / "src" / LINKER_TEMPLATE.name
    if digest(linker_copy) != digest(LINKER_TEMPLATE):
        raise RuntimeError(f"Generated application linker script differs from canonical template: {linker_copy}")
    evidence = {
        "application": app_name,
        "elf": str(elf),
        "elf_sha256": digest(elf),
        "linker_template_sha256": digest(LINKER_TEMPLATE),
        "ocm_allocations": allocations,
        "sdt_driver_section": sections[".drvcfg_sec"],
        "compilation": verify_compile_options(app_dir),
    }
    (build_dir / "ocm_verification.json").write_text(
        json.dumps(evidence, indent=2) + "\n", encoding="utf-8",
    )
    print(f"NATIVE_ELF={elf}")
    for name, allocation in allocations.items():
        print(
            f"OCM_VERIFIED={app_name} {name} "
            f"0x{allocation['address']:08x}..0x{allocation['end_exclusive'] - 1:08x}"
        )
    return evidence


def main():
    vitis_root = Path(os.environ["XILINX_VITIS"]).resolve()
    platform_sources = vitis_root.parent / "data/embeddedsw/lib/sw_apps/hello_world/src"
    inputs = [XSA, LINKER_TEMPLATE, Path(__file__).resolve()]
    inputs += [ASSIGNMENT_DIR / "common" / name for name in COMMON_FILES]
    inputs += [ASSIGNMENT_DIR / source_dir / "src/main.c" for _, source_dir in APPS]
    inputs += [platform_sources / name for name in ("platform.c", "platform.h")]
    for path in inputs:
        if not path.is_file():
            raise FileNotFoundError(f"Required native-build input is absent: {path}")
    input_hashes = {str(path): digest(path) for path in inputs}
    readelf = find_readelf(vitis_root)
    backup = preserve_workspace()
    client = vitis.create_client()
    evidence = []
    try:
        client.set_workspace(str(WORKSPACE))
        platform = client.create_platform_component(
            name=PLATFORM_NAME, hw_design=str(XSA), os="standalone",
            cpu="ps7_cortexa9_0", domain_name=DOMAIN_NAME,
        )
        checked_build(platform, PLATFORM_NAME)
        platform_xpfm = client.find_platform_in_repos(PLATFORM_NAME)
        for app_name, source_dir in APPS:
            app = client.create_app_component(
                name=app_name, platform=platform_xpfm, domain=DOMAIN_NAME,
                template="empty_application",
            )
            app.import_files(
                from_loc=str(ASSIGNMENT_DIR / source_dir / "src"),
                files=["main.c"], dest_dir_in_cmp="src",
            )
            app.import_files(
                from_loc=str(ASSIGNMENT_DIR / "common"),
                files=list(COMMON_FILES), dest_dir_in_cmp="src",
            )
            app.import_files(
                from_loc=str(platform_sources), files=["platform.c", "platform.h"],
                dest_dir_in_cmp="src",
            )
            app.import_files(
                from_loc=str(HARDWARE_DIR), files=[LINKER_TEMPLATE.name],
                dest_dir_in_cmp="src",
            )
            app.set_app_config(key="USER_COMPILE_OPTIMIZATION_LEVEL", values="-O2")
            # A distinct filename avoids the generated default lscript.ld path.
            app.set_app_config(
                key="USER_LINKER_SCRIPT",
                values=str(WORKSPACE / app_name / "src" / LINKER_TEMPLATE.name),
            )
            app.set_app_config(
                key="USER_LINK_OTHER_FLAGS",
                values=f"-Wl,-Map={WORKSPACE / app_name / 'build' / (app_name + '.map')}",
            )
            checked_build(app, app_name)
            evidence.append(verify_application(app_name, readelf))
    finally:
        vitis.dispose()

    if any(digest(path) != input_hashes[str(path)] for path in inputs):
        raise RuntimeError("A canonical input changed during the build; rebuild before claiming reproducibility")
    manifest = {
        "completed_utc": datetime.now(timezone.utc).isoformat(),
        "vitis_root": str(vitis_root),
        "xsa": str(XSA),
        "workspace_backup": backup,
        "input_sha256": input_hashes,
        "applications": evidence,
    }
    manifest_path = WORKSPACE / "native_build_evidence.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"NATIVE_BUILD_EVIDENCE={manifest_path}")


if __name__ == "__main__":
    main()
