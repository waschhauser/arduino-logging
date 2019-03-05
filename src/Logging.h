#ifndef _LOGGING_h
#define _LOGGING_h

#include "ILogging.h"
#include "ILogger.h"

class Logging final : public ILogger {
public:
   static ILogger* GetLogger();
   void LogMessage(ILogging::EClassLogLevel eClassLogLevel, const char *format, ...);
   void DumpHexData(ILogging::EClassLogLevel eLogLevel, const char *pbyBuffer, unsigned long uLen);
   ILogging::EClassLogLevel GetLogLevel();
   void SetLogLevel(ILogging::EClassLogLevel eLogLevel);
private:
   static ILogger* instance;
   Logging();
   Logging(const Logging&);
   ~Logging();
   ILogging::EClassLogLevel eCurrentLogLevel = ILogging::eClassLogNotInitialized;
   class CGuard {
   public:
      ~CGuard();
   };
};

#endif