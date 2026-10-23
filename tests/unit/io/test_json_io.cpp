#include "core/model.hpp"
#include "io/json_io.hpp"
#include "io/schema_validator.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

#include "unit/core/test_helpers.hpp"

namespace fiovivo::io {
namespace {

using nlohmann::json;

Diagram makeEmbeddedDiagram() {
  Diagram diagram;
  diagram.metadata.title = "Partida direta";
  diagram.metadata.author = "Test";
  diagram.metadata.created_utc = "2026-01-01T00:00:00Z";
  diagram.metadata.modified_utc = "2026-01-02T00:00:00Z";
  diagram.metadata.revision = "A";
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram, "Folha 1")).value();
  Component supply = test::makeSupplyAc1(diagram, "X1", {20.0, 30.0});
  supply.parameters["rated_voltage"] = 220.0;
  supply.parameters["frequency"] = std::int64_t{60};
  supply.parameters["neutral"] = true;
  supply.description = "Alimentação";
  const ComponentId supply_id = diagram.addComponent(sheet_id, std::move(supply)).value();
  Component coil = test::makeCoil(diagram, "KM1", {120.0, 40.0});
  coil.size = {40.0, 30.0};
  coil.rotation = Rotation::Deg90;
  coil.parameters["label_group"] = std::string("KM1");
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  EXPECT_TRUE(test::connect(diagram, sheet_id, supply_id, "L", coil_id, "A1").valid());
  EXPECT_TRUE(test::connect(diagram, sheet_id, supply_id, "N", coil_id, "A2").valid());
  Annotation annotation;
  annotation.id = diagram.ids().newAnnotationId();
  annotation.kind = AnnotationKind::Text;
  annotation.position = {10.0, 10.0};
  annotation.size = {50.0, 8.0};
  annotation.text = "Nota";
  annotation.font_size = 3.5;
  EXPECT_TRUE(diagram.addAnnotation(sheet_id, annotation));
  return diagram;
}

TEST(JsonIoTest, RoundTripIsByteIdentical) {
  const Diagram diagram = makeEmbeddedDiagram();
  const auto first = toJsonString(diagram);
  ASSERT_TRUE(first);
  const auto loaded = fromJsonString(first.value());
  ASSERT_TRUE(loaded);
  const auto second = toJsonString(loaded.value());
  ASSERT_TRUE(second);
  EXPECT_EQ(first.value(), second.value());
  EXPECT_EQ(loaded.value().metadata, diagram.metadata);
  EXPECT_EQ(loaded.value().sheets, diagram.sheets);
  EXPECT_EQ(loaded.value().wireCount(), diagram.wireCount());
}

TEST(JsonIoTest, UsesReadablePrefixedIds) {
  const Diagram diagram = makeEmbeddedDiagram();
  const auto text = toJsonString(diagram);
  ASSERT_TRUE(text);
  const json document = json::parse(text.value());
  EXPECT_EQ(document["format"], "dcj");
  EXPECT_EQ(document["format_version"], 1);
  EXPECT_EQ(document["sheets"].size(), 1U);
  EXPECT_EQ(document["sheets"][0]["id"], "s1");
  EXPECT_EQ(document["sheets"][0]["components"][0]["id"], "c1");
  EXPECT_EQ(document["sheets"][0]["components"][0]["pins"][0]["id"], "p1");
  EXPECT_EQ(document["sheets"][0]["wires"][0]["id"], "w1");
  EXPECT_EQ(document["library_refs"].size(), 2U);
}

TEST(JsonIoTest, PerservesUnknownFieldsAtEveryLevel) {
  json document = json::parse(toJsonString(makeEmbeddedDiagram()).value());
  document["custom_top"] = json{{"keep", true}};
  document["metadata"]["custom_metadata"] = 42;
  document["sheets"][0]["custom_sheet"] = "keep";
  document["sheets"][0]["components"][0]["custom_component"] = json::array({1, 2});
  document["sheets"][0]["components"][0]["pins"][0]["custom_pin"] = "pin";
  document["sheets"][0]["components"][0]["labels"]["custom_label"] = false;
  document["sheets"][0]["wires"][0]["custom_wire"] = 7;
  document["sheets"][0]["wires"][0]["from"]["custom_endpoint"] = "endpoint";
  document["sheets"][0]["wires"][0]["style"]["custom_style"] = "dashed";
  document["sheets"][0]["annotations"][0]["custom_annotation"] = 3.5;

  const auto loaded = fromJson(document);
  ASSERT_TRUE(loaded);
  const auto saved = toJson(loaded.value());
  ASSERT_TRUE(saved);
  EXPECT_EQ(saved.value()["custom_top"]["keep"], true);
  EXPECT_EQ(saved.value()["metadata"]["custom_metadata"], 42);
  EXPECT_EQ(saved.value()["sheets"][0]["custom_sheet"], "keep");
  EXPECT_EQ(saved.value()["sheets"][0]["components"][0]["custom_component"], json::array({1, 2}));
  EXPECT_EQ(saved.value()["sheets"][0]["components"][0]["pins"][0]["custom_pin"], "pin");
  EXPECT_EQ(saved.value()["sheets"][0]["components"][0]["labels"]["custom_label"], false);
  EXPECT_EQ(saved.value()["sheets"][0]["wires"][0]["custom_wire"], 7);
  EXPECT_EQ(saved.value()["sheets"][0]["wires"][0]["from"]["custom_endpoint"], "endpoint");
  EXPECT_EQ(saved.value()["sheets"][0]["wires"][0]["style"]["custom_style"], "dashed");
  EXPECT_EQ(saved.value()["sheets"][0]["annotations"][0]["custom_annotation"], 3.5);
}

