/**
  ******************************************************************************
  *
  * @file      ParsingCommonCMD.c
  *
  * @brief     Обработка команд общего (common) типа.
  *
  * @details   
  *            
  * @warning   **ВАЖНО!** \n 
  * Для работы необходимы библиотечные заголовочные файлы перечисленные в блоке Includes. \n 
  * Кроме того требуется:
  *                       + файл .с - файл;
  *                       + файл .h - header for .с.
  *
  * **Manual**
  * Модуль commonCMD.c обеспечивает:
  * - parsing (анализ) сообщения, поступившего от контроллера (RAPIDA) в модуль (микроконтроллер STM, GD, AT),
  *   в случае, если во входящем сообщении в EXID (Extended format frame identifier), поле "Тип интерфейса" имеет значение 0. \n 
  *   Если поле "Тип интерфейса" имеет значение 0, то поступила команда типа "Общая" (common).
  *
  * - вызов требуемой (по результатам анализа) команды и/или формирование ответного сообщения из модуля в контроллер.
  *
  * - вызов при необходимости команды отправки сформированного ответа в передающий буфер CAN.
  *
  * 
  * @copyright Copyright (C) 2022 Awada Systems. Все права защищены.
  *
  * @author    Larionov A.S. (larion.alex@mail.ru)
  *
  ******************************************************************************
**/

//---Includes-------------------------------------------------------------------//
#include "ParsingCommonCMD.h"
#include "MCU.h"
#include <string.h>
//------------------------------------------------------------------------------//

//---Private macros ------------------------------------------------------------//
#define MASK                           0x3F  /*!< Маска для полей "Тип команды" или "Параметры". */

//-Макросы для определения типа команды-//
#define MODULE_INFO_REQUEST            0x00  /*!< Запрос информации о модуле.         */
#define MODULE_SN_REQUEST              0x01  /*!< Запрос серийного номера модуля.     */
#define MODULE_INTERFACE_TYPES_REQUEST 0x02  /*!< Запрос типов интерфейсов модуля.    */
#define MCU_DATA_REQUEST               0x03  /*!< Запрос данных микроконтроллера.     */
#define UID_REQUEST                    0x04  /*!< Запрос UID.                         */
#define MODULE_STATE_REQUEST           0x05  /*!< Запрос состояния модуля.            */
#define CAN_MODE                       0x06  /*!< Изменение/Запрос режима работы CAN. */

#define MODULE_ADDRESS_CHANGE          0x30  /*!< Смена адреса модуля.                */
#define CAN_MODULE_SPEED_CHANGE        0x31  /*!< Смена скорости CAN модуля.          */
#define EXECUTING_PROTECTED_CMD        0x3D  /*!< Выполнение защищенной команды.      */
#define MODULE_RELOAD                  0x3E  /*!< Перезагрузка модуля.                */
#define SUPPORTED_COM_CMD_REQUEST      0x3F  /*!< Запрос поддерживаемых общих команд. */

#define ONE (uint64_t)(0x1)
#define SUPPORTED_COM_CMD_MASK ( (ONE << MODULE_INFO_REQUEST)            | \
                                 (ONE << MODULE_SN_REQUEST)              | \
                                 (ONE << MODULE_INTERFACE_TYPES_REQUEST) | \
                                 (ONE << MCU_DATA_REQUEST)               | \
                                 (ONE << UID_REQUEST)                    | \
                                 (ONE << MODULE_STATE_REQUEST)           | \
                                 (ONE << CAN_MODE)                       | \
                                 (ONE << MODULE_ADDRESS_CHANGE)          | \
                                 (ONE << CAN_MODULE_SPEED_CHANGE)        | \
                                 (ONE << EXECUTING_PROTECTED_CMD)        | \
                                 (ONE << MODULE_RELOAD)                  | \
                                 (ONE << SUPPORTED_COM_CMD_REQUEST) )/*!< Ответ на запрос поддерживаемых общих команд. */
//--------------------------------------//

