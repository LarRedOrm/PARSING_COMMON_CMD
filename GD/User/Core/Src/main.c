#include "MCU.h"

#include "ParsingCommonCMD.h"

//#define WRITE_DATA_TO_FLASH // Раскомментировать в случае если необходимо записать новые данные в области FLASH памяти Config Page и RO Constants.

#define NUM_OF_FRAME 5


uint32_t num  = 0;
uint32_t temp = 0;
uint32_t j    = 0;

canRxMsgBuf_struct Data_from_Buf;

//-Кадр для передачи из модуля (GD32F103R) в контроллер (RAPIDA controller)-
CAN_tx_frame_struct tx_frame = {.id.CM   = 1,  // Обмен между контроллером и модулем.
                                .id.CtoM = 0}; // Направление передачи: кадр будет передаваться из модуля (GD32F103R) в контроллер (RAPIDA controller).
//------------------------------------------------------------------------//

#ifdef WRITE_DATA_TO_FLASH
//---Блок используется для записи данных во FLASH---//
Config_struct       conf_to_flash;
RO_Constants_struct const_to_flash;
//--------------------------------------------------//
#endif
                                
int main (void)
{
MCU_Init();
  
#ifdef WRITE_DATA_TO_FLASH
//---Блок используется для записи данных во FLASH---//
/*
  * **Карта памяти Config Page**
  * | Параметр                        |   Адрес    |   0x00   |   0x01   | 0x02 | 0x03 |
  * | ------------------------------- | :--------: | :------: | :------: | :--: | :--: |
  * | Адрес модуля                    | 0x0801F000 |   addr   |   0xFF   | 0xFF | 0xFF |
  * | Скорость CAN                    | 0x0801F004 |          |          |      |      |
  * | Версия загрузчика               | 0x0801F008 | bl_minor | bl_major | 0xFF | 0xFF |
  * | Версия программы                | 0x0801F00C | sw_minor | sw_major | 0xFF | 0xFF |
  * | Флаг первого запуска            | 0x0801F010 |   flag   |   0xFF   | 0xFF | 0xFF |
  * | Не используется в модуле Modbus | 0x0801F014 |   0xFF   |   0xFF   | 0xFF | 0xFF |
  * | Параметры Modbus порта 0        | 0x0801F018 |   baud   |    par   | stop | 0xFF |
  * | Параметры Modbus порта 1        | 0x0801F01C |   baud   |    par   | stop | 0xFF |
  * | Параметры Modbus порта 2        | 0x0801F020 |   baud   |    par   | stop | 0xFF |
  * | Параметры Modbus порта 3        | 0x0801F024 |   baud   |    par   | stop | 0xFF |
*/
conf_to_flash.AddrModule                      = 14;           /*!< Адрес модуля.                  */
conf_to_flash.CanSpeed                        = CAN_BUS_250K; /*!< Скорость работы CAN (битрейт). */
conf_to_flash.BootloaderVersion.minor         = 1;            /*!< Версия загрузчика.             */
conf_to_flash.BootloaderVersion.major         = 2;            /*!< Версия загрузчика.             */
conf_to_flash.ProgramVersion.minor            = 3;            /*!< Версия программы.              */
conf_to_flash.ProgramVersion.major            = 4;            /*!< Версия программы.              */
conf_to_flash.FirstRunFlag                    = 0xAABBCCDD;   /*!< Флаг первого запуска.          */
conf_to_flash.ModbusPort1Param.baud           = 0x11;         /*!< Параметры Modbus порта 1.      */
conf_to_flash.ModbusPort1Param.par            = 0x22;         /*!< Параметры Modbus порта 1.      */
conf_to_flash.ModbusPort1Param.stop           = 0x33;         /*!< Параметры Modbus порта 1.      */
conf_to_flash.ModbusPort1Param.reserved_param = 0x44;         /*!< Параметры Modbus порта 1.      */
conf_to_flash.ModbusPort3Param.baud           = 0x55;         /*!< Параметры Modbus порта 3.      */
conf_to_flash.ModbusPort3Param.par            = 0x66;         /*!< Параметры Modbus порта 3.      */
conf_to_flash.ModbusPort3Param.stop           = 0x77;         /*!< Параметры Modbus порта 3.      */
conf_to_flash.ModbusPort3Param.reserved_param = 0x88;         /*!< Параметры Modbus порта 3.      */
  
Write_Config_to_flash (&conf_to_flash); // Запись во FLASH данных из Config Page.
/*
  * **Карта памяти RO Constants**
  * | Параметр           |   Адрес    |   0x00   |   0x01   | 0x02 | 0x03 |
  * | ------------------ | :--------: | :------: | :------: | :--: | :--: |
  * | Тип модуля         | 0x0801F800 |  class   |   0xFF   | 0xFF | 0xFF |
  * | Аппаратная ревизия | 0x0801F804 | hw_minor | hw_major | 0xFF | 0xFF |
  * | Серийный номер lw  | 0x0801F808 |   0xFF   |   0xFF   | 0xFF | 0xFF |
  * | Серийный номер hw  | 0x0801F80C |   0xFF   |   0xFF   | 0xFF | 0xFF |
*/
const_to_flash.ModulType              = 0xAB;       /*!< Тип модуля.                   */
const_to_flash.HardwareRevision.minor = 0x11;       /*!< Аппаратная ревизия.           */
const_to_flash.HardwareRevision.major = 0x22;       /*!< Аппаратная ревизия.           */
const_to_flash.reserved_hrev          = 0xABCD;     /*!< Зарезервировано.              */
const_to_flash.SerialNumberLW         = 0x11223344; /*!< Серийный номер младшее слово. */
const_to_flash.SerialNumberHW         = 0x55667788; /*!< Серийный номер старшее слово. */

Write_Words_to_flash (RO_CONST_START_ADDR_IN_FLASH, 4, (uint32_t*)&const_to_flash); // Запись во FLASH данных из RO Constants.
//--------------------------------------------------//
#endif


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
//Read_Config_from_flash(&conf_from_flash);
CAN_Settings CAN_Set;
CAN_Set.baudrate                     = *((can_speed*)CAN_SPEED_ADDR_IN_FLASH);
CAN_Set.NumOfFilters                 = 1;
CAN_Set.canFilters[0].filterNumber   = 1;
//CAN_Set.canFilters[0].address        = 7; // Адрес устройства (RAPIDA controller) которое будет посылать кадры в модуль (GD32F103R).
CAN_Set.canFilters[0].address        = ( *((uint8_t*)MODULE_ADDR_IN_FLASH) & MODULE_ADDR_MASK); // Собственный адрес модуля (микроконтроллера STM, GD, AT ...).
CAN_Set.canFilters[0].interface_type = 0;
CAN_Init(&CAN_Set);
//-----------------------------------------------------//

/*
//---Frame init - кадр для передачи из модуля (GD32F103R) в контроллер (RAPIDA controller)-// 
tx_frame.id.PR             = 0;
tx_frame.id.module_address = ( *((uint8_t*)MODULE_ADDR_IN_FLASH) & MODULE_ADDR_MASK); // Собственный адрес модуля (микроконтроллера STM, GD, AT ...).
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
SendModuleInfo();

//ParsingComCmd(&Data_from_Buf); // СТРОЧКА ДЛЯ ОТЛАДКИ

while(1)
  {
  CAN_IRQ_tracking();

//  getFromCanRxBuffer(&Data_from_Buf); // Чтение кадров из приёмного буфера.

  if (canRxBufIndex.amountOfElements)
    {
    getFromCanRxBuffer(&Data_from_Buf); // Чтение кадров из приёмного буфера.
      if (Data_from_Buf.interface_type == 0)
        {
        ParsingComCmd(&Data_from_Buf);
        }
      else
        {;}
    }

//  putIntoCanTxBuffer(&tx_frame);      // Отправка кадров в передающий буфер.

    
    
    
  Blink();

/*
  if (canStat.RX_OK)
    {
    Blink();
    }
*/

  }
}



