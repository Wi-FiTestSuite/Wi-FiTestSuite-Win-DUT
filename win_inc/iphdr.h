/*****************************************************************************
Copyright (c) Microsoft
All rights reserved.
Licensed under the Microsoft Limited Public License (the “License”); you may not
use this file except in compliance with the License.
You may obtain a copy of the License at http://msdn.microsoft.com/en-us/cc300389.

THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS
OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

See the Microsoft Limited Public License (Ms-LPL) for specific language governing
permissions and limitations under the License.
*****************************************************************************/

#ifndef _IPHDR_H_
#define _IPHDR_H_

#include <pshpack1.h>

#define IN
#define OUT

#define BIT0     0x00000001
#define BIT1     0x00000002
#define BIT2     0x00000004
#define BIT3     0x00000008
#define BIT4     0x00000010
#define BIT5     0x00000020
#define BIT6     0x00000040
#define BIT7     0x00000080
#define BIT8     0x00000100
#define BIT9     0x00000200
#define BIT10     0x00000400
#define BIT11     0x00000800
#define BIT12     0x00001000
#define BIT13     0x00002000
#define BIT14     0x00004000
#define BIT15     0x00008000
#define BIT16     0x00010000
#define BIT17     0x00020000
#define BIT18     0x00040000
#define BIT19     0x00080000
#define BIT20     0x00100000
#define BIT21     0x00200000
#define BIT22     0x00400000
#define BIT23     0x00800000
#define BIT24     0x01000000
#define BIT25     0x02000000
#define BIT26     0x04000000
#define BIT27     0x08000000
#define BIT28     0x10000000
#define BIT29     0x20000000
#define BIT30     0x40000000
#define BIT31     0x80000000

#define DBG_SERIOUS    2
#define DBG_WARNING    3
#define DBG_LOUD    4
#define DBG_TRACE    5

#define COMP_INIT    BIT0
#define COMP_HELP    BIT1
#define COMP_PARSE_CODE   BIT2
#define COMP_PING    BIT3
#define COMP_QOS    BIT4
#define COMP_ALL    ~(0)

typedef UCHAR  u1Byte,*pu1Byte;
typedef USHORT  u2Byte,*pu2Byte;
typedef ULONG  u4Byte,*pu4Byte;
typedef ULONGLONG u8Byte,*pu8Byte;
typedef UCHAR  BOOLEAN,*PBOOLEAN;


// IPv4 option for record route
#define IP_RECORD_ROUTE     0x7

// ICMP6 protocol value (used in the socket call and IPv6 header)
#define IPPROTO_ICMP6       58

// ICMP types and codes
#define ICMPV4_ECHO_REQUEST_TYPE   8
#define ICMPV4_ECHO_REQUEST_CODE   0
#define ICMPV4_ECHO_REPLY_TYPE     0
#define ICMPV4_ECHO_REPLY_CODE     0
#define ICMPV4_MINIMUM_HEADER      8

// ICPM6 types and codes
#define ICMPV6_ECHO_REQUEST_TYPE   128
#define ICMPV6_ECHO_REQUEST_CODE   0
#define ICMPV6_ECHO_REPLY_TYPE     129
#define ICMPV6_ECHO_REPLY_CODE     0

// Restore byte alignment back to default
#include <poppack.h>
 

typedef struct ifc_info_1
{
 HANDLE hIfc;
 HANDLE hFlow;
 HANDLE hFilter;  
}IFC_INFO, *PIFC_INFO;

typedef struct ifc_list_1
{
 ULONG  IfcCount;
 PIFC_INFO pIfcInfo;
}IFC_LIST, *PIFC_LIST;


