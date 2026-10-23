#pragma once

#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

namespace fiovivo {

struct Vec2 {
  double x = 0.0;
  double y = 0.0;

  friend constexpr bool operator==(const Vec2&, const Vec2&) = default;
};

[[nodiscard]] constexpr Vec2 operator+(Vec2 lhs, Vec2 rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}
[[nodiscard]] constexpr Vec2 operator-(Vec2 lhs, Vec2 rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}
[[nodiscard]] constexpr Vec2 operator*(Vec2 value, double factor) {
  return {value.x * factor, value.y * factor};
}
[[nodiscard]] constexpr Vec2 operator*(double factor, Vec2 value) { return value * factor; }
[[nodiscard]] constexpr Vec2 operator/(Vec2 value, double divisor) {
  return {value.x / divisor, value.y / divisor};
}
constexpr Vec2& operator+=(Vec2& lhs, Vec2 rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}
constexpr Vec2& operator-=(Vec2& lhs, Vec2 rhs) {
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  return lhs;
}

[[nodiscard]] inline double length(Vec2 value) { return std::hypot(value.x, value.y); }
[[nodiscard]] inline double distance(Vec2 a, Vec2 b) { return length(a - b); }

enum class Rotation { Deg0 = 0, Deg90 = 90, Deg180 = 180, Deg270 = 270 };

[[nodiscard]] Rotation nextClockwise(Rotation rotation);
[[nodiscard]] Rotation mirrorRotation(Rotation rotation);
[[nodiscard]] int rotationDegrees(Rotation rotation);
[[nodiscard]] Vec2 rotateVector(Vec2 value, Rotation rotation);
[[nodiscard]] Vec2 rotateAround(Vec2 point, Vec2 center, Rotation rotation);

struct Rect {
  double x = 0.0;
  double y = 0.0;
  double width = 0.0;
  double height = 0.0;

  [[nodiscard]] double left() const { return x; }
  [[nodiscard]] double top() const { return y; }
  [[nodiscard]] double right() const { return x + width; }
  [[nodiscard]] double bottom() const { return y + height; }
  [[nodiscard]] Vec2 topLeft() const { return {x, y}; }
  [[nodiscard]] Vec2 center() const { return {x + width / 2.0, y + height / 2.0}; }
  [[nodiscard]] bool contains(Vec2 point) const {
    return point.x >= left() && point.x <= right() && point.y >= top() && point.y <= bottom();
  }
  [[nodiscard]] bool contains(const Rect& other) const {
    return other.left() >= left() && other.right() <= right() && other.top() >= top() &&
           other.bottom() <= bottom();
  }
  [[nodiscard]] bool intersects(const Rect& other) const {
    return left() < other.right() && right() > other.left() && top() < other.bottom() &&
           bottom() > other.top();
  }
  [[nodiscard]] bool isValid() const { return width > 0.0 && height > 0.0; }
  [[nodiscard]] Rect expanded(double margin) const {
    return {x - margin, y - margin, width + margin * 2.0, height + margin * 2.0};
  }

  friend constexpr bool operator==(const Rect&, const Rect&) = default;
};

[[nodiscard]] Rect boundingRect(const std::vector<Vec2>& points);
[[nodiscard]] Rect rotatedBounds(Vec2 position, Vec2 size, Rotation rotation);

struct Segment {
  Vec2 from;
  Vec2 to;
};

[[nodiscard]] bool isOrthogonal(const Segment& segment);
[[nodiscard]] bool segmentsIntersect(const Segment& a, const Segment& b);
[[nodiscard]] bool segmentIntersectsRect(const Segment& segment, const Rect& rect);

struct RouteOptions {
  double stub_length = 6.0;
  double clearance = 2.0;
  bool prefer_horizontal_first = true;
};

// Returns an orthogonal polyline from start to end. The route starts and ends with a short stub
// derived from the requested directions (0, 0 means "derive automatically"). Obstacles are
// expanded by options.clearance. The function is deterministic: identical input yields identical
// output.
[[nodiscard]] std::vector<Vec2> orthogonalRoute(Vec2 start, Vec2 end, Vec2 start_direction,
                                                Vec2 end_direction,
                                                const std::vector<Rect>& obstacles = {},
                                                const RouteOptions& options = {});

// Snaps a point to the nearest multiple of step. A step <= 0 leaves the point unchanged.
[[nodiscard]] Vec2 snapToGrid(Vec2 point, double step);

} // namespace fiovivo
