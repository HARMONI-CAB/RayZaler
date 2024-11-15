//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//


#include "JsonSerializable.h"
#include <QJsonDocument>

JsonSerializable::~JsonSerializable()
{
}

void
JsonSerializable::setLastError(QString const &error)
{
  m_lastError = error;
}

__attribute__((__format__ (__printf__, 2, 0))) void
JsonSerializable::errorSprintf(const char *fmt, va_list ap)
{
  setLastError(QString::vasprintf(fmt, ap));
}

QString
JsonSerializable::lastError() const
{
  return m_lastError;
}


bool
JsonSerializable::deserialize(QByteArray const &json)
{
  QJsonDocument doc;
  QJsonObject obj;
  QJsonParseError errors;

  doc = QJsonDocument::fromJson(json, &errors);

  if (doc.isNull()) {
    m_lastError = errors.errorString();
    return false;
  }

  return deserialize(doc.object());
}

bool
JsonSerializable::deserialize(
    QJsonObject const &obj,
    QString const &key,
    QString &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      m_lastError = "Invalid value for property `" + key + "' (not a string)";
      return false;
    }

    value = obj[key].toString();
  }

  return true;
}

bool
JsonSerializable::deserialize(
    QJsonObject const &obj,
    QString const &key,
    QColor &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      m_lastError = "Invalid value for property `" + key + "' (not a string)";
      return false;
    }

    value.setNamedColor(obj[key].toString());
  }

  return true;
}


bool
JsonSerializable::deserialize(
    QJsonObject const &obj,
    QString const &key,
    std::list<std::string> &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isArray()) {
      m_lastError = "Invalid value for property `" + key + "' (not an array)";
      return false;
    }

    value.clear();
    for (auto el : obj[key].toArray())
      value.push_back(el.toString().toStdString());
  }

  return true;
}

bool
JsonSerializable::deserialize(
    QJsonObject const &obj,
    QString const &key,
    int &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isDouble()) {
      m_lastError = "Invalid value for property `" + key + "' (not a number)";
      return false;
    }

    value = obj[key].toInt();
  }

  return true;
}

bool
JsonSerializable::deserialize(
    QJsonObject const &obj,
    QString const &key,
    qreal &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isDouble()) {
      m_lastError = "Invalid value for property `" + key + "' (not a number)";
      return false;
    }

    value = obj[key].toDouble();
  }

  return true;
}

bool
JsonSerializable::deserialize(
    QJsonObject const &obj,
    QString const &key,
    bool &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isBool()) {
      m_lastError = "Invalid value for property `" + key + "' (not a boolean)";
      return false;
    }

    value = obj[key].toBool(value);
  }

  return true;
}

bool
JsonSerializable::deserialize(
    QJsonObject const &obj,
    QString const &key,
    std::map<std::string, std::string> &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isObject()) {
      m_lastError = "Invalid value for property `" + key + "' (not a JSON object)";
      return false;
    }

    value.clear();
    auto asObject = obj[key].toObject();
    for (auto p : asObject.keys()) {
      if (!asObject[p].isString()) {
        m_lastError = "Invalid entry `" + p + "` for dictionary `" + key + "' (not a string)";
        return false;
      }

      value[p.toStdString()] = asObject[p].toString().toStdString();
    }
  }

  return true;
}


bool
JsonSerializable::deserialize(
    QJsonObject const &obj,
    QString const &key,
    JsonSerializable &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isObject()) {
      m_lastError = "Invalid value for property `" + key + "' (not a JSON object)";
      return false;
    }

    value.loadDefaults();
    return value.deserialize(obj);
  }

  return true;
}