//-Макросы для определения типа параметров при команде CAN_MODE-//
#define NORMAL_MODE (uint8_t)0x0 /*!< Переключение режима работы CAN на "Normal communication mode". */
#define SILENT_MODE (uint8_t)0x1 /*!< Переключение режима работы CAN на "Silent communication mode". */
#define READ_MODE   (uint8_t)0x2 /*!< Определение  режима работы CAN.                                */
//--------------------------------------------------------------//

//-Макросы для определения параметров при команде UID_REQUEST-//
#define SECTION_LOW  (uint8_t)0x0 /*!< Младшая часть. */
#define SECTION_MID  (uint8_t)0x1 /*!< Средняя часть. */
#define SECTION_HIGH (uint8_t)0x2 /*!< Старшая часть. */
//------------------------------------------------------------//

#define Put16toFrameData (*((uint16_t*)(TXframePointer -> Data))) /*!< Скопировать данные формата uint16_t в TXframePointer -> Data. */
#define Put32toFrameData (*((uint32_t*)(TXframePointer -> Data))) /*!< Скопировать данные формата uint32_t в TXframePointer -> Data. */
#define Put64toFrameData (*((uint64_t*)(TXframePointer -> Data))) /*!< Скопировать данные формата uint64_t в TXframePointer -> Data. */

#define WAS    1
#define WASNOT 0

#define PROTECT_CMD_PARAM (uint8_t)0x15 /*!< Значение параметра для команды "Выполнение защищенной команды". */ 
//------------------------------------------------------------------------------//

//---Exported variables---------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Private types--------------------------------------------------------------//
//uint8_t ComCMDtype [12] = {};
//------------------------------------------------------------------------------//

//---Private variables----------------------------------------------------------//
//-Кадр для передачи из модуля (STM, GD, AT) в контроллер (RAPIDA controller)-
static CAN_tx_frame_struct  CAN_tx_frame; // Структура кадра для отправки.
static Config_struct        Config;       // Структура для хранения данных Config Page.
static RO_Constants_struct  RO_Constants; // Структура для хранения данных RO Constants.

static CAN_tx_frame_struct* TXframePointer = &CAN_tx_frame;
static Config_struct*       ConfigPointer  = &Config;
static RO_Constants_struct* ConstPointer   = &RO_Constants;

static uint8_t module_interface_types [8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}; // Набор констант для ответа на поступившую в модуль команду MODULE_INTERFACE_TYPES_REQUEST.
//------------------------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Private constants----------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Function prototypes--------------------------------------------------------//
void InitStructs (Config_struct* ConfigPointer, RO_Constants_struct*  ConstPointer, CAN_tx_frame_struct* TXframePointer);
//------------------------------------------------------------------------------//

//---Exported functions---------------------------------------------------------//
/**
  * @brief   Передача информации о модуле (микроконтроллере STM, GD, AT).
  * @details 
  * @return  None.
  */
void SendModuleInfo (void)
{
InitStructs (ConfigPointer, ConstPointer, TXframePointer);
//TXframePointer->Data[0] = ConstPointer  -> HardwareRevision.major;
//TXframePointer->Data[1] = ConstPointer  -> HardwareRevision.minor;
//Put16toFrameData = *(uint16_t*)(HARDWARE_REVISION_ADDR_IN_FLASH); // Копируем значение аппаратной ревизии из Flash.
TXframePointer->Data[0] = *(uint8_t*)(HARDWARE_REVISION_ADDR_IN_FLASH + 1); // Копируем значение аппаратной ревизии из Flash.
TXframePointer->Data[1] = *(uint8_t*)(HARDWARE_REVISION_ADDR_IN_FLASH);     // Копируем значение аппаратной ревизии из Flash.
TXframePointer->Data[2] = ConfigPointer -> BootloaderVersion.major;
TXframePointer->Data[3] = ConfigPointer -> BootloaderVersion.minor;
TXframePointer->Data[4] = ConfigPointer -> ProgramVersion.major;
TXframePointer->Data[5] = ConfigPointer -> ProgramVersion.minor;
putIntoCanTxBuffer(&CAN_tx_frame);
}
//------------------------------------------------------------------------------//


