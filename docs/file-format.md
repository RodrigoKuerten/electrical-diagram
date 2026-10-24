# The `.dcj` project format (format version 1)

`.dcj` means **DiagramaComandos JSON**, the working format name inherited from the original
specification. It is UTF-8 JSON, human-readable and intended to round-trip without losing
data that this version does not understand.

The canonical schema is [`file-format.schema.json`](file-format.schema.json). The loader
embeds that file at configure time and validates every document against it before building
the domain model.

## Top-level object

```json
{
  "format": "dcj",
  "format_version": 1,
  "app_version": "0.1.0",
  "metadata": {
    "title": "Partida direta",
    "author": "Example author",
    "created_utc": "2026-01-01T00:00:00Z",
    "modified_utc": "2026-01-01T00:00:00Z",
    "revision": "A"
  },
  "sheets": [ ... ],
  "library_refs": [
    { "type": "contactor_coil", "symbol_version": 1 }
  ]
}
```

| Field | Type | Required | Meaning |
|---|---|---|---|
| `format` | string | yes | Must be exactly `"dcj"`. |
| `format_version` | integer | yes | Must be `1`. Newer files are refused; older files will run through migrations when a migration exists. |
| `app_version` | string | no | Informational writer version. It does not affect loading. |
| `metadata` | object | yes | Project title, author, creation/modification timestamps and revision. |
| `sheets` | array | yes | One or more sheets. |
| `library_refs` | array | no | Written from the component types in use, sorted by type. Readers do not require it. |

`app_version` is written as `0.1.0` by the current application. Metadata timestamps are
ISO-8601 UTC strings. Saving from the UI updates `modified_utc`; the serializer itself does
not touch timestamps, which makes deterministic round-trip tests possible.

## IDs

Identifiers are strings with a one-letter prefix and a positive decimal number:

| Prefix | Kind | Example |
|---|---|---|
| `c` | component | `c12` |
| `p` | pin | `p41` |
| `w` | wire | `w7` |
| `s` | sheet | `s1` |
| `a` | annotation | `a3` |

IDs are allocated sequentially per file, are stable across saves and are never reused after
deletion. A save/load/save round-trip keeps the same IDs. IDs are global; a wire endpoint
references both the component and the pin.

## Sheet

```json
{
  "id": "s1",
  "name": "Folha 1",
  "paper": "A4",
  "orientation": "landscape",
  "margin": 10.0,
  "show_frame": true,
  "components": [ ... ],
  "wires": [ ... ],
  "annotations": [ ... ]
}
```

`paper` is one of `A4`, `A3`, `A2`, `A1`, `Letter`. `orientation` is `landscape` or
`portrait`. Margins and scene coordinates are in millimetres.

## Component

```json
{
  "id": "c1",
  "type": "contactor_coil",
  "tag": "KM1",
  "description": "Main contactor",
  "position": [120.0, 40.0],
  "size": [40.0, 30.0],
  "rotation": 90,
  "pins": [
    {
      "id": "p1",
      "name": "A1",
      "role": "coil_input",
      "offset": [10.0, 0.0],
      "required": true
    }
  ],
  "parameters": {
    "rated_voltage": 220,
    "label_group": ""
  },
  "labels": {
    "show_tag": true,
    "show_description": false,
    "tag_offset": [0.0, -6.0],
    "description_offset": [0.0, 6.0]
  }
}
```

- `position` is the top-left corner in the sheet, in millimetres.
- `size` is the unrotated symbol box. Rotation is around the box centre and is one of
  `0`, `90`, `180`, `270`.
- Pin offsets are in the unrotated component coordinate system. After rotation, a pin is
  at `position + rotate(offset, size / 2, rotation)`.
- `tag` must be unique within the diagram. It is validated on edit and on load.
- `parameters` values may be booleans, integers, numbers or strings. They are preserved
  as typed values. Timers use `delay_ms` and `time_base`; contactors use `rated_voltage` and
  `label_group`; a contact's `label_group` defaults to its tag with the trailing
  `.suffix` removed (for example `KM1.1` → `KM1`).
- `z_order` is an optional internal parameter used for canvas stacking. The properties dock
  hides it.
- Labels are displayed by the canvas. Offsets are in millimetres from the component origin.

### Pin roles

