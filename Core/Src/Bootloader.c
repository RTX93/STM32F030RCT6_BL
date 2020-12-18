/*
 * 	 File Name: Bootloader.c
 *
 *  Created on: Dec 2, 2020
 *      Author: Suresh
 */
/* ******************************************************************************************************************************** */
/*											Device Related Porting Info For STM BL	   										         */
/* ********************************************************************************************************************************* */


/* ******************************************************************************************************************************** */
/*													BOOTLOADER NOTE START																*/
/* ********************************************************************************************************************************* */
//Step #1  The RAM & FLASH LENGTH are different for each ChipSet of ST. So refer data sheet modify as per your need's
/*
PROJECT DETAILS:
----------------
   Project name: SVM MAIN CONTROLLER
   ChipSet Name: STM32F030RCT6
			RAM: 32K
		  FLASH: 256K

SOFTWARE DETAILS:
----------------

 BootLoader Size: 20K
Application Size: 256K - 20K


*/

//Step #2 Update "Step #1" Details & Over write Below Section (Available within the '{{{ }}}' ) in Linker File Script

#ifdef NOTE_FOR_BL
/* {{{ */

	 /* Memories Definition */
	MEMORY
	{
	  RAM    		(xrw)   : ORIGIN = 0x200000C0,   LENGTH = 32K - 192 /* 192 For Vector Table Relocate */
	  BOOTLOADER	(rx)    : ORIGIN = 0x8000000,   LENGTH = 20K
	  /*FIRMWARE		(rx)    : ORIGIN = 0x8005000,   LENGTH = 256K - 20K*/
	}
	/* Main App Start Symbol */
	_main_app_start_address = 0x08005000;

	/* For Main Application, change BOOTLOADER by FIRMWARE */
	REGION_ALIAS("FLASH", BOOTLOADER );

/* }}} */
#endif /*NOTE*/

//Step #3 Call "BL_JumpToApp" in Main after the Driver initialization.

/* ******************************************************************************************************************************** */
/*													BOOTLOADER NOTE END																*/
/* ********************************************************************************************************************************* */

/* ******************************************************************************************************************************** */
/*													FIRMWARE NOTE START																*/
/* ********************************************************************************************************************************* */
//Step #1  The RAM & FLASH LENGTH are different for each ChipSet of ST. So refer data sheet modify as per your need's
/*
PROJECT DETAILS:
----------------
   Project name: SVM MAIN CONTROLLER
   ChipSet Name: STM32F030RCT6
			RAM: 32K
		  FLASH: 256K

FIRMWARE DETAILS:
----------------

 BootLoader Size: 20K In Hex 0x5000h
Application Size: 256K - 20K


*/

//Step #2 Update "Step #1" Details & Over write Below Section (Available within the '/*{{{*/ /*}}}*/' ) in Linker File Script

#ifdef NOTE_FOR_APP
/* {{{ */

	 /* Memories Definition */
	MEMORY
	{
	  RAM    		(xrw)   : ORIGIN = 0x200000C0,   LENGTH = 32K - 192 /* 192 For Vector Table Relocate */
	  /*BOOTLOADER	(rx)    : ORIGIN = 0x8000000,   LENGTH = 20K*/
	  FIRMWARE		(rx)    : ORIGIN = 0x8005000,   LENGTH = 256K - 20K
	}
	/* Main App Start Symbol */
	_main_app_start_address = 0x08005000;

	/* For Main Application, change BOOTLOADER by FIRMWARE */
	REGION_ALIAS("FLASH", FIRMWARE );

/* }}} */
#endif

//Step #3 Copy below variables in main.c as global vairables(Available within the '/*{{{*/ /*}}}*/' )

#ifdef NOTE_FOR_APP
/* {{{ */

extern uint32_t _main_app_start_address;

#define FIRMWARE_START_ADDR	(uint32_t)(&_main_app_start_address)

/* }}} */
#endif



//Step #4 Call "Main_APP_Init " API in Before the Driver initialization. {For Reset vector table copying process}


