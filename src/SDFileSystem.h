/* ********************************************************
 *  Wraps the Arduino SD file system.
 *  ******************************************************** */
#pragma once

#include "Sd.h"
#include "ArduinoTimer.h"
#include "IFileManagerFileSystem.h"
#include "FixedStringBuffer.h"
#include "Formatting.h"

#include "utility/FileSystemWrapper.h"

class SDFileSystemWrapper : public FileSystemWrapper
{
public:
  SDFileSystemWrapper(const char *pchRootPath = nullptr)
      : FileSystemWrapper(pchRootPath)
  {
  }

  virtual bool DeleteFile(const char *pchPath) override
  {
    FixedStringBuffer<m_nMaxPathLength> PathBuffer;
    CompletePath(PathBuffer, pchPath);
    return SD.remove(PathBuffer.c_str());
  }

protected:
  virtual bool FileExists(const char *pchPath) override { return SD.exists(pchPath); }

  virtual File OpenFile(const char *pchPath, bool bWriteable, bool bTruncate)
  {
    FixedStringBuffer<m_nMaxPathLength> FullPath;
    CompletePath(FullPath, pchPath);

#if defined(ARDUINO_ARCH_ESP32)
    // work around for bug: https://esp32.com/viewtopic.php?f=14&t=8060
    bool bCreate = bWriteable && !FileExists(FullPath.c_str());
    File hFile = SD.open(FullPath.c_str(), bWriteable ? FILE_APPEND : FILE_READ);
    if (hFile && bCreate)
    {
      // close and re-open so that file size is correct.
      hFile.close();
      hFile = SD.open(FullPath.c_str(), bWriteable ? FILE_APPEND : FILE_READ);
    }
    return hFile;

#else
    return SD.open(FullPath.c_str(), bWriteable ? FILE_WRITE : FILE_READ);
#endif
  }

};
