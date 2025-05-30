// this automatic gain control (AGC) adjust the integration time based on the intensity
void AGC()
{
  // find the maximum intensity for both channels
  for(int i = 1; i < SPEC_CHANNELS; i++)                // loop for number of channel bins (288)
  {
    if (dataUpMax<dataUp[i])
    {
      dataUpMax = dataUp[i];                            // set to higher value
    }
    if (dataDwnMax<dataDwn[i])
    {
      dataDwnMax = dataDwn[i];                          // set to higher value
    }
  }
  Serial.println("Max Values");
  Serial.print(dataUpMax);  Serial.print("  ");Serial.println(dataDwnMax);
  
  // determine required  integration time from intensity
  AdjintegrationTimeUp = integrationTimeUp;
  if (dataUpMax>7200)                                   // set limit just below saturation
  {
    AdjintegrationTimeUp=integrationTimeUp-1;
  }
  else if (AdjintegrationTimeUp==0)
  {
    if (7000/dataUpMax!=1) AdjintegrationTimeUp=(7000.0/dataUpMax);
  }
  else
  {
    if (int((7000.0/dataUpMax)*integrationTimeUp)!=AdjintegrationTimeUp)
    {
      AdjintegrationTimeUp = 1+int((7000.0/dataUpMax)*integrationTimeUp); 
    }
  }

  // determine required  integration time from intensity
  AdjintegrationTimeDwn = integrationTimeDwn;
  if (dataDwnMax>7200)                                  // set limit just below saturation
  {
    AdjintegrationTimeDwn=integrationTimeDwn-1;
  }
  else if (AdjintegrationTimeDwn==0)
  {
    if (7000/dataDwnMax!=1) AdjintegrationTimeDwn=(7000.0/dataDwnMax);
  }
  else
  {
    if (int((7000.0/dataDwnMax)*integrationTimeDwn)!=AdjintegrationTimeDwn)
    {
      AdjintegrationTimeDwn = 1+int((7000.0/dataDwnMax)*integrationTimeDwn); 
    }
  }
  
  // check if integration time changed
  if (AdjintegrationTimeUp<0 || AdjintegrationTimeUp>20) AdjintegrationTimeUp=integrationTimeUp;
  if (integrationTimeUp!=AdjintegrationTimeUp)
  {
    Serial.println("Change Up");
    integrationTimeUp = AdjintegrationTimeUp;
    changeIntegrationUp = true;                         // set change integration flag    
  }
  else
  {
    Serial.println("No Change Up");
    changeIntegrationUp = false;                         // clear change integration flag
  }

  if (AdjintegrationTimeDwn<0 || AdjintegrationTimeDwn>20) AdjintegrationTimeDwn=integrationTimeDwn;
  if (integrationTimeDwn!=AdjintegrationTimeDwn)
  {
    Serial.println("Change Dwn");
    integrationTimeDwn = AdjintegrationTimeDwn;
    changeIntegrationDwn = true;                         // set change integration flag    
  }
  else
  {
    Serial.println("No Change Dwn");
    changeIntegrationDwn = false;                         // clear change integration flag
  }
    
  Serial.println("Integration Time");
  Serial.print(integrationTimeUp);  Serial.print("  ");Serial.println(integrationTimeDwn);
  Serial.println();
  dataUpMax = 0;                                          // clear max up value
  dataDwnMax = 0;                                         // clear max dwn value
}
//-----------------------------------------------------------------
