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
  };
}

#define RZError(fmt, arg...) \
  RZ::Logger::log(RZ::LOG_ERROR, __FILE__, __LINE__, fmt, ##arg)
#define RZWarning(fmt, arg...) \
  RZ::Logger::log(RZ::LOG_WARNING, __FILE__, __LINE__, fmt, ##arg)
#define RZInfo(fmt, arg...) \
  RZ::Logger::log(RZ::LOG_INFO, __FILE__, __LINE__, fmt, ##arg)

#endif // _LIBRZ_LOGGER_H
