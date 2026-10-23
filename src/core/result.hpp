#pragma once

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace fiovivo {

enum class ErrorCode {
  InvalidArgument,
  NotFound,
  DuplicateId,
  DuplicateTag,
  InvalidReference,
  SelfLoopNotAllowed,
  SamePin,
  DuplicateWire,
  PinAlreadyOccupied,
  ParseError,
  UnsupportedVersion,
  SchemaViolation,
  IoError,
  Oscillation,
  SimulationError,
  Unsupported,
  Cancelled,
};

struct Error {
  ErrorCode code = ErrorCode::InvalidArgument;
  std::string message;
  std::string context;

  [[nodiscard]] std::string toString() const {
    std::string result = message;
    if (!context.empty()) {
      result += " (";
      result += context;
      result += ')';
    }
    return result;
  }
};

[[nodiscard]] inline std::string_view errorCodeName(ErrorCode code) {
  switch (code) {
    case ErrorCode::InvalidArgument:
      return "invalid_argument";
    case ErrorCode::NotFound:
      return "not_found";
    case ErrorCode::DuplicateId:
      return "duplicate_id";
    case ErrorCode::DuplicateTag:
      return "duplicate_tag";
    case ErrorCode::InvalidReference:
      return "invalid_reference";
    case ErrorCode::SelfLoopNotAllowed:
      return "self_loop_not_allowed";
    case ErrorCode::SamePin:
      return "same_pin";
    case ErrorCode::DuplicateWire:
      return "duplicate_wire";
    case ErrorCode::PinAlreadyOccupied:
      return "pin_already_occupied";
    case ErrorCode::ParseError:
      return "parse_error";
    case ErrorCode::UnsupportedVersion:
      return "unsupported_version";
    case ErrorCode::SchemaViolation:
      return "schema_violation";
    case ErrorCode::IoError:
      return "io_error";
    case ErrorCode::Oscillation:
      return "oscillation";
    case ErrorCode::SimulationError:
      return "simulation_error";
    case ErrorCode::Unsupported:
      return "unsupported";
    case ErrorCode::Cancelled:
      return "cancelled";
  }
  return "unknown";
}

template <typename T> class Result {
public:
  Result(T value) : storage_(std::move(value)) {}
  Result(Error error) : storage_(std::move(error)) {}

  [[nodiscard]] bool hasValue() const { return std::holds_alternative<T>(storage_); }
  explicit operator bool() const { return hasValue(); }

  T& value() & {
    assert(hasValue() && "Result<T>::value() called on an error");
    return std::get<T>(storage_);
  }
  const T& value() const& {
    assert(hasValue() && "Result<T>::value() called on an error");
    return std::get<T>(storage_);
  }
  T&& value() && {
    assert(hasValue() && "Result<T>::value() called on an error");
    return std::get<T>(std::move(storage_));
  }

  [[nodiscard]] const Error& error() const& {
    assert(!hasValue() && "Result<T>::error() called on a value");
    return std::get<Error>(storage_);
  }
  [[nodiscard]] Error&& error() && {
    assert(!hasValue() && "Result<T>::error() called on a value");
    return std::get<Error>(std::move(storage_));
  }

  T valueOr(T fallback) const {
    if (hasValue()) {
      return std::get<T>(storage_);
    }
    return fallback;
  }

private:
  std::variant<T, Error> storage_;
};

template <> class Result<void> {
public:
  Result() = default;
  Result(Error error) : error_(std::move(error)) {}

  [[nodiscard]] bool hasValue() const { return !error_.has_value(); }
  explicit operator bool() const { return hasValue(); }

  [[nodiscard]] const Error& error() const& {
    assert(!hasValue() && "Result<void>::error() called on success");
    return *error_;
  }
  [[nodiscard]] Error&& error() && {
    assert(!hasValue() && "Result<void>::error() called on success");
    return std::move(*error_);
  }

private:
  std::optional<Error> error_;
};

[[nodiscard]] inline Error makeError(ErrorCode code, std::string message,
                                     std::string context = {}) {
  return Error{code, std::move(message), std::move(context)};
}

} // namespace fiovivo
