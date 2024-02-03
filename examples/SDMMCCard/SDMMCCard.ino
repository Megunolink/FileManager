/* ******************************************************************
*  SDMMCCard.ino
*  This example demonstrates use of a file manager that can be used
*  to manage files on an ESP32 device using the SDMMC SD card interface 
*  and MegunoLink's Device File Transfer visualizer.
*  
*  Note: currently only supported for ESP32 devices, the only device
*  we know of that supports the SD MMC interface. For a hookup guide,
*  see: https://www.youtube.com/watch?v=e1xOgZsnAuw
*
*  Program an Arduino equipped with an
*  SD card and open the "Device file transfer.mplz" MegunoLink 
*  project file in this sketch's project folder to send and receive
*  files to the SD card. 
*
*  You will need:
*    - MegunoLink's library for Arduino
*      https://www.megunolink.com/documentation/getting-started/arduino-integration/
*    - MegunoLink's file manager library
*      https://github.com/Megunolink/FileManager
* 
*  For more information:
*    - MegunoLink device file transfer visualizer documentation
*      https://www.megunolink.com/documentation/device-file-transfer/
*  ****************************************************************** */

#include "MegunoLink.h"
#include "CommandHandler.h"
#include <SDMMCFileManager.h>

// Decodes and processes serial commands. Uses increased serial receive
// buffer to support receiving files from MegunoLink. Set the same size
// in MegunoLink's Device file transfer visualizer settings to avoid
// buffer overflow errors. 
const int MaxCommands = 10; // user commands; FileManager commands don't contribute to this total.
const int MaxSerialBuffer = 60; // bytes
CommandHandler<MaxCommands, MaxSerialBuffer> Cmds;

// File manager implementation. 
SDMMCFileManager FileManager;

// Initialize the device's SD card. 
void InitSDCard()
{
#if defined(ARDUINO_ARCH_ESP32)
  // Many ESP32 boards need a pullup on the Data0Pin for
  // SD MMC interface. Implement with internal pullup. 
  const uint8_t Data0Pin = 2; 
  pinMode(Data0Pin, INPUT_PULLUP);
  delay(100);
#endif
  
  const char* MountPoint = "/sdcard";
  const bool OneBitMode = true; // true => 1 bit mode; false => 4 bit mode. 
  if (SD_MMC.begin(MountPoint, OneBitMode))
  {
    uint8_t uCardType = SD_MMC.cardType();
    if (uCardType == CARD_NONE)
    {
      Serial.println(F("No card installed"));
    }
    else
    {
      Serial.println(F("Card ready."));
    }
  }
  else
  {
    Serial.println(F("Failed to mount SD card"));
  }
}

void setup()
{
  Serial.begin(500000);
  Serial.println(F("SDMMC Card MegunoLink File Manager Tester"));
  Serial.println(F("========================================="));

  // Enable pull-ups for SD card pins
  pinMode(2, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);

  // Register the file manager command module
  // with the command handler. 
  Cmds.AddModule(&FileManager);

  // Initialize SD card. 
  InitSDCard();
}

void loop()
{
  // Decode and dispatch serial commands. 
  Cmds.Process();

  // Automatically close cached files if the connection
  // to MegunoLink is interrupted. 
  FileManager.Process();
}
