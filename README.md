# STM32F030RCT6_BL

REFER HOST APPLICATION
----------------------
https://github.com/hi3520/STM32_Firmware_Uploader_linux


BOOTLOADER NOTE START:
----

Step #1  The RAM & FLASH LENGTH are different for each ChipSet of ST. So refer data sheet modify as per your need's

PROJECT DETAILS:
----------------
   Project name: BOOTLOADER 

   ChipSet Name: STM32F030RCT6

			RAM: 32K

		  FLASH: 256K


SOFTWARE DETAILS:
----------------
 			 IDE: STM32CUBEIDE
 BootLoader Size: 20K
Application Size: 256K - 20K



Step #2 Based On "Step #1" Details & Modify Below Section and Update into Linker File Script
--


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



Step #3 Call "BL_JumpToApp" in Main after the Driver initialization
--

FIRMWARE NOTE START:
--

Step #1  The RAM & FLASH LENGTH are different for each ChipSet of ST. So refer data sheet modify as per your need's
---

Step #2 Based On "Step #1" Details & Modify Below Section and Update into Linker File Script
---

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


Step #3 Copy below variables in main.c as global vairables 
---

extern uint32_t _main_app_start_address;

#define FIRMWARE_START_ADDR	(uint32_t)(&_main_app_start_address)



Step #4 Call "Main_APP_Init " API in Before the Driver initialization. {For Reset vector table copying process}
---