TEST(JsonIoTest, RejectsNewerFormatVersion) {
  json document = json::parse(toJsonString(makeEmbeddedDiagram()).value());
  document["format_version"] = 2;
  const auto result = fromJson(document);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::UnsupportedVersion);
}

TEST(JsonIoTest, RejectsNonDcjDocument) {
  json document = json::parse(toJsonString(makeEmbeddedDiagram()).value());
  document["format"] = "other";
  const auto result = fromJson(document);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::SchemaViolation);
}

TEST(JsonIoTest, RejectsMissingRequiredField) {
  json document = json::parse(toJsonString(makeEmbeddedDiagram()).value());
  document["sheets"][0].erase("components");
  const auto result = fromJson(document);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::SchemaViolation);
  EXPECT_NE(result.error().toString().find("/sheets/0/components"), std::string::npos);
}

TEST(JsonIoTest, RejectsWrongFieldType) {
  json document = json::parse(toJsonString(makeEmbeddedDiagram()).value());
  document["sheets"][0]["components"][0]["tag"] = 123;
  const auto result = fromJson(document);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::SchemaViolation);
}

TEST(JsonIoTest, RejectsDanglingWireReference) {
  json document = json::parse(toJsonString(makeEmbeddedDiagram()).value());
  document["sheets"][0]["wires"][0]["to"]["pin"] = "p999";
  const auto result = fromJson(document);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::InvalidReference);
}

TEST(JsonIoTest, RejectsInvalidIdPrefix) {
  json document = json::parse(toJsonString(makeEmbeddedDiagram()).value());
  document["sheets"][0]["components"][0]["id"] = "sheet-1";
  const auto result = fromJson(document);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::SchemaViolation);
}

TEST(JsonIoTest, RejectsBadJsonWithoutThrowing) {
  const auto result = fromJsonString("{ this is not json");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::ParseError);
  EXPECT_FALSE(result.error().context.empty());
}

TEST(JsonIoTest, AcceptsUtf8Bom) {
  const std::string text = "\xEF\xBB\xBF" + toJsonString(makeEmbeddedDiagram()).value();
  const auto result = fromJsonString(text);
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value().metadata.title, "Partida direta");
}

TEST(JsonIoTest, RejectsEmptyDocument) {
  const auto result = fromJsonString("");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::ParseError);
}

TEST(JsonIoTest, RejectsNegativeTimeMetadata) {
  // Timestamps are strings, not numbers. This checks that numeric metadata is rejected by schema.
  json document = json::parse(toJsonString(makeEmbeddedDiagram()).value());
  document["metadata"]["created_utc"] = 0;
  const auto result = fromJson(document);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::SchemaViolation);
}

TEST(JsonIoTest, FileRoundTripThroughDisk) {
  const std::filesystem::path path =
      std::filesystem::temp_directory_path() / "fiovivo_json_io_test.dcj";
  const Diagram diagram = makeEmbeddedDiagram();
  ASSERT_TRUE(saveFile(diagram, path));
  const auto loaded = loadFile(path);
  ASSERT_TRUE(loaded);
  EXPECT_EQ(loaded.value().sheets, diagram.sheets);
  std::filesystem::remove(path);
}

TEST(JsonIoTest, ErrorOnMissingFile) {
  const auto result =
      loadFile(std::filesystem::temp_directory_path() / "fiovivo_does_not_exist.dcj");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::IoError);
}

TEST(JsonIoTest, SchemaItselfIsEmbeddedAndUsable) {
  EXPECT_TRUE(bundledSchema().is_object());
  EXPECT_EQ(bundledSchema()["title"], "FioVivo project file (.dcj)");
  const auto valid = validateAgainstSchema(json::parse(toJsonString(makeEmbeddedDiagram()).value()),
                                           bundledSchema());
  ASSERT_TRUE(valid);
}

} // namespace
} // namespace fiovivo::io
