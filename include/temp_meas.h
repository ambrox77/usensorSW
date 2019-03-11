/*
 * multi_adc.h
 *
 *  Created on: 30.06.2018
 *      Author: MCV
 */

#ifndef MULTI_ADC_H_
#define MULTI_ADC_H_

#include "stdio.h"


typedef struct{
	uint16_t temp_1;
	uint16_t temp_2;
	uint16_t temp_3;
	uint16_t temp_4;
}temp_measures_T;


void Temp_SENS_Init(void);
void TEMP_GetMeas(temp_measures_T* meas);
void DMA2_Stream0_IRQHandler();

#endif /* MULTI_ADC_H_ */
