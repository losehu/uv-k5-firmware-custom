/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */
#include "ui/ui.h"
#include <stdint.h>
#include <string.h>

#include <stdio.h>     // NULL
#include "app/mdc1200.h"
#include "app/uart.h"
#ifdef ENABLE_AM_FIX
	#include "am_fix.h"
#endif
#include "audio.h"
#include "board.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "version.h"
#include "app/app.h"
#include "app/dtmf.h"
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/syscon.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "driver/systick.h"
#ifdef ENABLE_UART
#include "driver/uart.h"
#endif
#include "helper/battery.h"
#include "helper/boot.h"

#include "ui/lock.h"
#include "ui/welcome.h"
#include "ui/menu.h"
#include "driver/eeprom.h"

void _putchar(__attribute__((unused)) char c)
{

#ifdef ENABLE_UART
    UART_Send((uint8_t *)&c, 1);
#endif

}
void Main(void)
{
	//BOOT_Mode_t  BootMode;

	// Enable clock gating of blocks we need
	SYSCON_DEV_CLK_GATE = 0
		| SYSCON_DEV_CLK_GATE_GPIOA_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_GPIOB_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_GPIOC_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_UART1_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_SPI0_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_SARADC_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_CRC_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_AES_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_PWM_PLUS0_BITS_ENABLE;

	SYSTICK_Init();
	BOARD_Init();


	boot_counter_10ms = 250;   // 2.5 sec
#ifdef ENABLE_UART
    UART_Init();
#endif



	memset(gDTMF_String, '-', sizeof(gDTMF_String));
	gDTMF_String[sizeof(gDTMF_String) - 1] = 0;

	BK4819_Init();

	BOARD_ADC_GetBatteryInfo(&gBatteryCurrentVoltage, &gBatteryCurrent);


    SETTINGS_InitEEPROM();
    SETTINGS_WriteBuildOptions();
    SETTINGS_LoadCalibration();
#ifdef ENABLE_MDC1200
    MDC1200_init();
#endif
	RADIO_ConfigureChannel(0, VFO_CONFIGURE_RELOAD);
	RADIO_ConfigureChannel(1, VFO_CONFIGURE_RELOAD);

	RADIO_SelectVfos();

	RADIO_SetupRegisters(true);

    for (unsigned int i = 0; i < ARRAY_SIZE(gBatteryVoltages); i++) {
        BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[i], &gBatteryCurrent);
    }
	BATTERY_GetReadings(false);

	#ifdef ENABLE_AM_FIX
		AM_fix_init();
	#endif

	//BootMode = BOOT_GetMode();
	
//	if (BootMode == BOOT_MODE_F_LOCK)
        gMenuListCount = 45;


	// wait for user to release all butts before moving on
//	if (/*!GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT) ||*/
//	     KEYBOARD_Poll() != KEY_INVALID ||
//		 BootMode != BOOT_MODE_NORMAL)
//	{	// keys are pressed
//		UI_DisplayReleaseKeys(BootMode);
		BACKLIGHT_TurnOn();
		while (KEYBOARD_Poll() != KEY_INVALID)  // 500ms
		{
        }
		gKeyReading0 = KEY_INVALID;
		gKeyReading1 = KEY_INVALID;
		gDebounceCounter = 0;
//	}

		UI_DisplayWelcome();
    boot_counter_10ms=250;
    while (boot_counter_10ms > 0)
    {
        if (KEYBOARD_Poll() == KEY_EXIT)
        {	// halt boot beeps
            boot_counter_10ms = 0;
            break;
        }
#ifdef ENABLE_BOOT_BEEPS

        if ((boot_counter_10ms % 25) == 0)
					AUDIO_PlayBeep(BEEP_880HZ_40MS_OPTIONAL);
#endif

    }


#ifdef ENABLE_PWRON_PASSWORD
		if (gEeprom.POWER_ON_PASSWORD < 1000000)
		{
			bIsInLockScreen = true;
			UI_DisplayLock();
			bIsInLockScreen = false;
		}
#endif

	//	BOOT_ProcessMode();
    GUI_SelectNextDisplay(DISPLAY_MAIN);

		GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);

		gUpdateStatus = true;

#ifdef ENABLE_VOICE
		{
			uint8_t Channel;

			AUDIO_SetVoiceID(0, VOICE_ID_WELCOME);

			Channel = gEeprom.ScreenChannel[gEeprom.TX_VFO];
			if (IS_MR_CHANNEL(Channel))
			{
				AUDIO_SetVoiceID(1, VOICE_ID_CHANNEL_MODE);
				AUDIO_SetDigitVoice(2, Channel + 1);
			}
			else if (IS_FREQ_CHANNEL(Channel))
				AUDIO_SetVoiceID(1, VOICE_ID_FREQUENCY_MODE);

			AUDIO_PlaySingleVoice(0);
		}
#endif

#ifdef ENABLE_NOAA
		RADIO_ConfigureNOAA();
#endif

//    uint8_t B[64];
//    memset(B,'B',sizeof (B));
//    for ( uint16_t j = 0; j < ARRAY_SIZE(MDC_ADD); ++j) {
//        for (uint16_t i =MDC_ADD[j]; i < MDC_ADD[j]+64; ++i) {
//            EEPROM_WriteBuffer(i,&B[i-MDC_ADD[j]]);
//        }
//    }
//
//
//    uint8_t A[16]={0XFF,0XFF,'B','G','2','F','Z','V',' ','U','V','K','5',' ','S','B'};
//    for (int i =MDC_ADD[0]; i < MDC_ADD[0]+16; ++i) {
//        EEPROM_WriteBuffer(i,&A[i-MDC_ADD[0]]);
//    }
	while (1)
	{



        APP_Update();

		if (gNextTimeslice)
		{
			APP_TimeSlice10ms();
		}

		if (gNextTimeslice_500ms)
		{
			APP_TimeSlice500ms();
		}
	}
}
