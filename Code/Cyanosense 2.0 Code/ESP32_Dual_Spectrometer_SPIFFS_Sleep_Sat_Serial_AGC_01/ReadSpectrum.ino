/*
 * This functions reads spectrometer data from SPEC_VIDEO
 * Look at the Timing Chart in the Datasheet for more info
 * Operational Example Using Minimum Times Except Clock:
 * Fclk = video data rate = 500kHz (ADC conversion time is < Fclk)
 * Start Pulse Cycle Time = 381/Fclk = 381/500kHz = 762us
 * Start Pulse Low Period = 375/Fclk = 375/ 500kHz = 750us
 * Start Pulse High Period = Start Pulse Cycle Time - Start Pulse Low Period = 762us - 750us = 12us 
 * Integration Time = Start Pulse High Period + 48 clk cycles = 12us = 48 * 1/500kHz = 108us
 * 
 * Integration Time =  high period of ST + 48 CLK cycles and starts 4 CLK after ST goes high 
 * There is confusion in the datasheet on this 
 * The video data starts at the first rising edge of the CLK after the ST goes low
 * Do not use the video data after the first ST after powerup
 * The integration time can be changed by changing the ratio of the high and low periods of ST
 * The first CLK after the ST goes low is the first start of output sequence and the video data 
 * should be aquired on the low period after the 88 CLK pulse
 * 
 * Using the ESP32 driver include the conversion time improves from 110us to 32us but the ADC output
 * value is not correct
 */
void readSpectrometer()
{
  if (EnableScan==true)
  {  
    int delayTime = 1;                // delay time in us, the normal execution speed adds an additional 1us
    for(int sensor = 0; sensor < 2; sensor++)   // tracks which sensor is scanning
    {
      for(int ac = 0; ac <= Averages; ac++)
      {
  // Start clock cycle
  // ignore first read after powerup
        digitalWrite(SPEC_CLK, LOW);
        delayMicroseconds(delayTime);
        digitalWrite(SPEC_CLK, HIGH);
        delayMicroseconds(delayTime);
        digitalWrite(SPEC_CLK, LOW);

  // this sets start high on boot but start is already high on subsequent scans
        digitalWrite(SPEC_ST, HIGH);        
        delayMicroseconds(delayTime);

  // Sample for a period of time
        for(int i = 0; i < 15; i++)       // add 15 more clk cycles before start goes low
        {
          digitalWrite(SPEC_CLK, HIGH);
          delayMicroseconds(delayTime);
          digitalWrite(SPEC_CLK, LOW);
          delayMicroseconds(delayTime); 
        }

  // Set SPEC_ST to low in prep for clocking out spectrum data
        digitalWrite(SPEC_ST, LOW);       // minimum low time 375/Fclk, should change on falling edge of clk

  // Sample for a period of time
        for(int i = 0; i < 88; i++)       // clk 88 cycles for data to be ready to clock out (original did 85+1 for 86)
        {
          digitalWrite(SPEC_CLK, HIGH);
          delayMicroseconds(delayTime);
          digitalWrite(SPEC_CLK, LOW);
          delayMicroseconds(delayTime); 
        }
  
  // Read from SPEC_VIDEO, data starts on 89th clk after Start goes low
        for(int i = 0; i < SPEC_CHANNELS; i++)       // loop for number of channel bins (288)
        {
        if (ac>0)                                    // do one dummy scan, array index 0
        {
          if (sensor<1)                              // check which sensor to scan
          {        
            dataUp[i] += adc1_get_raw(ADC1_CHANNEL_0);  // requires adc.h, Up Sensor, ADC conversion time, 33us
///            dataUp[i] += analogRead(SPEC_VIDEO_UP);    // read ADC Up data into array and sum next sample
          }
          else
          {
            dataDwn[i] += adc1_get_raw(ADC1_CHANNEL_1);  // requires adc.h, Down Sensor, ADC conversion time, 33us
///            dataDwn[i] += analogRead(SPEC_VIDEO_DWN);  // read ADC Down data into array and sum next sample 
          }                                            // total time is (one clock + ADC conversion time, 110us)
        }                                              // * channel bins
  // one clock cycle for each read
          digitalWrite(SPEC_CLK, HIGH);
          delayMicroseconds(delayTime);
          digitalWrite(SPEC_CLK, LOW);
          delayMicroseconds(delayTime);
        }

  // Set SPEC_ST (Start) to high, minimum time 6/Fclk, should change on falling edge of clk
  // minimum start pulse cycle time 381/Fclk
  // integration time is Start high period plus 48 clk cycles
        digitalWrite(SPEC_ST, HIGH);            // start of next scan sequence

  // Sample for a small amount of time
        for(int i = 0; i < 7; i++)              // clk 7 cycles
        {
          digitalWrite(SPEC_CLK, HIGH);
          delayMicroseconds(delayTime);
          digitalWrite(SPEC_CLK, LOW);
          delayMicroseconds(delayTime);
        }
        digitalWrite(SPEC_CLK, HIGH);           // set clk high for start of next sequence
        delayMicroseconds(delayTime);

        if (sensor<1)                           // check which sensor to delay
        {
          delay(integrationTimeUp);             // delay for Up integration time
        }
        else
        {
          delay(integrationTimeDwn);            // delay for Down integration time
        }
      }
    }  
  // calculate average scan values    
    for(int i = 0; i < SPEC_CHANNELS; i++)      // loop for number of channel bins (288)
    {
      dataUp[i] = dataUp[i]/Averages;           // calculate the average Up channel value
      dataDwn[i] = dataDwn[i]/Averages;         // calculate the average Dwn channel value
    }
  // AGC and check for scan count limit
    if (scanCount<3)              // three scans
    {
      scanCount +=1;              // increment scan counter
      AGC();                      // check for max intensity and set integration time 
    }
    else
    {
      changeIntegrationUp = false;
      changeIntegrationDwn = false;
    }
  // check if integration time was not changed  
    if (changeIntegrationUp==false && changeIntegrationDwn==false)
    {
  // power off spectrometers
      digitalWrite(RELAY, HIGH);                  // Turn Relay Off, 5V spectrometer power disabled

  // build and save the LittleFS data file
///    printLocalTime();
      buildFile(LittleFS, SpecDataFile);          // build and save LittleFS data file
      EnableScan = false;                         // clear enable scan flag
      if (ManualMode==false)                      // check for manual mode
      {
        FillTransmitBuffer = true;                // set fill transmit buffer flag
      }
    }
    else
    {
      memset(dataUp, 0, sizeof(dataUp));          // reset array values to 0
      memset(dataDwn, 0, sizeof(dataDwn));        // reset array values to 0 
    }
  }
}
//----------------------------------------------------------------------
