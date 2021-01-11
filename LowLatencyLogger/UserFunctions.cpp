#include "UserTypes.h"
#include "mlx90621.h"
// User data functions.  Modify these functions for your data items.

// Start time for data
static uint32_t startMicros;

// Acquire a data record.
void acquireData(data_t* data) {
  data->time = micros() - startMicros;
  if( mlxGetReading(data->u8data) != MLX_ERROR_NONE ){
    // TODO Add Error
  }
}

// Print a data record.
void printData(Print* pr, data_t* data) {
  if (startMicros == 0) {
    startMicros = data->time;
  }
  pr->print(data->time/* - startMicros*/);
  for (int i = 0; i < SENSOR_DIM; i++) {
    pr->write(',');
    pr->print(data->i16data[i]);
  }
  pr->println();
}

// Print data header.
void printHeader(Print* pr) {
  startMicros = 0;
  pr->print(F("micros"));
  for (int i = 0; i < SENSOR_DIM; i++) {
    pr->print(F(",adc"));
    pr->print(i);
  }
  pr->println();
}

// Sensor setup
void userSetup() {
  mlxInit();
}

