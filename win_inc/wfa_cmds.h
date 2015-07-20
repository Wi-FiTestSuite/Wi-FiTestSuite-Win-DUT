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
/*
* wfa_cmds.h:
*   definitions of command types.
*/
#ifndef _WFA_CMDS_H
#define _WFA_CMDS_H

typedef enum wfa_onoffType
{
	WFA_OFF = 0,
	WFA_ON = 1,
} wfaOnOffType;


typedef struct _tg_ping_start
{
	char dipaddr[IPV6_ADDRESS_STRING_LEN];  /**< destination/remote ip address */
	int  frameSize;
	float  frameRate;
	int  duration;
	int  type;
	int  qos;
	int  ipType;
} tgPingStart_t;

typedef struct ca_sta_set_ip_config
{
	char intf[WFA_IF_NAME_LEN];
	int isDhcp;
	char ipaddr[WFA_IP_ADDR_STR_LEN];
	char mask[WFA_IP_MASK_STR_LEN];
	char defGateway[WFA_IP_ADDR_STR_LEN];
	char pri_dns[WFA_IP_ADDR_STR_LEN];
	char sec_dns[WFA_IP_ADDR_STR_LEN];
	int	 ipType;
} caStaSetIpConfig_t;

typedef struct ca_sta_verify_ip_connection
{
	char dipaddr[WFA_IP_ADDR_STR_LEN];
	int timeout;
} caStaVerifyIpConnect_t;


typedef struct ca_sta_set_encryption
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	int encpType;
	char keys[4][32];  /* 26 hex */
	int activeKeyIdx;
} caStaSetEncryption_t;

typedef struct ca_sta_set_mode
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	char mode;
	int encpType;
	int channel;
	char keys[4][32];  /* 26 hex */
	int activeKeyIdx;
} caStaSetMode_t;

typedef enum wfa_supplicant_names
{ 
	eWindowsZeroConfig = 1, 
	/*eMarvell,
	eIntelProset,*/
	eWpaSupplicant, 
	/*eCiscoSecureClient, 
	eOpen1x,*/
	eWin7Supplicant,
	eDefault
} wfaSupplicantName;

typedef enum wfa_preambleType
{ 
	eLong = 1, 
	eShort
} wfaPreambleType;

typedef enum wfa_WirelessMode
{ 
	eModeB = 1,
	eModeBG,
	eModeA,
	eModeABG,
	eModeAN,
	eModeGN,
	eModeNL,
} wfaWirelessMode;


typedef enum wfa_reset_prog
{
	eResetProg11n =1,
} wfaResetProg;

typedef struct ca_sta_preset_parameters
{
	char intf[WFA_IF_NAME_LEN]; 
	wfaSupplicantName supplicant;
	BYTE rtsFlag;
	WORD rtsThreshold;
	BYTE fragFlag;
	WORD fragThreshold;
	BYTE preambleFlag;
	wfaPreambleType preamble;
	BYTE modeFlag;
	wfaWirelessMode wirelessMode;
	BYTE psFlag;
	BYTE legacyPowerSave;
	BYTE wmmFlag;
	BYTE wmmState;
	BYTE reset;
	BYTE ht;
#define NOACK_BE       0
#define NOACK_BK       1
#define NOACK_VI       2
#define NOACK_VO       3
	unsigned char noack_be;
	unsigned char noack_bk;
	unsigned char noack_vi;
	unsigned char noack_vo;
} caStaPresetParameters_t;

typedef struct ca_sta_set_psk
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	BYTE passphrase[64];
	char keyMgmtType[8];  /* WPA-PSK */
	int encpType;    /* TKIP    */
} caStaSetPSK_t;

typedef struct ca_sta_set_eaptls
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	char keyMgmtType[8];
	char encrptype[9];
	char trustedRootCA[128];
	char clientCertificate[128];
} caStaSetEapTLS_t;

typedef struct ca_sta_set_eapttls
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	char username[32];
	char passwd[16];
	char keyMgmtType[8];
	char encrptype[9];
	char trustedRootCA[32];
	char clientCertificate[32];
} caStaSetEapTTLS_t;

typedef struct ca_sta_set_eapsim
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	char username[32];
	char passwd[96];
	char keyMgmtType[8];
	char encrptype[9];
	char tripletCount;
	char tripletSet[3][64];
} caStaSetEapSIM_t;

#ifdef WFA_P2P
typedef struct ca_sta_set_p2p
{
	char intf[WFA_IF_NAME_LEN];
	unsigned char oper_chn_flag;
	WORD oper_chn;

	unsigned char listen_chn_flag;
	WORD listen_chn;  

	unsigned char intent_val_flag;
	WORD intent_val;

	unsigned char p2p_mode_flag;
	char p2p_mode[16];

	unsigned char ssid_flag;
	char ssid[WFA_SSID_NAME_LEN];

	unsigned char presistent_flag;
	int presistent;

	unsigned char intra_bss_flag;
	int intra_bss;

	unsigned char noa_duration_flag;
	int noa_duration;

	unsigned char noa_interval_flag;
	int noa_interval;

	unsigned char noa_count_flag;
	int noa_count;   
} caStaSetP2p_t;

