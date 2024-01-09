
# 操作说明(必读！！)
**主界面下**：
* **单按`上/下`**：调整频率（步长为菜单1项`步进频率`）
* **单按`数字`**：在频率模式下快捷输入频率
* **单按`*`**：输入要发送的DTMF(`A、B、C、D、*、#`对应`M、上、下、*、F`键侧键1退格,按PPT键发送)
* **长按`F`**：锁定键盘
* **长按`M`**：切换调制模式
* **长按`*`**：信道模式下是搜索列表,多次长按可切换(列表1/2/全部)
              频率模式下,从当前频率开始搜索
* **长按`0`/`F+0`**：打开/关闭收音机
* **长按`1`/`F+1`**：在信道模式下将当前信道复制到另一个VFO
* **长按`2`/`F+2`**：切换A/B通道
* **长按`3`/`F+3`**：切换频率/信道
* **长按`4`/`F+4`**：一键对频
* **长按`5`**：信道模式下切换搜索列表
* **长按`5`**：频率模式下设置搜索频率范围(从通道A到通道B频率),按*键开始搜索
* **`F+5`**：频谱
* **长按`6`/`F+6`**：切换发射功率
* **长按`7`/`F+7`**：声控发射开关
* **长按`8`/`F+8`**：一键倒频
* **长按`9`/`F+9`**：一键即呼
* **`F+M`**：按键音开关切换
* **`F+*`**：扫描(数字/模拟)亚音
* **短按`侧键1`**：监听
* **长按`侧键1`**：DTMF解码开关
* **短按`侧键2`**：设置宽窄带
* **长按`侧键2`**：手电筒





# 主要功能：
* 中文菜单
* **频谱分析仪**（`F+5`）
* **MDC信令**
* **MDC联系人**
我的MDC ID: **542B**希望大家不要和我抢，谢谢大家
* 信号强度指示器（ S表 ）
* 麦克风条
* 扫描列表分配的快捷方式（ 长按 `5` ）
* 扫描时的扫描列表切换（在扫描时长按 `*`
* AM 修复,显著提高接收质量
* 快速扫描
* 长按按钮执行 `F+` 操作的功能复制
* 菜单中的频道名称编辑
* 频道名称 + 频率显示选项
* 从菜单中可选择的可配置按钮功能
* 状态栏上的电池百分比/电压,可从菜单中选择
* 更长的背光时间
* 更多的频率步进
* 静噪更为敏感
* SSB 解调
* 来自菜单的电池电压校准
* 更好的电池百分比计算,可选择适用于 1600mAh 或 2200mAh
* 菜单中的更好的 DCS/CTCSS 扫描（在 RX DCS/CTCSS 菜单项中按 `*`）
* Piotr022 信号强度指示器样式
* 使用 EXIT 停止扫描时还原初始频率/频道,使用 MENU 按钮记住上次找到的传输


# Eeprom分布说明：
* **前8K**基本不变
* **0X1D00 ~ 0X1E00、0X1F90 ~ 0X1FF0** 22个MDC联系人，**0X1FFF**存放联系人数量
* 每个联系人占用16B，前2B为MDC ID，后14B为联系人名

  
**扩容版（K）**：
* **0x2000~1E31C**      中文字库（三个字库大小为：40960、40960、33564）
* **0x1E320~0x1E332**   开机字符1 ，**0xE31E** 存放长度
* **0x1E333~0x1E345**   开机字符2 ，**0x1E31F** 存放长度
* **0x1E350~0X1E450**   开机画面，128（宽）*64 单色图



# 示例

<p float="left">
  <img src="/images/c1.JPG" width=300 />
  <img src="/images/c2.JPG" width=300 />
  <img src="/images/c3.JPG" width=300 />
  <img src="/images/c4.JPG" width=300 />
</p>


# 用户功能自定义

你可以通过启用/禁用各种编译选项来定制固件。在 "Makefile" 的顶部找到这些选项（'0' = 禁用,'1' = 启用）

（请注意：有些功能已失效,标注`已失效！！`） 

