# PaceandLoveKeyboard
    受稚晖君启发，当时的铝厂的F96正好罢工，于是准备自己做一款键盘。
    改装的铝厂F96键盘
![](https://github.com/Marspacecraft/PaceandLoveKeyboard/blob/main/image/1.jpeg)    
    当时正好缅北内战，于是起名爱与和平键盘。
## 目录
    3D-freecad做的一些3D模型
    Frameware-键盘固件
        bootloader-引导固件
        PaceandLoveKeyboard-键盘固件
            CherryUSB-usbd代理，主要实现usb的hid和mcp的串口设别
            core-键盘主要代码
                Epaper，Oled，Fonts-调试的一些显示模块驱动
                Flash-外部flash，无法作为MSC设别，暂未调试成功
                joystick-摇杆驱动
                Keyboard-键盘相关代码
                Src-系统启动等相关代码
            Fatfs-文件系统。主要用于stm32内部flash和外部flash
            OBJ-键盘固件生成目录
            SFUD-外部flash驱动
            Ymodem-串口Ymodem协议，用于串口传输文件
    Hardware-嘉立创做的硬件电路（Flash处可能存在问题，无法做将外部flash做成MSC，问题目前没有解决，flash暂时没有使用）---本人纯属自学电路制作，电路图仅供参考。
    BLE-低功耗蓝牙，采用的是nrf52832，融合了nrf的两个demo，数据可以正常发送，但问题太多，以后有时间解决。

## 键盘原理
    keybit-物理按键编号
    keycode-hid报告的键盘键值
    keycombine-按键组合。一个keycombine对应一个keybit；keycombine由4个keycode组成。一个keycombine相当于同时按下数个键。
    hid报告描述
        8bit功能按键位
        8bit填充位
        6*8bit所有按键位
        104bit普通按键位
        5bit LED显示位
        3bit填充位

        共22byte，目前LED功能没有实现。

    按键扫描采用稚晖君的方案，定时通过移位寄存器读取按键状态，将按键状态保存到sg_BitMap_New中。经过抖动延时后再次读区按键状态到sg_BitMap_Debound。通过对比将结果保存到sg_BitMap_Old中。当按键按下时通过keybit对应的keycombine找到对应的keycode进行发送。

    按键映射层通过修改keycombine映射的keycode实现不同层以及修改键值等的管理。

## 灯效
    见[rgbmode.h](https://github.com/Marspacecraft/PaceandLoveKeyboard/blob/main/Frameware/PaceAndLoveKeyboard/Core/Led/rgbmode.h)文件说明

