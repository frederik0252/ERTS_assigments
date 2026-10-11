# Assignment 2 hand-in package

This is the 11 October 2026 post-review package, copied from the native working
machine without changing the verified PDFs or their LaTeX sources.

- [Hand-in journal](ERTS_Assignment_2_Handin_Journal.pdf): implementation,
  verification, physical counter confirmation, and corrected optimized timings.
- [Requirements review](ERTS_Assignment_2_Requirements_Review.pdf): current fix
  closure followed by the explicitly labelled historical pre-fix review.
- [Migration journal](ERTS_Vivado_2017_2_to_2026_1_Migration_Journal.pdf): the
  course's 2017.2 SDK/HDF flow versus the actual 2026.1 Vitis/XSA workflow.

The matching `.tex` files and shared style/closure files are included. Build from
this directory with a LaTeX installation that provides the packages used in the
source; the validated native build used Tectonic:

```sh
tectonic -X compile ERTS_Assignment_2_Handin_Journal.tex
tectonic -X compile ERTS_Assignment_2_Requirements_Review.tex
tectonic -X compile ERTS_Vivado_2017_2_to_2026_1_Migration_Journal.tex
```

## Evidence and provenance

`native_screenshots/` contains the five actual application captures referenced
by the journal. Their captions identify them as historical captures. The fresh
post-fix UART listing is typeset from the raw log, not presented as a screenshot.

`fix_evidence/` contains native tool logs, board UART/XSDB logs, timestamps,
artifact hashes, the user's physical counter observation, and host test logs.
Failed initial build attempts are retained alongside successful retry logs;
their presence does not mean the final native build failed. `pre_fix_hls/` and
`review_evidence/` are historical evidence, not fresh post-fix results.

Compact generated evidence is retained under:

- `fix_evidence/hls_reports/`: current baseline/pipelined synthesis and five-case
  VHDL co-simulation reports.
- `fix_evidence/native_build/Ex345/`: source/artifact manifest and ELF-derived
  on-chip heap/stack verification for all three native applications.
- `fix_evidence/native_build/Ex7/`: actual optimized compiler configuration.

The documents preserve the paths from the original working machine. References
to its sibling `outputs/fix_evidence`, `outputs/native_screenshots`, and
`outputs/review_evidence` correspond to the same-named directories here. The
compact build evidence above was copied from the generated workspaces named in
those documents. Absolute paths in raw metadata identify where the actual run
occurred; they are not portable build prerequisites.

Generated vendor workspaces, caches, bitstreams, ELF binaries, old document
backups, and the obsolete Word draft are deliberately not included. Rebuild
instructions and required course archives are documented in
[the hardware guide](../hardware/README.md).

## Remaining acceptance boundaries

The full 0..15 physical LED counter was confirmed by the user. Separate
switch-to-LED mapping observations remain pending. Course acceptance of
Vivado/Vitis 2026.1 instead of 2017.2 and of the documented adapted ANN reference
is not established; the untouched supplied ANN test still fails as documented.

The corrected `-O2` measurement is 44.2 us for ARM computation, 66.6 us for
accelerator computation, and 779.6 us for the full hardware transaction. It
supersedes the earlier 8.61x comparison against unoptimized software.
