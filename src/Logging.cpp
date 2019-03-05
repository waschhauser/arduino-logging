#include <stdarg.h>

#if ARDUINO
#include "Arduino.h"
#include <ArduinoJson.h>
#include <FS.h>

#elif WIN32
#include <string.h>
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

#elif __linux__
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <limits.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <sys/time.h>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;
#endif

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
#if ARDUINO
   if (SPIFFS.begin()) {

      StaticJsonBuffer<1024> jsonBuffer;

      File configFile = SPIFFS.open("/logging.json", "r");

      if (configFile) {

         size_t size = configFile.size();

         std::unique_ptr<char[]> buf(new char[size]);

         configFile.readBytes(buf.get(), size);
         configFile.close();

         JsonObject& json = jsonBuffer.parseObject(buf.get());
         if (json.success()) {
            JsonObject& logging = json["logging"];
            if (logging.success()) {
               const char* loglevel = logging["loglevel"];
               if (loglevel) {
                  if (strcmp(loglevel, "disabled") == 0) {
                     eCurrentLogLevel = ILogging::eClassLogLevelDisabled;
                  }
                  else if (strcmp(loglevel, "fatal") == 0) {
                     eCurrentLogLevel = ILogging::eClassLogLevelFatal;
                  }
                  else if (strcmp(loglevel, "error") == 0) {
                     eCurrentLogLevel = ILogging::eClassLogLevelError;
                  }
                  else if (strcmp(loglevel, "warning") == 0) {
                     eCurrentLogLevel = ILogging::eClassLogLevelWarning;
                  }
                  else if (strcmp(loglevel, "basic") == 0) {
                     eCurrentLogLevel = ILogging::eClassLogLevelBasic;
                  }
                  else if (strcmp(loglevel, "info") == 0) {
                     eCurrentLogLevel = ILogging::eClassLogLevelInfo;
                  }
                  else if (strcmp(loglevel, "debug") == 0) {
                     eCurrentLogLevel = ILogging::eClassLogLevelDebug;
                  }
                  else if (strcmp(loglevel, "trace") == 0) {
                     eCurrentLogLevel = ILogging::eClassLogLevelTrace;
                  }
                  else {
                     LogMessage(ILogging::eClassLogLevelError, "%s : Invalid LogLevel", LOGGING_ID);
                  }
               }
            }
         }
         else {
            LogMessage(ILogging::eClassLogLevelError, "% : Failed to parse logging.json file", LOGGING_ID);
         }

      }
      SPIFFS.end();
   }

#elif WIN32 || __linux__

#if WIN32
   char path[MAX_PATH];
   HMODULE hModule = GetModuleHandleW(NULL);
   GetModuleFileNameA(hModule, path, MAX_PATH);
   *strrchr(path, '\\') = '\0';
   strcat_s(path, "\\logging.json");
#elif __linux__
   char path[PATH_MAX];
   ssize_t len = ::readlink("/proc/self/exe", path, sizeof(path) - 1);
   path[len] = '\0';
   *strrchr(path, '/') = '\0';
   strcat(path, "/logging.json");
#endif
   
   std::ifstream stream(path);
   if (stream.is_open()) {
      json j;
      try {
         stream >> j;
      }
      catch (...) {
         LogMessage(ILogging::eClassLogLevelError, "%s : Failed to parse logging.json file", LOGGING_ID);
      }

      stream.close();

      json loglevel = j["logging"]["loglevel"];
      if (loglevel.is_string())
      {
         std::string s = loglevel;
         if (s.compare("disabled") == 0) {
            eCurrentLogLevel = ILogging::eClassLogLevelDisabled;
         }
         else if (s.compare("fatal") == 0) {
            eCurrentLogLevel = ILogging::eClassLogLevelFatal;
         }
         else if (s.compare("error") == 0) {
            eCurrentLogLevel = ILogging::eClassLogLevelError;
         }
         else if (s.compare("warning") == 0) {
            eCurrentLogLevel = ILogging::eClassLogLevelWarning;
         }
         else if (s.compare("basic") == 0) {
            eCurrentLogLevel = ILogging::eClassLogLevelBasic;
         }
         else if (s.compare("info") == 0) {
            eCurrentLogLevel = ILogging::eClassLogLevelInfo;
         }
         else if (s.compare("debug") == 0) {
            eCurrentLogLevel = ILogging::eClassLogLevelDebug;
         }
         else if (s.compare("trace") == 0) {
            eCurrentLogLevel = ILogging::eClassLogLevelTrace;
         }
         else {
            LogMessage(ILogging::eClassLogLevelError, "%s : Invalid LogLevel", LOGGING_ID);
         }
      }

   }

