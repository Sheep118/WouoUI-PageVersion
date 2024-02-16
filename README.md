# WouoUI-PageVersion
## 写在前面

### 简介&致谢

这是一个改动自WouoUI(1.2版本)的纯C语言，无依赖库，只适用于128*64OLED的代码框架，将WouoUI抽象出一部分统一的接口，以方便快速构建一个具有类似WouoUI风格的OLEDUI。

在此十分感谢WouoUI作者开源WouoUI的源码🙏🙏，这是[WouoUI的Github链接](https://github.com/RQNG/WouoUI )和[作者的b站链接](https://space.bilibili.com/9182439)。

(推荐大家可以去阅读下WouoUI的源代码(Arduino)，写得非常好，逻辑相当清晰)。

### 想法由来和一些啰嗦

WouoUIPage版的想法是源自我自己想将使用WouoUI用到自己设置的一个很简单时钟上，但由于使用的芯片是合宙的Air001(为啥用这个芯片，因为手头这个芯片剩的比较多😂)，不过，因为这个芯片只有4k的RAM和32k的Flash，移植U8g2后再移植WouoUI，芯片的Flash和RAM基本上就用完了，很难再写应用层的代码，因此，我自己根据128*64的OLED的需要，参考很多资料自己写了图形层的绘制驱动(oled_g.c和oled_g.h)。同时，把WouoUI用尚不成熟面向对象的思想用C语言重写了一遍并抽象出接口(oled_ui.c和oled_ui.h)，根据自己的需要，还加入了滚动的时钟的页面，至此，WouoUI Page版就此成型。

至于为何取名叫WouoUI Page版，一方面是由于这个项目本来就是从原来WouoUI改动而来，另一方面，抽象出来的接口都以一个页面(Page)为最小单位，所以取名叫WouoUI Page版😀。

### WouoUIPage版和WouoUI不同

因为我参考原作者的WouoUI是1.2版本的，因此，作者后来更新的一些新的动画没有加进来(之后看是否有空，再决定是否加进来了🤣, 到时候估计也需要换个芯片了，air001Falsh太小了，而且提供的例程也只支持keil内的O0优化)。

|        | WouoUI                                                       | Page版                                                       |
| ------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 依赖库 | 依赖U8g2库，适用性广，拓展性强 ；但消耗的RAM和FLASH可能会较多(特别是对于使用C语言开发而不是使用Arduino开发的情况) | 自己写的图形层驱动文件，功能比较少只提供必须用到的，纯C语言编写，对内存有限，使用C开发单片机来说比较合适；但由于没有使用U8g2库，适用性、拓展性差。(后面看看自己有没有时间将上面的这部分抽象移植到U8g2图形库上) |
| 接口   | 原作者的所有代码都在一个.ino文件中完成，方便查看源码; 但需要读懂一部分源码才能二次开发 | 统一了一部分接口，并做了抽象，二次开发时只需要按接口文档来使用提供的接口函数 |
| 适用性 | 原作者开发了适应多个屏幕尺寸的版本👍(这点我自己觉得可能很难做到) | 因为项目的需要，Page版只有128*64这一个尺寸适配，我也没有做其他屏幕的开发。 |

## 移植说明

WouoUIPage版源代码非常简单，只有oled_g.c , oled_ui.c 两个.c文件和 oled_g.h , oled_ui.h ,oled_conf.h , oledfont.h 和 oled_port.h 5个.h文件组成。

### 本项目的文件说明

```markdown
|---Csource
	|---src  (这个文件夹是WouoUIPage最主要的源代码文件，移植时主要用这里面的几个文件)
	|---example  (这个是使用WouoUIPage所提供的接口构建的一些应用的例子,只有对应.c.h文件)
|---ProjectExamples  (这个文件夹内放着使用WouoUIPage的工程的参考，按主控的类型划分，为移植时提供一些参考)
|---Image (存放一些展示用的图片)
```

### 移植大致流程

移植时，可以参考ProjectExamples文件夹下的工程(工程有目前有使用Air001和STM32的例子)

1. 将上述说明的7个文件包含在工程中。

2. 根据自己128*64的OLED屏和自己使用的MCU完成一个oled_port.c文件，这个文件包含提供的oled_port.h文件，并实现oled_port.h中声明的两个函数 `OLED_Init` `OLED_SendBuff` 。

   一般这两个函数OLED厂商都会提供对应的代码，或者根据厂商的代码简单修改也能得到。

   (这里建议先测试厂商的代码，确保能够驱动OLED屏，再移植WouoUIPage)

   >`void OLED_Init(void);`
   >
   >这个函数需要实现OLED的底层初始化，如IIC接口/SPI接口 和 D/C 、RST 等IO口的初始化

   > `void OLED_SendBuff(uint8_t buff[8][128]);`
   >
   > 这个函数需要实现128*64的OLED显存的刷新，即将传入的数组全部写到OLED显存中
   >
   > (这里建议硬件上使用SPI或者至少是硬件IIC，软件IIC刷新速率可能太慢，对动画效果有影响)

   > `OLED_LOG` 这个宏是用于打印一些错误提示信息的，但因为现在暂时还没有使用到🤔，不用理会。

OK ，到这里的话，就算移植结束了(感觉说是复制更为合适，因为基本没有改啥🤣)，可以开始使用WouoUIPage提供的接口了。

## 整体框架说明和配置文件oled_conf.h

### 整体框架说明

如下图所示，**WouoUIPage版以6种页面为基础来构建UI，内部有一个消息队列用于接收外界的消息，每个页面都有一个回调函数，会将当前页面选中项的信息传入，对应操作便可以写在回调函中。**

<img src="https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/image-20240210224956366.png" alt="image-20240210224956366" style="zoom:67%;" />

### 配置文件oled_conf.h

配置文件中主要有几个宏，具体的宏的解释可以直接看代码注释。

```c
#define UI_CONWIN_ENABLE            1  //是否使能 以"$ " 为开头的选项使用确认弹窗
#define UI_MAX_PAGE_NUM             32 //页面的最大数量，这个数字需要大于所有页面的page_id
#define UI_INPUT_MSG_QUNEE_SIZE     4  //ui内部消息对列的大小(至少需要是2)

//页面类型使能宏，使用对应的页面类型，则需要开启该宏，将宏置为1，默认都开启
#define PAGE_WAVE_ENABLE        1
#define PAGE_RADIO_ENABLE       1
#define PAGE_RADERPIC_ENABLE    1
#define PAGE_DIGITAL_ENABLE     1
```

## 接口函数的使用

这个部分会说明一部分构建UI可能用到的结构体类型(类)和一些函数。构建的时候也参考CSource/example的例子(其实只有两个🤣)。

### 类(结构体类型)

一共有6种页面类型 `TitlePage` 、`ListPage` 、`WavePage` 、 `RadioPage`、`RaderPicPage` 和`DigitalPage`  ，还有3个比较重要的类 `InputMsg` 、 `Option`  和 `CallBackFunc` 。其他的一些枚举类型一些只有个别函数才会用到的结构体就不一一说明了，根据函数说明填入对应类型即可。

- `InputMsg` : 输入信息枚举类型，作为`void OLED_MsgQueSend(InputMsg msg);` 的参数类型，用于给UI输入一个外界动作的消息，枚举的消息共有以下几种：

  ```C
  typedef enum 
  {
    msg_none = 0x00, //none表示没有操作
    msg_up,      //上，或者last消息，表上一个
    msg_down,    //下，或者next消息，表下一个
    msg_return,  //返回消息，表示返回，从一个页面退出
    msg_click,   //点击消息，表确认，确认某一选项，回调用一次回调
    msg_home,    //home消息，表回主界面(尚未设计，目前还没有设计对应的功能，默认以page_id为0的页面为主页面)
  } InputMsg; //输入消息类型，UI设计只供输入5种消息
  ```

- `Option` ：选项类，其结构体成员如下：

  ```c
  typedef struct 
  {
    uint8_t order;  //该选项在列表/磁贴中的排序(0-255)
    int16_t  item_max; //列表项对应变量可取的最大值(若是单选/多选框，该值无意义，可为0)
    int16_t  item_min; //列表项对应变量可取的最小值(若是单选/多选框，该值无意义，可为0)
    int16_t  step;//列表项对应变量的步长变化，只对数值弹窗有效(若是单选/多选框，该值无意义，可为0)
    int16_t  val;  //这个列表项关联的显示的值(可以用于设置初值) 
    String text;      
    //这个列表项显示的字符串
    //(通过选项的第一个字符判断为数值弹窗(~)/确认弹窗($)/其他项(-)/二值选项框(+)/单选项(=),
      /*注意只有WavePage和DigitalPage的字符串不需要前缀)*/
    //其中二值选项框由于二值项只能在列表中展示，因此只在列表选择页面中有效，在磁帖页面中如果出现+开头的字串默认为其他项
    //其实单选列表项，需使用其他项在应用层关联跳转单选终端页面实现(单选列表项必须使用=做字符串开头)。
  } Option; //通用选项类
  ```

  >Option变量的初始化需要注意以下两点：
  >
  >1. 字符串成员 text, 在ListPage和TitlePage页面都是有前缀的，且前缀是有意义的(用于识别弹窗和选框)；**text的第一个字符判断为数值弹窗(~)/确认弹窗($)/其他项(-)/二值选项框(+)/单选项(=)**。
  >
  >2. **如果想要一个选项不会受接收消息的控制，如某个数值变量(如电量，由外部来更改，不是需要设置的)只是用于展示的，step成员可以设置为0，这样即使收到对应的up/down消息也不会改变其值。同时，因为将step设为0时，通常将数值弹窗当作展示使用，所以此时会在进入数值弹窗前调用一次回调函数用于给数值赋值，若step不为0,则只有在数值弹窗内收到click才会调用回调** 。
  >
  >   同样的，**对于二值选框(text以"+ "开头) ， 如果step为0，收到up/down消息时也不会取反，只有step不为0时才会取反。**

- `CallBackFunc` : 回调函数类型，每个页面都需要一个回调函数(如果没有回调函数，在对应初始化函数中置NULL即可)。

  通常需要定义一个形如`void MainPage_CallBack(uint8_t self_page_id,Option* select_item)`的函数(其中，MainPage_CallBack是由我们定义的回调函数名(地址))，并该函数地址作为参数给对应的页面初始化函数。

  > 在页面中的选项被click时，该页面的回调函数会被调用，
  >
  > 传入回调函数的参数`self_page_id` 为当前页面的id(每个页面都有唯一的id，需要在对应页面初始化时传入)；
  >
  > 传入回调函数的参数`select_item`为当前页面被选中且click的选项(类型为Option)，可以通过读取该函数的order得知当前选中的是哪个选项，并在回调函数中进行对应的处理。

下面会分成6个基础页面类型来介绍对应的页面类型，同时说明接口函数的使用(其实，大部分接口函数直接看注释都能明白的🤣)。

### TitlePage类和接口函数

- TiltlePage页面的演示效果如下：

  <img src="https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/TitlePage%E6%BC%94%E7%A4%BA.gif" alt="TitlePage演示" style="zoom:80%;" />

- 接口函数只有一个

  `void OLED_TitlePageInit(TitlePage * title_page, uint8_t page_id,uint8_t item_num,Option* option_array,Icon *icon_array,CallBackFunc call_back);`

  >- 参数需要有TitlePage 页面对象的指针，唯一的page_id(每个页面必须有一个唯一的id);
  >
  >- 需要注意的有 ：**item_num 表示后面 option_array(选项数组)和 icon_arra(图标数组)的数组大小，三者必须保持一致**；
  >
  >- 同时，**option_array 中的选项内的text字符串需要有前缀，用于表示该选项的类型**
  >
  >  - **`"~ "` 前缀表示该选项为数值弹窗** 
  >
  >    (在TitlePage页面使用数值弹窗可以参考CSource/example下LittleClock的例子)
  >
  >  - **`"$ "` 前缀表示该选项为确认弹窗**
  >
  >  - 其他前缀为一般选项，没有特殊意义，需要注意的是TitlePage页面不会显示选项的text前缀，**因此，对于一般选项，字符串前最好空出两个空格或者使用`"- "` 前缀。**
  >
  >- **TitlePage会在选中选项上收到msg_click消息时，调用一次回调函数，并将选中的选项传入**。

### ListPage类和接口函数

- ListPage页面的演示效果如下：

  <img src="https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/ListPage%E6%BC%94%E7%A4%BA.gif" alt="ListPage演示" style="zoom:80%;" />

- 同样地，接口函数只有一个

  `void OLED_ListPageInit(ListPage * lp,uint8_t page_id,uint8_t item_num,Option *option_array,CallBackFunc call_back);`

  >- 参数需要有ListPage 页面对象的指针，唯一的page_id(每个页面必须有一个唯一的id);
  >- 同样需要注意的是：**item_num 表示后面option_array(选项数组)的数组大小，必须保持一致** 
  >- 同时，**option_array选项数组中text字符串使用前缀表示选项的类型**
  >  - **`"~ "`  前缀表示数值弹窗**
  >  - **`"$ "`  前缀表示确认弹窗**
  >  - **`"+ "`  前缀表示二值选项框** (会在选项后用一个框，表示是否框选)
  >  - **其他前缀没有特殊意义，与TitlePage不同的是，ListPage 会显示字符串的前缀，用于对齐选项，因此，强烈建议，其他选项使用 `"- "` 作为字符串前缀。**
  >- 同样，ListPage会在选中选项上收到msg_click消息时，调用回调函数，并将选中选项传入回调函数。

### RadioPage类和接口函数

- RadioPage页面的演示效果如下：

  <img src="https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/RadioPage%E6%BC%94%E7%A4%BA.gif" alt="RadioPage演示" style="zoom:80%;" />

- 其实，**RadioPage页面和ListPage页面是基本一样的，不同的是，对于使用`"= "` 作为text前缀的选项来说，RadioPage页面会将其处理为单选项，即，这个页面内，所有使用 `"= "` 为text前缀的选项只能有一个能被选中，以实现单选的效果。**因为，通常对于这样的选项页面，我们一整个页面内都是这种单选项，所以将其单独作为一个页面类型拿出。

- 其接口函数与ListPage基本一致：(注意事项也请参考上面😆)

  `void OLED_RadioPageInit(RadioPage * rp, uint8_t page_id, uint8_t item_num,Option *option_array,CallBackFunc call_back);` 

  唯一一点区别，可能就是第一个参数的类型，需要是RadioPage类型。

### WavePage类和接口函数

- WavePage页面的演示效果

  <img src="https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/WavePage%E6%BC%94%E7%A4%BA.gif" alt="WavePage演示" style="zoom:80%;" />

- WavePage页面有两个接口函数

1. `void OLED_WavePageInit(WavePage * wp, uint8_t page_id, uint8_t item_num, Option *option_array, CallBackFunc call_back);`  （照例是初始化函数）

   >- 参数需要有WavePage 页面对象的指针，唯一的page_id(每个页面必须有一个唯一的id);
   >- 同样需要注意的是：**item_num 表示后面option_array(选项数组)的数组大小，必须保持一致** 
   >
   >- **与其他页面不同的是，WavePage 页面选项text的字符串前缀均没有特殊意义，同时也不建议在WavePage页面使用选项前缀，因为需要显示波形的关系，选项字符串的大小最好不要超过5个字符，因此就不加前缀占用多余的字符了**
   >- 同样的，**WavePage页面也在选中项上收到msg_click消息时会调用一次回调函数。**

2. `void OLED_UIWaveUpdateVal(Option * op, int16_t val);`

   >- 这个函数用于更新每个选项的波形值，对应的参数就是要 **更新选项的指针** 和 **更新的值**。
   >
   >- 需要注意以下的是，**波形的绘制是与 `void OLED_UIProc(void);` 这个函数的调用一致的。**
   >
   >  **因此，建议波形的横坐标为一个随着 `OLED_UIProc()` 函数调用 一起自增的变量比较合适。**
   >
   >  (可以参考CSource/example下的UITest例子)

------------------------------

分割线在此，因为，以上的页面均是WouoUI原作者中的主要页面元素(再次感谢原作者🙏🙏)；以下是我自己因为项目需要自己增加的两个页面，两个页面的过渡动画都使用了和WouoUI一致的动画。

### RaderPicPage类和接口函数

- RaderPicPage页面演示效果：

  <img src="https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/RaderPicPage%E6%BC%94%E7%A4%BA.gif" alt="RaderPicPage演示" style="zoom:80%;" />

- RaderPicPage 类只有一个接口函数(就是初始化函数)

  `void OLED_RaderPicPageInit(RaderPicPage* rpp,uint8_t page_id,uint8_t pic_num,RaderPic * pic_array,RaderPicMode mode,CallBackFunc cb);`

  >- 参数需要有RaderPicPage 页面对象的指针，唯一的page_id(每个页面必须有一个唯一的id);
  >
  >- 需要注意的是，**pic_num是后面pic_array数组的数组大小，必须保持一致** ，同时，这里有一个RaderPicPage页面才会用到的类(结构体类型)RaderPic，详情在下方。
  >- 同时，**mode成员为一个枚举类型，用于设置页面图片全部绘制完成后的操作，有两个可取值**
  >  - `Rader_Pic_Mode_Loop` : 全部图片绘制结束后，清空页面，从头重新绘制
  >  - `Rader_Pic_Mode_Hold` : 全部图片绘制结束后，保持绘制完成的页面。
  >- 需要注意的另外一点是， **RaderPicPage页面没有选项，所以回调函数会在每一个图片绘制完成后调用，将该图片的顺序( 1 ~ pic_num ), 作为Option的order传入RaderPicPage的回调函数，可以在回调函数中接收Option的order以判断第几张图片已经绘制完成，再执行对应的操作**。
  >  - 这儿需要注意，**如果是页面处于 `Rader_Pic_Mode_Hold` 模式的话，所有图片绘制结束后，处于保持状态，会持续调用回调函数，并传入第pic_num张图片已绘制完成的Option(Option的order成员的值为pic_num)。**

- RaderPic类，镭射图片类型，其结构体成员如下：

  ```c
  typedef struct 
  {
      const uint8_t * pic; //图片指针
      int16_t start_x;    //起始x坐标
      int16_t start_y;    //起始y坐标
      uint8_t w;          //图片的宽
      uint8_t h;          //图片的高
      RaderDirection rd;  //控制绘制时的射线方向选择，枚举类型
  } RaderPic; //镭射图片对象
  ```

  其中结构体成员`rd` 为一个枚举类型 `RaderDirection` 变量，其可以有的取值有：

  ```c
  typedef enum
  {
      RD_LEFT_UP = 0x00, //射线从左上角出发
      RD_LEFT_DOWN,      //射线从左下角出发
      RD_RIGHT_UP,       //射线从右上角出发
      RD_RIGHT_DOWN,	   //射线从右下角出发
      RD_RIGHT,		   //射线从水平向右
      RD_LEFT, 		   //射线从水平向左
  } RaderDirection;
  ```

### DigitalPage类和接口函数

#### DigitalPage 页面演示效果：

<img src="https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/DigitalPage%E6%BC%94%E7%A4%BA.gif" alt="DigitalPage演示" style="zoom:80%;" />

#### DigitalPage 页面相关的接口函数有三个:

- `void OLED_DigitalPageInit(DigitalPage* dp, uint8_t page_id, Option * option_array, uint8_t  label_array_num, String * label_array, char gap_char, uint8_t gap_shine_time, uint8_t uline_shine_time,CallBackFunc cb);`

   >- 参数需要有DigitalPage 页面对象的指针，唯一的page_id(每个页面必须有一个唯一的id);
   >
   >- 需要注意的参数有：
   >
   >- **`option_array`: 这个选项数组的大小必须为3，因为3个选项分别表示DigitalPage页面中3个两位的十进制数字。**
   >
   >- `label_array_num` 为后面参数 `label_array` 标签数组的数组大小，必须保持一致。（其中，`String` 类型就是 `char*`, 在使用时，可以直接使用`String` 类型，具体可以参考 CSource/example下的UITest例子）
   >
   >- `gap_char` 为3个两位十进制数字间的单个分隔字符，`gap_shine_time`、`uline_shine_time` 用于控制 分隔字符 和 <u>编辑时出现的下划线</u> 的闪烁间隔，闪烁周期为 `OLED_UIProc()`函数运行一次的时间间隔 乘以 参数设置值。
   >
   >- 在DigitalPage 页面，**编辑每个数字(或者是标签)后收到msg_click 消息 是会触发一次回调，将 对应数字的Option 或者 标签赋值的text的Option 传入回调函数，以便接收click时的选中的数字或者标签字符串的值。** 建议在回调函数中检查option中的order时，使用DigitalPosIndex枚举类型进行判断。
   >
   >  ```c
   >  typedef enum //Digital页面从右往左为indexRigit到indexLeft
   >   {
   >       Digital_Pos_IndexRight = 0x00, //用于指示当前光标或者编辑的位置
   >       Digital_Pos_IndexMid,
   >       Digital_Pos_IndexLeft,
   >       Digital_Pos_IndexLabel,  //在标签处
   >       Digital_Pos_Complete,   //编辑完成
   >   }DigitalPosIndex; //用于在回调函数中检验选中项的op->order值,表示选中哪个数字位还是标签
   >  ```
   >
   >  同时，**在DigitalPage页面退出编辑模式(返回观察模式)时，也会调用一次回调函数函数，并传入的order为`Digital_Pos_Complete`的一个option** , 所以在回调函数中判断这个order，可以将对应的操作放在编辑结束返回到观察模式时执行(例如，等所有值设置结束再读取值)。 
   >

- `void OLED_DigitalPage_UpdateDigitalNumAnimation(DigitalPage * dp, uint8_t leftval, uint8_t midval, uint8_t rightval, DigitalDirect dir);`

  >- 用于更新DigitalPage页面中的数字，并触发滚动的动画。
  >- 需要注意的是，`dir` 是 `DigitalDirect` 枚举类型的一个变量，可取的值有 `Digital_Direct_Increase` ,  `Digital_Direct_Decrease` 两种，决定数字的滚动方向。

- `void OLED_DigitalPage_UpdateLabelAnimation(DigitalPage * dp, uint8_t label_index, DigitalDirect dir)`

  >- 用于更新DigitalPage页面中的标签，并触发滚动的动画。
  >
  >- 需要注意的是，只能将标签更新为初始化中设置的 `label_array` 标签数组中的一个便签。`label_index` 为要更新的标签在 `label_array` 的下标(取值0~`label_num_array -1`)；`label_index`可以超过 `label_num_array` ，当超过时会自动选择为0。
  >
  >  `dir` 与更新数字的函数一样，是 `DigitalDirect` 枚举类型的一个变量，可取的值有 `Digital_Direct_Increase` ,  `Digital_Direct_Decrease` 两种，决定的滚动方向。

#### DigitalPage页面操作逻辑的说明

DigitalPage类(结构体类型)中有一个比较重要的成员`mode` (为枚举类型 `DigitalMode`) 。该枚举类型的取值有如下几个

```c
typedef enum
{
    Digital_Mode_Observe = 0x00,   
    //观察模式没有光标(刚进入DigitalPage页面的模式;没有下划线光标)
    Digital_Mode_Edit    = 0x01,   
    //对编辑位置的编辑(可以通过up\down消息控制光标的移动；下划线光标闪烁)
    Digital_Mode_Singlebit = 0x02, //对单位数字的编辑(可以通过up\down消息控制单位数字/标签的加减和切换；下划线光标常亮)
} DigitalMode; //digital页面的模式
```

DigitalPage页面运行的状态机如下图所示：有兴趣的可以看看👀

<img src="https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/image-20240212120943969.png" alt="image-20240212120943969" style="zoom:67%;" />

### UI的一些接口函数和参数

一些UI使用的接口函数和参数一起在这儿说明。

#### 接口函数(有6个)

```c
//用于向UI传递一个消息Msg(msg_click/msg_up/msg_down/msg_return)
void OLED_MsgQueSend(InputMsg msg);
//UI必要的一些参数和变量的初始化
void OLED_UiInit(void);
//UI运行任务，需要放在主循环中循环调用，而且尽量不要阻塞
void OLED_UIProc(void);
//从一个页面跳转到另一个页面，常用于回调函数中调用，并确定页面的上下级关系(这样，在terminate_page页面收到return消息时，会返回self_page_id所代表的页面)
//@param self_page_id 是当前页面的id（回调函数中有这个参数）
//@param  terminate_page 要跳转的那个页面的地址(不需要理会是那种类型的页面，直接将页面地址作为参数传入即可)
void OLED_UIJumpToPage(uint8_t self_page_id,PageAddr terminate_page);
//切换当前页面的函数，与Jump函数不同的时，这个函数不会绑定上下级页面关系，
//terminate_page 页面收到return 消息不会返回当前页面(常用于临时的画面切换)
//@param terminate_page 要跳转的那个页面的地址(不需要理会是那种类型的页面，直接将页面地址作为参数传入即可)
void OLED_UIChangeCurrentPage(PageAddr terminate_page);
/*获取当前页面的id*/
uint8_t OLED_UIGetCurrentPageID(void);
```

需要注意的只有：

1. `void OLED_UIProc(void);` 需要在主循环中循环调用，且最好没有阻塞；
2. `OLED_UIJumpToPage` 函数会确认页面的上下级关系，`OLED_UIChangeCurrentPage` 函数只是切换页面，没有确认页面的上下级关系；
3. `OLED_UIJumpToPage` 函数和`OLED_UIChangeCurrentPage` 函数 的`terminate_page` 参数只需传入页面地址(指针)就行，不需要理会页面是什么类型。

#### UI参数

WouoUIPage的参数基本上WouoUI原作保持一致，如下:(由于我没有移植退出时的淡出动画，因此参数也会比WouoUI要少一些)

```c
  ui_para.ani_param[TILE_ANI] = 120;  // 磁贴动画速度
  ui_para.ani_param[LIST_ANI] = 120;  // 列表动画速度
  ui_para.ani_param[WIN_ANI] = 120;   // 弹窗动画速度
  ui_para.ani_param[TAG_ANI] = 60;   // 标签动画速度
  ui_para.ani_param[DIGI_ANI] = 100;  // 数字动画滚动速度(这些速度是越大运动越慢)
  ui_para.ufd_param[TILE_UFD] = True;   // 磁贴图标从头展开开关
  ui_para.ufd_param[LIST_UFD] = True;   // 菜单列表从头展开开关
  ui_para.loop_param[TILE_LOOP] = True;  // 磁贴图标循环模式开关
  ui_para.loop_param[LIST_LOOP] = True;  // 菜单列表循环模式开关
  ui_para.valwin_broken = True;           //弹窗背景虚化开关
  ui_para.conwin_broken = True;           //确认弹窗背景虚化开关
  ui_para.digital_ripple = True;           //digital页面波纹递增动画开关
  ui_para.raderpic_scan_mode = False;     //镭射文字只扫描亮处
  ui_para.raderpic_scan_rate = 4;        //镭射文字扫描速度
  ui_para.raderpic_move_rate = 50;        //镭射文字移动速度
```

## WouoUIPage整体的代码逻辑

这部分是针对想要二次开发WouoUIPage的人写的(其实是怕我之后回来想做改进的时候看不懂自己的代码🤣)，方便理解整体的代码逻辑，如果是单纯使用的话，参考例子和上面的接口说明应该就够了。

### 关于图形层

- 图形层中有一个`window` 类，这个类几乎在每个图形层的函数中都是作为第一个参数输入的。**这个`window` 只是用于控制绘制的边界，绘图函数绘制的点如果超过传入的`window` 的大小，就不会绘制到缓冲区中。**

  `oled_ui.h`  中提供了一个`w_all` 全局窗口变量，只是为了方便可能在除了上述页面之外，使用者还有其他绘图需要准备的，其边界为整个128*64的屏幕，这个窗口也是整个ui绘制时主要使用的窗口。

- 图形层中有一个更改画点颜色的函数:（其实是控制写入的字节以什么位运算写入缓冲区中）。

  ```c
  /**
  * @brief : void OLED_SetPointColor(uint8_t color)
  * @param : 设置画笔颜色，color：1=亮，0=灭,2=反色
  * @attention : None
  * @author : Sheep
  * @date : 23/10/04
  */
  void OLED_SetPointColor(uint8_t color)
  {
  	switch (color)
  	{
  		case 0: oled_color = '&';break;
  		case 1: oled_color = '|';break;
  		case 2: oled_color = '^';break;
  		default:break;
  	}
  	// oled_color = (color != 0 )?'|':'&';
  } 
  ```

### 关于UI层

- `PageAddr` 页面地址变量，其实是一个 `void*` 类型，为了用于实现 `OLED_UIJumpToPage` 函数和 `OLED_UIChangeCurrentPage` 函数的 "伪多态"。(其实内部所有`AnimInit` 、 `Show` 、 `React` 函数都使用这样的"伪多态") 。

  在获取页面地址后，直接强转为 `Page` 类，这是一个所有页面类型都必须包含的结构体成员(且必须是第一个，为了方便转换类型)。`Page` 类中包含了所有页面必须有的一些成员，如下：

  ```c
  //每个页面都有的三种方法
  typedef void (*PageAniInit)(PageAddr);  //页面的动画初始化函数
  typedef void (*PageShow)(PageAddr);    //页面的展示函数
  typedef void (*PageReact)(PageAddr);  //页面的响应函数
  typedef struct 
  {
    PageType page_type; //页面类型，以便在处理时调用不同函数绘制
    uint8_t page_id; //页面的序号，每个页面唯一一个，用于指示在数组中的位置，方便跳转
    uint8_t last_page_id; //上一个页面的id，用于返回时使用
    CallBackFunc cb; //页面的回调函数
    PageAniInit ani_init; 
    PageShow show;
    PageReact react;
  } Page; //最基本的页面类型(所有页面类型的基类和结构体的**第一个成员**)
  ```

- WouoUIPage的状态机(相比WouoUI原版减少了一个”退出过渡动画“，原因就是 Air001的RAM和Flash不太够🤣)。

  <img src="https://sheep-photo.oss-cn-shenzhen.aliyuncs.com/img/image-20240212120647148.png" alt="image-20240212120647148" style="zoom:67%;" />

### 关于注释

如果有小伙伴愿意查看源代码的话，可能会看到有三种风格的函数注释，这是因为我本身有两种函数注释风格，而且在后期写代码的过程中，使用了阿里的大模型帮忙生成注释，因此注释可能会有多种风格😆。

