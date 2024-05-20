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

#include <Logger.h>
#include <cstdio>

using namespace RZ;

Logger *Logger::m_logger = nullptr;
int Logger::m_maxLevel = LOG_INFO;

void
Logger::setDefaultLogger(Logger *logger)
{
  m_logger = logger;
}

void
Logger::setLogLevel(int max)
{
  m_maxLevel = max;
}

void
Logger::log(
        LogLevel level,
        std::string const &file,
        int line,
        const char *fmt,
        ...)
{
  if (m_logger != nullptr && level <= m_maxLevel) {
    std::va_list ap;
    va_start(ap, fmt);
    m_logger->logFunction(level, file, line, string_vprintf(fmt, ap));
    va_end(ap);
  }
}

void
StdErrLogger::logFunction(
    LogLevel level,
    std::string const &file,
    int line,
    std::string const &message)
{
  if (m_lineFeed) {
    switch (level) {
      case LOG_ERROR:
        fprintf(stderr, "RayZaler error: ");
        break;
      
      case LOG_WARNING:
        fprintf(stderr, "RayZaler warning: ");
        break;

      case LOG_INFO:
        fprintf(stderr, "RayZaler info: ");
        break;
    }
  }

  std::string copy = message;

  if (!copy.empty()) {
    if (copy[copy.size() - 1] == '\n') {
      copy.resize(copy.size() - 1);
      m_lineFeed = true;
    }

    fprintf(stderr, "%s", copy.c_str());

    if (m_lineFeed)
      fprintf(stderr, " (%s:%d)\n", file.c_str(), line);
  }
}
