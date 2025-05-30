// Main and Modules are Written by MAS18195
// This is currently a Debug Verison
// Required Modules:
//	BuildFile.ino
//	DeepSleep.ino
//	FileFunctions.ino
//	ReadSpectrum.ino
//	Time.ino
//	UploadData.ino

// This reads the Hammamatsu Spectrometers 288 channels and presents the data to the serial port
// where the host sends request for the data when the mode switch is set to upload mode
// The data is then streamed from LittleFS via the serial port in csv format
// In normal mode the scan will complete and the ESP32 will enter deep sleep and be woken by the
// alarm from the RTC module at the programmed time

// This has averaging add to the scan channel

// This has LittleSPIFFS to store and retrieve the sensor calibration coefficients
// They are applied to each of the 288 pixel channels

// Two spectrometers have been incorporated

// This adds the ability to adjust the integration time independently for both sensors

// Added deep sleep and RTC clock and second serial port for sat modem

// 04/01/2023

              
// *************** Saola ESP32-S2 PIN OUTS ***************
//    0-10 GPIO0-10    Analog Input, max 3.3V
//    3    GPIO3   IO, wake from deep sleep 
//    9    GPIO9   IO, SCL (clock line)
//    8    GPIO8   IO, SDA (data line)
//    16   GPIO16  IO, 
//    17   GPIO17  IO, DAC1, TXD1
//    18   GPIO18  IO, DAC2, RXD1
//    38   GPIO38  IO, FSPIWP
//    43   GPIO43  IO, TXD0
//    44   GPIO44  IO, RXD0
//    45   GPIO45  IO, 
//----------------------------------------------------------------------------------------


#include <LittleFS.h>
#include <esp_sleep.h>
#include <RTClib.h>
#include <driver/adc.h>


// spectrometer 
#define SPEC_ST      7                   // sensor start
#define SPEC_CLK     6                   // sensor clock
// ADC1_CHANNEL_0                        // GPIO-1 ADC1 channel 0 for API ADC call
// ADC1_CHANNEL_1                        // GPIO-2 ADC1 channel 1 for API ADC call
///#define SPEC_VIDEO_UP   1                // ADC1 channel 0 for Arduino ADC call
///#define SPEC_VIDEO_DWN  2                // ADC1 channel 1 for Arduino ADC call
#define WAKEUP_PIN   3                   // GPIO_NUM_3 used by RTC SQW for deep sleep wakeup
#define RELAY           16               // 5V spectrometer power enable/disable
#define SAT_MODEM_SLEEP 38               // sat modem enable/disable sleep
#define UPLOAD_SWITCH 45                 // data upload mode switch
#define SPEC_CHANNELS   289              // Spectrum Pixel Channel Bins + 1 because <= does not work
///#define WHITE_LED 

// sat modem serial port
#define SAT_RXD1 18                      // sat modem serial RXD0
#define SAT_TXD1 17                      // sat modem serial TXD0

// time 
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String fileDate;                         // create timestamp for file
RTC_DS3231 rtc;                          // declare rtc object
  
// sleep
int wakeup_hr = 11;                      // the time of hour for wakeup
int wakeup_min = 0;                      // the time of minutes for wakeup

// system variables
uint32_t dataUp[SPEC_CHANNELS];          // spectrum Up channel storage array
uint32_t dataDwn[SPEC_CHANNELS];         // spectrum Down channel storage array
String wavelengthUp;                     // wavelength of pixel Up channel
String wavelengthDwn;                    // wavelength of pixel Down channel
int Averages = 8;                        // number of scans to average, maximum 16
int integrationTimeUp = 0;               // Up sensor integration time in ms
int integrationTimeDwn = 10;              // Down sensor integration time in ms
String up;                               // string to store data to be written to LittleFS
String dwn;                              // string to store data to be written to LittleFS

// system flags
bool EnableScan = true;                  // scan enable/disable flag

// spectrum data file in LittleSF
const char * SpecDataFile = "/specdata.csv";  // Down sensor calibration coefficients file

// calibration files
const char * pathUp = "/cal_up.csv";      // Up sensor calibration coefficients file
const char * pathDwn = "/cal_down.csv";   // Down sensor calibration coefficients file

// A0cal, B1cal, B2cal, B3cal, B4cal, B5cal, spectrometer wavelength coefficients
double calCoefUp[6];                      // Up sensor calibration coefficients array
double calCoefDwn[6];                     // Down sensor calibration coefficients array

 
void setup()
{
  Serial.begin(115200);                   // Baud Rate set to 115200
  Serial1.begin(19200);                   // Sat Serial Port set baud rate to 19200 default pins
///  Serial1.begin(19200, SERIAL_8N1, SAT_RXD1, SAT_TXD1);  // configure sat modem serial port (does not work)  
///  delay(2000);                              // need delay for serial print statements, debug

  pinMode(UPLOAD_SWITCH, INPUT_PULLUP);   // data upload switch pin with pullup, low for upload
   
  // the alarm wakeup has pullup
  pinMode(WAKEUP_PIN, INPUT_PULLUP);
  
  // wake ESP32 on RTC alarm
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_3,0);  // wake on Low from RTC SQW pin
  
  // configure ADC for Arduino call 
