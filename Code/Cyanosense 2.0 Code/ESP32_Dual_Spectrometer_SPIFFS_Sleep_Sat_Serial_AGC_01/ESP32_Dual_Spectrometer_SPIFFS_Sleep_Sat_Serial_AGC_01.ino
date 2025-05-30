// This reads the Hammamatsu Spectrometers 288 channels and presents the data to the serial port
// where the host sends request for the data when the mode switch is set to upload mode
// The data is then streamed from LittleFS via the serial port in csv format
// In normal mode the scan will complete and the ESP32 will enter deep sleep and be woken by the
// alarm from the RTC module at the programmed time

// This has averaging added to the scan channel

// LittleSPIFFS is used to store and retrieve the sensor calibration coefficients
// which are applied to each of the 288 pixel channels

// Two spectrometers have been incorporated

// There is the ability to adjust the integration time independently for both sensors

// Added deep sleep, RTC clock, and second serial port with word to byte conversion and
// Rockblock 9603 sat modem
// Two sat modem transmissions are required, to transmit 160 intensity values for each sensor

// Manual upload capability to PC with keyboard commands with ~one year of data storage
// Added auto delete mechanism before memory overflow occurs 

// Automatic Gain Control (AGC) was incorporated to compensate for varying degrees of light conditiond

// version ESP32-S2 04/17/2023 build 2.0.7

              
// *************** Saola ESP32-S2 PIN OUTS ***************
//    0-10 GPIO0-10    ADC1 Input, max 3.3V
//    3    GPIO3   IO, wake from deep sleep 
//    9    GPIO9   IO, SCL (clock line)
//    8    GPIO8   IO, SDA (data line)
//    16   GPIO16  IO, 
//    17   GPIO17  IO, DAC1, TXD1
//    18   GPIO18  IO, DAC2, RXD1
//    19   GPIO18  IO, ADC2-8, USBD-, RTS1
//    38   GPIO38  IO, FSPIWP
//    43   GPIO43  IO, TXD0
//    44   GPIO44  IO, RXD0
//    45   GPIO45  IO, 
//----------------------------------------------------------------------------------------


#include <LittleFS.h>
#include <esp_sleep.h>
#include <RTClib.h>
#include <IridiumSBD.h>
#include <driver/adc.h>


// spectrometer 
#define SPEC_ST      7                   // sensor start pin
#define SPEC_CLK     6                   // sensor clock pin
// ADC1_CHANNEL_0                        // GPIO-1 ADC1 channel 0 for API ADC call
// ADC1_CHANNEL_1                        // GPIO-2 ADC1 channel 1 for API ADC call
///#define SPEC_VIDEO_UP   1                // ADC1 channel 0 for Arduino ADC call
///#define SPEC_VIDEO_DWN  2                // ADC1 channel 1 for Arduino ADC call
#define WAKEUP_PIN   3                   // GPIO_NUM_3 used by RTC SQW for deep sleep wakeup
#define RELAY           16               // 5V spectrometer power enable/disable pin
#define UPLOAD_SWITCH 45                 // data upload mode switch pin
#define SPEC_CHANNELS   289              // Spectrum Pixel Channel Bins + 1 because <= does not work
#define DATASTART  30                    // start index of data to send to sat modem (actual, 30) (testing, 30)
#define DATAEND  190                     // end index of data to send to sat modem (actual, 190) (testing, 50)
#define MINMEMORY  400.00                // minimum kB memory to delete saved data file
///#define WHITE_LED 

// sat modem serial port and parameters
#define IRIDIUMSERIAL Serial1            // redefine serial port for sat modem
#define SAT_MODEM_SLEEP 19               // sat modem enable/disable sleep
#define SAT_RXD1 18                      // sat modem serial RXD0 pin, for reference
#define SAT_TXD1 17                      // sat modem serial TXD0 pin, for reference
#define SIZEOFBUFFER 322                 // size of transmit buffer in bytes (max 340)
int signalQuality = -1;                  // set signal quality default
int err = 0;                             // sat modem error number
unsigned int sizeofBuffer;               // size of sat transmit buffer
unsigned int dataSize;                   // size of data in buffer to transmit
uint8_t bufferUp[SIZEOFBUFFER];          // sky transmit buffer for sat modem with high byte and low byte
uint8_t bufferDwn[SIZEOFBUFFER];         // water transmit buffer for sat modem with high byte and low byte

// time 
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String fileDate;                         // create timestamp for file
RTC_DS3231 rtc;                          // declare rtc object
  
// sleep
int wakeup_hr = 11;                      // the time of hour for wakeup
int wakeup_min = 0;                      // the time of minutes for wakeup
bool wakeupType = false;                 // clear wakeup type flag (not RTC wakeup)

