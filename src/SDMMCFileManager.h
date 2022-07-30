/* ********************************************************
 *  Implements file manager for ESP32 accessing memory
 *  cards through the SDMMC driver
 *  ******************************************************** */
#pragma once

#if defined(ARDUINO_ARCH_ESP32)

#include "FS.h"
#include "SD_MMC.h"
#include "SDFileManager.h"

class SDMMCFileManager
{
};

#include "utility/FileSystemWrapper.h"

class SDMMCFileSystemWrapper : public FileSystemWrapper
{
public:
  SDMMCFileSystemWrapper(const char *pchRootPath = nullptr)
      : FileSystemWrapper(pchRootPath)
  {
  }

  virtual bool DeleteFile(const char *pchPath) override
  {
    FixedStringBuffer<m_nMaxPathLength> PathBuffer;
    CompletePath(PathBuffer, pchPath);
    return SD_MMC.remove(PathBuffer.c_str());
  }

protected:

  virtual bool FileExists(const char *pchPath) override { return SD.exists(pchPath); }

  virtual File OpenFile(const char *pchPath, bool bWriteable, bool bTruncate) override
  {
    FixedStringBuffer<m_nMaxPathLength> FullPath;
    CompletePath(FullPath, pchPath);

    if (bTruncate)
    {
      SD_MMC.remove(FullPath.c_str());
    }

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
};

#endif
