#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "delay.h"
#include "debugkom.h"
#include "diag/Trace.h"
#include "coil_driver.h"
#include "phot_meas.h"
#include "temp_meas.h"
#include "u8g.h"
#include "u8g_arm.h"
//#include "driver.h"


//temp_measures_T measuren;
uint16_t spi_read;
u8g_t u8g;


void DispInit(void)
{
  u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_2x_hw_spi, u8g_com_hw_spi_fn);
  //IsDisplayInitialized = true;
  u8g_FirstPage(&u8g);
  do{ } while ( u8g_NextPage(&u8g) );

  u8g_FirstPage(&u8g);
  do
  {
    u8g_SetFont(&u8g, u8g_font_7x14B);
    u8g_SetFontPosTop(&u8g);
    char modelIdFwVersion[32];
    sprintf(modelIdFwVersion, "uSensor v.%d", 2);
    char ucW_Str = u8g_GetStrWidth(&u8g, modelIdFwVersion);
    u8g_DrawStr(&u8g, (128-ucW_Str)/2, 20, modelIdFwVersion);
  } while ( u8g_NextPage(&u8g) );

}

void main(void)
{
	SystemInit();
	DelayInit();

	//*** inicjalizacja debugcom ***
	Debug_Init();
	DispInit();
	//*** inicjalizacja dac ***
//	DAC_OUT_Coil_Ctrl_Init();
//	uint16_t value = 3000; 	//16bit
//	DAC_SetVoltage(value);

	//*** inicjalizacja wewn�trznego adc - temp_meas ***
//	Temp_SENS_Init();
//	TEMP_GetMeas(measuren);

	//*** inicjalizacja zewnetrznego adc phot_meas ***
//	Photo_Init();

//	Spi_Write8bit(0x77);					//COMMUNICATIONS REGISTER -> STATUS REGISTER
//	spi_read = Spi_Read8bit(0x71);			// set ERR, CH0
//	printf('read: %d', spi_read);



	while(1)
	{
		Debug_Main();
	}
}


