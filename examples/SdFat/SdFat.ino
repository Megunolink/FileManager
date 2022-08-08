/* ******************************************************************
 *  SdFat.ino
 *  This example demonstrates use of a file manager that can be used
 *  to manage files on a device's SD card using MegunoLink's Device
 *  File Transfer visualizer and the SdFat Arduino library. Program 
 *  an Arduino equipped with an SD card and open the 
 *  "Device file transfer.mplz" MegunoLink project file in this sketch's
 *  project folder to send and receive files to the SD card.
 * 
 *  You will need:
 *    - MegunoLink's library for Arduino
 *      https://www.megunolink.com/documentation/getting-started/arduino-integration/
 *    - MegunoLink's file manager library
 *      https://github.com/Megunolink/FileManager
 *    - SdFat library for Arduino
 *      https://github.com/greiman/SdFat
 * 
 *  For more information:
 *    - MegunoLink device file transfer visualizer documentation
 *      https://www.megunolink.com/documentation/device-file-transfer/
 *  ****************************************************************** */

#include "MegunoLink.h"
#include "CommandHandler.h"
#include "SDFatFileManager.h"

// Decodes and processes serial commands. Uses increased serial receive
// buffer to support receiving files from MegunoLink. Set the same size
// in MegunoLink's Device file transfer visualizer settings to avoid
// buffer overflow errors.
const int MaxCommands = 10;     // user commands; FileManager commands don't contribute to this total.
const int MaxSerialBuffer = 60; // bytes
CommandHandler<MaxCommands, MaxSerialBuffer> Cmds;

// File system using SdFat library. 
SdFat FileSystem;

// File manager implementation.
SdFatFileManager FileManager(FileSystem);

// Initialize the device's SD card.
void InitSDCard()
{
  DeviceFileTransfer dft;
  const uint8_t SD_CHIP_SELECT_PIN = 4;
  if (FileSystem.begin(SD_CHIP_SELECT_PIN))
  {
    Serial.println(F("SD card ready."));
  }
  else
  {
    Serial.println(F("Failed to mount SD card"));
    dft.ReportSDMountFailed();
  }
}

void setup()
{
  Serial.begin(500000);
  Serial.println(F("SdFat MegunoLink File Manager Tester"));
  Serial.println(F("===================================="));

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
