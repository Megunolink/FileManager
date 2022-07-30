#include "SDFileManager.h"
#include "Formatting.h"

SDFileManager::SDFileManager(IFileManagerFileSystem &rIFileSystem)
  : m_rIFileSystem(rIFileSystem)
{
}

void SDFileManager::Process()
{
}

DFTResult SDFileManager::EnumerateFiles(DeviceFileTransfer &dft)
{
  return m_rIFileSystem.ListFiles(dft);
}

DFTResult SDFileManager::SendFileContent(DeviceFileTransfer &dft, const char *pchPath, uint32_t uFirstByte)
{
  return m_rIFileSystem.SendFileContent(pchPath, uFirstByte, m_nMaxBlockToSend, dft);
}

DFTResult SDFileManager::ReceiveFileContent(DeviceFileTransfer &dft, const char *pchPath, uint32_t uFirstByte, const char *pchBase64FileData)
{
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




