/*
 * Ldrprotocol.c
 *
 *  Created on: 09-Dec-2020
 *      Author: Suresh
 */
/*---------------------------------------INCLUDES ----------------------------------------------------*/
#include "Bootloader.h"
#include "Ldrprotocol.h"
#include "CRC.h"

/*-----------------------------------------TYPEDEFS ---------------------------------------------------*/
	 
/*---------------------------------------- DEFINES ----------------------------------------------------*/

/*---------------------------------------- MACROS -----------------------------------------------------*/

/*-------------------------------------- GLOBAL STRUCTS ------------------------------------------------*/

IMG_HEADER_STRUCT  		StDownlaodImageHeader = {0};
BOOT_IMG_HEADER_STRUCT	StBootImgIndex = {0};

IMG_PAYLOAD_STRUCT StPayload = {0};


/*-------------------------------------- Extern GLOBAL VAR'S(HAL) -------------------------------------*/
 
/*-------------------------------------- GLOBAL VAR'S -------------------------------------------------*/ 


static PUINT8 psui8FirmwareBuffer=NULL;
UINT8 ui8DBG[ 256 ] ={0};


volatile UINT8 ui8RXTmp = 0,ui8PrevRXTmp=0;
static UINT32 sui32FwRxBuffIdx = 0;
UINT8 ui8PacketType =0, ui8DataCpyFlag =0, ui8InitCmdRetry=0;




/*----------------------------------LOCAL FUNCTION PROTO TYPE'S ----------------------------------------*/

/*----------------------------------------- FUNCTION'S --------------------------------------------------*/




/* ROUGH NOTE

 Send 'C' to Host
     |
    Yes 							-->Set STATE Flag 0x01
     |
  Validate CRC -> No --> Not Change HEADER Wait Valid Header
  |  		
 Yes		
  |			
 ACK Tx     						--> invalidate STATE Flag 0xFF
  |
 Wait For PayLoad Packet   			--> PayLoad Rx Then Set STATE Flag 0x02
  |
 Yes
  |
 Validate CRC
  |
 Program Into Flash Then Move Next Flash Add
  |
  ACK Tx

*/



