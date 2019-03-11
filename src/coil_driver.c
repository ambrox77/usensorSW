/*
 * i2c_fun.c
 *
 *  Created on: 15.07.2018
 *      Author: MCV
 */

#include <coil_driver.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "BusyDelay.h"


#define I2C_TRANSMITTER_MODE   1
#define I2C_RECEIVER_MODE      0
#define I2C_ACK_ENABLE         1
#define I2C_ACK_DISABLE        0

#define DAC_SCL GPIO_Pin_8		//Port B
#define DAC_SDA GPIO_Pin_9		//Port B
#define DAC_ADDR GPIO_Pin_5		//Port B
#define DAC_CRL_neg GPIO_Pin_6		//Port B
#define DAC_LDAC_neg GPIO_Pin_7		//Port B

#define I2C_TIMEOUT 0xFFFF


uint32_t I2C_Timeout;

static	uint8_t TM_I2C_Stop(I2C_TypeDef* I2Cx);
static 	uint8_t TM_I2C_ReadNack(I2C_TypeDef* I2Cx);
static 	uint8_t TM_I2C_ReadAck(I2C_TypeDef* I2Cx);
static 	void TM_I2C_WriteData(I2C_TypeDef* I2Cx, uint8_t data);
static 	int16_t TM_I2C_Start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction, uint8_t ack);
static 	void TM_I2C_Write(I2C_TypeDef* I2Cx, uint8_t address, uint8_t command, uint8_t data_msb, uint8_t data_lsb);
static 	void TM_I2C_ReadMultiNoRegister(I2C_TypeDef* I2Cx, uint8_t address, uint8_t* data, uint16_t count);
static 	uint8_t TM_I2C_ReadNoRegister(I2C_TypeDef* I2Cx, uint8_t address);
static 	void TM_I2C_ReadMulti(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, int8_t* data, uint16_t count);
static 	uint8_t TM_I2C_Read(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg);

// Externals:
// [0 0 0 1 1 A1 A0 R/W]
// Device Address Selection - table 7
// 	ADDR VDD - A1 A0 - 			00
// 	ADDR no connection A1 A0 - 	10
// 	ADDR GND - A1 A0 - 			11
// Write operation R/W = 		0
// first byte - 				0x1C

// [ 0 S C2 C1 C0 A2 A1 A0]
// S - multiple bite operation, set 0 for single write
// Command definition C2 C1 C0 - table 8
//	Write to input register n -								000
// 	Update DAC register n - 								001
// 	Write to input register n, update all (software LDAC) - 010
// 	Write to and update DAC channel n -						011
// 	Power up/power down - 									100
// 	Reset - 												101
// 	LDAC register setup - 									110
// 	Internal reference setup (on/off) - 					111
// DAC Adress Command A2 A1 A0 - table 9
// 	DAC_A -			000
// 	DAC_B - 		001
//  Both DACs -		111
// second byte (write to input, DAC_A) - 0x80
// 3rd and 4th - data

uint8_t DAC_SetVoltage(uint16_t volt)
{
	uint16_t f=0xffff, t;
	int8_t k=1;

	f=32765;
	t=32765;

	TM_I2C_Write(I2C1,(0x0F<<1),0x41,t>>8,t);

	while(1){
		TM_I2C_Write(I2C1,(0x0F<<1),0x40,f>>8,f);
		f=f+k;
		if(f==36000) k = -1;
		if(f==29000) k=1;
		//Delay_ms(1);
	}
	//TM_I2C_Write(I2C1,(0x0F<<1),0x47,0x005f,0);
	return 1;

}

void DAC_OUT_Coil_Ctrl_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = DAC_SCL|DAC_SDA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = DAC_ADDR|DAC_CRL_neg|DAC_LDAC_neg;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB,DAC_ADDR|DAC_CRL_neg|DAC_LDAC_neg);

	GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource9,GPIO_AF_I2C1);

	I2C_DeInit(I2C1);

	I2C_AnalogFilterCmd(I2C1,ENABLE);
	I2C_DigitalFilterConfig(I2C1,0x0F);

	I2C_StructInit(&I2C_InitStructure);
	I2C_InitStructure.I2C_ClockSpeed=400000;		//Fast mode 400kHz
	I2C_InitStructure.I2C_Mode=I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle=I2C_DutyCycle_16_9;
	I2C_InitStructure.I2C_OwnAddress1=0x00;
	I2C_InitStructure.I2C_Ack=I2C_Ack_Disable;
	I2C_InitStructure.I2C_AcknowledgedAddress=I2C_AcknowledgedAddress_7bit;

	I2C_Cmd(I2C1,DISABLE);
	I2C_Init(I2C1,&I2C_InitStructure);
	I2C_Cmd(I2C1,ENABLE);

	//TM_I2C_Write(I2C1,0xC0,0,0,0);
}




// Internals:

static	uint8_t TM_I2C_Read(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg)
{
	uint8_t received_data=0;
	TM_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
	TM_I2C_WriteData(I2Cx, reg);
	TM_I2C_Stop(I2Cx);
	TM_I2C_Start(I2Cx, address, I2C_RECEIVER_MODE, I2C_ACK_DISABLE);
	received_data = TM_I2C_ReadNack(I2Cx);
	return received_data;
}