typedef struct ca_sta_p2p_connect
{
	char intf[WFA_IF_NAME_LEN];
	char devId[WFA_P2P_DEVID_LEN];
} caStaP2pConnect_t;

typedef struct ca_sta_p2p_join
{
	char intf[WFA_IF_NAME_LEN];
	char devId[WFA_P2P_DEVID_LEN];
	char ssid[WFA_SSID_NAME_LEN];    
} caStaP2pJoin_t;


typedef struct ca_sta_p2p_start_grp_formation
{
	char intf[WFA_IF_NAME_LEN];
	char devId[WFA_P2P_DEVID_LEN];
	WORD intent_val;
} caStaP2pStartGrpForm_t;

typedef struct ca_sta_p2p_dissolve
{
	char intf[WFA_IF_NAME_LEN];
	char grpId[WFA_P2P_GRP_ID_LEN];   
} caStaP2pDissolve_t;

typedef struct ca_sta_send_p2p_inv_req
{
	char intf[WFA_IF_NAME_LEN];
	unsigned char grpId_flag;
	char grpId[WFA_P2P_GRP_ID_LEN];   
	char devId[WFA_P2P_DEVID_LEN];
} caStaSendP2pInvReq_t;

typedef struct ca_sta_send_p2p_prov_dis_req
{
	char intf[WFA_IF_NAME_LEN];
	char confMethod[16]; 
	char devId[WFA_P2P_DEVID_LEN];
} caStaSendP2pProvDisReq_t;

typedef struct ca_sta_wps_enter_pin
{
	char intf[WFA_IF_NAME_LEN];
	char wpsPin[WFA_WPS_PIN_LEN];
} caStaWpsEnterPin_t;

#endif
typedef struct ca_sta_set_eappeap
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	char username[32];
	char passwd[16];
	char keyMgmtType[8];
	char encrptype[9];
	char trustedRootCA[32];
	char innerEAP[16];
	int peapVersion;
} caStaSetEapPEAP_t;

typedef struct ca_sta_set_eapfast
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	char username[32];
	char passwd[16];
	char keyMgmtType[8];
	char encrptype[9];
	char trustedRootCA[32];
	char innerEAP[16];
	char validateServer;
	char pacFileName[32];
} caStaSetEapFAST_t;

typedef struct ca_sta_set_eapaka
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	char username[32];
	char passwd[96];
	char keyMgmtType[8];
	char encrptype[9];
	char tripletCount;
	char tripletSet[3][96];
} caStaSetEapAKA_t;

typedef struct ca_sta_set_systime
{
	BYTE month;
	BYTE date;
	WORD year;
	BYTE hours;
	BYTE minutes;
	BYTE seconds;
} caStaSetSystime_t;

typedef struct ca_sta_set_11n
{
	char intf[WFA_IF_NAME_LEN];
	BOOL _40_intolerant;
	BOOL addba_reject;
	BOOL ampdu;
	BOOL amsdu;
	BOOL greenfield;
	BOOL sgi20;
	unsigned short stbc_rx;
	unsigned short smps;
	char width[8];
	char mcs_fixedrate[4];
	BOOL mcs32;
	BOOL rifs_test;
	unsigned char txsp_stream;
	unsigned char rxsp_stream;
} caSta11n_t;

#define WFA_ENABLE 1
#define WFA_DISABLE 0
#define WFA_INVALID_BOOL 0xFF

typedef struct ca_sta_set_wireless
{
	char intf[WFA_IF_NAME_LEN];
	char band [8];
#define NOACK_BE       0
#define NOACK_BK       1
#define NOACK_VI       2
#define NOACK_VO       3
	unsigned char noAck[4];
} caStaSetWireless_t;

typedef struct ca_sta_send_addba
{
	char intf[WFA_IF_NAME_LEN];
	unsigned short tid; 
} caStaSetSendADDBA_t;

typedef struct ca_sta_set_rifs
{
	char intf [WFA_IF_NAME_LEN];
	unsigned int action;

} caStaSetRIFS_t;

typedef struct ca_sta_send_coexist_mgmt
{
	char intf[WFA_IF_NAME_LEN];
	char type[16];
	char value[16];
} caStaSendCoExistMGMT_t;

enum {
	eDEF_WPA2 = 1,
	eDEF_WMM = 2,
	eDEF_11N = 3,
	eDEF_VHT = 4,
	eDEF_P2P = 5,
	eDEF_PMF = 6,
	eDEF_MCAST = 7,
	eDEF_WMMPS = 8,
	eDEF_HS20 = 9,
	eDEF_WMMAC = 10,
};

