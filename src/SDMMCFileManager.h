/* ********************************************************
 *  Implements file manager for ESP32 accessing memory
 *  cards through the SDMMC driver
 *  ******************************************************** */
#pragma once

#if defined(ARDUINO_ARCH_ESP32)

#include "FS.h"
#include "SD_MMC.h"

#include "utility/FileManager.h"
#include "utility/FileSystemWrapper.h"

class SDMMCFileManager : protected FileSystemWrapper, public MLP::FileManager
{
public:
  SDMMCFileManager(const char *pchRootPath = nullptr)
    : FileSystemWrapper(pchRootPath), MLP::FileManager(*(static_cast<FileSystemWrapper *>(this)))
  {
  }

  using FileSystemWrapper::Process;

protected:
  virtual bool DeleteFile(const char *pchPath) override
  {
    return SD_MMC.remove(pchPath);
  }

  virtual bool FileExists(const char *pchPath) override
  {
    return SD_MMC.exists(pchPath);
  }

  virtual File OpenFile(const char *pchFullPath, bool bWriteable, bool bTruncate)
  {
#if defined(ARDUINO_ARCH_ESP32)
    // work around for bug: https://esp32.com/viewtopic.php?f=14&t=8060
    bool bCreate = bWriteable && !FileExists(pchFullPath);
    File hFile = SD_MMC.open(pchFullPath, bWriteable ? FILE_APPEND : FILE_READ);
    if (hFile && bCreate)
    {
      // close and re-open so that file size is correct.
      hFile.close();
      hFile = SD_MMC.open(pchFullPath, bWriteable ? FILE_APPEND : FILE_READ);
    }
    return hFile;

#else
    return SD_MMC.open(pchFullPath, bWriteable ? FILE_WRITE : FILE_READ);
#endif
  }

};

#endif
