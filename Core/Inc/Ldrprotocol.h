/*
 * Ldrprotocol.h
 *
 *  Created on: 09-Dec-2020
 *      Author: suresh
 */

#ifndef INC_LDRPROTOCOL_H_
#define INC_LDRPROTOCOL_H_
/*---------------------------------------INCLUDES ----------------------------------------------------*/
#include "stdlib.h"
#include <string.h>

/*----------------------------------------TYPEDEFS ---------------------------------------------------*/

/*--------------------------------------- DEFINES ----------------------------------------------------*/
#define PACKET_LENGTH		512

//Init Seq Related Define
#define INIT_PACKET_REQ_COUNT 50
#define INIT_PACKET_REQ_DELAY 2000

#define DATA_WAIT_TIME 	  	  4000

//For Swap 16 Bit data
#define ENABLE_CONV_16

/*------------------------------------------ MACROS ----------------------------------------------------*/


/*------------------------------------------- ENUMS ----------------------------------------------------*/
typedef enum tagdownloderEnum
{
  		/* 0x43 */ 		INIT_TX   		= 0x43  //Caps 'C'
  ,		/* 0x55 */ 		DATA_ACK  		= 0x55  
  ,		/* 0xEE */ 		DATA_NACK  		= 0xEE  
  ,		/* 0xBBDD */ 	IMAGE_HEADER  	= 0xBBDD  
  ,		/* 0xBEAF */ 	IMAGE_FOOTER  	= 0xBEAF  
  , 	/* 0xA5A5 */	PACKET_HEADER	= 0xA5A5
  ,		/* 0xDEAD */ 	PACKET_FOOTER  	= 0xDEAD

}DFU_ENUM;


typedef enum tadErrorCode
{
			/* 0x00 */  BL_OK       = 0x00
	,		/* 0x01 */ 	BL_ERROR    = 0x01
	,		/* 0x02 */ 	BL_BUSY		= 0x02
	,		/* 0x03 */ 	BL_TIMEOUT	= 0x03
} ERROR_CODE;

typedef enum tadImageProcess
{
			/* 0x00 */  INVALID_DATA        = 0x00
	,		/* 0x01 */ 	IMG_HEADER    		= 0x01
	,		/* 0x02 */ 	PAYLAOD_DATA		= 0x02
} IMG_PROCESS_ENUM;


/*------------------------------------------- STRUCTS --------------------------------------------------*/		
typedef struct tagBootImageHeader
{

 /* WORD -1- START */
  UINT32 ui32DownLoadMarker;
  /* WORD -1- END */

  /* WORD -2- START */
  UINT16 ui16ImgHdr; //0xBBDD
  UINT16 ui16ImgSize; 
  /* WORD -2- END */

  /* WORD -3- START */
  UINT16 ui16ImgCrc;
  UINT16 ui16TotalNoPkts;
  /* WORD -3- END */

  /* WORD -4- START */
  UINT16 ui16ImgFooter; //0xBEAF
 /* WORD -4- END */
 
} BOOT_IMG_HEADER_STRUCT;


typedef struct tagImageHeader
{

  /* WORD -1- START */
  UINT16 ui16ImgHdr; //0xBBDD
  UINT16 ui16ImgSize; 
  /* WORD -1- END */

  /* WORD -2- START */
  UINT16 ui16ImgCrc;
  UINT16 ui16TotalNoPkts;
  /* WORD -2- END */

  /* WORD -3- START */
  UINT16 ui16ImgFooter; //0xBEAF
 /* WORD -3- END */
 
} IMG_HEADER_STRUCT;

// Fixed packet Size 512Bytes
typedef struct tagImagePayLoad
{
	/* WORD -1- START */
	UINT16 ui16PL_ImgHdr; //0xA5A5
	UINT16 ui16PL_CRC;
	/* WORD -1- END */
	
	/* WORD -2- START */
	UINT16 ui16PL_PacketNo;
	UINT8 acui8PL_Data[PACKET_LENGTH];
	UINT16 ui16PL_Footer; //0xDEAD
	/* WORD -3- END */
	
}IMG_PAYLOAD_STRUCT;




/*------------------------------------ FUNCTION PROTO TYPE'S --------------------------------------------*/

void Initialize_Parameter ( void );


void UART_CallBack_Register( void );
void Print_PayLoad_Header ( void );
void Print_Img_Header ( void );

void FirmwareDowloadProcess( void );

ERROR_CODE SendResponseToHost( UINT8 ui8SendCmdToHost);

UINT16 ConvertBig162Little16( UINT16 ui16InData );

UINT8 Verify_App_Crc( void );

void RS485TxAPI_GPIO(GPIO_PinState EnRS485 );

void Print_BootImgIndex_Header ( void );

void ProgramBootImageHeaderIntoFlash (UINT32 ui32FlashOffet);

void ReadImageIndexTableFromFlash( UINT32 ui32FlashOffet );

void BL_PROCESS( void );


#endif /* INC_LDRPROTOCOL_H_ */
