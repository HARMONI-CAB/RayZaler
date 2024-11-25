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

#ifndef _LIBRZ_LOGGER_H
#define _LIBRZ_LOGGER_H

#include "Helpers.h"

namespace RZ {
  enum LogLevel {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO
  };

  class Logger {
      static Logger *m_logger;
      static int m_maxLevel;

    public:
      // To be implemented by all loggers
      virtual void logFunction(
        LogLevel level,
        std::string const &file,
        int line,
        std::string const &message) = 0;

      static void log(
        LogLevel level,
        std::string const &file,
        int line,
        const char *fmt, ...);

      static void setDefaultLogger(Logger *);
      static void setLogLevel(int max);
  };

  class StdErrLogger : public Logger {
      bool m_lineFeed = true;

    public:
      virtual void logFunction(
        LogLevel level,
        std::string const &file,
        int line,
        std::string const &message) override;
  };
}

#define RZError(fmt, arg...) \
  RZ::Logger::log(RZ::LOG_ERROR, __FILE__, __LINE__, fmt, ##arg)
#define RZWarning(fmt, arg...) \
  RZ::Logger::log(RZ::LOG_WARNING, __FILE__, __LINE__, fmt, ##arg)
#define RZInfo(fmt, arg...) \
  RZ::Logger::log(RZ::LOG_INFO, __FILE__, __LINE__, fmt, ##arg)

#endif // _LIBRZ_LOGGER_H
