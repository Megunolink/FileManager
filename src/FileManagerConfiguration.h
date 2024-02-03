/* ********************************************************
 *  Configuration for the file manager. Provides constants
 *  to set maximum file and path lengths. 
 *  ******************************************************** */
 
 namespace NFileManager
 {
 
  // Allow longer filenames for devices with more memory as newer
  // versions of littlefs support long filenames. 
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  // Maximum number of characters for root path (including null terminator).
  const int MaxRootPath = 30;

  // Maximum length for a filename. 
  const int MaxFilenameLength = 30;
#else
  // Maximum number of characters for root path (including null terminator).
  const int MaxRootPath = 9;

  // Maximum length for a filename. 
  const int MaxFilenameLength = 15;
#endif

 
 }