typedef struct ca_sta_reset_default
{
	char intf[WFA_IF_NAME_LEN];
	char set[8];

} caStaResetDefault_t;

typedef struct ca_sta_ReAssociate
{
	char intf[WFA_IF_NAME_LEN];
	char bssid[24];
} caStaReAssoc_t;

/* this is WMMPS power save mode setup: on/off; have to configure 
 STA_SET_UAPSD first before assoc  */
typedef struct ca_sta_SetPwrsave
{
	char intf[WFA_IF_NAME_LEN];
	char mode[24];
} caStaSetPwrSave_t;

/*  this is legancy capi command: off/PSPoll(on)/Fast/PSNonPoll */
typedef struct ca_sta_set_power_save
{
	char intf[WFA_IF_NAME_LEN];
	char powersave[24];
} caStaSetPowerSave_t;

typedef struct ca_sta_set_uapsd
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	int maxSPLength;
	BYTE acBE;
	BYTE acBK;
	BYTE acVI;
	BYTE acVO;
} caStaSetUAPSD_t;

typedef struct ca_sta_set_ibss
{
	char intf[WFA_IF_NAME_LEN];
	char ssid[WFA_SSID_NAME_LEN];
	int channel;
	int encpType;
	char keys[4][32];
	int activeKeyIdx;
} caStaSetIBSS_t;

typedef struct sta_upload
{
	int type;
	int next;     /* sequence number, 0 is the last one */
} caStaUpload_t;

typedef struct sta_debug_set
{
	unsigned short level;
	unsigned short state;
} staDebugSet_t;

typedef struct config
{
	BYTE wmm;
	int  rts_thr ;
	int  frag_thr ;
} wmmconf_t;

typedef struct wmm_tsinfo
{
	unsigned int Reserved1 :1;
	unsigned int TID       :4;
	unsigned int direction :2;
	unsigned int dummy1    :1;
	unsigned int dummy2    :1;
	unsigned int Reserved2 :1;
	unsigned int PSB       :1;
	unsigned int UP        :3;
	unsigned int Reserved3 :2;
	unsigned int Reserved4 :1;
	unsigned int Reserved5 :7;
}wmmtsinfo_t;

typedef struct wmm_tspec
{
	wmmtsinfo_t      tsinfo;
	BOOL Fixed;//The MSDU Fixed Bit
	unsigned short size;//The MSDU Size
	unsigned short maxsize;//MAximum MSDU Size
	unsigned int   min_srvc;//The minimum Service Interval
	unsigned int   max_srvc;//The maximum Service Interval
	unsigned int inactivity;//Inactivity Interval
	unsigned int suspension;//The Suspension Interval
	unsigned int srvc_strt_tim;//The Service Start Time
	unsigned int mindatarate;//The Minimum Data Rate
	unsigned int meandatarate;//The Minimum Data Rate
	unsigned int peakdatarate;//The Minimum Data Rate
	unsigned int burstsize;//The Maximum Burst Size
	unsigned int delaybound;//The Delay Bound
	unsigned int PHYrate;//The minimum PHY Rate
	float sba;//The Surplus Bandwidth Allownce
	unsigned short medium_time;//The medium time
}wmmtspec_t;

typedef struct wmmac_addts
{
	BYTE       dialog_token;
	BYTE       accesscat;
	wmmtspec_t tspec;
} wmmacadd_t;

typedef struct ca_sta_set_wmm
{
	char intf[WFA_IF_NAME_LEN];
	BYTE group;
	BYTE action;
#ifdef WFA_WMM_AC
	BYTE       send_trig;
	char       dipaddr[WFA_IP_ADDR_STR_LEN];
	BYTE       trig_ac;
#endif

	union _action
	{
		wmmconf_t   config;
		wmmacadd_t  addts;
		BYTE        delts;
	} actions;
} caStaSetWMM_t;

typedef struct dut_commands
{
	char intf[WFA_IF_NAME_LEN];
	union _cmds
	{
		int streamId;
		int iftype;
		tgProfile_t profile;
		tgPingStart_t startPing;
		char ssid[WFA_SSID_NAME_LEN];
		char resetProg[16];
		caStaSetIpConfig_t ipconfig;
		caStaVerifyIpConnect_t verifyIp;
		caStaSetEncryption_t wep;       
		caStaSetPSK_t        psk;
		caStaSetEapTLS_t     tls;
		caStaSetEapTTLS_t    ttls;
		caStaSetEapSIM_t     sim;
		caStaSetEapPEAP_t    peap;
		caStaSetUAPSD_t      uapsd;
		caStaSetIBSS_t       ibss;
		caStaUpload_t        upload;
		caStaSetWMM_t        setwmm;
		staDebugSet_t        dbg;	   

		int ipTypeV4V6;

	} cmdsu;    
} dutCommand_t;

#endif