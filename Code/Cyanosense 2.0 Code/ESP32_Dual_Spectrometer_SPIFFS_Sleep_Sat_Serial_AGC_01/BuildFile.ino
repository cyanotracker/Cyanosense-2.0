// Build data string and write to LittleFS file for serial upload
void buildFile(fs::FS &fs, const char * path)
{
  Serial.printf("Build File Array with %s\r\n", path);

  // create data header
  String UpHeader = "Up ";
  String DwnHeader = "Dwn ";
  
  UpHeader += fileDate + "\r\n";      // add unix timestamp
  DwnHeader += fileDate + "\r\n";     // add unix timestamp
  
  // build the string file for writing to LittleFS with header
  up = UpHeader;
  dwn = DwnHeader;
  for(int i = 1; i < SPEC_CHANNELS; i++)
  {
    up += String(dataUp[i])+",";
    dwn += String(dataDwn[i])+",";
  }
  up += "\r\n";             // add returns to end
  dwn += "\r\n";            // add returns to end

///  File file = fs.open(path, FILE_APPEND);
///  if(!file)
///  {
///     Serial.println("− failed to open file for appending");
///     return;
///  }

  // write the data to LittleFS
  appendFile(LittleFS, SpecDataFile, up.c_str());
  appendFile(LittleFS, SpecDataFile, dwn.c_str());
  
  up = "";                // empty strings to free space
  dwn = "";               // empty strings to free space
}
//-----------------------------------------------------------------
