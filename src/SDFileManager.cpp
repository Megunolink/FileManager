#include "SDFileManager.h"
#include "Formatting.h"

SDFileManager::SDFileManager(const char *pchRootPath /*= nullptr*/)
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

void SDFileManager::Process()
{
  if (m_CachedFile && m_tmrCloseCache.TimePassed_Milliseconds(m_nCacheTimeout))
  {
    m_CachedFile.close();
  }
}

DFTResult SDFileManager::EnumerateFiles(DeviceFileTransfer &dft)
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

DFTResult SDFileManager::SendFileContent(DeviceFileTransfer &dft, const char *pchPath, uint32_t uFirstByte)
{
  File hFile = OpenFileCached(pchPath, false, false);
  if (hFile)
  {
    if (hFile.seek(uFirstByte))
    {
      dft.SendFileBytes(pchPath, hFile, uFirstByte, m_nMaxBlockToSend);
      return DFTResult::Ok;
    }

    dft.SendFileBytes(pchPath, uFirstByte, DFTResult::SeekFailed);
    return DFTResult::SeekFailed;
  }

  dft.SendFileBytes(pchPath, uFirstByte, DFTResult::FileOpenFailed);
  return DFTResult::FileOpenFailed;
}

DFTResult SDFileManager::ReceiveFileContent(DeviceFileTransfer &dft, const char *pchPath, uint32_t uFirstByte, const char *pchBase64FileData)
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
    if (uFirstByte == (uint32_t) hFile.size())
    {
      int nWritten = DecodeFromBase64(hFile, pchBase64FileData);
      dft.FileReceiveResult(pchPath, uFirstByte, nWritten, nWritten == DECODE_BAD_DATA ? DFTResult::BadData : DFTResult:: Ok);
#if defined(ARDUINO_ARCH_ESP32)
      hFile.flush(); // Without flush, hFile.size() reports the wrong value on ESP32. 
#endif
      return DFTResult::Ok;
    }
    else
    {
#if 1
      Serial.print(F("Bad addr. Expected: "));
      Serial.print(hFile.size());
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

void SDFileManager::TransferComplete(const char *pchPath)
{
  CloseCachedFile(pchPath);
}

bool SDFileManager::FileExists(const char *pchPath)
{
  FixedStringBuffer<m_nMaxPathLength> FullPath;
  CompletePath(FullPath, pchPath);

  return SD.exists(FullPath.c_str());
}

DFTResult SDFileManager::DeleteFile(const char *pchPath)
{
  FixedStringBuffer<m_nMaxPathLength> FullPath;
  CompletePath(FullPath, pchPath);

  return SD.remove(FullPath.c_str()) ? DFTResult::Ok : DFTResult::DeleteFileFailed;
}

DFTResult SDFileManager::ClearAllFiles()
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
        if (DeleteFile(hFile.name()) != DFTResult::Ok)
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

File SDFileManager::OpenFile(const char *pchPath, bool bWriteable, bool bTruncate)
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

File SDFileManager::OpenFileCached(const char *pchPath, bool bWriteable, bool bTruncate)
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

time_t SDFileManager::GetLastWriteTime(File &hFile)
{
#if defined(ARDUINO_ARCH_ESP32)
  return hFile.getLastWrite();
#else
  return 0; // unsupported
#endif
}

void SDFileManager::CloseCachedFile(const char *pchPath)
{
  if (m_CachedFile && strcmp(pchPath, m_CachedFile.name()) == 0)
  {
    m_CachedFile.close();
  }
}

void SDFileManager::CompletePath(FixedStringPrint &rDestination, const char *pchPath)
{
  rDestination.print(m_achRootPath);
  rDestination.print(pchPath);
}
