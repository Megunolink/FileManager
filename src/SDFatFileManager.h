/* ********************************************************
 *  Wraps the Arduino SDFat file system.
 *  ******************************************************** */
#pragma once

#include "SdFat.h"

#include "utility/FileManager.h"
#include "utility/FileSystemWrapper.h"

class SdFat32FileManager : protected FileSystemWrapper<File32>, public MLP::FileManager
{
private:
  SdFat32 &m_rFileSystem;

public:
  SdFat32FileManager(SdFat32 &rFileSystem, const char *pchRootPath = nullptr)
    : FileSystemWrapper<File32>(pchRootPath)
    , MLP::FileManager(*(static_cast<FileSystemWrapper<File32> *>(this)))
    , m_rFileSystem(rFileSystem)
    {
    }

    using FileSystemWrapper::Process;

  protected:
    virtual bool RemoveFileAtPath(const char *pchFullPath) override
    {
      return m_rFileSystem.remove(pchFullPath);
    }

    virtual bool FileExists(const char *pchPath) override
    {
      return m_rFileSystem.exists(pchPath);
    }

    virtual File32 OpenFile(const char *pchPath, bool bWriteable, bool bCreate)
    {
      oflag_t Flags;
      if (bCreate)
      {
        Flags = O_RDWR | O_CREAT;
      }
      else if (bWriteable)
      {
        Flags = O_RDWR | O_AT_END;
      }
      else
      {
        Flags = O_RDONLY;
      }

      return m_rFileSystem.open(pchPath, Flags);
    }

    virtual const char* GetFilename(File32 hFile) override 
    { 
      static char achFilenameBuffer[13];
      hFile.getSFN(achFilenameBuffer, sizeof(achFilenameBuffer));
      return achFilenameBuffer;
    };

};
