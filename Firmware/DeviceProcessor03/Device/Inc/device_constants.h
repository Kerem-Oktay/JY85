/*
 * DeviceConstants.h
 *
 *  Created on: Nov 16, 2024
 *      Author: JY85
 */

#ifndef INC_DEVICE_CONSTANTS_H_
#define INC_DEVICE_CONSTANTS_H_

#define DVC_NUM_PROBES 						4
#define DVC_NUM_RELAYS_PER_PROBE 			8
#define DVC_TOTAL_RELAY_COUNT				21
#define DVC_REGISTER_WIDTH 					32
#define DVC_REGISTER_ADDR_WIDTH				8
#define DVC_REGISTER_DATA_WIDTH				DVC_REGISTER_WIDTH-DVC_REGISTER_ADDR_WIDTH
#define DVC_TOTAL_REGISTER_NUMBER  			1<<(DVC_REGISTER_ADDR_WIDTH)
#define DVC_MAX_NUM_ADC_SAMPLES    			8192

#define DVC_VOLT_SRC_1_AMP_POT_I2C_ADDR 	0x2F<<1
#define DVC_VOLT_SRC_1_OFS_POT_I2C_ADDR 	0x2E<<1
#define DVC_VOLT_SRC_2_AMP_POT_I2C_ADDR 	0x2E<<1
#define DVC_VOLT_SRC_2_OFS_POT_I2C_ADDR 	0x2F<<1
#define DVC_CURR_MRR_POT_I2C_ADDR  			0x2C<<1

#define DVC_POT_MCP4531_WP0_WR_CMD			0x00
#define DVC_POT_AD5245_WP_WR_CMD			0x00

#define DVC_PROBE_SUPPLY_DCV				1
#define DVC_PROBE_SUPPLY_ACV				2
#define DVC_PROBE_SUPPLY_CUR				3
#define DVC_PROBE_SUPPLY_GND				4
#define DVC_PROBE_MEASURE_VOL				1
#define DVC_PROBE_MEASURE_CUR				2

#define DVC_CUR_VOLT_2P_SEL_VOLT			1
#define DVC_CUR_VOLT_2P_SEL_CURR			2

#define DVC_USE_ADC_1_SAMPLING				1
#define DVC_USE_ADC_2_SAMPLING				2
#define DVC_USE_ADC_3_SAMPLING				4


#define DVC_STATUS    				0
#define DVC_MEASUREMENT_CONFIG    	1
#define DVC_PROBE_CONFIG    		2
#define DVC_2PM_DCRESISTANCE_1    	3
#define DVC_2PM_CURRVOLT_1    		4
#define DVC_2PM_CURRVOLT_2    		5
#define DVC_2PM_CURRVOLT_3    		6
#define DVC_2PM_CURRVOLT_4    		7
#define DVC_2PM_CAPVOLT_1    		8
#define DVC_2PM_CAPVOLT_2    		9
#define DVC_2PM_CAPVOLT_3    		10
#define DVC_2PM_IMPSPEC_1    		11
#define DVC_2PM_IMPSPEC_2    		12
#define DVC_2PM_IMPSPEC_3    		13
#define DVC_2PM_IMPSPEC_4    		14
#define DVC_2PM_IMPSPEC_5    		15
#define DVC_2PM_IMPSPEC_6    		16
#define DVC_2PM_IMPSPEC_7    		17
#define DVC_2PM_IMPSPEC_8    		18
#define DVC_3PM_TRANSCHAR_1    		19
#define DVC_3PM_TRANSCHAR_2    		20
#define DVC_3PM_TRANSCHAR_3    		21
#define DVC_3PM_TRANSCHAR_4    		22
#define DVC_3PM_TRANSCHAR_5    		23
#define DVC_3PM_OUTCHAR_1    		24
#define DVC_3PM_OUTCHAR_2    		25
#define DVC_3PM_OUTCHAR_3    		26
#define DVC_3PM_OUTCHAR_4    		27
#define DVC_3PM_OUTCHAR_5    		28
#define DVC_3PM_CAPVOLT_1    		29
#define DVC_3PM_CAPVOLT_2    		30
#define DVC_3PM_CAPVOLT_3    		31
#define DVC_3PM_ELECHEM_1    		32
#define DVC_3PM_ELECHEM_2    		33
#define DVC_3PM_ELECHEM_3    		34
#define DVC_3PM_ELECHEM_4    		35
#define DVC_3PM_ELECHEM_5    		36
#define DVC_4PM_PROBERESISTANCE_1  37
#define DVC_2PM_LOWRESISTANCE_1    38
#define DVC_4PM_IMPSPEC_1    		39
#define DVC_4PM_IMPSPEC_2    		40
#define DVC_4PM_IMPSPEC_3    		41
#define DVC_4PM_IMPSPEC_4    		42
#define DVC_4PM_IMPSPEC_5    		43
#define DVC_4PM_IMPSPEC_6    		44
#define DVC_4PM_IMPSPEC_7    		45
#define DVC_4PM_IMPSPEC_8    		46
#define DVC_FLUSH_SAMPLE_DATA_1		100
#define DVC_FLUSH_SAMPLE_DATA_2		101
#define DVC_FLUSH_SAMPLE_DATA_3		102


/*
 * Enum name: Probe Configurations
 * Description:
 * 	All available probe configurations.
 *
 */
typedef enum
{
	PROBES_2 = 1,		/*!< Probe Configuration: 2 probes */
	PROBES_3 = 2,		/*!< Probe Configuration: 3 probes */
	PROBES_4 = 3,		/*!< Probe Configuration: 4 probes */
} ProbeCfg_Type;


/*
 * Enum name: Measurements Configurations
 * Description:
 * 	All available measurement configurations.
 * 	Different probe configurations can interpret the same measurement configuration differently.
 *
 */
typedef enum
{
	DC_RESISTANCE 			  = 1,
	CURRENT_VOLTAGE 		  = 2,
	CAPACITANCE_VOLTAGE_2P    = 3,
	IMPEDANCE_SPECTROSCOPY_2P = 4,
	TRANSFER_CHARACTERISTICS  = 5,
	OUTPUT_CHARACTERISTICS 	  = 6,
	CAPACITANCE_VOLTAGE_3P    = 7,
	ELECTROCHEMICAL 		  = 8,
	LOW_RESISTANCE 			  = 9,
	PROBE_RESISTANCE 		  = 10,
	IMPEDANCE_SPECTROSCOPY_4P = 11,
} MeasurementCfg_Type;

/*
 * Enum name: Measurements Configurations
 * Description:
 * 	All available measurement configurations.
 * 	Different probe configurations can interpret the same measurement configuration differently.
 *
 */
typedef enum
{
	ProbeId_0 			= 0, 	/*!< Probe Identifier: Probe 0 */
	ProbeId_1 			= 1,	/*!< Probe Identifier: Probe 1 */
	ProbeId_2 			= 2,	/*!< Probe Identifier: Probe 2 */
	ProbeId_3			= 3,	/*!< Probe Identifier: Probe 3 */
	ProbeId_MultiProbe	= 4,	/*!< Probe Identifier: Multiple Probes */
} ProbeId_Type;

#endif /* INC_DEVICE_CONSTANTS_H_ */
