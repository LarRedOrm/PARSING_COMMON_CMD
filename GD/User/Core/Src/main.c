#include "MCU.h"
#include "CAN.h"
#include "commonCMD.h"

#define NUM_OF_FRAME 5

uint32_t num  = 0;
uint32_t temp = 0;
uint32_t j    = 0;

canRxMsgBuf_struct Data_from_Buf;

//-���� ��� �������� �� ������ (GD32F103R) � ���������� (RAPIDA controller)-
CAN_tx_frame_struct tx_frame = {.id.CM   = 1,  // ����� ����� ������������ � �������.
                                .id.CtoM = 0}; // ����������� ��������: ���� ����� ������������ �� ������ (GD32F103R) � ���������� (RAPIDA controller).
//------------------------------------------------------------------------//


int main (void)
{
MCU_Init();

//---������� �������� ������---//
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
CAN_Set.canFilters[0].address        = 7; // ����� ���������� (RAPIDA controller) ������� ����� �������� ����� � ������ (GD32F103R).
CAN_Set.canFilters[0].interface_type = 24;
//-----------------------------------------------------//

CAN_Init(&CAN_Set);

//---Frame init - ���� ��� �������� �� ������ (GD32F103R) � ���������� (RAPIDA controller)-// 
tx_frame.id.PR             = 0;
tx_frame.id.module_address = MODULE_ADDRESS; // ����������� ����� ������ (GD32F103R).
tx_frame.id.PART           = 0;
tx_frame.id.RESERVED       = 0;
tx_frame.id.interface_type = 24;
tx_frame.id.CMD            = 1;
//tx_frame.id.cmd_and_param  = 0xFFF;
tx_frame.id.cmd_type       = 12;
tx_frame.id.param          = 12;
tx_frame.NumOfData         = 8;
//-----------------------------------------------------------------------------------------//

/*
//---�������� ������ � ���������� NUM_OF_FRAME---//
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
SendModuleInfo();

ParsingComCmd(&Data_from_Buf); // ������� ��� �������

while(1)
  {
  CAN_IRQ_tracking();

//  getFromCanRxBuffer(&Data_from_Buf); // ������ ������ �� �������� ������.
    
  if (canRxBufIndex.amountOfElements)
    {
    getFromCanRxBuffer(&Data_from_Buf); // ������ ������ �� �������� ������.
      if (Data_from_Buf.interface_type == 0)
        {
        ParsingComCmd(&Data_from_Buf);
        }
      else
        {;}
    }

//  putIntoCanTxBuffer(&tx_frame);      // �������� ������ � ���������� �����.

    
    
    
//  Blink(); 

/*
  if (canStat.RX_OK)
    {
    Blink();
    }
*/

  }
}



