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
  * @brief Reason for reboot enum
  */
enum ReasonForReboot
{
EPRSTF = 0, /*!< External PIN reset flag.          */
PORRSTF,    /*!< Power reset flag.                 */
SWRSTF,     /*!< Software reset flag.              */
FWDGTRSTF,  /*!< Free watchdog timer reset flag.   */
WWDGTRSTF,  /*!< Window watchdog timer reset flag. */
LPRSTF      /*!< Low-power reset flag.             */
};
//------------------------------------------------------------------------------//

//---Exported types-------------------------------------------------------------//
//------------------------------------------------------------------------------//


//---Function prototypes--------------------------------------------------------//
void    MCU_Init            (void);
void    SystemReset         (void);
void    Read_MCU_UID        (uint32_t* IDarray);
uint8_t ReadReasonForReboot (void);

void    Blink               (void);
//------------------------------------------------------------------------------//

  
#endif /* __MCU_H */

//***********************************END OF FILE***********************************
