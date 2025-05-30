// The function below prints out data to the terminal or serial port
// It is currently configured to read XUp, YUp, XDown, YDown for the complete arrays
void printData()
{
  static int i = 0;                 // array data index
  
  if (Serial.available())           // if axis request is sent go read it
  {
    char c = Serial.read();         // variable to hold axis request
    if (c == 'X')                  // X Up axis request
    {
  // X Up variable 
      i += 1;                    // add 1 to match calibration coefficients pixels, (1-288)
      wavelengthUp = (calCoefUp[0]+(calCoefUp[1]*i)+(calCoefUp[2]*pow(i,2))+(calCoefUp[3]*pow(i,3))+
                      (calCoefUp[4]*pow(i,4))+(calCoefUp[5]*pow(i,5)));
      Serial.println(wavelengthUp); 
    }
    else if (c == 'U')              // Y Up axis request
    {
  // Y Up variable 
      Serial.println(dataUp[i]);
    }
    else if (c == 'W')             // X Down axis request
    {
  // X Down variable 
      wavelengthDwn = (calCoefDwn[0]+(calCoefDwn[1]*i)+(calCoefDwn[2]*pow(i,2))+(calCoefDwn[3]*pow(i,3))+
                       (calCoefDwn[4]*pow(i,4))+(calCoefDwn[5]*pow(i,5)));
      Serial.println(wavelengthDwn);       
    }
    else if (c == 'D')             // Y Up axis request
    {
  // Y Down variable 
      Serial.println(dataDwn[i]);
    }
    else if (c == 'E')              // end of data character
    {
  // End of Data 
      EnableScan = true;            // enable scan
      i = 0;                        // reset array data index to start
      memset(dataUp, 0, sizeof(dataUp));    // reset array values to 0
      memset(dataDwn, 0, sizeof(dataDwn));  // reset array values to 0 
    }
  }
}
//----------------------------------------------------------------------
