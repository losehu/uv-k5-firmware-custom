**Read this in other languages: [English](./README_en.md), [中文](./README.md).**

**语言版本: [English](./README_en.md), [中文](./README.md).**

# Replacing the bigger EEPROM
* **2Mbit** or **1Mbit**
**By replacing it, you may be able to achieve:**
* Display and input in any language (implemented)
* Automatic Doppler frequency shift function (implemented)
* Custom boot-up images (implemented)
* Custom SSTV images (guess)
* DOPPLER MODE (implemented)
* SI4732 FULL SSB SUPPORT
* ......
* More available flash allows for additional custom features(about 1.5K?).


# [K5Web](https://k5.vicicode.com/)
* Supports online firmware compilation, no need to install a compilation environment!!
* Doppler satellite, boot image text, and SI4732 SSB patch writing method!
* Supports **Creative Workshop**, where you can upload custom firmware and boot images after registering and logging in!!!

Visit: [K5Web](https://k5.vicicode.com/)

# [Custom Bootloader](https://github.com/losehu/uv-k5-bootloader-custom)

* By creating a bootloader that loads into RAM, firmware switching is achieved.
* Any firmware can be switched.
* Currently, it is only suitable for 4Mib EEPROM, but it can be easily extended to other sizes of EEPROM by modifying the code.

# Version Description
* Currently, the firmware is available in the following versions: **LOSEHUxxx**, **LOSEHUxxxK**, **LOSEHUxxxH**, **LOSEHUxxxE**, **LOSEHUxxxEK**, **LOSEHUxxxHS**
* **LOSEHUxxx**: Chinese firmware, no additional EEPROM needed, includes MDC1200, spectrum, and radio.
* **LOSEHUxxxK**: Chinese firmware, requires more than 1Mib EEPROM, includes Doppler mode, MDC1200, spectrum, radio, Chinese channel names, custom Chinese boot characters, and boot image.
* **LOSEHUxxxH**: Chinese firmware, requires more than 2Mib EEPROM, includes Doppler mode, Chinese input method, spectrum, radio, Chinese channel names, custom Chinese boot characters, and boot image.
* **LOSEHUxxxHS**: Chinese firmware, requires more than 2Mib EEPROM, includes SI4732 radio, Chinese input method, spectrum, Chinese channel names, custom Chinese boot characters, and boot image.
* **LOSEHUxxxE**: English firmware, no additional EEPROM needed, includes MDC1200, spectrum, radio, and messaging.
* **LOSEHUxxxEK**: English firmware, requires more than 1Mib EEPROM, includes Doppler mode, MDC1200, spectrum, radio, custom boot characters, and boot image.



# Multi-functional K5/6 Firmware

This firmware is based on modifications and merges of multiple open-source firmware, featuring the most diverse
functions:
* **Larger EEPROM capacity**
* **Automatic Doppler frequency shift**
* Custom boot logo
* **SI4732 support**
* **Chinese/English support**
* **Chinese input method**
* **GB22312 Chinese interface, channels**
* **Spectrum graph**
* **MDC1200 signaling, contacts**
* **SMS**
* **Signal strength indicator (S meter)**
* **One-touch frequency scanning**
* **Radio receiver**
* **AM fix**
* **SSB demodulation**


# Operating Instructions (Mandatory Reading!!)

| Key              | Function                                                                                                                                                 |
|-----------------|----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 🐤 **Main Interface** |                                                                                                                                                          |
| **Single Press `Up/Down`** | Adjust frequency (step size is set by menu item `Step Frequency`)                                                                                        |
| **Single Press `Number`** | Quickly input frequency in frequency mode                                                                                                                |
| **Single Press `*`** | Input DTMF to be sent (`A, B, C, D, *, #` correspond to `M, Up, Down, *, F` respectively. Side Key 1 acts as backspace, press PTT key to send)           |
| **Long Press `F`** | Keyboard Lock                                                                                                                                            |
| **Long Press `M`** | Switch modulation mode                                                                                                                                   |
| **Long Press `*`** | In channel mode, activates search list, multiple long presses toggle between lists (1/2/All). In frequency mode, initiates search from current frequency |
| **Long Press `0`/`F+0`** | Open/Close radio receiver(OR SI4732)                                                                                                                     |
| **Long Press `1`/`F+1`** | In channel mode, copies current channel to another VFO                                                                                                   |
| **Long Press `2`/`F+2`** | Switch between A/B channels                                                                                                                              |
| **Long Press `3`/`F+3`** | Switch between frequency/channel                                                                                                                         |
| **Long Press `4`/`F+4`** | One-touch frequency alignment                                                                                                                            |
| **Long Press `5`** | In channel mode, toggles search list                                                                                                                     |
| **Long Press `5`** | In frequency mode, sets search frequency range (from channel A to channel B frequency), press * key to start search                                      |
| **`F+5`** | Spectrum                                                                                                                                                 |
| **Long Press `6`/`F+6`** | Switch transmit power                                                                                                                                    |
| **Long Press `7`/`F+7`** | Voice-activated transmission switch                                                                                                                      |
| **Long Press `8`/`F+8`** | One-touch reverse frequency                                                                                                                              |
| **Long Press `9`/`F+9`** | One-touch call                                                                                                                                           |
| **`F+M`** | Open SMS                                                                                                                                                 |
| **`F+UP`** | Key tone switch                                                                                                                                          |
| **`F+Down`** | Automatic Doppler shift                                                                                                                                  |
| **`F+EXIT`** | Inverts menu navigation (Up/Down)                                                                                                                        |
| **`F+*`** | Scan (Digital/Analog) sub-audio                                                                                                                          |
| **Short Press Side Key 1** | Monitor                                                                                                                                                  |
| **Long Press Side Key 1** | DTMF decoding switch                                                                                                                                     |
| **Short Press Side Key 2** | Set wide/narrow band                                                                                                                                     |
| **Long Press Side Key 2** | Flashlight                                                                                                                                               |
|**Wide/Narrow Band, DTMF decoding, FM/AM/USB Switching**| Integrated into custom **Side Key and M**                                                                                                                |
| 🎤 **SI4732 Radio**          |                                                      |
| **Short press `Side Key 1`, Short press `Side Key 2`** | Change BFO in SSB mode                                      |
| **Short press `5`**                  | Enter frequency, **short press `*`** for decimal point, **short press `MENU`** to confirm                 |
| **Short press `0`**                  | Switch mode (AM/FM/SSB), **short press `F`** to switch LSB/USB                  |
| **Short press `1`, Short press `7`**        | Change step frequency                                               |
| **Short press `4`**                  | Toggle signal strength display                                             |
| **Short press `6`**                  | Change bandwidth                                                 |
| **Short press `2`, Short press `8`**        | Toggle ATT                                                |
| **Short press `3`, Short press `9`**        | Search up/down, **short press `EXIT`** to stop search                                       |
| 🔑 **Doppler Mode**               |                                                      |
| **Short press `5`**                  | Enter time, **short press `*`** for decimal point, **short press `MENU`** to confirm                 |
| **Short press `MENU`**               | Toggle parameters, adjust up/down                                            |
| **Short press `PPT`**                | Transmit                                                   |
| **Short press `Side Key 1`**                | Enable listening                                                 |
                                                                                                                           |
# Eeprom Layout Explanation

| Eeprom Address                          | Description                                                                                                                                             |
|----------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------|
| 😭 **General**                          | Version: LOSEHUxxx                                                                                                                                      |
| 0X01D00~0x02000                        | Rarely changed                                                                                                                                          |
| 0X01D00 ~ 0X01E00<br/>0X1F90 ~ 0X01FF0 | **MDC1200** - 22 MDC contacts<br/>Each contact occupies 16B, with the first 2B being MDC ID and the next 14B being contact name                         |
| 0X01FFF                                | **MDC1200** - Number of MDC contacts                                                                                                                    |
| 0x01FFD~0x01FFE                        | **MDC1200** - MDC ID                                                                                                                                    |
| 0x01FF8~0x01FFC                        | Side key functions                                                                                                                                      |
| 0x01FFD~0x01FFE                        | **MDC1200** - MDC ID                                                                                                                                    |
| 😱 **Expanded Version (K, H)**          | Version: LOSEHUxxxK, LOSEHUxxxH                                                                                                                         |
| 0x02000~0x02012                        | Boot character 1                                                                                                                                        |
| 0x02012~0x02024                        | Boot character 2                                                                                                                                        |
| 0x02024~0x02025                        | Length of boot characters 1 and 2                                                                                                                       |
| 0x02080~0x02480                        | Boot screen, length 128 (width) * 64/8 = 1024 = 0x400                                                                                                   |
| 0x01FFD~0x01FFE                        | **MDC1200** - MDC ID                                                                                                                                    |
| 0x02480~0x0255C                        | gFontBigDigits, length 11 * 20 = 220 = 0XDC                                                                                                             |
| 0x0255C~0x0267C                        | gFont3x5, length 96 * 3 = 288 = 0X120                                                                                                                   |
| 0x0267C~0x028B0                        | gFontSmall, length 96 * 6 = 564 = 0X234                                                                                                                 |
| 0x028B0~0x02B96                        | Menu encoding, length 53 * 14 = 742 = 0X2E6                                                                                                             |
| 0x02BA0~0x02BA9                        | **Doppler** - Satellite names, with the first character first, up to 9 English characters, the last one being '\0'                                      |
| 0x02BAA~0x02BAF                        | **Doppler** - Year (tens and units), month, day, hour, minute, and second of start transit time                                                         |
| 0x02BB0~0x2BB5                         | **Doppler** - Year (tens and units), month, day, hour, minute, and second of departure time                                                             |
| 0x02BB6~0x02BB7                        | **Doppler** - Total transit time (seconds), with the low byte first and the high byte second                                                            |
| 0x02BB8~0x02BB9                        | **Doppler** - Transmitter sub-audio, with the low byte first and the high byte second                                                                   |
| 0x02BBA~0x02BBB                        | **Doppler** - Receiver sub-audio, with the low byte first and the high byte second                                                                      |
| 0x02C00~0x02C64                        | **Doppler** - CTCSS_Options, length 50 * 2 = 100 = 0x64                                                                                                 |
| 0x02C64~0x02D34                        | **Doppler** - DCS_Options, length 104 * 2 = 208 = 0xD0                                                                                                  |
| 0x02BBC~0X02BBF                        | **Doppler** - Difference between start transit time and UNIX timestamp of January 1, 2000, with the low byte first and the high byte second             |
| 0X02BC0~0X02BC5                        | **Doppler** - Year (tens and units), month, day, hour, minute, and second of current time                                                               |
| 0x02E00~0x1E1E6                        | GB2312 Chinese font library, total 6763 * 11 * 12/8 = 111590 = 0x1B3E6                                                                                  |
| 0x1E200~0x20000(MAX)                   | **Doppler** - 2*n (even) second satellite data, 8B per second, including uplink/downlink frequency/10, with the low byte first and the high byte second |
| 😰 **2Mib Expanded Version (H)**        | Version: LOSEHUxxxH                                                                                                                                     |
| 0x20000~0x26B00                        | **Chinese Input Method** - Pinyin index, corresponding number of characters, starting address of characters                                             |
| 0x26B00~0X2A330                        | **Chinese Input Method** - Pinyin Chinese character table                                                                                               |
| 0x3C228~0x40000                        | **SI4732**-patch，Length 0x3DD8，used to update SI4732 firmware                                                                                           |
| 0x3C210~0x3C21C                        | **SI4732**FM、AM、SSB Freq、Mode                                                                                                                           |
[Doppler Eeprom Layout Explanation](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/doc/多普勒eeprom详细说明.txt)

# Examples

<p float="left">
  <img src="/images/c1.JPG" width=300 />
  <img src="/images/c2.JPG" width=300 />
  <img src="/images/c3.JPG" width=300 />
  <img src="/images/c4.JPG" width=300 />
</p>

# User Function Customization

You can customize the firmware by enabling/disabling various compilation options.

| Compilation Option                     | Description                                                                                                                     |
|----------------------------------------|---------------------------------------------------------------------------------------------------------------------------------|
| 🧰 **Quansheng Basic Functions**       | [Quansheng Basic Functions](https://github.com/egzumer/uv-k5-firmware-custom)                                                   |
| ENABLE_UART                            | UART, without this, you cannot configure the radio via PC!                                                                      |
| ENABLE_AIRCOPY                         | AirCopy wireless copy                                                                                                           |
| ENABLE_FMRADIO                         | FM radio function                                                                                                               |
| ENABLE_NOAA                            | NOAA function (only useful in the US)                                                                                           |
| ENABLE_VOICE                           | Voice broadcast                                                                                                                 |
| ENABLE_VOX                             | VOX voice-controlled transmission                                                                                               |
| ENABLE_ALARM                           | TX alarm                                                                                                                        |
| ENABLE_PWRON_PASSWORD                  | Boot password                                                                                                                   |
| ENABLE_DTMF_CALLING                    | DTMF dialing function, call initiation, call reception, group call, contact list, etc.                                          |
| ENABLE_FLASHLIGHT                      | Enable top flashlight LED light (on, blink, SOS)                                                                                |
| ⌚ **Custom Module**                    |                                                                                                                                 |
| ENABLE_BIG_FREQ                        | Large font frequency display (similar to official Quansheng firmware)                                                           |
| ENABLE_KEEP_MEM_NAME                   | Keep channel name when saving memory channel                                                                                    |
| ENABLE_WIDE_RX                         | Receive full range from 18MHz to 1300MHz (although the front end/power amplifier is not designed for the entire range)          |
| ENABLE_TX_WHEN_AM                      | Allow TX when RX is set to AM (always FM)                                                                                       |
| ENABLE_F_CAL_MENU                      | Enable hidden frequency calibration menu for radio                                                                              |
| ENABLE_CTCSS_TAIL_PHASE_SHIFT          | Use standard CTCSS tail phase shift instead of the unique QS 55Hz tone method                                                   |
| ENABLE_BOOT_BEEPS                      | Provide audio feedback for users at startup, indicating the position of the volume knob                                         |
| ENABLE_SHOW_CHARGE_LEVEL               | Display battery charge level while radio is charging                                                                            |
| ENABLE_REVERSE_BAT_SYMBOL              | Mirror battery symbol in status bar (positive pole on right)                                                                    |
| ENABLE_NO_CODE_SCAN_TIMEOUT            | Disable 32-second CTCSS/DCS scan timeout (exit button instead of waiting for timeout to end scan)                               |
| ENABLE_AM_FIX                          | Dynamically adjust front-end gain in AM mode to help prevent AM demodulator saturation, temporarily ignore RSSI level on screen |
| ENABLE_SQUELCH_MORE_SENSITIVE          | Slightly increase squelch sensitivity                                                                                           |
| ENABLE_FASTER_CHANNEL_SCAN             | Increase channel scan speed, but also increase squelch sensitivity                                                              |
| ENABLE_RSSI_BAR                        | Enable RSSI bar graph level in dBm/Sn units, instead of small antenna symbol                                                    |
| ENABLE_AUDIO_BAR                       | Display audio bar level while transmitting                                                                                      |
| ENABLE_COPY_CHAN_TO_VFO                | Copy current channel setting to frequency mode. Long press `1 BAND` in channel mode                                             |
| ENABLE_SPECTRUM                        | Spectrum analyzer, activated by `F` + `5 NOAA`                                                                                  |
| ENABLE_REDUCE_LOW_MID_TX_POWER         | Reduce mid and low power settings even lower                                                                                    |
| ENABLE_BYP_RAW_DEMODULATORS            | Additional BYP (bypass?) and RAW demodulation options, proven not very useful, but available if you want to experiment          |
| ENABLE_SCAN_RANGES                     | Scan range mode for frequency scanning                                                                                          |
| ENABLE_BLOCK                           | EEPROM lock                                                                                                                     |
| ENABLE_WARNING                         | Beep prompt                                                                                                                     |
| ENABLE_CUSTOM_SIDEFUNCTIONS            | Custom side key function                                                                                                        |
| ENABLE_SIDEFUNCTIONS_SEND              | Custom side key function (side key transmit function)                                                                           |
| ENABLE_AUDIO_BAR_DEFAULT               | Default audio bar style                                                                                                         |
| 📡 **Automatic Doppler**               | [Automatic Doppler](https://github.com/losehu/uv-k5-firmware-custom)                                                            |
| ENABLE_DOPPLER                         | Automatic Doppler function                                                                                                      |
| 📧 **SMS**                             | [SMS](https://github.com/joaquimorg/uv-k5-firmware-custom)                                                                      |
| ENABLE_MESSENGER                       | Send and receive short text messages (button = `F` + `MENU`)                                                                    |
| ENABLE_MESSENGER_DELIVERY_NOTIFICATION | Send notification to sender if message received                                                                                 |
| ENABLE_MESSENGER_NOTIFICATION          | Play sound when message received                                                                                                |
| 📱 **MDC1200**                         | [MDC1200](https://github.com/OneOfEleven/uv-k5-firmware-custom)                                                                 |
| ENABLE_MDC1200                         | MDC1200 transmission function                                                                                                   |
| ENABLE_MDC1200_SHOW_OP_ARG             | MDC display head/tail parameter                                                                                                 |
| ENABLE_MDC1200_SIDE_BEEP               | MDC side tone                                                                                                                   |
| ENABLE_MDC1200_CONTACT                 | MDC contact                                                                                                                     |
| 🎛️ **DOCK**                           | [DOCK](https://github.com/nicsure/QuanshengDock)                                                                                |
| ENABLE_DOCK                            | Allow control of the radio via PC, no screen display!                                                                           |
| 🚫 **Debug**                           |                                                                                                                                 |
| ENABLE_AM_FIX_SHOW_DATA                | Display debug data for AM fix                                                                                                   |
| ENABLE_AGC_SHOW_DATA                   | Display ACG parameters                                                                                                          |
| ENABLE_UART_RW_BK_REGS                 | Added two extra commands to read and write BK4819 registers                                                                     |
| ⚠️ **Compilation Options**             |                                                                                                                                 |
| ENABLE_CLANG                           | Experimental, build with clang instead of gcc (if this option is enabled, LTO will be disabled)                                 |
| ENABLE_SWD                             | Use the CPU's SWD port, required for debugging/programming                                                                      |
| ENABLE_OVERLAY                         | CPU FLASH-related content, not needed                                                                                           |
| ENABLE_LTO                             | Reduce the size of the compiled firmware, but may break EEPROM reading (OVERLAY will be disabled after enabling)                |

# Donations

If this project has been helpful to you, consider sponsoring to support development work.

[Donation List](https://losehu.github.io/payment-codes/#%E6%94%B6%E6%AC%BE%E7%A0%81) Thank you very much for
your support!!!

Donation Codes:

[![Donation Codes](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/show.png)](https://losehu.github.io/payment-codes/)


## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=losehu/uv-k5-firmware-custom&type=Date)](https://star-history.com/#losehu/uv-k5-firmware-custom&Date)

