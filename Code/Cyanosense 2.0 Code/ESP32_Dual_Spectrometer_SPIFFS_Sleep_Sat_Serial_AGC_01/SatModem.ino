// sat modem initialize and transmit
// if the sat modem does not respond this will loop forever but it most likely means something is dead
void satModem()
{
  if (TransmitData==true)
  {
  // Begin satellite modem operation
    Serial.print("Waking up Modem");
    gpio_hold_dis(GPIO_NUM_19);              // set sat modem deep sleep pin to unhold state
    modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);     // DEFAULT_POWER_PROFILE or USB_POWER_PROFILE
    err = modem.begin();                                      // wake up modem
    if (err != ISBD_SUCCESS)
    {
      Serial.print("Couldn't begin modem operations. Error: ");
      Serial.println(err);
      if (err == ISBD_NO_MODEM_DETECTED)
        Serial.println("No modem detected: Check Wiring.");
      return;
    }
  
  // Check the signal quality 
  // This returns a number between 0 and 5.
  // 2 or better is preferred but it will work a 0.
    err = modem.getSignalQuality(signalQuality);
    if (err != ISBD_SUCCESS)   
    {
      Serial.print("Signal Quality Failed. Error: ");
      Serial.println(err);
      return;
    }
    Serial.print("On a scale of 0 to 5, signal quality is currently: ");
    Serial.println(signalQuality);

  // Send the message
    Serial.print("Trying to send one of two messages. This might take several minutes.\r\n");
    err = modem.sendSBDBinary(bufferUp, dataSize);        // sky sensor
    if (err != ISBD_SUCCESS)
    {
      Serial.print("sendSBDBinary Send failed: error ");
      Serial.println(err);
      if (err == ISBD_SENDRECEIVE_TIMEOUT)
        Serial.println("First message transmit failure, try again with a better view of the sky.");
        Serial.println("Abort second data transmission.");
    }
    else
    {
      Serial.println("First message transmitted successfully!");
      Serial.print("Trying to send two of two messages. This might take several minutes.\r\n");
      err = modem.sendSBDBinary(bufferDwn, dataSize);        // water sensor
      if (err != ISBD_SUCCESS)
      {
        Serial.print("sendSBDBinary Send failed: error ");
        Serial.println(err);
        if (err == ISBD_SENDRECEIVE_TIMEOUT)
          Serial.println("Second message transmit failure, try again with a better view of the sky.");
      }
      else
      {
        Serial.println("Second message transmitted successfully!");
      }
    }
      
  // Put modem to sleep
    Serial.println("Putting modem to sleep.");
    err = modem.sleep();
    if (err != ISBD_SUCCESS)
    {
      Serial.print("Sleep Failed. Error: ");
      Serial.println(err);
    } 
    TransmitData = false;                       // clear transmit data flag
    GotoSleep = true;                           // set go to deep sleep flag
  }
}
//----------------------------------------------------------------------
