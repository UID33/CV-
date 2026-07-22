/**
 * @file MPU6500.h
 * @brief Driver interface for the MPU-6500 6-axis accelerometer and gyroscope module.
 *
 * This header provides register definitions, data structures, enumerations, and
 * function prototypes required for configuring and reading sensor data from the
 * MPU-6500 IMU device. The driver supports power management control, full-scale
 * sensitivity configuration, temperature reading, and raw sensor data acquisition.
 *
 * @author iek
 * @date   December 09, 2025
 */

#ifndef __MPU6500_H
#define __MPU6500_H

/**
 * @brief  HAL library include for STM32 platform.
 *
 * @note   The user must include the correct HAL header file based on their MCU family.
 *         For example:
 *         @code
 *         #include "stm32g0xx_hal.h"   // For STM32G0 series
 *         #include "stm32f4xx_hal.h"   // For STM32F4 series
 *         #include "stm32l4xx_hal.h"   // For STM32L4 series
 *         @endcode
 *         Make sure to match this include with your STM32 device.
 */
#include "stm32f1xx_hal.h"

/**
 * @brief  I2C handle used for communication with the HMC5883L sensor.
 *
 * @note   The user must define the I2C handle according to the hardware setup.
 *         For example, if I2C1 is used:
 *         @code
 *         extern I2C_HandleTypeDef hi2c1;
 *         @endcode
 *         Replace 'hi2c1' with the appropriate I2C handle name used in your project.
 */
extern I2C_HandleTypeDef hi2c2;

/**
 * @brief Selects the I2C interface used by the MPU6500 driver.
 *
 * Change this definition if the MPU6500 is connected to another I2C peripheral.
 * Example: &hi2c2 or &hi2c3 depending on your hardware configuration.
 */
#define MPU_Port   &hi2c2

/**
 * @brief MPU6500 I2C address selection.
 *
 * If the AD0 pin is tied to GND, the address becomes 0x68.
 * If the AD0 pin is tied to VCC, define AD0_ON to use address 0x69.
 *
 * AD0 = 0 → 0x68
 * AD0 = 1 → 0x69
 */
#ifdef AD0_ON
#define MPU6500_ADDR 	(0x69 << 1)
#else
#define MPU6500_ADDR 	(0x68 << 1)
#endif

/**
 * @brief MPU-6500 Specific Who Am I Value
 * Register 117 (0x75) default value is 0x70
 */
#define MPU6500_WHO_AM_I_VAL   0x70

/* --- PWR_MGMT_1 (0x6B) Register Bit Definitions --- */
#define MPU_BIT_DEVICE_RESET   0x80
#define MPU_BIT_SLEEP          0x40
#define MPU_BIT_CYCLE          0x20
#define MPU_BIT_TEMP_DIS       0x08

/* Clock Source Options */
#define MPU_CLK_INTERNAL       0x00
#define MPU_CLK_PLL_AUTO       0x01

/* ---------------------------------------------------------------------------
 * MPU-6500 Register Map Definitions
 * Source: MPU-6000/MPU-6500 Register Map Document
 * --------------------------------------------------------------------------- */

/* --- Configuration Registers --- */
#define MPU_REG_SMPLRT_DIV      0x19
#define MPU_REG_CONFIG          0x1A
#define MPU_REG_GYRO_CONFIG     0x1B
#define MPU_REG_ACCEL_CONFIG    0x1C
#define MPU_REG_ACCEL_CONFIG_2  0x1D

/* --- Accelerometer Data Registers (Read-Only) --- */
#define MPU_REG_ACCEL_XOUT_H    0x3B
#define MPU_REG_ACCEL_XOUT_L    0x3C
#define MPU_REG_ACCEL_YOUT_H    0x3D
#define MPU_REG_ACCEL_YOUT_L    0x3E
#define MPU_REG_ACCEL_ZOUT_H    0x3F
#define MPU_REG_ACCEL_ZOUT_L    0x40

