#include <string.h>
#include "bootblock_oldstyle.h"
#include "serial_vectors.h"
#include "systime.h"
#include "rdy_run.h"
#include "netx_consoleapp.h"
#include "uart_standalone.h"
#include "uprintf.h"
#include "flasher_version.h"
#include "flasher_spi.h"
#include "spi.h"

extern unsigned long load_address;
extern unsigned long flash_data_start;

/* The reserved area from pulBufferStart to pulBufferEnd is used 
   to decompress the SPI flash list. */
extern unsigned long pulBufferStart;
extern unsigned long pulBufferEnd;

void ramtest_clear_serial_vectors(void); 
void ramtest_clear_serial_vectors(void)
{
	tSerialVectors.fn.fnGet   = NULL;
	tSerialVectors.fn.fnPut   = NULL;
	tSerialVectors.fn.fnPeek  = NULL;
	tSerialVectors.fn.fnFlush = NULL;
}
/*-------------------------------------------------------------------------*/

void boot_flasher_main(const BOOTBLOCK_OLDSTYLE_U_T *ptBootBlock) __attribute__ ((noreturn));
void boot_flasher_main(const BOOTBLOCK_OLDSTYLE_U_T *ptBootBlock)
{
	NETX_CONSOLEAPP_RESULT_T tResult;
	void *pvReturnMessage;
	
	FLASHER_SPI_CONFIGURATION_T tFlasherSpiConfig; /* see spi.h */
	FLASHER_SPI_FLASH_T tSpiFlashDescription;  /* see spi_flash.h */
	
	char *pcBufferStart; /* RAM buffer area (to unpack SPI flash types)*/
	char *pcBufferEnd; 

	unsigned char *pucDataStartAdr; /* data to flash in RAM */
	unsigned char *pucDataEndAdr;
	unsigned long ulDataByteSize;
	
	unsigned long ulStartAdr;  /* area to flash */
	unsigned long ulEndAdr;
	
	unsigned long ulEraseStartAdr; /* area to erase*/
	unsigned long ulEraseEndAdr;
	
	tFlasherSpiConfig.uiUnit = 0;  /* selects the SPI unit */
	tFlasherSpiConfig.uiChipSelect = 0;  /* selects the CS line on that unit */
	tFlasherSpiConfig.ulInitialSpeedKhz = 1000;
	tFlasherSpiConfig.ulMaximumSpeedKhz = 25000;
	tFlasherSpiConfig.uiIdleCfg = 
		/* default line state in idle mode: all lines driven high */
		MSK_SQI_CFG_IDLE_IO1_OE + MSK_SQI_CFG_IDLE_IO1_OUT +
		MSK_SQI_CFG_IDLE_IO2_OE + MSK_SQI_CFG_IDLE_IO2_OUT +
		MSK_SQI_CFG_IDLE_IO3_OE + MSK_SQI_CFG_IDLE_IO3_OUT;
	tFlasherSpiConfig.uiMode = FLASHER_SPI_MODE3;
	tFlasherSpiConfig.aucMmio[0] = 0xff; /* MMIO pin for chip select*/
	tFlasherSpiConfig.aucMmio[1] = 0xff; /* MMIO pin for clock */
	tFlasherSpiConfig.aucMmio[2] = 0xff; /* MMIO pin for MISO */
	tFlasherSpiConfig.aucMmio[3] = 0xff; /* MMIO pin for MOSI */
	
	pcBufferStart = (char*) &pulBufferStart; /* RAM buffer */
	pcBufferEnd   = (char*) &pulBufferEnd; 
	
	pucDataStartAdr = (unsigned char*) &flash_data_start;      /* data in RAM to write to flash*/
	pucDataEndAdr   = (unsigned char*) (&load_address + ptBootBlock->s.sizApplicationDword);  
	ulDataByteSize  = (unsigned long) (pucDataEndAdr - pucDataStartAdr);
	
	/* Define the area of the flash we want to erase/write
	   and the location of the data to write in RAM. */
	ulStartAdr = 0;                          /* start offset in flash */
	ulEndAdr = ulStartAdr + ulDataByteSize;  /* end offset: start + data size*/
	
	systime_init();

#if CFG_DEBUGMSG==1
	uart_standalone_initialize();
#else
	ramtest_clear_serial_vectors();
#endif

	uprintf("\f. *** Flasher by cthelen@hilscher.com ***\n");
	uprintf("V" FLASHER_VERSION_ALL "\n\n");
	uprintf("data:   0x%08x - 0x%08x (size 0x%08x bytes)\n", pucDataStartAdr, pucDataEndAdr, ulDataByteSize);
	uprintf("flash:  0x%08x - 0x%08x\n", ulStartAdr, ulEndAdr);
	uprintf("buffer: 0x%08x - 0x%08x\n", pcBufferStart, pcBufferEnd);
	
	
	tResult = spi_detect(&tFlasherSpiConfig, &tSpiFlashDescription, pcBufferEnd);
	if (NETX_CONSOLEAPP_RESULT_OK != tResult) 
	{
		/* detect failed */
		uprintf("Error: Flash detection failed\n");
	}
	else
	{
		/* Check if the area is empty (all bytes 0xff) */
		
		tResult = spi_isErased(&tSpiFlashDescription, ulStartAdr, ulEndAdr, &pvReturnMessage);
		/* pvReturnMessage == 0xff, if the area is erased. */
		
		if (NETX_CONSOLEAPP_RESULT_OK != tResult) 
		{
			/* isErased failed */
			uprintf("Error: Is erased check failed\n");
		}
		else
		{
			/* If it is not erased, do erase it */
			/* Start by extending the range to be erased to a full sector */
			
			if (pvReturnMessage != (void*)(unsigned long) 0xff)
			{
				tResult = spi_getEraseArea(&tSpiFlashDescription, ulStartAdr, ulEndAdr, &ulEraseStartAdr, &ulEraseEndAdr);
				if (NETX_CONSOLEAPP_RESULT_OK != tResult) 
				{
					/* getEraseArea failed */
					uprintf("Error: getEraseArea failed\n");
				}
				else
				{	
					/* erase */
					tResult = spi_erase(&tSpiFlashDescription, ulEraseStartAdr, ulEraseEndAdr);
					if (NETX_CONSOLEAPP_RESULT_OK != tResult) 
					{
						/* erase failed */
						uprintf("Error: Flash erase failed\n");
					}
					else
					{
						/* Check again if the area is erased */
						tResult = spi_isErased(&tSpiFlashDescription, ulStartAdr, ulEndAdr, &pvReturnMessage);
						/* pvReturnMessage == 0xff, if the area is erased. */
						
						if (NETX_CONSOLEAPP_RESULT_OK != tResult) 
						{
							/* isErased failed */
						} else if (pvReturnMessage != (void*)(unsigned long) 0xff)
						{
							/* erase failed */
							tResult = NETX_CONSOLEAPP_RESULT_ERROR;
							uprintf("Error: Flash erase failed\n");
						}
					}
				}
			}
			
			/* We assume that the flash area is erased now. */
			if (NETX_CONSOLEAPP_RESULT_OK == tResult) 
			{
				/* Write to flash */
				tResult = spi_flash(&tSpiFlashDescription, ulStartAdr, ulDataByteSize, pucDataStartAdr);
				if (NETX_CONSOLEAPP_RESULT_OK != tResult) 
				{
					/* write failed */
					uprintf("Error: Flash write failed\n");
				}
				else
				{
					/* Verify once again. 
					   This is usually not necessary, because the flash functions also verify. */
					tResult = spi_verify(&tSpiFlashDescription, ulStartAdr, ulEndAdr, pucDataStartAdr, &pvReturnMessage); 
					/* pvReturnMessage is the same as tResult */
					if (NETX_CONSOLEAPP_RESULT_OK != tResult) 
					{
						/* verify failed */
						uprintf("Error: Verify failed\n");
					}
					else if ((void*)NETX_CONSOLEAPP_RESULT_OK != pvReturnMessage)
					{
						/* verify failed  */
						tResult = NETX_CONSOLEAPP_RESULT_ERROR;
						uprintf("Error: Verify failed\n");
					}
					else
					{
						/* success: verify OK */
						uprintf("Success: Verify OK\n");
					}
				}
			}
		}
	}
	
	if (NETX_CONSOLEAPP_RESULT_OK == tResult) 
	{
		uprintf("\n");
		uprintf(" ***   *    * \n");
		uprintf("*   *  *  *   \n");
		uprintf("*   *  * *    \n");
		uprintf("*   *  **     \n");
		uprintf("*   *  * *    \n");
		uprintf("*   *  *  *   \n");
		uprintf(" ***   *   *  \n");
		uprintf("\n");
		
		rdy_run_setLEDs(RDYRUN_GREEN);
	}
	else
	{
		uprintf("\n");
		uprintf("*****  ****   ****    ***   ****  \n");
		uprintf("*      *   *  *   *  *   *  *   * \n");
		uprintf("*      *   *  *   *  *   *  *   * \n");
		uprintf("****   ****   ****   *   *  ****  \n");
		uprintf("*      **     **     *   *  **    \n");
		uprintf("*      * *    * *    *   *  * *   \n");
		uprintf("*****  *  *   *  *    ***   *  *  \n");
		uprintf("\n");
		
		rdy_run_setLEDs(RDYRUN_YELLOW);
	}
	
	while(1);
}

/*-------------------------------------------------------------------------*/