#endif
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
#if ARDUINO
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
   switch (eLogLevel)
   {
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

   switch (eLogLevel)
   {
   case ILogging::eClassLogLevelFatal:
   case ILogging::eClassLogLevelError:
   case ILogging::eClassLogLevelWarning:
      Serial.write("\033[37m");
      break;
   default:
      break;
   }

   Serial.println();
#elif WIN32

   m_Mutex.lock();
   HANDLE hConsole;
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   WORD currentConsoleAttr;
   hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
   if (hConsole != NULL) {
      GetConsoleScreenBufferInfo(hConsole, &csbi);
      currentConsoleAttr = csbi.wAttributes;
      switch (eLogLevel)
      {
      case ILogging::eClassLogLevelFatal:
      case ILogging::eClassLogLevelError:
         SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
         break;
      case ILogging::eClassLogLevelWarning:
         SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
         break;
      default:
         break;
      }
   }

   switch (eLogLevel)
   {
   case ILogging::eClassLogNotInitialized:
      break;
   case ILogging::eClassLogLevelDisabled:
      break;
   case ILogging::eClassLogLevelFatal:
      printf("FATAL");
      break;
   case ILogging::eClassLogLevelError:
      printf("ERROR");
      break;
   case ILogging::eClassLogLevelWarning:
      printf("WARN ");
      break;
   case ILogging::eClassLogLevelBasic:
      printf("BASIC");
      break;
   case ILogging::eClassLogLevelInfo:
      printf("INFO ");
      break;
   case ILogging::eClassLogLevelDebug:
      printf("DEBUG");
      break;
   case ILogging::eClassLogLevelTrace:
      printf("TRACE");
      break;
   default:
      break;
   }

   SYSTEMTIME localTime;
   GetLocalTime(&localTime);

   printf(" %02d:%02d:%02d.%03d : ", localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);

   printf("%s", buffer);
   printf("\n");

   if (hConsole != NULL) {
      SetConsoleTextAttribute(hConsole, currentConsoleAttr);
   }

   m_Mutex.unlock();

#elif __linux__
   m_Mutex.lock();
   switch (eLogLevel) {
   case ILogging::eClassLogLevelFatal:
   case ILogging::eClassLogLevelError:
      printf("\033[31m");
      break;
   case ILogging::eClassLogLevelWarning:
      printf("\033[33m");
      break;
   default:
      break;
   }
   switch (eLogLevel)
   {
   case ILogging::eClassLogNotInitialized:
      break;
   case ILogging::eClassLogLevelDisabled:
      break;
   case ILogging::eClassLogLevelFatal:
      printf("FATAL");
      break;
   case ILogging::eClassLogLevelError:
      printf("ERROR");
      break;
   case ILogging::eClassLogLevelWarning:
      printf("WARN ");
      break;
   case ILogging::eClassLogLevelBasic:
      printf("BASIC");
      break;
   case ILogging::eClassLogLevelInfo:
      printf("INFO ");
      break;
   case ILogging::eClassLogLevelDebug:
      printf("DEBUG");
      break;
   case ILogging::eClassLogLevelTrace:
      printf("TRACE");
      break;
   default:
      break;
   }

   timeval curTime;
   gettimeofday(&curTime, NULL);
   int milli = curTime.tv_usec / 1000;

   char buffer2[10];
   strftime(buffer2, 10, " %H:%M:%S", localtime(&curTime.tv_sec));

   printf(" %s.%03d : ", buffer2, milli);

   printf("%s", buffer);

   switch (eLogLevel)
   {
   case ILogging::eClassLogLevelFatal:
   case ILogging::eClassLogLevelError:
   case ILogging::eClassLogLevelWarning:
      printf("\033[37m");
      break;
   default:
      break;
   }

   printf("\n");
   m_Mutex.unlock();

#endif
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