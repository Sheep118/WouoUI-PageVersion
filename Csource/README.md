# 本文件夹的一些小说明

## 文件目录

WouoUIPage版源代码非常简单(CSource文件夹下)，按功能可以划分为以下几部分

|           代码文件            |                             作用                             |
| :---------------------------: | :----------------------------------------------------------: |
| WouoUI_conf.h/WouoUI_common.h |  配置文件，定义屏幕宽高、字体等参数/内部通用的一些函数和宏   |
|        WouoUI_font.c/h        |                        字体文件和数组                        |
|        WouoUI_anim.c/h        |                        非线性动画实现                        |
|        WouoUI_msg.c/h         |                      输入消息队列的实现                      |
|       WouoUI_graph.c/h        |             图形层，提供基本的图形和字符绘制函数             |
|      **WouoUI_page.c/h**      |          **提供了Title\List\Wave三个基本的全页面**           |
|      **WouoUI_win.c/h**       |    **提供了Msg\Conf\Val\Spin\ListWin五种基本的弹窗页面**     |
|        **WouoUI.c/h**         |                 **核心状态机和UI调度的代码**                 |
|        WouoUI_user.c/h        | 用户UI文件，内实现了一个使用WouoUIPage版提供接口实现的一个UI的样例 |

