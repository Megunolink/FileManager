/* ********************************************************
 *  Wraps the Arduino SDFat file system.
 *  ******************************************************** */
#pragma once

#include "SdFat.h"

#include "utility/FileManager.h"
#include "utility/FileSystemWrapper.h"

#if SDFAT_FILE_TYPE == 1
typedef File32 SdfmFile;
#elif SDFAT_FILE_TYPE == 2
typedef ExFile SdfmFile;
#elif SDFAT_FILE_TYPE == 3
typedef FsFile SdfmFile;
#else  // SDFAT_FILE_TYPE
#error Invalid SDFAT_FILE_TYPE
#endif  // SDFAT_FILE_TYPE


class SdFatFileManager : protected FileSystemWrapper<SdfmFile>, public MLP::FileManager
{
private:
  SdFat &m_rFileSystem;

public:
  SdFatFileManager(SdFat &rFileSystem, const char *pchRootPath = nullptr)
    : FileSystemWrapper<SdfmFile>(pchRootPath)
    , MLP::FileManager(*(static_cast<FileSystemWrapper<SdfmFile> *>(this)))
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

    virtual SdfmFile OpenFile(const char *pchPath, bool bWriteable, bool bCreate)
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

    virtual const char* GetFilename(SdfmFile hFile) override 
    { 
      static char achFilenameBuffer[NFileManager::MaxFilenameLength];
      hFile.getName(achFilenameBuffer, sizeof(achFilenameBuffer));
      return achFilenameBuffer;
    };

};