```
ENABLE_CLANG                  := 0     **实验性的,使用 clang 而不是 gcc 构建（如果启用此功能,LTO 将被禁用）
ENABLE_SWD                    := 0       仅在使用 CPU 的 SWD 端口（调试/编程）时才需要）
ENABLE_OVERLAY                := 0       CPU 闪存的东西,不需要
ENABLE_LTO                    := 1     **实验性的,减小了已编译固件的大小,但可能会破坏 EEPROM 读取（如果启用此功能,则 OVERLAY 将被禁用）
ENABLE_UART                   := 1       没有这个,你就不能通过PC配置无线电！
ENABLE_AIRCOPY                := 0       AirCopy无线复制
ENABLE_FMRADIO                := 1       收音机功能
ENABLE_NOAA                   := 0       NOAA功能
ENABLE_VOICE                  := 0       语音提示
ENABLE_VOX                    := 1       VOX
ENABLE_ALARM                  := 0       TX 警报
ENABLE_1750HZ                 := 0       已失效！侧键 1750Hz TX 音（旧中继器访问）
ENABLE_PWRON_PASSWORD         := 0       开机密码
ENABLE_DTMF_CALLING           := 1
ENABLE_FLASHLIGHT             := 1
ENABLE_BIG_FREQ               := 1       频率模式大字体
ENABLE_KEEP_MEM_NAME          := 1       在（重新）保存信道时保持信道名称
ENABLE_WIDE_RX                := 1       18MHz 至 1300MHz 接收（尽管前端/PA 不是为全范围设计的）
ENABLE_TX_WHEN_AM             := 0       当 RX 设置为 AM 时允许 FM 发射
ENABLE_F_CAL_MENU             := 0       启用隐藏频率校准菜单
ENABLE_CTCSS_TAIL_PHASE_SHIFT := 0       标准的 CTCSS 尾部相移,而不是泉盛自己的 55Hz 音调方法
ENABLE_BOOT_BEEPS             := 0       在启动时为用户提供有关音量旋钮位置的音频反馈
ENABLE_SHOW_CHARGE_LEVEL      := 0       显示收音机充电时的充电情况
ENABLE_REVERSE_BAT_SYMBOL     := 0       镜像状态栏上的电池符号（正极在右侧） 
ENABLE_NO_CODE_SCAN_TIMEOUT   := 1       禁用 32 秒 CTCSS/DCS 扫描超时（按退出按钮而不是超时结束扫描） 
ENABLE_AM_FIX                 := 1       在AM模式下动态调整前端增益,以防止AM解调器饱和,忽略屏幕上的RSSI电平（暂时） 
ENABLE_SQUELCH_MORE_SENSITIVE := 1       使静噪电平更灵敏一点 - 我计划让用户自己调整值
ENABLE_FASTER_CHANNEL_SCAN    := 1       提高信道扫描速度,但静噪也变得更加抽搐
ENABLE_RSSI_BAR               := 1       启用 dBm/Sn RSSI 条形图电平代替小天线符号 
ENABLE_AUDIO_BAR              := 1       实验性,当发射时显示音频条电平
ENABLE_COPY_CHAN_TO_VFO       := 1       在信道模式下长按“1 BAND” 将当前信道复制到另一个 VFO . 
ENABLE_SPECTRUM               := 1       fagci 频谱分析仪,用`F`+`5 NOAA`激活 
ENABLE_REDUCE_LOW_MID_TX_POWER:= 0       使中低功率更低
ENABLE_BYP_RAW_DEMODULATORS   := 0       额外的 BYRP（旁路？） 和 RAW 解调选项,被证明不是很有用,但如果您想尝试
ENABLE_BLMIN_TMP_OFF          := 0       已失效！！可配置按钮的附加功能,可打开和关闭`BLMin`,并将其保存到 EEPROM
ENABLE_SCAN_RANGES            := 1       扫描范围
ENABLE_MDC1200                := 1       MDC1200发送功能
ENABLE_MDC1200_SHOW_OP_ARG    := 1       MDC显示首尾音参数
ENABLE_MDC1200_SIDE_BEEP      := 0       MDC侧音
ENABLE_MDC1200_CONTACT        := 1       MDC联系人，开启后ENABLE_MDC1200_SHOW_OP_ARG失效
ENABLE_AM_FIX_SHOW_DATA       := 0       显示 AM 修复的调试数据（仍在调整） 
ENABLE_AGC_SHOW_DATA          := 0       显示ACG参数
```

# 打赏

如果这个项目对您有帮助,可以考虑赞助来支持开发工作。

这是：[打赏名单](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/sponsors.md) 非常感谢各位对汉化工作的支持！！！

打赏码：

[![打赏码](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/show.png)](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/payment-codes.md)

# 全新的中文固件

这个仓库是 OneOfEleven、fagci、egzumer三个版本的合并,并加入了我的修改

[https://github.com/egzumer/uv-k5-firmware-custom](https://github.com/egzumer/uv-k5-firmware-custom)  
[https://github.com/OneOfEleven/uv-k5-firmware-custom](https://github.com/OneOfEleven/uv-k5-firmware-custom)  
[https://github.com/fagci/uv-k5-firmware-fagci-mod/tree/refactor](https://github.com/fagci/uv-k5-firmware-fagci-mod/tree/refactor)

所有这些都是 DualTachyon 的开源固件的克隆和定制版本,可以在以下位置找到：

[https://github.com/DualTachyon/uv-k5-firmware](https://github.com/DualTachyon/uv-k5-firmware) .. 一个很酷的成果！

在使用此固件时,请自担风险（全部）。 祝您使用愉快。

# 免责声明：
* **如若需要验机,完全不建议刷写自定义固件！！！！！！！！！！！！！！**
* 本固件仅供技术交流和个人学习使用。任何个人或组织在使用本固件时必须**遵守中华人民共和国相关法律法规及无线电管理条例**。
* 作者对于他人使用本固件所产生的任何违法行为**概不负责**,包括但不限于未经授权擅自修改、使用本固件进行非法活动等行为。
* 如因使用本固件造成的损失,**作者不承担任何法律责任**。
* 使用者在下载、安装和使用本固件时,默认已经阅读、理解并同意本免责声明的所有内容。**如有异议,请立即停止使用本固件**。
* 作者保留在法律允许范围内对本免责声明进行解释和修改的权利。**任何未经授权的固件修改、传播所造成的违法行为,一概与作者无关**。

# License

Copyright 2023 Dual Tachyon
https://github.com/DualTachyon

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.