/* --- Temperature Data Registers (Read-Only) --- */
#define MPU_REG_TEMP_OUT_H      0x41
#define MPU_REG_TEMP_OUT_L      0x42

/* --- Gyroscope Data Registers (Read-Only) --- */
#define MPU_REG_GYRO_XOUT_H     0x43
#define MPU_REG_GYRO_XOUT_L     0x44
#define MPU_REG_GYRO_YOUT_H     0x45
#define MPU_REG_GYRO_YOUT_L     0x46
#define MPU_REG_GYRO_ZOUT_H     0x47
#define MPU_REG_GYRO_ZOUT_L     0x48

/* --- Power Management Registers --- */
#define MPU_REG_PWR_MGMT_1      0x6B
#define MPU_REG_PWR_MGMT_2      0x6C
#define MPU_REG_WHO_AM_I        0x75

/* DLPF Config */
#define MPU_DLPF_CFG_1          0x01  // ~184Hz bandwidth

/*
 * Structure that holds only RAW (unscaled) data.
 */
typedef struct {
	int16_t Accel_X;
	int16_t Accel_Y;
	int16_t Accel_Z;
	int16_t Gyro_X;
	int16_t Gyro_Y;
	int16_t Gyro_Z;
} MPU6500_RawData_t;

/* Read modes */
typedef enum {
	MPU_READ_ACCEL_X,
	MPU_READ_ACCEL_Y,
	MPU_READ_ACCEL_Z,
	MPU_READ_ACCEL_ALL,
	MPU_READ_GYRO_X,
	MPU_READ_GYRO_Y,
	MPU_READ_GYRO_Z,
	MPU_READ_GYRO_ALL,
	MPU_READ_ALL_SENSORS
} MPU_read_mode_t;

/* Status codes */
typedef enum {
	MPU_OK = 1,
	MPU_ERR_I2C = 0,
	MPU_ERR_DEVICE_ID = -1,
	MPU_ERR_TIMEOUT = -2,
	MPU_ERR_PARAM = -3
} mpu_status_t;

/* Power mode selection */
typedef enum {
	MPU_MODE_SLEEP,
	MPU_MODE_NORMAL,
	MPU_MODE_LOW_POWER_ACC
} MPU_PowerMode;

/* Standby masks for PWR_MGMT_2 */
typedef enum {
	MPU_STBY_ZG = 0x01,
	MPU_STBY_YG = 0x02,
	MPU_STBY_XG = 0x04,
	MPU_STBY_ZA = 0x08,
	MPU_STBY_YA = 0x10,
	MPU_STBY_XA = 0x20,
} MPU_axis_standby_t;

/* Gyroscope Full Scale */
typedef enum {
	MPU_GYRO_FS_250 = 0x00,
	MPU_GYRO_FS_500 = 0x08,
	MPU_GYRO_FS_1000 = 0x10,
	MPU_GYRO_FS_2000 = 0x18
} MPU_gyro_fs_t;

/* Accelerometer Full Scale */
typedef enum {
	MPU_ACCEL_FS_2G = 0x00,
	MPU_ACCEL_FS_4G = 0x08,
	MPU_ACCEL_FS_8G = 0x10,
	MPU_ACCEL_FS_16G = 0x18
} MPU_accel_fs_t;

/* Function Prototypes */
mpu_status_t MPU6500_Init(void);
void MPU6500_Set_PowerMode(MPU_PowerMode mode);
void MPU6500_Set_SampleRate(uint16_t frequency_hz);
float MPU6500_Read_Temperature(void);
void MPU6500_Set_Temperature_Sensor(uint8_t state);
void MPU6500_Set_Axis_Standby(MPU_axis_standby_t axis_mask, uint8_t state);
void MPU6500_Set_Accel_Sensitivity(MPU_accel_fs_t fs_range);
void MPU6500_Set_Gyro_Sensitivity(MPU_gyro_fs_t fs_range);
void MPU6500_Read_Raw(MPU6500_RawData_t *pData, MPU_read_mode_t mode);

#endif /* MPU_6500_H_ */
