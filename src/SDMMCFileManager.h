/* ********************************************************
 *  Implements file manager for ESP32 accessing memory 
 *  cards through the SDMMC driver
 *  ******************************************************** */
#pragma once

#if defined(ARDUINO_ARCH_ESP32)

#include "FS.h"
#include "SD_MMC.h"
#include "SDFileManager.h"

class SDMMCFileManager : public SDFileManager
{
public:
  SDMMCFileManager(const char* pchRootPath = nullptr);

protected:
  virtual DFTResult DeleteFile(const char *pchPath) override;
  virtual File OpenFile(const char* pchPath, bool bWriteable, bool bTruncate) override;

};

#endif
