#ifndef _ILOGGING_h
#define _ILOGGING_h

class ILogging {
public:
   enum EClassLogLevel {
      eClassLogNotInitialized = -1,
      eClassLogLevelDisabled = 0,
      eClassLogLevelFatal = 1,
      eClassLogLevelError = 2,
      eClassLogLevelWarning = 3,
      eClassLogLevelBasic = 4,
      eClassLogLevelInfo = 5,
      eClassLogLevelDebug = 6,
      eClassLogLevelTrace = 7,
   };
};

#endif