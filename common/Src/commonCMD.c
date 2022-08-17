/**
  ******************************************************************************
  *
  * @file      commonCMD.c
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
#include "commonCMD.h"
#include "MCU.h"
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
#define SILENCE_MODE                   0x06  /*!< Изменение/Запрос режима работы CAN. */

#define MODULE_ADDRESS_CHANGE          0x30  /*!< Смена адреса модуля.                */
#define CAN_MODULE_SPEED_CHANGE        0x31  /*!< Смена скорости CAN модуля.          */
#define EXECUTING_PROTECTED_CMD        0x3D  /*!< Выполнение защищенной команды.      */
#define MODULE_OVERLOAD                0x3E  /*!< Перезагрузка модуля.                */
#define SUPPORTED_COM_CMD_REQUEST      0x3F  /*!< Запрос поддерживаемых общих команд. */
//--------------------------------------//

//-Макросы для определения типа параметров при команде SILENCE_MODE-//
#define NORMAL_MODE 0x0
#define CAN_TX_OFF  0x1
#define READ_MODE   0x2
//------------------------------------------------------------------//

#define Put16toFrameData (*((uint16_t*)(TXframePointer -> Data))) // Скопировать данные формата uint16_t в TXframePointer -> Data.
#define Put32toFrameData (*((uint32_t*)(TXframePointer -> Data))) // Скопировать данные формата uint32_t в TXframePointer -> Data.
#define Put64toFrameData (*((uint64_t*)(TXframePointer -> Data))) // Скопировать данные формата uint64_t в TXframePointer -> Data.
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
TXframePointer -> id.param = 0;
InitStructs (ConfigPointer, ConstPointer, TXframePointer);
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
uint8_t CMD_type = ((canRxMsg->cmd13param >> 6) & MASK);
uint32_t IDarray [3];


CMD_type = MCU_DATA_REQUEST; // СТРОЧКА ДЛЯ ОТЛАДКИ

  
switch (CMD_type)
{
case (MODULE_INFO_REQUEST): // Информация о модуле. Передается при старте модуля и как ответ на команду 0x00.
  SendModuleInfo();
  break;

case (MODULE_SN_REQUEST): // Запрос серийного номера модуля.
  TXframePointer -> id.cmd_type        = MODULE_SN_REQUEST;
  TXframePointer -> id.param           = 0;
  Put64toFrameData = *((uint64_t*)(ConstPointer -> SerialNumberLW));
  putIntoCanTxBuffer(&CAN_tx_frame);
  break;

case (MODULE_INTERFACE_TYPES_REQUEST):
  TXframePointer -> id.cmd_type = MODULE_INTERFACE_TYPES_REQUEST;
  TXframePointer -> id.param    = 0;
  putIntoCanTxBuffer(&CAN_tx_frame);
  break;

case (MCU_DATA_REQUEST): // Запрос данных микроконтроллера.
  TXframePointer -> id.cmd_type = MCU_DATA_REQUEST;
  TXframePointer -> id.param    = 0;
  Put16toFrameData = Read_MCU_FMD();
  putIntoCanTxBuffer(&CAN_tx_frame);
  break;

case (UID_REQUEST):      // Запрос UID.
  Read_MCU_UID(IDarray); // Чтение UID в микроконтроллере (STM, GD, AT).
  TXframePointer -> id.cmd_type = UID_REQUEST;

  for (uint8_t i=0; i<3; i++)
    {
    TXframePointer -> id.param = i;
    Put32toFrameData = IDarray[i];
    putIntoCanTxBuffer(TXframePointer);
    }
  break;

case (MODULE_STATE_REQUEST): // Запрос состояния модуля.
  TXframePointer -> id.cmd_type = MODULE_STATE_REQUEST;
  TXframePointer -> id.param    = (ReadReasonForReboot() & 0x3F);
  break;

case (SILENCE_MODE): // Изменение режима работы CAN.
  TXframePointer -> id.cmd_type = SILENCE_MODE;
  switch (canRxMsg->cmd13param & MASK)
    {
  	case (NORMAL_MODE):
      SetCanMode(NORMAL_MODE);
      TXframePointer -> id.param = ReadCanMode();
      putIntoCanTxBuffer(&CAN_tx_frame);
  		break; // В CAN.c должны быть функции для определения режима работы CAN и установки нужного режима.
  	case (CAN_TX_OFF):
      SetCanMode(CAN_TX_OFF);
  		break;
  	default:
  		break;
    }





  break;









default:
  break;
}
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
Read_RO_Constants_from_flash (ConstPointer);

TXframePointer -> id.CM             = 1;                           // Обмен между контроллером и модулем.
TXframePointer -> id.CtoM           = 0;                           // Направление передачи: кадр будет передаваться из модуля в контроллер.
TXframePointer -> id.module_address = ConfigPointer -> AddrModule; // Собственный адрес модуля (STM, GD, AT).
TXframePointer -> id.PART           = 0;                           // Часть посылки.
TXframePointer -> id.interface_type = 0;                           // Тип интерфейса.
TXframePointer -> id.CMD            = 1;                           // Тип кадра, CMD=1 - команда, CMD=0 - данные.
TXframePointer -> id.cmd_type       = 0;                           // Тип команды.
TXframePointer -> id.param          = 0;                           // Параметры.
TXframePointer -> NumOfData         = 8;                           // Количество данных для передачи.
}
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------//









//***************************************END OF FILE**************************************//
