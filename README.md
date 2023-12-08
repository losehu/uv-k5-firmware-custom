
# 快捷键操作
* 按**侧键1后开机**打开隐藏菜单
* **长按M**切换调制模式
* **F+M**按键音开关切换
* **F+5频谱**
* **短按侧键1**监听
* **长按侧键1**DTMF解码开关
* **短按侧键2**设置宽窄带
* **长按侧键2**手电筒
* **长按6**切换发射功率
* **长按5**切换搜索列表

# 主要功能：
* 中文菜单
* 按**侧键1**后开机打开隐藏菜单
* 许多来自 OneOfEleven 的模块：
  * **1o11 MDC信令**（实现中）
  * AM 修复，显著提高接收质量
  * 长按按钮执行 `F+` 操作的功能复制
  * 快速扫描
  * 菜单中的频道名称编辑
  * 频道名称 + 频率显示选项
  * 扫描列表分配的快捷方式（长按 `5 NOAA`）
  * 扫描时的扫描列表切换（在扫描时长按 `* Scan`）
  * 从菜单中可选择的可配置按钮功能
  * 状态栏上的电池百分比/电压，可从菜单中选择
  * 更长的背光时间
  * 麦克风条
  * 信号强度指示器（RSSI s-meter）
  * 更多的频率步进
  * 静噪更为敏感
* fagci 频谱分析仪（**F+5** 打开）
* SSB 解调（从 fagci 采用）
* 背光调暗
* 来自菜单的电池电压校准
* 更好的电池百分比计算，可选择适用于 1600mAh 或 2200mAh
* 更多可配置的按钮功能
* 长按 MENU 作为另一个可配置按钮
* 菜单中的更好的 DCS/CTCSS 扫描（在 RX DCS/CTCSS 菜单项中按 `* SCAN`）
* Piotr022 信号强度指示器样式
* 使用 EXIT 停止扫描时还原初始频率/频道，使用 MENU 按钮记住上次找到的传输
* 重新排序并重命名菜单条目
* 修复 LCD 干扰崩溃问题



# 示例

<p float="left">
  <img src="/images/c1.JPG" width=300 />
  <img src="/images/c2.JPG" width=300 />
  <img src="/images/c3.JPG" width=300 />
  <img src="/images/c4.JPG" width=300 />
</p>


# 用户功能自定义

你可以通过启用/禁用各种编译选项来定制固件，这允许我们删除某些固件功能，以便在闪存中为其他功能腾出空间。你会在 "Makefile" 的顶部找到这些选项（'0' = 禁用，'1' = 启用）... 

（请注意：以下内容是一个示例，实际的 "Makefile" 可能会有所不同） 