void FirmwareDowloadProcess( void )
{	

	INT8 i8TxDataCount = INIT_PACKET_REQ_COUNT;
	UINT16 ui16CalCRC = FALSE;
	static UINT16 ui16CurrentPaketCount =0;
	static UINT8  u8BreakPktPoll = TRUE;
	static UINT32  u32TstTimerCount = 0;
	UINT32 ui32Flash_Addr = DRV_HW_SR_FLASH_ADDRESS;

 	STM32_Serial_PutString("REQUESTING FIRMWARE DOWNLOAD!!!\r\n");
 
	ui8InitCmdRetry = FALSE; //Reset 

	//Send INIT cmd to HOST for transmit the Data
	while(i8TxDataCount > 0 )
	{
		SendResponseToHost(INIT_TX);
		
		HAL_Delay(INIT_PACKET_REQ_DELAY);

		if((UINT8)TRUE == ui8InitCmdRetry)
		{
 		    //STM32_Serial_PutString("Break Tx Data Req Loop\r\n");
			break;
		}
		i8TxDataCount--;
	}
	

	if((UINT8)TRUE == ui8InitCmdRetry)
	{
  
		while( (UINT8)TRUE == u8BreakPktPoll )
		{


			switch(ui8PacketType)
			{
				case IMG_HEADER:
				{
					//Copy Data From UART Rx Buffer
					{
						memcpy(&StDownlaodImageHeader,&psui8FirmwareBuffer[0],( sizeof(IMG_HEADER_STRUCT)));

						//Clear Buffer
						memset(psui8FirmwareBuffer,0x00,1024);
						
						//Enable RX For RS485
						RS485TxAPI_GPIO(GPIO_PIN_RESET);
					}

					//Reset Data Wait Timer Flag
					u32TstTimerCount =0;
					
#ifdef ENABLE_CONV_16
					//Convert Header Big 2 Small
					StDownlaodImageHeader.ui16ImgHdr 		= ConvertBig162Little16(StDownlaodImageHeader.ui16ImgHdr);
					StDownlaodImageHeader.ui16ImgSize		= ConvertBig162Little16(StDownlaodImageHeader.ui16ImgSize);
					StDownlaodImageHeader.ui16ImgCrc		= ConvertBig162Little16(StDownlaodImageHeader.ui16ImgCrc);
					StDownlaodImageHeader.ui16TotalNoPkts	= ConvertBig162Little16(StDownlaodImageHeader.ui16TotalNoPkts);
					StDownlaodImageHeader.ui16ImgFooter		= ConvertBig162Little16(StDownlaodImageHeader.ui16ImgFooter);
#endif	//ENABLE_CONV_16	


					if( ((UINT16)0 < (UINT16)StDownlaodImageHeader.ui16TotalNoPkts) && ( (UINT16)StDownlaodImageHeader.ui16ImgFooter == (UINT16) IMAGE_FOOTER))
					{
						//Reset Current Packet Count
						ui16CurrentPaketCount = 0;

						//Once Header Rx Then Reset App Program Address
						ui32Flash_Addr = DRV_HW_SR_FLASH_ADDRESS;

						//Erase Flash For Program APP
						FLASH_ERASE(ui32Flash_Addr,StDownlaodImageHeader.ui16ImgSize);

	
						//Program Image index in to Flash
						ProgramBootImageHeaderIntoFlash(BOOT_IMAGE_HEADER_OFFSET);
						
						//Send ACK To HOST
						SendResponseToHost(DATA_ACK);	

						STM32_Serial_PutString("\r\nTx ACK\r\n");
					}
					else
					{
						//Debug Print of HEADER
						Print_Img_Header();	
						
						//Send NACK
						SendResponseToHost(DATA_NACK);
						
						STM32_Serial_PutString("\r\nTx NACK\r\n");
					}

					//Reset the Flag
					ui8PacketType = INVALID_DATA;
				}break;
				
				case PAYLAOD_DATA:
				{
					
					//Copy Data From UART Rx Buffer
					{
						//Header & 
						memcpy(&StPayload.ui16PL_ImgHdr,&psui8FirmwareBuffer[0], sizeof(UINT16));
						memcpy(&StPayload.ui16PL_CRC,&psui8FirmwareBuffer[2], sizeof(UINT16));
						memcpy(&StPayload.ui16PL_PacketNo,&psui8FirmwareBuffer[4], sizeof(UINT16));

						//Copy PayLoad Data
						memcpy(&StPayload.acui8PL_Data,&psui8FirmwareBuffer[6], PACKET_LENGTH);


						memcpy(&StPayload.ui16PL_Footer,&psui8FirmwareBuffer[sui32FwRxBuffIdx-1], sizeof(UINT16));

						//Clear Buffer
						memset(psui8FirmwareBuffer,0x00,1024);

						//Enable RX For RS485
						RS485TxAPI_GPIO(GPIO_PIN_RESET);

					}

#ifdef ENABLE_CONV_16					
					//Convert Header Big 2 Small
					StPayload.ui16PL_ImgHdr 	= ConvertBig162Little16(StPayload.ui16PL_ImgHdr);
					StPayload.ui16PL_CRC		= ConvertBig162Little16(StPayload.ui16PL_CRC);
					StPayload.ui16PL_PacketNo	= ConvertBig162Little16(StPayload.ui16PL_PacketNo);
					StPayload.ui16PL_Footer 	= ConvertBig162Little16(StPayload.ui16PL_Footer);					
#endif	//ENABLE_CONV_16

					//For Debug purpose Need to remove
					sprintf(ui8DBG,"ACT-PKT:[%X] Exp-PKT:[%X]RX PKTNO:[%X]\r\n",ui16CurrentPaketCount,
																				StDownlaodImageHeader.ui16TotalNoPkts,
																				StPayload.ui16PL_PacketNo);
					STM32_Serial_PutString(ui8DBG);


					//Reset Data Wait Timer Flag
					u32TstTimerCount =0;
					
					if( (UINT16)ui16CurrentPaketCount < (UINT16)StDownlaodImageHeader.ui16TotalNoPkts ) 
					{
					
	

						//Validate CRC
						ui16CalCRC = crc_16( &StPayload.acui8PL_Data[0], PACKET_LENGTH);

						sprintf(ui8DBG,"\r\nEXP:[%X] ACT:[%X]\r\n",StPayload.ui16PL_CRC,ui16CalCRC);
						STM32_Serial_PutString(ui8DBG);

						if( ui16CalCRC == StPayload.ui16PL_CRC )
						{
							//Program Data into Flash
							UserProgramIntoFlash(ui32Flash_Addr,&StPayload.acui8PL_Data[0],PACKET_LENGTH);

							//Increase Flash Address
							ui32Flash_Addr = ui32Flash_Addr+PACKET_LENGTH;

							//In Success case increase the current packet count for NACK Case Don't increase the current packet count
							ui16CurrentPaketCount++;

							//Send ACK To HOST
							SendResponseToHost(DATA_ACK);

							STM32_Serial_PutString("\r\nData Tx ACK\r\n");
							
						}
						else
						{
							//Debug Print of HEADER
							Print_PayLoad_Header();
							
							//Send ACK To HOST
							SendResponseToHost(DATA_NACK);	

							STM32_Serial_PutString("\r\nData Tx NACK\r\n");

						}
						

					} 
					else if( (UINT16)ui16CurrentPaketCount == (UINT16)StDownlaodImageHeader.ui16TotalNoPkts )
					{
							STM32_Serial_PutString("\r\nAll Data packets Received\r\n");
							
							u8BreakPktPoll = FALSE;
					}  
					
					//Reset the Flag
					ui8PacketType = INVALID_DATA;

				}break;	
			}
#if 1
			if((UINT16)ui16CurrentPaketCount == (UINT16)StDownlaodImageHeader.ui16TotalNoPkts)
			{
					STM32_Serial_PutString("\r\n All DATA RxD \r\n");
	
					
					//For Debug purpose Need to remove
					sprintf(ui8DBG,"ACT-PKT:[%X] Exp-PKT:[%X]RX PKTNO:[%X]\r\n",ui16CurrentPaketCount,
																				StDownlaodImageHeader.ui16TotalNoPkts,
																				StPayload.ui16PL_PacketNo);
					STM32_Serial_PutString(ui8DBG);
					
					u8BreakPktPoll = FALSE;

					//Reset chip
					HAL_NVIC_SystemReset();
					
			}  
#endif

			if ( DATA_WAIT_TIME < u32TstTimerCount )
			{
				STM32_Serial_PutString("\r\n CONNECTION LOST-HOST NOT RESPOND\r\n");
				u8BreakPktPoll = FALSE;
			}
			
			HAL_Delay(5);
			//Timer Count Flag
			u32TstTimerCount++;

		}
	}
	else
	{
		STM32_Serial_PutString("No Response From Host\r\n");
	}
	
}

