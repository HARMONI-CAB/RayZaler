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

void sensibleUnits(qreal &, qreal &, QString &);
void sensibleUnits(qreal &, QString &);
QString toSensibleUnits(qreal val);
QString asScientific(qreal);
QString toSuperIndex(QString const &);
QString timeDeltaToString(struct timeval const &);
QString appendExtToPath(QString const &path, QString const &ext);
void grayOutPixmap(QPixmap &dest, QPixmap const &orig);
void fixLabelSizeToContents(QLabel *label, QString text);

#endif // GUIHELPERS_H
