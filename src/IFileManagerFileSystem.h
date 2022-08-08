#pragma once

#include "utility/FileManager.h"

class IFileManagerFileSystem
{
public:
  virtual void Process() {} 

  virtual bool FileExists(const char* pchPath) = 0; 
  virtual bool DeleteFile(const char* pchPath) = 0; 
  virtual DFTResult ListFiles(DeviceFileTransfer &dft) = 0; 
  virtual DFTResult ReceiveFileContent(const char* pchPath, uint32_t uFirstByte, const char* pchBase64Data, DeviceFileTransfer &dft) = 0; 
  virtual DFTResult SendFileContent(const char*pchPath, uint32_t uFirstByte, uint32_t uBlockSize, DeviceFileTransfer &dft);
  virtual void TransferComplete(const char *pchPath) = 0;

  virtual DFTResult ClearAllFiles() = 0; 

};