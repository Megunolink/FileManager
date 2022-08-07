/* ********************************************************
 *  Provides a general wrapper for file systems that use the
 *  standard File class.
 *  ******************************************************** */
#pragma once

#include "ArduinoTimer.h"
#include "../IFileManagerFileSystem.h"
#include "FixedStringBuffer.h"
#include "Formatting.h"

template <typename TFile>
class FileSystemWrapper : public IFileManagerFileSystem
{
protected:
  // File we are currently working to send/receive. Kept
  // open to improve performance. File is closed when
  // MegunoLink reports transfer is complete and/or after
  // time-out.
  TFile m_CachedFile;

  // True if the cached file is opened for writing; false if read-only.
  bool m_bCachedIsWriteable;

  // Time since cached file was last used. Closed after not used for a while.
  ArduinoTimer m_tmrCloseCache;

  // Maximum time to keep the cached file open if it isn't being used.
  static const int m_nCacheTimeout = 3000; // ms.

  // Maximum number of characters for root path (including null terminator).
  static const int m_nMaxRootPath = 9;

  // Maximum length of file path combined with root path.
  static const int m_nMaxPathLength = m_nMaxRootPath + 15;

  // Root path for the folder we manage.
  char m_achRootPath[m_nMaxRootPath];

public:
  FileSystemWrapper(const char *pchRootPath = nullptr)
  {
    if (pchRootPath == nullptr)
    {
      m_achRootPath[0] = '/';
      m_achRootPath[1] = '\0';
    }
    else
    {
      strncpy(m_achRootPath, pchRootPath, sizeof(m_achRootPath));
      if (m_achRootPath[sizeof(m_achRootPath) - 1] != '\0')
      {
        m_achRootPath[sizeof(m_achRootPath) - 1] = '\0';
      }
    }
  }

  virtual void Process()
  {
    if (m_CachedFile && m_tmrCloseCache.TimePassed_Milliseconds(m_nCacheTimeout))
    {
      m_CachedFile.close();
    }
  }

  virtual DFTResult ListFiles(DeviceFileTransfer &dft) override
  {
    TFile hRoot = OpenFile(m_achRootPath, false, false);
    if (!hRoot)
    {
      Serial.print(F("Bad root path: "));
      Serial.println(m_achRootPath);
      return DFTResult::BadRoot;
    }

    DFTResult Result = DFTResult::Ok;
    if (hRoot.isDirectory())
    {
      while (TFile hFile = hRoot.openNextFile())
      {
        if (!hFile.isDirectory())
        {
          dft.SendFileInfo(GetFilename(hFile), hFile.size(), GetLastWriteTime(hFile));
        }
        hFile.close();
      }
    }
    else
    {
      Serial.println(F("Bad root dir"));
      Result = DFTResult::BadRoot;
    }

    hRoot.close();
    return Result;
  }

  virtual DFTResult ReceiveFileContent(const char *pchRelativePath, uint32_t uFirstByte, const char *pchBase64Data, DeviceFileTransfer &dft) override
  {
    FixedStringBuffer<m_nMaxPathLength> FullPath;
    CompletePath(FullPath, pchRelativePath);

    bool bCreateNew = uFirstByte == 0;
    if (bCreateNew)
    {
      if (m_CachedFile)
      {
        m_CachedFile.close();
      }

      if (FileExists(FullPath.c_str()))
      {
        RemoveFileAtPath(FullPath.c_str());
      }
    }

    TFile& hFile = OpenFileCached(FullPath.c_str(), true, bCreateNew);
    if (hFile)
    {
      if (uFirstByte == (uint32_t)hFile.size())
      {
        int nWritten = DecodeFromBase64(hFile, pchBase64Data);
        dft.FileReceiveResult(pchRelativePath, uFirstByte, nWritten, nWritten == DECODE_BAD_DATA ? DFTResult::BadData : DFTResult::Ok);
#if defined(ARDUINO_ARCH_ESP32)
        hFile.flush(); // Without flush, hFile.size() reports the wrong value on ESP32.
#endif
        return DFTResult::Ok;
      }
      else
      {
#if 0
        Serial.print(F("Bad addr. Expected: "));
        Serial.print((uint32_t)hFile.size());
        Serial.print(F(", got: "));
        Serial.println(uFirstByte);
#endif
        dft.FileReceiveResult(pchRelativePath, uFirstByte, 0, DFTResult::BadDataBlockAddress);
        return DFTResult::BadDataBlockAddress;
      }
    }
    else
    {
      dft.FileReceiveResult(pchRelativePath, uFirstByte, 0, DFTResult::FileOpenFailed);
      return DFTResult::FileOpenFailed;
    }
  }

