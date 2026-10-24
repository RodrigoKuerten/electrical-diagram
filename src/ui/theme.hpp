#pragma once

#include <QColor>
#include <QPalette>
#include <QString>

#include <array>
#include <map>
#include <string_view>
#include <utility>
#include <vector>

class QApplication;

namespace fiovivo::ui {

enum class ThemeMode { Light, Dark };

// Every colour used by the application is addressed through this enum. New UI code must not
// construct colours directly; add a token here instead.
enum class ThemeToken {
  WindowBackground,
  WindowText,
  BaseBackground,
  BaseText,
  AlternateBase,
  ButtonBackground,
  ButtonText,
  TooltipBackground,
  TooltipText,
  HighlightBackground,
  HighlightedText,
  DisabledText,
  Link,
  CanvasBackground,
  CanvasBorder,
  SheetBackground,
  SheetBorder,
  SheetText,
  SheetTextMuted,
  TitleBlockBackground,
  GridMinor,
  GridMajor,
  SymbolStroke,
  SymbolActive,
  SymbolDanger,
  SymbolSuccess,
  WireDefault,
  WireEnergized,
  Selection,
  TextPrimary,
  TextSecondary,
  StatusError,
  StatusWarning,
  StatusInfo,
  StatusSuccess,
  IconPrimary,
  LibrarySymbolTile,
  SheetShadow,
  Focus,
  TextOnLight,
  TextOnDark,
  Count,
};

inline constexpr std::size_t kThemeTokenCount = static_cast<std::size_t>(ThemeToken::Count);

struct ThemePalette {
  ThemeMode mode = ThemeMode::Light;
  std::array<QColor, kThemeTokenCount> colors{};

  [[nodiscard]] QColor color(ThemeToken token) const;
};

[[nodiscard]] const std::array<ThemeToken, kThemeTokenCount>& allThemeTokens();
[[nodiscard]] std::string_view themeTokenName(ThemeToken token);

[[nodiscard]] ThemePalette lightTheme();
[[nodiscard]] ThemePalette darkTheme();
[[nodiscard]] ThemePalette themePalette(ThemeMode mode);

[[nodiscard]] ThemeMode themeModeFromName(const QString& name);
[[nodiscard]] QString themeModeName(ThemeMode mode);

// Builds the QPalette used by Qt widgets. The palette is explicit and never derived from the
// operating-system colour scheme.
[[nodiscard]] QPalette applicationPalette(const ThemePalette& palette);

// Applies the Fusion style and the explicit palette, records the palette as the current one and
// returns it. Existing widgets receive QEvent::ApplicationPaletteChange from Qt.
[[nodiscard]] ThemePalette applyTheme(QApplication& application, ThemeMode mode);

void setCurrentTheme(const ThemePalette& palette);
[[nodiscard]] const ThemePalette& currentTheme();

// WCAG 2.x relative-luminance contrast ratio.
[[nodiscard]] double contrastRatio(const QColor& first, const QColor& second);

// Token pairs that carry text or symbol strokes and must meet WCAG AA (4.5:1).
[[nodiscard]] const std::vector<std::pair<ThemeToken, ThemeToken>>& contrastPairs();

// Maps the colour literals present in the legacy SVG artwork to theme tokens. The catalog
// replaces this mechanism in Track B; until then it lets the existing artwork follow the theme
// without editing assets/symbols/.
[[nodiscard]] std::map<QString, QString> symbolColorOverrides(const ThemePalette& palette);

// Picks TextOnLight or TextOnDark, whichever contrasts more with the supplied background.
[[nodiscard]] QColor readableTextColor(const QColor& background, const ThemePalette& palette);

} // namespace fiovivo::ui
