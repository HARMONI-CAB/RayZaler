#include "GUIHelpers.h"
#include <cstdlib>

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