  virtual void TransferComplete(const char *pchRelativePath) override
  {
    CloseCachedFile(pchRelativePath);
  }

  virtual DFTResult SendFileContent(const char *pchRelativePath, uint32_t uFirstByte, uint32_t uBlockSize, DeviceFileTransfer &dft) override
  {
    FixedStringBuffer<m_nMaxPathLength> FullPath;
    CompletePath(FullPath, pchRelativePath);

    TFile& hFile = OpenFileCached(FullPath.c_str(), false, false);
    if (hFile)
    {
      if (hFile.seek(uFirstByte))
      {
        dft.SendFileBytes(pchRelativePath, hFile, uFirstByte, uBlockSize);
        return DFTResult::Ok;
      }

      dft.SendFileBytes(pchRelativePath, uFirstByte, DFTResult::SeekFailed);
      return DFTResult::SeekFailed;
    }

    dft.SendFileBytes(pchRelativePath, uFirstByte, DFTResult::FileOpenFailed);
    return DFTResult::FileOpenFailed;
  }

  virtual DFTResult ClearAllFiles() override
  {
    TFile hRoot = OpenFile(m_achRootPath, false, false);
    if (!hRoot)
    {
      return DFTResult::BadRoot;
    }

    DFTResult Result = DFTResult::Ok;
    FixedStringBuffer<m_nMaxPathLength> FullPath;
    if (hRoot.isDirectory())
    {
      while (TFile hFile = hRoot.openNextFile())
      {
        if (!hFile.isDirectory())
        {
          CompletePath(FullPath, GetFilename(hFile));
          if (!RemoveFileAtPath(FullPath.c_str()))
          {
            Result = DFTResult::DeleteFileFailed;
          }
        }
        hFile.close();
      }
    }
    else
    {
      Result = DFTResult::BadRoot;
    }

    hRoot.close();
    return Result;
  }

protected:
  virtual bool RemoveFileAtPath(const char* pchFullPath) = 0;
  virtual bool FileExists(const char *pchFullPath) = 0;
  virtual TFile OpenFile(const char *pchFullPath, bool bWriteable, bool bTruncate) = 0;

  virtual bool DeleteFile(const char*pchFilename) override
  {
    FixedStringBuffer<m_nMaxPathLength> FullPath;
    CompletePath(FullPath, pchFilename);
    return RemoveFileAtPath(FullPath.c_str());
  }

  virtual const char *GetFilename(TFile hFile)
  {
    // SdFat library has special accessors for retrieving filename.
    // Must override in derived classes.
#if !defined(SD_FAT_VERSION)
    return hFile.name();
#else
    return nullptr;
#endif
  };

  TFile &OpenFileCached(const char *pchFullPath, bool bWriteable, bool bCreate)
  {
    if (bWriteable && bCreate && m_CachedFile)
    {
      m_CachedFile.close();
    }
    else if (m_CachedFile)
    {
      FixedStringBuffer<m_nMaxPathLength> FullCacheFilePath;
      CompletePath(FullCacheFilePath, GetFilename(m_CachedFile));
      if (strcmp(pchFullPath, FullCacheFilePath.c_str()) == 0 && m_bCachedIsWriteable == bWriteable)
      {
        m_tmrCloseCache.Reset();
        return m_CachedFile;
      }
      else
      {
        m_CachedFile.close();
      }
    }

    m_CachedFile = OpenFile(pchFullPath, bWriteable, bCreate);
    m_tmrCloseCache.Reset();
    m_bCachedIsWriteable = bWriteable;

    return m_CachedFile;
  }

  void CloseCachedFile(const char *pchRelativePath)
  {
    if (m_CachedFile)
    {
      FixedStringBuffer<m_nMaxPathLength> FullPath;
      CompletePath(FullPath, pchRelativePath);

      FixedStringBuffer<m_nMaxPathLength> FullCacheFilePath;
      CompletePath(FullCacheFilePath, GetFilename(m_CachedFile));

      if (strcmp(FullPath.c_str(), FullCacheFilePath.c_str()) == 0)
      {
        m_CachedFile.close();
      }
    }
  }

  void CompletePath(FixedStringPrint &rDestination, const char *pchPath)
  {
    rDestination.begin();
    rDestination.print(m_achRootPath);
    rDestination.print(pchPath);
  }

  time_t GetLastWriteTime(TFile &hFile)
  {
    // Only ESP32 SD card library (and not the SD fat library) supports
    // retrieving write time as a time_t. 
#if defined(ARDUINO_ARCH_ESP32) && !defined(SD_FAT_VERSION)
    return hFile.getLastWrite();
#else
    return 0; // unsupported
#endif
  }
};