/**
  ******************************************************************************
  * @file    switch_network.c
  * @brief   Relay control of the switch network.
  ******************************************************************************
  */

/*
 *	Author: JY-85
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "../Inc/switch_network.h"

void DVC_Clear_Switch_Network(SwitchNetwork_TypeDef switch_network){
	for(int i = 0; i < DVC_TOTAL_RELAY_COUNT; i++){
		switch_network.Relays[i].RelayState = GPIO_PIN_RESET;
		HAL_GPIO_WritePin(switch_network.Relays[i].AssociatedGPIOPort,
						  switch_network.Relays[i].AssociatedGPIO,
						  switch_network.Relays[i].RelayState);
		HAL_Delay(300);
	}
	return;
}
void DVC_Set_Switch_Network(SwitchNetwork_TypeDef switch_network){
	if(!switch_network.ValidSwitchNetwork){return;}
	for(int i = 0; i < DVC_TOTAL_RELAY_COUNT; i++){
		HAL_GPIO_WritePin(switch_network.Relays[i].AssociatedGPIOPort,
						  switch_network.Relays[i].AssociatedGPIO,
						  switch_network.Relays[i].RelayState);
		HAL_Delay(300);
	}
	return;
}
void DVC_Map_Switch_Network(SwitchNetwork_TypeDef switch_network, uint32_t switch_network_config){
	return;
}
