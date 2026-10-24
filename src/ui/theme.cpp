#include "ui/theme.hpp"

#include <QApplication>
#include <QStyleFactory>

#include <algorithm>
#include <array>
#include <cmath>

namespace fiovivo::ui {
namespace {

[[nodiscard]] QColor color(const char* value) { return QColor(QString::fromLatin1(value)); }

void assign(QPalette& palette, QPalette::ColorRole role, const QColor& value) {
  palette.setColor(role, value);
}

void assignDisabled(QPalette& palette, QPalette::ColorRole role, const QColor& value) {
  palette.setColor(QPalette::Disabled, role, value);
}

[[nodiscard]] double channelLuminance(double channel) {
  return channel <= 0.04045 ? channel / 12.92 : std::pow((channel + 0.055) / 1.055, 2.4);
}

} // namespace

QColor ThemePalette::color(ThemeToken token) const {
  return colors.at(static_cast<std::size_t>(token));
}

const std::array<ThemeToken, kThemeTokenCount>& allThemeTokens() {
  static const std::array<ThemeToken, kThemeTokenCount> tokens = [] {
    std::array<ThemeToken, kThemeTokenCount> result{};
    for (std::size_t index = 0; index < kThemeTokenCount; ++index) {
      result[index] = static_cast<ThemeToken>(index);
    }
    return result;
  }();
  return tokens;
}

std::string_view themeTokenName(ThemeToken token) {
  switch (token) {
  case ThemeToken::WindowBackground:
    return "window.background";
  case ThemeToken::WindowText:
    return "window.text";
  case ThemeToken::BaseBackground:
    return "base.background";
  case ThemeToken::BaseText:
    return "base.text";
  case ThemeToken::AlternateBase:
    return "base.alternate";
  case ThemeToken::ButtonBackground:
    return "button.background";
  case ThemeToken::ButtonText:
    return "button.text";
  case ThemeToken::TooltipBackground:
    return "tooltip.background";
  case ThemeToken::TooltipText:
    return "tooltip.text";
  case ThemeToken::HighlightBackground:
    return "highlight.background";
  case ThemeToken::HighlightedText:
    return "highlight.text";
  case ThemeToken::DisabledText:
    return "text.disabled";
  case ThemeToken::Link:
    return "link";
  case ThemeToken::CanvasBackground:
    return "canvas.background";
  case ThemeToken::CanvasBorder:
    return "canvas.border";
  case ThemeToken::SheetBackground:
    return "sheet.background";
  case ThemeToken::SheetBorder:
    return "sheet.border";
  case ThemeToken::SheetText:
    return "sheet.text";
  case ThemeToken::SheetTextMuted:
    return "sheet.text.muted";
  case ThemeToken::TitleBlockBackground:
    return "title_block.background";
  case ThemeToken::GridMinor:
    return "grid.minor";
  case ThemeToken::GridMajor:
    return "grid.major";
  case ThemeToken::SymbolStroke:
    return "symbol.stroke";
  case ThemeToken::SymbolActive:
    return "symbol.active";
  case ThemeToken::SymbolDanger:
    return "symbol.danger";
  case ThemeToken::SymbolSuccess:
    return "symbol.success";
  case ThemeToken::WireDefault:
    return "wire.default";
  case ThemeToken::WireEnergized:
    return "wire.energized";
  case ThemeToken::Selection:
    return "selection";
  case ThemeToken::TextPrimary:
    return "text.primary";
  case ThemeToken::TextSecondary:
    return "text.secondary";
  case ThemeToken::StatusError:
    return "status.error";
  case ThemeToken::StatusWarning:
    return "status.warning";
  case ThemeToken::StatusInfo:
    return "status.info";
  case ThemeToken::StatusSuccess:
    return "status.success";
  case ThemeToken::IconPrimary:
    return "icon.primary";
  case ThemeToken::LibrarySymbolTile:
    return "library.symbol_tile";
  case ThemeToken::SheetShadow:
    return "sheet.shadow";
  case ThemeToken::Focus:
    return "focus";
  case ThemeToken::TextOnLight:
    return "text.on_light";
  case ThemeToken::TextOnDark:
    return "text.on_dark";
  case ThemeToken::Count:
    break;
  }
  return "unknown";
}

ThemePalette lightTheme() {
  ThemePalette palette;
  palette.mode = ThemeMode::Light;
  const auto set = [&palette](ThemeToken token, const char* value) {
    palette.colors[static_cast<std::size_t>(token)] = color(value);
  };
  set(ThemeToken::WindowBackground, "#f3f5f8");
  set(ThemeToken::WindowText, "#111827");
  set(ThemeToken::BaseBackground, "#ffffff");
  set(ThemeToken::BaseText, "#111827");
  set(ThemeToken::AlternateBase, "#eef2f6");
  set(ThemeToken::ButtonBackground, "#e8edf3");
  set(ThemeToken::ButtonText, "#111827");
  set(ThemeToken::TooltipBackground, "#1f2937");
  set(ThemeToken::TooltipText, "#f9fafb");
  set(ThemeToken::HighlightBackground, "#1d4ed8");
  set(ThemeToken::HighlightedText, "#ffffff");
  set(ThemeToken::DisabledText, "#5b6573");
  set(ThemeToken::Link, "#1d4ed8");
  set(ThemeToken::CanvasBackground, "#d7dde5");
  set(ThemeToken::CanvasBorder, "#64748b");
  set(ThemeToken::SheetBackground, "#ffffff");
  set(ThemeToken::SheetBorder, "#64748b");
  set(ThemeToken::SheetText, "#1f2933");
  set(ThemeToken::SheetTextMuted, "#475569");
  set(ThemeToken::TitleBlockBackground, "#ffffff");
  set(ThemeToken::GridMinor, "#e2e8f0");
  set(ThemeToken::GridMajor, "#cbd5e1");
  set(ThemeToken::SymbolStroke, "#1f2933");
  set(ThemeToken::SymbolActive, "#b45309");
  set(ThemeToken::SymbolDanger, "#b91c1c");
  set(ThemeToken::SymbolSuccess, "#15803d");
  set(ThemeToken::WireDefault, "#1f2933");
  set(ThemeToken::WireEnergized, "#b45309");
  set(ThemeToken::Selection, "#1d4ed8");
  set(ThemeToken::TextPrimary, "#111827");
  set(ThemeToken::TextSecondary, "#475569");
  set(ThemeToken::StatusError, "#b91c1c");
  set(ThemeToken::StatusWarning, "#a16207");
  set(ThemeToken::StatusInfo, "#1d4ed8");
  set(ThemeToken::StatusSuccess, "#166534");
  set(ThemeToken::IconPrimary, "#1f2933");
  set(ThemeToken::LibrarySymbolTile, "#ffffff");
  set(ThemeToken::SheetShadow, "#0f172a");
  palette.colors[static_cast<std::size_t>(ThemeToken::SheetShadow)].setAlpha(45);
  set(ThemeToken::Focus, "#1d4ed8");
  set(ThemeToken::TextOnLight, "#111827");
  set(ThemeToken::TextOnDark, "#ffffff");
  return palette;
}

ThemePalette darkTheme() {
  ThemePalette palette;
  palette.mode = ThemeMode::Dark;
  const auto set = [&palette](ThemeToken token, const char* value) {
    palette.colors[static_cast<std::size_t>(token)] = color(value);
  };
  set(ThemeToken::WindowBackground, "#1e2329");
  set(ThemeToken::WindowText, "#e7edf3");
  set(ThemeToken::BaseBackground, "#161a1f");
  set(ThemeToken::BaseText, "#e7edf3");
  set(ThemeToken::AlternateBase, "#20262d");
  set(ThemeToken::ButtonBackground, "#242b33");
  set(ThemeToken::ButtonText, "#e7edf3");
  set(ThemeToken::TooltipBackground, "#2b333c");
  set(ThemeToken::TooltipText, "#e7edf3");
  set(ThemeToken::HighlightBackground, "#2b6cb0");
  set(ThemeToken::HighlightedText, "#ffffff");
  set(ThemeToken::DisabledText, "#7b8794");
  set(ThemeToken::Link, "#63b3ed");
  set(ThemeToken::CanvasBackground, "#20262d");
  set(ThemeToken::CanvasBorder, "#475569");
  // The drawing sheet is intentionally paper-white in both themes: it represents the printable
  // page, and symbols keep their print colours on it.
  set(ThemeToken::SheetBackground, "#ffffff");
  set(ThemeToken::SheetBorder, "#94a3b8");
  set(ThemeToken::SheetText, "#1f2933");
  set(ThemeToken::SheetTextMuted, "#475569");
  set(ThemeToken::TitleBlockBackground, "#ffffff");
  set(ThemeToken::GridMinor, "#e2e8f0");
  set(ThemeToken::GridMajor, "#cbd5e1");
  set(ThemeToken::SymbolStroke, "#1f2933");
  set(ThemeToken::SymbolActive, "#b45309");
  set(ThemeToken::SymbolDanger, "#b91c1c");
  set(ThemeToken::SymbolSuccess, "#15803d");
  set(ThemeToken::WireDefault, "#1f2933");
  set(ThemeToken::WireEnergized, "#b45309");
  set(ThemeToken::Selection, "#1d4ed8");
  set(ThemeToken::TextPrimary, "#1f2933");
  set(ThemeToken::TextSecondary, "#a8b3c1");
  set(ThemeToken::StatusError, "#ff6b6b");
  set(ThemeToken::StatusWarning, "#fbbf24");
  set(ThemeToken::StatusInfo, "#63b3ed");
  set(ThemeToken::StatusSuccess, "#4ade80");
  set(ThemeToken::IconPrimary, "#e7edf3");
  set(ThemeToken::LibrarySymbolTile, "#ffffff");
  set(ThemeToken::SheetShadow, "#000000");
  palette.colors[static_cast<std::size_t>(ThemeToken::SheetShadow)].setAlpha(90);
  set(ThemeToken::Focus, "#63b3ed");
  set(ThemeToken::TextOnLight, "#111827");
  set(ThemeToken::TextOnDark, "#ffffff");
  return palette;
}

ThemePalette themePalette(ThemeMode mode) {
  return mode == ThemeMode::Dark ? darkTheme() : lightTheme();
}

ThemeMode themeModeFromName(const QString& name) {
  return name.compare(QStringLiteral("dark"), Qt::CaseInsensitive) == 0 ? ThemeMode::Dark
                                                                        : ThemeMode::Light;
}

QString themeModeName(ThemeMode mode) {
  return mode == ThemeMode::Dark ? QStringLiteral("dark") : QStringLiteral("light");
}

QPalette applicationPalette(const ThemePalette& palette) {
  const QColor window = palette.color(ThemeToken::WindowBackground);
  const QColor base = palette.color(ThemeToken::BaseBackground);
  const QColor button = palette.color(ThemeToken::ButtonBackground);
  const QColor disabled = palette.color(ThemeToken::DisabledText);

  QPalette result;
  assign(result, QPalette::Window, window);
  assign(result, QPalette::WindowText, palette.color(ThemeToken::WindowText));
  assign(result, QPalette::Base, base);
  assign(result, QPalette::AlternateBase, palette.color(ThemeToken::AlternateBase));
  assign(result, QPalette::ToolTipBase, palette.color(ThemeToken::TooltipBackground));
  assign(result, QPalette::ToolTipText, palette.color(ThemeToken::TooltipText));
  assign(result, QPalette::Text, palette.color(ThemeToken::BaseText));
  assign(result, QPalette::Button, button);
  assign(result, QPalette::ButtonText, palette.color(ThemeToken::ButtonText));
  assign(result, QPalette::BrightText, palette.color(ThemeToken::StatusError));
  assign(result, QPalette::Link, palette.color(ThemeToken::Link));
  assign(result, QPalette::LinkVisited, palette.color(ThemeToken::Link));
  assign(result, QPalette::Highlight, palette.color(ThemeToken::HighlightBackground));
  assign(result, QPalette::HighlightedText, palette.color(ThemeToken::HighlightedText));
  assign(result, QPalette::PlaceholderText, disabled);
  assign(result, QPalette::Light, button.lighter(140));
  assign(result, QPalette::Midlight, button.lighter(115));
  assign(result, QPalette::Mid, button.darker(115));
  assign(result, QPalette::Dark, button.darker(130));
  assign(result, QPalette::Shadow, button.darker(150));

  assignDisabled(result, QPalette::WindowText, disabled);
  assignDisabled(result, QPalette::Text, disabled);
  assignDisabled(result, QPalette::ButtonText, disabled);
  assignDisabled(result, QPalette::HighlightedText, disabled);
  return result;
}

ThemePalette applyTheme(QApplication& application, ThemeMode mode) {
  const ThemePalette palette = themePalette(mode);
  application.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
  application.setPalette(applicationPalette(palette));
  setCurrentTheme(palette);
  return palette;
}

namespace {

ThemePalette& currentThemeStorage() {
  static ThemePalette current = lightTheme();
  return current;
}

} // namespace

void setCurrentTheme(const ThemePalette& palette) { currentThemeStorage() = palette; }

const ThemePalette& currentTheme() { return currentThemeStorage(); }

double contrastRatio(const QColor& first, const QColor& second) {
  const auto luminance = [](const QColor& value) {
    const double red = channelLuminance(value.redF());
    const double green = channelLuminance(value.greenF());
    const double blue = channelLuminance(value.blueF());
    return 0.2126 * red + 0.7152 * green + 0.0722 * blue;
  };
  const double first_luminance = luminance(first);
  const double second_luminance = luminance(second);
  const double lighter = std::max(first_luminance, second_luminance);
  const double darker = std::min(first_luminance, second_luminance);
  return (lighter + 0.05) / (darker + 0.05);
}

const std::vector<std::pair<ThemeToken, ThemeToken>>& contrastPairs() {
  static const std::vector<std::pair<ThemeToken, ThemeToken>> pairs = {
      {ThemeToken::WindowText, ThemeToken::WindowBackground},
      {ThemeToken::BaseText, ThemeToken::BaseBackground},
      {ThemeToken::ButtonText, ThemeToken::ButtonBackground},
      {ThemeToken::TooltipText, ThemeToken::TooltipBackground},
      {ThemeToken::HighlightedText, ThemeToken::HighlightBackground},
      {ThemeToken::Link, ThemeToken::WindowBackground},
      {ThemeToken::TextPrimary, ThemeToken::SheetBackground},
      {ThemeToken::TextSecondary, ThemeToken::BaseBackground},
      {ThemeToken::SheetText, ThemeToken::SheetBackground},
      {ThemeToken::SheetTextMuted, ThemeToken::SheetBackground},
      {ThemeToken::SymbolStroke, ThemeToken::SheetBackground},
      {ThemeToken::SymbolActive, ThemeToken::SheetBackground},
      {ThemeToken::SymbolDanger, ThemeToken::SheetBackground},
      {ThemeToken::SymbolSuccess, ThemeToken::SheetBackground},
      {ThemeToken::SymbolStroke, ThemeToken::LibrarySymbolTile},
      {ThemeToken::SymbolActive, ThemeToken::LibrarySymbolTile},
      {ThemeToken::WireDefault, ThemeToken::SheetBackground},
      {ThemeToken::WireEnergized, ThemeToken::SheetBackground},
      {ThemeToken::Selection, ThemeToken::SheetBackground},
      {ThemeToken::StatusError, ThemeToken::WindowBackground},
      {ThemeToken::StatusWarning, ThemeToken::WindowBackground},
      {ThemeToken::StatusInfo, ThemeToken::WindowBackground},
      {ThemeToken::StatusSuccess, ThemeToken::WindowBackground},
      {ThemeToken::IconPrimary, ThemeToken::WindowBackground},
      {ThemeToken::IconPrimary, ThemeToken::ButtonBackground},
  };
  return pairs;
}

std::map<QString, QString> symbolColorOverrides(const ThemePalette& palette) {
  return {
      {QStringLiteral("#1f2933"), palette.color(ThemeToken::SymbolStroke).name()},
      {QStringLiteral("#f2b134"), palette.color(ThemeToken::SymbolActive).name()},
      {QStringLiteral("#f7d154"), palette.color(ThemeToken::SymbolActive).name()},
      {QStringLiteral("#f2d27a"), palette.color(ThemeToken::SymbolActive).name()},
      {QStringLiteral("#c0392b"), palette.color(ThemeToken::SymbolDanger).name()},
      {QStringLiteral("#4caf50"), palette.color(ThemeToken::SymbolSuccess).name()},
  };
}

QColor readableTextColor(const QColor& background, const ThemePalette& palette) {
  const QColor light = palette.color(ThemeToken::TextOnLight);
  const QColor dark = palette.color(ThemeToken::TextOnDark);
  return contrastRatio(background, light) >= contrastRatio(background, dark) ? light : dark;
}

} // namespace fiovivo::ui
