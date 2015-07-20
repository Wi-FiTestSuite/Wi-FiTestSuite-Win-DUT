/****************************************************************************
*
* Copyright (c) 2014 Wi-Fi Alliance
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

#ifndef _WFA_MAIN_H
#define _WFA_MAIN_H

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <windef.h>
#include <Qos.h>
#include <NtDDNdis.h>
#include <in6addr.h>
#include <WinSock2.h>
#include <Ws2def.h>
#include <ws2ipdef.h>

#include "iphdr.h"
#include <time.h>

#include "wfa_debug.h"
#include "wfa_types.h"
#include "wfa_sock.h"
#include "wfa_tlv.h"
#include "wfa_tg.h"
#include "wfa_miscs.h"
#include "wfa_rsp.h"
#include "wfa_ver.h"
#include "wfa_cmds.h"

#define ENCRYPT_NONE         0
#define ENCRYPT_WEP          1
#define ENCRYPT_TKIP         2
#define ENCRYPT_AESCCMP      3
#define GROUP_WMMCONF        1
#define WMMAC_UPLINK         0
#define WMMAC_DOWNLINK       1
#define WMMAC_BIDIR          3
#define GROUP_WMMAC          0

#define WMMAC_ADDTS          0
#define WMMAC_DELTS          1
#define WMMAC_AC_BE          0
#define WMMAC_AC_BK          1
#define WMMAC_AC_VI          2
#define WMMAC_AC_VO          3

#define WFA_MAX_TRAFFIC_STREAMS            16

/* maximum number of streams to support */
#define WFA_MAX_WMM_STREAMS            16

#define MAX_CMD_BUFF        1024 
#define MAX_PARMS_BUFF      640 

#define WFA_BUFF_32         32
#define WFA_BUFF_64         64
#define WFA_BUFF_128        128
#define WFA_BUFF_256        256
#define WFA_BUFF_512        512

//fix: restore the WFA_BUFF_1K to 1024
#define WFA_BUFF_1K         1024
#define WFA_BUFF_4K         4096
#define WFA_CMD_STR_SZ      1024 
#define WFA_THREADS_NUM         16 // change it to 16 to be consistent with the number of WMM threads

enum tg_port
{
	UDP_PORT_BE1 = 0,
	UDP_PORT_BE2 = 1,
	UDP_PORT_BK1 = 2,
	UDP_PORT_BK2 = 3,
	UDP_PORT_VI1 = 4,
	UDP_PORT_VI2 = 5,
	UDP_PORT_VO1 = 6,
	UDP_PORT_VO2 = 7,
	UDP_PORT_ND1 = 8,
	UDP_PORT_ND2 = 9
};

typedef int (*xcCommandFuncPtr)(int len, BYTE *parms, int *respLen, BYTE *respBuf);
typedef int (*commandStrProcFuncPtr)(char *, BYTE *, int *);

typedef struct type_name_str_table
{
	int type;
	char name[32];
	commandStrProcFuncPtr cmdProcFunc;
} typeNameStr_t;

#endif
