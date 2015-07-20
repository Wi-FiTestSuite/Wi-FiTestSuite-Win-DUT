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

#ifndef _WFA_TYPES_H
#define _WFA_TYPES_H 

#define WFA_IF_NAME_LEN 16
#define WFA_VERSION_LEN 64
#define WFA_SSID_NAME_LEN 64
#define WFA_IP_ADDR_STR_LEN  16
#define WFA_IP_MASK_STR_LEN  16
#define WFA_MAX_DNS_NUM      2
#define WFA_MAC_ADDR_STR_LEN 20
#define WFA_IP_V6_ADDR_STR_LEN  48
#define WFA_CLI_CMD_RESP_LEN 512

#ifdef WFA_P2P
#define WFA_P2P_DEVID_LEN 16
#define WFA_P2P_GRP_ID_LEN 128
#define WFA_WPS_PIN_LEN 256
#define WFA_PSK_PP_LEN	256
#endif
#define IF_80211   1
#define IF_ETH     2
#define WMM_UNKNOWN_VENDOR 0
#define WMM_ATHEROS		1
#define WMM_BROADCOMM	2
#define WMM_INTEL		3
#define WMM_MARVELL		4
#define WMM_RALINK      5
#define WMM_STAUT       6
#define WMM_APUT        7

typedef unsigned short WORD;
typedef unsigned char BYTE;

enum _response_staus
{
   STATUS_RUNNING = 0x0001,
   STATUS_INVALID = 0x0002,
   STATUS_ERROR = 0x0003,
   STATUS_COMPLETE = 0x0004,
};
typedef int BOOL;
typedef enum returnTypes
{
   WFA_SUCCESS = 0,
   WFA_FAILURE = 1,
   WFA_ERROR = -1,
} retType_t;
#endif
