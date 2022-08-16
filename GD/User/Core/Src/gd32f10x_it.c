/*!
    \file    gd32f10x_it.c
    \brief   interrupt service routines
    
    \version 2015-11-16, V1.0.0, demo for GD32F10x
    \version 2017-06-30, V2.0.0, demo for GD32F10x
    \version 2021-04-30, V2.1.0, demo for GD32F10x
*/

/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

//---Includes-------------------------------------------------------------------//
#include "gd32f10x_it.h"
#include "systick.h"
#include "CAN_GD32F103R.h"
//------------------------------------------------------------------------------//

//---Defines--------------------------------------------------------------------//
//------------------------------------------------------------------------------//

FlagStatus receive_flag;
//can_receive_message_struct receive_message;
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
delay_decrement();
}

/*!
    \brief      this function handles CAN0 TX exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBD_HP_CAN0_TX_IRQHandler(void)
{
uint32_t CAN_TSTAT_reg = CAN_TSTAT(CAN0);

//---TX mailbox 0 transmit finished---//
if (CAN_TSTAT_reg & CAN_TSTAT_MTF0)     
  {
  if ( (CAN_TSTAT_reg & CAN_TSTAT_MTFNERR0) && (!(CAN_TSTAT_reg & CAN_TSTAT_MTE0)) )
    {
    canStat.TX_OK++;
//    CAN_TX_Start = 1;
    }
  else
    {
    canStat.TX_ERR++;
    }
  CAN_TX();
  CAN_TSTAT(CAN0) |= CAN_TSTAT_MTF0;// This bit reset by software when write 1 to this bit or TEN bit in CAN_TMI0 is 1.
  }
//------------------------------------//

//---TX mailbox 1 transmit finished---//
else if (CAN_TSTAT_reg & CAN_TSTAT_MTF1) // TX mailbox 1 transmit finished.
  {
  if ( (CAN_TSTAT_reg & CAN_TSTAT_MTFNERR1) && (!(CAN_TSTAT_reg & CAN_TSTAT_MTE1)) )
    {
    canStat.TX_OK++;
//    CAN_TX_Start = 1;
    }
  else
    {
    canStat.TX_ERR++;
    }
  CAN_TX();
  CAN_TSTAT(CAN0) |= CAN_TSTAT_MTF1;// This bit reset by software when write 1 to this bit or TEN bit in CAN_TMI1 is 1.
  }
//------------------------------------//

//---TX mailbox 2 transmit finished---//
else if (CAN_TSTAT_reg & CAN_TSTAT_MTF2) // TX mailbox 2 transmit finished.
  {
  if ( (CAN_TSTAT_reg & CAN_TSTAT_MTFNERR2) && (!(CAN_TSTAT_reg & CAN_TSTAT_MTE2)) )
    {
    canStat.TX_OK++;
//    CAN_TX_Start = 1;
    }
  else
    {
    canStat.TX_ERR++;
    }
  CAN_TX();
  CAN_TSTAT(CAN0) |= CAN_TSTAT_MTF2;// This bit reset by software when write 1 to this bit or TEN bit in CAN_TMI2 is 1.
  }
//------------------------------------//
else
  CAN_TSTAT(CAN0) |= CAN_TSTAT_MTF0 | CAN_TSTAT_MTF1 | CAN_TSTAT_MTF2;
}

/*!
    \brief      this function handles CAN0 RX FIFO0 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBD_LP_CAN0_RX0_IRQHandler(void)
{
if (CAN_RFIFO0(CAN0) & CAN_RFIFO0_RFO0) // Receive FIFO0 overfull.
  {
  canStat.RX_MISS++;
  CAN_RX_Start      = 1;                // Флаг запуска функции CAN_RX(). Если CAN_RX_Start = 1, будет вызвана функция CAN_RX() из mainloop.
  CAN_RFIFO0(CAN0) |= CAN_RFIFO0_RFO0;  // This bit is set by hardware when receive FIFO0 is overfull and reset by software when write 1 to this bit.
  }
  
if (CAN_RFIFO0(CAN0) & CAN_RFIFO0_RFF0) // Receive FIFO0 full.
  CAN_RFIFO0(CAN0) |= CAN_RFIFO0_RFF0;

CAN_RX();

/*
// На случай если проверка флага в mainloop не подойдёт,
// можно принудительно вызывать CAN_RX() после overfull, пока CAN буфер не очистится. 
while (CAN_RX_Start)
  CAN_RX();
*/
}

/*!
    \brief      this function handles CAN0 RX FIFO1 exception
    \param[in]  none
    \param[out] none
    \retval     none

void CAN0_RX1_IRQHandler(void)
{
;
}
*/

/*!
    \brief      this function handles CAN0 EWMC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void CAN0_EWMC_IRQHandler(void)
{
if(CAN_ERR(CAN0) & CAN_ERR_RECNT) // Receive Error Count defined by the CAN standard.
  {
  canStat.RX_ERR++;
  }
CAN_STAT(CAN0) |= CAN_STAT_ERRIF; // Clear error interrupt flag. This bit is cleared by software when write 1 to this bit.
}

