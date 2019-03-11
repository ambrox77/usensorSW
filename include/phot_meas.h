/*
 * multi_adc.h
 *
 *  Created on: 30.06.2018
 *      Author: MCV
 */

#ifndef MULTI_ADC_H_
#define MULTI_ADC_H_

#include "stdio.h"


//typedef struct{
//	uint16_t photo_A;
//	uint16_t photo_B;
//	uint16_t photo_C;
//	uint16_t photo_D;
//}phot_measures_T;


void Photo_Init(void);
//<<<<<<< HEAD
int8_t Spi_Read8bit(uint8_t data);
void Spi_Write8bit(uint8_t data);

//=======
//>>>>>>> 70982ca285d9fdaf3244f07c7000b50ed46b8019
//void PHOT_GetMeas(phot_measures_T* meas);
//void DMA2_Stream0_IRQHandler();

#endif /* MULTI_ADC_H_ */
