/**
 * @file MPU6500.c
 * @brief Implementation of the MPU-6500 accelerometer and gyroscope driver.
 *
 * This source file contains the function implementations for initializing the
 * MPU-6500 device, configuring power modes, setting sampling parameters, and
 * reading raw sensor outputs. It provides the low-level I2C communication logic
 * required to interact with the MPU-6500 register map.
 *
 * The driver supports:
 *  - Device initialization and ID verification
 *  - Power management and low-power accelerometer mode
 *  - Temperature sensor reading and control
 *  - Gyroscope and accelerometer full-scale configuration
 *  - Raw data acquisition for all axes
 *
 * @author  iek
 * @date    December 09, 2025
 */

#include "mpu6500.h"

/**
 * @brief Initializes the MPU-6500 with default configurations.
 * @retval mpu_status_t: Status code (MPU_OK = 1 means success)
 */
mpu_status_t MPU6500_Init(void) {
	uint8_t check;
	uint8_t Data;


	if (HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_WHO_AM_I,
	I2C_MEMADD_SIZE_8BIT, &check, 1, 100) != HAL_OK) {
		return MPU_ERR_I2C;
	}


	if (check != MPU6500_WHO_AM_I_VAL) {
		return MPU_ERR_DEVICE_ID;
	}


	Data = 0x00;
	if (HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_1,
	I2C_MEMADD_SIZE_8BIT, &Data, 1, 100) != HAL_OK) {
		return MPU_ERR_I2C;
	}

	return MPU_OK;
}

/**
 * @brief Sets the power mode for the MPU-6500.
 * @param mode Power mode selection.
 * @see MPU_PowerMode
 */
void MPU6500_Set_PowerMode(MPU_PowerMode mode) {
	uint8_t data_pwr1 = 0;
	uint8_t data_pwr2 = 0;

	if (mode == MPU_MODE_SLEEP) {
		data_pwr1 = MPU_BIT_SLEEP;
		HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_1,
		I2C_MEMADD_SIZE_8BIT, &data_pwr1, 1, 100);
	} else if (mode == MPU_MODE_NORMAL) {

		data_pwr1 = MPU_CLK_PLL_AUTO;
		HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_1,
		I2C_MEMADD_SIZE_8BIT, &data_pwr1, 1, 100);


		data_pwr2 = 0x00;
		HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_2,
		I2C_MEMADD_SIZE_8BIT, &data_pwr2, 1, 100);
	} else if (mode == MPU_MODE_LOW_POWER_ACC) {

		data_pwr1 = MPU_BIT_CYCLE | MPU_BIT_TEMP_DIS | MPU_CLK_PLL_AUTO;
		HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_1,
		I2C_MEMADD_SIZE_8BIT, &data_pwr1, 1, 100);


		data_pwr2 = 0xC0 | MPU_STBY_XG | MPU_STBY_YG | MPU_STBY_ZG;
		HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_2,
		I2C_MEMADD_SIZE_8BIT, &data_pwr2, 1, 100);
	}
}

/**
 * @brief Sets the sensor sample rate.
 * @param frequency_hz Desired sampling frequency (4Hz - 1000Hz).
 */
void MPU6500_Set_SampleRate(uint16_t frequency_hz) {
	uint8_t dlpf_cfg = MPU_DLPF_CFG_1;
	uint8_t smplrt_div;

	if (frequency_hz < 4)
		frequency_hz = 4;
	if (frequency_hz > 1000)
		frequency_hz = 1000;

	smplrt_div = (1000 / frequency_hz) - 1;


	HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_CONFIG,
	I2C_MEMADD_SIZE_8BIT, &dlpf_cfg, 1, 100);


	HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_SMPLRT_DIV,
	I2C_MEMADD_SIZE_8BIT, &smplrt_div, 1, 100);
}

/**
 * @brief Reads the temperature value from the sensor.
 * @retval Temperature in Celsius as float.
 */
float MPU6500_Read_Temperature(void) {
	uint8_t Rec_Data[2];
	int16_t temp_raw;
	float temperature_c;


	HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_TEMP_OUT_H,
	I2C_MEMADD_SIZE_8BIT, Rec_Data, 2, 100);

	temp_raw = (int16_t) (Rec_Data[0] << 8 | Rec_Data[1]);

	temperature_c = ((float) temp_raw / 333.87f) + 21.0f;

	return temperature_c;
}

/**
 * @brief Enables or disables the temperature sensor.
 * @param state 1 = Enable, 0 = Disable.
 */
void MPU6500_Set_Temperature_Sensor(uint8_t state) {
	uint8_t current_reg;

	HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_1,
	I2C_MEMADD_SIZE_8BIT, &current_reg, 1, 100);

	if (state == 1) {
		current_reg &= ~MPU_BIT_TEMP_DIS;
	} else {
		current_reg |= MPU_BIT_TEMP_DIS;
	}

	HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_1,
	I2C_MEMADD_SIZE_8BIT, &current_reg, 1, 100);
}

/**
 * @brief Enables or disables a specific axis (standby control).
 * @param axis Axis selection.
 * @param state 1 = Enable, 0 = Standby.
 * @see MPU_axis_standby_t
 */
