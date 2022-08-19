/**
  ******************************************************************************
  *
  * @file      ParsingCommonCMD.c
  *
  * @brief     ��������� ������ ������ (common) ����.
  *
  * @details   
  *            
  * @warning   **�����!** \n 
  * ��� ������ ���������� ������������ ������������ ����� ������������� � ����� Includes. \n 
  * ����� ���� ���������:
  *                       + ���� .� - ����;
  *                       + ���� .h - header for .�.
  *
  * **Manual**
  * ������ commonCMD.c ������������:
  * - parsing (������) ���������, ������������ �� ����������� (RAPIDA) � ������ (��������������� STM, GD, AT),
  *   � ������, ���� �� �������� ��������� � EXID (Extended format frame identifier), ���� "��� ����������" ����� �������� 0. \n 
  *   ���� ���� "��� ����������" ����� �������� 0, �� ��������� ������� ���� "�����" (common).
  *
  * - ����� ��������� (�� ����������� �������) ������� �/��� ������������ ��������� ��������� �� ������ � ����������.
  *
  * - ����� ��� ������������� ������� �������� ��������������� ������ � ���������� ����� CAN.
  *
  * 
  * @copyright Copyright (C) 2022 Awada Systems. ��� ����� ��������.
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
#define MASK                           0x3F  /*!< ����� ��� ����� "��� �������" ��� "���������". */

//-������� ��� ����������� ���� �������-//
#define MODULE_INFO_REQUEST            0x00  /*!< ������ ���������� � ������.         */
#define MODULE_SN_REQUEST              0x01  /*!< ������ ��������� ������ ������.     */
#define MODULE_INTERFACE_TYPES_REQUEST 0x02  /*!< ������ ����� ����������� ������.    */
#define MCU_DATA_REQUEST               0x03  /*!< ������ ������ ����������������.     */
#define UID_REQUEST                    0x04  /*!< ������ UID.                         */
#define MODULE_STATE_REQUEST           0x05  /*!< ������ ��������� ������.            */
#define CAN_MODE                       0x06  /*!< ���������/������ ������ ������ CAN. */

#define MODULE_ADDRESS_CHANGE          0x30  /*!< ����� ������ ������.                */
#define CAN_MODULE_SPEED_CHANGE        0x31  /*!< ����� �������� CAN ������.          */
#define EXECUTING_PROTECTED_CMD        0x3D  /*!< ���������� ���������� �������.      */
#define MODULE_RELOAD                  0x3E  /*!< ������������ ������.                */
#define SUPPORTED_COM_CMD_REQUEST      0x3F  /*!< ������ �������������� ����� ������. */

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
                                 (ONE << SUPPORTED_COM_CMD_REQUEST) )/*!< ����� �� ������ �������������� ����� ������. */
//--------------------------------------//

//-������� ��� ����������� ���� ���������� ��� ������� CAN_MODE-//
#define NORMAL_MODE (uint8_t)0x0 /*!< ������������ ������ ������ CAN �� "Normal communication mode". */
#define SILENT_MODE (uint8_t)0x1 /*!< ������������ ������ ������ CAN �� "Silent communication mode". */
#define READ_MODE   (uint8_t)0x2 /*!< �����������  ������ ������ CAN.                                */
//--------------------------------------------------------------//

//-������� ��� ����������� ���������� ��� ������� UID_REQUEST-//
#define SECTION_LOW  (uint8_t)0x0 /*!< ������� �����. */
#define SECTION_MID  (uint8_t)0x1 /*!< ������� �����. */
#define SECTION_HIGH (uint8_t)0x2 /*!< ������� �����. */
//------------------------------------------------------------//

#define Put16toFrameData (*((uint16_t*)(TXframePointer -> Data))) /*!< ����������� ������ ������� uint16_t � TXframePointer -> Data. */
#define Put32toFrameData (*((uint32_t*)(TXframePointer -> Data))) /*!< ����������� ������ ������� uint32_t � TXframePointer -> Data. */
#define Put64toFrameData (*((uint64_t*)(TXframePointer -> Data))) /*!< ����������� ������ ������� uint64_t � TXframePointer -> Data. */

#define WAS    1
#define WASNOT 0

#define PROTECT_CMD_PARAM (uint8_t)0x15 /*!< �������� ��������� ��� ������� "���������� ���������� �������". */ 
//------------------------------------------------------------------------------//

//---Exported variables---------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Private types--------------------------------------------------------------//
//uint8_t ComCMDtype [12] = {};
//------------------------------------------------------------------------------//

