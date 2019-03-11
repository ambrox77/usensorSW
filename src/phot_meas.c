/*
 * multi_adc.c
 *
 *  Created on: 15.07.2018
 *      Author: MCV
 */

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "diag/Trace.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <phot_meas.h>

#define CHANNELS_NUMBER 4
#define AVERAGE_CYCLES 4
#define BUFFER_SIZE (CHANNELS_NUMBER*AVERAGE_CYCLES)


uint16_t ADC1ConvertedValues[BUFFER_SIZE];
uint8_t phot_transfer_ready_flag=0;


// ******** EXTERNAL ****************
void Photo_Init(void){

	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;


	/* GPIOG Peripheral clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Enable the SPI clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);

	/* Configure PC10, PC11 and PC12 in output SCLK, DOUT and DIN  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);

	/* Configure PA15 in output CS_neg */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Set hight state to CS */
	GPIO_SetBits(GPIOA, GPIO_Pin_15);

	/* SPI configuration -------------------------------------------------------*/
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI3, &SPI_InitStructure);

	/* Enable the SPI peripheral */
	SPI_Cmd(SPI1, ENABLE);
}


// ******** INTERNAL ****************

int8_t Spi_Read8bit(uint8_t data)
{
//	uint16_t data;
//	data=(reg|0x80)<<8;


    GPIO_ResetBits(GPIOA, GPIO_Pin_15);
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI3, data);
//	    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==0);
//	    SPI_I2S_ReceiveData(SPI1);
//	    SPI_I2S_SendData(SPI1, 0xff);
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);
    data = SPI_I2S_ReceiveData(SPI3);
    GPIO_SetBits(GPIOA, GPIO_Pin_15);

    return data;
}
void Spi_Write8bit(uint8_t data)
{
//	uint16_t temp;
//
//	temp = reg;
//	temp = temp << 8;
//	temp |= data;
//	temp &= ~0x8000;

    GPIO_ResetBits(GPIOA, GPIO_Pin_15);
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI3, data);
//	    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==0);
//	    SPI_I2S_ReceiveData(SPI1);
//	    SPI_I2S_SendData(SPI1, 0xff);
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);
    SPI_I2S_ReceiveData(SPI3);
    GPIO_SetBits(GPIOA, GPIO_Pin_15);
}
void Spi_Read(int8_t* x, int8_t* y, int8_t* z){


	*x = Spi_GetReg(0x29);
	*y = Spi_GetReg(0x2B);
	*z = Spi_GetReg(0x2D);
}


////////////////////////////////////////////////////////////////////////////////////////////




//void PHOT_Init(void){
//	ADC_InitTypeDef       ADC_InitStruct;
//	ADC_CommonInitTypeDef ADC_CommonInitStruct;
//	DMA_InitTypeDef       DMA_InitStruct;
//	GPIO_InitTypeDef      GPIO_InitStruct;
//	NVIC_InitTypeDef NVIC_InitStruct;
//
//	/* Enable ADC1, DMA2 and GPIO clocks ****************************************/
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOA, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);//ADC1 is connected to the APB2 peripheral bus
//
//	/* DMA2 Stream0 channel0 configuration **************************************/
//	DMA_InitStruct.DMA_Channel = DMA_Channel_0;
//	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;//ADC1's data register
//	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)&ADC1ConvertedValues;
//	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
//	DMA_InitStruct.DMA_BufferSize = BUFFER_SIZE;
//	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//Reads 16 bit values
//	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//Stores 16 bit values
//	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
//	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
//	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
//	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
//	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
//	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
//	DMA_Init(DMA2_Stream0, &DMA_InitStruct);
//	DMA_Cmd(DMA2_Stream0, ENABLE);
//
//	/* Configure GPIO pins ******************************************************/
//	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;// PC0, PC1, PC2, PC3
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;//The pins are configured in analog mode
//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL ;//We don't need any pull up or pull down
//	GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
//	GPIO_Init(GPIOC, &GPIO_InitStruct);//Initialize GPIOC pins with the configuration
//	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;//PA1
//	GPIO_Init(GPIOA, &GPIO_InitStruct);//Initialize GPIOA pins with the configuration
//
//	/* ADC Common Init **********************************************************/
//	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
//	ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div2;
//	ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
//	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
//	ADC_CommonInit(&ADC_CommonInitStruct);
//
//	/* ADC1 Init ****************************************************************/
//	ADC_DeInit();
//	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;//Input voltage is converted into a 12bit int (max 4095)
//	ADC_InitStruct.ADC_ScanConvMode = ENABLE;//The scan is configured in multiple channels
//	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;//Continuous conversion: input signal is sampled more than once
//	ADC_InitStruct.ADC_ExternalTrigConv = 0;
//	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
//	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;//Data converted will be shifted to right
//	ADC_InitStruct.ADC_NbrOfConversion = CHANNELS_NUMBER;
//	ADC_Init(ADC1, &ADC_InitStruct);//Initialize ADC with the configuration
//
//	/* Select the channels to be read from **************************************/
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_144Cycles);//PC0
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_144Cycles);//PC1
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_144Cycles);//PC2
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_144Cycles);//PC3
//
//
//
//	NVIC_InitStruct.NVIC_IRQChannel = DMA2_Stream0_IRQn;
//	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
//	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
//	NVIC_Init(&NVIC_InitStruct);
//
//	/* Enable DMA request after last transfer (Single-ADC mode) */
//	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
//
//	//DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
//
//
//	/* Enable ADC1 DMA */
//	ADC_DMACmd(ADC1, ENABLE);
//
//	/* Enable ADC1 */
//	ADC_Cmd(ADC1, ENABLE);
//
//	/* Start ADC1 Software Conversion */
//	ADC_SoftwareStartConv(ADC1);
//}
//
//
////void DMA2_Stream0_IRQHandler()
////{
////	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TC)!= RESET){
////		 DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC);
////		 phot_transfer_ready_flag=1;
////		}
////}
