/* ********************************************************
 *  Provides a general wrapper for file systems that use the
 *  standard File class.
 *  ******************************************************** */
#pragma once

#include "ArduinoTimer.h"
#include "../IFileManagerFileSystem.h"
#include "FixedStringBuffer.h"
#include "Formatting.h"

#include "SD.h" // for File definition.

class FileSystemWrapper : public IFileManagerFileSystem
{
protected:
  // File we are currently working to send/receive. Kept
  // open to improve performance. File is closed when
  // MegunoLink reports transfer is complete and/or after
  // time-out.
  File m_CachedFile;

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
    File hRoot = OpenFile(m_achRootPath, false, false);
    if (!hRoot)
    {
      Serial.print(F("Bad root path: "));
      Serial.println(m_achRootPath);
      return DFTResult::BadRoot;
    }

    DFTResult Result = DFTResult::Ok;
    if (hRoot.isDirectory())
    {
      while (File hFile = hRoot.openNextFile())
      {
        if (!hFile.isDirectory())
        {
          dft.SendFileInfo(hFile.name(), hFile.size(), GetLastWriteTime(hFile));
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

  virtual DFTResult ReceiveFileContent(const char *pchPath, uint32_t uFirstByte, const char *pchBase64Data, DeviceFileTransfer &dft) override
  {
    if (uFirstByte == 0)
    {
      if (m_CachedFile)
      {
        m_CachedFile.close();
      }
      if (FileExists(pchPath))
      {
        DeleteFile(pchPath);
      }
    }

    File hFile = OpenFileCached(pchPath, true, uFirstByte == 0);
    if (hFile)
    {
      if (uFirstByte == (uint32_t)hFile.size())
      {
        int nWritten = DecodeFromBase64(hFile, pchBase64Data);
        dft.FileReceiveResult(pchPath, uFirstByte, nWritten, nWritten == DECODE_BAD_DATA ? DFTResult::BadData : DFTResult::Ok);
#if defined(ARDUINO_ARCH_ESP32)
        hFile.flush(); // Without flush, hFile.size() reports the wrong value on ESP32.
#endif
        return DFTResult::Ok;
      }
      else
      {
#if 1
        Serial.print(F("Bad addr. Expected: "));
        Serial.print((uint32_t)hFile.size());
        Serial.print(F(", got: "));
        Serial.println(uFirstByte);
#endif
        dft.FileReceiveResult(pchPath, uFirstByte, 0, DFTResult::BadDataBlockAddress);
        return DFTResult::BadDataBlockAddress;
      }
    }
    else
    {
      dft.FileReceiveResult(pchPath, uFirstByte, 0, DFTResult::FileOpenFailed);
      return DFTResult::FileOpenFailed;
    }
  }

  virtual void TransferComplete(const char *pchPath) override
  {
    CloseCachedFile(pchPath);
  }

  virtual DFTResult SendFileContent(const char *pchPath, uint32_t uFirstByte, uint32_t uBlockSize, DeviceFileTransfer &dft) override
  {
    File hFile = OpenFileCached(pchPath, false, false);
    if (hFile)
    {
      if (hFile.seek(uFirstByte))
      {
        dft.SendFileBytes(pchPath, hFile, uFirstByte, uBlockSize);
        return DFTResult::Ok;
      }

      dft.SendFileBytes(pchPath, uFirstByte, DFTResult::SeekFailed);
      return DFTResult::SeekFailed;
    }

    dft.SendFileBytes(pchPath, uFirstByte, DFTResult::FileOpenFailed);
    return DFTResult::FileOpenFailed;
  }

  virtual DFTResult ClearAllFiles() override
  {
    File hRoot = OpenFile(m_achRootPath, false, false);
    if (!hRoot)
    {
      return DFTResult::BadRoot;
    }

    DFTResult Result = DFTResult::Ok;
    if (hRoot.isDirectory())
    {
      while (File hFile = hRoot.openNextFile())
      {
        if (!hFile.isDirectory())
        {
          if (!DeleteFile(hFile.name()))
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
  virtual bool FileExists(const char *pchPath) = 0;

  virtual File OpenFile(const char *pchPath, bool bWriteable, bool bTruncate) = 0;

  File OpenFileCached(const char *pchPath, bool bWriteable, bool bTruncate)
  {
    if (bWriteable && bTruncate && m_CachedFile)
    {
      m_CachedFile.close();
    }
    else if (m_CachedFile)
    {
      if (strcmp(pchPath, m_CachedFile.name()) == 0 && m_bCachedIsWriteable == bWriteable)
      {
        m_tmrCloseCache.Reset();
        return m_CachedFile;
      }
      else
      {
        m_CachedFile.close();
      }
    }

    m_CachedFile = OpenFile(pchPath, bWriteable, bTruncate);
    m_tmrCloseCache.Reset();
    m_bCachedIsWriteable = bWriteable;

    return m_CachedFile;
  }

  void CloseCachedFile(const char *pchPath)
  {
    if (m_CachedFile && strcmp(pchPath, m_CachedFile.name()) == 0)
    {
      m_CachedFile.close();
    }
  }


  void CompletePath(FixedStringPrint &rDestination, const char *pchPath)
  {
    rDestination.print(m_achRootPath);
    rDestination.print(pchPath);
  }

  time_t GetLastWriteTime(File &hFile)
  {
#if defined(ARDUINO_ARCH_ESP32)
    return hFile.getLastWrite();
#else
    return 0; // unsupported
#endif
  }  
};