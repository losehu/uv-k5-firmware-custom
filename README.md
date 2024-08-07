**Read this in other languages: [English](./README_en.md), [中文](./README.md).**

**语言版本: [English](./README_en.md), [中文](./README.md).**


# [K5Web]( https://k5.vicicode.com/)
* 支持在线固件功能编译，无需安装编译环境！！
* 多普勒卫星、开机图片文字、SI4732 SSB补丁的写频方式！
* 支持**创意工坊**，注册后登录可上传自定义固件和开机图片！！！

请访问：[K5Web]( https://k5.vicicode.com/)

# [自定义引导](https://github.com/losehu/uv-k5-bootloader-custom)
* 通过创立一个引导程序加载进RAM实现固件切换
* 可切换任意固件
* 目前仅适用于4Mib的Eeprom，通过修改代码可轻松拓展至其他大小Eeprom

# 版本说明

* 目前分为如下几个版本：**LOSEHUxxx**、**LOSEHUxxxK**、**LOSEHUxxxH**、**LOSEHUxxxE**、**LOSEHUxxxEK**、**LOSEHUxxxHS**
* **LOSEHUxxx**:中文固件，无需扩容，包含MDC1200、频谱、收音机
* **LOSEHUxxxK**:中文固件，需要1Mib以上Eeprom，包含多普勒模式、MDC1200、频谱、收音机、中文信道名、自定义开机中文字符、开机图片
* **LOSEHUxxxH**:中文固件，需要2Mib以上Eeprom，包含多普勒模式、中文输入法、频谱、收音机、中文信道名、自定义开机中文字符、开机图片
* **LOSEHUxxxHS**:中文固件，需要2Mib以上Eeprom，包含SI4732收音机、中文输入法、频谱、中文信道名、自定义开机中文字符、开机图片
* **LOSEHUxxxE**:英文固件，无需扩容，包含MDC1200、频谱、收音机、短信
* **LOSEHUxxxEK**:英文固件，需要1Mib以上Eeprom，包含多普勒模式、MDC1200、频谱、收音机、自定义开机字符、开机图片


# 多功能的K5/6固件

该固件基于多个开源固件修改合并，拥有最多样性的功能

* **更大容量的Eeprom芯片**
* **自动多普勒频移**
* **自定义开机图**
* **SI4732支持**
* **中/英文支持**
* **中文输入法**
* **GB22312中文界面、信道**
* **频谱图**
* **MDC1200信令、联系人**
* **短信**
* **信号强度指示器（ S表 ）**
* **一键扫频**
* **收音机**
* **AM 修复**
* **SSB 解调**

# 操作说明(必读！！)

| 按键                         | 功能                                                   |
|----------------------------|------------------------------------------------------|
| 🐤 **主界面下**                |                                                      |
| **单按`上/下`**                | 调整频率（步长为菜单1项`步进频率`）                                  |
| **单按`数字`**                 | 在频率模式下快捷输入频率                                         |
| **单按`*`**                  | 输入要发送的DTMF(`A、B、C、D、*、#`对应`M、上、下、*、F`键侧键1退格,按PPT键发送) |
| **长按`F`**                  | 锁定键盘                                                 |
| **长按`M`**                  | 切换调制模式                                               |
| **长按`*`**                  | 信道模式下是搜索列表,多次长按可切换(列表1/2/全部)，频率模式下,从当前频率开始搜索         |
| **长按`0`/`F+0`**            | 打开/关闭收音机(或SI4732)                                    |
| **长按`1`/`F+1`**            | 在信道模式下将当前信道复制到另一个VFO                                 |
| **长按`2`/`F+2`**            | 切换A/B通道                                              |
| **长按`3`/`F+3`**            | 切换频率/信道                                              |
| **长按`4`/`F+4`**            | 一键对频                                                 |
| **长按`5`**                  | 信道模式下切换搜索列表                                          |
| **长按`5`**                  | 频率模式下设置搜索频率范围(从通道A到通道B频率),按*键开始搜索                    |
| **`F+5`**                  | 频谱                                                   |
| **长按`6`/`F+6`**            | 切换发射功率                                               |
| **长按`7`/`F+7`**            | 声控发射开关                                               |
| **长按`8`/`F+8`**            | 一键倒频                                                 |
| **长按`9`/`F+9`**            | 一键即呼                                                 |
| **`F+M`**                  | 打开短信                                                 |
| **`F+UP`**                 | 按键音开关                                                |
| **`F+Down`**               | 自动多普勒                                                |
| **`F+EXIT`**               | 菜单上下颠倒                                               |
| **`F+*`**                  | 扫描(数字/模拟)亚音                                          |
| **短按`侧键1`**                | 监听                                                   |
| **长按`侧键1`**                | DTMF解码开关                                             |
| **短按`侧键2`**                | 设置宽窄带                                                |
| **长按`侧键2`**                | 手电筒                                                  |
| **宽窄带、DTMF解码、切换FM/AM/USB** | 集成至自定义的 **侧键与M**                                     |
| 🎤 **SI4732收音机**           |                                                      |
| **短按`侧键1`、短按`侧键2`**        |      SSB模式下更改bfo                                                 |
| **短按`5`**                  | 输入频率，**短按`*`** 小数点 , **短按`MENU`** 确认                 |
| **短按`0`**                  | 切换模式(AM/FM/SSB)，**短按`F`** 切换LSB/USB                  |
| **短按`1`**、**短按`7`**        | 切换步进频率                                               |
| **短按`4`**                  | 切换显示信号强度                                             |
| **短按`6`**                  | 切换带宽                                                 |
| **短按`2`**、**短按`8`**        | 切换ATT                                                |
| **短按`3`**、**短按`9`**        |     上下搜索 ，**短按`EXIT`** 停止搜索                                       |
| 🔑 **多普勒模式**               |                                                      |
| **短按`5`**                  | 输入时间，**短按`*`** 小数点 , **短按`MENU`** 确认                 |
| **短按`MENU`**               | 切换参数，上下调节                                            |
| **短按`PPT`**                | 发射                                                   |
| **短按`侧键1`**                | 开启监听                                                 |




# Eeprom分布说明

| Eeprom地址                               | 描述                                                        |
|----------------------------------------|-----------------------------------------------------------|
| 😭 **通用**                              | 版本号：LOSEHUxxx                                             |
| 0X01D00~0x02000                        | 基本不变                                                      |
| 0X01D00 ~ 0X01E00<br/>0X1F90 ~ 0X01FF0 | **MDC1200**-22个MDC联系人<br/>每个联系人占用16B，前2B为MDC ID，后14B为联系人名 |
| 0X01FFF                                | **MDC1200**-MDC联系人数量                                      |
| 0x01FFD~0x01FFE                        | **MDC1200**-MDC ID                                        |
| 0x01FF8~0x01FFC                        | 侧键功能                                                      |
| 0x01FFD~0x01FFE                        | **MDC1200**-MDC ID                                        |
| 😱 **扩容版(K、H)**                        | 版本号：LOSEHUxxxK、LOSEHUxxxH                                 |
| 0x02000~0x02012                        | 开机字符1                                                     |
| 0x02012~0x02024                        | 开机字符2                                                     |
| 0x02024~0x02025                        | 开机字符1、2的长度                                                |
| 0x02080~0x02480                        | 开机画面，长度128（宽）*64/8=1024=0x400                             |
| 0x01FFD~0x01FFE                        | **MDC1200**-MDC ID                                        |
| 0x02480~0x0255C                        | gFontBigDigits，长度11*20=220=0XDC                           |
| 0x0255C~0x0267C                        | gFont3x5，长度96*3=288=0X120                                 |
| 0x0267C~0x028B0                        | gFontSmall，长度96*6=564=0X234                               |
| 0x028B0~0x02B96                        | 菜单编码，长度53*14=742=0X2E6                                    |
| 0x02BA0~0x02BA9                        | **多普勒**-卫星名称,首字符在前,最多9个英文，最后一个为'\0'                       |
| 0x02BAA~0x02BAF                        | **多普勒**-开始过境时间的年份十位个位、月、日、时、分、秒                           |
| 0x02BB0~0x2BB5                         | **多普勒**-离境时间的年份十位个位、月、日、时、分、秒                             |
| 0x02BB6~0x02BB7                        | **多普勒**-总过境时间（秒），低位在前，高位在后                                |
| 0x02BB8~0x02BB9                        | **多普勒**-手台的发射亚音，低位在前，高位在后                                 |
| 0x02BBA~0x02BBB                        | **多普勒**-手台的接收亚音，低位在前，高位在后                                 |
| 0x02C00~0x02C64                        | **多普勒**-CTCSS_Options,长度50*2=100=0x64                     |
| 0x02C64~0x02D34                        | **多普勒**-DCS_Options,长度104*2=208=0xD0                      |
| 0x02BBC~0X02BBF                        | **多普勒**-开始过境时间与2000年1月1日UNIX时间戳的差,低位在前，高位在后               |
| 0X02BC0~0X02BC5                        | **多普勒**-当前时间的年份十位个位、月、日、时、分、秒                             |
| 0x02E00~0x1E1E6                        | GB2312中文字库,共6763*11*12/8=111590=0x1B3E6                   |
| 0x1E200~0x20000(MAX)                   | **多普勒**-第2*n（偶数）秒卫星数据，每秒8B,包括上下行频率/10，低位在前，高位在后           |
| 😰 **2Mib扩容版（H）**                      | 版本号：LOSEHUxxxH                                            |
| 0x20000~0x26B00                        | **中文输入法**-拼音索引、对应字数、字的起始地址                                |
| 0x26B00~0x2A330                        | **中文输入法**-拼音汉字表                                           |
| 0x3C228~0x40000                        | **SI4732**-patch，长度为0x3DD8，用于SI4732的固件升级                  |
| 0x3C210~0x3C21C                        | **SI4732**FM、AM、SSB频率、模式                                  |
[多普勒Eeprom分布说明](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/doc/多普勒eeprom详细说明.txt)


# 用户功能自定义

你可以通过启用/禁用各种编译选项来定制固件

| 编译选项                                   | 描述                                                                            |
|----------------------------------------|-------------------------------------------------------------------------------|
| 🧰 **泉盛基本功能**                          | [Quansheng Basic Functions](https://github.com/egzumer/uv-k5-firmware-custom) |
| ENABLE_UART                            | 串口，没有这个,你就不能通过PC配置无线电！                                                        |
| ENABLE_AIRCOPY                         | AirCopy无线复制                                                                   |
| ENABLE_FMRADIO                         | 收音机功能                                                                         |
| ENABLE_NOAA                            | NOAA功能 (只有在美国有用)                                                              |
| ENABLE_VOICE                           | 语音播报                                                                          |
| ENABLE_VOX                             | VOX声控发射                                                                       |
| ENABLE_ALARM                           | TX 警报                                                                         |
| ENABLE_PWRON_PASSWORD                  | 开机密码                                                                          |
| ENABLE_DTMF_CALLING                    | DTMF拨号功能，呼叫发起，呼叫接收，群组通话，联系人列表等                                                |
| ENABLE_FLASHLIGHT                      | 启用顶部手电筒LED灯（开启，闪烁，SOS）                                                        |
| ⌚ **自定义模组**                            |                                                                               |
| ENABLE_BIG_FREQ                        | 大号字体的频率显示（类似官方泉盛固件）                                                           |
| ENABLE_KEEP_MEM_NAME                   | 在重新保存内存频道时保持频道名称                                                              |
| ENABLE_WIDE_RX                         | 全频18MHz至1300MHz接收（尽管前端/功率放大器未设计用于整个范围）                                        |
| ENABLE_TX_WHEN_AM                      | 当RX设置为AM时允许TX（始终为FM）                                                          |
| ENABLE_F_CAL_MENU                      | 启用收音机的隐藏频率校准菜单                                                                |
| ENABLE_CTCSS_TAIL_PHASE_SHIFT          | 使用标准CTCSS尾部相移，而不是QS独有的55Hz音调方法                                                |
| ENABLE_BOOT_BEEPS                      | 在启动时为用户提供音频反馈，指示音量旋钮的位置                                                       |
| ENABLE_SHOW_CHARGE_LEVEL               | 在收音机充电时显示电池充电水平                                                               |
| ENABLE_REVERSE_BAT_SYMBOL              | 在状态栏上镜像电池符号（正极在右侧）                                                            |
| ENABLE_NO_CODE_SCAN_TIMEOUT            | 禁用32秒CTCSS/DCS扫描超时（按退出按钮而不是等待超时结束扫描                                           |
| ENABLE_AM_FIX                          | 在AM模式下动态调整前端增益，以帮助防止AM解调器饱和，暂时忽略屏幕上的RSSI级别                                    |
| ENABLE_SQUELCH_MORE_SENSITIVE          | 将静噪电平稍微调敏感一些                                                                  |
| ENABLE_FASTER_CHANNEL_SCAN             | 增加频道扫描速度，但静噪调敏度也增加了                                                           |
| ENABLE_RSSI_BAR                        | 启用以dBm/Sn为单位的RSSI条形图水平，取代小天线符号                                                |
| ENABLE_AUDIO_BAR                       | 发送时显示音频条级别                                                                    |
| ENABLE_COPY_CHAN_TO_VFO                | 将当前频道设置复制到频率模式。在频道模式下长按  `1 BAND`                                             |
| ENABLE_SPECTRUM                        | fagci 频谱分析仪，`F` + `5 NOAA`激活                                                  |
| ENABLE_REDUCE_LOW_MID_TX_POWER         | 使中等和低功率设置更低                                                                   |
| ENABLE_BYP_RAW_DEMODULATORS            | 额外的BYP（旁路？）和RAW解调选项，被证明并不十分有用，但如果你想实验的话，它是存在的                                 |
| ENABLE_SCAN_RANGES                     | 频率扫描的扫描范围模式                                                                   |
| ENABLE_BLOCK                           | Eeprom上锁                                                                      |
| ENABLE_WARNING                         | 	    BEEP提示音                                                                  |
| ENABLE_CUSTOM_SIDEFUNCTIONS            | 自定义侧键功能                                                                       |
| ENABLE_SIDEFUNCTIONS_SEND              | 自定义侧键功能（侧键发射功能）                                                               |
| ENABLE_AUDIO_BAR_DEFAULT               | 默认语音条样式                                                                       |
| 📡 **自动多普勒**                           | [Automatic Doppler](https://github.com/losehu/uv-k5-firmware-custom)          |
| ENABLE_DOPPLER                         | 自动多普勒功能                                                                       |
| 📧 **短信**                              | [SMS](https://github.com/joaquimorg/uv-k5-firmware-custom)                    |
| ENABLE_MESSENGER                       | 发送和接收短文本消息（按键 = `F` + `MENU`）                                                 |
| ENABLE_MESSENGER_DELIVERY_NOTIFICATION | 如果收到消息，则向发送方发送通知                                                              |
| ENABLE_MESSENGER_NOTIFICATION          | 在收到消息时播放声音                                                                    |
| 📱 **MDC1200**                         | [MDC1200](https://github.com/OneOfEleven/uv-k5-firmware-custom)               |
| ENABLE_MDC1200                         | MDC1200发送功能                                                                   |
| ENABLE_MDC1200_SHOW_OP_ARG             | MDC显示首尾音参数                                                                    |
| ENABLE_MDC1200_SIDE_BEEP               | MDC侧音                                                                         |
| ENABLE_MDC1200_CONTACT                 | MDC联系人                                                                        |
| 🎛️ **DOCK**                           | [DOCK](https://github.com/nicsure/QuanshengDock)                              |
| ENABLE_DOCK                            | 允许通过电脑控制手台，无屏幕显示！                                                             |
| 🚫 **调试**                              |                                                                               |
| ENABLE_AM_FIX_SHOW_DATA                | 显示AM修复的调试数据                                                                   |
| ENABLE_AGC_SHOW_DATA                   | 显示ACG参数                                                                       |
| ENABLE_UART_RW_BK_REGS                 | 添加了两个额外的命令，允许读取和写入BK4819寄存器                                                   |
| ⚠️ **编译选项**                            |                                                                               |
| ENABLE_CLANG                           | 实验性质，使用clang而不是gcc构建（如果启用此选项，LTO将被禁用）                                         |
| ENABLE_SWD                             | 使用CPU的SWD端口，调试/编程时需要                                                          |
| ENABLE_OVERLAY                         | CPU FLASH相关内容，不需要                                                             |
| ENABLE_LTO                             | 减小编译固件的大小，但可能会破坏EEPROM读取（启用后OVERLAY将被禁用）                                      |

# 打赏

如果这个项目对您有帮助,可以考虑赞助来支持开发工作。

这是：[打赏名单](https://losehu.github.io/payment-codes/#%E6%94%B6%E6%AC%BE%E7%A0%81) 非常感谢各位的支持！！！

打赏码：

[![打赏码](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/show.png)](https://losehu.github.io/payment-codes/)


## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=losehu/uv-k5-firmware-custom&type=Date)](https://star-history.com/#losehu/uv-k5-firmware-custom&Date)

