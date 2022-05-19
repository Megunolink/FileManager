/* ********************************************************
 *  Implements file system using Arduino SD card library.
 *  ******************************************************** */

#pragma once

#include "utility/FileManager.h"
#include "ArduinoTimer.h"
#include "FixedStringBuffer.h"

#include <SD.h>

class SDFileManager : public MLP::FileManager
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

  // Maximum block size for sending file content. Should be a multiple of
  // 3 for best performance. 
  static const int m_nMaxBlockToSend = 510;

public:
  SDFileManager(const char* pchRootPath = nullptr);

  void Process();

protected:
  virtual DFTResult EnumerateFiles(DeviceFileTransfer &dft) override;
  virtual DFTResult SendFileContent(DeviceFileTransfer &dft, const char *pchPath, uint32_t uFirstByte) override;
  virtual DFTResult ReceiveFileContent(DeviceFileTransfer &dft, const char *pchPath, uint32_t uFirstByte, const char* pchBase64FileData) override;
  virtual void TransferComplete(const char *pchPath) override;

  virtual DFTResult DeleteFile(const char *pchPath) override;
  virtual DFTResult ClearAllFiles() override;

protected:
  virtual File OpenFile(const char* pchPath, bool bWriteable, bool bTruncate);
  File OpenFileCached(const char* pchPath, bool bWriteable, bool bTruncate);
  time_t GetLastWriteTime(File &hFile);
  void CloseCachedFile(const char *pchPath);
  bool FileExists(const char* pchPath);

  void CompletePath(FixedStringPrint &rDestination, const char* pchPath);
};