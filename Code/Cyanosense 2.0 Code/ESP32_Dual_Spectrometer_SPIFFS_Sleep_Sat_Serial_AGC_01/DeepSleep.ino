// The function checks for the scan to be complete and then enters deep sleep
void deepSleep()
{
  if (GotoSleep==true)
  {  
  // go into deep sleep mode, all processes are complete
    Serial.println("Going to sleep now"); Serial.println();        // debug
    digitalWrite(RELAY, HIGH);              // make sure spectrometers are off
    gpio_hold_en(GPIO_NUM_19);              // set sat modem deep sleep pin to hold state
    // gpio_deep_sleep_hold_en();              // activate sat modem deep sleep pin hold
    // gpio_deep_sleep_hold_dis();              // activate sat modem deep sleep pin unhold
    esp_deep_sleep_start();                 // start deep sleep
  } 
}
//----------------------------------------------------------------------

// Method to print the reason by which ESP32 has been awaken from sleep
void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO");
      wakeupType = true; 
      break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
//----------------------------------------------------------------------