/* ******************************************************************************************************************************** */
/*													BOOTLOADER NOTE END																*/
/* ********************************************************************************************************************************* */

/* ---------------------------------------------------------------------------------------------------------------------------------- */

/*---------------------------------------INCLUDES ----------------------------------------------------*/
#include "Bootloader.h"
#include "Ldrprotocol.h"

/*----------------------------------------TYPEDEFS ---------------------------------------------------*/

/*--------------------------------------- DEFINES ----------------------------------------------------*/

/*---------------------------------------- MACROS ----------------------------------------------------*/

/*-------------------------------------- Extern GLOBAL VAR'S(HAL) -------------------------------------*/
extern UINT8 ui8DBG[ 256 ];

extern BOOT_IMG_HEADER_STRUCT	StBootImgIndex;
/*-------------------------------------- GLOBAL VAR'S -------------------------------------------------*/

/*---------------------------------- FUNCTION PROTO TYPE'S --------------------------------------------*/

/*----------------------------------------- FUNCTION'S -------------------------------------------------*/


void BL_JumpToApp( void )
{

	STM32_Serial_PutString("P\r\nJUMP TO APP\r\n");

	/* Jump to user application */
	// disable global interrupt
	__disable_irq();

	// Disable all peripheral interrupts
	__NVIC_DisableIRQ(SysTick_IRQn);
	__NVIC_DisableIRQ(USART2_IRQn);

	__NVIC_DisableIRQ(WWDG_IRQn);
	__NVIC_DisableIRQ(RTC_IRQn);
	__NVIC_DisableIRQ(FLASH_IRQn);
	__NVIC_DisableIRQ(RCC_IRQn);
	__NVIC_DisableIRQ(EXTI0_1_IRQn);
	__NVIC_DisableIRQ(EXTI2_3_IRQn);
	__NVIC_DisableIRQ(EXTI4_15_IRQn);
	__NVIC_DisableIRQ(DMA1_Channel1_IRQn);
	__NVIC_DisableIRQ(DMA1_Channel2_3_IRQn);
	__NVIC_DisableIRQ(DMA1_Channel4_5_IRQn);
	__NVIC_DisableIRQ(ADC1_IRQn);
	__NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
	__NVIC_DisableIRQ(TIM1_CC_IRQn);
	__NVIC_DisableIRQ(TIM3_IRQn);
	__NVIC_DisableIRQ(TIM6_IRQn);
	__NVIC_DisableIRQ(TIM7_IRQn);
	__NVIC_DisableIRQ(TIM14_IRQn);
	__NVIC_DisableIRQ(TIM15_IRQn);
	__NVIC_DisableIRQ(TIM16_IRQn);
	__NVIC_DisableIRQ(TIM17_IRQn);
	__NVIC_DisableIRQ(I2C1_IRQn);
	__NVIC_DisableIRQ(I2C2_IRQn);
	__NVIC_DisableIRQ(SPI1_IRQn);
	__NVIC_DisableIRQ(SPI2_IRQn);
	__NVIC_DisableIRQ(USART1_IRQn);
	__NVIC_DisableIRQ(USART2_IRQn);
	__NVIC_DisableIRQ(USART3_6_IRQn);


	uint32_t MemoryAddr = (uint32_t)&_main_app_start_address;
	uint32_t *pMem = (uint32_t *)MemoryAddr;

	// First address is the stack pointer initial value
	__set_MSP(*pMem); // Set stack pointer

	// Now get main app entry point address
	pMem++;
	void (*pMainApp)(void) = (void (*)(void))(*pMem);

	// Jump to main application (0x0800 0004)
	pMainApp();

	STM32_Serial_PutString("JUMP TO APP NOT HAPPENED\r\n");

}


