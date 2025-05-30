/*
 * measurement_routines.c
 *
 *  Created on: Feb 27, 2025
 *      Author: User
 */

#include "switch_network.h"
#include "device_sourcing.h"
#include "device_monitoring.h"
#include "device_registers.h"
#include "run_device.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

extern DMA_HandleTypeDef handle_GPDMA1_Channel12;
extern DMA_HandleTypeDef handle_GPDMA1_Channel13;
extern DMA_HandleTypeDef handle_GPDMA1_Channel14;

uint8_t volt_src_dds_spi_tx_buf[] = {0x21,0x00,0x61,0x8E,0x40,0x00,0xC0,0x00,0x20,0x00};  // 400Hz
uint8_t volt_src_gain_i2c_tx_buf[] = {0,126};
uint8_t volt_src_offset_i2c_tx_buf[] = {0,126};
uint8_t curr_mrr_i2c_tx_buf[] = {0,100};

extern uint16_t adc_samples_1[DVC_MAX_NUM_ADC_SAMPLES];
extern uint16_t adc_samples_2[DVC_MAX_NUM_ADC_SAMPLES];
extern uint16_t adc_samples_3[DVC_MAX_NUM_ADC_SAMPLES];

extern RegisterMap_TypeDef device_registers;
extern SwitchNetwork_TypeDef device_switch_network;

extern volatile uint8_t adc_1_busy;
extern volatile uint8_t adc_2_busy;
extern volatile uint8_t adc_3_busy;

extern volatile uint8_t adc_1_full;
extern volatile uint8_t adc_2_full;
extern volatile uint8_t adc_3_full;

void dvc_exec_msr_dc_resistance_2p(void){
	uint32_t pot_val;
	float current_val;
	// set busy flag to indicate a measurement is in progress
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);

	// get potentiometer value to configure current mirror
	current_val = get_register(&device_registers,DVC_2PM_DCRESISTANCE_1);

	// calculate the potentiometer value
	pot_val = calculate_pot_value_curr_mirr(current_val);

	// prepare buffer for potentiometer I2C tx
	set_pot_buffer(curr_mrr_i2c_tx_buf,DVC_POT_AD5245_WP_WR_CMD,pot_val);

	// configure the switch network
	map_switch_network(&device_switch_network,get_register(&device_registers,DVC_PROBE_CONFIG));
	set_switch_network(&device_switch_network);
	HAL_Delay(1000);
	// configure the current mirror (2 seconds delay for stabilization)
	config_current_mirror(&hi2c3,DVC_CURR_MRR_POT_I2C_ADDR,curr_mrr_i2c_tx_buf);
	HAL_Delay(2000);

	// collect ADC samples
	collect_adc_samples_it(DVC_USE_ADC_1_SAMPLING);

	// wait for adc dma to complete
	while(adc_1_busy);

	// disconnect switch network to cut power
	clear_switch_network(&device_switch_network);

	// clear busy flag to indicate a measurement is complete
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));

}

