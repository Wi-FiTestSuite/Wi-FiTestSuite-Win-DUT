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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>

#include "wfa_debug.h"
#include "wfa_types.h"
#include "wfa_main.h"
#include "wfa_tlv.h"
#include "wfa_miscs.h"
#include "wfa_ca.h"
#include "wfa_rsp.h"
#include "wfa_sock.h"
#include "wfa_ca_resp.h"

dutCommandRespFuncPtr wfaCmdRespProcFuncTbl[(WFA_STA_RESPONSE_END - WFA_STA_COMMANDS_END)+(WFA_STA_NEW_CMD_RESPONSE_END - WFA_STA_NEW_CMD_RESPONSE_START - 1)] =
{
	caCmdNotDefinedYet,
	wfaGetVersionResp,                   /* WFA_GET_VERSION_RESP_TLV - WFA_STA_COMMANDS_END                  (1) */
	wfaTrafficAgentPingStartResp,        /* WFA_TRAFFIC_SEND_PING_RESP_TLV - WFA_STA_COMMANDS_END            (2) */
	wfaTrafficAgentPingStopResp,         /* WFA_TRAFFIC_STOP_PING_RESP_TLV - WFA_STA_COMMANDS_END            (3) */
	wfaTrafficAgentConfigResp,           /* WFA_TRAFFIC_AGENT_CONFIG_RESP_TLV - WFA_STA_COMMANDS_END         (4) */
	wfaTrafficAgentSendResp,             /* WFA_TRAFFIC_AGENT_SEND_RESP_TLV - WFA_STA_COMMANDS_END           (5) */
	wfaTrafficAgentRecvStartResp,        /* WFA_TRAFFIC_AGENT_RECV_START_RESP_TLV - WFA_STA_COMMANDS_END     (6) */ 
	wfaTrafficAgentRecvStopResp,         /* WFA_TRAFFIC_AGENT_RECV_STOP_RESP_TLV - WFA_STA_COMMANDS_END      (7) */
	wfaTrafficAgentResetResp,            /* WFA_TRAFFIC_AGENT_RESET_RESP_TLV - WFA_STA_COMMANDS_END          (8) */
	caCmdNotDefinedYet,                  /* WFA_TRAFFIC_AGENT_STATUS_RESP_TLV - WFA_STA_COMMANDS_END         (9) */

	wfaStaGetIpConfigResp,               /* WFA_STA_GET_IP_CONFIG_RESP_TLV - WFA_STA_COMMANDS_END           (10) */
	wfaStaSetIpConfigResp,               /* WFA_STA_SET_IP_CONFIG_RESP_TLV - WFA_STA_COMMANDS_END           (11) */
	wfaStaGetMacAddressResp,             /* WFA_STA_GET_MAC_ADDRESS_RESP_TLV - WFA_STA_COMMANDS_END         (12) */
	caCmdNotDefinedYet,                  /* WFA_STA_SET_MAC_ADDRESS_RESP_TLV - WFA_STA_COMMANDS_END         (13) */
	wfaStaIsConnectedResp,               /* WFA_STA_IS_CONNECTED_RESP_TLV - WFA_STA_COMMANDS_END            (14) */
	wfaStaVerifyIpConnectResp,           /* WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV - WFA_STA_COMMANDS_END    (15) */
	wfaStaGetBSSIDResp,                  /* WFA_STA_GET_BSSID_RESP_TLV - WFA_STA_COMMANDS_END               (16) */
	wfaStaGetStatsResp,                  /* WFA_STA_GET_STATS_RESP_TLV - WFA_STA_COMMANDS_END               (17) */
	wfaStaSetEncryptionResp,             /* WFA_STA_SET_ENCRYPTION_RESP_TLV - WFA_STA_COMMANDS_END          (18) */
	wfaStaSetPSKResp,                    /* WFA_STA_SET_PSK_RESP_TLV - WFA_STA_COMMANDS_END                 (19) */
	wfaStaSetEapTLSResp,                 /* WFA_STA_SET_EAPTLS_RESP_TLV - WFA_STA_COMMANDS_END              (20) */
	wfaStaSetUAPSDResp,                  /* WFA_STA_SET_UAPSD_RESP_TLV - WFA_STA_COMMANDS_END               (21) */
	wfaStaAssociateResp,                 /* WFA_STA_ASSOCIATE_RESP_TLV - WFA_STA_COMMANDS_END               (22) */
	wfaStaSetEapTTLSResp,                 /* WFA_STA_SET_EAPTTLS_RESP_TLV - WFA_STA_COMMANDS_END             (23) */
	wfaStaSetEapSIMResp,                 /* WFA_STA_SET_EAPSIM_RESP_TLV - WFA_STA_COMMANDS_END              (24) */
	wfaStaSetEapPEAPResp,                /* WFA_STA_SET_PEAP_RESP_TLV - WFA_STA_COMMANDS_END                (25) */
	wfaStaSetIBSSResp,                   /* WFA_STA_SET_IBSS_RESP_TLV - WFA_STA_COMMANDS_END                (26) */
	wfaStaGetInfoResp,                   /* WFA_STA_GET_INFO_RESP_TLV - WFA_STA_COMMANDS_END                (27) */
	wfaDeviceGetInfoResp,                /* WFA_DEVICE_GET_INFO_RESP_TLV - WFA_STA_COMMANDS_END             (28) */
	wfaDeviceListIFResp,                 /* WFA_DEVICE_LIST_IF_RESP_TLV - WFA_STA_COMMANDS_END              (29) */
	wfaStaDebugSetResp,                  /* WFA_STA_DEBUG_SET_RESP_TLV - WFA_STA_COMMANDS_END               (30) */
	wfaStaSetModeResp,                   /* WFA_STA_SET_MODE_RESP_TLV - WFA_STA_COMMANDS_END                (31) */
	wfaStaUploadResp,                    /* WFA_STA_UPLOAD_RESP_TLV - WFA_STA_COMMANDS_END                  (32) */
	wfaStaSetWMMResp,                    /* WFA_STA_SET_WMM_RESP_TLV - WFA_STA_COMMANDS_END                  (33) */
	wfaStaPresetParametersResp,          /* (34) */
	wfaStaSetEapFASTResp,                /* (35)*/
	wfaStaSetEapAKAResp,                 /* (36)*/
	wfaStaSetSystimeResp,                /* (37)*/
	wfaStaSet11nResp,                    /* (38)*/
	wfaStaSetWirelessResp,               /* (39)*/
	wfaStaSendADDBAResp,                 /* (40)*/
	wfaStaCoexMgmtResp,
	wfaStaRifsTestResp,
	wfaStaResetDefaultResp,
	wfaStaDisconnectResp,
	wfaStaReAssociationResp,
	wfaStaSetPwrSaveResp,                /*    60 */
	wfaStaSetPowerSaveResp,              /*    61 legacy power save */
	wfaStaCliCmdResp,
#ifdef WFA_P2P

	wfaStaGetP2pDevAddressResp,          
	wfaStaSetP2pResp,   
	wfaStaP2pConnectResp,
	wfaStaP2pJoinResp,            
	wfaStaP2pStartGrpFormResp,         

	wfaStaP2pDissolveResp,            
	wfaStaSendP2pInvReqResp,  
	wfaStaAcceptP2pInvReqResp,  
	wfaStaSendP2pProvDisReqResp,     
	wfaStaSetWpsPbcResp, 

	wfaStaWpsReadPinResp,      
	wfaStaWpsEnterPinResp,  
	wfaStaGetPskResp,      
#endif

};

