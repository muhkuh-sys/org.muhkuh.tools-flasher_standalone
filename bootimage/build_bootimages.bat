mkdir flash_blinki_intram
mkdir flash_blinki_intram_dbg
mkdir flash_blinki_nxhx51R4_sdram
mkdir flash_blinki_nxhx51R4_sdram_dbg

python append.py -v debug ^
	..\targets\netX56_standalone_flasher_intram\bootimages\netx56_standalone_flasher_intram.bin ^
	blinki_netx56_spi_intram.bin ^
	flash_blinki_intram\netx.rom

python append.py -v debug ^
	..\targets\netX56_standalone_flasher_intram_dbg\bootimages\netx56_standalone_flasher_intram_dbg.bin ^
	blinki_netx56_spi_intram.bin ^
	flash_blinki_intram_dbg\netx.rom

setlocal
rem for NXHX51-ETM Rev 4   MT48LC2M32B2-6A IT J
rem values from netx.xml and Secmem
set SdramPars=-g 0x030d0001 -t 0x00a12151
set SdramPars=-g 51183617 -t 10559825
rem COMPVERIFY-20  
set SdramPars=-g 0x030D0001 -t 0x01A22251
set SdramPars=-g 51183617 -t 27402833

python append.py -v debug %SdramPars%^
	..\targets\netX56_standalone_flasher_sdram\bootimages\netx56_standalone_flasher_sdram.bin ^
	blinki_netx56_spi_intram.bin ^
	flash_blinki_nxhx51R4_sdram\netx.rom

python append.py -v debug %SdramPars% ^
	..\targets\netX56_standalone_flasher_sdram_dbg\bootimages\netx56_standalone_flasher_sdram_dbg.bin ^
	blinki_netx56_spi_intram.bin ^
	flash_blinki_nxhx51R4_sdram_dbg\netx.rom
