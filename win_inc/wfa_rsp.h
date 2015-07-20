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

#ifndef _WFA_RSP_H
#define _WFA_RSP_H

#define WFA_OUT_BUFSIZE       128 

typedef struct _tg_ping_stop_resp
{
	int sendCnt;
	int repliedCnt;
} tgPingStopResp_t;

typedef struct ca_sta_get_ipconfig_resp
{
	int isDhcp;
	char ipaddr[WFA_IP_ADDR_STR_LEN];
	char mask[WFA_IP_MASK_STR_LEN];
	char dns[WFA_MAX_DNS_NUM][WFA_IP_ADDR_STR_LEN];
	int ipTypeV6;
	char ipV6addr[WFA_IP_V6_ADDR_STR_LEN];
} caStaGetIpConfigResp_t;

typedef struct ca_sta_get_stats_resp
{
	int status;
	int txFrames;
	int rxFrames;
	int txMulticast;
	int rxMulticast;
	int fcsErrors ;
	int txRetries;
} caStaGetStatsResp_t;

typedef struct ca_device_get_info_resp
{
	char vendor[16];
	char model[16];
	char version[16];
} caDeviceGetInfoResp_t;

typedef struct ca_sta_upload_resp
{
	short seqnum;
	short nbytes;
	char bytes[256]; 
} caStaUploadResp_t;

typedef struct ca_device_list_if_resp
{
	int status;
#define IF_80211   1
#define IF_ETH     2
	int iftype;

	char ifs[3][64];

} caDeviceListIFResp_t;

typedef struct ca_sta_cli_command_resp
{
	int status;
	short resFlag;
	char result[WFA_CLI_CMD_RESP_LEN]; 
} caStaCliCmdResp_t;

#ifdef WFA_P2P

typedef struct ca_P2p_sta_get_psk_resp
{
	char ssid[WFA_SSID_NAME_LEN];
	char passPhrase[WFA_PSK_PP_LEN];
} caP2pStaGetPskResp_t;

#endif


typedef struct dut_cmd_response
{
	int status;
	int streamId;
	union _cmdru
	{
		tgStats_t stats;
		tgPingStopResp_t pingStp;
		caStaGetIpConfigResp_t getIfconfig;
		caStaGetStatsResp_t ifStats;
		caDeviceGetInfoResp_t devInfo;
		caDeviceListIFResp_t ifList;
		caStaUploadResp_t  uld;
		char version[WFA_VERSION_LEN];
		char info[WFA_OUT_BUFSIZE];
		char bssid[WFA_MAC_ADDR_STR_LEN];
		char mac[WFA_MAC_ADDR_STR_LEN];       
#ifdef WFA_P2P
		char devid[WFA_P2P_DEVID_LEN];
		char grpid[WFA_P2P_GRP_ID_LEN];	
		char p2presult[8];
		char wpsPin[WFA_WPS_PIN_LEN];
		caP2pStaGetPskResp_t pskInfo;

#endif

		int connected;
	} cmdru;
}dutCmdResponse_t;

#endif