static	void TM_I2C_ReadMulti(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, int8_t* data, uint16_t count)
{
	TM_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_ENABLE);
	TM_I2C_WriteData(I2Cx, reg);
	//TM_I2C_Stop(I2Cx);
	TM_I2C_Start(I2Cx, address, I2C_RECEIVER_MODE, I2C_ACK_ENABLE);
	while (count--) {
		if (!count) {
			/* Last byte */
			*data++ = TM_I2C_ReadNack(I2Cx);
		} else {
			*data++ = TM_I2C_ReadAck(I2Cx);
		}
	}
}

static	uint8_t TM_I2C_ReadNoRegister(I2C_TypeDef* I2Cx, uint8_t address)
{
	uint8_t data;
	TM_I2C_Start(I2Cx, address, I2C_RECEIVER_MODE, I2C_ACK_ENABLE);
	/* Also stop condition happens */
	data = TM_I2C_ReadNack(I2Cx);
	return data;
}

static	void TM_I2C_ReadMultiNoRegister(I2C_TypeDef* I2Cx, uint8_t address, uint8_t* data, uint16_t count)
{
	TM_I2C_Start(I2Cx, address, I2C_RECEIVER_MODE, I2C_ACK_ENABLE);
	while (count--) {
		if (!count) {
			/* Last byte */
			*data = TM_I2C_ReadNack(I2Cx);
		} else {
			*data = TM_I2C_ReadAck(I2Cx);
		}
	}
}

static	void TM_I2C_Write(I2C_TypeDef* I2Cx, uint8_t address, uint8_t command, uint8_t data_msb, uint8_t data_lsb)
{
	TM_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
	TM_I2C_WriteData(I2Cx, command);
	TM_I2C_WriteData(I2Cx, data_msb);
	TM_I2C_WriteData(I2Cx, data_lsb);
	TM_I2C_Stop(I2Cx);
}


static	int16_t TM_I2C_Start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction, uint8_t ack)
{
	/* Generate I2C start pulse */
	I2Cx->CR1 |= I2C_CR1_START;

	/* Wait till I2C is busy */
	I2C_Timeout = I2C_TIMEOUT;
	while (!(I2Cx->SR1 & I2C_SR1_SB)) {
		if (--I2C_Timeout == 0x00) {
			return 1;
		}
	}

	/* Enable ack if we select it */
	if (ack) {
		I2Cx->CR1 |= I2C_CR1_ACK;
	}

	/* Send write/read bit */
	if (direction == I2C_TRANSMITTER_MODE) {
		/* Send address with zero last bit */
		I2Cx->DR = address & ~I2C_OAR1_ADD0;

		/* Wait till finished */
		I2C_Timeout = I2C_TIMEOUT;
		while (!(I2Cx->SR1 & I2C_SR1_ADDR)) {
			if (--I2C_Timeout == 0x00) {
				return 1;
			}
		}
	}
	if (direction == I2C_RECEIVER_MODE) {
		/* Send address with 1 last bit */
		I2Cx->DR = address | I2C_OAR1_ADD0;

		/* Wait till finished */
		I2C_Timeout = I2C_TIMEOUT;
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
			if (--I2C_Timeout == 0x00) {
				return 1;
			}
		}
	}

	/* Read status register to clear ADDR flag */
	I2Cx->SR2;

	/* Return 0, everything ok */
	return 0;
}

static	void TM_I2C_WriteData(I2C_TypeDef* I2Cx, uint8_t data)
{
	/* Wait till I2C is not busy anymore */
	I2C_Timeout = I2C_TIMEOUT;
	while (!(I2Cx->SR1 & I2C_SR1_TXE) && I2C_Timeout) {
		I2C_Timeout--;
	}

	/* Send I2C data */
	I2Cx->DR = data;
}

static	uint8_t TM_I2C_ReadAck(I2C_TypeDef* I2Cx)
{
	uint8_t data;

	/* Enable ACK */
	I2Cx->CR1 |= I2C_CR1_ACK;

	/* Wait till not received */
	I2C_Timeout = I2C_TIMEOUT;
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
		if (--I2C_Timeout == 0x00) {
			return 1;
		}
	}

	/* Read data */
	data = I2Cx->DR;

	/* Return data */
	return data;
}

static	uint8_t TM_I2C_ReadNack(I2C_TypeDef* I2Cx)
{
	uint8_t data;

	/* Disable ACK */
	I2Cx->CR1 &= ~I2C_CR1_ACK;

	/* Generate stop */
	I2Cx->CR1 |= I2C_CR1_STOP;

	/* Wait till received */
	I2C_Timeout = I2C_TIMEOUT;
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
		if (--I2C_Timeout == 0x00) {
			return 1;
		}
	}

	/* Read data */
	data = I2Cx->DR;

	/* Return data */
	return data;
}

static	uint8_t TM_I2C_Stop(I2C_TypeDef* I2Cx)
{
	/* Wait till transmitter not empty */
	I2C_Timeout = I2C_TIMEOUT;
	while (((!(I2Cx->SR1 & I2C_SR1_TXE)) || (!(I2Cx->SR1 & I2C_SR1_BTF)))) {
		if (--I2C_Timeout == 0x00) {
			return 1;
		}
	}

	/* Generate stop */
	I2Cx->CR1 |= I2C_CR1_STOP;

	/* Return 0, everything ok */
	return 0;
}
