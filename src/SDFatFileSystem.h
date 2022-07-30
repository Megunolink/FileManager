/* ********************************************************
 *  Wraps the Arduino SDFat file system.
 *  ******************************************************** */
#pragma once

#include "SdFat.h"

class SdFat32FileManager : protected FileSystemWrapper, public MLP::FileManager
{
private:
  SdFat32 &m_rFileSystem;

public:
  SdFat32FileSystem(SdFat32 &rFileSystem, const char *pchRootPath = nullptr);
    : FileSystemWrapper(pchRootPath), MLP::FileManager(*(static_cast<FileSystemWrapper *>(this)))
    {
    }

    using FileSystemWrapper::Process;

  protected:
    virtual bool DeleteFile(const char *pchPath) override
    {
      return m_rFileSystem.remove(pchPath);
    }

    virtual bool FileExists(const char *pchPath) override
    {
      return m_rFileSystem.exists(pchPath);
    }

    virtual Stream OpenFile(const char *pchPath, bool bWriteable, bool bTruncate)
    {
      oflag_t Flags;
      if (bTruncate)
      {
        Flags = O_TRUNC | O_CREAT;
      }
      else if (bWriteable)
      {
        Flags = O_RDWR;
      }
      else
      {
        Flags = O_RDONLY;
      }

      return m_rFileSystem.open(pchPath, Flags);
    }
};
