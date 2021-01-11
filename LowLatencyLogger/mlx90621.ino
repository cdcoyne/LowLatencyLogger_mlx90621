/* MLX90621 specific functionality to init and read data. 
Used specifically w/ SDFat library and the lowLatencyLogger example
included with that library */

#include <Wire.h>
#include "SdFat.h"
#include "UserTypes.h" /* for PIN_SS */

#include "mlx90621.h"

#define SAMPLE_RATE 64 //TODO move this to be configurable

/* sensor specific defines */
#define NUM_SENSORS_ROW 16
#define NUM_SENSORS_COL 4

#define QTY_DATA_PER_ROW_BYTES (NUM_SENSORS_ROW*2)/* 16 bit value per sensor */

#define SIZE_EEPROM_BYTES 256

/* communication protocol defines */
#define I2C_ADDR_EEPROM 0x50
#define I2C_ADDR_DATA 0x60

#define CMD_READ 0x02
#define CMD_WRITE_CONFIG 0x03
#define CMD_WRITE_TRIM 0x04

#define ADDR_REG_CONFIGURATION 0x92
#define ADDR_REG_TRIMMING 0x93

#define ADDR_RAM_IR_START 0x00
#define ADDR_RAM_PTAT 0x40
#define ADDR_RAM_COMPENSATION 0x41

static uint8_t eeprom[SIZE_EEPROM_BYTES];

/* calculate checksum val needed for communication w/ sensor */
static uint8_t getcheckval( uint8_t input ,uint8_t offset )
{
  uint16_t checkvalmath;

  checkvalmath = input < 0xaa ? 0x100 : 0;
  checkvalmath += input;
  checkvalmath -= offset;
  return (checkvalmath & 0xff);
}

/* initialize sensor and read out eeprom to SD card */
MLX_ERROR_T mlxInit( void )
{
  uint16_t bytesRx = 0;
  SdFat SD;

  Wire.begin();
  Wire.setClock(400000);

  /* per datasheet wait 5mS after POR */
  delay(5);

/* read eeprom from MLX */
  Wire.beginTransmission(I2C_ADDR_EEPROM);
  Wire.write(0x00);
  Wire.endTransmission();

  while ( bytesRx < SIZE_EEPROM_BYTES ) {
    /* seems like requestFrom doesn't like reading many bytes at once, so loop */
    Wire.requestFrom(I2C_ADDR_EEPROM,8);
    while (Wire.available() ){
      eeprom[bytesRx]=Wire.read();
      bytesRx++;
    }
  }

/* osc trim value */
  Wire.beginTransmission(I2C_ADDR_DATA);
  Wire.write(CMD_WRITE_TRIM);//command
  Wire.write(getcheckval(eeprom[0xf7],0xaa));
  Wire.write(eeprom[0xf7]);
  Wire.write(0x56);
  Wire.write(0x00);
  Wire.endTransmission();

//e = 1hz,d=2,c=4,b=8,a=16,9=32,8=64
#if SAMPLE_RATE == 64
#define CFG_LOW_BYTE 0x38
#elif SAMPLE_RATE == 32
#define CFG_LOW_BYTE 0x39
#endif

  /* write config reg */
  Wire.beginTransmission(I2C_ADDR_DATA);
  Wire.write(CMD_WRITE_CONFIG);//command
  Wire.write(getcheckval(CFG_LOW_BYTE,0x55));
  Wire.write(CFG_LOW_BYTE);
  Wire.write(0xf1);
  Wire.write(0x46);
  Wire.endTransmission();

  /* read config reg */
  Wire.beginTransmission(I2C_ADDR_DATA);
  Wire.write(CMD_READ);//command
  Wire.write(ADDR_REG_CONFIGURATION);
  Wire.write(0x00);//step
  Wire.write(0x01);//num reads
  Wire.endTransmission(false);
  if ( Wire.requestFrom(I2C_ADDR_DATA,2)!= 2 ) {
    return MLX_ERROR_READ_REG_CONFIG_LEN;
  }
  else {
    uint8_t checkval;
    checkval = Wire.read();//lb
    checkval = Wire.read();//hb
    if ( (checkval & 0x04) == 0 ){
      return MLX_ERROR_READ_REG_CONFIG_CHECK;
    }
  }

  /* save eeprom to SD card if needed */
  if ( !SD.begin(PIN_SS)){
    SD.initErrorPrint(&Serial);
  }
  else if ( !SD.exists("eeprom.txt") ){
    File eeFile;
    uint16_t i;
    eeFile = SD.open("eeprom.txt", O_CREAT | O_WRITE );
    eeFile.print(eeprom[0],HEX);
    for ( i = 1 ; i < SIZE_EEPROM_BYTES; i++ ){
      eeFile.print(',');
      eeFile.print(eeprom[i],HEX);
    }
    eeFile.close();
  }
  return MLX_ERROR_NONE;
}

/* complete read of the sensor data. 
    argument is a pointer to data array
    returns 0 on success or error > 0 */
MLX_ERROR_T mlxGetReading( uint8_t *data )
{
  uint16_t i;

  /* break up reads because requestFrom didnt like large quantities. 
    * the sensor memory is actually setup for column and then row so the loop 
    * isnt actually reading 1 row at a time, but at the end a complete read is done */
  for ( i = 0 ;i < NUM_SENSORS_COL; i++ ){
    Wire.beginTransmission(I2C_ADDR_DATA);
    Wire.write(CMD_READ);//command
    Wire.write(NUM_SENSORS_ROW*i);//address
    Wire.write(0x01);//addr step
    Wire.write(NUM_SENSORS_ROW);//num reads
    Wire.endTransmission(false);

    if ( Wire.requestFrom(I2C_ADDR_DATA,QTY_DATA_PER_ROW_BYTES) != QTY_DATA_PER_ROW_BYTES ){
      return MLX_ERROR_READ_IR_DATA;
    }
    else {
      int j;
      for (j =0;j<QTY_DATA_PER_ROW_BYTES;j++){
        data[j+(i*QTY_DATA_PER_ROW_BYTES)] = Wire.read();
      }
    }
  }

  /* read PTAT and compensation */
  Wire.beginTransmission(I2C_ADDR_DATA);
  Wire.write(CMD_READ);//command
  Wire.write(ADDR_RAM_PTAT);//address
  Wire.write(0x01);//addr step
  Wire.write(0x02);//num reads
  Wire.endTransmission(false);
  if ( Wire.requestFrom(I2C_ADDR_DATA,4) != 4 ){
    return MLX_ERROR_READ_PTAT;
  }
  else {
    data[0x80] = Wire.read();
    data[0x81] = Wire.read();
    data[0x82] = Wire.read();
    data[0x83] = Wire.read();
  }

  return MLX_ERROR_NONE;
}
