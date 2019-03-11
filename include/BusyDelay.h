/*=======================================================================================*
 * @file    BusyDelay.h
 * @author  Damian Pala
 * @date    26-08-2017
 * @brief   Header file for BusyDelay module
 *
 *          This file contains API of BusyDelay module
 *======================================================================================*/
/*----------------------- DEFINE TO PREVENT RECURSIVE INCLUSION ------------------------*/
#ifndef BUSY_DELAY_H_
#define BUSY_DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup BusyDelay Description
 * @{
 * @brief Module for... .
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/
/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include <stdint.h>
#include "cmsis_device.h"

/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                     ####### EXPORTED TYPE DECLARATIONS #######                       */
/*======================================================================================*/
/*---------------------------- ALL TYPE DECLARATIONS -----------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/

/*------------------------------- STRUCT AND UNIONS ------------------------------------*/

/*======================================================================================*/
/*                    ####### EXPORTED OBJECT DECLARATIONS #######                      */
/*======================================================================================*/

/*======================================================================================*/
/*                   ####### EXPORTED FUNCTIONS PROTOTYPES #######                      */
/*======================================================================================*/
void BusyDelay_Init(uint32_t coreClock);
static inline void BusyDelay_Us(uint32_t numberOfUs)
{
  /* SystemCoreClk / 1000000 - 2 = number of NOPs */
  if (numberOfUs)
  {
    __ASM volatile
    (
      ".syntax unified\n"
      "1:\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "NOP\n"
              "SUBS %[numberOfUs], %[numberOfUs], #1\n"
              "BNE 1b\n"
        ".syntax divided\n"
              : [numberOfUs] "+l" (numberOfUs) :
    );
  }
}

void BusyDelay_Ms(uint16_t numberOfMs);

/*======================================================================================*/
/*                          ####### INLINE FUNCTIONS #######                            */
/*======================================================================================*/

/**
 * @} end of group BusyDelay
 */

#ifdef __cplusplus
}
#endif

#endif /* BUSY_DELAY_H_ */
