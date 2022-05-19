#include "FileManager.h"
#include "Formatting.h"

using namespace MLP;

const char Cmd_ListFiles = '?';
const char Cmd_GetFileContent = '<';
const char Cmd_PutFileContent = '>';
const char Cmd_DeleteFile = 'd';
const char Cmd_DeleteAllFiles = 'x';
const char Cmd_TransferComplete = '.';
const char Cmd_Unknown = '*';

FileManager::FileManager(FileManagerOptions fmo)
    : CommandModule(F("FM"))
{
  m_Options = fmo;
}

void FileManager::DispatchCommand(CommandParameter &p)
{
  const char *pchCommand = p.NextParameter();
  switch (*pchCommand)
  {
  case Cmd_ListFiles:
    HandleListFiles(p);
    break;

  case Cmd_GetFileContent:
    HandleGetFileContent(p);
    break;

  case Cmd_PutFileContent:
    HandlePutFileContent(p);
    break;

  case Cmd_DeleteFile:
    HandleDeleteFile(p);
    break;

  case Cmd_DeleteAllFiles:
    HandleDeleteAllFiles(p);
    break;

  case Cmd_TransferComplete:
    HandleTransferComplete(p);
    break;

  default:
    HandleUnknownCommand(p);
    break;
  }
}

void FileManager::SetOptions(FileManagerOptions opt)
{
  m_Options = opt;
}

void FileManager::HandleListFiles(CommandParameter &p)
{
  DeviceFileTransfer dft(p.Response);
  DFTResult Result = EnumerateFiles(dft);
  ReportFailures(dft, Cmd_ListFiles, Result);
}

void FileManager::HandleGetFileContent(CommandParameter &p)
{
  uint32_t uFirstByte = p.NextParameterAsUnsignedLong(0);
  const char *pchFile = p.RemainingParameters();
  DeviceFileTransfer dft(p.Response);
  SendFileContent(dft, pchFile, uFirstByte);
}

void FileManager::HandlePutFileContent(CommandParameter &p)
{
  long lAddress = p.NextParameterAsU32FromHex();
  const char *pchBase64Data = p.NextParameter();
  uint16_t uExpectedChecksum = p.NextParameterAsU16FromHex();
  const char *pchFile = p.RemainingParameters();

  DeviceFileTransfer dft(p.Response);
  uint16_t uActualChecksum = CalculateChecksumFromBase64(pchBase64Data);
  if (uActualChecksum == uExpectedChecksum)
  {
    ReceiveFileContent(dft, pchFile, lAddress, pchBase64Data);
  }
  else
  {
    dft.FileReceiveResult(pchFile, lAddress, 0, DFTResult::BadChecksum);
  }
}

void FileManager::HandleTransferComplete(CommandParameter &p)
{
  const char *pchPath = p.RemainingParameters();
  TransferComplete(pchPath);
}

void FileManager::HandleDeleteFile(CommandParameter &p)
{
  const char *pchPath = p.RemainingParameters();
  DFTResult Result; 

  DeviceFileTransfer dft(p.Response);
  if (IsOptionEnabled(FileManagerOptions::AllowFileDeletion))
  {
    Result = DeleteFile(pchPath);
  }
  else
  {
    Serial.println(F("File del dsbld"));
    Result = DFTResult::FileDeleteDisabled;
  }
  dft.FileDeleteResult(pchPath, Result);
}

void FileManager::HandleDeleteAllFiles(CommandParameter &p)
{
  uint16_t nRequestId = p.NextParameterAsUnsignedLong();
  DFTResult Result; 

  DeviceFileTransfer dft(p.Response);
  if (IsOptionEnabled(FileManagerOptions::AllowClearCard))
  {
    Result = ClearAllFiles();
  }
  else
  {
    Serial.println(F("Clr fldr dsabld"));
    Result = DFTResult::DeleteAllDisabled;  
  }
  dft.AllFilesDeleted(nRequestId, Result);
}

void FileManager::HandleUnknownCommand(CommandParameter &p)
{
  Serial.println(F("Unk file mgr cmd"));
  DeviceFileTransfer dft(p.Response);
  ReportFailures(dft, Cmd_Unknown, DFTResult::UnknownCommand);
}

bool FileManager::IsOptionEnabled(FileManagerOptions fmo) const
{
  uint16_t uOptions = (uint16_t)m_Options;
  uint16_t uTest = (uint16_t)fmo;
  return (uOptions & uTest) == uTest;
}

void FileManager::ReportFailures(DeviceFileTransfer &dft, char chContext, DFTResult Result, const char *pchPath, uint32_t uContext)
{
  if (Result != DFTResult::Ok)
  {
    dft.SendError(Result, chContext, pchPath, uContext);
  }
}