//---Private variables----------------------------------------------------------//
//-���� ��� �������� �� ������ (STM, GD, AT) � ���������� (RAPIDA controller)-
static CAN_tx_frame_struct  CAN_tx_frame; // ��������� ����� ��� ��������.
static Config_struct        Config;       // ��������� ��� �������� ������ Config Page.
static RO_Constants_struct  RO_Constants; // ��������� ��� �������� ������ RO Constants.

static CAN_tx_frame_struct* TXframePointer = &CAN_tx_frame;
static Config_struct*       ConfigPointer  = &Config;
static RO_Constants_struct* ConstPointer   = &RO_Constants;

static uint8_t module_interface_types [8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}; // ����� �������� ��� ������ �� ����������� � ������ ������� MODULE_INTERFACE_TYPES_REQUEST.
//------------------------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Private constants----------------------------------------------------------//
//------------------------------------------------------------------------------//

//---Function prototypes--------------------------------------------------------//
void InitStructs (Config_struct* ConfigPointer, RO_Constants_struct*  ConstPointer, CAN_tx_frame_struct* TXframePointer);
//------------------------------------------------------------------------------//

//---Exported functions---------------------------------------------------------//
/**
  * @brief   �������� ���������� � ������ (���������������� STM, GD, AT).
  * @details 
  * @return  None.
  */
void SendModuleInfo (void)
{
InitStructs (ConfigPointer, ConstPointer, TXframePointer);
//TXframePointer->Data[0] = ConstPointer  -> HardwareRevision.major;
//TXframePointer->Data[1] = ConstPointer  -> HardwareRevision.minor;
//Put16toFrameData = *(uint16_t*)(HARDWARE_REVISION_ADDR_IN_FLASH); // �������� �������� ���������� ������� �� Flash.
TXframePointer->Data[0] = *(uint8_t*)(HARDWARE_REVISION_ADDR_IN_FLASH + 1); // �������� �������� ���������� ������� �� Flash.
TXframePointer->Data[1] = *(uint8_t*)(HARDWARE_REVISION_ADDR_IN_FLASH);     // �������� �������� ���������� ������� �� Flash.
TXframePointer->Data[2] = ConfigPointer -> BootloaderVersion.major;
TXframePointer->Data[3] = ConfigPointer -> BootloaderVersion.minor;
TXframePointer->Data[4] = ConfigPointer -> ProgramVersion.major;
TXframePointer->Data[5] = ConfigPointer -> ProgramVersion.minor;
putIntoCanTxBuffer(&CAN_tx_frame);
}
//------------------------------------------------------------------------------//


/**
  * @brief   ������ ��������� ���������.
  * @details ������ ���������, ������������ �� ����������� (RAPIDA) � ������ (��������������� STM, GD, AT),
  *          � ������, ���� �� �������� ��������� � EXID (Extended format frame identifier), ���� "��� ����������" ����� �������� 0.
  * @param   canRxMsg - ��������� ���� canRxMsgBuf_struct* �� ��������� ������, ����������� �� �������� ������.
  * @return  None.
  */
