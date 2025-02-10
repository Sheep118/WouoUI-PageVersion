#ifndef __WOUOUI_COMMON_H__
#define __WOUOUI_COMMON_H__

#include "WouoUI_conf.h"           // 用户配置文件 
#include "stdint.h"                // 标准整数类型
#include "stdbool.h"               // 标准布尔类型
#include "stdlib.h"               // 标准库(sprintf等)
#include "stdio.h"              // 标准输入输出(printf等)
#include "stdarg.h"             // 标准参数处理(va_list、__VA_VARS__等)
#include "string.h"             // 字符串处理函数(strlen等)
// #include "stddef.h"             //off_setof函数
// #define ACCESS_MEMBER(ptr, type, member) (*(type *)((char *)(ptr) + offsetof(type, member)))


#define UINT_DIVISION_FLOOR(a,b) (a/b)
#define UINT_DIVISION_CELL(a,b)  ((a + b - 1) / b)
#define UINT_DIVISION_ROUND(a,b) ((a + (b / 2)) / b)

#define MAX(a,b)                 ((a)>(b)?(a):(b))
#define MIN(a,b)                 ((a)<(b)?(a):(b))
#define ABS(a)                   ((a)>0?(a):(-(a)))   
#define UNUSED_PARAMETER(x)     ((void)x) // 对未使用参数的处理，防止警告


#define WOUOUI_LOG_E(fmt,...)   WOUOUI_LOG("error,In %s[%d]:" fmt "\r\n",__FILE__,__LINE__,##__VA_ARGS__) 
#define WOUOUI_LOG_W(fmt,...)   WOUOUI_LOG("warning,In %s[%d]:" fmt "\r\n",__FILE__,__LINE__,##__VA_ARGS__) 
#define WOUOUI_LOG_I(fmt,...)   WOUOUI_LOG("info,In %s[%d]:" fmt "\r\n",__FILE__,__LINE__,##__VA_ARGS__) 


#endif
