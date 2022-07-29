/* ********************************************************
 *  Implements communication with MegunoLink to manage files
 *  on local device storage
 *  ******************************************************** */
#pragma once

#include <Arduino.h>
#include "CommandProcessor.h"
#include "MegunoLink.h"
#include "CommandModule.h"

namespace MLP
{
  enum class FileManagerOptions
  {
    DisableDeletion = 0,
    AllowFileDeletion = 1,
    AllowClearCard = 2,
    AllowDeletion = AllowFileDeletion | AllowClearCard,
  };

  class FileManager : public CommandModule
  {
  public:
    FileManager(FileManagerOptions fmo = FileManagerOptions::AllowDeletion);

    virtual void DispatchCommand(CommandParameter &p) override;

    void SetOptions(FileManagerOptions opt);
    bool IsFileDeleteEnabled() const { return IsOptionEnabled(FileManagerOptions::AllowFileDeletion); }
    bool IsCardClearEnabled() const { return IsOptionEnabled(FileManagerOptions::AllowClearCard); }

  protected:
    void HandleListFiles(CommandParameter &p);
    void HandleGetFileContent(CommandParameter &p);
    void HandlePutFileContent(CommandParameter &p);
    void HandleTransferComplete(CommandParameter &p);
    void HandleDeleteFile(CommandParameter &p);
    void HandleDeleteAllFiles(CommandParameter &p);
    void HandleUnknownCommand(CommandParameter &p);

    FileManagerOptions m_Options;

    bool IsOptionEnabled(FileManagerOptions fmo) const;
    void ReportFailures(DeviceFileTransfer &dft, char chContext, DFTResult Result, const char *pchPath = nullptr, uint32_t uContext = 0);

  protected:
    virtual DFTResult EnumerateFiles(DeviceFileTransfer &dft) = 0;
    virtual DFTResult SendFileContent(DeviceFileTransfer &dft, const char *pchPath, uint32_t uFirstByte) = 0;
    virtual DFTResult ReceiveFileContent(DeviceFileTransfer &dft, const char *pchPath, uint32_t uFirstByte, const char* pchBase64FileData) = 0; 
    virtual void TransferComplete(const char *pchPath) = 0;

    virtual DFTResult DeleteFile(const char *pchPath) = 0;
    virtual DFTResult ClearAllFiles() = 0;
  };
}