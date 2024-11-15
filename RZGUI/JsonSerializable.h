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

#ifndef JSONSERIALIZABLE_H
#define JSONSERIALIZABLE_H

#include <QString>
#include <QColor>
#include <QJsonObject>
#include <QByteArray>
#include <cstdarg>
#include <QJsonArray>

class JsonSerializable {
  QString m_lastError;

protected:
  void setLastError(QString const &);
  void errorSprintf(const char *fmt, va_list);

  bool deserialize(QJsonObject const &, QString const &, int &);
  bool deserialize(QJsonObject const &, QString const &, bool &);
  bool deserialize(QJsonObject const &, QString const &, qreal &);
  bool deserialize(QJsonObject const &, QString const &, QString &);
  bool deserialize(QJsonObject const &, QString const &, QColor &);

  bool deserialize(QJsonObject const &, QString const &, std::list<std::string> &);
  bool deserialize(QJsonObject const &, QString const &, std::map<std::string, std::string> &);

  bool deserialize(QJsonObject const &, QString const &, JsonSerializable &);

  template <class T>
  inline bool
  deserialize(
      QJsonObject const &obj,
      QString const &key,
      std::list<T> &value)
  {
    if (obj.contains(key)) {
      if (!obj[key].isArray()) {
        m_lastError = "Invalid value for property `" + key + "' (not an array)";
        return false;
      }

      value.clear();
      for (auto el : obj[key].toArray()) {
        if (!el.isObject()) {
          m_lastError = "Element inside of `" + key + "' is not an object";
          return false;
        }

        T newVal;
        if (!newVal.deserialize(el.toObject()))
          return false;
        value.push_back(newVal);
      }
    }

    return true;
  }

public:
  JsonSerializable() = default;
  JsonSerializable(const JsonSerializable &) = default;
  JsonSerializable(JsonSerializable &&) = default;
  JsonSerializable& operator=(const JsonSerializable &) = default;
  JsonSerializable& operator=(JsonSerializable &&) = default;

  virtual ~JsonSerializable();

  bool deserialize(QByteArray const &);

  virtual QJsonObject serialize() const = 0;
  virtual bool deserialize(QJsonObject const &) = 0;
  virtual void loadDefaults() = 0;

  QString lastError() const;
};

#endif // JSONSERIALIZABLE_H
