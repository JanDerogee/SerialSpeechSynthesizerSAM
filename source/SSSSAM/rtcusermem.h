#ifndef _RTCUSERMEM_H_
#define _RTCUSERMEM_H_

/*---------------------------------------------*/

/*a simple struct to hold all variables to must survive a reset*/
// Structure which will be stored in RTC memory. First field is CRC32, which is calculated based on the
// rest of structure contents. Any fields can go after CRC32. We use byte array as an example.
typedef struct
{
  uint32_t crc32;
  uint32_t counter;
  byte data[504];
} rtcData_TYPE;

extern rtcData_TYPE rtcData;  /*structure holding all the settings that should be available to all callers who includes this .h file*/

/*---------------------------------------------*/

void WritePersistentStruct(void);
unsigned char ReadPersistentStruct(void);
uint32_t calculateCRC32(const uint8_t *data, size_t length);  /*CRC function used to ensure data validity*/

/*---------------------------------------------*/

#endif