void MPU6500_Set_Axis_Standby(MPU_axis_standby_t axis_mask, uint8_t state) {
	uint8_t current_reg;

	HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_2,
	I2C_MEMADD_SIZE_8BIT, &current_reg, 1, 100);

	if (state == 1) {
		current_reg &= ~(axis_mask);
	} else {
		current_reg |= axis_mask;
	}

	HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_PWR_MGMT_2,
	I2C_MEMADD_SIZE_8BIT, &current_reg, 1, 100);
}

/**
 * @brief Sets the gyroscope full-scale sensitivity range.
 * @param fs_range Gyroscope full-scale range.
 * @see MPU_gyro_fs_t
 */
void MPU6500_Set_Gyro_Sensitivity(MPU_gyro_fs_t fs_range) {
	uint8_t current_reg;

	HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_GYRO_CONFIG,
	I2C_MEMADD_SIZE_8BIT, &current_reg, 1, 100);
	current_reg &= 0xE7;
	current_reg |= fs_range;
	HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_GYRO_CONFIG,
	I2C_MEMADD_SIZE_8BIT, &current_reg, 1, 100);
}

/**
 * @brief Sets the accelerometer full-scale sensitivity range.
 * @param fs_range Accelerometer full-scale range.
 * @see MPU_accel_fs_t
 */
void MPU6500_Set_Accel_Sensitivity(MPU_accel_fs_t fs_range) {
	uint8_t current_reg;

	HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_ACCEL_CONFIG,
	I2C_MEMADD_SIZE_8BIT, &current_reg, 1, 100);
	current_reg &= 0xE7;
	current_reg |= fs_range;
	HAL_I2C_Mem_Write(MPU_Port, MPU6500_ADDR, MPU_REG_ACCEL_CONFIG,
	I2C_MEMADD_SIZE_8BIT, &current_reg, 1, 100);
}

/**
 * @brief  读取未经任何处理的原始传感器数据。
 * @param  pData: 指向用于存储结果的结构体的指针。
 * @param  mode: 数据读取模式选择。
 * @see    MPU6500_RawData_
 * @see    MPU_read_mode_t
 */
void MPU6500_Read_Raw(MPU6500_RawData_t *pData, MPU_read_mode_t mode) {
	uint8_t buffer[14];

	switch (mode) {

	case MPU_READ_ACCEL_X:
		HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_ACCEL_XOUT_H,
		I2C_MEMADD_SIZE_8BIT, buffer, 2, 100);
		pData->Accel_X = (int16_t) (buffer[0] << 8 | buffer[1]);
		break;

	case MPU_READ_ACCEL_Y:
		HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_ACCEL_YOUT_H,
		I2C_MEMADD_SIZE_8BIT, buffer, 2, 100);
		pData->Accel_Y = (int16_t) (buffer[0] << 8 | buffer[1]);
		break;

	case MPU_READ_ACCEL_Z:
		HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_ACCEL_ZOUT_H,
		I2C_MEMADD_SIZE_8BIT, buffer, 2, 100);
		pData->Accel_Z = (int16_t) (buffer[0] << 8 | buffer[1]);
		break;

	case MPU_READ_ACCEL_ALL:
		HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_ACCEL_XOUT_H,
		I2C_MEMADD_SIZE_8BIT, buffer, 6, 100);
		pData->Accel_X = (int16_t) (buffer[0] << 8 | buffer[1]);
		pData->Accel_Y = (int16_t) (buffer[2] << 8 | buffer[3]);
		pData->Accel_Z = (int16_t) (buffer[4] << 8 | buffer[5]);
		break;

	case MPU_READ_GYRO_X:
		HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_GYRO_XOUT_H,
		I2C_MEMADD_SIZE_8BIT, buffer, 2, 100);
		pData->Gyro_X = (int16_t) (buffer[0] << 8 | buffer[1]);
		break;

	case MPU_READ_GYRO_ALL:
		HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_GYRO_XOUT_H,
		I2C_MEMADD_SIZE_8BIT, buffer, 6, 100);
		pData->Gyro_X = (int16_t) (buffer[0] << 8 | buffer[1]);
		pData->Gyro_Y = (int16_t) (buffer[2] << 8 | buffer[3]);
		pData->Gyro_Z = (int16_t) (buffer[4] << 8 | buffer[5]);
		break;

	case MPU_READ_ALL_SENSORS:
		HAL_I2C_Mem_Read(MPU_Port, MPU6500_ADDR, MPU_REG_ACCEL_XOUT_H,
		I2C_MEMADD_SIZE_8BIT, buffer, 14, 100);

		pData->Accel_X = (int16_t) (buffer[0] << 8 | buffer[1]);
		pData->Accel_Y = (int16_t) (buffer[2] << 8 | buffer[3]);
		pData->Accel_Z = (int16_t) (buffer[4] << 8 | buffer[5]);

		pData->Gyro_X = (int16_t) (buffer[8] << 8 | buffer[9]);
		pData->Gyro_Y = (int16_t) (buffer[10] << 8 | buffer[11]);
		pData->Gyro_Z = (int16_t) (buffer[12] << 8 | buffer[13]);
		break;

	default:
		break;
	}
}
