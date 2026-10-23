#include <gtest/gtest.h>

#include "core/types.hpp"

namespace fiovivo {
namespace {

TEST(TypesTest, PinRoleRoundTrip) {
  for (PinRole role : {PinRole::Terminal, PinRole::CoilInput, PinRole::CoilOutput,
                       PinRole::ContactCommon, PinRole::ContactNo, PinRole::ContactNc,
                       PinRole::SupplyIn, PinRole::SupplyOut, PinRole::Neutral, PinRole::Earth,
                       PinRole::SignalIn, PinRole::SignalOut}) {
    const auto parsed = pinRoleFromString(toString(role));
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(*parsed, role);
  }
  EXPECT_FALSE(pinRoleFromString("not-a-role").has_value());
}

TEST(TypesTest, SourceRoles) {
  EXPECT_TRUE(isSourceRole(PinRole::SupplyIn));
  EXPECT_TRUE(isSourceRole(PinRole::SupplyOut));
  EXPECT_TRUE(isSourceRole(PinRole::Neutral));
  EXPECT_FALSE(isSourceRole(PinRole::Earth));
  EXPECT_FALSE(isSourceRole(PinRole::CoilInput));
}

TEST(TypesTest, PaperDimensionsLandscapeAndPortrait) {
  const Vec2 landscape = paperDimensions(PaperSize::A4, Orientation::Landscape);
  EXPECT_DOUBLE_EQ(landscape.x, 297.0);
  EXPECT_DOUBLE_EQ(landscape.y, 210.0);
  const Vec2 portrait = paperDimensions(PaperSize::A4, Orientation::Portrait);
  EXPECT_DOUBLE_EQ(portrait.x, 210.0);
  EXPECT_DOUBLE_EQ(portrait.y, 297.0);
  const Vec2 letter = paperDimensions(PaperSize::Letter, Orientation::Landscape);
  EXPECT_NEAR(letter.x, 279.4, 1e-9);
  EXPECT_NEAR(letter.y, 215.9, 1e-9);
}

TEST(TypesTest, PaperAndOrientationRoundTrip) {
  for (PaperSize size : {PaperSize::A4, PaperSize::A3, PaperSize::A2, PaperSize::A1,
                         PaperSize::Letter}) {
    EXPECT_EQ(paperSizeFromString(toString(size)), size);
  }
  EXPECT_EQ(orientationFromString("portrait"), Orientation::Portrait);
  EXPECT_EQ(orientationFromString("landscape"), Orientation::Landscape);
  EXPECT_FALSE(orientationFromString("diagonal").has_value());
  EXPECT_FALSE(paperSizeFromString("A0").has_value());
}

TEST(TypesTest, ParameterAccessors) {
  ParameterMap parameters;
  parameters["enabled"] = true;
  parameters["count"] = std::int64_t{7};
  parameters["voltage"] = 220.0;
  parameters["name"] = std::string("KM1");
  EXPECT_TRUE(parameterBool(parameters, "enabled"));
  EXPECT_FALSE(parameterBool(parameters, "missing"));
  EXPECT_EQ(parameterInt(parameters, "count"), 7);
  EXPECT_EQ(parameterInt(parameters, "voltage"), 220);
  EXPECT_DOUBLE_EQ(parameterNumber(parameters, "voltage"), 220.0);
  EXPECT_EQ(parameterString(parameters, "name"), "KM1");
  EXPECT_EQ(parameterString(parameters, "missing", "fallback"), "fallback");
  EXPECT_NE(findParameter(parameters, "name"), nullptr);
  EXPECT_EQ(findParameter(parameters, "other"), nullptr);
}

TEST(TypesTest, ComponentCategorization) {
  EXPECT_EQ(categorizeComponent(component_type::kSupplyAc3), ComponentCategory::Supply);
  EXPECT_EQ(categorizeComponent(component_type::kFuse), ComponentCategory::Protection);
  EXPECT_EQ(categorizeComponent(component_type::kContactorCoil), ComponentCategory::Contactor);
  EXPECT_EQ(categorizeComponent(component_type::kPushbuttonNo), ComponentCategory::Control);
  EXPECT_EQ(categorizeComponent(component_type::kTimerOn), ComponentCategory::Timer);
  EXPECT_EQ(categorizeComponent(component_type::kPilotLamp), ComponentCategory::Indication);
  EXPECT_EQ(categorizeComponent(component_type::kMotor3ph), ComponentCategory::Machine);
  EXPECT_EQ(categorizeComponent(component_type::kTerminal), ComponentCategory::Terminal);
  EXPECT_EQ(categorizeComponent(component_type::kLabelBox), ComponentCategory::Annotation);
  EXPECT_EQ(categorizeComponent("future_symbol"), ComponentCategory::Unknown);
  EXPECT_TRUE(isTimer(component_type::kTimerOff));
  EXPECT_TRUE(isCoil(component_type::kTimerOn));
  EXPECT_FALSE(isCoil(component_type::kContactorAuxNo));
  EXPECT_TRUE(isSupply(component_type::kSupplyDc));
}

TEST(TypesTest, WireDashAndAnnotationRoundTrip) {
  for (WireDash dash : {WireDash::Solid, WireDash::Dashed, WireDash::Dotted}) {
    EXPECT_EQ(wireDashFromString(toString(dash)), dash);
  }
  for (AnnotationKind kind : {AnnotationKind::Text, AnnotationKind::Rectangle,
                              AnnotationKind::Legend}) {
    EXPECT_EQ(annotationKindFromString(toString(kind)), kind);
  }
}

} // namespace
} // namespace fiovivo