void Main_APP_Init( void )
{
	// Copy interrupt vector table to the RAM.
	volatile uint32_t *VectorTable = (volatile uint32_t *)0x20000000;
	uint32_t ui32_VectorIndex = 0;

	for(ui32_VectorIndex = 0; ui32_VectorIndex < 48; ui32_VectorIndex++)
	{
		VectorTable[ui32_VectorIndex] = *(__IO uint32_t*)((uint32_t)FIRMWARE_START_ADDR + (ui32_VectorIndex << 2));
	}

	__HAL_RCC_AHB_FORCE_RESET();

	//	Enable SYSCFG peripheral clock
	__HAL_RCC_SYSCFG_CLK_ENABLE();

	__HAL_RCC_AHB_RELEASE_RESET();

	// Remap RAM into 0x0000 0000
	__HAL_SYSCFG_REMAPMEMORY_SRAM();

	__enable_irq();
}




/*
 * General Function Definitions
 */
void STM32_Serial_PutString(char *s)
{
  while (*s != '\0')
  {
	STM32_SerialPutChar(*s);
	s++;
  }
}

/*--------------------------------------------- HAL UART INTERFACE ------------------------------------------------------*/

void STM32_SerialPutChar(uint8_t c)
{
	HAL_UART_Transmit(&DBUG_UART,&c,1,1000);
}

/*--------------------------------------------- HAL FLASH INTERFACE ------------------------------------------------------*/


uint8_t UserProgramIntoFlash(uint32_t Address, char * pcBuffer,uint16_t u16length)
{
	uint64_t WrData = 0;
	uint32_t inc=0;

	while(inc < u16length)
	{
		memcpy(&WrData,&pcBuffer[inc],4);
		
		INT_FLASH_WRITE_WORD(Address,WrData);

		Address = (Address + 4);
		inc = inc + 4;	
	}	

	return 1;
}


void INT_FLASH_WRITE_WORD(uint32_t Address,uint64_t Data)
{
    HAL_StatusTypeDef STATUS = HAL_ERROR;
	uint32_t TypeProgram 	 = FLASH_TYPEPROGRAM_WORD;
	


	HAL_FLASH_Unlock();

	STATUS = HAL_FLASH_Program(TypeProgram,Address,Data);

	if( HAL_OK != STATUS)
	{
		STM32_Serial_PutString("PRG_FAIL\r\n");
	}
	HAL_FLASH_Lock();
 
}

void INT_FLASH_READ(PUINT8 pui8RDData, PUINT8 pu8FLASH_ADDRESS, UINT32 Length )
{

	for(UINT32 ui32inc =0;ui32inc< Length; ui32inc++ )
	{
			//sprintf(ui8DBG,"%X\r\n",pu8FLASH_ADDRESS);
			//STM32_Serial_PutString(ui8DBG);
			
			pui8RDData[ui32inc] = (*(uint8_t*)pu8FLASH_ADDRESS);
			
			pu8FLASH_ADDRESS	= pu8FLASH_ADDRESS+1;
	}

}



void FLASH_ERASE(uint32_t FLASH_ADDRESS , uint32_t DataLength )
{
	HAL_StatusTypeDef STATUS = HAL_ERROR;
	FLASH_EraseInitTypeDef  Flash_Erase	= {0};
	uint32_t PageError = 0, NoOfPage =0,ModValue=0;
	
	NoOfPage = (DataLength / FLASH_SECTOR_LEN);

	ModValue = ( DataLength % FLASH_SECTOR_LEN ); //For Find Fractioned Page Size

	if(ModValue)
	{
		NoOfPage = NoOfPage + 1; // +1 For Fractioned Number Make Full Number
	}	
/*
	sprintf(ui8DBG,"\r\nIMG-LEN:[%X]N-Page:[%X]\r\n",DataLength,NoOfPage);
	STM32_Serial_PutString(ui8DBG);
*/						
	HAL_FLASH_Unlock();
	
	Flash_Erase.TypeErase 		= FLASH_TYPEERASE_PAGES;
	Flash_Erase.PageAddress 	= FLASH_ADDRESS;
	Flash_Erase.NbPages			= NoOfPage;

	STATUS = HAL_FLASHEx_Erase(&Flash_Erase,&PageError);
	
	if( HAL_OK != STATUS)
	{
		STM32_Serial_PutString("ERASE_FAIL\r\n");
	}
	
	HAL_FLASH_Lock();

}



	
