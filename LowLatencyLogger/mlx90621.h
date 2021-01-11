/* mlx90621 interface header file */
#ifndef MLX90621_H
#define MLX90621_H

typedef enum {
    MLX_ERROR_NONE,
    MLX_ERROR_READ_REG_CONFIG_LEN,
    MLX_ERROR_READ_REG_CONFIG_CHECK,
    MLX_ERROR_READ_IR_DATA,
    MLX_ERROR_READ_PTAT
} MLX_ERROR_T;

MLX_ERROR_T mlxInit( void );
MLX_ERROR_T mlxGetReading( uint8_t *data );

#endif 