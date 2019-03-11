#ifndef _U8G_ARM_H
#define _U8G_ARM_H

#include "u8g.h"
#include "stm32f4xx_conf.h"



#define SPI_PORT_PCLK RCC_AHB1Periph_GPIOB
#define SPI_PORT GPIOB
#define SPI_SCK_PIN GPIO_Pin_10
#define SPI_SCK_PIN_SOURCE GPIO_PinSource10
#define SPI_MOSI_PIN GPIO_Pin_15
#define SPI_MOSI_PIN_SOURCE GPIO_PinSource15

#define DISP_PORT_PCLK RCC_AHB1Periph_GPIOB
#define DISP_PORT GPIOB
#define DISP_CS_PIN GPIO_Pin_12
#define DISP_A0_PIN GPIO_Pin_14
#define DISP_RST_PIN GPIO_Pin_13


#define CS_ON()        GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define CS_OFF()       GPIO_ResetBits(GPIOB, GPIO_Pin_12)

void SPI2_Init(void);
void SPI_Out (unsigned char);
void set_gpio_level(GPIO_TypeDef*, uint16_t, uint8_t);

//*************************************************************************

void TimingDelay_Decrement(void);
void u8g_Delay(uint16_t);


uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);

#endif


