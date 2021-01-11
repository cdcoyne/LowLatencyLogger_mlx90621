#ifndef UserTypes_h
#define UserTypes_h
#include "Arduino.h"
// User data types.  Modify for your data items.

/* pin defines */
#define PIN_SS 4
#define PIN_ERROR_LED 8
#define PIN_SWITCH_INPUT 6

//#define FILE_BASE_NAME "adc4pin"
#define SENSOR_DIM 66 /* 64 ir elements, PTAT, Compensation */
struct data_t  {
  unsigned long time;
  union{
    uint8_t u8data[SENSOR_DIM*2];
    int16_t i16data[SENSOR_DIM];
  };
};
void acquireData(data_t* data);
void printData(Print* pr, data_t* data);
void printHeader(Print* pr);
void userSetup();

#define SWITCH_RECORD_ON (digitalRead(PIN_SWITCH_INPUT) == 1)
#define SWITCH_RECORD_OFF (digitalRead(PIN_SWITCH_INPUT) == 0)


#endif  // UserTypes_h
