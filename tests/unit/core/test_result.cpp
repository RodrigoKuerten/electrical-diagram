#include "core/result.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace fiovivo {
namespace {

TEST(ResultTest, HoldsAndMovesValues) {
  Result<int> value(42);
  EXPECT_TRUE(value.hasValue());
  EXPECT_TRUE(static_cast<bool>(value));
  EXPECT_EQ(value.value(), 42);
  EXPECT_EQ(value.valueOr(0), 42);
  EXPECT_EQ(std::move(value).value(), 42);

  Result<std::string> text(std::string("hello"));
  EXPECT_EQ(text.value(), "hello");
  EXPECT_EQ(text.valueOr("fallback"), "hello");
}

TEST(ResultTest, HoldsErrors) {
  Result<int> failure(makeError(ErrorCode::NotFound, "missing", "/a/b"));
  ASSERT_FALSE(failure.hasValue());
  EXPECT_FALSE(static_cast<bool>(failure));
  EXPECT_EQ(failure.error().code, ErrorCode::NotFound);
  EXPECT_EQ(failure.error().toString(), "missing (/a/b)");
  EXPECT_EQ(failure.valueOr(7), 7);

  Result<void> void_value;
  EXPECT_TRUE(void_value.hasValue());
  EXPECT_TRUE(static_cast<bool>(void_value));

  Result<void> void_error(makeError(ErrorCode::IoError, "no file"));
  ASSERT_FALSE(void_error.hasValue());
  EXPECT_EQ(std::move(void_error).error().message, "no file");
  EXPECT_EQ(void_error.error().toString(), "no file");
}

TEST(ResultTest, ErrorCodeNamesCoverAllCodes) {
  const ErrorCode codes[] = {
      ErrorCode::InvalidArgument,
      ErrorCode::NotFound,
      ErrorCode::DuplicateId,
      ErrorCode::DuplicateTag,
      ErrorCode::InvalidReference,
      ErrorCode::SelfLoopNotAllowed,
      ErrorCode::SamePin,
      ErrorCode::DuplicateWire,
      ErrorCode::PinAlreadyOccupied,
      ErrorCode::ParseError,
      ErrorCode::UnsupportedVersion,
      ErrorCode::SchemaViolation,
      ErrorCode::IoError,
      ErrorCode::Oscillation,
      ErrorCode::SimulationError,
      ErrorCode::Unsupported,
      ErrorCode::Cancelled,
  };
  for (ErrorCode code : codes) {
    EXPECT_FALSE(errorCodeName(code).empty());
  }
}

TEST(ResultTest, ErrorWithoutContextPrintsMessageOnly) {
  const Error error = makeError(ErrorCode::ParseError, "bad json");
  EXPECT_EQ(error.toString(), "bad json");
  EXPECT_EQ(error.context, "");
}

} // namespace
} // namespace fiovivo