`terminal`, `coil_input`, `coil_output`, `contact_common`, `contact_no`, `contact_nc`,
`supply_in`, `supply_out`, `neutral`, `earth`, `signal_in`, `signal_out`.

A pin carries at most one connection in the original specification. FioVivo deliberately
allows multiple wire IDs per pin so parallel circuits are possible; the JSON representation
does not store the back-reference list (it is rebuilt from the wires). Junction dots are a
canvas convention for three or more incident wires.

## Wire

```json
{
  "id": "w1",
  "from": { "component": "c1", "pin": "p1" },
  "to": { "component": "c2", "pin": "p4" },
  "route": [[50.0, 40.0], [90.0, 40.0], [90.0, 100.0], [120.0, 100.0]],
  "style": {
    "color": "#1f2933",
    "thickness": 0.8,
    "dash": "solid"
  },
  "allow_self_loop": false
}
```

- A wire connects exactly two distinct pins. `allow_self_loop` is required for a wire whose
  two endpoints are on the same component; the default is `false`.
- `route` is an ordered polyline in millimetres. It is orthogonal by convention; the editor
  uses `core/routing` and stores the resulting points. A route with fewer than two points is
  invalid.
- `dash` is `solid`, `dashed` or `dotted`.
- Deleting a component removes every wire attached to it.

## Annotation

```json
{
  "id": "a1",
  "kind": "text",
  "position": [10.0, 270.0],
  "size": [120.0, 8.0],
  "text": "Control circuit supplied from L1 + N.",
  "font_size": 3.5,
  "color": "#1f2933"
}
```

`kind` is `text`, `rectangle` or `legend`. The canvas paints text and rectangle framing;
legend entries are currently rendered as framed text.

## Unknown-field preservation

On load, the writer stores every unrecognized member of the root, metadata, sheet,
component, pin, labels, wire, endpoint, style and annotation objects as raw JSON. On save
those members are re-emitted before the known fields, so known fields always win on a name
collision. This provides forward compatibility as long as a future version can still parse
the old required fields. `library_refs` is regenerated rather than preserved.

The schema allows additional members; referential and semantic checks are separate.

## Load pipeline

1. Parse UTF-8 JSON (a UTF-8 BOM is accepted).
2. Check `format == "dcj"` and the presence of an integer `format_version`.
3. Refuse a version newer than the supported version with a clear message.
4. Run migrations for older versions (the framework is present; no pre-v1 format exists).
5. Validate against the embedded `file-format.schema.json` using the in-tree schema subset
   validator. Errors name the JSON pointer, for example `/sheets/0/components/2/tag`.
6. Rebuild the domain model and check referential integrity:
   - unique component, pin and wire IDs;
   - wire endpoints must reference existing pins on existing components;
   - tags must be unique in the diagram;
   - annotations and sheets must have unique IDs;
   - the diagram allocator is advanced past every loaded ID.
7. Return a `Result<Diagram>`; parse, schema, reference and I/O failures never throw across
   the library boundary.

Newer-format test, dangling references, wrong types, missing fields, bad IDs and malformed
JSON are covered by unit tests.

## Migration framework

`format_version` is dispatched through a migration chain. Today only version 1 is supported;
there are no historical fixtures invented for the test suite. The loader refuses versions
below 1 as unsupported. When version 2 is introduced, a function `v1 → v2` must be added,
the current schema bump handled, and a fixture stored under `tests/data/`. The tests for
newer-version refusal and v1 acceptance are already in place.

## Autosave and recovery

- Every two minutes, a modified project is written to
  `QStandardPaths::AppDataLocation/recovery/recovery.dcj` plus `recovery.json` metadata.
- The main file is never overwritten by the autosave.
- On startup, a recovery file newer than the main file (or an unsaved project) triggers a
  restore prompt.
- A successful manual save removes the recovery copy.

Simulation snapshots are supported in memory by `sim::Simulator::snapshot()` and
`restore()` for reproducible tests, but a snapshot file format is not exposed in this build.

## Determinism

The serializer emits an object with sorted keys (nlohmann/json's default ordered map),
two-space indentation and a final LF. Unknown values are re-parsed and re-emitted. With
timestamps fixed, `load → save` is byte-identical. The bundled examples are tested this way,
and the BOM exporter has a golden file in `tests/data/golden/`.