ERROR_CODE SendResponseToHost(UINT8 ui8SendCmdToHost)
{
	ERROR_CODE Status = ERROR;
	
	RS485TxAPI_GPIO(GPIO_PIN_SET);
	//Host missed the Data if immediatly send data to HOST. So Use 1Sec Delay to Avoid that issue
	HAL_Delay(1000);
	Status = HAL_UART_Transmit(&FIRMWARE_UART,(UINT8*)&ui8SendCmdToHost,1,1000);
	HAL_Delay(10);
	RS485TxAPI_GPIO(GPIO_PIN_RESET);

	return Status;
}



/*--------------------------------------------------- GPIO  ---------------------------------------------------*/

/*
 *RS485 Set 1 Tx Enabled,Set 0 Rx Enabled
 */
void RS485TxAPI_GPIO(GPIO_PinState EnRS485 )
{
	 HAL_GPIO_WritePin(GPIOB,UART2_RS485_EN_Pin , EnRS485);
}

/*--------------------------------------------------- UART  ---------------------------------------------------*/



void UART_CallBack_Register( void )
{
	HAL_UART_Receive_IT(&FIRMWARE_UART,&ui8RXTmp, 1);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
#if 0	
{
	HAL_UART_Receive_IT(&FIRMWARE_UART,&ui8RXTmp, 1);
	
	HAL_UART_Transmit(&DBUG_UART,&ui8RXTmp,1,1000);

}
#else
{
	if( huart->Instance == FIRMWARE_UART_INSTANCE ) 
	 {
		HAL_UART_Receive_IT(&FIRMWARE_UART,&ui8RXTmp, 1);

	 //If Previous Index & Current Index has Image/Packet Header Then Reset Index
	  if( ((ui8RXTmp == (UINT8)0xDD) && (ui8PrevRXTmp == (UINT8)0xBB)) ||  ((ui8RXTmp == (UINT8)0xA5) && (ui8PrevRXTmp == (UINT8)0xA5)) )
	   {
	   		if((ui8PrevRXTmp == (UINT8)0xBB))
	   		{
	   			ui8DataCpyFlag = IMG_HEADER;
				ui8InitCmdRetry = (UINT8)TRUE; //For Break the Init cmd
	   		}

			if((ui8PrevRXTmp == (UINT8)0xA5))
	   		{
	   			ui8DataCpyFlag = PAYLAOD_DATA;
	   		}

		   // 1St Byte of Header
	   	   sui32FwRxBuffIdx = 0;
			
		   psui8FirmwareBuffer[sui32FwRxBuffIdx] = ui8PrevRXTmp;

		   // 2nd Byte of Header
		   sui32FwRxBuffIdx++;
		   
		   psui8FirmwareBuffer[sui32FwRxBuffIdx] = ui8RXTmp;
		   
	   }
	   else
	   {
		   sui32FwRxBuffIdx++;
		   psui8FirmwareBuffer[sui32FwRxBuffIdx] = ui8RXTmp;
	   }

	  //If Previous Index & Current Index has Image/Packet Footer Then Copy Data & Reset Counter,Buffer
	   if( ( ((UINT8)psui8FirmwareBuffer[sui32FwRxBuffIdx-1] == (UINT8)0xBE) && ((UINT8)psui8FirmwareBuffer[sui32FwRxBuffIdx] == (UINT8)0xAF) )||\
	   		( ((UINT8)psui8FirmwareBuffer[sui32FwRxBuffIdx-1] == (UINT8)0xDE) && ((UINT8)psui8FirmwareBuffer[sui32FwRxBuffIdx] == (UINT8)0xAD) ) )
	   {
	   		if( ui8DataCpyFlag == IMG_HEADER )
	   		{
			
				//Header Received
				ui8PacketType = IMG_HEADER;
			}
			else if( ui8DataCpyFlag == PAYLAOD_DATA )
			{
				//Packet Received
				ui8PacketType = PAYLAOD_DATA;
			}	

			
		   sui32FwRxBuffIdx = 0;
   
	   }

	   ui8PrevRXTmp = ui8RXTmp ; //For Header Comparision

	 }

}
#endif

void ProgramBootImageHeaderIntoFlash ( UINT32 ui32FlashOffet )
{
	ReadImageIndexTableFromFlash(BOOT_IMAGE_HEADER_OFFSET);
	
	FLASH_ERASE(ui32FlashOffet,FLASH_SECTOR_LEN);

	 //Update img header value into Image index table
	 {
		StBootImgIndex.ui32DownLoadMarker	= INVALID_MARKER;
			
		StBootImgIndex.ui16ImgHdr			= StDownlaodImageHeader.ui16ImgHdr;
		
		StBootImgIndex.ui16ImgSize			= StDownlaodImageHeader.ui16ImgSize;
		
		StBootImgIndex.ui16ImgCrc			= StDownlaodImageHeader.ui16ImgCrc;
		
		StBootImgIndex.ui16TotalNoPkts		= StDownlaodImageHeader.ui16TotalNoPkts;
		
		StBootImgIndex.ui16ImgFooter		= StDownlaodImageHeader.ui16ImgFooter;
	 }

	UserProgramIntoFlash(ui32FlashOffet,&StBootImgIndex,sizeof(BOOT_IMG_HEADER_STRUCT));
	
}

UINT8 aui8RDData[128]={0};

void ReadImageIndexTableFromFlash( UINT32 ui32FlashOffet )
{
	//STM32_Serial_PutString("--------RD INDEX ST---------\r\n");

	INT_FLASH_READ(&aui8RDData[0],ui32FlashOffet,sizeof(BOOT_IMG_HEADER_STRUCT));

	//STM32_Serial_PutString("--------RD INDEX ED---------\r\n");

	memcpy(&StBootImgIndex,aui8RDData,sizeof(BOOT_IMG_HEADER_STRUCT));

	Print_BootImgIndex_Header();

}
void Print_BootImgIndex_Header ( void )
{
	STM32_Serial_PutString("--------BOOT INDEX---------\r\n");

	sprintf(ui8DBG,"%X",StBootImgIndex.ui32DownLoadMarker);
	STM32_Serial_PutString(ui8DBG);

	sprintf(ui8DBG,"%X",StBootImgIndex.ui16ImgHdr);
	STM32_Serial_PutString(ui8DBG);
								
	sprintf(ui8DBG,"%X",StBootImgIndex.ui16ImgSize);
	STM32_Serial_PutString(ui8DBG);

	sprintf(ui8DBG,"%X",StBootImgIndex.ui16ImgCrc);
	STM32_Serial_PutString(ui8DBG);

	sprintf(ui8DBG,"%X",StBootImgIndex.ui16TotalNoPkts);
	STM32_Serial_PutString(ui8DBG);

	sprintf(ui8DBG,"%X",StBootImgIndex.ui16ImgFooter);
	STM32_Serial_PutString(ui8DBG);
		
}



void Print_Img_Header ( void )
{
	STM32_Serial_PutString("--------IMG HDR---------\r\n");

	sprintf(ui8DBG,"%X",StDownlaodImageHeader.ui16ImgHdr);
	STM32_Serial_PutString(ui8DBG);
								
	sprintf(ui8DBG,"%X",StDownlaodImageHeader.ui16ImgSize);
	STM32_Serial_PutString(ui8DBG);

	sprintf(ui8DBG,"%X",StDownlaodImageHeader.ui16ImgCrc);
	STM32_Serial_PutString(ui8DBG);

	sprintf(ui8DBG,"%X",StDownlaodImageHeader.ui16TotalNoPkts);
	STM32_Serial_PutString(ui8DBG);

	sprintf(ui8DBG,"%X",StDownlaodImageHeader.ui16ImgFooter);
	STM32_Serial_PutString(ui8DBG);
	
}

void Print_PayLoad_Header ( void )
{
	STM32_Serial_PutString("--------IMG PAYLOAD HDR---------\r\n");
	sprintf(ui8DBG,"%X",StPayload.ui16PL_ImgHdr);
	STM32_Serial_PutString(ui8DBG);
								
	sprintf(ui8DBG,"%X",StPayload.ui16PL_CRC);
	STM32_Serial_PutString(ui8DBG);

	sprintf(ui8DBG,"%X",StPayload.ui16PL_PacketNo);
	STM32_Serial_PutString(ui8DBG);

	for(int inc=0;inc<PACKET_LENGTH;inc++)
	{
		sprintf(ui8DBG,"%X",StPayload.acui8PL_Data[inc]);
		STM32_Serial_PutString(ui8DBG);
	}

	sprintf(ui8DBG,"%X",StPayload.ui16PL_Footer);
	STM32_Serial_PutString(ui8DBG);


}



UINT16 ConvertBig162Little16( UINT16 ui16InData )
{

	return(UINT16) ( ((ui16InData&0xFF00) >> 8)| ((ui16InData & 0x00FF) << 8));

}


UINT8 Verify_App_Crc( void )
{
	UINT16 ui16ActCRC 	= 0;
	UINT16 HeaderCRC 	= StBootImgIndex.ui16ImgCrc;
	UINT16 ui16ImgSize 	= StBootImgIndex.ui16ImgSize;

	PUINT8 pui8Address = DRV_HW_SR_FLASH_ADDRESS;


	pui8Address = DRV_HW_SR_FLASH_ADDRESS;


	ui16ActCRC = crc_16( DRV_HW_SR_FLASH_ADDRESS, ui16ImgSize);




	if ( (HeaderCRC == ui16ActCRC )&& ( ui16ActCRC > 0) )
	{
		//STM32_Serial_PutString("\r\n!!!CRC MATCHED!!!\r\n");
		return TRUE;
	}
	
	sprintf(ui8DBG,"\r\nEXP CRC:%X ACT CRC:%X IMG SZ:%X",HeaderCRC,ui16ActCRC,ui16ImgSize);
	STM32_Serial_PutString(ui8DBG);
	STM32_Serial_PutString("\r\n!!!CRC-NOT-MATCHED!!!\r\n");
	return FALSE;

}

void Initialize_Parameter ( void )
{
	//Register Call Back
	UART_CallBack_Register();
	
	//Enable RS485 Rx mode
	RS485TxAPI_GPIO(GPIO_PIN_RESET);

	//Init CRC
	Init_CRC16();

	//Allocate Memory for UART Rx Buffer
	psui8FirmwareBuffer = malloc(1024);
	
	if(psui8FirmwareBuffer == NULL )
	{
		STM32_Serial_PutString("Fail To Allocate UART Rx Buffer\r\n");
	}
	memset(psui8FirmwareBuffer,0x00,(PACKET_LENGTH*2));

	//STM32_Serial_PutString("Read App Img Index Table\r\n");

	//Update Image Index Table
	ReadImageIndexTableFromFlash(BOOT_IMAGE_HEADER_OFFSET);
	
}


void BL_PROCESS( void )
{
	UINT8 ui8Status = FALSE;
	
	//Jump To Firmware Downloader
	if( ((UINT32)DOWN_MARKER == (UINT32)StBootImgIndex.ui32DownLoadMarker) || ( StBootImgIndex.ui16ImgHdr != IMAGE_HEADER ))
	{
	
		sprintf(ui8DBG,"MKR:%X HDR:%X\r\n",StBootImgIndex.ui32DownLoadMarker,StBootImgIndex.ui16ImgHdr);  
		STM32_Serial_PutString(ui8DBG);
	  
		//Main Process of init & Collect the Data packet from HOST
		FirmwareDowloadProcess();
	}
	
	//Verify CRC 
	ui8Status = Verify_App_Crc();
	
	if( ui8Status == TRUE )
	{
	
	 //Jump To Firmware   
	 BL_JumpToApp();
	 
	}else
	{
		//Main Process of init & Collect the Data packet from HOST
		FirmwareDowloadProcess();
	}

}
