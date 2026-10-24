# Track A - Light-theme audit and fix

Status: implemented locally; awaiting Gate A approval.

## 1. Reproduction and environment

The defect was reproduced on Windows 11 with the operating system set to dark mode:

- Qt 6.12.0, MSYS2 UCRT64, MinGW GCC 16.2.0, Debug build.
- Application settings: `ui/theme=light`.
- The window title bar and native controls were light, but the editor itself used a dark
  palette. Switching to **Dark** and back to **Light** reproduced the same result.

The Qt probe used during the investigation showed:

```text
theme=light
style=fusion
palette Window=#323232 WindowText=#f0f0f0 Base=#242424
```

`QFusionStyle::standardPalette()` followed the Windows dark colour scheme. The old light
branch assumed that it returned a fixed light palette:

```cpp
application.setStyle(QStyleFactory::create("Fusion"));
application.setPalette(application.style()->standardPalette());
```

That assumption is false on a dark host. The application therefore applied a dark palette
while `AppSettings::theme()` still said `light`.

## 2. Problems found

| # | Surface | Observed before the fix | Root cause | Files |
|---|---|---|---|---|
| 1 | Whole application | `light` looked dark; the menu/toolbar could disagree with the content area | `QStyle::standardPalette()` depends on the OS colour scheme and was used as "light" | `src/ui/app_settings.cpp` |
| 2 | Canvas and sheet | Canvas background, page shadow, page border and title block were fixed colours set ad hoc by `MainWindow`, producing a mixed light canvas and dark application | No theme object; scene stored literal colours in members and helpers | `src/ui/sheet_scene.{hpp,cpp}`, `src/ui/main_window.cpp` |
| 3 | Grid and title block | Grid and title-block labels were hard-coded light-sheet colours and could not follow a theme | Literal colours in `drawBackground()` | `src/ui/sheet_scene.cpp` |
| 4 | Symbols | Legacy SVG artwork contains literal stroke/fill colours (`#1f2933`, `#f2b134`, `#f7d154`, `#f2d27a`, `#c0392b`, `#4caf50`); the bright active fills were below 4.5:1 on white | SVG renderer loaded artwork verbatim; no theme mapping | `src/library/svg_renderer.{hpp,cpp}`, `assets/symbols/*.svg` |
| 5 | Library icons | Symbols were nearly invisible on a dark dock because the transparent icons were drawn over a dark background | No light tile and no theme-aware icon compositing | `src/ui/library_dock.cpp` |
| 6 | Toolbar and menu icons | Platform icon set was designed for one background and did not change with the theme | `QStyle::standardIcon()` used everywhere | `src/ui/main_window.cpp` |
| 7 | Canvas labels, pins, wires, selection | Label outline used literal white; pin/wire/selection fallbacks had literal colours | Item paint code read hard-coded values | `src/ui/component_item.cpp`, `pin_item.cpp`, `wire_item.cpp` |
| 8 | Docks | Dock text and backgrounds followed the dark OS palette in "light" mode | Same application-palette failure | `src/ui/app_settings.cpp` |
| 9 | Problems and About dialogs | Inline stylesheet colours bypassed the palette; warning/error colours were fixed | `setStyleSheet("color:#...")` | `src/ui/problems_dock.cpp`, `src/ui/dialogs/about_dialog.cpp` |
| 10 | Preferences | Wire/energized colour buttons used literal `white` text and per-user colours; those values bypassed the theme | Custom colour settings and inline CSS | `src/ui/dialogs/preferences_dialog.cpp`, `src/ui/app_settings.cpp` |
| 11 | Properties dock | Wire colour swatch used literal `white` text; a light swatch could be unreadable | Inline CSS and no contrast decision | `src/ui/properties_dock.cpp` |
| 12 | Runtime switch | Only the scene was refreshed; icons, library thumbnails, problems summary and symbol renderers were not | `MainWindow::applyTheme()` touched only a few scene colours | `src/ui/main_window.cpp` |

## 3. Fix

