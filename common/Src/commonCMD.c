/**
  ******************************************************************************
  *
  * @file      commonCMD.c
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
#include "commonCMD.h"
#include "MCU.h"
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
#define SILENCE_MODE                   0x06  /*!< ���������/������ ������ ������ CAN. */

#define MODULE_ADDRESS_CHANGE          0x30  /*!< ����� ������ ������.                */
#define CAN_MODULE_SPEED_CHANGE        0x31  /*!< ����� �������� CAN ������.          */
#define EXECUTING_PROTECTED_CMD        0x3D  /*!< ���������� ���������� �������.      */
#define MODULE_OVERLOAD                0x3E  /*!< ������������ ������.                */
#define SUPPORTED_COM_CMD_REQUEST      0x3F  /*!< ������ �������������� ����� ������. */
//--------------------------------------//

//-������� ��� ����������� ���� ���������� ��� ������� SILENCE_MODE-//
#define NORMAL_MODE 0x0
#define CAN_TX_OFF  0x1
#define READ_MODE   0x2
//------------------------------------------------------------------//

#define Put16toFrameData (*((uint16_t*)(TXframePointer -> Data))) // ����������� ������ ������� uint16_t � TXframePointer -> Data.
#define Put32toFrameData (*((uint32_t*)(TXframePointer -> Data))) // ����������� ������ ������� uint32_t � TXframePointer -> Data.
#define Put64toFrameData (*((uint64_t*)(TXframePointer -> Data))) // ����������� ������ ������� uint64_t � TXframePointer -> Data.
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
TXframePointer -> id.param = 0;
InitStructs (ConfigPointer, ConstPointer, TXframePointer);
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
uint8_t CMD_type = ((canRxMsg->cmd13param >> 6) & MASK);
uint32_t IDarray [3];


CMD_type = MCU_DATA_REQUEST; // ������� ��� �������

  
switch (CMD_type)
{
case (MODULE_INFO_REQUEST): // ���������� � ������. ���������� ��� ������ ������ � ��� ����� �� ������� 0x00.
  SendModuleInfo();
  break;

case (MODULE_SN_REQUEST): // ������ ��������� ������ ������.
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

case (MCU_DATA_REQUEST): // ������ ������ ����������������.
  TXframePointer -> id.cmd_type = MCU_DATA_REQUEST;
  TXframePointer -> id.param    = 0;
  Put16toFrameData = Read_MCU_FMD();
  putIntoCanTxBuffer(&CAN_tx_frame);
  break;

case (UID_REQUEST):      // ������ UID.
  Read_MCU_UID(IDarray); // ������ UID � ���������������� (STM, GD, AT).
  TXframePointer -> id.cmd_type = UID_REQUEST;

  for (uint8_t i=0; i<3; i++)
    {
    TXframePointer -> id.param = i;
    Put32toFrameData = IDarray[i];
    putIntoCanTxBuffer(TXframePointer);
    }
  break;

case (MODULE_STATE_REQUEST): // ������ ��������� ������.
  TXframePointer -> id.cmd_type = MODULE_STATE_REQUEST;
  TXframePointer -> id.param    = (ReadReasonForReboot() & 0x3F);
  break;

case (SILENCE_MODE): // ��������� ������ ������ CAN.
  TXframePointer -> id.cmd_type = SILENCE_MODE;
  switch (canRxMsg->cmd13param & MASK)
    {
  	case (NORMAL_MODE):
      SetCanMode(NORMAL_MODE);
      TXframePointer -> id.param = ReadCanMode();
      putIntoCanTxBuffer(&CAN_tx_frame);
  		break; // � CAN.c ������ ���� ������� ��� ����������� ������ ������ CAN � ��������� ������� ������.
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
Read_RO_Constants_from_flash (ConstPointer);

TXframePointer -> id.CM             = 1;                           // ����� ����� ������������ � �������.
TXframePointer -> id.CtoM           = 0;                           // ����������� ��������: ���� ����� ������������ �� ������ � ����������.
TXframePointer -> id.module_address = ConfigPointer -> AddrModule; // ����������� ����� ������ (STM, GD, AT).
TXframePointer -> id.PART           = 0;                           // ����� �������.
TXframePointer -> id.interface_type = 0;                           // ��� ����������.
TXframePointer -> id.CMD            = 1;                           // ��� �����, CMD=1 - �������, CMD=0 - ������.
TXframePointer -> id.cmd_type       = 0;                           // ��� �������.
TXframePointer -> id.param          = 0;                           // ���������.
TXframePointer -> NumOfData         = 8;                           // ���������� ������ ��� ��������.
}
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------//









//***************************************END OF FILE**************************************//
