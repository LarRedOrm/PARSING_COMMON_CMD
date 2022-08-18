/**
  ******************************************************************************
  *
  * @file      MCU.h
  *
  * @brief     Header for MCU.c file.
  *
  * @copyright Copyright (C) 2022 Awada Systems. Все права защищены.
  *
  * @author    Larionov A.S. (larion.alex@mail.ru)
  *
  ******************************************************************************
**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MCU_H
#define __MCU_H

//---Includes-------------------------------------------------------------------//
#include <stdint.h>
//------------------------------------------------------------------------------//

//---Defines--------------------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Types----------------------------------------------------------------------//
/**
  * @brief   Reason for reset enum
  * @details From STM32 Reference manual RM0008.
  */
typedef enum
{
PIN = 0,     /*!< PIN reset flag:                   PINRSTF.  */
POWER,       /*!< Power reset flag:                 PORRSTF.  */
SOFTWARE,    /*!< Software reset flag:              SFTRSTF.  */
INDEP_WDG,   /*!< Independent watchdog reset flag:  IWDGRSTF. */
WINDOW_WDG,  /*!< Window watchdog reset flag:       WWDGRSTF. */
LOW_POWER,   /*!< Low-power reset flag:             LPWRRSTF. */
ERROR_REASON /*!< ERROR                                       */
} ResetReason_enum;


//------------------------------------------------------------------------------//

//---Exported types-------------------------------------------------------------//
//------------------------------------------------------------------------------//


//---Function prototypes--------------------------------------------------------//
void    MCU_Init            (void);
void    SystemReset         (void);
uint8_t Read_MCU_UID        (uint8_t* IDarray);
ResetReason_enum ReadReasonForReboot (void);

void    Blink               (void);
//------------------------------------------------------------------------------//

  
#endif /* __MCU_H */

//***********************************END OF FILE***********************************