### 3.1 One palette module, named tokens

`src/ui/theme.{hpp,cpp}` now owns every theme colour. Tokens are addressed by the
`ThemeToken` enum and have stable names such as:

- `window.background`, `window.text`, `base.background`, `base.text`, `button.*`,
  `tooltip.*`, `highlight.*`, `link`, `text.disabled`
- `canvas.background`, `canvas.border`, `sheet.background`, `sheet.border`, `sheet.text`,
  `sheet.text.muted`, `title_block.background`, `grid.minor`, `grid.major`, `sheet.shadow`
- `symbol.stroke`, `symbol.active`, `symbol.danger`, `symbol.success`
- `wire.default`, `wire.energized`, `selection`, `focus`
- `text.primary`, `text.secondary`, `status.error`, `status.warning`, `status.info`,
  `status.success`, `icon.primary`, `library.symbol_tile`, `text.on_light`, `text.on_dark`

Both themes define all tokens. No hex colour literal exists outside
`src/ui/theme.cpp`; the theme test scans `src/`, `tools/` and `tests/`.

The light palette is explicit (`window.background = #f3f5f8`, `base.background = #ffffff`,
`canvas.background = #d7dde5`, etc.). It is never derived from the operating-system
palette. The dark palette keeps the printable sheet white, because the sheet represents
paper; the canvas around it is dark.

### 3.2 Application and widget palettes

`AppSettings::applyAppearance()` now calls `applyTheme(QApplication&, ThemeMode)`. That
function installs the Fusion style, builds a complete `QPalette` from the tokens, and records
the palette through `currentTheme()`. `QEvent::ApplicationPaletteChange` refreshes existing
widgets, so docks, menus, status bar and dialogs switch without restart.

On Windows the native menu bar is disabled (`Qt::AA_DontUseNativeMenuBar`). Native Windows
menus follow the OS theme and would contradict the in-application choice. The Qt-drawn menu
bar now uses the same palette as the rest of the window.

### 3.3 Canvas, sheet and title block

`SheetScene::setThemePalette()` stores a `ThemePalette` and all scene painting reads tokens:
canvas, page, shadow, border, grid, frame, title block, label text and value text. Item paint
code obtains `textColor()`, `pinColor()`, `wireColor()`, `energizedColor()` and
`selectionColor()` from the scene palette. Label outlines use `sheet.background`, not
literal white.

The print/export page remains white in dark theme by design. Transparent exports still use
transparent pixels.

### 3.4 Symbols without editing the legacy artwork

`SymbolSvgRenderer` can now parse an SVG after applying caller-supplied literal-colour
overrides. `ui/theme.cpp` maps the six colours used by the legacy library:

| Legacy colour | Token |
|---|---|
| `#1f2933` | `symbol.stroke` |
| `#f2b134` | `symbol.active` |
| `#f7d154` | `symbol.active` |
| `#f2d27a` | `symbol.active` |
| `#c0392b` | `symbol.danger` |
| `#4caf50` | `symbol.success` |

`MainWindow::applyTheme()` reloads the render cache with those overrides. This keeps
`assets/symbols/` unchanged (the Track B catalog will replace the mechanism) while making
the active state accessible: `symbol.active` is `#b45309`, 5.02:1 on the white sheet,
instead of the previous bright yellow.

### 3.5 Icons and library thumbnails

`src/ui/theme_icons.cpp` draws original monochrome icons with `icon.primary`.
`MainWindow::updateThemeIcons()` regenerates all action icons when the theme changes.
`LibraryDock::setThemePalette()` composites each symbol onto a `library.symbol_tile`
(white) tile, so the dark dock keeps readable symbols without altering the SVG.

### 3.6 Docks and dialogs

- `ProblemsDock` uses a palette copy with `status.error` or `status.warning`; it no longer
  uses an inline stylesheet. `refreshTheme()` is called on a theme switch.
- `AboutDialog` uses a palette copy with `status.error`.
- `PreferencesDialog` no longer exposes per-user wire/energized colours; those were content
  overrides that bypassed the theme. The two colour rows were removed.
