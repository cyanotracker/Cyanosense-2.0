// List files in a dir
void listDir(fs::FS &fs, const char * dirname, uint8_t levels) 
{
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) 
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) 
  {
    Serial.println(" - not a directory");
    return;
  }
//----------------------------------------------------------------------

  File file = root.openNextFile();
  while (file) 
  {
    if (file.isDirectory()) 
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) 
      {
        listDir(fs, file.name(), levels - 1);
      }
    } 
    else 
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
//----------------------------------------------------------------------

// Delete file from LittleFS
void deleteFile(fs::FS &fs, const char * path)
{
  Serial.printf("Deleting file: %s\r\n", path);
  if(LittleFS.remove(path))
  {
    Serial.println("− file deleted");
  } 
  else 
  {
    Serial.println("− delete failed");
  }
}
//-----------------------------------------------------------------

// Read from file in LittleFS
void readFile(fs::FS &fs, const char * path)
{
  Serial.printf("Reading file: %s\r\n", path);
  
  ///   File file = LittleFS.open(path);          // with implicit command
  File file = LittleFS.open(path, "r");        // with explicit command
  
  if(!file || file.isDirectory())
  {
     Serial.println("− failed to open file for reading");
     return;
  }
  
  Serial.println("− read from file:");
  while(file.available())
  {
    Serial.write(file.read());
  }
}
//-----------------------------------------------------------------

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message)
{
  Serial.printf("Writing file: %s\r\n", path);
  
  File file = LittleFS.open(path, FILE_WRITE);
  if(!file)
  {
    Serial.println("− failed to open file for writing");
    return;
  }
  if(file.print(message))
  {
    Serial.println("− file written");
  }
  else 
  {
    Serial.println("− frite failed");
  }
}
//-----------------------------------------------------------------

// Append to file in LittleFS
void appendFile(fs::FS &fs, const char * path, const char * message)
{
  Serial.printf("Appending to file: %s\r\n", path);

  File file = LittleFS.open(path, FILE_APPEND);
  if(!file)
  {
    Serial.println("− failed to open file for appending");
    return;
  }
  if(file.print(message))
  {
    Serial.println("− message appended");
  } 
  else 
  {
    Serial.println("− append failed");
  }
}
//-----------------------------------------------------------------

// Rename a file in LittleFS
void renameFile(fs::FS &fs, const char * path1, const char * path2)
{
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (LittleFS.rename(path1, path2)) 
  {
    Serial.println("− file renamed");
  } 
  else 
  {
    Serial.println("− rename failed");
  }
}
//-----------------------------------------------------------------

// check for available memory and delete data file if too large
void checkAvailableMemory()
{
  // get available LittleFS memory (takes 72us)
  AvailLittleFS = (float)LittleFS.totalBytes() / 1024.0 - (float)LittleFS.usedBytes() / 1024.0;
  Serial.print(AvailLittleFS);  Serial.println("kbyte");   // debug
  
  // delete data file if memory is too low
  if (AvailLittleFS<MINMEMORY)                      // check if data memory is too low
  {
    deleteFile(LittleFS, SpecDataFile);             // delete data file
    listDir(LittleFS, "/", 0);                      // list files in LittleFS
  }
}
//---------------------------------------------------------------------
