// print the local time
void printLocalTime()
{
  DateTime now = rtc.now();
/*
  // the stored alarm value
  char alarm1Date[10] = "hh:mm:ss";
  DateTime alarm1 = rtc.getAlarm1();
  alarm1.toString(alarm1Date);
  Serial.print("Alarm1 Time: ");
  Serial.print(alarm1Date);
  
  // print date and time
  Serial.print("Current Date and Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
*/
  // create timestamp for file
  fileDate = now.year();
  fileDate += '/';
  fileDate += now.month();
  fileDate += '/';
  fileDate += now.day();
  fileDate += " ";
  fileDate += now.hour();
  fileDate += ':';
  fileDate += now.minute();
  fileDate += ':';
  fileDate += now.second();
  Serial.println(fileDate);
  
  // print temperature of RTC module
  Serial.print("Temperature: ");
  Serial.print(rtc.getTemperature());
  Serial.println(" C");
  Serial.println();
} 
//-----------------------------------------------------------------