void dvc_exec_msr_current_voltage(void){
	HAL_StatusTypeDef result;
	uint32_t pot_val_gain, pot_val_offset, pot_val_curr_mirr, method_sel, dds_freq_val;
	uint32_t start_param, end_param, incr_param;

	// set busy flag to indicate a measurement is in progress
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);

	// grab the measurement parameters from the registers;
	method_sel = get_register(&device_registers,DVC_2PM_CURRVOLT_1) & 0x3;
	start_param = get_register(&device_registers,DVC_2PM_CURRVOLT_2);
	end_param = get_register(&device_registers,DVC_2PM_CURRVOLT_3);
	incr_param = get_register(&device_registers,DVC_2PM_CURRVOLT_4);

	// configure the basic DC voltage source with 0Hz
	dds_freq_val = 0;
	pot_val_gain = 64;
	set_ad9833_dds_buffer(volt_src_dds_spi_tx_buf,dds_freq_val);
	set_pot_buffer(volt_src_gain_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_gain);
	config_volt_src_gain(&hi2c1,DVC_VOLT_SRC_1_AMP_POT_I2C_ADDR,volt_src_gain_i2c_tx_buf);
	config_dds_freq(&hspi2,volt_src_dds_spi_tx_buf);

	// configure the switch network
	map_switch_network(&device_switch_network,get_register(&device_registers,DVC_PROBE_CONFIG));
	set_switch_network(&device_switch_network);

	for(int param = start_param; param <= end_param; param += incr_param){
		if(method_sel == DVC_CUR_VOLT_2P_SEL_VOLT){
			pot_val_offset = calculate_pot_value_volt_offset((float)param/1000.0);
			set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset);
			config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);
			HAL_Delay(2000);
			// collect ADC samples
			collect_adc_samples_it(DVC_USE_ADC_3_SAMPLING);

			// wait for adc dma to complete
			while(adc_3_busy);

			// clear busy flag to indicate a measurement is complete
			set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
						 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));

			// wait for python to grab the data
			while(adc_3_full);
		}
		else if (method_sel == DVC_CUR_VOLT_2P_SEL_CURR){
			pot_val_curr_mirr = calculate_pot_value_curr_mirr((float)param);
			set_pot_buffer(curr_mrr_i2c_tx_buf,DVC_POT_AD5245_WP_WR_CMD,pot_val_curr_mirr);
			config_current_mirror(&hi2c3,DVC_CURR_MRR_POT_I2C_ADDR,curr_mrr_i2c_tx_buf);
			HAL_Delay(2000);

			// collect ADC samples
			collect_adc_samples_it(DVC_USE_ADC_1_SAMPLING);

			// wait for adc dma to complete
			while(adc_1_busy);

			// clear busy flag to indicate a measurement is complete
			set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
						 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));

			// wait for python to grab the data
			while(adc_1_full);
		}

		// set busy flag to indicate a measurement is in progress
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);
	}

	// disconnect switch network to cut power
	clear_switch_network(&device_switch_network);

	// clear busy flag to indicate a measurement is complete
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));
}

void dvc_exec_msr_capacitance_voltage_2p(void){
	HAL_StatusTypeDef result;
	uint32_t pot_val_gain, pot_val_offset;

	// set busy flag to indicate a measurement is in progress
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);

	// grab the measurement parameters from the registers;
	uint32_t start_volt_offset = get_register(&device_registers,DVC_2PM_CAPVOLT_1);
	uint32_t end_volt_offset = get_register(&device_registers,DVC_2PM_CAPVOLT_2);
	uint32_t incr_volt_offset = get_register(&device_registers,DVC_2PM_CAPVOLT_3);

	// configure the basic DC voltage source with 10kHz
	uint32_t freq_28b = 107374;

	float amp_volt = 400;
	pot_val_gain = calculate_pot_value_volt_gain(amp_volt/1000.0);
	set_pot_buffer(volt_src_gain_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_gain);
	config_volt_src_gain(&hi2c1,DVC_VOLT_SRC_1_AMP_POT_I2C_ADDR,volt_src_gain_i2c_tx_buf);

	set_ad9833_dds_buffer(volt_src_dds_spi_tx_buf,freq_28b);
	config_dds_freq(&hspi2,volt_src_dds_spi_tx_buf);

	// configure the switch network
	map_switch_network(&device_switch_network,get_register(&device_registers,DVC_PROBE_CONFIG));
	set_switch_network(&device_switch_network);

	for(int param = start_volt_offset; param <= end_volt_offset; param += incr_volt_offset){
		pot_val_offset = calculate_pot_value_volt_offset(param/1000.0);
		set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset);
		config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);
		HAL_Delay(2000);

		// collect ADC samples
		collect_adc_samples_it(DVC_USE_ADC_3_SAMPLING);

		// wait for adc dma to complete
		while(adc_3_busy);

		// clear busy flag to indicate a measurement is complete
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));

		// wait for python to grab the data
		while(adc_3_full);

		// set busy flag to indicate a measurement is in progress
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);
	}

	// disconnect switch network to cut power
	clear_switch_network(&device_switch_network);

	// clear busy flag to indicate a measurement is complete
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));
}

