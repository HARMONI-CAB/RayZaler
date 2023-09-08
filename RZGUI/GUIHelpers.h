#ifndef GUIHELPERS_H
#define GUIHELPERS_H

#include <QString>

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

qreal   randUniform();
qreal   randNormal();

QString asScientific(qreal);
QString toSuperIndex(QString const &);

#endif // GUIHELPERS_H
