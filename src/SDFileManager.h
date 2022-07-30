/* ********************************************************
 *  Implements file system using Arduino SD card library.
 *  ******************************************************** */

#pragma once

#include "utility/FileManager.h"
#include "FixedStringBuffer.h"
#include "IFileManagerFileSystem.h"

#include <SD.h>

class SDFileManager : public MLP::FileManager
{
protected:
  IFileManagerFileSystem &m_rIFileSystem;

  // Maximum block size for sending file content. Should be a multiple of
  // 3 for best performance. 
  static const int m_nMaxBlockToSend = 510;

public:
  SDFileManager(IFileManagerFileSystem &m_rIFileSystem);

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

};