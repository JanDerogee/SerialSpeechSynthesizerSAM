// Struct data with the maximum size of 512 bytes can be stored
// in the RTC user rtcDataory using the ESP-specifc APIs.
// The stored data can be retained between deep sleep cycles.
// However, the data WILL be lost after power cycling the ESP8266.

/*the code here is derived from the example: https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/RTCUserMemory/RTCUserMemory.ino  */

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <Arduino.h>      /*important to avoid all sorts of strange error messages*/
#include "rtcusermem.h"   /*the RTC user mem is RAM unaffected by reset (perfect for short term storage*/


/*-------------------------------------------------*/

rtcData_TYPE rtcData;  /*structure holding all the settings that should be available to all callers who includes this .h file*/

/*-------------------------------------------------*/

/*Write struct to RTC memory*/
void WritePersistentStruct(void)
{
  rtcData.crc32 = calculateCRC32((uint8_t*) &rtcData.data[0], sizeof(rtcData.data));    /*Update CRC32 of data*/
  ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData));                     /*Write struct to RTC memory*/  
}

/*Read struct from RTC memory*/
unsigned char ReadPersistentStruct(void)
{  
  ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData));
  uint32_t crcOfData = calculateCRC32((uint8_t*) &rtcData.data[0], sizeof(rtcData.data));
  if (crcOfData != rtcData.crc32)
  {
    //Serial.println("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably invalid!");
    return(false);
  }
  else
  {
    //Serial.println("CRC32 check ok, data is probably valid.");
    return(true);
  }  
}


/*calculation of checksum value*/
uint32_t calculateCRC32(const uint8_t *data, size_t length)
{ 
  uint32_t crc;
  bool bit;
  uint8_t c;
  uint32_t i;

  crc = 0xffffffff;
  while (length--)
  {
    c = *data++;
    for (i=0x80; i>0; i>>=1)
    {
      bit = crc & 0x80000000;
      if (c & i)  {bit = !bit;}     
      crc <<= 1;
      if (bit)    {crc ^= 0x04c11db7;}
    }
  }
  return crc;
}
