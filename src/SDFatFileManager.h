/* ********************************************************
 *  Wraps the Arduino SDFat file system.
 *  ******************************************************** */
#pragma once

#include "SdFat.h"

#include "utility/FileManager.h"
#include "utility/FileSystemWrapper.h"

class SdFatFileManager : protected FileSystemWrapper<FsFile>, public MLP::FileManager
{
private:
  SdFat &m_rFileSystem;

public:
  SdFatFileManager(SdFat &rFileSystem, const char *pchRootPath = nullptr)
    : FileSystemWrapper<FsFile>(pchRootPath)
    , MLP::FileManager(*(static_cast<FileSystemWrapper<FsFile> *>(this)))
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

    virtual FsFile OpenFile(const char *pchPath, bool bWriteable, bool bCreate)
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

    virtual const char* GetFilename(FsFile hFile) override 
    { 
      static char achFilenameBuffer[13];
      hFile.getName(achFilenameBuffer, sizeof(achFilenameBuffer));
      return achFilenameBuffer;
    };

};
