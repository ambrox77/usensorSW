#include "u8g_arm.h"
#include "BusyDelay.h"

void u8g_Delay(uint16_t val){
	BusyDelay_Us(1000UL*(uint32_t)val);
}

void u8g_MicroDelay(void){
	BusyDelay_Us(1);
}

void u8g_10MicroDelay(void){
	BusyDelay_Us(10);
}

void set_gpio_level(GPIO_TypeDef* reg, uint16_t pin, uint8_t level){
  if ( level == 0 ) {
	  GPIO_ResetBits(reg, pin);
  }
  else {
	  GPIO_SetBits(reg, pin);
  }
}

void SPI2_Init(void){
	GPIO_InitTypeDef SPI_PinInit;
	SPI_InitTypeDef SPI2_Init;

	RCC_AHB1PeriphClockCmd(SPI_PORT_PCLK, ENABLE);
	GPIO_StructInit(&SPI_PinInit);
	SPI_PinInit.GPIO_Pin = SPI_SCK_PIN | SPI_MOSI_PIN;
	SPI_PinInit.GPIO_Mode = GPIO_Mode_AF;
	SPI_PinInit.GPIO_OType = GPIO_OType_PP;
	SPI_PinInit.GPIO_Speed = GPIO_Speed_100MHz;
	SPI_PinInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(SPI_PORT, &SPI_PinInit);
	GPIO_PinAFConfig(SPI_PORT, SPI_SCK_PIN_SOURCE, GPIO_AF_SPI2);
	GPIO_PinAFConfig(SPI_PORT, SPI_MOSI_PIN_SOURCE, GPIO_AF_SPI2);

	//CS_OFF(); //czy tu na pewno CS=0?

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE); //clock SPI
	SPI_StructInit(&SPI2_Init);
	SPI2_Init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI2_Init.SPI_CPHA = SPI_CPHA_2Edge; // CPHA = 1
	SPI2_Init.SPI_CPOL = SPI_CPOL_High; // CPOL = 1
	SPI2_Init.SPI_CRCPolynomial = 7; // standard CRC-8
	SPI2_Init.SPI_DataSize = SPI_DataSize_8b;//
	SPI2_Init.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI2_Init.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI2_Init.SPI_Mode = SPI_Mode_Master;
	SPI2_Init.SPI_NSS = SPI_NSS_Soft;
	SPI_Init (SPI2, &SPI2_Init);
	SPI_Cmd	 (SPI2, ENABLE);
}

void SPI_Out (unsigned char ucData){
	SPI2->DR = ucData;
	while ((SPI2->SR & SPI_I2S_FLAG_TXE) == 0); //wait until transmit complete
	while (SPI2->SR & SPI_I2S_FLAG_BSY);
}

uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr){
	switch(msg){
		case U8G_COM_MSG_STOP:
			//STOP THE DEVICE
		break;

		case U8G_COM_MSG_INIT:{
			//INIT HARDWARE INTERFACES, TIMERS, GPIOS...

			//Display ports init
			GPIO_InitTypeDef DISP_Pin_Init;
			RCC_AHB1PeriphClockCmd(DISP_PORT_PCLK, ENABLE);
			DISP_Pin_Init.GPIO_Pin = DISP_CS_PIN | DISP_A0_PIN | DISP_RST_PIN;
			DISP_Pin_Init.GPIO_Mode = GPIO_Mode_OUT;
			DISP_Pin_Init.GPIO_OType = GPIO_OType_PP;
			DISP_Pin_Init.GPIO_Speed = GPIO_Speed_100MHz;
			DISP_Pin_Init.GPIO_PuPd = GPIO_PuPd_NOPULL;
			GPIO_Init(DISP_PORT, &DISP_Pin_Init);

			SPI2_Init();
			u8g_MicroDelay();
		}
		break;

		case U8G_COM_MSG_ADDRESS:
			//SWITCH FROM DATA TO COMMAND MODE /* define cmd (arg_val = 0) or data mode (arg_val = 1) */
			u8g_10MicroDelay();
			set_gpio_level(DISP_PORT, DISP_A0_PIN, arg_val);
			u8g_10MicroDelay();
		break;

		case U8G_COM_MSG_CHIP_SELECT:
			if ( arg_val == 0 ) {
				// disable
				uint8_t i;
				// this delay is required to avoid that the display is switched off too early --> DOGS102 with LPC1114
				for( i = 0; i < 3; i++ )
					u8g_10MicroDelay();
				set_gpio_level(DISP_PORT, DISP_CS_PIN, 1);
			}
			else {
				/* enable */
				set_gpio_level(DISP_PORT, DISP_CS_PIN, 0);
			}
			u8g_MicroDelay();
		break;

		case U8G_COM_MSG_RESET:
			//TOGGLE THE RESET PIN ON THE DISPLAY BY THE VALUE IN arg_val
			set_gpio_level(DISP_PORT, DISP_RST_PIN, arg_val);
			u8g_10MicroDelay();
		break;

		case U8G_COM_MSG_WRITE_BYTE:
			//WRITE BYTE TO DEVICE
			SPI_Out(arg_val);
			u8g_MicroDelay();
		break;

		case U8G_COM_MSG_WRITE_SEQ:
		case U8G_COM_MSG_WRITE_SEQ_P:{
			//WRITE A SEQUENCE OF BYTES TO THE DEVICE
			register uint8_t *ptr = arg_ptr;
			while( arg_val > 0 ){
				SPI_Out(*ptr++);
				arg_val--;
			}
		} break;
	}
	return 1;
}



