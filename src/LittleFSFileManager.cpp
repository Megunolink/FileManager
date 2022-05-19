#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)

#include "LittleFSFileManager.h"
#include "FixedStringBuffer.h"

LittleFSFileManager::LittleFSFileManager(const char *pchRootPath)
    : SDFileManager(pchRootPath)
{
}

File LittleFSFileManager::OpenFile(const char *pchPath, bool bWriteable, bool bTruncate)
{
#if defined(ARDUINO_ARCH_ESP32)
  FixedStringBuffer<m_nMaxPathLength> PathBuffer;
  CompletePath(PathBuffer, pchPath);

  return LittleFS.open(PathBuffer.c_str(), bWriteable ? FILE_APPEND : FILE_READ, bTruncate);

#elif defined(ARDUINO_ARCH_ESP8266)
  return LittleFS.open(pchPath, bWriteable ? "w" : "r");

#endif
}

DFTResult LittleFSFileManager::DeleteFile(const char *pchPath)
{
  FixedStringBuffer<m_nMaxPathLength> PathBuffer;
  CompletePath(PathBuffer, pchPath);
  return LittleFS.remove(PathBuffer.c_str()) ? DFTResult::Ok : DFTResult::DeleteFileFailed;
}

DFTResult LittleFSFileManager::ClearAllFiles()
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
        if (DeleteFile(PathBuffer.c_str()) != DFTResult::Ok)
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

  return SDFileManager::ClearAllFiles();

#endif
}

#endif