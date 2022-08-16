#include "commonCMD.h"
#include "CAN.h"


#define GD
//#define AT

#ifdef GD
#include "CAN_GD32F103R.h"
#define GD32F103C_EVAL_BOARD
#endif

#ifdef AT
#include "CAN.h"
#endif

#define NUM_OF_FRAME 5

uint32_t num  = 0;
uint32_t temp = 0;
uint32_t j    = 0;

canRxMsgBuf_struct Data_from_Buf;

//-Кадр для передачи из модуля (GD32F103R) в контроллер (RAPIDA controller)-
CAN_tx_frame_struct tx_frame = {.id.CM   = 1,  // Обмен между контроллером и модулем.
                                .id.CtoM = 0}; // Направление передачи: кадр будет передаваться из модуля (GD32F103R) в контроллер (RAPIDA controller).
//------------------------------------------------------------------------//


int main (void)
{
//---Очистка приёмного буфера---//
for (uint8_t i = 0; i<CAN_RX_BUF_SIZE; i++)
  {
  for (uint8_t j = 0; j<8; j++)
    {
    canRxMsgBuf[i].Data[j] = 0;
    }
  }
//------------------------------//


//---CAN init------------------------------------------//
CAN_Settings CAN_Set;
CAN_Set.baudrate                     = CAN_BUS_250K;
CAN_Set.NumOfFilters                 = 1;
CAN_Set.canFilters[0].filterNumber   = 1;
CAN_Set.canFilters[0].address        = 7; // Адрес устройства (RAPIDA controller) которое будет посылать кадры в модуль (GD32F103R).
CAN_Set.canFilters[0].interface_type = 24;
//CAN_Set.NumOfFilters                 = 0; // No filters.
//-----------------------------------------------------//

Init_Driver(&CAN_Set);

//---Frame init - кадр для передачи из модуля (GD32F103R) в контроллер (RAPIDA controller)-// 
tx_frame.id.PR             = 0;
tx_frame.id.module_address = MODULE_ADDRESS; // Собственный адрес модуля (GD32F103R).
tx_frame.id.PART           = 0;
tx_frame.id.RESERVED       = 0;
tx_frame.id.interface_type = 24;
tx_frame.id.CMD            = 1;
tx_frame.id.cmd_and_param  = 0xFFF;
tx_frame.NumOfData         = 8;
//-----------------------------------------------------------------------------------------//

/*
//---Отправка кадров в количестве NUM_OF_FRAME---//
for (uint8_t i=0; i<NUM_OF_FRAME; i++)
  {
  j++;
  for (uint8_t n = 0; n<tx_frame.NumOfData; n++)
    {
    tx_frame.Data[n] = j;
    }
  putIntoCanTxBuffer(&tx_frame);
  }
//-----------------------------------------------//
*/


while(1)
  {
  CAN_IRQ_tracking();

  getFromCanRxBuffer(&Data_from_Buf); // Чтение кадров из приёмного буфера.

  if (Data_from_Buf.interface_type == 0)
    ParsingComCmd(&Data_from_Buf);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
//  putIntoCanTxBuffer(&tx_frame);      // Отправка кадров в передающий буфер.

    
    
    
    
    
    
    
    
    
    
    
    

#if defined GD32F103C_EVAL_BOARD || defined GD32F103C_EVAL_BOARD_LOOPBACK_MODE
  if (canStat.RX_OK)
    {
    GPIO_OCTL(GPIOC) |= GPIO_OCTL_OCTL0;
    delay_1ms(100);
    GPIO_OCTL(GPIOC) &= ~GPIO_OCTL_OCTL0;
    delay_1ms(100);
    GPIO_OCTL(GPIOC) |= GPIO_OCTL_OCTL0;
    }
#endif
  }
}



