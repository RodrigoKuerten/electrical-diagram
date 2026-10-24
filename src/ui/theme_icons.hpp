#pragma once

#include "ui/theme.hpp"

#include <QIcon>

namespace fiovivo::ui {

enum class ThemeIcon {
  New,
  Open,
  Save,
  Undo,
  Redo,
  Cut,
  Copy,
  Paste,
  ZoomIn,
  ZoomOut,
  Fit,
  Play,
  Pause,
  Step,
  Stop,
  Grid,
  Wire,
  Validate,
};

// Draws a small monochrome icon using icon.primary. Icons are generated for the active theme
// instead of relying on platform icon sets that only suit one background.
[[nodiscard]] QIcon themeIcon(ThemeIcon icon, const ThemePalette& palette);

} // namespace fiovivo::ui
