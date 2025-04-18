/*
 * device_sourcing.c
 *
 *  Created on: Mar 6, 2025
 *      Author: User
 */
#include "main.h"
#include "device_constants.h"
#include "device_sourcing.h"


extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

void set_ad9833_dds_buffer(uint8_t *buffer, uint32_t freq_28b) {
	// Combine 14 MSBs and 14 LSBs
//    uint32_t freq_28b = ((freq_14msb & 0x3FFF) << 14) | (freq_14lsb & 0x3FFF);
    // Populate the buffer according to the command sequence
    buffer[0] = 0x21; buffer[1] = 0x00; // Control Register
    buffer[2] = 0x40 | ((freq_28b >> 8) & 0x3F); // Frequency Register 0 LSB
    buffer[3] = freq_28b & 0xFF;
    buffer[4] = 0x40 | ((freq_28b >> 22) & 0x3F); // Frequency Register 0 MSB
    buffer[5] = (freq_28b >> 14) & 0xFF;
    buffer[6] = 0xC0; buffer[7] = 0x00; // Phase Register 0
    buffer[8] = 0x20; buffer[9] = 0x00; // Exit Reset
}

void set_pot_buffer(uint8_t *buffer, uint32_t cmd, uint32_t value) {
    buffer[0] = cmd;
    buffer[1] = value;
}

int calculate_pot_value_curr_mirr(float Current) {
	// Function to calculate pot_value
    // Define the constants
//    const float constant = 10.319;
//    const float exponent = -1.031;
//    const float offset = 255;
    const float constant = 15.682;
    const float exponent = -0.9434;
    const float offset = 255;
    // Calculate pot_value using the provided formula
    float pot_value = offset - pow(Current/constant/1000.0, exponent);

    // Return the calculated pot_value
    return (int)round(pot_value);
}

int calculate_pot_value_volt_offset(float voltage) {
	// Function to calculate pot_value
    // Define the constants
    const float constant = 4.8155;
    const float divisor = 0.038;

    // Calculate pot_value using the provided formula
    float pot_value = (constant - voltage)/divisor;

    // Return the calculated pot_value
    return (int)round(pot_value);
}

int calculate_pot_value_volt_gain(float voltage) {
	// Function to calculate pot_value
    // Define the constants
    const float constant = 4.2703;
    const float divisor = 0.0315;

    // Calculate pot_value using the provided formula
    float pot_value = (constant - voltage)/divisor;

    // Return the calculated pot_value
    return (int)round(pot_value);
}

HAL_StatusTypeDef config_dds_freq(SPI_HandleTypeDef *hspi, uint8_t *buffer){
	HAL_StatusTypeDef result;
	result = HAL_SPI_Transmit(hspi, buffer, 10, 1000);
	HAL_Delay(100);
	return result;
}

HAL_StatusTypeDef config_volt_src_gain(I2C_HandleTypeDef *hi2c, uint16_t pot_addr, uint8_t *buffer){
	HAL_StatusTypeDef result;
	result = HAL_I2C_Master_Transmit(hi2c, pot_addr, buffer, 2, 1000);
	HAL_Delay(100);
	return result;
}

HAL_StatusTypeDef config_volt_src_offset(I2C_HandleTypeDef *hi2c, uint16_t pot_addr, uint8_t *buffer){
	HAL_StatusTypeDef result;
	result = HAL_I2C_Master_Transmit(hi2c, pot_addr, buffer, 2, 1000);
	HAL_Delay(100);
	return result;
}

HAL_StatusTypeDef config_current_mirror(I2C_HandleTypeDef *hi2c, uint16_t pot_addr, uint8_t *buffer){
	HAL_StatusTypeDef result;
	result = HAL_I2C_Master_Transmit(hi2c, pot_addr, buffer, 2, 1000);
	HAL_Delay(100);
	return result;
}

void pot_val_test(void) {
	uint8_t test_volt_src_dds_spi_tx_buf[] = {0x21,0x00,0x61,0x8E,0x40,0x00,0xC0,0x00,0x20,0x00};  // 400Hz
	uint8_t test_volt_src_gain_i2c_tx_buf[] = {0,126};
	uint8_t test_volt_src_offset_i2c_tx_buf[] = {0,126};
	uint8_t test_curr_mrr_i2c_tx_buf[] = {0,100};

	for(int i = 1; i < 255; ){
		set_pot_buffer(test_curr_mrr_i2c_tx_buf,DVC_POT_AD5245_WP_WR_CMD,i);
		config_current_mirror(&hi2c3,DVC_CURR_MRR_POT_I2C_ADDR,test_curr_mrr_i2c_tx_buf);
		i += 5;
		HAL_Delay(300);
	}
//	set_ad9833_dds_buffer(test_volt_src_dds_spi_tx_buf,0);
//	config_dds_freq(&hspi2,test_volt_src_dds_spi_tx_buf);
//	for(int i = 0; i < 128; ){
//		set_pot_buffer(test_volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,i);
//		config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,test_volt_src_offset_i2c_tx_buf);
//		i += 10;
//	}

//	set_pot_buffer(test_volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,64);
//	config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,test_volt_src_offset_i2c_tx_buf);
//	config_dds_freq(&hspi2,test_volt_src_dds_spi_tx_buf);
//
//	for(int i = 0; i < 128; ){
//		set_pot_buffer(test_volt_src_gain_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,i);
//		config_volt_src_gain(&hi2c1,DVC_VOLT_SRC_1_AMP_POT_I2C_ADDR,test_volt_src_gain_i2c_tx_buf);
//		i += 5;
//		if (i > 127)
//			i = 127;
//	}

//	set_pot_buffer(test_curr_mrr_i2c_tx_buf,DVC_POT_AD5245_WP_WR_CMD,calculate_pot_value_curr_mirr(0.2));
//	config_current_mirror(&hi2c3,DVC_CURR_MRR_POT_I2C_ADDR,test_curr_mrr_i2c_tx_buf);
//
//	set_pot_buffer(test_volt_src_gain_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,calculate_pot_value_volt_gain(1));
//	set_pot_buffer(test_volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,calculate_pot_value_volt_offset(2));
//	set_ad9833_dds_buffer(test_volt_src_dds_spi_tx_buf,107374);
//
//	config_dds_freq(&hspi2,test_volt_src_dds_spi_tx_buf);
//	config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,test_volt_src_offset_i2c_tx_buf);
//	config_volt_src_gain(&hi2c1,DVC_VOLT_SRC_1_AMP_POT_I2C_ADDR,test_volt_src_gain_i2c_tx_buf);
//
//	config_dds_freq(&hspi1,test_volt_src_dds_spi_tx_buf);
//	config_volt_src_offset(&hi2c3,DVC_VOLT_SRC_2_OFS_POT_I2C_ADDR,test_volt_src_offset_i2c_tx_buf);
//	config_volt_src_gain(&hi2c3,DVC_VOLT_SRC_2_AMP_POT_I2C_ADDR,test_volt_src_gain_i2c_tx_buf);

	HAL_Delay(100);

}
