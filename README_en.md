**Read this in other languages: [English](README_en.md), [中文](README.md).**

**语言版本: [English](README_en.md), [中文](README.md).**

# some advice for HAM outside of China who are using the K5
**In China, about 2000 K5 devices have retained the larger-capacity EEPROM chip, such as the M24M02. We moved many constants to the EEPROM to gain more flash space.It works very well.**

**Replacing the EEPROM is relatively simple, and many people without prior experience have successfully completed the operation. However, please make sure to back up your calibration files!**

**By replacing it, you may be able to achieve:**
* Display and input in any language (implemented)
* Automatic Doppler frequency shift function (implemented)
* Custom boot-up images (implemented)
* Custom SSTV images (guess)
* ......
* More available flash allows for additional custom features(about 1.5K?).
# Multi-functional K5/6 Firmware

This firmware is based on modifications and merges of multiple open-source firmware, featuring the most diverse
functions:

* **Larger EEPROM chip**
* **Automatic Doppler Shift**
* **English Support (to be implemented...)**
* **Chinese Input Method (under implementation...)**
* **GB22312 Chinese Interface, Channels**
* **Spectrum Analyzer**
* **MDC1200 Signaling, Contacts**
* **SMS**
* **Signal Strength Indicator (S-meter)**
* **One-button Frequency Sweep**
* **FM Radio**
* **AM Fix**
* **SSB Demodulation**

# Operating Instructions (Must Read!!)

| Button                       | Function                                                                                                                                          |
|------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------|
| 🐤 **Main Interface**        |                                                                                                                                                   |
| **Single Press `Up/Down`**   | Adjust frequency (step size is menu 1 item `Frequency Step`)                                                                                      |
| **Single Press `Number`**    | Quick input frequency in frequency mode                                                                                                           |
| **Single Press `*`**         | Input DTMF to be sent (`A, B, C, D, *, #` corresponds to `M, Up, Down, *, F` keys respectively, side key 1 is backspace, press PPT key to send)   |
| **Long Press `F`**           | Lock keyboard                                                                                                                                     |
| **Long Press `M`**           | Switch modulation mode                                                                                                                            |
| **Long Press `*`**           | In channel mode, it is a search list, long press multiple times to switch (list 1/2/all); in frequency mode, starts search from current frequency |
| **Long Press `0`/`F+0`**     | Open/close radio                                                                                                                                  |
| **Long Press `1`/`F+1`**     | In channel mode, copy current channel to another VFO                                                                                              |
| **Long Press `2`/`F+2`**     | Switch A/B channels                                                                                                                               |
| **Long Press `3`/`F+3`**     | Switch frequency/channel                                                                                                                          |
| **Long Press `4`/`F+4`**     | One-button frequency alignment                                                                                                                    |
| **Long Press `5`**           | In channel mode, switch search list                                                                                                               |
| **Long Press `5`**           | In frequency mode, set search frequency range (from channel A to channel B frequency), press * to start search                                    |
| **`F+5`**                    | Spectrum                                                                                                                                          |
| **Long Press `6`/`F+6`**     | Switch transmit power                                                                                                                             |
| **Long Press `7`/`F+7`**     | Voice-activated transmission switch                                                                                                               |
| **Long Press `8`/`F+8`**     | One-button reverse frequency                                                                                                                      |
| **Long Press `9`/`F+9`**     | One-button instant call                                                                                                                           |
| **`F+M`**                    | Open SMS                                                                                                                                          |
| **`F+UP`**                   | Key tone switch                                                                                                                                   |
| **`F+Down`**                 | Automatic Doppler                                                                                                                                 |
| **`F+*`**                    | Scan (Digital/Analog) CTCSS/DCS                                                                                                                   |
| **Short Press `Side Key 1`** | Monitor                                                                                                                                           |
| **Long Press `Side Key 1`**  | DTMF decoding switch                                                                                                                              |
| **Short Press `Side Key 2`** | Set wide/narrow bandwidth                                                                                                                         |
| **Long Press `Side Key 2`**  | Flashlight                                                                                                                                        |

# EEPROM Distribution Description

[EEPROM Distribution Description](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/doc/README.md)

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

[Donation List](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/sponsors.md) Thank you very much for
your support!!!

Donation Codes:

[![Donation Codes](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/show.png)](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/payment-codes.md)

# Disclaimer:

* The author **does not assume any responsibility** for any illegal activities arising from the use of this firmware,
  including but not limited to unauthorized modification, use of the firmware for illegal activities, etc.
* The author **does not assume any legal responsibility** for any losses caused by the use of this firmware.
* Users who download, install, and use this firmware are deemed to have read, understood, and agreed to all the contents
  of this disclaimer by default. **If you have any objections, please stop using this firmware immediately**.
* The author reserves the right to interpret and modify this disclaimer within the scope permitted by law. **Any
  unauthorized firmware modification or distribution will not be the responsibility of the author**.