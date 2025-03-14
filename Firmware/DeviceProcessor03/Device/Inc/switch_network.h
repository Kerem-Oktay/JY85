/*
 * switch_network.h
 *
 *  Created on: Nov 16, 2024
 *      Author: User
 */

#ifndef INC_SWITCH_NETWORK_H_
#define INC_SWITCH_NETWORK_H_

#include "device_constants.h"
#include "main.h"


typedef struct{
	uint8_t RelayId;
	ProbeId_Type AssociatedProbe;
	GPIO_PinState RelayState;
	GPIO_TypeDef *AssociatedGPIOPort;
	uint16_t AssociatedGPIO;
} SwitchRelay_TypeDef;

typedef struct{
	SwitchRelay_TypeDef Relays[DVC_TOTAL_RELAY_COUNT+1];
	uint8_t ValidSwitchNetwork;
} SwitchNetwork_TypeDef;

void init_switch_network(SwitchNetwork_TypeDef* switch_network);
void clear_switch_network(SwitchNetwork_TypeDef* switch_network);
void set_switch_network(SwitchNetwork_TypeDef* switch_network);
void map_switch_network(SwitchNetwork_TypeDef* switch_network, uint32_t switch_network_state);

#endif /* INC_SWITCH_NETWORK_H_ */
