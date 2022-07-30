/* ********************************************************
 *  Implements file manager for ESP32 and ESP8266 LittleFS
 *  file system. 
 *  ******************************************************** */
#pragma once

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)

#include "FS.h"
#include "LittleFS.h"

#include "utility/FileManager.h"
#include "utility/FileSystemWrapper.h"
#include "ArduinoTimer.h"

class LittleFSFileManager : protected FileSystemWrapper, public MLP::FileManager
{
public:
  LittleFSFileManager(const char* pchRootPath = nullptr)
    : FileSystemWrapper(pchRootPath), MLP::FileManager(*(static_cast<FileSystemWrapper *>(this)))
  {
  }

  using FileSystemWrapper::Process;

protected:
  virtual bool DeleteFile(const char *pchPath) override
  {
    return LittleFS.remove(pchPath);
  }

  virtual bool FileExists(const char *pchPath) override
  {
    return LittleFS.exists(pchPath);
  }

  virtual File OpenFile(const char *pchFullPath, bool bWriteable, bool bTruncate)
  {
#if defined(ARDUINO_ARCH_ESP32)

  return LittleFS.open(pchFullPath, bWriteable ? FILE_APPEND : FILE_READ, bTruncate);

#elif defined(ARDUINO_ARCH_ESP8266)

  return LittleFS.open(pchFullPath, bWriteable ? "w" : "r");

#endif
  }

  virtual DFTResult ClearAllFiles() override
  {
#if defined(ARDUINO_ARCH_ESP32)
    File hRoot = OpenFile(m_achRootPath, false, false);
    if (!hRoot)
    {
      Serial.println(F("Failed to open root path"));
      return DFTResult::BadRoot;
    }

    DFTResult Result = DFTResult::Ok;
    if (hRoot.isDirectory())
    {
      FixedStringBuffer<m_nMaxPathLength> PathBuffer;
      while (File hFile = hRoot.openNextFile())
      {
        if (!hFile.isDirectory())
        {
          PathBuffer.begin();
          PathBuffer.print(hFile.path());
          hFile.close();
          if (LittleFS.remove(PathBuffer.c_str()))
          {
            Result = DFTResult::DeleteFileFailed;
          }
        }
        else
        {
          hFile.close();
        }
      }
    }
    else
    {
      Serial.println(F("Root is not a directory"));
      Result = DFTResult::BadRoot;
    }

    hRoot.close();
    return Result;
#else

    return FileSystemWrapper::ClearAllFiles();

#endif
}

};

#endif
