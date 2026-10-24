# Golden files

Golden files are intentional snapshots of deterministic exporter output.

- `partida-direta-bom.csv` is produced from `examples/partida-direta.dcj` through the BOM
  exporter and compared in `ExampleTest.BomMatchesGoldenFile`.

Do not update a golden file automatically. If an exporter change is intentional, review the
diff, run the generator deliberately, and commit both the code change and the golden file in
the same pull request. The SVG exporter has a determinism test (two exports in the same run
must be byte-identical) and a structural test (no raster images, text remains text).