void ParsingComCmd (canRxMsgBuf_struct* canRxMsg)
{
uint8_t        CMD_type = ((canRxMsg->cmd13param >> 6) & MASK);
static uint8_t IDarray [24] = {0}; // 24 - ������ UID � ������ �������� ���������.
static uint8_t IDsize;
static uint8_t ProtectCMDstate;  // ���� ����������� ������� EXECUTING_PROTECTED_CMD: ProtectCMDstate = WAS    - ������� ���������;
                                 //                                                   ProtectCMDstate = WASNOT - ������� �� ���������;
//CMD_type = MCU_DATA_REQUEST; // ������� ��� �������

switch (CMD_type)
  {
  case (MODULE_INFO_REQUEST): // ���������� � ������. ���������� ��� ������ ������ � ��� ����� �� ������� 0x00.
    SendModuleInfo();
    break;

  case (MODULE_SN_REQUEST): // ������ ��������� ������ ������.
    TXframePointer -> id.cmd_type = MODULE_SN_REQUEST;
    TXframePointer -> id.param    = 0;
    TXframePointer -> NumOfData   = 8;
//    Put64toFrameData              = *((uint64_t*)(ConstPointer -> SerialNumberLW));
    Put64toFrameData              = *((uint64_t*)SERIAL_NUMBER_ADDR_IN_FLASH);
    putIntoCanTxBuffer(TXframePointer);
    break;

  case (MODULE_INTERFACE_TYPES_REQUEST): // ������ ����� ����������� ������.
    TXframePointer -> id.cmd_type = MODULE_INTERFACE_TYPES_REQUEST;
    TXframePointer -> id.param    = 0;
    TXframePointer -> NumOfData   = 8;
    memcpy(TXframePointer -> Data, module_interface_types, 8); 
    putIntoCanTxBuffer(TXframePointer);
    break;

  case (MCU_DATA_REQUEST): // ������ ������ ����������������.
    TXframePointer -> id.cmd_type = MCU_DATA_REQUEST;
    TXframePointer -> id.param    = 0;
    TXframePointer -> NumOfData   = 2;
    Put16toFrameData = Read_MCU_FMD(); // ������ ��������� Flash memory density.
    putIntoCanTxBuffer(TXframePointer);
    break;

  case (UID_REQUEST): // ������ UID.
  //------------------------------------------------------------------------------------------------------------------------
    TXframePointer -> id.cmd_type = UID_REQUEST;
    if (!IDarray[0]) // ���� � ������� ����� ������� 0, �� ��������� �������� UID. ������� ��� ����� �� ������ ������ ���.
      {
      IDsize = Read_MCU_UID(IDarray); // ������ UID � ���������������� (STM, GD, AT).
      }
    switch (canRxMsg->cmd13param & MASK) // ����������� ��������� �������.
      {
      case (SECTION_LOW): // ������ �� ������� ����� UID.
        //-���������� ���������� ���� ������ ��� ��������-//
        if (IDsize < 9)
          TXframePointer -> NumOfData = IDsize;
        else
          TXframePointer -> NumOfData = 8;
        //------------------------------------------------//
        TXframePointer -> id.param = SECTION_LOW;
        Put64toFrameData           = *((uint64_t*)IDarray); // �������� ������ 8 ���� (� 0 �� 7) �� ������� IDarray.
        break;

      case (SECTION_MID): // ������ �� ������� ����� UID.
        //-���������� ���������� ���� ������ ��� ��������-//
        if (IDsize < 9)
          TXframePointer -> NumOfData = 0;
        else if (IDsize < 17)
          TXframePointer -> NumOfData = IDsize - 8;
        else// if (IDsize < 25)
          TXframePointer -> NumOfData = 8;
        //------------------------------------------------//
        TXframePointer -> id.param = SECTION_MID;
        Put64toFrameData           = *((uint64_t*)(IDarray + 8)); // �������� � 8 �� 15 ����� �� ������� IDarray.
        break;

      case (SECTION_HIGH): // ������ �� ������� ����� UID.

        //-���������� ���������� ���� ������ ��� ��������-//
        if (IDsize < 17)
          TXframePointer -> NumOfData = 0;
        else// if (IDsize < 25)
          TXframePointer -> NumOfData = IDsize - 16;
        //------------------------------------------------//
        TXframePointer -> id.param = SECTION_HIGH;
        Put64toFrameData           = *((uint64_t*)(IDarray + 16)); // �������� � 16 �� 23 ����� �� ������� IDarray.
        break;
      default:
        break;
      }
    putIntoCanTxBuffer(TXframePointer);
    break;
  //------------------------------------------------------------------------------------------------------------------------

  case (MODULE_STATE_REQUEST): // ������ ��������� ������.
    TXframePointer -> id.cmd_type = MODULE_STATE_REQUEST;
    TXframePointer -> id.param    = ReadReasonForReboot();
    TXframePointer -> NumOfData   = 0;
    putIntoCanTxBuffer(TXframePointer);
    break;

  case (CAN_MODE): // ��������� ��� ����������� ������ ������ CAN.
  //------------------------------------------------------------------------------------------------------------------------
    TXframePointer -> id.cmd_type = CAN_MODE;
    TXframePointer -> NumOfData   = 0;
    switch (canRxMsg->cmd13param & MASK)
      {
      case (NORMAL_MODE):
        SetReadCanMode(NORMAL_MODE); // ��������� ��� CAN ����� "Normal communication mode".
        TXframePointer -> id.param = SetReadCanMode(READ_MODE);
        putIntoCanTxBuffer(TXframePointer);
        break; // � CAN.c ������ ���� ������� ��� ����������� ������ ������ CAN � ��������� ������� ������.
      case (SILENT_MODE):
        TXframePointer -> id.param = SILENT_MODE;
        putIntoCanTxBuffer(TXframePointer);
//        SetReadCanMode(SILENT_MODE); // ��������� ��� CAN ����� "Silent communication mode".
        break;
      case (READ_MODE):
        if (SetReadCanMode(READ_MODE) == NORMAL_MODE)
          {
          TXframePointer -> id.param = NORMAL_MODE;
          putIntoCanTxBuffer(TXframePointer);
          break;
          }
        else // ���� ��� CAN ���������� ����� "�������� � CAN ���������".
          {
          SetReadCanMode(NORMAL_MODE); // ��������� ��� CAN ����� "Normal communication mode".
          TXframePointer -> id.param = SILENT_MODE;
          putIntoCanTxBuffer(TXframePointer);
          SetReadCanMode(SILENT_MODE); // ��������� ��� CAN ����� "Silent communication mode".
          break;
          }
      default:
        break;
      }
  //------------------------------------------------------------------------------------------------------------------------
      
  case (MODULE_ADDRESS_CHANGE): // ����� ������ ������.
    if (ProtectCMDstate == WAS)
      {
      TXframePointer -> id.cmd_type = MODULE_ADDRESS_CHANGE;
      TXframePointer -> id.param    = (canRxMsg->cmd13param & MASK); // ����������� ��������� (������ ������) �������.
      TXframePointer -> NumOfData   = 1;
//      TXframePointer -> Data[0]     = ( *((uint8_t*)MODULE_ADDR_IN_FLASH) & MODULE_ADDR_MASK); // ����������� ������ ����� ������ (���������������� STM, GD, AT ...).
      TXframePointer -> Data[0]     = ConfigPointer -> AddrModule;
      putIntoCanTxBuffer(TXframePointer);
        
      ConfigPointer -> AddrModule = TXframePointer -> id.param;
      Write_Config_to_flash(ConfigPointer);
        
      ProtectCMDstate = WASNOT;
      SystemReset();
      }
    break;

  case (CAN_MODULE_SPEED_CHANGE): // ����� �������� ������ CAN ������.
    if (ProtectCMDstate == WAS)
      {
      TXframePointer -> id.cmd_type = CAN_MODULE_SPEED_CHANGE;
      TXframePointer -> id.param    = (canRxMsg->cmd13param & MASK); // ����������� ��������� (������ �������� ��������) �������.
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

  case (EXECUTING_PROTECTED_CMD): // ���������� ���������� �������.
    if( (canRxMsg->cmd13param & MASK) == PROTECT_CMD_PARAM)
      {
      ProtectCMDstate = WAS;
      return;
      }
    break;

  case (MODULE_RELOAD): // ������������ ������.
    SystemReset();
    break;

  case (SUPPORTED_COM_CMD_REQUEST): // ������ �������������� ����� ������.
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
  * @brief   ������������� ��������.
  * @details ������� ���������� ������ ������������� ������� FLASH ������ �
  *          ��������� ���������������� ������� ��������� ���� RO_Constants_struct � Config_struct.
  *          ����� ����, ������� ��������� ��������� ���� CAN_tx_frame_struct ������� �� ���������.
  *          ��������� CAN_tx_frame_struct - ��������� ����� ��� ��������.
  * @param   Config_      - ��������� ���� Config_struct*       �� ��������� � ������� Config.
  * @param   RO_Constants - ��������� ���� RO_Constants_struct* �� ��������� � ������� RO Constants.
  * @param   tx_frame     - ��������� ���� CAN_tx_frame_struct* �� ��������� ����� ��� ��������.
  * @return  None.
  */
void InitStructs (Config_struct* ConfigPointer, RO_Constants_struct*  ConstPointer, CAN_tx_frame_struct* TXframePointer)
{
Read_Config_from_flash (ConfigPointer);
//Read_RO_Constants_from_flash (ConstPointer);

TXframePointer -> id.CM             = 1;                           // ����� ����� ������������ � �������.
TXframePointer -> id.CtoM           = 0;                           // ����������� ��������: ���� ����� ������������ �� ������ � ����������.
TXframePointer -> id.module_address = ConfigPointer -> AddrModule; // ����������� ����� ������ (STM, GD, AT).
TXframePointer -> id.PART           = 0;                           // ����� �������.
TXframePointer -> id.interface_type = 0;                           // ��� ����������.
TXframePointer -> id.CMD            = 1;                           // ��� �����, CMD=1 - �������, CMD=0 - ������.
TXframePointer -> id.cmd_type       = 0;                           // ��� �������.
TXframePointer -> id.param          = 0;                           // ���������.
TXframePointer -> NumOfData         = 6;                           // ���������� ������ ��� ��������.
}
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------//









//***************************************END OF FILE**************************************//