// system variables
uint32_t dataUp[SPEC_CHANNELS];          // spectrum Up channel storage array
uint32_t dataDwn[SPEC_CHANNELS];         // spectrum Down channel storage array
const int Averages = 8;                  // number of scans to average, maximum 16
int integrationTimeUp = 0;               // Up sensor integration time in ms
int integrationTimeDwn = 0;              // Down sensor integration time in ms
String wavelengthUp;                     // wavelength of pixel Up channel
String wavelengthDwn;                    // wavelength of pixel Down channel
String up;                               // string to store data to be written to LittleFS
String dwn;                              // string to store data to be written to LittleFS
float AvailLittleFS;                     // amount of LittleFS memory available 
unsigned int dataUpMax;
unsigned int dataDwnMax;
unsigned int AdjintegrationTimeUp;
unsigned int AdjintegrationTimeDwn;
unsigned int scanCount;

// system flags
bool EnableScan = true;                  // set scan enable flag
bool FillTransmitBuffer = false;         // clear fill transmit buffer flag 
bool TransmitData = false;               // clear transmit data flag
bool GotoSleep = false;                  // clear go to deep sleep flag
bool changeIntegrationUp = false;        // clear change integration flag
bool changeIntegrationDwn = false;       // clear change integration flag
bool ManualMode = false;                 // clear manual mode flag to enable sat modem operations

// spectrum data file in LittleSF
const char * SpecDataFile = "/specdata.csv";  // Down sensor calibration coefficients file

// calibration files
const char * pathUp = "/cal_up.csv";      // Up sensor calibration coefficients file
const char * pathDwn = "/cal_down.csv";   // Down sensor calibration coefficients file

// A0cal, B1cal, B2cal, B3cal, B4cal, B5cal, spectrometer wavelength coefficients
double calCoefUp[6];                      // Up sensor calibration coefficients array
double calCoefDwn[6];                     // Down sensor calibration coefficients array

// Declare the IridiumSBD object with SLEEP pin
IridiumSBD modem(IRIDIUMSERIAL, SAT_MODEM_SLEEP);

 
void setup()
{
  Serial.begin(115200);                   // Baud Rate set to 115200
  IRIDIUMSERIAL.begin(19200);             // Start the serial port connected to the satellite modem, default pins
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

  // turn off alarm 2 to make sure it is off
  // this isn't done at reboot, so a previously set alarm could easily go overlooked
  rtc.disableAlarm(2);
/*
  // schedule an alarm 30 seconds in the future for testing, this is for debug
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
*/   
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
 
  // Set desired pins to OUTPUT
  pinMode(SPEC_CLK, OUTPUT);
  pinMode(SPEC_ST, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(SAT_MODEM_SLEEP, OUTPUT);
  
  digitalWrite(SPEC_CLK, HIGH);           // Set SPEC_CLK High
  digitalWrite(SPEC_ST, LOW);             // Set SPEC_ST Low
///  pinMode(WHITE_LED, OUTPUT);

  // put sat modem to sleep
  digitalWrite(SAT_MODEM_SLEEP, LOW);     // Set sat modem to sleep, low, hard coded

  // power on spectrometers
  digitalWrite(RELAY, LOW);               // Turn Relay On, 5V spectrometer power enabled
  delay(100);                             // delay for spectrometers to stabilize
  
  // Mount the file system 
  LittleFS.begin(); 
  checkAvailableMemory();                 // check on available LittleFS memory
  
  // List directory, from root, 2 levels deep
////  Serial.println();                             // debug
////  listDir(LittleFS, "/", 2);                    // debug

  // Open Up file for reading
////  Serial.println();                             // debug
////  Serial.printf("Reading file: %s\r\n", path);  // debug
  File file = LittleFS.open(pathUp, "r");

  // Read from file and load calibration coefficient array
////  Serial.println();                             // debug
////  Serial.println("Reading from file:");         // debug
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
////  Serial.println();                              // debug
////  Serial.println("Reading from file:");          // debug
  j = 0;
  while (file.available()) 
  {
///    Serial.write(file.read());             // send file contents to serial port, debug
    String x = file.readStringUntil(',');     // read and place each coefficient in variable
    calCoefDwn[j] = x.toDouble();             // convert string to double float
////    Serial.print(x);  Serial.print("  "); Serial.println(calCoef[j], 20);       // debug
    j += 1;                                   // increment array index
  }
  file.close();                               // close little fs file
////  LittleFS.end();                             // stop littlespiffs to allow OTA updates

  printLocalTime();                           // print current local time and box internal temperature
  
  // check if upload switch is on, low state
  if (digitalRead(UPLOAD_SWITCH)==LOW)
  {
  // power off spectrometers
    digitalWrite(RELAY, HIGH);                // Turn Relay Off, 5V spectrometer power disabled
    uploadData();                             // upload data
  }

  // Print the wakeup reason for ESP32 
  Serial.println();
  print_wakeup_reason();
  if(wakeupType==false)                     // not RTC wakeup
  {
    GotoSleep = true;                       // set go to sleep flag
    deepSleep();                            // go to deep sleep
  }   
}
//----------------------------------------------------------------------


void loop()
{
  readSpectrometer();             // get spectrometer data
  buildBuffer();                  // fill the transmit buffer with byte data
  satModem();                     // transmit sat modem data in binary
  deepSleep();                    // enter deep sleep
}
//----------------------------------------------------------------------
