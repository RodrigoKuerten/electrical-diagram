#pragma once

#include <QString>

namespace fiovivo::ui {

// Returns a directory containing the symbols/ and examples/ folders, or an empty string.
[[nodiscard]] QString findAssetRoot();
[[nodiscard]] QString findExamplesDirectory();
[[nodiscard]] QString findTranslationFile(const QString& language_code);
[[nodiscard]] QString findSchemaFile();

} // namespace fiovivo::ui
