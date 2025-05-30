// Build binary data and write to buffer
void buildBuffer()
{
  if (FillTransmitBuffer==true)
  {
  // the sat modem binary transmit starts the array at 0
  // encode into two bytes high byte, low byte
    int j = 0;                                        // initialize buffer index
   
    for(int i = DATASTART; i < DATAEND; i++)
    {
      bufferUp[j] = highByte(dataUp[i]);
      bufferUp[j+1] = lowByte(dataUp[i]);
      Serial.print(i);  Serial.print("  "); Serial.print(dataUp[i]);  Serial.println();
      Serial.print(bufferUp[j]); Serial.print("  "); Serial.println(bufferUp[j+1]); 
  
  // decode back to int value
///    decoded_Buffer = (buffer[j] << 8)+ buffer[j+1];    // left shift low byte 8 bits and add to high byte
///    Serial.println(decoded_Buffer);  Serial.println();
      j+=2;                                           // increment buffer index
    }
    Serial.println();

    j = 0;                                            // reset buffer index
    
    // encode down channel into two bytes high byte, low byte
    for(int i = DATASTART; i < DATAEND; i++)
    {
      bufferDwn[j] = highByte(dataDwn[i]);
      bufferDwn[j+1] = lowByte(dataDwn[i]);
      Serial.print(i);  Serial.print("  "); Serial.print(dataDwn[i]);  Serial.println();
      Serial.print(bufferDwn[j]); Serial.print("  "); Serial.println(bufferDwn[j+1]); 
  
  // decode back to int word value
///    decoded_Buffer = (buffer[j] << 8)+ buffer[j+1];    // left shift low byte 8 bits and add to high byte
///    Serial.println(decoded_Buffer);  Serial.println();
      j+=2;                                           // increment buffer index
    }
    dataSize = j;                                     // size of data in buffer
    FillTransmitBuffer = false;                       // clear transmit flag
    TransmitData = true;                              // set transmit data flag   
  }  
}
//-----------------------------------------------------------------
 