- `PropertiesDock` picks `text.on_light` or `text.on_dark` for wire-colour swatches based on
  the actual contrast ratio.

### 3.7 File format

Wire style and annotation colours are now optional. If omitted, the viewer uses the active
theme token (`wire.default` or `text.primary`). Existing files with explicit colours keep
them. This avoids writing a theme colour into a document merely to satisfy the schema.
`docs/file-format.schema.json` and `docs/file-format.md` were updated; the schema is embedded
at configure time.

## 4. Contrast

The test computes WCAG 2.x relative luminance. All defined text/symbol pairs must reach
4.5:1 in both themes. Twenty-five pairs are checked; the lowest ratios are:

| Pair | Light | Dark |
|---|---:|---:|
| `status.warning` on `window.background` | 4.51 | 9.48 |
| `symbol.active` on `sheet.background` | 5.02 | 5.02 |
| `symbol.success` on `sheet.background` | 5.02 | 5.02 |
| `highlight.text` on `highlight.background` | 6.70 | 5.42 |
| `wire.energized` on `sheet.background` | 5.02 | 5.02 |
| `selection` on `sheet.background` | 6.70 | 6.70 |
| `window.text` on `window.background` | 16.24 | 13.41 |
| `symbol.stroke` on `sheet.background` | 14.76 | 14.76 |

The complete pair list is `contrastPairs()` in `src/ui/theme.cpp`.

## 5. Before and after screenshots

The images were generated with `tools/theme_screenshots.cpp`, which constructs the real
`MainWindow` and grabs each surface with Qt (no desktop compositor involved). The `before`
set was produced from the pre-fix code; the `after` set was produced from this branch.
To regenerate a set:

```text
build/windows-msys2-debug/bin/fiovivo_theme_screenshots.exe --theme light --out docs/theme-audit/after/light
build/windows-msys2-debug/bin/fiovivo_theme_screenshots.exe --theme dark  --out docs/theme-audit/after/dark
```


| Surface | Before (light request, broken) | After (light) | After (dark) |
|---|---|---|---|
| Full window | [before](before/light/01-full-window.png) | [after](after/light/01-full-window.png) | [after](after/dark/01-full-window.png) |
| Canvas and selection | [before](before/light/02-canvas.png) | [after](after/light/02-canvas.png) | [after](after/dark/02-canvas.png) |
| Toolbar and icons | [before](before/light/03-toolbar.png) | [after](after/light/03-toolbar.png) | [after](after/dark/03-toolbar.png) |
| Status bar | [before](before/light/04-status-bar.png) | [after](after/light/04-status-bar.png) | [after](after/dark/04-status-bar.png) |
| Symbol library | [before](before/light/05-library-dock.png) | [after](after/light/05-library-dock.png) | [after](after/dark/05-library-dock.png) |
| Properties dock | [before](before/light/06-properties-dock.png) | [after](after/light/06-properties-dock.png) | [after](after/dark/06-properties-dock.png) |
| Problems dock | [before](before/light/07-problems-dock.png) | [after](after/light/07-problems-dock.png) | [after](after/dark/07-problems-dock.png) |
| Simulation log | [before](before/light/08b-simulation-log-dock.png) | [after](after/light/08b-simulation-log-dock.png) | [after](after/dark/08b-simulation-log-dock.png) |
| View menu | [before](before/light/09-view-menu.png) | [after](after/light/09-view-menu.png) | [after](after/dark/09-view-menu.png) |
| Preferences dialog | [before](before/light/10-preferences-dialog.png) | [after](after/light/10-preferences-dialog.png) | [after](after/dark/10-preferences-dialog.png) |
| About dialog | [before](before/light/11-about-dialog.png) | [after](after/light/11-about-dialog.png) | [after](after/dark/11-about-dialog.png) |

The dark-theme before/after pairs are also under `before/dark/` and `after/dark/`.

## 6. Tests

Run from a configured build directory:

