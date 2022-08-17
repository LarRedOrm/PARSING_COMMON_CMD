/**
  ******************************************************************************
  * 
  * @file      MCU_GD32F10R.c
  * 
  * @brief     ������������� ������� MCU_GD32F10R.
  * 
  * @details   ������� ��� ������ � MCU_GD32F10R.
  *
  * @warning   **�����!**                                                                     \n 
  * ��� ������ �������� ��������� ���������� ������� flash ��� ����������� ����������������.
  *                       + ���� .c - ;
  *                       + ���� .h - header for .c.
  *
  * **Manual**                                                                                \n 
  * � �������� ����������� ��������� �������:
  *
  * \n \n 
  *
  * @copyright Copyright (C) 2022 Awada Systems. ��� ����� ��������.
  *
  * @author    Larionov A.S. (larion.alex@mail.ru)
  * 
  ******************************************************************************
**/

//---Includes-------------------------------------------------------------------//
#include "MCU_GD32F103R.h"
#include "gd32f10x.h"
//------------------------------------------------------------------------------//

//---Private macros-------------------------------------------------------------//
/** @defgroup GD_BOARD_SELECT
  * @brief  	������� ��� ������ ������ ������ ���������.
  * @{
  */
//#define GD32F103R_MCU                      /*!< - ������ ��������� �� ���������������� GD32F103R. */
#define GD32F103C_EVAL_BOARD               /*!< - ������ ��������� � ������� ����� GD GD32F103C_EVAL � ������� Operating modes. */
//#define GD32F103C_EVAL_BOARD_LOOPBACK_MODE /*!< - ������ ��������� � ������� ����� GD GD32F103C_EVAL � ������ Test Loop back mode. */
/**
  * @}
  */


/** @defgroup NVIC_MACROS
  * @brief  	������� ��� ��������� ����� NVIC.
  * @{
  */
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0   ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1   ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2   ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3   ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4   ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif
/**
  * @}
  */
//------------------------------------------------------------------------------//

//---Second includes------------------------------------------------------------//
#if defined GD32F103C_EVAL_BOARD || defined GD32F103C_EVAL_BOARD_LOOPBACK_MODE
  #include "systick.h"
#endif
//------------------------------------------------------------------------------//

//---Exported variables---------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Private types--------------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Private variables----------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Private constants----------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Function prototypes--------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Exported functions---------------------------------------------------------//

/**
  * @brief   ������������� ����������������.
  * @details ��������� ������������:
  *          ���. ������� -> internal 8MRC (8MHz) -> external HXTAL (8MHz) -> PLL -> \n 
  *          -> 96MHz -> CK_SYS 96MHz -> AHB 96MHz -> /1 -> APB2 96 MHz              \n 
  *                                               |                                  \n 
  *                                                -> /2 -> APB1 48 MHz
  * @param   None.
  * @return  None.
  */
void MCU_Init (void)
{
RCU_RSTSCK |= RCU_RSTSCK_RSTFC; // Reset flag clear. This bit is set by software to clear all reset flags in Reset source/clock register (RCU_RSTSCK).
RCU_APB1EN |= RCU_APB1EN_PMUEN; // Enabled power management unit (PMU) clock.

NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);                                        // System interrupt init.
NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0)); // SysTick_IRQn interrupt configuration.
/*
LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {;}
*/
  
#if defined GD32F103C_EVAL_BOARD || defined GD32F103C_EVAL_BOARD_LOOPBACK_MODE
//---Configure the system clock---
systick_config();
SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; // ���������� ���������� �� ������� SysTick.

//---��������� GPIO-----------------------------------------//
RCU_APB2EN |= RCU_APB2EN_PCEN; // IO port C clock enabled.
RCU_APB2EN |= RCU_APB2EN_PEEN; // IO port E clock enabled.

//---PC0, PC2 ---> LED2, LED3---//
GPIO_CTL0(GPIOC) &= (~( GPIO_CTL0_MD0 | GPIO_CTL0_CTL0 |
                        GPIO_CTL0_MD2 | GPIO_CTL0_CTL2 )); // Clear.

GPIO_CTL0(GPIOC) |= GPIO_MODE_SET(0, (GPIO_MODE_OUT_PP |
                                      GPIO_OSPEED_10MHZ)
                                      & (uint32_t)0x0FU); // GPIO output with push-pull.  
GPIO_CTL0(GPIOC) |= GPIO_MODE_SET(2, (GPIO_MODE_OUT_PP |
                                      GPIO_OSPEED_10MHZ)
                                      & (uint32_t)0x0FU); // GPIO output with push-pull.  
//------------------------------//                        
                        
//---PE0, PE2 ---> LED4, LED5---//
GPIO_CTL0(GPIOE) &= (~( GPIO_CTL0_MD0 | GPIO_CTL0_CTL0 |
                        GPIO_CTL0_MD2 | GPIO_CTL0_CTL2 )); // Clear.
GPIO_CTL0(GPIOE) |= GPIO_MODE_SET(0, (GPIO_MODE_OUT_PP |
                                      GPIO_OSPEED_10MHZ)
                                      & (uint32_t)0x0FU); // GPIO output with push-pull.  
GPIO_CTL0(GPIOE) |= GPIO_MODE_SET(2, (GPIO_MODE_OUT_PP |
                                      GPIO_OSPEED_10MHZ)
                                      & (uint32_t)0x0FU); // GPIO output with push-pull.  
//------------------------------//
//----------------------------------------------------------//
#endif
}
//------------------------------------------------------------------------------//


/**
  * @brief   System Reset.
  * @details Initiates a system reset request to reset the MCU.
  * @return  None.
  */
void SystemReset (void)
{
NVIC_SystemReset();
}
//------------------------------------------------------------------------------//


/**
  * @brief   ������ Unique device ID.
  * @details ������ �������� Unique device ID � ������������ ���������������� (STM, GD, AT).
  * @param   IDarray - ��������� �� ������ ���� uint32_t. ������ �������� Unique device ID.
  * @return  None.
  */
void Read_MCU_UID (uint32_t* IDarray)
{
IDarray [0] = *(uint32_t*)0x1FFFF7E8;
IDarray [1] = *(uint32_t*)0x1FFFF7EC;
IDarray [2] = *(uint32_t*)0x1FFFF7F0;
}
//------------------------------------------------------------------------------//


/**
  * @brief   __weak ������� - ������ ���� ������� �����������.
  * @details ������� ���������� ��� ������� ����������� ������ (���������������� (STM, GD, AT)).
  * @return  None.
  */
uint8_t ReadReasonForReboot (void)
{
uint32_t temp = RCU_RSTSCK; // Reset source/clock register (RCU_RSTSCK).
temp = (temp >> 26);

return (uint8_t)temp;
}
//------------------------------------------------------------------------------//





/**
  * @brief   __weak ������� - �������.
  * @return  None.
  */
#if defined GD32F103C_EVAL_BOARD || defined GD32F103C_EVAL_BOARD_LOOPBACK_MODE
void Blink (void)
{
GPIO_OCTL(GPIOC) |= GPIO_OCTL_OCTL0;
delay_1ms(100);
GPIO_OCTL(GPIOC) &= ~GPIO_OCTL_OCTL0;
delay_1ms(100);
}
#endif
//------------------------------------------------------------------------------//


//---Private functions----------------------------------------------------------//
//------------------------------------------------------------------------------//


//***************************************END OF FILE**************************************//
