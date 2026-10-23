#include "core/model.hpp"
#include "io/bom.hpp"

#include <gtest/gtest.h>

#include <string>

#include "unit/core/test_helpers.hpp"

namespace fiovivo::io {
namespace {

TEST(BomTest, GroupsAndSortsByTypeThenTag) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram, "Folha 1")).value();
  Component coil = test::makeCoil(diagram, "KM2", {0.0, 0.0});
  coil.description = "Contator auxiliar";
  coil.parameters["rated_voltage"] = 220.0;
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(coil)));
  ASSERT_TRUE(diagram.addComponent(sheet_id, test::makeSupplyAc1(diagram, "X1", {100.0, 0.0})));
  Component coil2 = test::makeCoil(diagram, "KM1", {200.0, 0.0});
  coil2.parameters["label_group"] = std::string("KM1");
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(coil2)));

  const auto csv = bomToCsv(diagram);
  ASSERT_TRUE(csv);
  const std::string expected =
      "\"Sheet\",\"Tag\",\"Type\",\"Description\",\"Parameters\"\n"
      "\"Folha 1\",\"KM1\",\"contactor_coil\",\"\",\"label_group=KM1\"\n"
      "\"Folha 1\",\"KM2\",\"contactor_coil\",\"Contator auxiliar\",\"rated_voltage=220\"\n"
      "\"Folha 1\",\"X1\",\"supply_ac1\",\"\",\"\"\n";
  EXPECT_EQ(csv.value(), expected);
}

TEST(BomTest, EscapesQuotesAndNewlines) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram, "A \"quoted\" sheet")).value();
  Component coil = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  coil.description = "line one\nline two";
  coil.parameters["label_group"] = std::string("KM1");
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(coil)));
  const auto csv = bomToCsv(diagram);
  ASSERT_TRUE(csv);
  EXPECT_NE(csv.value().find("\"A \"\"quoted\"\" sheet\""), std::string::npos);
  EXPECT_NE(csv.value().find("line one\nline two"), std::string::npos);
}

TEST(BomTest, ParameterSummaryIsStable) {
  ParameterMap parameters;
  parameters["z"] = std::int64_t{2};
  parameters["a"] = true;
  parameters["m"] = 1.5;
  parameters["s"] = std::string("x");
  EXPECT_EQ(parameterSummary(parameters), "a=true; m=1.5; s=x; z=2");
}

} // namespace
} // namespace fiovivo::io
