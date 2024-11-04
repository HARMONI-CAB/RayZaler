#include "GUIHelpers.h"
#include <cstdlib>
#include <sys/time.h>
#include <QLabel>
#include <QFontMetrics>
#include <QPixmap>
#include <QImage>

qreal
randUniform()
{
  return SCAST(qreal, rand()) / SCAST(qreal, RAND_MAX);
}

qreal
randNormal()
{
  // Box-Muller method
  qreal u = SCAST(qreal, rand() + 1) / (SCAST(qreal, RAND_MAX) + 1);
  return sqrt(-log(u));
}

QString
appendExtToPath(QString const &path, QString const &ext)
{
  qsizetype indexDot, indexBar;

  indexDot = path.lastIndexOf('.');
  indexBar = path.lastIndexOf('/');

  //
  // a.x       2 -1
  // a        -1 -1 (indexDot < 0)
  // /a/b.x    4  2
  // /a.x/b    2  4 (indexDot < indexBar)
  // /a.x/b.x  6  4


  if (indexDot < 0 || indexDot < indexBar)
    return path + "." + ext;

  return path.first(indexDot) + "." + ext;
}

void
fixLabelSizeToContents(QLabel *label, QString text)
{
  QFont font;
  QFontMetrics fm(font);

  label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QSize size = label->size();

  auto advance = fm.horizontalAdvance(text);
  size.setWidth(advance);

  label->setMinimumSize(size);
  label->setMaximumSize(size);

}

QString
toSuperIndex(QString const &string)
{
  QString result = string;

  return result
        .replace(QString("0"), QString("⁰"))
        .replace(QString("1"), QString("¹"))
        .replace(QString("2"), QString("²"))
        .replace(QString("3"), QString("³"))
        .replace(QString("4"), QString("⁴"))
        .replace(QString("5"), QString("⁵"))
        .replace(QString("6"), QString("⁶"))
        .replace(QString("7"), QString("⁷"))
        .replace(QString("8"), QString("⁸"))
        .replace(QString("9"), QString("⁹"))
        .replace(QString("+"), QString("⁺"))
        .replace(QString("-"), QString("⁻"));
}

void
sensibleUnits(qreal &value, qreal &factor, QString &units)
{
  qreal absValue  = fabs(value);
  QString names[] = {"fm", "pm", "nm", "µm", "mm", "cm", "m", "km"};
  qreal factors[] = {1e-15, 1e-12, 1e-9, 1e-6, 1e-3, 1e-2, 1, 1e3};

  if (iszero(absValue)) {
    value  = 0;
    factor = 1;
    units  = "m";
  } else {
    unsigned int N   = sizeof(factors) / sizeof(factors[0]);
    unsigned int i = 0;

    for (i = 1; i < N; ++i)
      if (absValue < factors[i])
        break;
    
    factor = factors[i - 1];
    value /= factor;
    units  = names[i - 1];
  }
}

void
sensibleUnits(qreal &value, QString &units)
{
  qreal factor;

  sensibleUnits(value, factor, units);
}

QString
toSensibleUnits(qreal val)
{
  QString units;

  sensibleUnits(val, units);

  return QString::asprintf("%.3g ", val) + units;
}

QString
asScientific(qreal value)
{
  qreal inAbs = fabs(value);
  qreal exponent = floor(log10(inAbs));
  bool  isInf = std::isinf(value);
  bool  haveExponent = std::isfinite(exponent);
  QString result = "NaN";

  if (!isInf) {
    qreal mag = 1, mantissa;
    int iExponent;
    if (haveExponent) {
      iExponent = static_cast<int>(exponent);
      if (iExponent >= 0 && iExponent < 3)
        iExponent = 0;
      haveExponent = iExponent != 0;
    } else {
      iExponent = 0;
    }

    mag = pow(10., iExponent);
    mantissa = value / mag;

    result = QString::number(mantissa);

    if (haveExponent) {
       if (result == "1")
         result = "";
       else
         result += "×";
       result += "10" + toSuperIndex(QString::number(exponent));
    }
  } else {
    result = value < 0 ? "-∞" : "∞";
  }

  return result;
}

QString
timeDeltaToString(struct timeval const &tv)
{
  if (tv.tv_sec <= 0) {
    if (tv.tv_usec < 2000) {
      return QString::number(tv.tv_usec) + " µs";
    } else {
      return QString::number(SCAST(qreal, tv.tv_usec) * 1e-3) + " ms";
    }
  } else if (tv.tv_sec < 60) {
    qreal val = SCAST(qreal, tv.tv_sec) + 1e-6 * SCAST(qreal, tv.tv_usec);
    return QString::number(val) + " s";
  } else {
    auto sec  =  tv.tv_sec % 60;
    auto min  = (tv.tv_sec / 60) % 60;
    auto hour = (tv.tv_sec / 3600) % 24;
    auto day  =  tv.tv_sec / 86400;

    QString result;

    if (day > 0)
      result += QString::number(day) + "d, ";

    result += QString::asprintf("%02ldh %02ldm %02lds", hour, min, sec);

    return result;
  }
}

void
grayOutPixmap(QPixmap &dest, QPixmap const &pixmap)
{
  QImage image = pixmap.toImage();
  
  for (int x = 0; x < image.width(); ++x) {
    for (int y = 0; y < image.height(); ++y) {
      QColor color = QColor::fromRgba(image.pixel(x, y));

      int m = color.red() + color.green() + color.blue();

      m /= 3;

      color.setRgba(qRgba(m, m, m, color.alpha() / 2));
      image.setPixelColor(x, y, color);
    } 
  }

  dest = QPixmap::fromImage(image);
}