void dvc_exec_msr_impedance_spectroscopy_2p(void){
	HAL_StatusTypeDef result;
	uint32_t pot_val_gain, pot_val_offset;

	// set busy flag to indicate a measurement is in progress
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);

	// grab the measurement parameters from the registers;
	uint32_t start_freq_l14b = get_register(&device_registers,DVC_2PM_IMPSPEC_1);
	uint32_t start_freq_u14b = get_register(&device_registers,DVC_2PM_IMPSPEC_2);
	uint32_t end_freq_l14b = get_register(&device_registers,DVC_2PM_IMPSPEC_3);
	uint32_t end_freq_u14b = get_register(&device_registers,DVC_2PM_IMPSPEC_4);
	uint32_t incr_freq_l14b = get_register(&device_registers,DVC_2PM_IMPSPEC_5);
	uint32_t incr_freq_u14b = get_register(&device_registers,DVC_2PM_IMPSPEC_6);
	uint32_t max_volt = get_register(&device_registers,DVC_2PM_IMPSPEC_7);
	uint32_t min_volt = get_register(&device_registers,DVC_2PM_IMPSPEC_8);

	// configure the basic DC voltage source with 0Hz
	uint32_t start_freq_28b = (start_freq_u14b << 14) | start_freq_l14b;
	uint32_t end_freq_28b = (end_freq_u14b << 14) | end_freq_l14b;
	uint32_t incr_freq_28b = (incr_freq_u14b << 14) | incr_freq_l14b;

	float mid_volt = ((float)max_volt+(float)min_volt)/2;
	float amp_volt = ((float)max_volt-(float)min_volt);

	pot_val_offset = calculate_pot_value_volt_offset(mid_volt/1000.0);
	pot_val_gain = calculate_pot_value_volt_gain(amp_volt/1000.0);

	set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset);
	set_pot_buffer(volt_src_gain_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_gain);

	config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);
	config_volt_src_gain(&hi2c1,DVC_VOLT_SRC_1_AMP_POT_I2C_ADDR,volt_src_gain_i2c_tx_buf);

	// configure the switch network
	map_switch_network(&device_switch_network,get_register(&device_registers,DVC_PROBE_CONFIG));
	set_switch_network(&device_switch_network);

	for(int param = start_freq_28b; param <= end_freq_28b+5; param += incr_freq_28b){
		set_ad9833_dds_buffer(volt_src_dds_spi_tx_buf,param);
		config_dds_freq(&hspi2,volt_src_dds_spi_tx_buf);
		HAL_Delay(2000);

		// collect ADC samples
		collect_adc_samples_it(DVC_USE_ADC_1_SAMPLING | DVC_USE_ADC_3_SAMPLING);

		// wait for adc dma to complete
		while(adc_1_busy | adc_3_busy);

		// clear busy flag to indicate a measurement is complete
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));

		// wait for python to grab the data
		while(adc_1_full | adc_3_full);

		// set busy flag to indicate a measurement is in progress
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);
	}

	// disconnect switch network to cut power
	clear_switch_network(&device_switch_network);

	// clear busy flag to indicate a measurement is complete
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));
}

