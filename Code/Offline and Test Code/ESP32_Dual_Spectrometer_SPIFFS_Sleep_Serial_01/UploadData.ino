void uploadData()
{
  Serial.println();
  Serial.println("Entering Upload Mode Press G then Enter to start serial data transfer.");
  Serial.println("After transfer is complete Press D then Enter to delete old data file.");

  while(true)
  {
  // check for request to send data
    if (Serial.available())          // if request is sent go read it
    {
      char c = Serial.read();        // variable to hold request
      if (c == 'G')                  // G data request
      {
  // G, Send get data character
        Serial.println();
        Serial.println("Wavelength Up"); 
        for(int i = 1; i < SPEC_CHANNELS; i++)  // starting at 1 to match start of calibration coefficients pixels, (1-288)
        {
          wavelengthUp += String((calCoefUp[0]+(calCoefUp[1]*i)+(calCoefUp[2]*pow(i,2))+(calCoefUp[3]*pow(i,3))+
                          (calCoefUp[4]*pow(i,4))+(calCoefUp[5]*pow(i,5))))+",";
        }
        Serial.println(wavelengthUp); Serial.println();
        
        Serial.println("Wavelength Down"); 
        for(int i = 1; i < SPEC_CHANNELS; i++)  // starting at 1 to match start of calibration coefficients pixels, (1-288)
        {
          wavelengthDwn += String((calCoefDwn[0]+(calCoefDwn[1]*i)+(calCoefDwn[2]*pow(i,2))+(calCoefDwn[3]*pow(i,3))+
                          (calCoefDwn[4]*pow(i,4))+(calCoefDwn[5]*pow(i,5))))+",";
        }
        Serial.println(wavelengthDwn); Serial.println();
        wavelengthUp = "";                              // clear string
        wavelengthDwn = "";                             // clear string
  
  // read LittleFS data file and send to serial port
        Serial.println(); 
        readFile(LittleFS, SpecDataFile);               // send data to serial port
        Serial.println();
        listDir(LittleFS, "/", 0);                      // list files in LittleFS without sub directories
      }
      else if (c == 'D')             // D, delete data file
      {
        deleteFile(LittleFS, SpecDataFile);             // delete data file
        listDir(LittleFS, "/", 0);                      // list files in LittleFS
      }
    }
  }  
}
//----------------------------------------------------------------------