```text
ctest --test-dir build/windows-msys2-debug --output-on-failure -j4
100% tests passed out of 140
```

Release build with warnings-as-errors:

```text
ctest --test-dir build/windows-msys2-release --output-on-failure -j4 -E RendersFiveHundredComponents
100% tests passed out of 139
```

`CanvasRenderingPerformanceTest.RendersFiveHundredComponents` exceeds its 100 ms budget
in this local environment (branch: 132-136 ms; baseline `4e70051`: 137-149 ms on repeated
runs, Qt 6.12/MinGW). It is a pre-existing local performance/environment issue, not a
theme regression, and the branch is slightly faster in the repeated comparison. The CI
budget uses the configured Qt version and remains the authority for that test.

New theme tests:

| Test | What it proves |
|---|---|
| `ThemePaletteTest.EveryTokenExistsInBothThemes` | All tokens exist, have unique names and valid colours in light and dark |
| `ThemePaletteTest.NoHexColorsOutsidePaletteModule` | No `#rrggbb` literal in `src/`, `tools/` or `tests/` outside `theme.cpp` |
| `ThemePaletteTest.ContrastPairsMeetWcagAa` | Every text/symbol token pair reaches 4.5:1 in both themes |
| `ThemeRuntimeTest.LightDarkLightRestoresTheCanvasExactly` | Light -> dark -> light renders the fixed scene with **0** differing pixels; light and dark differ |
| `ThemeRuntimeTest.PreferencesSwitchTheWholeWindowWithoutRestart` | Drives the real Preferences action and checks the application palette, settings and scene palette after both switches |
| `ThemeGoldenTest.LightAndDarkReferenceScenes` | Golden PNGs for a fixed A4 landscape scene (sheet, title block, supply, coil, motor, one wire, selected coil) in both themes |

Golden comparison tolerance: a pixel is counted as different when any channel differs by
more than 12 levels; at most 1% of pixels may differ. This covers platform font rasterisation
differences while still catching layout or colour regressions. The runtime-switch test is
exact (0 pixels) because it runs in one process.

Golden files:

- `tests/golden/theme/light.png`
- `tests/golden/theme/dark.png`

Update only when intentional:

```text
build/windows-msys2-debug/bin/fiovivo_theme_tests.exe --update-golden
```

The runner prints each changed key, for example:

```text
[GOLDEN] updated tests/golden/theme/light.png
```

## 7. Surfaces not fixed and deliberate limitations

| Surface | Status / reason |
|---|---|
| OS window title bar and native frame | Controlled by the window manager / Windows; the application cannot recolour it. The menu bar, toolbar, status bar and client area are themed. |
| Native file dialogs (`QFileDialog`, `QPrintDialog`) | Provided by the operating system; they follow the OS theme, not the app palette. |
| macOS native menu bar | Stays native by platform convention. The Windows-specific override is disabled elsewhere. |
| Legacy SVG source colours | The files in `assets/symbols/` still contain their original literals. They are mapped to tokens at renderer load time; editing them now would pre-empt Track B's catalog work. |
| Sheet in dark theme | Remains paper-white intentionally, because it is the printable page and keeps symbol/print contrast predictable. |
| Per-user wire/energized colours | Removed from Preferences. Those settings bypassed the palette and could produce arbitrary contrast. Explicit colours already stored in a `.dcj` file are still honoured as content. |

## 8. Deviations and decisions for approval

1. **File format:** wire-style and annotation `color` fields are now optional, and omitted
   when not explicitly set. This is backward compatible; existing files still load and keep
   their colours.
2. **Windows menu bar:** native menus are replaced by Qt-drawn menus on Windows so the menu
   can follow the in-app theme. This changes the menu-bar look slightly on Windows.
3. **Symbol active colour:** the bright active fills became the darker accessible
   `symbol.active` amber. The energized outline and wire highlight still use the same token,
   so the state remains obvious.
4. **Symbol artwork:** legacy symbols are recoloured at load time, not edited in place, to
   respect the Track B boundary.
