#!/usr/bin/env bash
set -euo pipefail

assignment_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
plantuml_version="1.2026.10"
plantuml_jar="${ERTS_PLANTUML_JAR:-$assignment_dir/.tools/plantuml-$plantuml_version.jar}"

if [[ ! -f "$plantuml_jar" ]]; then
    mkdir -p -- "$(dirname -- "$plantuml_jar")"
    download_file="$plantuml_jar.download"
    trap 'rm -f -- "$download_file"' EXIT
    curl -fsSL --max-time 120 \
        "https://github.com/plantuml/plantuml/releases/download/v$plantuml_version/plantuml.jar" \
        -o "$download_file"
    mv -- "$download_file" "$plantuml_jar"
    trap - EXIT
fi

java_bin="${ERTS_JAVA:-}"
if [[ -z "$java_bin" ]]; then
    java_bin="$(command -v java || true)"
fi
if [[ -z "$java_bin" && -n "${JAVA_HOME:-}" && -x "$JAVA_HOME/bin/java" ]]; then
    java_bin="$JAVA_HOME/bin/java"
fi
if [[ -z "$java_bin" ]]; then
    printf '%s\n' 'Set ERTS_JAVA to a Java 17 or newer executable.' >&2
    exit 1
fi

"$java_bin" -Djava.awt.headless=true -jar "$plantuml_jar" -failfast2 -tsvg "$assignment_dir"/docs/uml/*.puml
"$java_bin" -Djava.awt.headless=true -jar "$plantuml_jar" -failfast2 -tpng "$assignment_dir"/docs/uml/*.puml