int caCmdNotDefinedYet(BYTE *cmdBuf)
{
	int done;

	sprintf(wfaCAAgetData.gRespStr, "status,ERROR,Command Not Defined\r\n");
	DPRINT_INFO(WFA_OUT, " %s\n", wfaCAAgetData.gRespStr);
	/* make sure if getting send error, will close the socket */
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	done = 0;
	return done;
}

int wfaStaSetUAPSDResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *setUAPSDResp = (dutCmdResponse_t *) (cmdBuf + 4);

	switch(setUAPSDResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "STA Set UAPSD running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, " %s\n", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaVerifyIpConnectResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *verifyResp = (dutCmdResponse_t *)(cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaVerifyIpConnectResp\n");
	switch(verifyResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaVerifyConnect running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,connected,%i\r\n", verifyResp->cmdru.connected);
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}


int wfaStaSetIpConfigResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *setIpConfigResp = (dutCmdResponse_t *)(cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSetIpConfigResp ...\n");

	switch(setIpConfigResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "traffic agent config running ...\n");
		done = 1;
		break;
	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	return done;
}

int wfaStaIsConnectedResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *connectedResp = (dutCmdResponse_t *)(cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaIsConnectedResp ...\n");
	switch(connectedResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaIsConnectd running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,connected,%i\r\n", connectedResp->cmdru.connected);
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;
	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	return done;
}

int wfaStaGetIpConfigResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *getIpConfigResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaGetIpConfigResp ...\n");
	switch(getIpConfigResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaGetIpConfig running ...\n");
		done = 1;
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	case STATUS_COMPLETE:
		if(getIpConfigResp->cmdru.getIfconfig.ipTypeV6 == 1)
		{
			sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,ip,%s\r\n", getIpConfigResp->cmdru.getIfconfig.ipV6addr);
		}
		else
		{
			if(strlen(getIpConfigResp->cmdru.getIfconfig.dns[0]) == 0)
				*getIpConfigResp->cmdru.getIfconfig.dns[0] = '\0';
			if(strlen(getIpConfigResp->cmdru.getIfconfig.dns[1]) == 0)
				*getIpConfigResp->cmdru.getIfconfig.dns[1] = '\0';

			sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,dhcp,%i,ip,%s,mask,%s,primary-dns,%s\r\n", getIpConfigResp->cmdru.getIfconfig.isDhcp,
				getIpConfigResp->cmdru.getIfconfig.ipaddr,
				getIpConfigResp->cmdru.getIfconfig.mask,
				getIpConfigResp->cmdru.getIfconfig.dns[0]);
		}
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	return done;
}

int wfaGetVersionResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *getverResp =(dutCmdResponse_t *)(cmdBuf + 4);

	switch(getverResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaGetVersion running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,version,%s\r\n", getverResp->cmdru.version);
		break;
	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	return done ;
}

int wfaStaGetInfoResp(BYTE *cmdBuf)
{
	dutCmdResponse_t *infoResp = (dutCmdResponse_t *)(cmdBuf + 4);
	int done = 0;

	switch(infoResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaGetInfo running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,%s\r\n", infoResp->cmdru.info);
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaTrafficAgentConfigResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *agtConfigResp = (dutCmdResponse_t *)(cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaTrafficAgentConfigResp ...\n");
	switch(agtConfigResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaTrafficAgentConfig running ...\n");
		done = 1;
		break;
	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,streamID,%i\r\n", agtConfigResp->streamId);
		break;
	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	return done;
}

int wfaTrafficAgentSendResp(BYTE *cmdBuf)
{
	int done=1,i;
	char copyBuf[64];
	int errorStatus = 0;
	wfaTLV *ptlv = (wfaTLV *)cmdBuf;
	int len = ptlv->len;
	int numStreams;
	dutCmdResponse_t *statResp = (dutCmdResponse_t *)(cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaTrafficAgentSendResp ...\n");

	numStreams = (len/sizeof(dutCmdResponse_t));
	for(i=0; i<numStreams; i++) 
	{
		if(statResp->status != STATUS_COMPLETE) 
		{
			errorStatus = 1;
		}
	}

	if(errorStatus) 
	{
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR");
	} 
	else 
	{
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,streamID,");
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, " %i", statResp[i].streamId);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, ",txFrames,", 10);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, "%i ", statResp[i].cmdru.stats.txFrames);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, ",rxFrames,", 10);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, "%i ", statResp[i].cmdru.stats.rxFrames);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, ",txPayloadBytes,", 16);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, "%i ", statResp[i].cmdru.stats.txPayloadBytes);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, ",rxPayloadBytes,", 16);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, "%i ", statResp[i].cmdru.stats.rxPayloadBytes);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}
		strncat(wfaCAAgetData.gRespStr, ",outOfSequenceFrames,", 20);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, "%i ", statResp[i].cmdru.stats.outOfSequenceFrames);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, "\r\n", 4);
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	return done;
}

int wfaTrafficAgentRecvStartResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *recvStartResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaTrafficAgentRecvStartResp ...\n");
	switch(recvStartResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaTrafficAgentRecvStart running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaTrafficAgentRecvStopResp(BYTE *cmdBuf)
{
	int done=1;
	int i = 0;
	int errorStatus = 0;
	char copyBuf[64];
	BYTE *dutRsp = cmdBuf+4;
	BYTE *startRsp = dutRsp; 
	wfaTLV *ptlv = (wfaTLV *)cmdBuf;
	int len = ptlv->len;
	int numStreams = len/sizeof(dutCmdResponse_t);

	//DPRINT_INFO(WFA_OUT, "Entering wfaTrafficAgentRecvStopResp ...\n");

	dutCmdResponse_t statResp[WFA_MAX_TRAFFIC_STREAMS];

	DPRINT_INFO(WFA_OUT, "Entering wfaTrafficAgentRecvStopResp ...\n");
	for(i=0; i<numStreams; i++) 
	{
		dutRsp = startRsp + i * sizeof(dutCmdResponse_t);
		memcpy(&statResp[i], dutRsp, sizeof(dutCmdResponse_t));
	}
	for(i=0; i<numStreams; i++) 
	{
		if(statResp[i].status != STATUS_COMPLETE)
		{
			errorStatus = 1;
		}
	}

	if(errorStatus) 
	{
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR");
	} 
	else 
	{
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,streamID,");
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, " %d", statResp[i].streamId); 
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, ",txFrames,", 10);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, " %u", statResp[i].cmdru.stats.txFrames);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, ",rxFrames,", 10);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, " %u", statResp[i].cmdru.stats.rxFrames);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, ",txPayloadBytes,", 16);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, " %llu", statResp[i].cmdru.stats.txPayloadBytes);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, ",rxPayloadBytes,", 16);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, " %llu", statResp[i].cmdru.stats.rxPayloadBytes);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}

		strncat(wfaCAAgetData.gRespStr, ",outOfSequenceFrames,", 20);
		for(i=0; i<numStreams; i++) 
		{
			sprintf(copyBuf, " %d", statResp[i].cmdru.stats.outOfSequenceFrames);
			strncat(wfaCAAgetData.gRespStr, copyBuf, sizeof(copyBuf)-1);
		}
		strncat(wfaCAAgetData.gRespStr, "\r\n", 4);
	}

	DPRINT_INFO(WFA_OUT, " %s\n", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	return done;
}

int wfaTrafficAgentResetResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *resetResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaTrafficAgentResetResp ...\n");
	switch(resetResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaTrafficAgentReset running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, " %s\n", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaTrafficAgentPingStartResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *staPingResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaTrafficAgentPingStartResp ...\n");

	switch(staPingResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaTrafficAgentPingStart running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,streamID,%i\r\n", staPingResp->streamId);
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, " %s\n", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaTrafficAgentPingStopResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *stpResp = (dutCmdResponse_t *) (cmdBuf + 4);

	switch(stpResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaTrafficAgentPingStop running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		{
			sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,sent,%d,replies,%d\r\n",
				stpResp->cmdru.pingStp.sendCnt,
				stpResp->cmdru.pingStp.repliedCnt);
			break;
		}

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	return done;
}

int wfaStaGetMacAddressResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *getmacResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaGetMacAddressResp ...\n");
	switch(getmacResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaGetMacAddress running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,mac,%s\r\n", getmacResp->cmdru.mac);
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,mac,00:00:00:00:00:00\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,mac,00:00:00:00:00:00\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaGetBSSIDResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *getBssidResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaGetMacAddressResp ...\n");
	switch(getBssidResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaGetBSSID running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,bssid,%s\r\n", getBssidResp->cmdru.bssid);
		break;
	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,mac,00:00:00:00:00:00\r\n");
		break;
	default:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,mac,00:00:00:00:00:00\r\n");
	}

	DPRINT_INFO(WFA_OUT, " %s\n", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetEncryptionResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *getBssidResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaGetBSSIDResp ...\n");
	switch(getBssidResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSetEncryption running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,bssid,%s\r\n", getBssidResp->cmdru.bssid);
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, " %s\n", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetEapTLSResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *setEapTLSResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSetEapTLSResp ...\n");
	switch(setEapTLSResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSetEapTLS running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, " %s\n", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;

}

int wfaStaSetPSKResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *setPSKResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSetPSKResp ...\n");
	switch(setPSKResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSetPSK running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetEapTTLSResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *setEapTTLSResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSetEapTTLSResp ...\n");
	switch(setEapTTLSResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSetEapTTLS running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, sizeof(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetEapPEAPResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *setEapPeapResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSetEapPEAPResp ...\n");
	switch(setEapPeapResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSetEapPEAP running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, sizeof(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetEapSIMResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *setEapSIMResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSetEapSIMResp ...\n");
	switch(setEapSIMResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSetEapSIM running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, sizeof(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaAssociateResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *assocResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaAssociateResp ...\n");
	switch(assocResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaAssociate running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetIBSSResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *setIBSSResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSetIBSSResp ...\n");
	switch(setIBSSResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaIBSS running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaGetStatsResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *getStatsResp = (dutCmdResponse_t *) (cmdBuf + 4);
	caStaGetStatsResp_t *stats = &getStatsResp->cmdru.ifStats;

	DPRINT_INFO(WFA_OUT, "Entering wfaStaGetStatsResp ...\n");

	switch(getStatsResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaGetStats running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,txFrames,%i,rxFrames,%i,txMulticast,%i,rxMulticast,%i,fcsErrors,%i,txRetries,%i\r\n",
			stats->txFrames, stats->rxFrames, stats->txMulticast, stats->rxMulticast, stats->fcsErrors, stats->txRetries);
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaDeviceGetInfoResp(BYTE *cmdBuf)
{
	int done=1;
	dutCmdResponse_t *devInfoResp = (dutCmdResponse_t *) (cmdBuf + 4);
	caDeviceGetInfoResp_t *dinfo = &devInfoResp->cmdru.devInfo;

	switch(devInfoResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaDeviceGetInfo running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,vendor,%s,model,%s,version,%s\r\n",
			dinfo->vendor, dinfo->model, dinfo->version);
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaDeviceListIFResp(BYTE *cmdBuf)
{
	int done=0, i;
	dutCmdResponse_t *devListIfResp = (dutCmdResponse_t *) (cmdBuf + 4);
	caDeviceListIFResp_t *ifResp = &devListIfResp->cmdru.ifList;

	switch(devListIfResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaDeviceListIF running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		if(ifResp->iftype == IF_80211)
		{
			sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,interfaceType,802.11,interfaceID");
		}
		else if(ifResp->iftype == IF_ETH)
			sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,interfaceType,Ethernet,interfaceID");

		for(i=0; i<1; i++)
		{
			if(ifResp->ifs[i][0] != '\0')
			{
				strncat(wfaCAAgetData.gRespStr,",", 4);
				strncat(wfaCAAgetData.gRespStr, ifResp->ifs[i], sizeof(ifResp->ifs[i]));
				strncat(wfaCAAgetData.gRespStr, "\r\n", 4);
			}
		}

		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaDebugSetResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *debugResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaDebugSetResp ...\n");
	switch(debugResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaDebugSet running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetModeResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *setModeResp = (dutCmdResponse_t *) (cmdBuf + 4);

	switch(setModeResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaSetMode running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}


int wfaStaUploadResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *uploadResp = (dutCmdResponse_t *) (cmdBuf + 4);
	caStaUploadResp_t *upld = &uploadResp->cmdru.uld;

	switch(uploadResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaUpload running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,code,%i,%s\r\n", 
			upld->seqnum, upld->bytes);
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetWMMResp(BYTE *cmdBuf)
{
	return wfaStandardReturn(cmdBuf);
}

int wfaStaPresetParametersResp(BYTE *cmdBuf)
{
	return wfaStandardReturn(cmdBuf);
}

int wfaStaSetEapFASTResp(BYTE *cmdBuf)
{
	return wfaStandardReturn(cmdBuf);
}
int wfaStaSetEapAKAResp(BYTE *cmdBuf)
{
	return wfaStandardReturn(cmdBuf);
}

int wfaStaSetSystimeResp(BYTE *cmdBuf)
{
	return wfaStandardReturn(cmdBuf);
}

int wfaStaSet11nResp(BYTE *cmdBuf)
{

	DPRINT_INFO(WFA_OUT, " START - wfaStaSet11nResp");
	return wfaStandardReturn(cmdBuf);
}

int wfaStaSetWirelessResp(BYTE *cmdBuf)
{
	DPRINT_INFO(WFA_OUT, " START - wfaStaSetWirelessResp");
	return wfaStandardReturn(cmdBuf);
}

int wfaStaSendADDBAResp(BYTE *cmdBuf)
{
	DPRINT_INFO(WFA_OUT, " START - wfaStaSendADDBAResp");
	return wfaStandardReturn(cmdBuf);
}

int wfaStaResetDefaultResp(BYTE *cmdBuf)
{
	DPRINT_INFO(WFA_OUT, " START - wfaStaResetDefaultResp");
	return wfaStandardReturn(cmdBuf);
}

int wfaStaCoexMgmtResp(BYTE *cmdBuf)
{
	DPRINT_INFO(WFA_OUT, " START - wfaStaCoexMgmtResp");
	return wfaStandardReturn(cmdBuf);
}

int wfaStaRifsTestResp(BYTE *cmdBuf)
{
	DPRINT_INFO(WFA_OUT, " START - wfaStaRifsTestResp");
	return wfaStandardReturn(cmdBuf);
}


int wfaStaDisconnectResp(BYTE *cmdBuf)
{
	DPRINT_INFO(WFA_OUT, " START - wfaStaDisconnectResp");
	return wfaStandardReturn(cmdBuf);
}

int wfaStaReAssociationResp(BYTE *cmdBuf)
{
	DPRINT_INFO(WFA_OUT, " START - wfaStaReAssociationResp");
	return wfaStandardReturn(cmdBuf);
}

int wfaStaCliCmdResp(BYTE *cmdBuf)
{
	int done=0;
	caStaCliCmdResp_t *staCliCmdResp = (caStaCliCmdResp_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaCliCmdResp ...\n");
	switch(staCliCmdResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaCliCmdResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		if(staCliCmdResp->resFlag == 1)
		{
			sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,%s\r\n", staCliCmdResp->result);
		}
		else
		{
			sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		}
		break;

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStandardReturn(BYTE *cmdBuf) {
	int done=0;
	dutCmdResponse_t *Resp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering Standard Resp ...\n");
	switch(Resp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaSta standard running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;  

	case STATUS_ERROR:
		sprintf(wfaCAAgetData.gRespStr, "status,ERROR\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));
	return done;

}

int wfaStaSetPwrSaveResp(BYTE *cmdBuf) /* WMMPS  */
{
	DPRINT_INFO(WFA_OUT, " START - wfaStaSetPwrSaveResp ");
	return wfaStandardReturn(cmdBuf);
}

int wfaStaSetPowerSaveResp(BYTE *cmdBuf) /* legacy power save */
{
	DPRINT_INFO(WFA_OUT, " START -wfaStaSetPowerSaveResp - legacy ");
	return wfaStandardReturn(cmdBuf);
}

#ifdef WFA_P2P
int wfaStaGetP2pDevAddressResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pDevAddResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaGetP2pDevAddressResp ...\n");
	switch(p2pDevAddResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaGetP2pDevAddressResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,devid,%s\r\n", p2pDevAddResp->cmdru.devid);
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetP2pResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *staSetp2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSetP2pResp ...\n");
	switch(staSetp2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSetP2pResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaP2pConnectResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaP2pConnectResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaP2pConnectResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,groupid,%s\r\n", p2pResp->cmdru.grpid);
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}



int wfaStaP2pJoinResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaP2pJoinResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaP2pJoinResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}



int wfaStaP2pStartGrpFormResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaP2pStartGrpFormResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaP2pStartGrpFormResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,result,%s,groupid,%s\r\n", p2pResp->cmdru.p2presult,p2pResp->cmdru.grpid);
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}


int wfaStaP2pDissolveResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaP2pDissolveResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaP2pDissolveResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSendP2pInvReqResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSendP2pInvReqResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSendP2pInvReqResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}


int wfaStaAcceptP2pInvReqResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaAcceptP2pInvReqResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaAcceptP2pInvReqResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSendP2pProvDisReqResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSendP2pProvDisReqResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSendP2pProvDisReqResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaSetWpsPbcResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaSetWpsPbcResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaSetWpsPbcResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaWpsReadPinResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaWpsReadPinResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaWpsReadPinResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,pin,%s\r\n", p2pResp->cmdru.wpsPin);
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaWpsEnterPinResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);

	DPRINT_INFO(WFA_OUT, "Entering wfaStaWpsEnterPinResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaWpsEnterPinResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE\r\n");
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}

int wfaStaGetPskResp(BYTE *cmdBuf)
{
	int done=0;
	dutCmdResponse_t *p2pResp = (dutCmdResponse_t *) (cmdBuf + 4);
	caP2pStaGetPskResp_t *pskInfo= &p2pResp->cmdru.pskInfo;

	DPRINT_INFO(WFA_OUT, "Entering wfaStaGetPskResp ...\n");
	switch(p2pResp->status)
	{
	case STATUS_RUNNING:
		DPRINT_INFO(WFA_OUT, "wfaStaGetPskResp running ...\n");
		done = 1;
		break;

	case STATUS_COMPLETE:
		sprintf(wfaCAAgetData.gRespStr, "status,COMPLETE,passphrase,%s,ssid,%s\r\n", pskInfo->passPhrase,pskInfo->ssid);
		break;

	default:
		sprintf(wfaCAAgetData.gRespStr, "status,INVALID\r\n");
	}

	DPRINT_INFO(WFA_OUT, "%s", wfaCAAgetData.gRespStr);
	wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)wfaCAAgetData.gRespStr, strlen(wfaCAAgetData.gRespStr));

	return done;
}


#endif



