# WouoUIPagePC端实现

WouoUIPage是一个与硬件平台无关，纯C语言的UI库（目前只能应用于128*64的单色OLED屏幕上，后期会改进，支持更多尺寸）。因此，我们可以在PC上实现它，本文就以在PC上使用 VScode+MinGW+EGE 的组合，来搭建**PC上可以用于测试自己写UI代码的WouoUIPage环境**。

WouoUIPage的github链接：[https://github.com/Sheep118/WouoUI-PageVersion](https://github.com/Sheep118/WouoUI-PageVersion)

## 环境搭建流程

### VScode+MinGW+EGE

需要准备的东西有VScode、EGE安装包、MinGW安装包，需要实现能够在PC上用C语言进行绘图，后期才能移植WouoUIPage。所以，搭建PC能用于C语言屏幕绘制的环境，可以参考下面这个博客。

[VScode+MinGW+EasyGraphicEngine，ege图形库配置教程_安装的mingw怎么使用graphic图形库-CSDN博客](https://blog.csdn.net/qq272508839/article/details/104287255)

这里总结一下博客里提及的操作，以便检测EGE的环境是否安装成功。

- 安装VScode+MinGW，这个按博客中提供的视频操作，问题不大，安装完MinGW后可以新建个C文件或者C++文件，使用gcc或者g++编译测试。
- 在easy graphic engine官网上下载ege_19.01_all的zip包，我们需要获取里面的图形库添加到MinGW的路径中。
- 复制两个东西：
  - ege_19.01_all压缩包下`include`文件夹下的`ege`文件夹、`ege.h` 、`graphics.h` 两个文件，将这三个东西复制到MinGW安装路径中的`include` 文件夹下。
  - (我使用的是64位的版本)将ege_19.01_all压缩包下`lib`文件夹下`mingw64`下的`libgraphics64.a` 库文件复制到MinGW安装路径中的`lib` 文件夹下。（32位的安装环境则对应换成32位）。
- 使用VScode任意打开一个空的文件夹，在文件夹下建立`.vscode` 文件夹(注意有个.前缀)，将博客提供的`c_cpp_properties.json` 、 `tasks.json` 、`launch.json` 新建对应的文件，并复制进去即可（**注意需要更换对应MinGW的路径**）。
-  之后在VScode里打开上面的文件夹，新建一个out的目录和`main.cpp` 文件，将博客中提及对应的测试代码粘贴进去，按`Ctrl+Shift+B`编译，按`F5`编译执行即可。

### 移植WouoUIPage

我将WouoUIPage的SDK，也就是移植完成的VScode文件夹上传到GitHub中了，可以直接将Github中`ProjectExamples\PCSimulate` 下的zip包下载，解压，将解压出来的文件中，`.vscode\c_cpp_properties.json` 、`.vscode\tasks.json` 、`.vscode\launch.json` 这三个json文件中MinGW的路径改为自己的安装路径即可。

需要注意一点：**由于我没有讲编译的依赖关系写入`.vscode\tasks.json` 文件中，而是使用Makefile来组织依赖关系，因此，电脑中需要安装Make**

这里讲一下zip提供的SDK（移植好的zip包）中各个文件夹的作用。

```C
|--.vscode   #vscode的配置文件
    |--c_cpp_properties.json  #指定编译器
    |--tasks.json		#指定编译的任务命令
    |--launch.json    #指定运行时的shell命令
|--build  #编译中间的.o文件存放文件夹
|--out   #最终的可执行文件输出文件夹
|--WouoUIPage  #WouoUIPage的库文件夹
    |-- #注意，这里面的oled_port.c和oled_port.h改写过
|--UITest  #调用WouoUIPage编写的UI文件，编写自己UI测试时，主要修改这里面的文件
    |--TestUI.c
    |--TestUI.h
|--main.cpp  #调用EGE库的主函数
|--Makefile  #编译依赖关系的组织文件
|--simulate_conf.h  #PC端仿真的配置头文件
```

## 具体实现的方式解释

### WouoUIPage的移植

主要更改了 `oled_port.c` 和 `oled_port.h` 这两个文件。

>`oled_port.c`
>
>```C
>#include "oled_port.h"
>
>void OLED_SendBuff(uint8_t buff[8][128]) //将8*128字节的buff一次性全部发送的函数
>{
>    for (int y = 0; y < 8; ++y) {
>        for (int x = 0; x < 128; ++x) {
>            for (int bit = 0; bit < 8; ++bit) {
>                uint8_t value = (buff[y][x] >> bit) & 1; // 获取相应位的值
>                drawBigPixel(x, y*8+bit, value);
>            }
>        }
>    }
>}
>```
>
>使用了`drawBigPixel` 这个函数，实现把缓存的点全部绘制到屏幕上，这个函数的实现在`main.cpp` 中，如下，并在`oled_port.h` 中将其声明。
>
>```C
>// 画大像素点的函数（一个大像素由PIXEL_SIZExPIXEL_SIZE的小像素组成）
>void drawBigPixel(int x, int y, uint8_t value) {
>    COLORREF color = (value == 0 ? BACK_COLOR : FORE_COLOR); // 数组值为0，对应背景色；非0，对应前景色
>    setfillcolor(color);	//设置填充颜色
>    // 使用bar函数绘制一个大像素（PIXEL_SIZExPIXEL_SIZE块）
>    int x0 = x * PIXEL_SIZE;
>    int y0 = y * PIXEL_SIZE;
>    bar(x0, y0, x0 + PIXEL_SIZE, y0 + PIXEL_SIZE); // bar函数的坐标是按照左上角和右下角来绘制矩形的
>}
>```



### Makefile文件的依赖关系

makefile文件指定了文件的依赖关系，使用gcc编译`.` 、`WouoUIPage` 、`UITest` 这三个文件夹下所有的.c文件，并使用g++编译mian.cpp，最后链接成可执行放在`out`目录下。makefile文件的内容如下：

```makefile
CC := gcc
CXX := g++
CFLAGS := -Wall
CXXFLAGS := -Wall
TARGET := ./out/main.exe

# 手动指定源文件
SRC_DIRS := . WouoUIPage UITest
C_SOURCES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) #在给定源文件路径下搜索.c文件
CPP_SOURCES := $(wildcard *.cpp)   

C_OBJS := $(patsubst %.c,./build/%.o,$(notdir $(C_SOURCES)))
CPP_OBJS := $(patsubst %.cpp,./build/%.o,$(notdir $(CPP_SOURCES))) # 在build目录下生成不带子目录的.o文件
OBJS := $(C_OBJS) $(CPP_OBJS)

# 需要增加VPATH变量来定位源文件
VPATH := $(SRC_DIRS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ -lgraphics64 -luuid -lmsimg32 -lgdi32 -limm32 -lole32 -loleaut32

./build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

./build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf ./build/*.o
	rm -f $(TARGET)

.PHONY: all clean
```

## 演示效果

![20240506_142513 -big-original](https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/20240506_142513%20-big-original.gif)
