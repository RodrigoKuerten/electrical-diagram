#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/ids.hpp"
#include "core/model.hpp"

namespace fiovivo {

enum class ProblemSeverity { Error, Warning, Info };

enum class ProblemCode {
  DuplicateTag,
  DuplicateId,
  FloatingWire,
  InvalidWireEndpoint,
  SelfLoopNotAllowed,
  RequiredPinUnconnected,
  UnmatchedContact,
  CoilWithoutContact,
  MissingSupplyPath,
  ComponentOverlap,
  InvalidComponent,
  InvalidPinConnection,
};

[[nodiscard]] std::string_view toString(ProblemSeverity severity);
[[nodiscard]] std::string_view toString(ProblemCode code);

struct Problem {
  ProblemSeverity severity = ProblemSeverity::Warning;
  ProblemCode code = ProblemCode::InvalidComponent;
  std::string message;
  SheetId sheet;
  std::optional<ComponentId> component;
  std::optional<WireId> wire;
  std::optional<PinRef> pin;
};

struct ValidationReport {
  std::vector<Problem> problems;

  [[nodiscard]] bool hasErrors() const;
  [[nodiscard]] bool hasWarnings() const;
  [[nodiscard]] std::size_t errorCount() const;
  [[nodiscard]] std::size_t warningCount() const;
  [[nodiscard]] std::size_t infoCount() const;
  void add(Problem problem);
  void clear() { problems.clear(); }
};

// Runs every structural and electrical sanity rule that does not require a simulation state.
// Labels are resolved with Component::labelGroup(). The report is deterministic and ordered by
// sheet, then by rule, then by component/wire id.
[[nodiscard]] ValidationReport validate(const Diagram& diagram);

// Connectivity helper used by validation and the UI. It treats every pass-through element as
// potentially conductive (any contact position), but never traverses loads.
[[nodiscard]] bool coilHasSupplyPath(const Diagram& diagram, const Component& coil);

} // namespace fiovivo
