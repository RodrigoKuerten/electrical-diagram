# FioVivo symbol library

This directory contains the CADe_SIMU 4.0 symbol library used by FioVivo.

- **215 JSON manifests**, one per component type.
- **351 SVG files**: a base SVG for every type plus state variants such as `_active`,
  `_tripped`, `_blown` and `_position_1..3`.
- **111 exact CADe symbols** are exposed in the library and Insert menu. Their geometry was
  recorded from the real GDI primitives of CADe_SIMU 4.0 and converted at the canvas scale
  `672 px = 297 mm` (`2.262626 px/mm`).
- The 20 original FioVivo identifiers (`supply_ac3`, `fuse`, `contactor_coil`,
  `pushbutton_no`, etc.) and 84 unverified types remain loadable for old documents, but are
  hidden under the internal `legacy` category. They keep the old pin names and artwork so
  existing examples and tests do not break.

See [`docs/cadesimu/report.md`](../../docs/cadesimu/report.md) for the GDI capture method,
evidence and limitations, and
[`docs/cadesimu/inventory.md`](../../docs/cadesimu/inventory.md) for the complete type list.

## File conventions

- Lowercase filenames. The base SVG is `<type>.svg`; state variants are
  `<type>_<state>.svg` (for example `contactor_3p_active.svg`).
- One scene unit equals one millimetre.
- Exact symbols use a viewBox in recorded GDI pixels and a physical `width`/`height` in
  millimetres; the manifest size and pin offsets are expressed in millimetres.
- The background is transparent.
- Symbol strokes and text use `#1f2933`; terminal labels use `#1f5f9e`; active parts use
  `#f2b134`; fault marks use `#c0392b`. These literals are replaced by theme tokens at load
  time (`symbolColorOverrides`).
- Terminal labels are part of the SVG, matching the CADe_SIMU convention. The component tag
  and simulation status are painted by `ComponentItem`, not embedded in the artwork.
- The component tag is drawn immediately to the left of the symbol, vertically centred, as in
  CADe_SIMU.

## Regenerating exact artwork

```bash
python tools/re_cadesimu/build_exact_assets.py
python tools/re_cadesimu/finalize_catalog.py
```

`build_exact_assets.py` reads the isolated GDI traces in `docs/cadesimu/gdi_clean/`, rebuilds the
SVGs, recalculates the pin endpoints, preserves the existing metadata and regenerates state
variants. `finalize_catalog.py` hides any type without a verified clean trace.

The older hand-drawn generator (`tools/cadesimu_catalog/generate.py`) is retained for the legacy
types and as a reference, but it is no longer the source of truth for the visible library.
