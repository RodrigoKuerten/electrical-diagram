#pragma once

#include <QHash>
#include <QMetaType>
#include <QSet>
#include <QString>
#include <QtGlobal>

namespace fiovivo::ui {

struct SimItemState {
  bool energized = false;
  bool closed = false;
  bool timer_output = false;
  qint64 timer_elapsed_ms = 0;
  int selector_position = 1;
  bool input_active = false;
  bool tripped = false;
};

struct SimFrame {
  qint64 time_ms = 0;
  quint64 cycle = 0;
  QHash<quint64, SimItemState> states;
  QSet<quint64> live_wires;
};

struct SimLogEntry {
  qint64 time_ms = 0;
  quint64 component = 0;
  QString tag;
  QString state;
  QString value;
};

} // namespace fiovivo::ui

Q_DECLARE_METATYPE(fiovivo::ui::SimItemState)
Q_DECLARE_METATYPE(fiovivo::ui::SimFrame)
Q_DECLARE_METATYPE(fiovivo::ui::SimLogEntry)
