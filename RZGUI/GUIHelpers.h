#ifndef GUIHELPERS_H
#define GUIHELPERS_H

#include <QString>

class QLabel;
class QPixmap;

#define SCAST(type, value) static_cast<type>(value)
#define TOINT(value) SCAST(int, value)

#define BLOCKSIG_BEGIN(object)                   \
  do {                                           \
    QObject *obj = object;                       \
    bool blocked = (object)->blockSignals(true)

#define BLOCKSIG_END()                           \
    obj->blockSignals(blocked);                  \
  } while (false)

#define BLOCKSIG(object, op)                     \
  do {                                           \
    bool blocked = (object)->blockSignals(true); \
    (object)->op;                                \
    (object)->blockSignals(blocked);             \
  } while (false)

struct timeval;
qreal   randUniform();
qreal   randNormal();

namespace RZ {
  class Element;
}

// https://www.johndcook.com/wavelength_to_RGB.html
static inline uint32_t
wl2uint32_t(qreal w)
{
  qreal red, green, blue;
  qreal factor, gamma;
  uint R, G, B;
  uint32_t tuple = 0;

  if (w >= 380 && w < 440) {
    red   = -(w - 440) / (440 - 380);
    green = 0.0;
    blue  = 1.0;
  } else if (w >= 440 && w < 490) {
    red   = 0.0;
    green = (w - 440) / (490 - 440);
    blue  = 1.0;
  } else if (w >= 490 && w < 510) {
    red   = 0.0;
    green = 1.0;
    blue  = -(w - 510) / (510 - 490);
  } else if (w >= 510 && w < 580) {
    red   = (w - 510) / (580 - 510);
    green = 1.0;
    blue  = 0.0;
  } else if (w >= 580 && w < 645) {
    red   = 1.0;
    green = -(w - 645) / (645 - 580);
    blue  = 0.0;
  } else if (w >= 645 && w < 781) {
    red   = 1.0;
    green = 0.0;
    blue  = 0.0;
  } else {
    red   = 0.0;
    green = 0.0;
    blue  = 0.0;
  }


  // Let the intensity fall off near the vision limits

  if (w >= 380 && w < 420)
      factor = 0.3 + 0.7*(w - 380) / (420 - 380);
  else if (w >= 420 && w < 701)
      factor = 1.0;
  else if (w >= 701 && w < 781)
      factor = 0.3 + 0.7*(780 - w) / (780 - 700);
  else
      factor = 0.0;

  gamma = 0.80;

  R = static_cast<uint>(qBound(0., 255 * pow(red   * factor, gamma), 255.));
  G = static_cast<uint>(qBound(0., 255 * pow(green * factor, gamma), 255.));
  B = static_cast<uint>(qBound(0., 255 * pow(blue  * factor, gamma), 255.));

  tuple |= R << 16;
  tuple |= G << 8;
  tuple |= B;

  return tuple;
}

void sensibleUnits(qreal &, qreal &, QString &);
void sensibleUnits(qreal &, QString &);
QString toSensibleUnits(qreal val);
QString asScientific(qreal);
QString toSuperIndex(QString const &);
QString timeDeltaToString(struct timeval const &);
QString appendExtToPath(QString const &path, QString const &ext);
QString argbToCss(uint32_t);
QPixmap *elementIcon(RZ::Element *element);
void grayOutPixmap(QPixmap &dest, QPixmap const &orig);
void fixLabelSizeToContents(QLabel *label, QString text);

#endif // GUIHELPERS_H
