#include "MCU.h"

#include "ParsingCommonCMD.h"

//#define WRITE_DATA_TO_FLASH // ����������������� � ������ ���� ���������� �������� ����� ������ � ������� FLASH ������ Config Page � RO Constants.

#define NUM_OF_FRAME 5


uint32_t num  = 0;
uint32_t temp = 0;
uint32_t j    = 0;

canRxMsgBuf_struct Data_from_Buf;

//-���� ��� �������� �� ������ (GD32F103R) � ���������� (RAPIDA controller)-
CAN_tx_frame_struct tx_frame = {.id.CM   = 1,  // ����� ����� ������������ � �������.
                                .id.CtoM = 0}; // ����������� ��������: ���� ����� ������������ �� ������ (GD32F103R) � ���������� (RAPIDA controller).
//------------------------------------------------------------------------//

#ifdef WRITE_DATA_TO_FLASH
//---���� ������������ ��� ������ ������ �� FLASH---//
Config_struct       conf_to_flash;
RO_Constants_struct const_to_flash;
//--------------------------------------------------//
#endif
                                
int main (void)
{
MCU_Init();
  
#ifdef WRITE_DATA_TO_FLASH
//---���� ������������ ��� ������ ������ �� FLASH---//
/*
  * **����� ������ Config Page**
  * | ��������                        |   �����    |   0x00   |   0x01   | 0x02 | 0x03 |
  * | ------------------------------- | :--------: | :------: | :------: | :--: | :--: |
  * | ����� ������                    | 0x0801F000 |   addr   |   0xFF   | 0xFF | 0xFF |
  * | �������� CAN                    | 0x0801F004 |          |          |      |      |
  * | ������ ����������               | 0x0801F008 | bl_minor | bl_major | 0xFF | 0xFF |
  * | ������ ���������                | 0x0801F00C | sw_minor | sw_major | 0xFF | 0xFF |
  * | ���� ������� �������            | 0x0801F010 |   flag   |   0xFF   | 0xFF | 0xFF |
  * | �� ������������ � ������ Modbus | 0x0801F014 |   0xFF   |   0xFF   | 0xFF | 0xFF |
  * | ��������� Modbus ����� 0        | 0x0801F018 |   baud   |    par   | stop | 0xFF |
  * | ��������� Modbus ����� 1        | 0x0801F01C |   baud   |    par   | stop | 0xFF |
  * | ��������� Modbus ����� 2        | 0x0801F020 |   baud   |    par   | stop | 0xFF |
  * | ��������� Modbus ����� 3        | 0x0801F024 |   baud   |    par   | stop | 0xFF |
*/
conf_to_flash.AddrModule                      = 14;           /*!< ����� ������.                  */
conf_to_flash.CanSpeed                        = CAN_BUS_250K; /*!< �������� ������ CAN (�������). */
conf_to_flash.BootloaderVersion.minor         = 1;            /*!< ������ ����������.             */
conf_to_flash.BootloaderVersion.major         = 2;            /*!< ������ ����������.             */
conf_to_flash.ProgramVersion.minor            = 3;            /*!< ������ ���������.              */
conf_to_flash.ProgramVersion.major            = 4;            /*!< ������ ���������.              */
conf_to_flash.FirstRunFlag                    = 0xAABBCCDD;   /*!< ���� ������� �������.          */
conf_to_flash.ModbusPort1Param.baud           = 0x11;         /*!< ��������� Modbus ����� 1.      */
conf_to_flash.ModbusPort1Param.par            = 0x22;         /*!< ��������� Modbus ����� 1.      */
conf_to_flash.ModbusPort1Param.stop           = 0x33;         /*!< ��������� Modbus ����� 1.      */
conf_to_flash.ModbusPort1Param.reserved_param = 0x44;         /*!< ��������� Modbus ����� 1.      */
conf_to_flash.ModbusPort3Param.baud           = 0x55;         /*!< ��������� Modbus ����� 3.      */
conf_to_flash.ModbusPort3Param.par            = 0x66;         /*!< ��������� Modbus ����� 3.      */
conf_to_flash.ModbusPort3Param.stop           = 0x77;         /*!< ��������� Modbus ����� 3.      */
conf_to_flash.ModbusPort3Param.reserved_param = 0x88;         /*!< ��������� Modbus ����� 3.      */
  
Write_Config_to_flash (&conf_to_flash); // ������ �� FLASH ������ �� Config Page.
/*
  * **����� ������ RO Constants**
  * | ��������           |   �����    |   0x00   |   0x01   | 0x02 | 0x03 |
  * | ------------------ | :--------: | :------: | :------: | :--: | :--: |
  * | ��� ������         | 0x0801F800 |  class   |   0xFF   | 0xFF | 0xFF |
  * | ���������� ������� | 0x0801F804 | hw_minor | hw_major | 0xFF | 0xFF |
  * | �������� ����� lw  | 0x0801F808 |   0xFF   |   0xFF   | 0xFF | 0xFF |
  * | �������� ����� hw  | 0x0801F80C |   0xFF   |   0xFF   | 0xFF | 0xFF |
*/
const_to_flash.ModulType              = 0xAB;       /*!< ��� ������.                   */
const_to_flash.HardwareRevision.minor = 0x11;       /*!< ���������� �������.           */
const_to_flash.HardwareRevision.major = 0x22;       /*!< ���������� �������.           */
const_to_flash.reserved_hrev          = 0xABCD;     /*!< ���������������.              */
const_to_flash.SerialNumberLW         = 0x11223344; /*!< �������� ����� ������� �����. */
const_to_flash.SerialNumberHW         = 0x55667788; /*!< �������� ����� ������� �����. */

Write_Words_to_flash (RO_CONST_START_ADDR_IN_FLASH, 4, (uint32_t*)&const_to_flash); // ������ �� FLASH ������ �� RO Constants.
//--------------------------------------------------//
#endif


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
//Read_Config_from_flash(&conf_from_flash);
CAN_Settings CAN_Set;
CAN_Set.baudrate                     = *((can_speed*)CAN_SPEED_ADDR_IN_FLASH);
CAN_Set.NumOfFilters                 = 1;
CAN_Set.canFilters[0].filterNumber   = 1;
//CAN_Set.canFilters[0].address        = 7; // ����� ���������� (RAPIDA controller) ������� ����� �������� ����� � ������ (GD32F103R).
CAN_Set.canFilters[0].address        = ( *((uint8_t*)MODULE_ADDR_IN_FLASH) & MODULE_ADDR_MASK); // ����������� ����� ������ (���������������� STM, GD, AT ...).
CAN_Set.canFilters[0].interface_type = 0;
CAN_Init(&CAN_Set);
//-----------------------------------------------------//

/*
//---Frame init - ���� ��� �������� �� ������ (GD32F103R) � ���������� (RAPIDA controller)-// 
tx_frame.id.PR             = 0;
tx_frame.id.module_address = ( *((uint8_t*)MODULE_ADDR_IN_FLASH) & MODULE_ADDR_MASK); // ����������� ����� ������ (���������������� STM, GD, AT ...).
tx_frame.id.PART           = 0;
tx_frame.id.RESERVED       = 0;
tx_frame.id.interface_type = 24;
tx_frame.id.CMD            = 1;
//tx_frame.id.cmd_and_param  = 0xFFF;
tx_frame.id.cmd_type       = 12;
tx_frame.id.param          = 12;
tx_frame.NumOfData         = 8;
//-----------------------------------------------------------------------------------------//
*/

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

//ParsingComCmd(&Data_from_Buf); // ������� ��� �������

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

    
    
    
  Blink();

/*
  if (canStat.RX_OK)
    {
    Blink();
    }
*/

  }
}



