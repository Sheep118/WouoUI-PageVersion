# WouoUI-PageVersion
## 写在前面

### 简介

这是一个改动自WouoUI的纯C语言，无依赖库，只适用于128*64OLED的代码框架，将WouoUI抽象出一部分统一的接口，以方便快速构建一个类似具有WouoUI风格的OLEDUI。

在此十分感谢WouoUI作者开源WouoUI的源码🙏🙏，这是[WouoUI的Github链接](https://github.com/RQNG/WouoUI )和[作者的b站链接](https://space.bilibili.com/9182439)。

### 想法由来和一些啰嗦

WouoUIPage版的想法是源自我自己想将使用WouoUI用到自己设置的一个很简单时钟上，但由于使用的芯片是合宙的Air001(为啥用这个芯片，因为手头这个芯片剩的比较多😂)，不过，因为这个芯片只有4k的RAM和32k的Flash，移植U8g2后再移植WouoUI，芯片的Flash和RAM基本上就用完了，很难再写应用层的代码，因此，我自己根据128*64的OLED的需要，参考很多资料自己写了图形层的绘制驱动(oled_g.c和oled_g.h)。同时，把WouoUI用尚不成熟面向对象的思想用C语言重写了一遍并抽象出接口(oled_ui.c和oled_ui.h)，根据自己的需要，还加入了滚动的时钟的页面，至此，WouoUI Page版就此成型。

