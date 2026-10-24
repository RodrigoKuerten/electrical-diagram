#include "core/geometry.hpp"

#include <gtest/gtest.h>

#include <cmath>

namespace fiovivo {
namespace {

TEST(GeometryTest, RotationCyclesClockwise) {
  EXPECT_EQ(nextClockwise(Rotation::Deg0), Rotation::Deg90);
  EXPECT_EQ(nextClockwise(Rotation::Deg90), Rotation::Deg180);
  EXPECT_EQ(nextClockwise(Rotation::Deg180), Rotation::Deg270);
  EXPECT_EQ(nextClockwise(Rotation::Deg270), Rotation::Deg0);
}

TEST(GeometryTest, RotationVectorIsExactForRightAngles) {
  const Vec2 point{2.0, 3.0};
  const Vec2 rotated = rotateVector(point, Rotation::Deg90);
  EXPECT_DOUBLE_EQ(rotated.x, -3.0);
  EXPECT_DOUBLE_EQ(rotated.y, 2.0);
  const Vec2 restored = rotateVector(rotated, Rotation::Deg270);
  EXPECT_DOUBLE_EQ(restored.x, point.x);
  EXPECT_DOUBLE_EQ(restored.y, point.y);
}

TEST(GeometryTest, RotatesAroundCenter) {
  const Vec2 result = rotateAround({0.0, 0.0}, {5.0, 5.0}, Rotation::Deg180);
  EXPECT_NEAR(result.x, 10.0, 1e-12);
  EXPECT_NEAR(result.y, 10.0, 1e-12);
}

TEST(GeometryTest, RotatedBoundsSwapsDimensions) {
  const Rect bounds = rotatedBounds({10.0, 20.0}, {40.0, 20.0}, Rotation::Deg90);
  EXPECT_NEAR(bounds.width, 20.0, 1e-12);
  EXPECT_NEAR(bounds.height, 40.0, 1e-12);
  EXPECT_NEAR(bounds.center().x, 30.0, 1e-12);
  EXPECT_NEAR(bounds.center().y, 30.0, 1e-12);
}

TEST(GeometryTest, BoundingRectHandlesPoints) {
  const Rect bounds = boundingRect({{5.0, 8.0}, {-2.0, 3.0}, {9.0, -1.0}});
  EXPECT_DOUBLE_EQ(bounds.left(), -2.0);
  EXPECT_DOUBLE_EQ(bounds.top(), -1.0);
  EXPECT_DOUBLE_EQ(bounds.width, 11.0);
  EXPECT_DOUBLE_EQ(bounds.height, 9.0);
}

TEST(GeometryTest, OrthogonalRouteHasOnlyRightAngles) {
  const std::vector<Vec2> route =
      orthogonalRoute({0.0, 0.0}, {100.0, 50.0}, {1.0, 0.0}, {-1.0, 0.0});
  ASSERT_GE(route.size(), 2U);
  EXPECT_EQ(route.front(), (Vec2{0.0, 0.0}));
  EXPECT_EQ(route.back(), (Vec2{100.0, 50.0}));
  for (std::size_t i = 1; i < route.size(); ++i) {
    EXPECT_TRUE(isOrthogonal({route[i - 1], route[i]})) << "segment " << i;
  }
}

TEST(GeometryTest, OrthogonalRouteIsDeterministic) {
  const auto first = orthogonalRoute({0.0, 0.0}, {80.0, 40.0}, {1.0, 0.0}, {0.0, 1.0});
  const auto second = orthogonalRoute({0.0, 0.0}, {80.0, 40.0}, {1.0, 0.0}, {0.0, 1.0});
  ASSERT_EQ(first.size(), second.size());
  for (std::size_t i = 0; i < first.size(); ++i) {
    EXPECT_EQ(first[i], second[i]);
  }
}

TEST(GeometryTest, OrthogonalRouteUsesDirectLineWhenAligned) {
  const auto route = orthogonalRoute({10.0, 0.0}, {10.0, 50.0}, {0.0, 1.0}, {0.0, -1.0});
  ASSERT_EQ(route.size(), 2U);
  EXPECT_EQ(route.front(), (Vec2{10.0, 0.0}));
  EXPECT_EQ(route.back(), (Vec2{10.0, 50.0}));
}

TEST(GeometryTest, SegmentIntersectsRect) {
  EXPECT_TRUE(segmentIntersectsRect({{0.0, 5.0}, {10.0, 5.0}}, {2.0, 0.0, 4.0, 10.0}));
  EXPECT_FALSE(segmentIntersectsRect({{0.0, 5.0}, {1.0, 5.0}}, {2.0, 0.0, 4.0, 10.0}));
}

TEST(GeometryTest, SnapToGrid) {
  EXPECT_EQ(snapToGrid({7.4, 12.6}, 5.0), (Vec2{5.0, 15.0}));
  EXPECT_EQ(snapToGrid({7.4, 12.6}, 0.0), (Vec2{7.4, 12.6}));
}

TEST(GeometryTest, MirrorRotation) {
  EXPECT_EQ(mirrorRotation(Rotation::Deg0), Rotation::Deg0);
  EXPECT_EQ(mirrorRotation(Rotation::Deg90), Rotation::Deg270);
  EXPECT_EQ(mirrorRotation(Rotation::Deg180), Rotation::Deg180);
  EXPECT_EQ(mirrorRotation(Rotation::Deg270), Rotation::Deg90);
}

TEST(GeometryTest, RotationDegreesAndDegenerateInputs) {
  EXPECT_EQ(rotationDegrees(Rotation::Deg90), 90);
  EXPECT_EQ(rotateVector({2.0, 3.0}, Rotation::Deg0), (Vec2{2.0, 3.0}));
  EXPECT_EQ(rotateVector({2.0, 3.0}, Rotation::Deg180), (Vec2{-2.0, -3.0}));
  EXPECT_EQ(boundingRect({}), (Rect{}));
  EXPECT_TRUE((Rect{0.0, 0.0, 10.0, 10.0}).isValid());
  EXPECT_FALSE((Rect{0.0, 0.0, 0.0, 10.0}).isValid());
  EXPECT_TRUE((Rect{0.0, 0.0, 10.0, 10.0}).contains(Rect{1.0, 1.0, 2.0, 2.0}));
}

TEST(GeometryTest, SegmentsIntersect) {
  EXPECT_TRUE(segmentsIntersect({{0.0, 0.0}, {10.0, 0.0}}, {{5.0, -5.0}, {5.0, 5.0}}));
  EXPECT_FALSE(segmentsIntersect({{0.0, 0.0}, {1.0, 0.0}}, {{5.0, 5.0}, {6.0, 5.0}}));
  EXPECT_FALSE(segmentsIntersect({{0.0, 0.0}, {1.0, 0.0}}, {{5.0, 5.0}, {6.0, 6.0}}));
}

TEST(GeometryTest, RouteAvoidsObstacleWhenPossible) {
  const Rect obstacle{40.0, 10.0, 20.0, 20.0};
  const auto route =
      orthogonalRoute({0.0, 20.0}, {100.0, 20.0}, {1.0, 0.0}, {-1.0, 0.0}, {obstacle});
  ASSERT_GE(route.size(), 2U);
  bool touches = false;
  for (std::size_t index = 1; index < route.size(); ++index) {
    touches = touches || segmentIntersectsRect({route[index - 1], route[index]}, obstacle);
  }
  EXPECT_FALSE(touches);
}

TEST(GeometryTest, RectOverlapAndMargin) {
  const Rect a{0.0, 0.0, 10.0, 10.0};
  const Rect b{5.0, 5.0, 10.0, 10.0};
  EXPECT_TRUE(a.intersects(b));
  EXPECT_TRUE(a.contains(Vec2{2.0, 2.0}));
  EXPECT_FALSE(a.contains(Vec2{11.0, 2.0}));
  EXPECT_TRUE(a.expanded(1.0).contains(Vec2{-1.0, -1.0}));
}

} // namespace
} // namespace fiovivo
