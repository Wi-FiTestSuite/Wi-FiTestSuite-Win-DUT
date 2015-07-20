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

/**
 * @file wfa_cmdtbl.c
 * @brief File containing the function table associated with tags
*/

#include "wfa_dut.h"

/**
 * NotDefinedYet(): a dummy function
 */
int NotDefinedYet(int len, unsigned char *params, int *respLen, BYTE *respBuf)
{
	DPRINT_WARNING(WFA_WNG, "The command processing function not defined.\n");

	/* need to send back a response */
	return WFA_SUCCESS;
}

/* globally define the function table */
xcCommandFuncPtr gWfaCmdFuncTbl[WFA_STA_COMMANDS_END+(WFA_STA_NEW_COMMANDS_END - WFA_STA_NEW_COMMANDS_START - 1)] =
{
	/* Traffic Agent Commands */
	NotDefinedYet,            /*    None                               (0) */
	agtCmdProcGetVersion,     /*    WFA_GET_VERSION_TLV                (1) */
	wfaTGSendPing,            /*    WFA_TRAFFIC_SEND_PING_TLV          (2) */
	wfaTGStopPing,            /*    WFA_TRAFFIC_STOP_PING_TLV          (3) */
	wfaTGConfig,              /*    WFA_TRAFFIC_AGENT_CONFIG_TLV       (4) */
	wfaTGSendStart,           /*    WFA_TRAFFIC_AGENT_SEND_TLV         (5) */
	wfaTGRecvStart,           /*    WFA_TRAFFIC_AGENT_RECV_START_TLV   (6) */
	wfaTGRecvStop,            /*    WFA_TRAFFIC_AGENT_RECV_STOP_TLV    (7) */
	wfaTGReset,               /*    WFA_TRAFFIC_AGENT_RESET_TLV        (8) */
	NotDefinedYet,            /*    WFA_TRAFFIC_AGENT_STATUS_TLV       (9) */

	/* Control and Configuration Commands */ 
	wfaStaGetIpConfig,        /*    WFA_STA_GET_IP_CONFIG_TLV          (10)*/
	wfaStaSetIpConfig,        /*    WFA_STA_SET_IP_CONFIG_TLV          (11)*/
	wfaStaGetMacAddress,      /*    WFA_STA_GET_MAC_ADDRESS_TLV        (12)*/
	NotDefinedYet,            /*    WFA_STA_SET_MAC_ADDRESS_TLV        (13)*/
	wfaStaIsConnected,        /*    WFA_STA_IS_CONNECTED_TLV           (14)*/
	wfaStaVerifyIpConnection, /*    WFA_STA_VERIFY_IP_CONNECTION_TLV   (15)*/
	wfaStaGetBSSID,           /*    WFA_STA_GET_BSSID_TLV              (16)*/
	wfaStaGetStats,           /*    WFA_STA_GET_STATS_TLV              (17)*/
	wfaSetEncryption,         /*    WFA_STA_SET_ENCRYPTION_TLV         (18)*/
	wfaStaSetPSK,             /*    WFA_STA_SET_PSK_TLV                (19)*/
	wfaStaSetEapTLS,          /*    WFA_STA_SET_EAPTLS_TLV             (20)*/
	wfaStaSetUAPSD,           /*    WFA_STA_SET_UAPSD_TLV              (21)*/
	wfaStaAssociate,          /*    WFA_STA_ASSOCIATE_TLV              (22)*/
	wfaStaSetEapTTLS,         /*    WFA_STA_SET_EAPTTLS_TLV            (23)*/
	wfaStaSetEapSIM,          /*    WFA_STA_SET_EAPSIM_TLV             (24)*/
	wfaStaSetPEAP,            /*    WFA_STA_SET_PEAP_TLV               (25)*/
	wfaStaSetIBSS,            /*    WFA_STA_SET_IBSS_TLV               (26)*/
	wfaStaGetInfo,            /*    WFA_STA_GET_INFO_TLV               (27)*/
	wfaDeviceGetInfo,         /*    WFA_DEVICE_GET_INFO_TLV            (28)*/
	wfaDeviceListIF,          /*    WFA_DEVICE_LIST_IF_TLV]            (29)*/ 
	wfaStaDebugSet,           /*    WFA_STA_DEBUG_SET                  (30)*/
	wfaStaSetMode,            /*    WFA_STA_SET_MODE                   (31)*/
	wfaStaUpload,             /*    WFA_STA_UPLOAD                     (32)*/
	wfaStaSetWMM,             /*    WFA_STA_SET_WMM                    (33)*/
	wfaStaPresetParameters,   /*    WFA_STA_PRESET_PARAMETERS          (34)*/
	wfaStaSetEapFAST,         /*    WFA_STA_SET_EAPFAST_TLV            (35)*/
	wfaStaSetEapAKA,          /*    WFA_STA_SET_EAPAKA_TLV             (36)*/
	wfaStaSetSystime,         /*    WFA_STA_SET_SYSTIME_TLV            (37)*/
	wfaStaSet11n,             /*    WFA_STA_SET_11n_TLV                (38)*/
	wfaStaSetWireless,        /*    WFA_STA_SET_WIRELESS_TLV           (39)*/
	wfaStaSendADDBA,          /*    WFA_STA_SEND_ADDBA_TLV             (40)*/
	wfaStaSendCoExistMGMT,    /*    WFA_STA_SET_COEXIST_MGMT_TLV       (41)*/
	wfaStaSetRIFS,            /*    WFA_STA_SET_RIFS_TEST_TLV          (42)*/
	wfaStaResetDefault,       /*    WFA_STA_RESET_DEFAULT_TLV          (43)*/
	wfaStaDisconnect,         /*    WFA_STA_DISCONNECT_TLV             (44)*/
	wfaStaReassociate,        /*    WFA_STA_REASSOCIATE_TLV            (45)*/
	wfaStaSetPwrSave,         /*    WFA_STA_SET_PWRSAVE_TLV            (60)*/
	wfaStaSetPowerSave,       /*    WFA_STA_SET_POWER_SAVE_TLV legacy  (61)*/
	wfaStaCliCommand,         /*    WFA_STA_CLI_CMD_TLV                (46)*/

#ifdef WFA_P2P
	wfaStaGetP2pDevAddress,   /*                                       (47)*/
	wfaStaSetP2p,             /*                                       (48)*/  
	wfaStaP2pConnect,         /*                                       (49)*/    
	wfaStaP2pJoin,            /*                                       (50)*/
	wfaStaP2pStartGrpFormation, /*                                     (51)*/
	wfaStaP2pDissolve,        /*                                       (52)*/
	wfaStaSendP2pInvReq,      /*                                       (53)*/  
	wfaStaAcceptP2pReq,       /* WFA_STA_UPLOAD                        (54)*/  
	wfaStaSendP2pProvDisReq,  /*                                       (55)*/    
	wfaStaSetWpsPbc,          /*                                       (56)*/    
	wfaStaWpsReadPin,         /*                                       (57)*/
	wfaStaWpsEnterPin,        /*                                       (58)*/  
	wfaStaGetPsk,             /*                                       (59)*/    
#endif 
};