/**
  * @brief   Анализ входящего сообщения.
  * @details Анализ сообщения, поступившего от контроллера (RAPIDA) в модуль (микроконтроллер STM, GD, AT),
  *          в случае, если во входящем сообщении в EXID (Extended format frame identifier), поле "Тип интерфейса" имеет значение 0.
  * @param   canRxMsg - указатель типа canRxMsgBuf_struct* на структуру данных, считываемых из приёмного буфера.
  * @return  None.
  */
void ParsingComCmd (canRxMsgBuf_struct* canRxMsg)
{
uint8_t        CMD_type = ((canRxMsg->cmd13param >> 6) & MASK);
static uint8_t IDarray [24] = {0}; // 24 - размер UID в байтах согласно протоколу.
static uint8_t IDsize;
static uint8_t ProtectCMDstate;  // Флаг поступившей команды EXECUTING_PROTECTED_CMD: ProtectCMDstate = WAS    - команда поступила;
                                 //                                                   ProtectCMDstate = WASNOT - команда не поступила;
//CMD_type = MCU_DATA_REQUEST; // СТРОЧКА ДЛЯ ОТЛАДКИ

switch (CMD_type)
  {
  case (MODULE_INFO_REQUEST): // Информация о модуле. Передается при старте модуля и как ответ на команду 0x00.
    SendModuleInfo();
    break;

  case (MODULE_SN_REQUEST): // Запрос серийного номера модуля.
    TXframePointer -> id.cmd_type = MODULE_SN_REQUEST;
    TXframePointer -> id.param    = 0;
    TXframePointer -> NumOfData   = 8;
//    Put64toFrameData              = *((uint64_t*)(ConstPointer -> SerialNumberLW));
    Put64toFrameData              = *((uint64_t*)SERIAL_NUMBER_ADDR_IN_FLASH);
    putIntoCanTxBuffer(TXframePointer);
    break;

  case (MODULE_INTERFACE_TYPES_REQUEST): // Запрос типов интерфейсов модуля.
    TXframePointer -> id.cmd_type = MODULE_INTERFACE_TYPES_REQUEST;
    TXframePointer -> id.param    = 0;
    TXframePointer -> NumOfData   = 8;
    memcpy(TXframePointer -> Data, module_interface_types, 8); 
    putIntoCanTxBuffer(TXframePointer);
    break;

  case (MCU_DATA_REQUEST): // Запрос данных микроконтроллера.
    TXframePointer -> id.cmd_type = MCU_DATA_REQUEST;
    TXframePointer -> id.param    = 0;
    TXframePointer -> NumOfData   = 2;
    Put16toFrameData = Read_MCU_FMD(); // Чтение параметра Flash memory density.
    putIntoCanTxBuffer(TXframePointer);
    break;

  case (UID_REQUEST): // Запрос UID.
  //------------------------------------------------------------------------------------------------------------------------
    TXframePointer -> id.cmd_type = UID_REQUEST;
    if (!IDarray[0]) // Если в младшем байте массива 0, то прочитать значение UID. Сделано так чтобы не читать каждый раз.
      {
      IDsize = Read_MCU_UID(IDarray); // Чтение UID в микроконтроллере (STM, GD, AT).
      }
    switch (canRxMsg->cmd13param & MASK) // Определение параметра команды.
      {
      case (SECTION_LOW): // Запрос на младшую часть UID.
        //-Вычисление количества байт данных для отправки-//
        if (IDsize < 9)
          TXframePointer -> NumOfData = IDsize;
        else
          TXframePointer -> NumOfData = 8;
        //------------------------------------------------//
        TXframePointer -> id.param = SECTION_LOW;
        Put64toFrameData           = *((uint64_t*)IDarray); // Копируем первые 8 байт (с 0 по 7) из массива IDarray.
        break;

      case (SECTION_MID): // Запрос на среднюю часть UID.
        //-Вычисление количества байт данных для отправки-//
        if (IDsize < 9)
          TXframePointer -> NumOfData = 0;
        else if (IDsize < 17)
          TXframePointer -> NumOfData = IDsize - 8;
        else// if (IDsize < 25)
          TXframePointer -> NumOfData = 8;
        //------------------------------------------------//
        TXframePointer -> id.param = SECTION_MID;
        Put64toFrameData           = *((uint64_t*)(IDarray + 8)); // Копируем с 8 по 15 байты из массива IDarray.
        break;

      case (SECTION_HIGH): // Запрос на старшую часть UID.

        //-Вычисление количества байт данных для отправки-//
        if (IDsize < 17)
          TXframePointer -> NumOfData = 0;
        else// if (IDsize < 25)
          TXframePointer -> NumOfData = IDsize - 16;
        //------------------------------------------------//
        TXframePointer -> id.param = SECTION_HIGH;
        Put64toFrameData           = *((uint64_t*)(IDarray + 16)); // Копируем с 16 по 23 байты из массива IDarray.
        break;
      default:
        break;
      }
    putIntoCanTxBuffer(TXframePointer);
    break;
  //------------------------------------------------------------------------------------------------------------------------

  case (MODULE_STATE_REQUEST): // Запрос состояния модуля.
    TXframePointer -> id.cmd_type = MODULE_STATE_REQUEST;
    TXframePointer -> id.param    = ReadReasonForReboot();
    TXframePointer -> NumOfData   = 0;
    putIntoCanTxBuffer(TXframePointer);
    break;

  case (CAN_MODE): // Изменение или определение режима работы CAN.
  //------------------------------------------------------------------------------------------------------------------------
    TXframePointer -> id.cmd_type = CAN_MODE;
    TXframePointer -> NumOfData   = 0;
    switch (canRxMsg->cmd13param & MASK)
      {
      case (NORMAL_MODE):
        SetReadCanMode(NORMAL_MODE); // Установка для CAN режим "Normal communication mode".
        TXframePointer -> id.param = SetReadCanMode(READ_MODE);
        putIntoCanTxBuffer(TXframePointer);
        break; // В CAN.c должны быть функции для определения режима работы CAN и установки нужного режима.
      case (SILENT_MODE):
        TXframePointer -> id.param = SILENT_MODE;
        putIntoCanTxBuffer(TXframePointer);
//        SetReadCanMode(SILENT_MODE); // Установка для CAN режим "Silent communication mode".
        break;
      case (READ_MODE):
        if (SetReadCanMode(READ_MODE) == NORMAL_MODE)
          {
          TXframePointer -> id.param = NORMAL_MODE;
          putIntoCanTxBuffer(TXframePointer);
          break;
          }
        else // Если для CAN установлен режим "Отправка в CAN отключена".
          {
          SetReadCanMode(NORMAL_MODE); // Установка для CAN режим "Normal communication mode".
          TXframePointer -> id.param = SILENT_MODE;
          putIntoCanTxBuffer(TXframePointer);
          SetReadCanMode(SILENT_MODE); // Установка для CAN режим "Silent communication mode".
          break;
          }
      default:
        break;
      }
  //------------------------------------------------------------------------------------------------------------------------
      
  case (MODULE_ADDRESS_CHANGE): // Смена адреса модуля.
    if (ProtectCMDstate == WAS)
      {
      TXframePointer -> id.cmd_type = MODULE_ADDRESS_CHANGE;
      TXframePointer -> id.param    = (canRxMsg->cmd13param & MASK); // Определение параметра (нового адреса) команды.
      TXframePointer -> NumOfData   = 1;
//      TXframePointer -> Data[0]     = ( *((uint8_t*)MODULE_ADDR_IN_FLASH) & MODULE_ADDR_MASK); // Собственный старый адрес модуля (микроконтроллера STM, GD, AT ...).
      TXframePointer -> Data[0]     = ConfigPointer -> AddrModule;
      putIntoCanTxBuffer(TXframePointer);
        
      ConfigPointer -> AddrModule = TXframePointer -> id.param;
      Write_Config_to_flash(ConfigPointer);
        
      ProtectCMDstate = WASNOT;
      SystemReset();
      }
    break;

  case (CAN_MODULE_SPEED_CHANGE): // Смена скорости работы CAN модуля.
    if (ProtectCMDstate == WAS)
      {
      TXframePointer -> id.cmd_type = CAN_MODULE_SPEED_CHANGE;
      TXframePointer -> id.param    = (canRxMsg->cmd13param & MASK); // Определение параметра (нового значения скорости) команды.
      TXframePointer -> NumOfData   = 1;
//      TXframePointer -> Data[0]     = *((can_speed*)CAN_SPEED_ADDR_IN_FLASH);
      TXframePointer -> Data[0]     = ConfigPointer -> CanSpeed;
      putIntoCanTxBuffer(TXframePointer);

      ConfigPointer -> CanSpeed = TXframePointer -> id.param;
      Write_Config_to_flash(ConfigPointer);

      ProtectCMDstate = WASNOT;
      SystemReset();
      }
    break;

  case (EXECUTING_PROTECTED_CMD): // Выполнение защищенной команды.
    if( (canRxMsg->cmd13param & MASK) == PROTECT_CMD_PARAM)
      {
      ProtectCMDstate = WAS;
      return;
      }
    break;

  case (MODULE_RELOAD): // Перезагрузка модуля.
    SystemReset();
    break;

  case (SUPPORTED_COM_CMD_REQUEST): // Запрос поддерживаемых общих команд.
    TXframePointer -> id.cmd_type = SUPPORTED_COM_CMD_REQUEST;
    TXframePointer -> id.param    = 0;
    TXframePointer -> NumOfData   = 8;
    Put64toFrameData              = SUPPORTED_COM_CMD_MASK;
    putIntoCanTxBuffer(TXframePointer);
    break;

  default:
    break;
  }
CMD_type        = 0xEE;
ProtectCMDstate = WASNOT;
}
//------------------------------------------------------------------------------//




