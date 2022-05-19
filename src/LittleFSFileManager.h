/* ********************************************************
 *  Implements file manager for ESP32 and ESP8266 LittleFS
 *  file system. 
 *  ******************************************************** */
#pragma once

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)

#include "SDFileManager.h"
#include "ArduinoTimer.h"

#include "FS.h"
#include "LittleFS.h"

class LittleFSFileManager : public SDFileManager
{
public:
  LittleFSFileManager(const char* pchRootPath = nullptr);

protected:
  virtual DFTResult DeleteFile(const char *pchPath) override;
  virtual DFTResult ClearAllFiles() override;

protected:
  virtual File OpenFile(const char* pchPath, bool bWriteable, bool bTruncate) override;

};

#endif