#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#define FOREGROUND_RED       0x0004 // text color contains red.
#define FOREGROUND_WHITE     (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED) // White.
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
#define BACKGROUND_BLUE      0x0010 // background color contains blue.
#define BACKGROUND_GREEN     0x0020 // background color contains green.
#define BACKGROUND_RED       0x0040 // background color contains red.
#define BACKGROUND_INTENSITY 0x0080 // background color is intensified.
#define FOREGROUND_YELLOW (FOREGROUND_GREEN | FOREGROUND_RED) // Yellow
#define FOREGROUND_INT_BLUE (FOREGROUND_BLUE | FOREGROUND_INTENSITY) // Intensified Blue
#define FOREGROUND_INT_GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY) // Intensified Green
#define FOREGROUND_INT_RED (FOREGROUND_RED | FOREGROUND_INTENSITY) // Intensified Red
#define FOREGROUND_INT_YELLOW (FOREGROUND_YELLOW | FOREGROUND_INTENSITY) //Intensified Yellow
#define FOREGROUND_INT_WHITE (FOREGROUND_WHITE | FOREGROUND_INTENSITY)
#define DEFAULT_FOREGROUD_COLOR FOREGROUND_WHITE // White
#define DEFAULT_OUTPUT_COLOR FOREGROUND_INT_WHITE
#define DEFAULT_HELP_COLOR  FOREGROUND_INT_YELLOW
#define DEFAULT_IMPORTANT_COLOR FOREGROUND_INT_GREEN
#define DEFAULT_ERROR_COLOR  FOREGROUND_INT_RED


#define SetPrintColor(__Color) SetConsoleTextAttribute((HANDLE)GetStdHandle(STD_OUTPUT_HANDLE), __Color)
#define DefaultPrintColor()  SetPrintColor(DEFAULT_FOREGROUD_COLOR)

#ifdef _DEBUG
    #define RT_TRACE(__Comp, __Level, __Fmt)  \
    {            \
        if(__Level <= GlobalDebugLevel)    \
        {           \
           printf __Fmt;       \
           if((__Comp) & GlobalDebugComponents) \
           {          \
              RT_LOG __Fmt;      \
           }          \
        }           \
    }
#else
    #define RT_TRACE(__Comp, __Level, __Fmt) \
    {           \
       if(__Level <= GlobalDebugLevel)   \
       {          \
          printf __Fmt;      \
       }          \
    }
#endif // #ifdef _DEBUG

//
// Printf with color
//
#define C_RT_TRACE(_Color, _Comp, _Level, _Fmt)   \
{              \
    SetPrintColor(_Color);        \
    RT_TRACE(_Comp, _Level, _Fmt);      \
    DefaultPrintColor();        \
}

#define RT_PRINT_ADDR(_Comp, _Level, _TitleString, _Ptr)  \
{                \
    int  __i;            \
    pu1Byte Ptr = (pu1Byte)_Ptr;        \
    RT_TRACE(_Comp, _Level, (_TitleString));     \
    for( __i = 0; __i < 6; __i ++ )        \
    {               \
       RT_TRACE(_Comp, _Level,         \
          ("%02X%s", Ptr[__i], (__i == 5) ? "\n":"-"));  \
    }               \
}

#define C_RT_PRINT_ADDR(_Color, _Comp, _Level, _TitleString, _Ptr) \
{                 \
    SetPrintColor(_Color);           \
    RT_PRINT_ADDR(_Comp, _Level, _TitleString, _Ptr);    \
    DefaultPrintColor();           \
}

#define RT_PRINT_DATA(_Comp, _Level, _TitleString, _HexData, _HexDataLen)  \
{                    \
    int  __i;                \
    pu1Byte ptr = (pu1Byte)_HexData;           \
    RT_TRACE(_Comp, _Level, _TitleString);          \
    for( __i = 0; __i < (int)_HexDataLen; __i ++ )        \
    {                   \
       RT_TRACE(_Comp, _Level,             \
          ("%02X%s", ptr[__i],            \
          (((__i + 1) % 16) == 0) ? "\n" :          \
          ((((__i + 1) % 8) == 0) && ((__i + 1) < (int)_HexDataLen)) ? " - " :          \
          ((((__i + 1) % 4) == 0)?"  ":" ")))        \
    }                   \
    RT_TRACE(_Comp, _Level, ("\n"));           \
}

#define C_RT_PRINT_DATA(_Color, _Comp, _Level, _TitleString, _HexData, _HexDataLen)  \
{                      \
    SetPrintColor(_Color);                \
    RT_PRINT_DATA(_Comp, _Level, _TitleString, _HexData, _HexDataLen);     \
    DefaultPrintColor();                \
}

#define DISABLE_TRACE()       \
{            \
    if(GlobalDebugLevel > 0)     \
       GlobalDebugLevel = (~GlobalDebugLevel); \
}

#define ENABLE_TRACE()       \
{            \
    if(GlobalDebugLevel < 0)     \
       GlobalDebugLevel = (~GlobalDebugLevel); \
}

int
RT_LOG ( IN const char *format, IN ...);

extern INT  GlobalDebugLevel;
extern UINT GlobalDebugComponents;

#endif