//---Private functions----------------------------------------------------------//
/**
  * @brief   Инициализация структур.
  * @details Функция производит чтение установленные области FLASH памяти и
  *          заполняет соответствующими данными структуры типа RO_Constants_struct и Config_struct.
  *          Кроме того, функция заполняет структуру типа CAN_tx_frame_struct данными по умолчанию.
  *          Структура CAN_tx_frame_struct - структура кадра для отправки.
  * @param   Config_      - указатель типа Config_struct*       на структуру с данными Config.
  * @param   RO_Constants - указатель типа RO_Constants_struct* на структуру с данными RO Constants.
  * @param   tx_frame     - указатель типа CAN_tx_frame_struct* на структуру кадра для отправки.
  * @return  None.
  */
void InitStructs (Config_struct* ConfigPointer, RO_Constants_struct*  ConstPointer, CAN_tx_frame_struct* TXframePointer)
{
Read_Config_from_flash (ConfigPointer);
//Read_RO_Constants_from_flash (ConstPointer);

TXframePointer -> id.CM             = 1;                           // Обмен между контроллером и модулем.
TXframePointer -> id.CtoM           = 0;                           // Направление передачи: кадр будет передаваться из модуля в контроллер.
TXframePointer -> id.module_address = ConfigPointer -> AddrModule; // Собственный адрес модуля (STM, GD, AT).
TXframePointer -> id.PART           = 0;                           // Часть посылки.
TXframePointer -> id.interface_type = 0;                           // Тип интерфейса.
TXframePointer -> id.CMD            = 1;                           // Тип кадра, CMD=1 - команда, CMD=0 - данные.
TXframePointer -> id.cmd_type       = 0;                           // Тип команды.
TXframePointer -> id.param          = 0;                           // Параметры.
TXframePointer -> NumOfData         = 6;                           // Количество данных для передачи.
}
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------//









//***************************************END OF FILE**************************************//
