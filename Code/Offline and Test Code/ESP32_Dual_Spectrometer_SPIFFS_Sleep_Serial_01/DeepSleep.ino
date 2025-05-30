// The function checks for the scan to be complete and then enters deep sleep
void deepSleep()
{
  if (EnableScan==false)
  {  
  // go into deep sleep mode, all processes are complete
    Serial.println("Going to sleep now");                            // debug
    esp_deep_sleep_start();                   // start deep sleep
  } 
}
//----------------------------------------------------------------------
