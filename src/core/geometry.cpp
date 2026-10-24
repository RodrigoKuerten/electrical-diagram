#include "core/geometry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace fiovivo {
namespace {

constexpr double kEpsilon = 1e-9;

[[nodiscard]] bool nearlyEqual(double a, double b) { return std::abs(a - b) <= kEpsilon; }

[[nodiscard]] std::vector<Vec2> simplify(const std::vector<Vec2>& input) {
  std::vector<Vec2> result;
  result.reserve(input.size());
  for (const Vec2& point : input) {
    if (!result.empty()) {
      const Vec2& previous = result.back();
      if (nearlyEqual(previous.x, point.x) && nearlyEqual(previous.y, point.y)) {
        continue;
      }
      if (result.size() >= 2) {
        const Vec2& before = result[result.size() - 2];
        const bool same_x = nearlyEqual(before.x, previous.x) && nearlyEqual(previous.x, point.x);
        const bool same_y = nearlyEqual(before.y, previous.y) && nearlyEqual(previous.y, point.y);
        if (same_x || same_y) {
          result.back() = point;
          continue;
        }
      }
    }
    result.push_back(point);
  }
  return result;
}

[[nodiscard]] double collisionLength(const std::vector<Vec2>& route,
                                     const std::vector<Rect>& rects) {
  double total = 0.0;
  for (std::size_t i = 1; i < route.size(); ++i) {
    const Segment segment{route[i - 1], route[i]};
    for (const Rect& rect : rects) {
      if (segmentIntersectsRect(segment, rect)) {
        total += distance(segment.from, segment.to);
        break;
      }
    }
  }
  return total;
}

[[nodiscard]] std::vector<Vec2> makeRoute(Vec2 start, Vec2 s1, Vec2 e1, Vec2 end) {
  std::vector<Vec2> points{start, s1};
  if (!(nearlyEqual(s1.x, e1.x) || nearlyEqual(s1.y, e1.y))) {
    if (nearlyEqual(start.x, s1.x)) {
      points.push_back({s1.x, e1.y});
    } else {
      points.push_back({e1.x, s1.y});
    }
  }
  points.push_back(e1);
  points.push_back(end);
  return simplify(points);
}

} // namespace

Rotation nextClockwise(Rotation rotation) {
  switch (rotation) {
  case Rotation::Deg0:
    return Rotation::Deg90;
  case Rotation::Deg90:
    return Rotation::Deg180;
  case Rotation::Deg180:
    return Rotation::Deg270;
  case Rotation::Deg270:
    return Rotation::Deg0;
  }
  return Rotation::Deg0;
}

Rotation mirrorRotation(Rotation rotation) {
  switch (rotation) {
  case Rotation::Deg0:
    return Rotation::Deg0;
  case Rotation::Deg90:
    return Rotation::Deg270;
  case Rotation::Deg180:
    return Rotation::Deg180;
  case Rotation::Deg270:
    return Rotation::Deg90;
  }
  return Rotation::Deg0;
}

int rotationDegrees(Rotation rotation) { return static_cast<int>(rotation); }

Vec2 rotateVector(Vec2 value, Rotation rotation) {
  switch (rotation) {
  case Rotation::Deg0:
    return value;
  case Rotation::Deg90:
    return {-value.y, value.x};
  case Rotation::Deg180:
    return {-value.x, -value.y};
  case Rotation::Deg270:
    return {value.y, -value.x};
  }
  return value;
}

Vec2 rotateAround(Vec2 point, Vec2 center, Rotation rotation) {
  return center + rotateVector(point - center, rotation);
}

Rect boundingRect(const std::vector<Vec2>& points) {
  if (points.empty()) {
    return {};
  }
  double min_x = points.front().x;
  double min_y = points.front().y;
  double max_x = min_x;
  double max_y = min_y;
  for (const Vec2& point : points) {
    min_x = std::min(min_x, point.x);
    min_y = std::min(min_y, point.y);
    max_x = std::max(max_x, point.x);
    max_y = std::max(max_y, point.y);
  }
  return {min_x, min_y, max_x - min_x, max_y - min_y};
}

Rect rotatedBounds(Vec2 position, Vec2 size, Rotation rotation) {
  const Vec2 center = position + size / 2.0;
  const std::array<Vec2, 4> corners{
      rotateAround(position, center, rotation),
      rotateAround({position.x + size.x, position.y}, center, rotation),
      rotateAround({position.x + size.x, position.y + size.y}, center, rotation),
      rotateAround({position.x, position.y + size.y}, center, rotation)};
  return boundingRect({corners.begin(), corners.end()});
}

bool isOrthogonal(const Segment& segment) {
  return nearlyEqual(segment.from.x, segment.to.x) || nearlyEqual(segment.from.y, segment.to.y);
}

bool segmentsIntersect(const Segment& a, const Segment& b) {
  const double a_min_x = std::min(a.from.x, a.to.x);
  const double a_max_x = std::max(a.from.x, a.to.x);
  const double a_min_y = std::min(a.from.y, a.to.y);
  const double a_max_y = std::max(a.from.y, a.to.y);
  const double b_min_x = std::min(b.from.x, b.to.x);
  const double b_max_x = std::max(b.from.x, b.to.x);
  const double b_min_y = std::min(b.from.y, b.to.y);
  const double b_max_y = std::max(b.from.y, b.to.y);
  return a_min_x <= b_max_x && a_max_x >= b_min_x && a_min_y <= b_max_y && a_max_y >= b_min_y;
}

