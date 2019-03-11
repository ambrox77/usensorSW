/*
 * komunikacja.h
 *
 *  Created on: 12 wrz 2016
 *      Author: Marcin
 */

#ifndef KOMUNIKACJA_H_
#define KOMUNIKACJA_H_

typedef struct{
	uint8_t flaga_pomiar;
	uint8_t flaga_step;
	uint8_t flaga_dir;
	uint8_t flaga_button;
}debugkom_record_T;


#define DEBUG_BUF_LENGTH 256

void Debug_Init(void);

void Debug_Main(void);
void Debug_Main_InputH(void);
void PC_Debug(volatile char *s);


#endif /* KOMUNIKACJA_H_ */
