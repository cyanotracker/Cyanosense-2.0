// Main and Modules are Written by MAS18195
// Spectrometer Test Code that interfaces with Python
// Required Modules:
//	ListFiles.ino
//	PrintSpectrum.ino
//	ReadSpectrum.ino

// This reads the Hammamatsu Spectrometers 288 channels and presents the data to the serial port
// where the host sends request for the X and Y data and an End of Data command when complete
// The host controls how much data is read and it can be less than the 288 channels
// A new scan will not start until the End of Data command is sent

// This has averaging add to the scan channel

// This has LittleSPIFFS to store and retrieve the sensor calibration coefficients
// They are applied to each of the 288 pixel channels

// Two spectrometers have be incorporated

// This adds the ability to adjust the integration time independently for both sensors

// 02/07/2023

// *************** WEMOS PRO PIN OUTS ***************
//    AO            Analog Input, max 3.3V
//    D0   GPIO16  IO, to reset for deep sleep wake, no interrupt or pullup
//    D1   GPIO5   IO, SCL (clock line)
//    D2   GPIO4   IO, SDA (data line)
//    D3   GPIO0   IO, 10k Pull-up
//    D4   GPIO2   IO, 10k Pull-up, Built-in LED
//    D5   GPIO14  IO, SCK (serial clock)
//    D6   GPIO12  IO, MISO (master in slave out)
//    D7   GPIO13  IO, MOSI (master out slave in)
//    D8   GPIO15  IO, 10k Pull-down, SS (slave select)
//----------------------------------------------------------------------------------------


#include <LittleFS.h>
#include <driver/adc.h>
//#include "esp_adc_cal.h"



#define SPEC_ST      9                   // sensor start
#define SPEC_CLK     8                   // sensor clock
// #define SPEC_VIDEO_UP   1                 // ADC1 channel 0
// #define SPEC_VIDEO_DWN  2                 // ADC1 channel 1
///#define WHITE_LED        
#define RELAY       17                    // 5V power enable/disable
#define SPEC_CHANNELS    289              // Spectrum Pixel Channel Bins (288+1 to correct data) 

float wavelengthUp = 0;                   // wavelength of pixel Up channel
float wavelengthDwn = 0;                  // wavelength of pixel Down channel
uint32_t dataUp[SPEC_CHANNELS];           // spectrum Up channel storage array
uint32_t dataDwn[SPEC_CHANNELS];          // spectrum Down channel storage array
bool EnableScan = true;                   // scan enable/disable
int Averages = 8;                         // number of scans to average, maximum 16
int integrationTimeUp = 1;                // Up sensor integration time in ms
int integrationTimeDwn = 2;               // Down sensor integration time in ms
const char * pathUp = "/cal_up.csv";      // Up sensor calibration coefficients file
const char * pathDwn = "/cal_down.csv";    // Down sensor calibration coefficients file

// A0cal, B1cal, B2cal, B3cal, B4cal, B5cal
double calCoefUp[6];                      // Up sensor calibration coefficients array
double calCoefDwn[6];                     // Down sensor calibration coefficients array


void setup()
{
  Serial.begin(115200);                   // Baud Rate set to 115200
////  delay(2000);                            // need delay for serial print statements, debug
  
  // configure ADC for Ardunio call 
///  analogReadResolution(13);               // valid values 12, 11, 10, 9
///  analogSetAttenuation(ADC_2_5db);        // valid values ADC_0db, ADC_2_5db, ADC_6db, ADC_11db default
///  analogSetAttenuation(ADC_0db);
///  analogSetClockDiv(1);                   // valid values 1 default (fastest), 255 (slowest)

  // configure ADC with API
  adc1_config_width(ADC_WIDTH_BIT_13);    // requries adc.h
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_2_5);    // requires adc.h
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_2_5);    // requires adc.h
  
  // Set desired pins to OUTPUT
  pinMode(SPEC_CLK, OUTPUT);
  pinMode(SPEC_ST, OUTPUT);
  pinMode(RELAY, OUTPUT);
///  pinMode(WHITE_LED, OUTPUT);
  digitalWrite(SPEC_CLK, HIGH);           // Set SPEC_CLK High
  digitalWrite(SPEC_ST, LOW);             // Set SPEC_ST Low
  
  // Mount the file system 
  LittleFS.begin(); 

  // List directory, from root, 2 levels deep
////  Serial.println();                         // debug
////  listDir(LittleFS, "/", 2);                // debug

  // Open Up file for reading
////  Serial.println();                         // debug
////  Serial.printf("Reading file: %s\r\n", path);   // debug
  File file = LittleFS.open(pathUp, "r");

  // Read from file and load calibration coefficient array
////  Serial.println();                         // debug
////  Serial.println("Reading from file:");     // debug
  int j = 0;
  while (file.available()) 
  {
///    Serial.write(file.read());             // send file contents to serial port, debug
    String x = file.readStringUntil(',');     // read and place each coefficient in variable
    calCoefUp[j] = x.toDouble();              // convert string to double float
////    Serial.print(x);  Serial.print("  "); Serial.println(calCoef[j], 20);       // debug
    j += 1;                                   // increment array index
  }
  file.close();
  
  // Open Down file for reading
  ////  Serial.println();                            // debug
////  Serial.printf("Reading file: %s\r\n", path);   // debug
  file = LittleFS.open(pathDwn, "r");

  // Read from file and load calibration coefficient array
////  Serial.println();                         // debug
////  Serial.println("Reading from file:");     // debug
  j = 0;
  while (file.available()) 
  {
///    Serial.write(file.read());             // send file contents to serial port, debug
    String x = file.readStringUntil(',');     // read and place each coefficient in variable
    calCoefDwn[j] = x.toDouble();             // convert string to double float
////    Serial.print(x);  Serial.print("  "); Serial.println(calCoef[j], 20);       // debug
    j += 1;                                   // increment array index
  }
  file.close();
  LittleFS.end();                         // stop littlespiffs to allow OTA updates 
    
  // relay control test
  digitalWrite(RELAY, LOW);               // Turns Relay On, 5V power enabled
}
//----------------------------------------------------------------------


void loop()
{
  readSpectrometer();             // get spectrometer data
  printData();                    // make data available over serial port
}
//----------------------------------------------------------------------