```
ENABLE_CLANG                  := 0     **实验性的，使用 clang 而不是 gcc 构建（如果启用此功能，LTO 将被禁用）
ENABLE_SWD                    := 0       仅在使用 CPU 的 SWD 端口（调试/编程）时才需要）
ENABLE_OVERLAY                := 0       CPU 闪存的东西，不需要
ENABLE_LTO                    := 0     **实验性的，减小了已编译固件的大小，但可能会破坏 EEPROM 读取（如果启用此功能，则 OVERLAY 将被禁用）
ENABLE_UART                   := 1       没有这个，你就不能通过PC配置无线电！
ENABLE_AIRCOPY                := 0       AirCopy无线复制
ENABLE_FMRADIO                := 0       收音机功能
ENABLE_NOAA                   := 0       NOAA功能
ENABLE_VOICE                  := 0       语音提示
ENABLE_VOX                    := 0       VOX
ENABLE_ALARM                  := 0       TX 警报
ENABLE_1750HZ                 := 0       侧键 1750Hz TX 音（旧中继器访问）
ENABLE_PWRON_PASSWORD         := 1       开机密码
ENABLE_BIG_FREQ               := 0       频率模式大字体
ENABLE_SMALL_BOLD             := 1       粗体通道名称/编号（当名称+频率频道显示模式时）
ENABLE_KEEP_MEM_NAME          := 1       在（重新）保存信道时保持信道名称
ENABLE_WIDE_RX                := 1       18MHz 至 1300MHz 接收（尽管前端/PA 不是为全范围设计的）
ENABLE_TX_WHEN_AM             := 0       当 RX 设置为 AM 时允许 FM 发射
ENABLE_F_CAL_MENU             := 0       启用隐藏频率校准菜单
ENABLE_CTCSS_TAIL_PHASE_SHIFT := 1       标准的 CTCSS 尾部相移，而不是泉盛自己的 55Hz 音调方法
ENABLE_BOOT_BEEPS             := 0       在启动时为用户提供有关音量旋钮位置的音频反馈
ENABLE_SHOW_CHARGE_LEVEL      := 0       显示收音机充电时的充电情况
ENABLE_REVERSE_BAT_SYMBOL     := 1       镜像状态栏上的电池符号（正极在右侧） 
ENABLE_NO_CODE_SCAN_TIMEOUT   := 1       禁用 32 秒 CTCSS/DCS 扫描超时（按退出按钮而不是超时结束扫描） 
ENABLE_AM_FIX                 := 1       在AM模式下动态调整前端增益，以防止AM解调器饱和，忽略屏幕上的RSSI电平（暂时） 
ENABLE_AM_FIX_SHOW_DATA       := 1       显示 AM 修复的调试数据（仍在调整） 
ENABLE_SQUELCH_MORE_SENSITIVE := 0       使静噪电平更灵敏一点 - 我计划让用户自己调整值
ENABLE_FASTER_CHANNEL_SCAN    := 0       提高信道扫描速度，但静噪也变得更加抽搐
ENABLE_RSSI_BAR               := 1       启用 dBm/Sn RSSI 条形图电平代替小天线符号 
ENABLE_AUDIO_BAR              := 0       实验性，当发射时显示音频条电平
ENABLE_COPY_CHAN_TO_VFO       := 1       在通道模式下长按“1 BAND” 将当前通道复制到另一个 VFO . 
ENABLE_SPECTRUM               := 1       fagci 频谱分析仪，用`F`+`5 NOAA`激活 
ENABLE_REDUCE_LOW_MID_TX_POWER:= 0       使中低功率更低
ENABLE_BYP_RAW_DEMODULATORS   := 0       额外的 BYRP（旁路？） 和 RAW 解调选项，被证明不是很有用，但如果您想尝试
ENABLE_BLMIN_TMP_OFF          := 0       可配置按钮的附加功能，可打开和关闭`BLMin`，并将其保存到 EEPROM
```

# 打赏

如果这个项目对您有帮助，可以考虑赞助来支持开发工作。

这是：[打赏名单](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/sponsors.md) 非常感谢各位对汉化工作的支持！！！

打赏码：

[![打赏码](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/show.png)](https://github.com/losehu/uv-k5-firmware-chinese/blob/main/payment/payment-codes.md)

# 全新的中文固件

这个仓库是 OneOfEleven、fagci、egzumer三个版本的合并，并加入了我的修改

[https://github.com/egzumer/uv-k5-firmware-custom](https://github.com/egzumer/uv-k5-firmware-custom)  
[https://github.com/OneOfEleven/uv-k5-firmware-custom](https://github.com/OneOfEleven/uv-k5-firmware-custom)  
[https://github.com/fagci/uv-k5-firmware-fagci-mod/tree/refactor](https://github.com/fagci/uv-k5-firmware-fagci-mod/tree/refactor)

所有这些都是 DualTachyon 的开源固件的克隆和定制版本，可以在以下位置找到：

[https://github.com/DualTachyon/uv-k5-firmware](https://github.com/DualTachyon/uv-k5-firmware) .. 一个很酷的成果！

在使用此固件时，请自担风险（全部）。绝对不能保证它在任何情况下都能在你的电台上正常工作，它甚至可能会使你的电台变砖，如果出现这种情况，你可能需要购买另一台电台。

总之，玩得开心。

# 免责声明：
* **如若需要验机，完全不建议刷写自定义固件！！！！！！！！！！！！！！**
* 本固件仅供技术交流和个人学习使用。任何个人或组织在使用本固件时必须**遵守中华人民共和国相关法律法规及无线电管理条例**。
* 作者对于他人使用本固件所产生的任何违法行为**概不负责**，包括但不限于未经授权擅自修改、使用本固件进行非法活动等行为。
* 如因使用本固件造成的损失，**作者不承担任何法律责任**。
* 使用者在下载、安装和使用本固件时，默认已经阅读、理解并同意本免责声明的所有内容。**如有异议，请立即停止使用本固件**。
* 作者保留在法律允许范围内对本免责声明进行解释和修改的权利。**任何未经授权的固件修改、传播所造成的违法行为，一概与作者无关**。

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



