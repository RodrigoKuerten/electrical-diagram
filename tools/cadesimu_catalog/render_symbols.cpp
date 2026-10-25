#include <QGuiApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>
#include <QTextStream>
#include <QStringList>

int main(int argc, char** argv) {
  QGuiApplication app(argc, argv);
  if (argc < 3) {
    return 2;
  }
  const QDir in(QString::fromLocal8Bit(argv[1]));
  QDir out(QString::fromLocal8Bit(argv[2]));
  out.mkpath(".");
  const QStringList files = in.entryList({"*.svg"}, QDir::Files, QDir::Name);
  QTextStream log(stdout);
  for (const QString& name : files) {
    QSvgRenderer renderer(in.filePath(name));
    if (!renderer.isValid()) {
      log << "INVALID " << name << "\n";
      continue;
    }
    QImage image(320, 320, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRectF bounds = renderer.viewBoxF();
    const qreal scale = qMin(280.0 / qMax(1.0, bounds.width()), 280.0 / qMax(1.0, bounds.height()));
    const QSizeF size(bounds.width() * scale, bounds.height() * scale);
    const QRectF target((320 - size.width()) / 2.0, (320 - size.height()) / 2.0, size.width(), size.height());
    renderer.render(&painter, target);
    painter.end();
    image.save(out.filePath(name + ".png"));
  }
  log << "rendered " << files.size() << " files\n";
  return 0;
}
