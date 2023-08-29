#ifndef GUIHELPERS_H
#define GUIHELPERS_H

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


#endif // GUIHELPERS_H
