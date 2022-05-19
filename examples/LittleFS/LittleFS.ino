#include "MegunoLink.h"
#include "CommandHandler.h"
#include "LittleFS.h"
#include "LittleFSFileManager.h"

CommandHandler<10, 60> Cmds;
LittleFSFileManager FileManager;

void InitLittleFS()
{
#if defined(ARDUINO_ARCH_ESP32)
  bool bInitialized = LittleFS.begin(true);

#elif defined(ARDUINO_ARCH_ESP8266)

  bool bInitialized = LittleFS.begin();
#else
  bool bInitialized = false;
#endif

  if (bInitialized)
  {
    Serial.println(F("Little FS ready"));
  }
  else
  {
    Serial.println(F("Failed to initialize LittleFS"));
  }
}

void setup()
{
  Serial.begin(500000);
  Serial.println();
  Serial.println(F("LittleFS MegunoLink File Manager Tester"));
  Serial.println(F("======================================="));

  Cmds.AddModule(&FileManager);
  InitLittleFS();
}

void loop()
{
  Cmds.Process();
  FileManager.Process();
}