void dvc_exec_msr_transfer_characteristics(void){
	HAL_StatusTypeDef result;
	uint32_t pot_val_gain, pot_val_offset_drain, pot_val_offset_gate;

	// set busy flag to indicate a measurement is in progress
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);

	// grab the measurement parameters from the registers;
	uint32_t probe_sel = get_register(&device_registers,DVC_3PM_TRANSCHAR_1);
	uint32_t drain_volt = get_register(&device_registers,DVC_3PM_TRANSCHAR_2);
	uint32_t gate_volt_start = get_register(&device_registers,DVC_3PM_TRANSCHAR_3);
	uint32_t gate_volt_end = get_register(&device_registers,DVC_3PM_TRANSCHAR_4);
	uint32_t gate_volt_incr = get_register(&device_registers,DVC_3PM_TRANSCHAR_5);

	uint32_t drain_probe_sel = (probe_sel >> 4) & 0xF;
	uint32_t gate_probe_sel = (probe_sel) & 0xF;

	// configure the basic DC voltage source with 0Hz
	uint32_t freq_28b = 0;
	pot_val_gain = 64;

	set_ad9833_dds_buffer(volt_src_dds_spi_tx_buf,freq_28b);
	config_dds_freq(&hspi2,volt_src_dds_spi_tx_buf);
	config_dds_freq(&hspi1,volt_src_dds_spi_tx_buf);

	set_pot_buffer(volt_src_gain_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_gain);
	config_volt_src_gain(&hi2c1,DVC_VOLT_SRC_1_AMP_POT_I2C_ADDR,volt_src_gain_i2c_tx_buf);
	config_volt_src_gain(&hi2c3,DVC_VOLT_SRC_2_AMP_POT_I2C_ADDR,volt_src_gain_i2c_tx_buf);

	// configure the switch network
	map_switch_network(&device_switch_network,get_register(&device_registers,DVC_PROBE_CONFIG));
	set_switch_network(&device_switch_network);

	for(int param = gate_volt_start; param <= gate_volt_end; param += gate_volt_incr){
		if(drain_probe_sel < gate_probe_sel){
			pot_val_offset_drain = calculate_pot_value_volt_offset(drain_volt/1000.0);
			set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset_drain);
			config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);

			pot_val_offset_gate = calculate_pot_value_volt_offset(param/1000.0);
			set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset_gate);
			config_volt_src_offset(&hi2c3,DVC_VOLT_SRC_2_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);
			HAL_Delay(2000);
		}
		else{
			pot_val_offset_drain = calculate_pot_value_volt_offset(drain_volt/1000.0);
			set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset_drain);
			config_volt_src_offset(&hi2c3,DVC_VOLT_SRC_2_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);

			pot_val_offset_gate = calculate_pot_value_volt_offset(param/1000.0);
			set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset_gate);
			config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);
			HAL_Delay(2000);
		}

		// collect ADC samples
		collect_adc_samples_it(DVC_USE_ADC_3_SAMPLING);

		// wait for adc dma to complete
		while(adc_3_busy);

		// clear busy flag to indicate a measurement is complete
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));

		// wait for python to grab the data
		while(adc_3_full);

		// set busy flag to indicate a measurement is in progress
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);
	}

	// disconnect switch network to cut power
	clear_switch_network(&device_switch_network);

	// clear busy flag to indicate a measurement is complete
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));
}

