/*
 * Bootloader.h
 *
 *  Created on: Dec 2, 2020
 *      Author: Suresh
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_
/*---------------------------------------INCLUDES ----------------------------------------------------*/
#include "main.h"
#include "stdlib.h"
#include <string.h>
/*----------------------------------------TYPEDEFS ---------------------------------------------------*/

typedef unsigned char   UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;

typedef unsigned char*   PUINT8;
typedef unsigned short* PUINT16;
typedef unsigned int*   PUINT32;


typedef char  INT8;
typedef short INT16;
typedef int   INT32;

typedef char CHAR;




/*--------------------------------------- DEFINES ----------------------------------------------------*/
#define TRUE 		(UINT8)1
#define FALSE		(UINT8)0

#define FLASH_SECTOR_LEN (2 * 1024)
/*---------------------------------------- MACROS ----------------------------------------------------*/


/*-------------------------------------- Extern GLOBAL VAR'S(HAL) -------------------------------------------------*/

extern uint32_t _main_app_start_address;// main app start address defined in linker file in the name of "_main_app_start_address"

extern UART_HandleTypeDef huart2; //UART
extern UART_HandleTypeDef huart6; //UART
extern UART_HandleTypeDef huart5; //UART


/*-------------------------------------- GLOBAL VAR'S -------------------------------------------------*/

#define DBUG_UART 					huart6 
#define FIRMWARE_UART   			huart2
#define FIRMWARE_UART_INSTANCE		USART2

#define FIRMWARE_START_ADDR			(uint32_t)(&_main_app_start_address)

#define DRV_HW_SR_FLASH_ADDRESS 	FIRMWARE_START_ADDR
														//200KB
#define BOOT_IMAGE_HEADER_OFFSET    0x8032000

#define DOWN_MARKER					0xEEDDCCBB
#define INVALID_MARKER				0xFFFFFFFF


/*---------------------------------- FUNCTION PROTO TYPE'S ---------------------------------------------*/

void STM32_Serial_PutString(char *s);

void STM32_SerialPutChar(uint8_t c);

void BL_JumpToApp( void );

void Main_APP_Init( void );

void INT_FLASH_READ(PUINT8 pui8RDData, PUINT8 pu8FLASH_ADDRESS, UINT32 Length );

void INT_FLASH_WRITE_WORD(uint32_t Address,uint64_t Data);

uint8_t UserProgramIntoFlash(uint32_t Address, char * pcBuffer,uint16_t u16length);


#endif /* INC_BOOTLOADER_H_ */
