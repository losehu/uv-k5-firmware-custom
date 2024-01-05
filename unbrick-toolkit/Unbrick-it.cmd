@REM Fires 1-handed unbricking command using cheap ST-Link v2 "baite" debug probe
@REM No need to solder. Just press the right wires on the right pads with your fingers.
@REM Restores bootloader 2.00.06 with factory firmware 2.01.27
@ECHO ******** Launching 1 shot unbricking process with OpenOCD (STLink v2)...
@ECHO .
@CD OpenOCD
@openocd -f interface/stlink.cfg -f target/dp32g030.cfg -c "reset halt" -c "uv_flash_bl bootloader.bin" -c "uv_flash_fw k5_v2.01.27_fw.bin" -c "shutdown"
@PAUSE