bool segmentIntersectsRect(const Segment& segment, const Rect& rect) {
  const double min_x = std::min(segment.from.x, segment.to.x);
  const double max_x = std::max(segment.from.x, segment.to.x);
  const double min_y = std::min(segment.from.y, segment.to.y);
  const double max_y = std::max(segment.from.y, segment.to.y);
  if (max_x < rect.left() || min_x > rect.right() || max_y < rect.top() || min_y > rect.bottom()) {
    return false;
  }
  if (nearlyEqual(min_x, max_x)) {
    return min_x >= rect.left() && min_x <= rect.right();
  }
  if (nearlyEqual(min_y, max_y)) {
    return min_y >= rect.top() && min_y <= rect.bottom();
  }
  return true;
}

std::vector<Vec2> orthogonalRoute(Vec2 start,
                                  Vec2 end,
                                  Vec2 start_direction,
                                  Vec2 end_direction,
                                  const std::vector<Rect>& obstacles,
                                  const RouteOptions& options) {
  std::vector<Rect> expanded;
  expanded.reserve(obstacles.size());
  for (const Rect& rect : obstacles) {
    expanded.push_back(rect.expanded(options.clearance));
  }

  if (nearlyEqual(start.x, end.x) || nearlyEqual(start.y, end.y)) {
    const Segment direct{start, end};
    const bool blocked = std::any_of(expanded.begin(), expanded.end(), [&direct](const Rect& rect) {
      return segmentIntersectsRect(direct, rect);
    });
    if (!blocked) {
      return simplify({start, end});
    }
  }

  auto normalize = [](Vec2 value) {
    Vec2 result{};
    if (std::abs(value.x) > std::abs(value.y)) {
      result.x = value.x < 0.0 ? -1.0 : 1.0;
    } else if (value.y != 0.0) {
      result.y = value.y < 0.0 ? -1.0 : 1.0;
    }
    return result;
  };

  Vec2 start_dir = normalize(start_direction);
  if (start_dir.x == 0.0 && start_dir.y == 0.0) {
    start_dir = options.prefer_horizontal_first ? Vec2{end.x >= start.x ? 1.0 : -1.0, 0.0}
                                                : Vec2{0.0, end.y >= start.y ? 1.0 : -1.0};
  }
  Vec2 end_dir = normalize(end_direction);
  if (end_dir.x == 0.0 && end_dir.y == 0.0) {
    end_dir = options.prefer_horizontal_first ? Vec2{0.0, end.y >= start.y ? 1.0 : -1.0}
                                              : Vec2{end.x >= start.x ? 1.0 : -1.0, 0.0};
  }

  const Vec2 s1 = start + start_dir * options.stub_length;
  const Vec2 e1 = end + end_dir * options.stub_length;

  std::vector<std::vector<Vec2>> candidates;
  candidates.push_back(makeRoute(start, s1, e1, end));

  // Alternative candidates use a single middle coordinate. They give the router a chance to go
  // around a symbol instead of through it.
  const std::array<double, 2> middle_x{(start.x + end.x) / 2.0, (start.x + end.x) / 2.0};
  const std::array<double, 2> middle_y{start.y, end.y};
  candidates.push_back(simplify({start, s1, {middle_x[0], s1.y}, {middle_x[0], end.y}, e1, end}));
  candidates.push_back(simplify({start, s1, {s1.x, middle_y[1]}, {e1.x, middle_y[1]}, e1, end}));
  candidates.push_back(simplify({start, s1, {end.x, s1.y}, end}));
  candidates.push_back(simplify({start, s1, {s1.x, end.y}, end}));

  if (!expanded.empty()) {
    Rect combined = expanded.front();
    for (const Rect& rect : expanded) {
      const double left = std::min(combined.left(), rect.left());
      const double top = std::min(combined.top(), rect.top());
      const double right = std::max(combined.right(), rect.right());
      const double bottom = std::max(combined.bottom(), rect.bottom());
      combined = Rect{left, top, right - left, bottom - top};
    }
    const double top = combined.top() - 1.0;
    const double bottom = combined.bottom() + 1.0;
    const double left = combined.left() - 1.0;
    const double right = combined.right() + 1.0;
    candidates.push_back(simplify({start, {start.x, top}, {end.x, top}, end}));
    candidates.push_back(simplify({start, {start.x, bottom}, {end.x, bottom}, end}));
    candidates.push_back(simplify({start, {left, start.y}, {left, end.y}, end}));
    candidates.push_back(simplify({start, {right, start.y}, {right, end.y}, end}));
  }

  std::size_t best_index = 0;
  double best_cost = std::numeric_limits<double>::infinity();
  for (std::size_t i = 0; i < candidates.size(); ++i) {
    const std::vector<Vec2>& candidate = candidates[i];
    bool orthogonal = true;
    double total_length = 0.0;
    for (std::size_t p = 1; p < candidate.size(); ++p) {
      const Segment segment{candidate[p - 1], candidate[p]};
      if (!isOrthogonal(segment)) {
        orthogonal = false;
        break;
      }
      total_length += distance(segment.from, segment.to);
    }
    if (!orthogonal) {
      continue;
    }
    const double cost = collisionLength(candidate, expanded) * 1000.0 + total_length +
                        static_cast<double>(candidate.size()) * 0.01;
    if (cost < best_cost) {
      best_cost = cost;
      best_index = i;
    }
  }
  return candidates[best_index];
}

Vec2 snapToGrid(Vec2 point, double step) {
  if (step <= 0.0) {
    return point;
  }
  return {std::round(point.x / step) * step, std::round(point.y / step) * step};
}

} // namespace fiovivo