///  analogReadResolution(13);               // valid values 12, 11, 10, 9
///  analogSetAttenuation(ADC_2_5db);        // valid values ADC_0db, ADC_2_5db, ADC_6db, ADC_11db default
///  analogSetClockDiv(1);                   // valid values 1 default (fastest), 255 (slowest)

  // configure ADC with API
  adc1_config_width(ADC_WIDTH_BIT_13);    // requries adc.h

  // Set the ADC Channel gain with API
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_2_5);    // requires adc.h, 33us conversion time
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_2_5);    // requires adc.h, 33us conversion time

  // initializing the rtc
  if(!rtc.begin()) 
  {
    Serial.println("Couldn't find RTC!");
    Serial.flush();
    while (1) delay(10);
  }

  // this will adjust to the date and time at each compilation, put in once to set clock and then rem out
///     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // don't need the 32K Pin, so disable it
  rtc.disable32K();
  
  // set alarm 1, 2 flag to false (because alarm 1, 2 didn't happen yet)
  // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  // stop oscillating signal at SQW Pin otherwise setAlarm1 will fail
  rtc.writeSqwPinMode(DS3231_OFF);

  // turn off alarm 2 (in case it isn't off already)
  // again, this isn't done at reboot, so a previously set alarm could easily go overlooked
  rtc.disableAlarm(2);

  // schedule an alarm 30 seconds in the future for testing
  
  if(!rtc.setAlarm1( rtc.now() + TimeSpan(30),DS3231_A1_Second ))   // this mode triggers the alarm when the seconds match
  {
    Serial.println();
    Serial.println("Error, alarm wasn't set!");
  }
  else 
  {
    Serial.println();
    Serial.println("Alarm will happen in 30 seconds!");
  }
  
  /*
  // Set to an explicit time
  // The alarm will trigger when seconds, minutes and hours match, resulting in triggering once per day,
  // every day at (15:34:00). The date, month and year are all ignored.
  // rtc.setAlarm1(DateTime(2020, 6, 25, 15, 34, 0), DS3231_A1_Hour), Hour can be replaced with Minutes or Second
                                    // hours, minutes, seconds
  if(!rtc.setAlarm1(DateTime(2023, 1, 01, wakeup_hr, wakeup_min, 0), DS3231_A1_Hour))
  {
    Serial.println();
    Serial.println("Error, alarm wasn't set!");
  }
  else 
  {
    Serial.println();
    Serial.print("Alarm will happen at ");  
    Serial.print(wakeup_hr);  Serial.print(":");
    Serial.println(wakeup_min);  Serial.println();
  }
  */
 
  // Set desired pins to OUTPUT
  pinMode(SPEC_CLK, OUTPUT);
  pinMode(SPEC_ST, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(SAT_MODEM_SLEEP, OUTPUT);
  
  digitalWrite(SPEC_CLK, HIGH);           // Set SPEC_CLK High
  digitalWrite(SPEC_ST, LOW);             // Set SPEC_ST Low
///  pinMode(WHITE_LED, OUTPUT);

  // put sat modem to sleep
  digitalWrite(SAT_MODEM_SLEEP, LOW);     // Set sat modem to sleep, low
    
  // power on spectrometers
  digitalWrite(RELAY, LOW);               // Turn Relay On, 5V spectrometer power enabled
  delay(100);                             // delay for spectrometers to stabilize
  
  // Mount the file system 
  LittleFS.begin(); 

  // List directory, from root, 2 levels deep
////  Serial.println();                           // debug
////  listDir(LittleFS, "/", 2);                  // debug

  // Open Up file for reading
////  Serial.println();                           // debug
////  Serial.printf("Reading file: %s\r\n", path);   // debug
  File file = LittleFS.open(pathUp, "r");

  // Read from file and load calibration coefficient array
////  Serial.println();                           // debug
////  Serial.println("Reading from file:");       // debug
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
////  Serial.println();                              // debug
////  Serial.printf("Reading file: %s\r\n", path);   // debug
  file = LittleFS.open(pathDwn, "r");

  // Read from file and load calibration coefficient array
////  Serial.println();                           // debug
////  Serial.println("Reading from file:");       // debug
  j = 0;
  while (file.available()) 
  {
///    Serial.write(file.read());             // send file contents to serial port, debug
    String x = file.readStringUntil(',');     // read and place each coefficient in variable
    calCoefDwn[j] = x.toDouble();             // convert string to double float
////    Serial.print(x);  Serial.print("  "); Serial.println(calCoef[j], 20);       // debug
    j += 1;                                   // increment array index
  }
  file.close();                           // close little fs file
////  LittleFS.end();                         // stop littlespiffs to allow OTA updates

  // check if upload switch is on, low
  if (digitalRead(UPLOAD_SWITCH)==LOW)
  {
    uploadData();                           // upload data
  } 
}
//----------------------------------------------------------------------


void loop()
{
  readSpectrometer();             // get spectrometer data
  deepSleep();                    // enter deep sleep
}
//----------------------------------------------------------------------