void dvc_exec_msr_output_characteristics(void){
	HAL_StatusTypeDef result;
	uint32_t pot_val_gain, pot_val_offset_drain, pot_val_offset_gate;

	// set busy flag to indicate a measurement is in progress
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);

	// grab the measurement parameters from the registers;
	uint32_t probe_sel = get_register(&device_registers,DVC_3PM_OUTCHAR_1);
	uint32_t gate_volt = get_register(&device_registers,DVC_3PM_OUTCHAR_2);
	uint32_t drain_volt_start = get_register(&device_registers,DVC_3PM_OUTCHAR_3);
	uint32_t drain_volt_end = get_register(&device_registers,DVC_3PM_OUTCHAR_4);
	uint32_t drain_volt_incr = get_register(&device_registers,DVC_3PM_OUTCHAR_5);

	uint32_t drain_probe_sel = (probe_sel >> 4) & 0xF;
	uint32_t gate_probe_sel = (probe_sel) & 0xF;

	// configure the basic DC voltage source with 0Hz
	uint32_t freq_28b = 0;
	pot_val_gain = 64;

	set_ad9833_dds_buffer(volt_src_dds_spi_tx_buf,freq_28b);
	config_dds_freq(&hspi2,volt_src_dds_spi_tx_buf);
	config_dds_freq(&hspi1,volt_src_dds_spi_tx_buf);

	set_pot_buffer(volt_src_gain_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_gain);
	config_volt_src_gain(&hi2c1,DVC_VOLT_SRC_1_AMP_POT_I2C_ADDR,volt_src_gain_i2c_tx_buf);
	config_volt_src_gain(&hi2c3,DVC_VOLT_SRC_2_AMP_POT_I2C_ADDR,volt_src_gain_i2c_tx_buf);

	// configure the switch network
	map_switch_network(&device_switch_network,get_register(&device_registers,DVC_PROBE_CONFIG));
	set_switch_network(&device_switch_network);

	for(int param = drain_volt_start; param <= drain_volt_end; param += drain_volt_incr){
		if(drain_probe_sel < gate_probe_sel){
			pot_val_offset_drain = calculate_pot_value_volt_offset(param/1000.0);
			set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset_drain);
			config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);

			pot_val_offset_gate = calculate_pot_value_volt_offset(gate_volt/1000.0);
			set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset_gate);
			config_volt_src_offset(&hi2c3,DVC_VOLT_SRC_2_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);
			HAL_Delay(2000);
		}
		else{
			pot_val_offset_drain = calculate_pot_value_volt_offset(param/1000.0);
			set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset_drain);
			config_volt_src_offset(&hi2c3,DVC_VOLT_SRC_2_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);

			pot_val_offset_gate = calculate_pot_value_volt_offset(gate_volt/1000.0);
			set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset_gate);
			config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);
			HAL_Delay(2000);
		}

		// collect ADC samples
		collect_adc_samples_it(DVC_USE_ADC_3_SAMPLING);

		// wait for adc dma to complete
		while(adc_3_busy);

		// clear busy flag to indicate a measurement is complete
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));

		// wait for python to grab the data
		while(adc_3_full);

		// set busy flag to indicate a measurement is in progress
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);
	}

	// disconnect switch network to cut power
	clear_switch_network(&device_switch_network);

	// clear busy flag to indicate a measurement is complete
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));
}

void dvc_exec_msr_capacitance_voltage_3p(void){
	HAL_StatusTypeDef result;
	uint32_t pot_val_gain, pot_val_offset;

	// set busy flag to indicate a measurement is in progress
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);

	// grab the measurement parameters from the registers;
	uint32_t start_volt_offset = get_register(&device_registers,DVC_2PM_CAPVOLT_1);
	uint32_t end_volt_offset = get_register(&device_registers,DVC_2PM_CAPVOLT_2);
	uint32_t incr_volt_offset = get_register(&device_registers,DVC_2PM_CAPVOLT_3);

	// configure the basic DC voltage source with 1MHz
	uint32_t freq_28b = 10737418;

	float amp_volt = 3000;
	pot_val_gain = calculate_pot_value_volt_gain(amp_volt/1000.0);
	set_pot_buffer(volt_src_gain_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_gain);
	config_volt_src_gain(&hi2c1,DVC_VOLT_SRC_1_AMP_POT_I2C_ADDR,volt_src_gain_i2c_tx_buf);

	set_ad9833_dds_buffer(volt_src_dds_spi_tx_buf,freq_28b);
	config_dds_freq(&hspi2,volt_src_dds_spi_tx_buf);

	// configure the switch network
	map_switch_network(&device_switch_network,get_register(&device_registers,DVC_PROBE_CONFIG));
	set_switch_network(&device_switch_network);

	for(int param = start_volt_offset; param <= end_volt_offset; param += incr_volt_offset){
		pot_val_offset = calculate_pot_value_volt_offset(param/1000.0);
		set_pot_buffer(volt_src_offset_i2c_tx_buf,DVC_POT_MCP4531_WP0_WR_CMD,pot_val_offset);
		config_volt_src_offset(&hi2c1,DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR,volt_src_offset_i2c_tx_buf);
		HAL_Delay(2000);

		// collect ADC samples
		collect_adc_samples_it(DVC_USE_ADC_3_SAMPLING);

		// wait for adc dma to complete
		while(adc_3_busy);

		// clear busy flag to indicate a measurement is complete
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));

		// wait for python to grab the data
		while(adc_3_full);

		// set busy flag to indicate a measurement is in progress
		set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
					 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);
	}

	// disconnect switch network to cut power
	clear_switch_network(&device_switch_network);

	// clear busy flag to indicate a measurement is complete
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));
}

