# 本文件夹的一些小说明

## 文件目录

```c
|---Air001(使用air001作为主控，SSD1306为OLED驱动器的 WouoUIPage版工程)
    |---Port_C_SPI(SPI驱动的底层代码文件`oled_port.c`)
    |---Air001_Demo.zip(完整工程文件的压缩包，包含TestUI和LittleClok两个例子)
|---Stm32(使用Stm32作为主控，SSD1306为OLED驱动器的 WouoUIPage版工程)
    |---STM32_HWSPI_TestUI.zip(完整工程文件的压缩包,固件库,TestUI例子，使用硬件SPI)
    |---Port_C_SPI_STD(Stm32标准固件库硬件spi的底层驱动文件`oled_port.c`)
|---PCSimuleta(在PC端使用EGE库MinGW搭建的实验环境)
    |---PCSimulate.zip (zip包直接解压后使用vscdoe打开更换MinGW的路径即可)
    |---README.md(环境搭建的一些参考)
```

