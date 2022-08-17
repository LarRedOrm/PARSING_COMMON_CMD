/**
  ******************************************************************************
  *
  * @file      MCU_GD32F10R.h
  *
  * @brief     Header for MCU_GD32F10R.c file.
  *
  * @copyright Copyright (C) 2022 Awada Systems. ��� ����� ��������.
  *
  * @author    Larionov A.S. (larion.alex@mail.ru)
  *
  ******************************************************************************
**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MCU_GD32F10R_H
#define __MCU_GD32F10R_H

//---Includes-------------------------------------------------------------------//
#include <stdint.h>
//------------------------------------------------------------------------------//

//---Defines--------------------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Types----------------------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Exported types-------------------------------------------------------------//
//------------------------------------------------------------------------------//


//---Function prototypes--------------------------------------------------------//
void    MCU_Init             (void);
void    SystemReset          (void);
void    Read_MCU_UID         (uint32_t* IDarray);
uint8_t ReadReasonForReboot (void);

void     Blink        (void);
//------------------------------------------------------------------------------//

  
#endif /* __MCU_GD32F10R_H */

//***********************************END OF FILE***********************************