void dvc_exec_msr_electrochemical(void){

}

void dvc_exec_msr_low_resistance(void){
	uint32_t pot_val, current_val;
	// set busy flag to indicate a measurement is in progress
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);

	// get potentiometer value to configure current mirror
	current_val = get_register(&device_registers,DVC_2PM_LOWRESISTANCE_1);

	// calculate the potentiometer value
	pot_val = calculate_pot_value_curr_mirr(current_val);

	// prepare buffer for potentiometer I2C tx
	set_pot_buffer(curr_mrr_i2c_tx_buf,DVC_POT_AD5245_WP_WR_CMD,pot_val);

	// configure the switch network
	map_switch_network(&device_switch_network,get_register(&device_registers,DVC_PROBE_CONFIG));
	set_switch_network(&device_switch_network);

	// configure the current mirror (2 seconds delay for stabilization)
	config_current_mirror(&hi2c3,DVC_CURR_MRR_POT_I2C_ADDR,curr_mrr_i2c_tx_buf);
	HAL_Delay(2000);

	// collect ADC samples
	collect_adc_samples_it(DVC_USE_ADC_1_SAMPLING | DVC_USE_ADC_2_SAMPLING);

	// wait for adc dma to complete
	while(adc_1_busy || adc_2_busy);

	// disconnect switch network to cut power
	clear_switch_network(&device_switch_network);

	// clear busy flag to indicate a measurement is complete
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));
}

void dvc_exec_msr_dc_resistance_4p(void){
	uint32_t pot_val, current_val;
	// set busy flag to indicate a measurement is in progress
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) | 0x4);

	// get potentiometer value to configure current mirror
	current_val = get_register(&device_registers,DVC_4PM_PROBERESISTANCE_1);

	// calculate the potentiometer value
	pot_val = calculate_pot_value_curr_mirr(current_val);

	// prepare buffer for potentiometer I2C tx
	set_pot_buffer(curr_mrr_i2c_tx_buf,DVC_POT_AD5245_WP_WR_CMD,pot_val);

	// configure the switch network
	map_switch_network(&device_switch_network,get_register(&device_registers,DVC_PROBE_CONFIG));
	set_switch_network(&device_switch_network);

	// configure the current mirror (2 seconds delay for stabilization)
	config_current_mirror(&hi2c3,DVC_CURR_MRR_POT_I2C_ADDR,curr_mrr_i2c_tx_buf);
	HAL_Delay(2000);

	// collect ADC samples
	collect_adc_samples_it(DVC_USE_ADC_1_SAMPLING | DVC_USE_ADC_2_SAMPLING);

	// wait for adc dma to complete
	while(adc_1_busy || adc_2_busy);

	// disconnect switch network to cut power
	clear_switch_network(&device_switch_network);

	// clear busy flag to indicate a measurement is complete
	set_register(&device_registers,DVC_MEASUREMENT_CONFIG,
				 get_register(&device_registers,DVC_MEASUREMENT_CONFIG) & ~(0x4));
}

void dvc_exec_msr_impedance_spectroscopy_4p(void){

}
