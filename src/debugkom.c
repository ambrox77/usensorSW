/*
 * komunikacja.c
 *
 *  Created on: 12 wrz 2016
 *      Author: Marcin
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"
#include "debugkom.h"
#include "timer.h"
#include "stm32f4_discovery.h"


#define UART_FTDI 1
#define UART_DEBUG 1

char *err[]={" OK "," Blad 1 "," Blad 2 "," Blad 3 "," Timeout "};
char *state[]={" A "," B "," C "," D "," E "};

static enum {DTIME,DATA,DAKC,DKAL,DST,DPOW,DREG}msgFormat=DATA;

volatile uint8_t debugBuf[DEBUG_BUF_LENGTH],debugOVF=0,debugBufRDY=0;
volatile uint16_t debugBufCnt=0;
uint8_t debugTimer;

debugkom_record_T debug_record;

uint16_t coil1=0,coil2=0;
//static uint16_t copy_meas;

char cbuf[64];

static void Debug_InputHandler(void);
static void Debug_Raport(void);
static void Uart_Init();
static void Uart_BufClr();
/*
 * 	Inicjalizacja modułu
 */
void Debug_Init(void)
{

	Uart_Init();
	Uart_BufClr();
	if(Timer_Register(&debugTimer,1000,timerOpt_AUTORESET) != 1)PC_Debug("Blad rejestracji timera!\n\r");
	PC_Debug("\n\ruSensor  v2.0\n\r");
}


void Debug_Main(void)
{
	Debug_InputHandler();

	if(Timer_Check(&debugTimer)==1){
		Debug_Raport();
	}
}

void Debug_Main_InputH(void)
{
	Debug_InputHandler();
}

static void Debug_Raport(void)
{
//	phot_measures_T pomiary;

	switch(msgFormat){
	case DTIME:
//		sprintf(cbuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d \n\r",sTime.date,sTime.month,sTime.year,sTime.hours,sTime.minutes,sTime.seconds);
		PC_Debug(cbuf);
		break;

	case DATA:
//		PHOT_GetMeas(&pomiary);
//
//		sprintf(cbuf,"%d %d %d %d C1: %d C2: %d\n\r",pomiary.photo_A,pomiary.photo_B,
//				pomiary.photo_C,pomiary.photo_D,coil1,coil2);
		PC_Debug(cbuf);
		break;
	default:
		break;
	}
	PC_Debug("\r\n\r\n");
}

static void Debug_InputHandler(void)
{
	uint8_t p;
	uint16_t temp=0;
//	drv_meas_T drv_measure;

	if(debugBufRDY==1){
		debugBufRDY=0;

		for(uint8_t i=0;i<debugBufCnt;i++){
			if((debugBuf[i] <= 'z') && (debugBuf[i]>='a')){
				debugBuf[i] = debugBuf[i]-('a'-'A');
			}
		}

		switch(debugBuf[0]){

		case 'F':{

			p = 1;
			while(p<50){
				if((debugBuf[p]=='\r') || (debugBuf[p]=='\n'))break;
				if(temp!=0)temp*=10;
				temp=temp+(debugBuf[p]-48);
				p++;
			}

			coil1 = temp;
//			DAC_SetVoltage(I2C1,coil1);
			PC_Debug("COIL1 Set\n\r");
			}
			break;

		case 'T':{

			p = 1;
			while(p<50){
				if((debugBuf[p]=='\r') || (debugBuf[p]=='\n'))break;
				if(temp!=0)temp*=10;
				temp=temp+(debugBuf[p]-48);
				p++;
			}

			coil2 = temp;
//			DAC_SetVoltage(I2C2,coil2);
			PC_Debug("COIL2 Set\n\r");
			}
			break;

		case 'W':{
//				DRV_GetMeas(&drv_measure);
//				sprintf(cbuf,"MEAS: %d\n\r", abs(copy_meas-drv_measure.meas));
				PC_Debug("W!!");
				}
				break;
		case 'R':{
//				DRV_GetMeas(&drv_measure);
//				copy_meas=drv_measure.meas;
//				drv_measure.meas=0;
//				sprintf(cbuf,"MEAS: %d\n\r", drv_measure.meas);
//				PC_Debug(cbuf);
				}
				break;
		case 'E':{
				debug_record.flaga_pomiar=1;
				break;
				}
		case 'S':{
				debug_record.flaga_step=1;
				break;
				}
		case 'D':{
				debug_record.flaga_dir=1;
				if(debugBuf[1] == 'E'){
					debug_record.flaga_dir=0;
					}
				break;
				}
		case 'B':{
					debug_record.flaga_button=1;
					break;
					}
		default:
			PC_Debug("Blad!\n\r");
			break;
		}
		Uart_BufClr();

	}
	else if(debugOVF==1){
		Uart_BufClr();
	}
}

/*
 * 	Nadawanie przez UART
 */
void PC_Debug(volatile char *s)
{
	while(*s){
		while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
		USART_SendData(USART3, *s++);
	}
}
/*
 * Legacy
 */
void PC_Send(volatile char *s)
{
	PC_Debug(s);
}
/*
 * Odbi�r
 */
void USART3_IRQHandler(void)
{
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET){
		if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE)){
			if(debugBufCnt < DEBUG_BUF_LENGTH){
				debugBuf[debugBufCnt] = USART_ReceiveData(USART3);
				if(debugBuf[debugBufCnt] == '\r'){
					debugBufRDY=1;
					USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
				}
				else{
					debugBufCnt++;
				}
			}
			else{
				USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
				debugOVF=1;
			}
		}
	}
}
/*
 * Przygotowanie bufora
 */
static void Uart_BufClr()
{
	for(uint16_t i=0;i<DEBUG_BUF_LENGTH;i++){
		debugBuf[i]=0;
	}
	debugBufCnt=0;
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}
/*
 * Konfiguracja UART:
 *
 * 	USART1: Komunikacja debug
 *
 * 	PB: 6,7
 *
 * 	115200,8,1,0,0
 *
 */
static void Uart_Init()
{
	USART_InitTypeDef UART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	// GPIO Config
	// TX - PB3 RX - PB4
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

	// UART Config

	UART_InitStructure.USART_BaudRate = 115200;
	UART_InitStructure.USART_WordLength = USART_WordLength_8b;
	UART_InitStructure.USART_StopBits = USART_StopBits_1;
	UART_InitStructure.USART_Parity = USART_Parity_No;
	UART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	UART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART3,&UART_InitStructure);
	USART_Cmd(USART3, ENABLE);


	// NVIC Config
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

 	NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}




