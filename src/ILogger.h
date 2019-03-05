#ifndef _ILOGGER_h
#define _ILOGGER_h

#include "ILogging.h"

class ILogger {
public:
   void virtual LogMessage(ILogging::EClassLogLevel eClassLogLevel, const char *format, ...) = 0;
   void virtual DumpHexData(ILogging::EClassLogLevel eLogLevel, const char *pbyBuffer, unsigned long uLen) = 0;
   ILogging::EClassLogLevel virtual GetLogLevel() = 0;
   void virtual SetLogLevel(ILogging::EClassLogLevel eLogLevel) = 0;
};

#endif