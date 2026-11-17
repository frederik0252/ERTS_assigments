# Zybo hardware acceleration notes

Obsidian notes for the original Zybo XC7Z010, maintained on the `project_notes`
branch of `ERTS_assigments`.

Open this checkout folder as an Obsidian vault. Start with [[Zybo HLS]].
The notes use standard Markdown, task lists and Mermaid; no plugins are needed.

## Scope

Board constraints, toolchains, inference deployment, hardware/software
interfaces, HLS design, numerical verification and board measurements belong
here. Deep learning project content, datasets and training experiments are
outside this repository's notes scope.

## Current work

- [ ] Check RF-DETR Nano deployment feasibility on the Zybo ARM processor.
- [ ] Establish actual inference memory use and latency.
- [ ] Select an HLS target from measured bottlenecks.
- [ ] Compare complete ARM-only and ARM+FPGA execution.

The candidate deployment and acceleration are not yet validated. See
[[Zybo HLS#RF-DETR Nano feasibility|the feasibility checks]] before fixing a design.
