#include <stdarg.h>

#include "Arduino.h"

#include "ILogging.h"
#include "Logging.h"

using namespace std;

Logging::CGuard::~CGuard() {
   if (NULL != Logging::instance) {
      delete Logging::instance;
      Logging::instance = NULL;
   }
}

Logging::~Logging() {
}

ILogger* Logging::GetLogger() {
   static CGuard g;
   if (!instance) {
      instance = new Logging();
   }
   return instance;
}

Logging::Logging() {
   eCurrentLogLevel = ILogging::eClassLogLevelInfo;
}

void Logging::LogMessage(ILogging::EClassLogLevel eLogLevel, const char * format, ...) {
   if ((eLogLevel > eCurrentLogLevel) || (eCurrentLogLevel < ILogging::eClassLogLevelFatal)) {
      return;
   }
   
   va_list arg;
   va_start(arg, format);
   char temp[64];
   char* buffer = temp;
   size_t len = vsnprintf(temp, sizeof(temp), format, arg);
   va_end(arg);
   
   if (len > sizeof(temp) - 1) {
      buffer = new char[len + 1];
      if (!buffer) {
         return;
      }
      va_start(arg, format);
      vsnprintf(buffer, len + 1, format, arg);
      va_end(arg);
   }

   switch (eLogLevel) {
   case ILogging::eClassLogLevelFatal:
   case ILogging::eClassLogLevelError:
      Serial.write("\033[31m");
      break;
   case ILogging::eClassLogLevelWarning:
      Serial.write("\033[33m");
      break;
   default:
      break;
   }
   
   switch (eLogLevel) {
   case ILogging::eClassLogNotInitialized:
      break;
   case ILogging::eClassLogLevelDisabled:
      break;
   case ILogging::eClassLogLevelFatal:
      Serial.write("FATAL");
      break;
   case ILogging::eClassLogLevelError:
      Serial.write("ERROR");
      break;
   case ILogging::eClassLogLevelWarning:
      Serial.write("WARN ");
      break;
   case ILogging::eClassLogLevelBasic:
      Serial.write("BASIC");
      break;
   case ILogging::eClassLogLevelInfo:
      Serial.write("INFO ");
      break;
   case ILogging::eClassLogLevelDebug:
      Serial.write("DEBUG");
      break;
   case ILogging::eClassLogLevelTrace:
      Serial.write("TRACE");
      break;
   default:
      break;
   }
   
   Serial.write(" : ");
   Serial.write((const uint8_t*)buffer, len);

   switch (eLogLevel) {
   case ILogging::eClassLogLevelFatal:
   case ILogging::eClassLogLevelError:
   case ILogging::eClassLogLevelWarning:
      Serial.write("\033[37m");
      break;
   default:
      break;
   }

   Serial.println();

   if (buffer != temp) {
      delete[] buffer;
   }
}

void Logging::DumpHexData(ILogging::EClassLogLevel eLogLevel, const char * pbyBuffer, unsigned long uLen) {
	
   if ((eLogLevel > eCurrentLogLevel) || (eCurrentLogLevel < ILogging::eClassLogLevelFatal)) {
      return;
   }

   char szBuffer[69], temp[3], temp2[19];
   unsigned char x = 0;
   unsigned int i;
   unsigned int l;

   strcpy(szBuffer, " ");
   strcpy(temp2, "  ");

   for (i = 0; i < uLen; i++) {

      if (x == 8) {
         strcat(szBuffer, " ");
      }

      sprintf(temp, " %02X", pbyBuffer[i]);
      strcat(szBuffer, temp);
	  
      if ((pbyBuffer[i] > 0x1F) && (pbyBuffer[i] < 0x7F)) {
         temp2[x + 2] = pbyBuffer[i];
      }
      else {
         temp2[x + 2] = '.';
      }

      temp2[x + 3] = '\0';

      x++;

      if (x == 16) {
         x = 0;
         strcat(szBuffer, temp2);
         LogMessage(eLogLevel, "%s", szBuffer);
         strcpy(szBuffer, " ");
         strcpy(temp2, "  ");
      }
   }

   if (x > 0) {

      if (x < 9) {
         strcat(szBuffer, " ");
      }

      for (i = x; i < 16; i++) {
         strcat(szBuffer, "   ");
      }

      strcat(szBuffer, temp2);
      LogMessage(eLogLevel, "%s", szBuffer);
   }
}

ILogging::EClassLogLevel Logging::GetLogLevel() {
   return eCurrentLogLevel;
}

void Logging::SetLogLevel(ILogging::EClassLogLevel eLogLevel) {
   if (eLogLevel == eCurrentLogLevel) {
      return;
   }
   eCurrentLogLevel = eLogLevel;
}

ILogger* Logging::instance = 0;