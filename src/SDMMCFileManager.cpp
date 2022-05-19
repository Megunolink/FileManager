#if defined(ARDUINO_ARCH_ESP32)

#include "SDMMCFileManager.h"
#include "FixedStringBuffer.h"

SDMMCFileManager::SDMMCFileManager(const char *pchRootPath)
    : SDFileManager(pchRootPath)
{
}

File SDMMCFileManager::OpenFile(const char *pchPath, bool bWriteable, bool bTruncate)
{
  if (bTruncate)
  {
    DeleteFile(pchPath);
  }

  FixedStringBuffer<m_nMaxPathLength> FullPath;
  CompletePath(FullPath, pchPath);

#if defined(ARDUINO_ARCH_ESP32)
  // work around for bug: https://esp32.com/viewtopic.php?f=14&t=8060
  bool bCreate = bWriteable && !FileExists(FullPath.c_str());
  File hFile = SD_MMC.open(FullPath.c_str(), bWriteable ? FILE_APPEND : FILE_READ);
  if (hFile && bCreate)
  {
    // close and re-open so that file size is correct.
    hFile.close();
    hFile = SD_MMC.open(FullPath.c_str(), bWriteable ? FILE_APPEND : FILE_READ);
  }
  return hFile;

#else
  File hFile = SD_MMC.open(FullPath.c_str(), bWriteable ? FILE_APPEND : FILE_READ);
#endif


  return hFile;
}

DFTResult SDMMCFileManager::DeleteFile(const char *pchPath)
{
  FixedStringBuffer<m_nMaxPathLength> PathBuffer;
  CompletePath(PathBuffer, pchPath);
  return SD_MMC.remove(PathBuffer.c_str()) ? DFTResult::Ok : DFTResult::DeleteFileFailed;
}

#endif