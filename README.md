# STM32F030RCT6_BL



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
