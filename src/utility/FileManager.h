/* ********************************************************
 *  Implements communication with MegunoLink to manage files
 *  on local device storage
 *  ******************************************************** */
#pragma once

#include <Arduino.h>
#include "CommandProcessor.h"
#include "MegunoLink.h"
#include "CommandModule.h"
#include "../IFileManagerFileSystem.h"

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
  private:
    IFileManagerFileSystem &m_rFileSystem;

    // Maximum block size for sending file content. Should be a multiple of
    // 3 for best performance. Maybe this should come from the communication
    // channel in the future. 
    static const int m_nMaxBlockToSend = 510;
    
    FileManager(const FileManager&) ;
  protected:
    FileManager(IFileManagerFileSystem &rFileSystem, FileManagerOptions fmo = FileManagerOptions::AllowDeletion);
    
  public:
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

  };
}