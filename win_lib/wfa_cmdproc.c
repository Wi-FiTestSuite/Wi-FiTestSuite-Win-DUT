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
#include "wfa_ca.h"

extern void printProfile(tgProfile_t *);
int wfaStandardBoolParsing (char *str);

/* command KEY WORD String table */
typeNameStr_t keywordStr[] =
{
	{ KW_PROFILE,      "profile",                 NULL},
	{ KW_DIRECTION,    "direction",               NULL},
	{ KW_DIPADDR,      "destination",             NULL},
	{ KW_DPORT,        "destinationport",         NULL},
	{ KW_SIPADDR,      "source",                  NULL},
	{ KW_SPORT,        "sourceport",              NULL},
	{ KW_FRATE,        "framerate",               NULL},
	{ KW_DURATION,     "duration",                NULL},
	{ KW_PLOAD,        "payloadsize",             NULL},
	{ KW_TCLASS,       "trafficClass",            NULL},    /* It is to indicate WMM traffic pattern                */
	{ KW_STREAMID,     "streamid",                NULL},
	{ KW_STARTDELAY,   "startdelay",              NULL},    /* It is used to schedule multi-stream test such as WMM */
	{ KW_NUMFRAME,     "numframes",               NULL},
	{ KW_USESYNCCLOCK, "useSyncClock",            NULL},
	{ KW_USERPRIORITY, "userpriority",            NULL},
	{ KW_MAXCNT,       "maxcnt",                  NULL},
	{ KW_HTI,           "hti",                    NULL},
}; 

/* profile type string table */
typeNameStr_t profileStr[] =
{
	{ PROF_FILE_TX,    "file_transfer",           NULL},
	{ PROF_MCAST,      "multicast",               NULL},
	{ PROF_IPTV,       "iptv",                    NULL},    /* This is used for WMM, confused? */
	{ PROF_TRANSC,     "transaction",             NULL},
	{ PROF_START_SYNC, "start_sync",              NULL},
	{ PROF_UAPSD,      "uapsd",                   NULL}
};

/* direction string table */
typeNameStr_t direcStr[] =
{
	{ DIRECT_SEND,  "send",                       NULL},
	{ DIRECT_RECV,  "receive",                    NULL}
};

/*
* cmdProcNotDefinedYet(): a dummy function
*/
int cmdProcNotDefinedYet(char *pcmdStr, char *buf, int *len)
{
	DPRINT_INFO(WFA_OUT, "The command processing function not defined.\n");

	/* need to send back a response */

	return (WFA_SUCCESS);
}



/*
*  xcCmdProcGetVersion(): process the command get_version string from TM 
*                         to convert it into a internal format
*  input:        pcmdStr -- a string pointer to the command string
*/
int xcCmdProcGetVersion(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	DPRINT_INFO(WFA_OUT, "start xcCmdProcGetVersion ...\n");

	if(aBuf == NULL)
		return WFA_FAILURE;

	/* encode the tag without values */
	wfaEncodeTLV(WFA_GET_VERSION_TLV, 0, NULL, aBuf);

	*aLen = 4;

	return WFA_SUCCESS;
}

/*
*  xcCmdProcAgentConfig(): process the command traffic_agent_config string 
*                          from TM to convert it into a internal format
*  input:        pcmdStr -- a string pointer to the command string
*/
int xcCmdProcAgentConfig(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	char *str;
	int i = 0, j=0, kwcnt = 0;
	int userPrio = 0;

	wfaTLV *hdr = (wfaTLV *)aBuf;
	tgProfile_t tgpf = {0, 0, "", -1, "", -1, 0, 0, 0, TG_WMM_AC_BE, 0};
	tgProfile_t *pf = &tgpf;

	DPRINT_INFO(WFA_OUT, "start xcCmdProcAgentConfig ...\n");
	DPRINT_INFO(WFA_OUT, "params:  %s\n", pcmdStr);

	if(aBuf == NULL)
		return WFA_FAILURE;

	while((str = strtok_r(NULL, ",", (char **)&pcmdStr)) != NULL) 
	{
		for(i = 0; i<sizeof(keywordStr)/sizeof(typeNameStr_t); i++)
		{
			if(strcasecmp(str, keywordStr[i].name) == 0)
			{
				switch(keywordStr[i].type)
				{
				case  KW_PROFILE:
					str = strtok_r(NULL, ",", (char **)&pcmdStr);
					if(isString(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect profile keyword format\n");
						return WFA_FAILURE;               
					}

					for(j = 0; j < PROF_LAST; j++)
						if(strcasecmp(str, profileStr[j].name) == 0)
						{
							pf->profile = profileStr[j].type; 
						}

						DPRINT_INFO(WFA_OUT, "profile type %i\n", pf->profile);
						kwcnt++;
						str = NULL;
						break;

				case KW_DIRECTION:
					str = strtok_r(NULL, ",", (char **)&pcmdStr);
					if(isString(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect direction keyword format\n");
						return WFA_FAILURE;               
					}

					if(strcasecmp(str, "send") == 0)
					{
						pf->direction = DIRECT_SEND;  
					}
					else if(strcasecmp(str, "receive") == 0)
					{
						pf->direction = DIRECT_RECV;
					}
					else
					{
						DPRINT_INFO(WFA_OUT, "Undefined Traffic Flow Direction\n");
					}

					DPRINT_INFO(WFA_OUT, "direction %i\n", pf->direction);
					kwcnt++;
					str = NULL;
					break;

				case KW_DIPADDR: /* dest ip address */
					memcpy(pf->dipaddr, strtok_r(NULL, ",", &pcmdStr), IPV4_ADDRESS_STRING_LEN);
					if(isIpV4Addr(pf->dipaddr) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect ipaddr format\n");
						return WFA_FAILURE;               
					}
					DPRINT_INFO(WFA_OUT, "dipaddr %s\n", pf->dipaddr);

					kwcnt++;
					str = NULL;
					break;

				case KW_DPORT:
					str = strtok_r(NULL, ",", &pcmdStr);  
					if(isNumber(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect port number format\n");
						return WFA_FAILURE;
					}
					DPRINT_INFO(WFA_OUT, "dport %s\n", str);
					pf->dport = atoi(str);

					kwcnt++;
					str = NULL;
					break;

				case KW_SIPADDR:
					memcpy(pf->sipaddr, strtok_r(NULL, ",", &pcmdStr), IPV4_ADDRESS_STRING_LEN);

					if(isIpV4Addr(pf->sipaddr) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect ipaddr format\n");
						return WFA_FAILURE;               
					}
					DPRINT_INFO(WFA_OUT, "sipaddr %s\n", pf->sipaddr);
					kwcnt++;
					str = NULL;
					break;

				case KW_SPORT:
					str = strtok_r(NULL, ",", &pcmdStr);  
					if(isNumber(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect port number format\n");
						return WFA_FAILURE;
					}
					DPRINT_INFO(WFA_OUT, "sport %s\n", str);
					pf->sport = atoi(str);

					kwcnt++;
					str = NULL;
					break; 

				case KW_FRATE:
					str = strtok_r(NULL, ",", &pcmdStr);  
					if(isNumber(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect frame rate format\n");
						return WFA_FAILURE;
					}
					DPRINT_INFO(WFA_OUT, "framerate %s\n", str);
					pf->rate = atoi(str);
					kwcnt++;
					str = NULL;
					break;

				case KW_DURATION:
					str = strtok_r(NULL, ",", &pcmdStr);  
					if(isNumber(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect duration format\n");
						return WFA_FAILURE;
					}
					DPRINT_INFO(WFA_OUT, "duration %s\n", str);
					pf->duration = atoi(str); 
					kwcnt++;
					str = NULL;
					break;

				case KW_PLOAD:
					str = strtok_r(NULL, ",", &pcmdStr);  
					if(isNumber(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect payload format\n");
						return WFA_FAILURE;
					}
					DPRINT_INFO(WFA_OUT, "payload %s\n", str);
					pf->pksize = atoi(str); 
					kwcnt++;
					str = NULL;
					break;

				case KW_STARTDELAY:
					str = strtok_r(NULL, ",", &pcmdStr);
					if(isNumber(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect startDelay format\n");
						return WFA_FAILURE;
					}
					DPRINT_INFO(WFA_OUT, "startDelay %s\n", str);
					pf->startdelay = atoi(str); 
					kwcnt++;
					str = NULL;
					break;

				case KW_TCLASS:
					str = strtok_r(NULL, ",", &pcmdStr);  

					// if user priority is used, tclass is ignored.
					if(userPrio == 1)
						break;

					if(strcasecmp(str, "voice") == 0)
					{
						pf->trafficClass = TG_WMM_AC_VO; 
					}
					else if(strcasecmp(str, "voice2") == 0)
					{
						pf->trafficClass = TG_WMM_AC_VO2; 
					}
					else if(strcasecmp(str, "Video") == 0)
					{
						pf->trafficClass = TG_WMM_AC_VI; 
					} 
					else if(strcasecmp(str, "Video2") == 0)
					{
						pf->trafficClass = TG_WMM_AC_VI2; 
					} 
					else if(strcasecmp(str, "Background") == 0)
					{
						pf->trafficClass = TG_WMM_AC_BK; 
					} 
					else if(strcasecmp(str, "Background2") == 0)
					{
						pf->trafficClass = TG_WMM_AC_BK2; 
					} 
					else if(strcasecmp(str, "BestEffort") == 0)
					{
						pf->trafficClass = TG_WMM_AC_BE; 
					}
					else if(strcasecmp(str, "BestEffort2") == 0)
					{
						pf->trafficClass = TG_WMM_AC_BE2; 

					}
					else
					{
						pf->trafficClass = TG_WMM_AC_BE;
					}

					kwcnt++;
					str = NULL;
					break;

				case KW_STREAMID:
					kwcnt++;
					break;

				case KW_USERPRIORITY:
					str = strtok_r(NULL, ",", &pcmdStr);

					if( strcasecmp(str, "6") == 0 )
					{
						pf->trafficClass = TG_WMM_AC_UP6;
					}
					if( strcasecmp(str, "7") == 0 )
					{
						pf->trafficClass = TG_WMM_AC_UP7;
					}
					if( strcasecmp(str, "5") == 0 )
					{
						pf->trafficClass = TG_WMM_AC_UP5;
					}
					if( strcasecmp(str, "4") == 0 )
					{
						pf->trafficClass = TG_WMM_AC_UP4;
					}
					if( strcasecmp(str, "1") == 0 )
					{
						pf->trafficClass = TG_WMM_AC_UP1;
					}
					if( strcasecmp(str, "2") == 0 )
					{
						pf->trafficClass = TG_WMM_AC_UP2;
					}
					if( strcasecmp(str, "0") == 0 )
					{
						pf->trafficClass = TG_WMM_AC_UP0;
					}
					if( strcasecmp(str, "3") == 0 )
					{
						pf->trafficClass = TG_WMM_AC_UP3;
					}
					else  // default
					{
						pf->trafficClass =TG_WMM_AC_UP0;
					}

					// if User Priority is used
					userPrio = 1;

					kwcnt++;
					str = NULL;
					break;

				case KW_NUMFRAME:
					str = strtok_r(NULL, ",", &pcmdStr);  
					if(isNumber(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect numframe format\n");
						return WFA_FAILURE;
					}
					DPRINT_INFO(WFA_OUT, "num frame %s\n", str);
					kwcnt++;
					str = NULL;
					break;

				case KW_USESYNCCLOCK:
					str = strtok_r(NULL, ",", &pcmdStr);  
					if(isNumber(str) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "Incorrect sync clock format\n");
						return WFA_FAILURE;
					}
					DPRINT_INFO(WFA_OUT, "sync clock %s\n", str);
					kwcnt++;
					str = NULL;
					break;

					//------------------------------------------------
				case KW_HTI:
					str = strtok_r(NULL, ",", (char **)&pcmdStr);            

					if(strcasecmp(str, "on") == 0)
					{
						pf->hti = WFA_ON;  
					}
					else if(strcasecmp(str, "off") == 0)
					{
						pf->hti = WFA_OFF; 
					}
					else
					{
						DPRINT_ERR(WFA_ERR, "Incorrect HTI vlaue\n");
						return WFA_FAILURE;
					}
					DPRINT_INFO(WFA_OUT, "HTI Value %s\n", str);
					kwcnt++;
					str = NULL;
					break;
					//-----------------------------------------------

				default:
					;
				} /* switch */

				if(str==NULL)
					break;
			}  /* if */
		} /* for */
	} /* while */

	printProfile(pf);
	hdr->tag =  WFA_TRAFFIC_AGENT_CONFIG_TLV;   
	hdr->len = sizeof(tgProfile_t);

	memcpy(aBuf+4, pf, sizeof(tgpf));

	*aLen = 4+sizeof(tgProfile_t);

	return WFA_SUCCESS;
}

/*
* xcCmdProcAgentSend(): Process and send the Control command 
*                       "traffic_agent_send"
* input - pcmdStr  parameter string pointer
* return - WFA_SUCCESS or WFA_FAILURE;
*/
int xcCmdProcAgentSend(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	wfaTLV *hdr = (wfaTLV *)aBuf;
	char *str, *sid;
	int strid;
	int id_cnt = 0;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, 512);

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcAgentSend ...\n");
	/* there is only one stream for baseline. Will support
	* multiple streams later.
	*/
	str = strtok_r(NULL, ",", &pcmdStr);  

	if(str == NULL || str[0] == '\0')
		return WFA_FAILURE;

	/* take the stream ids */
	if(strcasecmp(str, "streamid") != 0)
	{
		DPRINT_ERR(WFA_ERR, "invalid type name\n");
		return WFA_FAILURE;
	}

	/*
	* To handle there are multiple stream ids such as WMM
	*/
	while(1)
	{
		sid = strtok_r (NULL, " ", &pcmdStr);  
		if(sid == NULL)
			break;

		DPRINT_INFO(WFA_OUT, "sid %s\n", sid);
		if(isNumber(sid) == WFA_FAILURE)
			continue;

		strid = atoi(sid);
		DPRINT_INFO(WFA_OUT, "id %i\n", strid);
		id_cnt++; 

		memcpy(aBuf+4*id_cnt, (char *)&strid, 4);
	}

	hdr->tag =  WFA_TRAFFIC_AGENT_SEND_TLV;   
	hdr->len = 4*id_cnt;  /* multiple 4s if more streams */

	*aLen = 4 + 4*id_cnt;

	return WFA_SUCCESS;
}

/*
* xcCmdProcAgentReset(): Process and send the Control command 
*                       "traffic_agent_reset"
* input - pcmdStr  parameter string pointer
* return - WFA_SUCCESS or WFA_FAILURE;
*/
int xcCmdProcAgentReset(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	wfaTLV *hdr = (wfaTLV *)aBuf;

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcAgentReset ...\n");

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	hdr->tag =  WFA_TRAFFIC_AGENT_RESET_TLV;   
	hdr->len = 0;  /* multiple 4s if more streams */

	*aLen = 4;

	return WFA_SUCCESS;
}

/*
* xcCmdProcAgentRecvStart(): Process and send the Control command 
*                       "traffic_agent_receive_start"
* input - pcmdStr  parameter string pointer
* return - TRUE or FALSE;
*/
int xcCmdProcAgentRecvStart(char *pcmdStr, BYTE *aBuf, int *aLen)
{

	wfaTLV *hdr = (wfaTLV *)aBuf;
	char *str, *sid;
	int strid;
	int id_cnt = 0;

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcAgentRecvStart ...%s\n", pcmdStr);

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	/* there is only one stream for baseline. Will support
	* multiple streams later.
	*/
	str = strtok_r(NULL, ",", &pcmdStr);  

	if(str == NULL || str[0] == '\0')
	{
		DPRINT_ERR(WFA_ERR, "Null string\n");
		return WFA_FAILURE;
	}


	if(strcasecmp(str, "streamid") != 0)
	{
		DPRINT_ERR(WFA_ERR, "invalid type name\n");
		return WFA_FAILURE;
	}

	while(1)
	{
		sid = strtok_r (NULL, " ", &pcmdStr);  
		if(sid == NULL)
			break;

		if(isNumber(sid) == WFA_FAILURE)
			continue;

		strid = atoi(sid);
		id_cnt++; 

		memcpy(aBuf+4*id_cnt, (char *)&strid, 4);
	}

	hdr->tag =  WFA_TRAFFIC_AGENT_RECV_START_TLV;   
	hdr->len = 4*id_cnt;  /* multiple 4s if more streams */

	*aLen = 4 + 4*id_cnt;

	return WFA_SUCCESS;
}

/*
* xcCmdProcAgentRecvStop(): Process and send the Control command 
*                       "traffic_agent_receive_stop"
* input - pcmdStr  parameter string pointer
* return - WFA_SUCCESS or WFA_FAILURE;
*/
int xcCmdProcAgentRecvStop(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	wfaTLV *hdr = (wfaTLV *)aBuf;
	char *str, *sid;
	int strid;
	int id_cnt = 0;

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcAgentRecvStop ...\n"); 

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	/* there is only one stream for baseline. Will support
	* multiple streams later.
	*/
	str = strtok_r(NULL, ",", &pcmdStr);  

	if(str == NULL || str[0] == '\0')
		return WFA_FAILURE;

	if(strcasecmp(str, "streamid") != 0)
	{
		DPRINT_ERR(WFA_ERR, "invalid type name\n");
		return WFA_FAILURE;
	}
	while(1)
	{
		sid = strtok_r (NULL, " ", &pcmdStr);  
		if(sid == NULL)
			break;

		if(isNumber(sid) == WFA_FAILURE)
			continue;

		strid = atoi(sid);
		id_cnt++; 

		memcpy(aBuf+4*id_cnt, (char *)&strid, 4);
	}

	hdr->tag =  WFA_TRAFFIC_AGENT_RECV_STOP_TLV;   
	hdr->len = 4*id_cnt;  /* multiple 4s if more streams */

	*aLen = 4 + 4*id_cnt;

	return WFA_SUCCESS;
}

/*
* xcCmdProcAgentSendPing(): Process and send the Control command 
*                       "traffic_agent_send_ping"
* input - pcmdStr  parameter string pointer
* return - WFA_SUCCESS or WFA_FAILURE;
* 
* Most Wi-Fi Cert Tests always use Ping to check the connectivity
*/
int xcCmdProcAgentSendPing(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	wfaTLV *hdr = (wfaTLV *)aBuf;
	tgPingStart_t *staping = (tgPingStart_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	staping->type = 0;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "destination") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staping->dipaddr, str, 15);
			DPRINT_INFO(WFA_OUT, "destination %s\n", staping->dipaddr);
		}
		if(strcasecmp(str, "frameSize") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			staping->frameSize=atoi(str);
			DPRINT_INFO(WFA_OUT, "framesize %i\n", staping->frameSize);
		}
		if(strcasecmp(str, "frameRate") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			staping->frameRate= (float) atof(str);
			DPRINT_INFO(WFA_OUT, "framerate %f\n", staping->frameRate);
		}
		if(strcasecmp(str, "duration") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			staping->duration=atoi(str);
			DPRINT_INFO(WFA_OUT, "duration %i\n", staping->duration);
		}
		if(strcasecmp(str, "type") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "udp") == 0)
				staping->type = 1; 
			else
				staping->type = 0;
		}
		if(strcasecmp(str, "iptype") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "2") == 0)
				staping->ipType = 2; 
			else
				staping->ipType = 1;
		}
		if(strcasecmp(str, "qos") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "vo") == 0)
			{
				staping->qos = TG_WMM_AC_VO;
			}
			else if(strcasecmp(str, "vi") == 0)
			{
				staping->qos = TG_WMM_AC_VI;
			}
			else if(strcasecmp(str, "be") ==0)
			{
				staping->qos = TG_WMM_AC_BE;
			}
			else if(strcasecmp(str, "bk") == 0)
			{
				staping->qos = TG_WMM_AC_BK;
			}
			else
			{
				// be
				staping->qos = TG_WMM_AC_BE;
			}
		}
	}

	hdr->tag =  WFA_TRAFFIC_SEND_PING_TLV;   
	hdr->len = sizeof(tgPingStart_t);

	*aLen = hdr->len + 4;

	return WFA_SUCCESS;
}

int xcCmdProcAgentStopPing(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	wfaTLV *hdr = (wfaTLV *)aBuf;
	char *str;
	int strid;
	str = strtok_r(NULL, ",", &pcmdStr);  

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	if(str == NULL || str[0] == '\0')
		return WFA_FAILURE;

	if(strcasecmp(str, "streamid") == 0)
		str = strtok_r(NULL, ",", &pcmdStr);  
	else
	{
		DPRINT_ERR(WFA_ERR, "invalid type name\n");
		return WFA_FAILURE;
	}

	if(isNumber(str) == WFA_FAILURE)
		return WFA_FAILURE;

	strid = atoi(str);

	memcpy(aBuf+4, (char *)&strid, 4);

	hdr->tag =  WFA_TRAFFIC_STOP_PING_TLV;   
	hdr->len = 4;  /* multiple 4s if more streams */

	*aLen = 8;

	return WFA_SUCCESS;
}

int xcCmdProcStaGetIpConfig(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	int slen;
	char *str = NULL;
	dutCommand_t getipconf;
	memset(&getipconf, 0, sizeof(dutCommand_t));

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaGetIpConfig ...\n");

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	str = strtok_r(NULL, ",", &pcmdStr);
	str = strtok_r(NULL, ",", &pcmdStr);
	if(str == NULL)
		return WFA_FAILURE;


	slen = strlen(str);
	memcpy(getipconf.intf, str, slen);
	wfaEncodeTLV(WFA_STA_GET_IP_CONFIG_TLV, sizeof(dutCommand_t), (BYTE *)&getipconf, aBuf);

	*aLen = 4+sizeof(getipconf);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetIpConfig(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	dutCommand_t staSetIpConfig;
	caStaSetIpConfig_t *setip = (caStaSetIpConfig_t *)&staSetIpConfig.cmdsu.ipconfig; 
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);
	memset(&staSetIpConfig, 0, sizeof(staSetIpConfig));/*  must init it */
	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setip->intf, str, 15);
			DPRINT_INFO(WFA_OUT, "interface %s\n", setip->intf);
		}
		else if(strcasecmp(str, "dhcp") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			setip->isDhcp = atoi(str);  
			DPRINT_INFO(WFA_OUT, "dhcp %i\n", setip->isDhcp);
		}
		else if(strcasecmp(str, "ip") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setip->ipaddr, str, 15);
			DPRINT_INFO(WFA_OUT, "ip %s\n", setip->ipaddr);
		}
		else if(strcasecmp(str, "mask") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setip->mask, str, 15);
			DPRINT_INFO(WFA_OUT, "mask %s\n", setip->mask);
		}
		else if(strcasecmp(str, "defaultGateway") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setip->defGateway, str, 15);
			DPRINT_INFO(WFA_OUT, "gw %s\n", setip->defGateway);
		}
		else if(strcasecmp(str, "primary-dns") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setip->pri_dns, str, 15);
			DPRINT_INFO(WFA_OUT, "dns p %s\n", setip->pri_dns);
		}
		else if(strcasecmp(str, "secondary-dns") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setip->sec_dns, str, 15);
			DPRINT_INFO(WFA_OUT, "dns s %s\n", setip->sec_dns);
		}
		else if(strcasecmp(str, "type") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "2") == 0)
				setip->ipType = 2; 
			else
				setip->ipType = 1;
		}
		else
		{
			DPRINT_ERR(WFA_ERR, "invalid command\n");
			return WFA_FAILURE;
		}
	}

	wfaEncodeTLV(WFA_STA_SET_IP_CONFIG_TLV, sizeof(staSetIpConfig), (BYTE *)&staSetIpConfig, aBuf);

	*aLen = 4+sizeof(staSetIpConfig);

	return WFA_SUCCESS;
}

int xcCmdProcStaGetMacAddress(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	int slen;
	char *str = NULL;
	dutCommand_t getmac;

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaGetMacAddress ...\n");

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	memset(&getmac, 0, sizeof(getmac));
	str = strtok_r(NULL, ",", &pcmdStr);
	str = strtok_r(NULL, ",", &pcmdStr);
	if(str == NULL)
		return WFA_FAILURE;

	slen = strlen(str);
	memcpy(getmac.intf, str, slen);
	wfaEncodeTLV(WFA_STA_GET_MAC_ADDRESS_TLV, sizeof(getmac), (BYTE *)&getmac, aBuf);

	*aLen = 4+sizeof(getmac);

	return WFA_SUCCESS;
}


int xcCmdProcStaIsConnected(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	int slen;
	char *str = NULL;
	dutCommand_t isconnected;

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaIsConnected\n");

	memset(&isconnected, 0, sizeof(isconnected));

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	str = strtok_r(NULL, ",", &pcmdStr);
	str = strtok_r(NULL, ",", &pcmdStr);
	if(str == NULL)
		return WFA_FAILURE;

	slen = strlen(str);
	memcpy(isconnected.intf, str, slen);
	wfaEncodeTLV(WFA_STA_IS_CONNECTED_TLV, sizeof(isconnected), (BYTE *)&isconnected, aBuf);

	*aLen = 4+sizeof(isconnected);

	return WFA_SUCCESS;
}

int xcCmdProcStaVerifyIpConnection(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	wfaTLV *hdr = (wfaTLV *)aBuf;
	dutCommand_t *verifyip = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaVerifyIpConnection ...\n");

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(verifyip->intf, str);
			verifyip->intf[15]='\0';
			DPRINT_INFO(WFA_OUT, "interface %s %i\n", verifyip->intf, strlen(verifyip->intf));
		}
		else if(strcasecmp(str, "destination") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(verifyip->cmdsu.verifyIp.dipaddr, str, 15);
			DPRINT_INFO(WFA_OUT, "ip %s\n", verifyip->cmdsu.verifyIp.dipaddr);
		}
		else if(strcasecmp(str, "timeout") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			verifyip->cmdsu.verifyIp.timeout = atoi(str);  
			DPRINT_INFO(WFA_OUT, "timeout %i\n", verifyip->cmdsu.verifyIp.timeout);
		}
	}

	wfaEncodeTLV(WFA_STA_VERIFY_IP_CONNECTION_TLV, sizeof(verifyip), (BYTE *)&verifyip, aBuf);

	hdr->tag =  WFA_STA_VERIFY_IP_CONNECTION_TLV;   
	hdr->len = sizeof(dutCommand_t);

	*aLen = 4+sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaGetBSSID(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	char *str = NULL;
	dutCommand_t getbssid;

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaGetBSSID ...\n");

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	memset(&getbssid, 0, sizeof(getbssid));
	str = strtok_r(NULL, ",", &pcmdStr);
	str = strtok_r(NULL, ",", &pcmdStr);
	if(str == NULL)
		return WFA_FAILURE;

	memcpy(getbssid.intf, str, WFA_IF_NAME_LEN-1);
	getbssid.intf[WFA_IF_NAME_LEN-1] = '\0';
	wfaEncodeTLV(WFA_STA_GET_BSSID_TLV, sizeof(getbssid), (BYTE *)&getbssid, aBuf);

	*aLen = 4+sizeof(getbssid);

	return WFA_SUCCESS;
}


int xcCmdProcStaGetStats(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	char *str = NULL;
	dutCommand_t getstats;

	DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaGetStats ...\n");
	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	memset(&getstats, 0, sizeof(getstats));
	str = strtok_r(NULL, ",", &pcmdStr);
	/* need to check if the parameter name is called interface */ 
	str = strtok_r(NULL, ",", &pcmdStr);
	if(str == NULL)
		return WFA_FAILURE;

	memcpy(getstats.intf, str, WFA_IF_NAME_LEN-1);
	getstats.intf[WFA_IF_NAME_LEN-1] = '\0';
	wfaEncodeTLV(WFA_STA_GET_STATS_TLV, sizeof(getstats), (BYTE *)&getstats, aBuf);

	*aLen = 4+sizeof(getstats);

	return WFA_SUCCESS;
}


int  xcCmdProcStaSetEncryption(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetEncryption_t *setencryp = (caStaSetEncryption_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setencryp->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setencryp->ssid, str, 64);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			if(strcasecmp(str, "wep") == 0)
				setencryp->encpType = ENCRYPT_WEP;
			else
				setencryp->encpType = 0;
		}
		else if(strcasecmp(str, "key1") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setencryp->keys[0], str, 26);
			DPRINT_INFO(WFA_OUT, "%s\n", setencryp->keys[0]);
			setencryp->activeKeyIdx = 0;
		}
		else if(strcasecmp(str, "key2") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setencryp->keys[1], str, 26);
			DPRINT_INFO(WFA_OUT, "%s\n", setencryp->keys[1]);
		}
		else if(strcasecmp(str, "key3") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setencryp->keys[2], str, 26);
			DPRINT_INFO(WFA_OUT, "%s\n", setencryp->keys[2]);
		}
		else if(strcasecmp(str, "key4") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setencryp->keys[3], str, 26);
			DPRINT_INFO(WFA_OUT, "%s\n", setencryp->keys[3]);
		}
		else if(strcasecmp(str, "activeKey") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			setencryp->activeKeyIdx =  atoi(str);
		}
		else
		{
			DPRINT_INFO(WFA_WNG, "Incorrect Command, check syntax\n");
		}
	}

	wfaEncodeTLV(WFA_STA_SET_ENCRYPTION_TLV, sizeof(caStaSetEncryption_t), (BYTE *)setencryp, aBuf);

	*aLen = 4+sizeof(caStaSetEncryption_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetPSK(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetPSK_t *setencryp = (caStaSetPSK_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setencryp->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setencryp->ssid, str, 64);
			DPRINT_INFO(WFA_OUT, "ssid %s\n", setencryp->ssid);
		}
		else if(strcasecmp(str, "passPhrase") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setencryp->passphrase, str, 63);
		}
		else if(strcasecmp(str, "keyMgmtType") == 0)
		{
			str=strtok_r(NULL, ",", &pcmdStr);
			strncpy(setencryp->keyMgmtType, str, 7);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);

			if(strcasecmp(str, "tkip") == 0)
				setencryp->encpType = ENCRYPT_TKIP;
			else if(strcasecmp(str, "aes-ccmp") == 0)
				setencryp->encpType = ENCRYPT_AESCCMP;
		}
	}

	wfaEncodeTLV(WFA_STA_SET_PSK_TLV, sizeof(caStaSetPSK_t), (BYTE *)setencryp, aBuf);

	*aLen = 4+sizeof(caStaSetPSK_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetEapTLS(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetEapTLS_t *setsec = (caStaSetEapTLS_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->ssid, str, 64);
		}
		else if(strcasecmp(str, "keyMgmtType") == 0)
		{
			str=strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->keyMgmtType, str, 8);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->encrptype, str, 8);
		}
		else if(strcasecmp(str, "trustedRootCA") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->trustedRootCA, str);
		}
		else if(strcasecmp(str, "clientCertificate") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->clientCertificate, str);
		}
	}

	wfaEncodeTLV(WFA_STA_SET_EAPTLS_TLV, sizeof(caStaSetEapTLS_t), (BYTE *)setsec, aBuf);

	*aLen = 4+sizeof(caStaSetEapTLS_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetEapTTLS(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetEapTTLS_t *setsec = (caStaSetEapTTLS_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->ssid, str, 64);
		}
		else if(strcasecmp(str, "username") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->username, str);
		}
		else if(strcasecmp(str, "password") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->passwd, str);
		}
		else if(strcasecmp(str, "keyMgmtType") == 0)
		{
			str=strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->keyMgmtType, str, 7);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->encrptype, str, 8);
		}
		else if(strcasecmp(str, "trustedRootCA") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->trustedRootCA, str);
		}
		else if(strcasecmp(str, "clientCertificate") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->clientCertificate, str);
		}
	}

	wfaEncodeTLV(WFA_STA_SET_EAPTTLS_TLV, sizeof(caStaSetEapTTLS_t), (BYTE *)setsec, aBuf);

	*aLen = 4+sizeof(caStaSetEapTTLS_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetEapSIM(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetEapSIM_t *setsec = (caStaSetEapSIM_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->ssid, str, 64);
		}
		else if(strcasecmp(str, "username") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->username, str);
		}
		else if(strcasecmp(str, "password") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->passwd, str);
		}
		else if(strcasecmp(str, "keyMgmtType") == 0)
		{
			str=strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->keyMgmtType, str, 7);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->encrptype, str,8);
		}
		else if(strcasecmp(str, "triplet1") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setsec->tripletSet[0], str, 63);
			DPRINT_INFO(WFA_OUT, "Triplet1 : %s\n", setsec->tripletSet[0]);
			setsec->tripletCount = 1;
		}
		else if(strcasecmp(str, "triplet2") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setsec->tripletSet[1], str, 63);
			DPRINT_INFO(WFA_OUT, "Triplet2 : %s\n", setsec->tripletSet[1]);
			setsec->tripletCount=2;
		}
		else if(strcasecmp(str, "triplet3") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setsec->tripletSet[2], str, 63);
			DPRINT_INFO(WFA_OUT, "Triplet1 : %s\n", setsec->tripletSet[2]);
			setsec->tripletCount = 3;
		}
	}

	wfaEncodeTLV(WFA_STA_SET_EAPSIM_TLV, sizeof(caStaSetEapSIM_t), (BYTE *)setsec, aBuf);

	*aLen = 4+sizeof(caStaSetEapSIM_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetPEAP(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetEapPEAP_t *setsec = (caStaSetEapPEAP_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->ssid, str, 64);
		}
		else if(strcasecmp(str, "username") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->username, str);
		}
		else if(strcasecmp(str, "password") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->passwd, str);
		}
		else if(strcasecmp(str, "keyMgmtType") == 0)
		{
			str=strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->keyMgmtType, str, 7);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->encrptype, str, 8);
		}
		else if(strcasecmp(str, "innerEAP") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->innerEAP, str);
		}
		else if(strcasecmp(str, "trustedRootCA") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->trustedRootCA, str,31);
		}
		else if(strcasecmp(str, "peapVersion") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setsec->peapVersion = atoi(str);
		}
	}


	wfaEncodeTLV(WFA_STA_SET_PEAP_TLV, sizeof(caStaSetEapPEAP_t), (BYTE *)setsec, aBuf);

	*aLen = 4+sizeof(caStaSetEapPEAP_t);

	return WFA_SUCCESS;
}


int xcCmdProcStaSetIBSS(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetIBSS_t *setibss = (caStaSetIBSS_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	int i = 0;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setibss->intf, str, 15);
			DPRINT_INFO(WFA_OUT, "interface %s\n", setibss->intf);

		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setibss->ssid, str, 64);
			DPRINT_INFO(WFA_OUT, "ssid %s\n", setibss->ssid);
		}
		else if(strcasecmp(str, "channel") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setibss->channel = atoi(str);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "wep") == 0)
				setibss->encpType = ENCRYPT_WEP;
			else
				setibss->encpType = 0;
		}
		else if(strncasecmp(str, "key1", 4) == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setibss->keys[i++], str, 26);
			setibss->activeKeyIdx = 0;
		}
		else if(strncasecmp(str, "key2", 4) == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setibss->keys[i++], str, 26);
		}
		else if(strncasecmp(str, "key3", 4) == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setibss->keys[i++], str, 26);
		}
		else if(strncasecmp(str, "key4", 4) == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setibss->keys[i++], str, 26);
		}
		else if(strcasecmp(str, "activeKey") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setibss->activeKeyIdx = atoi(str);
		}
	}

	wfaEncodeTLV(WFA_STA_SET_IBSS_TLV, sizeof(caStaSetIBSS_t), (BYTE *)setibss, aBuf);

	*aLen = 4+sizeof(caStaSetIBSS_t);

	return WFA_SUCCESS;
}

int xcCmdProcDeviceGetInfo(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	wfaEncodeTLV(WFA_DEVICE_GET_INFO_TLV, 0, NULL, aBuf);

	*aLen = 4;

	return WFA_SUCCESS;
}

int xcCmdProcStaGetInfo(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	char *str;
	dutCommand_t *getInfo = (dutCommand_t *) (aBuf+sizeof(wfaTLV));

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	str = strtok_r(NULL, ",", &pcmdStr);
	if(str == NULL || str[0] == '\0')
	{
		DPRINT_ERR(WFA_ERR, "Null pt, xcCmdProcStaGetInfo:parameter from UCC is not found!\n");
		return WFA_FAILURE;
	}

	if(strcasecmp(str, "interface") == 0)
	{
		str = strtok_r(NULL, ",", &pcmdStr);  
		strncpy(getInfo->intf, str, 15);
		DPRINT_INFO(WFA_OUT, "interface %s\n", getInfo->intf);

	}

	wfaEncodeTLV(WFA_STA_GET_INFO_TLV, sizeof(dutCommand_t), (BYTE *)getInfo, aBuf);

	*aLen = 4 + sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaUpload(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	char *str;
	dutCommand_t *dutCmd = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	caStaUpload_t *tdp = &dutCmd->cmdsu.upload;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	str = strtok_r(NULL, ",", &pcmdStr);
	if(str == NULL || str[0] == '\0')
	{
		DPRINT_ERR(WFA_ERR, "Null pt, xcCmdProcStaUpload:parameter from UCC is not found!\n");
		return WFA_FAILURE;
	}

	if(strcasecmp(str, "test") == 0)
	{
		str = strtok_r(NULL, ",", &pcmdStr);  
		if(str == NULL || str[0] == '\0' )
		{
			DPRINT_ERR(WFA_ERR, "xcCmdProcStaUpload:Null pt 1, parameter test has no value!\n");
			return WFA_FAILURE; //break here;
		}
		if(strcasecmp(str, "voice") == 0)
		{
			tdp->type = WFA_UPLOAD_VHSO_RPT;
			DPRINT_INFO(WFA_OUT, "testdata voice %i\n", tdp->type);
			str = strtok_r(NULL, ",", &pcmdStr);
			tdp->next = atoi(str);
		}
	}

	wfaEncodeTLV(WFA_STA_UPLOAD_TLV, sizeof(dutCommand_t), (BYTE *)dutCmd, aBuf);

	*aLen = 4 + sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaAssociate(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	dutCommand_t *setassoc = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			if (str == NULL)
			{
				DPRINT_ERR(WFA_ERR, "xcCmdProcStaAssociate:Null pt 1, parameter pass in from UCC is not found!\n");
				return WFA_FAILURE; //break;
			}
			strncpy(setassoc->intf, str, 15);
			DPRINT_INFO(WFA_OUT, "interface %s\n", setassoc->intf);

		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			if(str == NULL || str[0] == '\0' )
			{
				DPRINT_ERR(WFA_ERR, "xcCmdProcStaAssociate:Null pt 2, parameter pass in from UCC is not found!\n");
				return WFA_FAILURE; //break;
			}
			strncpy(setassoc->cmdsu.ssid, str, 64);
			DPRINT_INFO(WFA_OUT, "ssid %s\n", setassoc->cmdsu.ssid);
		}
	}

	wfaEncodeTLV(WFA_STA_ASSOCIATE_TLV, sizeof(dutCommand_t), (BYTE *)setassoc, aBuf);

	*aLen = 4+sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcDeviceListIF(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	dutCommand_t *getdevlist = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	str = strtok_r(NULL, ",", &pcmdStr);
	if(str == NULL || str[0] == '\0')
	{
		DPRINT_ERR(WFA_ERR, "Null pt, parameter interfaceType from UCC do not have value!\n");
		return WFA_FAILURE;
	}

	if(strcasecmp(str, "interfaceType") == 0)
	{
		str = strtok_r(NULL, ",", &pcmdStr);  
		if(strcmp(str, "802.11") == 0)
			getdevlist->cmdsu.iftype = IF_80211;

		DPRINT_INFO(WFA_OUT, "interface type %i\n", getdevlist->cmdsu.iftype);
	}

	wfaEncodeTLV(WFA_DEVICE_LIST_IF_TLV, sizeof(dutCommand_t), (BYTE *)getdevlist, aBuf);

	*aLen = 4 + sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetUAPSD(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetUAPSD_t *setuapsd = (caStaSetUAPSD_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	wfaTLV *hdr = (wfaTLV *)aBuf;

	DPRINT_INFO(WFA_OUT, "start xcCmdProcAgentConfig ...\n");
	DPRINT_INFO(WFA_OUT, "params:  %s\n", pcmdStr);
	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);
	setuapsd->acBE = 0;
	setuapsd->acBK = 0;
	setuapsd->acVI = 0;
	setuapsd->acVO = 0;

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setuapsd->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setuapsd->ssid, str, 64);
		}
		else if(strcasecmp(str, "acBE") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
#ifndef WFA_WMM_AC
			setuapsd->acBE = atoi(str);
#else
			if(strncasecmp(str, "UPLINK", 6) == 0)
				setuapsd->acBE = DIR_UP;
			else if(strncasecmp(str, "DOWNLINK", 8) == 0)
				setuapsd->acBE = DIR_DOWN;
			else if(strncasecmp(str, "BIDIR", 5) == 0)
				setuapsd->acBE = DIR_BIDIR;
			else
				setuapsd->acBE = DIR_NONE;
#endif

		}
		else if(strcasecmp(str, "acBK") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
#ifndef WFA_WMM_AC
			setuapsd->acBK = atoi(str);
#else
			if(strncasecmp(str, "UPLINK", 6) == 0)
				setuapsd->acBK = DIR_UP;
			else if(strncasecmp(str, "DOWNLINK", 8) == 0)
				setuapsd->acBK = DIR_DOWN;
			else if(strncasecmp(str, "BIDIR", 5) == 0)
				setuapsd->acBK = DIR_BIDIR;
			else
				setuapsd->acBK = DIR_NONE;
#endif
		}
		else if(strcasecmp(str, "acVI") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
#ifndef WFA_WMM_AC
			setuapsd->acVI = atoi(str);
#else
			if(strncasecmp(str, "UPLINK", 6) == 0)
				setuapsd->acVI = DIR_UP;
			else if(strncasecmp(str, "DOWNLINK", 8) == 0)
				setuapsd->acVI = DIR_DOWN;
			else if(strncasecmp(str, "BIDIR", 5) == 0)
				setuapsd->acVI = DIR_BIDIR;
			else
				setuapsd->acVI = DIR_NONE;
#endif
		}
		else if(strcasecmp(str, "acVO") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
#ifndef WFA_WMM_AC
			setuapsd->acVO = atoi(str);
#else
			if(strncasecmp(str, "UPLINK", 6) == 0)
				setuapsd->acVO = DIR_UP;
			else if(strncasecmp(str, "DOWNLINK", 8) == 0)
				setuapsd->acVO = DIR_DOWN;
			else if(strncasecmp(str, "BIDIR", 5) == 0)
				setuapsd->acVO = DIR_BIDIR;
			else
				setuapsd->acVO = DIR_NONE;
#endif
		}
	}
	hdr->tag =  WFA_STA_SET_UAPSD_TLV;   
	hdr->len = sizeof(caStaSetUAPSD_t);

	memcpy(aBuf+4, setuapsd, sizeof(caStaSetUAPSD_t));

	*aLen = 4+sizeof(caStaSetUAPSD_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaDebugSet(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	dutCommand_t *debugSet = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "level") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			if(atoi(str) == WFA_DEBUG_INFO || WFA_DEBUG_WARNING)
			{
				debugSet->cmdsu.dbg.level = atoi(str);
				DPRINT_INFO(WFA_OUT, "dbg level %i\n", debugSet->cmdsu.dbg.level);
			}
			else
				return WFA_FAILURE;  /* not support */

		}
		else if(strcasecmp(str, "enable") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(str == NULL || str[0] == '\0' )
			{
				DPRINT_ERR(WFA_ERR, "Null pt, parameter value from UCC is not found!\n");
				return WFA_FAILURE; //break;
			}
			DPRINT_INFO(WFA_OUT, "enable %i\n", atoi(str));
			switch(atoi(str)) /* enable */
			{
			case 1:
				debugSet->cmdsu.dbg.state = 1;
				DPRINT_INFO(WFA_OUT, "enable\n");
				break;
			case 0:
				debugSet->cmdsu.dbg.state = 0;
				DPRINT_INFO(WFA_OUT, "disable\n");
				break;
			default:
				return WFA_FAILURE;  /* command invalid */
			}
		}
	}

	wfaEncodeTLV(WFA_STA_DEBUG_SET_TLV, sizeof(dutCommand_t), (BYTE *)debugSet, aBuf);

	*aLen = 4 + sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetMode(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetMode_t *setmode = (caStaSetMode_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setmode->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setmode->ssid, str, 64);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "wep") == 0)
				setmode->encpType = ENCRYPT_WEP;
			else
				setmode->encpType = 0;
		}
		else if(strcasecmp(str, "key1") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy((char *)setmode->keys[0], str, 26);
			DPRINT_INFO(WFA_OUT, "%s\n", setmode->keys[0]);
			setmode->activeKeyIdx = 0;
		}
		else if(strcasecmp(str, "key2") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy((char *)setmode->keys[1], str, 26);
			DPRINT_INFO(WFA_OUT, "%s\n", setmode->keys[1]);
		}
		else if(strcasecmp(str, "key3") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy((char *)setmode->keys[2], str, 26);
			DPRINT_INFO(WFA_OUT, "%s\n", setmode->keys[2]);
		}
		else if(strcasecmp(str, "key4") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy((char *)setmode->keys[3], str, 26);
			DPRINT_INFO(WFA_OUT, "%s\n", setmode->keys[3]);
		}
		else if(strcasecmp(str, "activeKey") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setmode->activeKeyIdx =  atoi(str);
		}
		else if(strcasecmp(str, "mode") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "adhoc") == 0)
				setmode->mode = 1;
			else
				setmode->mode = 0;
		}
		else if(strcasecmp(str, "channel") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setmode->channel = atoi(str);
		}
		else
		{
			DPRINT_INFO(WFA_WNG, "Incorrect Command, check syntax\n");
		}
	}

	wfaEncodeTLV(WFA_STA_SET_MODE_TLV, sizeof(caStaSetMode_t), (BYTE *)setmode, aBuf);
	*aLen = 4+sizeof(caStaSetMode_t);

	return WFA_SUCCESS;
}


int xcCmdProcStaSetWMM(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetWMM_t *setwmm = (caStaSetWMM_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	wfaTLV *hdr = (wfaTLV *)aBuf;

	DPRINT_INFO(WFA_OUT, "start xcCmdProcStaSetWMM ...\n");
	DPRINT_INFO(WFA_OUT, "params:  %s\n", pcmdStr);
	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);
	/* Some default values, in case they are not specified*/
	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setwmm->intf, str, 15);
		}
		else if(strcasecmp(str, "GROUP") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			if(strcasecmp(str,"WMMAC") == 0)
				setwmm->group = GROUP_WMMAC;
			else if(strcasecmp(str,"WMM-CONFIG") == 0)
			{
				setwmm->group = GROUP_WMMCONF;
				setwmm->actions.config.frag_thr = 2346;
				setwmm->actions.config.rts_thr = 2346;
				setwmm->actions.config.wmm = 1;
			}
		}
		else if(strcasecmp(str, "ACTION") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			if(strcasecmp(str,"addts") == 0)
			{
				//Put default values for the tspec element
				setwmm->action = WMMAC_ADDTS;
				setwmm->actions.addts.accesscat = WMMAC_AC_BE;
				setwmm->actions.addts.tspec.tsinfo.dummy1 = 1;
				setwmm->actions.addts.tspec.tsinfo.dummy2 = 0;
			}
			else if(strcasecmp(str,"delts") == 0)
			{
				setwmm->action = WMMAC_DELTS;
				DPRINT_INFO(WFA_OUT,"action is %d\n",setwmm->action);
			}
		}
		else if(strcasecmp(str, "RTS_thr") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.config.rts_thr = atoi(str);
		}
		else if(strcasecmp(str, "wmm") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(atoi(str) != 0)
				setwmm->actions.config.wmm = 1;
			else
				setwmm->actions.config.wmm = 0;
		}
		else if(strcasecmp(str, "Frag_thr") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.config.frag_thr = atoi(str);
		}
		else if(strcasecmp(str, "DIALOG_TOKEN") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.dialog_token = atoi(str);
		}
		else if(strcasecmp(str, "TID") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(setwmm->action == WMMAC_ADDTS) 
				setwmm->actions.addts.tspec.tsinfo.TID  = atoi(str);
			else
				setwmm->actions.delts = atoi(str);
		}
		else if(strcasecmp(str, "DIRECTION") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str,"UP") == 0)
				setwmm->actions.addts.tspec.tsinfo.direction = WMMAC_UPLINK;
			else if(strcasecmp(str,"DOWN") == 0)
				setwmm->actions.addts.tspec.tsinfo.direction = WMMAC_DOWNLINK;
			else if(strcasecmp(str,"BIDI") == 0)
				setwmm->actions.addts.tspec.tsinfo.direction = WMMAC_BIDIR;
		}
		else if(strcasecmp(str, "PSB") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str,"UAPSD") == 0)
				setwmm->actions.addts.tspec.tsinfo.PSB = 1;
			else
				setwmm->actions.addts.tspec.tsinfo.PSB = 0;
		}
		else if(strcasecmp(str, "UP") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.tsinfo.UP = atoi(str);
		}
		else if(strcasecmp(str, "Fixed") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "true") == 0)
				setwmm->actions.addts.tspec.Fixed = 1;
			else
				setwmm->actions.addts.tspec.Fixed = 0;
		}
		else if(strcasecmp(str, "SIZE") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.size = atoi(str);
		}
		else if(strcasecmp(str, "MAXSIZE") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.maxsize = atoi(str);
		}
		else if(strcasecmp(str, "MIN_SRVC_INTRVL") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.min_srvc = atoi(str);
		}
		else if(strcasecmp(str, "MAX_SRVC_INTRVL") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.max_srvc = atoi(str);
		}
		else if(strcasecmp(str, "INACTIVITY") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.inactivity = atoi(str);
		}
		else if(strcasecmp(str, "SUSPENSION") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.suspension = atoi(str);
		}
		else if(strcasecmp(str, "SRVCSTARTTIME") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.srvc_strt_tim = atoi(str);
		}
		else if(strcasecmp(str, "MINDATARATE") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.mindatarate = atoi(str);
		}
		else if(strcasecmp(str, "MEANDATARATE") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.meandatarate = atoi(str);
		}
		else if(strcasecmp(str, "PEAKDATARATE") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.peakdatarate = atoi(str);
		}
		else if(strcasecmp(str, "BURSTSIZE") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.burstsize = atoi(str);
		}
		else if(strcasecmp(str, "DELAYBOUND") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.delaybound = atoi(str);
		}
		else if(strcasecmp(str, "PHYRATE") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.PHYrate = atoi(str);
		}
		else if(strcasecmp(str, "SBA") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.sba = (float) atof(str);
		}
		else if(strcasecmp(str, "MEDIUM_TIME") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			setwmm->actions.addts.tspec.medium_time = atoi(str);
		}
		else if(strcasecmp(str, "ACCESSCAT") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str,"VO") == 0)
				setwmm->actions.addts.accesscat = WMMAC_AC_VO;
			else if(strcasecmp(str,"VI") == 0)
				setwmm->actions.addts.accesscat = WMMAC_AC_VI;
			else if(strcasecmp(str,"BE") == 0)
				setwmm->actions.addts.accesscat = WMMAC_AC_BE;
			else if(strcasecmp(str,"BK") == 0)
				setwmm->actions.addts.accesscat = WMMAC_AC_BK;
		}
	}
	if(setwmm->action == WMMAC_ADDTS)
	{
#if 0
		DPRINT_INFO(WFA_OUT, "ADDTS AC PARAMS: dialog id: %d, TID: %d, DIRECTION: %d, PSB: %d, UP: %d,\
							 Fixed %d, MSDU Size: %d, Max MSDU Size %d, MIN SERVICE INTERVAL: %d, MAX SERVICE INTERVAL: %d\
							 ,INACTIVITY: %d,SUSPENSION %d,SERVICE START TIME: %d,MIN DATARATE: %d,MEAN DATA RATE: %d\
							 , PEAK DATA RATE: %d,BURSTSIZE: %d,DELAY BOUND: %d,PHYRATE: %d, SPLUSBW: %f,MEDIUM TIME: %d, ACCESSCAT: %d\n"\
							 ,setwmm->actions.addts.dialog_token,setwmm->actions.addts.tspec.tsinfo.TID\
							 ,setwmm->actions.addts.tspec.tsinfo.direction,setwmm->actions.addts.tspec.tsinfo.PSB,setwmm->actions.addts.tspec.tsinfo.UP\
							 ,setwmm->actions.addts.tspec.Fixed,setwmm->actions.addts.tspec.size, setwmm->actions.addts.tspec.maxsize,\
							 setwmm->actions.addts.tspec.min_srvc,\
							 setwmm->actions.addts.tspec.max_srvc,setwmm->actions.addts.tspec.inactivity,setwmm->actions.addts.tspec.suspension,\
							 setwmm->actions.addts.tspec.srvc_strt_tim,setwmm->actions.addts.tspec.mindatarate,setwmm->actions.addts.tspec.meandatarate\
							 ,setwmm->actions.addts.tspec.peakdatarate,setwmm->actions.addts.tspec.burstsize,\
							 setwmm->actions.addts.tspec.delaybound,setwmm->actions.addts.tspec.PHYrate,setwmm->actions.addts.tspec.sba,\
							 setwmm->actions.addts.tspec.medium_time,setwmm->actions.addts.accesscat);
#endif
	}
	else
		DPRINT_INFO(WFA_OUT, "DELTS AC PARAMS: TID:  %d\n", setwmm->actions.delts);

	hdr->tag =  WFA_STA_SET_WMM_TLV;   
	hdr->len = sizeof(caStaSetWMM_t);
	memcpy(aBuf+4, setwmm, sizeof(caStaSetWMM_t));

	*aLen = 4+sizeof(caStaSetWMM_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaPresetTestParameters(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaPresetParameters_t *presetTestParams = (caStaPresetParameters_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	caStaPresetParameters_t initParams = { "0", (wfaSupplicantName)(0), 0, 0x00, 0x0000, 0x00, 0x0000, (wfaPreambleType)0x00, 0,(wfaWirelessMode) 0x00, 0, 0xFF};


	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	memcpy(presetTestParams, &initParams, sizeof(caStaPresetParameters_t));

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(presetTestParams->intf, str, 15);
		}
		else if(strcasecmp(str, "supplicant") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			DPRINT_INFO(WFA_OUT, "Supplicant Name is %s\n", str);

			if(strcasecmp(str, "ZeroConfig") == 0)
			{
				presetTestParams->supplicant = eWindowsZeroConfig;
			}
			else if(strcasecmp(str, "Default") == 0)
			{			   
				presetTestParams->supplicant = eWpaSupplicant;
			}
		}
		else if(strcasecmp(str, "RTS") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			DPRINT_INFO(WFA_OUT, "RTS Threshold is %s\n", str);

			presetTestParams->rtsThreshold = atoi(str);
			presetTestParams->rtsFlag = 1;
			DPRINT_INFO(WFA_OUT, "\nSetting RTS Threshold as %d\n", presetTestParams->rtsThreshold);
		}
		else if(strcasecmp(str, "FRGMNT") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			DPRINT_INFO(WFA_OUT, "FRGMNT Threshold is %s\n", str);

			presetTestParams->fragThreshold = atoi(str);
			presetTestParams->fragFlag = 1;
			DPRINT_INFO(WFA_OUT, "\nSetting FRGMNT Threshold as %d\n", presetTestParams->fragThreshold);
		}
		else if(strcasecmp(str, "preamble") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			DPRINT_INFO(WFA_OUT, "preamble is %s\n", str);

			if(strcasecmp(str, "Long") == 0)
				presetTestParams->preamble = eLong;
			else
				presetTestParams->preamble = eShort;

			presetTestParams->preambleFlag = 1;
			DPRINT_INFO(WFA_OUT, "\nSetting preamble as %d\n", presetTestParams->preamble);
		}

		else if(strcasecmp(str, "mode") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			DPRINT_INFO(WFA_OUT, "modeis %s\n", str);

			if(strcasecmp(str, "11b") == 0 || strcasecmp(str, "b") == 0)
				presetTestParams->wirelessMode = eModeB;
			else if(strcasecmp(str, "11g") == 0 || strcasecmp(str, "g") == 0 || strcasecmp(str, "bg") ==0 )
				presetTestParams->wirelessMode = eModeBG;
			else if(strcasecmp(str, "11a") == 0 || strcasecmp(str, "a") == 0)
				presetTestParams->wirelessMode = eModeA;
			else if(strcasecmp(str, "11abg") == 0 || strcasecmp(str, "abg") == 0)
				presetTestParams->wirelessMode = eModeABG;
			else if(strcasecmp(str, "11na") == 0)
				presetTestParams->wirelessMode = eModeAN;
			else if(strcasecmp(str, "11ng") == 0)
				presetTestParams->wirelessMode = eModeGN;
			else if(strcasecmp(str, "11nl") == 0)
				presetTestParams->wirelessMode = eModeNL;   // n+abg

			presetTestParams->modeFlag = 1;
			DPRINT_INFO(WFA_OUT, "\nSetting Mode as %d\n", presetTestParams->wirelessMode);
		}
		else if(strcasecmp(str, "powersave") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			DPRINT_INFO(WFA_OUT, "powersave is %s\n", str);

			if(strcasecmp(str, "on") == 0 || strcasecmp(str, "pspoll")==0)
				presetTestParams->legacyPowerSave = 1;
			else if (strcasecmp(str, "fast") == 0)
				presetTestParams->legacyPowerSave = 2;
			else if (strcasecmp(str, "psnonpoll") == 0)
				presetTestParams->legacyPowerSave = 3;
			else
				presetTestParams->legacyPowerSave = 0;

			presetTestParams->psFlag = 1;
			DPRINT_INFO(WFA_OUT, "\nSetting legacyPowerSave as %d\n", presetTestParams->legacyPowerSave);
		}

		else if(strcasecmp(str, "wmm") == 0)
		{
			presetTestParams->wmmFlag = 1;
			str = strtok_r(NULL, ",", &pcmdStr);
			DPRINT_INFO(WFA_OUT, "wmm is %s\n", str);

			if(strcasecmp(str, "on") == 0)
				presetTestParams->wmmState = 1;
			else if(strcasecmp(str, "off") == 0)
				presetTestParams->wmmState = 0;
		}

		else if(strcasecmp(str, "ht") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "on") == 0)
			{
				presetTestParams->ht = 1;
			}
			else
			{
				presetTestParams->ht = 0;
			}
		}

		else if(strcasecmp(str, "reset") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "11n") == 0)
			{
				presetTestParams->reset = eResetProg11n;
				DPRINT_INFO(WFA_OUT, "reset to %s\n", str);
			}
		}
#if 1
		else if(strcasecmp(str, "noack") == 0)
		{
			char *setvalues =strtok_r(NULL, ",", &pcmdStr);
			if(setvalues != NULL)
			{
				str=strtok_r(NULL, ":", &setvalues);
				if(str != NULL)
				{
					if(strcasecmp(str, "enable") == 0)
						presetTestParams->noack_be = 2;
					else if(strcasecmp(str, "disable") == 0)
						presetTestParams->noack_be = 1;
				}
				/* BK */
				str=strtok_r(NULL, ":", &setvalues);
				if(str != NULL)
				{
					if(strcasecmp(str, "enable") == 0)
						presetTestParams->noack_bk = 2;
					else if(strcasecmp(str, "disable") == 0)
						presetTestParams->noack_bk = 1;
				}
				/* VI */ 
				str=strtok_r(NULL, ":", &setvalues);
				if(str != NULL)
				{
					if(strcasecmp(str, "enable") == 0)
						presetTestParams->noack_vi = 2;
					else if(strcasecmp(str, "disable") == 0)
						presetTestParams->noack_vi = 1;
				}
				/* VO */
				str=strtok_r(NULL, ":", &setvalues);
				if(str != NULL)
				{
					if(strcasecmp(str, "enable") == 0)
						presetTestParams->noack_vo = 2;
					else if(strcasecmp(str, "disable") == 0)
						presetTestParams->noack_vo = 1;
				}
			}
		}
#endif
	}
	DPRINT_INFO(WFA_OUT, "\nThe Tag value for preset is : %d",WFA_STA_PRESET_PARAMETERS_TLV);
	wfaEncodeTLV(WFA_STA_PRESET_PARAMETERS_TLV, sizeof(caStaPresetParameters_t), (BYTE *)presetTestParams, aBuf);

	*aLen = 4 + sizeof(caStaPresetParameters_t);

	return WFA_SUCCESS;
}


int xcCmdProcStaSetEapFAST(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetEapFAST_t *setsec = (caStaSetEapFAST_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->ssid, str, 64);
		}
		else if(strcasecmp(str, "username") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->username, str);
		}
		else if(strcasecmp(str, "password") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->passwd, str);
		}
		else if(strcasecmp(str, "keyMgmtType") == 0)
		{
			str=strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->keyMgmtType, str, 7);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->encrptype, str, 8);
		}
		else if(strcasecmp(str, "trustedRootCA") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->trustedRootCA, str,31);
		}
		else if(strcasecmp(str, "innerEAP") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->innerEAP, str);
		}
		else if(strcasecmp(str, "validateServer") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "yes") == 0)
			{
				setsec->validateServer=1;
			}
			else if(strcasecmp(str, "no") == 0)
			{
				setsec->validateServer=0;
			}

		}
		else if(strcasecmp(str, "pacFile") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->pacFileName, str);
		}
	}

	wfaEncodeTLV(WFA_STA_SET_EAPFAST_TLV, sizeof(caStaSetEapFAST_t), (BYTE *)setsec, aBuf);

	*aLen = 4+sizeof(caStaSetEapFAST_t);

	return WFA_SUCCESS;
}


int xcCmdProcStaSetEapAKA(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetEapAKA_t *setsec = (caStaSetEapAKA_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->intf, str, 15);
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setsec->ssid, str, 64);
		}
		else if(strcasecmp(str, "username") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->username, str);
		}
		else if(strcasecmp(str, "password") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strcpy(setsec->passwd, str);
		}
		else if(strcasecmp(str, "keyMgmtType") == 0)
		{
			str=strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->keyMgmtType, str, 7);
		}
		else if(strcasecmp(str, "encpType") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(setsec->encrptype, str, 8);
		}

		else if(strcasecmp(str, "triplet1") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setsec->tripletSet[0], str, 63);
			DPRINT_INFO(WFA_OUT, "Triplet1 : %s\n", setsec->tripletSet[0]);
			setsec->tripletCount = 1;
		}
		else if(strcasecmp(str, "triplet2") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setsec->tripletSet[1], str, 63);
			DPRINT_INFO(WFA_OUT, "Triplet2 : %s\n", setsec->tripletSet[1]);
			setsec->tripletCount=2;
		}
		else if(strcasecmp(str, "triplet3") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy((char *)setsec->tripletSet[2], str, 63);
			DPRINT_INFO(WFA_OUT, "Triplet1 : %s\n", setsec->tripletSet[2]);
			setsec->tripletCount = 3;
		}
	}

	wfaEncodeTLV(WFA_STA_SET_EAPAKA_TLV, sizeof(caStaSetEapAKA_t), (BYTE *)setsec, aBuf);

	*aLen = 4+sizeof(caStaSetEapAKA_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetSystime(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetSystime_t *systime = (caStaSetSystime_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "month") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			systime->month=atoi(str);
			DPRINT_INFO(WFA_OUT, "\n month %i \n", systime->month);
		}
		else if(strcasecmp(str, "date") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			systime->date=atoi(str);
			DPRINT_INFO(WFA_OUT, "\n date %i \n", systime->date);
		}
		else if(strcasecmp(str, "year") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			systime->year=atoi(str);
			DPRINT_INFO(WFA_OUT, "\n year %i \n", systime->year);
		}
		else if(strcasecmp(str, "hours") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			systime->hours=atoi(str);
			DPRINT_INFO(WFA_OUT, "\n hours %i \n", systime->hours);
		}
		else if(strcasecmp(str, "minutes") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			systime->minutes=atoi(str);
			DPRINT_INFO(WFA_OUT, "\n minutes %i \n", systime->minutes);
		}
		else if(strcasecmp(str, "seconds") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			systime->seconds=atoi(str);
			DPRINT_INFO(WFA_OUT, "\n seconds %i \n", systime->seconds);
		}
	}

	wfaEncodeTLV(WFA_STA_SET_SYSTIME_TLV, sizeof(caStaSetSystime_t), (BYTE *)systime, aBuf);

	*aLen = 4+sizeof(caStaSetSystime_t);

	return WFA_SUCCESS;
}

caSta11n_t init11nParams = {"wifi0", 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,0xFFFF, 0xFFFF, "", "", 0xFF, 0xFF, 0xFF, 0xFF};

int xcCmdProcStaSet11n(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caSta11n_t *v11nParams = (caSta11n_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	DPRINT_INFO(WFA_OUT,"xcCmdProcStaSet11n Starts...");

	if(aBuf == NULL)
		return WFA_FAILURE;   

	memset(aBuf, 0, *aLen);

	memcpy(v11nParams, &init11nParams, sizeof(caSta11n_t));

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "ampdu") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->ampdu=wfaStandardBoolParsing(str);
			if (v11nParams->ampdu<0) 
			{
				DPRINT_INFO(WFA_OUT, "Invalid AMPDU Value %s\n",str); 
				return WFA_FAILURE;
			}   
			DPRINT_INFO(WFA_OUT, "\n AMPDU -%i- \n", v11nParams->ampdu);
		}  
		else if(strcasecmp(str, "40_intolerant") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->_40_intolerant=wfaStandardBoolParsing(str);
			if (v11nParams->_40_intolerant<0) 
			{
				DPRINT_INFO(WFA_OUT, "Invalid _40_intolerant Value %s\n",str); 
				return WFA_FAILURE;
			}   
			DPRINT_INFO(WFA_OUT, "\n _40_intolerant -%i- \n", v11nParams->_40_intolerant);
		}
		else if(strcasecmp(str, "sgi20") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->sgi20=wfaStandardBoolParsing(str);
			if (v11nParams->sgi20<0) 
			{
				DPRINT_INFO(WFA_OUT, "Invalid sgi20 Value %s\n",str); 
				return WFA_FAILURE;
			}   
			DPRINT_INFO(WFA_OUT, "\n sgi20 -%i- \n", v11nParams->sgi20);
		}
		else if(strcasecmp(str, "amsdu") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->amsdu=wfaStandardBoolParsing(str);
			if (v11nParams->amsdu<0) 
			{
				DPRINT_INFO(WFA_OUT, "Invalid amsdu Value %s\n",str); 
				return WFA_FAILURE;
			}   
			DPRINT_INFO(WFA_OUT, "\n amsdu -%i- \n", v11nParams->amsdu);
		}
		else if(strcasecmp(str, "addba_reject") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->addba_reject=wfaStandardBoolParsing(str);
			if (v11nParams->addba_reject<0) 
			{
				DPRINT_INFO(WFA_OUT, "Invalid addba_reject Value %s\n",str); 
				return WFA_FAILURE;
			}   
			DPRINT_INFO(WFA_OUT, "\n addba_reject -%i- \n", v11nParams->addba_reject);
		}
		else if(strcasecmp(str, "greenfield") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->greenfield=wfaStandardBoolParsing(str);
			if (v11nParams->greenfield<0)
			{
				DPRINT_INFO(WFA_OUT, "Invalid greenfield Value %s\n",str); 
				return WFA_FAILURE;
			}   
			DPRINT_INFO(WFA_OUT, "\n greenfield -%i- \n", v11nParams->greenfield);
		}
		else if(strcasecmp(str, "mcs32") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->mcs32=wfaStandardBoolParsing(str);
			if (v11nParams->mcs32<0) 
			{
				DPRINT_INFO(WFA_OUT, "Invalid mcs32 Value %s\n",str); 
				return WFA_FAILURE;
			}   
			DPRINT_INFO(WFA_OUT, "\n mcs32 -%i- \n", v11nParams->mcs32);
		}
		else if(strcasecmp(str, "rifs_test") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->rifs_test=wfaStandardBoolParsing(str);
			if (v11nParams->rifs_test<0) 
			{
				DPRINT_INFO(WFA_OUT, "Invalid rifs_test Value %s\n",str); 
				return WFA_FAILURE;
			}   
			DPRINT_INFO(WFA_OUT, "\n rifs_test -%i- \n", v11nParams->rifs_test);
		}
		else if(strcasecmp(str, "width") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(v11nParams->width,str,7);
			DPRINT_INFO(WFA_OUT, "\n width -%s- \n", v11nParams->width);
		}
		else if(strcasecmp(str, "mcs_fixedrate") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(v11nParams->mcs_fixedrate,str,63);
			DPRINT_INFO(WFA_OUT, "\n mcs fixedrate -%s- \n", v11nParams->mcs_fixedrate);
		}
		else if(strcasecmp(str, "stbc_rx") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->stbc_rx = atoi(str);
			DPRINT_INFO(WFA_OUT, "\n stbc rx -%d- \n", v11nParams->stbc_rx);
		}
		else if(strcasecmp(str, "smps") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if(strcasecmp(str, "dynamic") == 0)
			{
				v11nParams->smps = 0;
			} 
			else if(strcasecmp(str, "static")==0)
			{
				v11nParams->smps = 1;
			}
			else if(strcasecmp(str, "nolimit") == 0)
			{
				v11nParams->smps = 2;
			}
			//v11nParams->smps = atoi(str);
			DPRINT_INFO(WFA_OUT, "\n smps  -%d- \n", v11nParams->smps);
		}
		else if(strcasecmp(str, "txsp_stream") == 0 )
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->txsp_stream = atoi(str);
			DPRINT_INFO(WFA_OUT, "\n txsp_stream -%d- \n", v11nParams->txsp_stream);
		}
		else if(strcasecmp(str, "rxsp_stream") == 0) 
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			v11nParams->rxsp_stream = atoi(str);
			DPRINT_INFO(WFA_OUT, "\n rxsp_stream -%d- \n", v11nParams->rxsp_stream);
		}
	}
	wfaEncodeTLV(WFA_STA_SET_11N_TLV, sizeof(caSta11n_t), (BYTE *)v11nParams, aBuf);
	*aLen = 4+sizeof(caSta11n_t);

	return WFA_SUCCESS;

}
int xcCmdProcStaSetWireless(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetWireless_t *staWirelessParams = (caStaSetWireless_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	DPRINT_INFO(WFA_OUT,"xcCmdProcStaSetWireless Starts...");

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staWirelessParams->intf, str, 15);
		}
		else if(strcasecmp(str, "band") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(staWirelessParams->band,str,7);      
			DPRINT_INFO(WFA_OUT, "\n Band -%s- \n", staWirelessParams->band);
		}
		else if(strcasecmp(str, "noack") == 0)
		{
			char *ackpol;
			int ackpolcnt = 0;
			char *setvalues =strtok_r(NULL, ",", &pcmdStr);

			if(setvalues != NULL)
			{
				while((ackpol = strtok_r(NULL, ":", &setvalues)) != NULL && ackpolcnt < 4)
				{
					if(strcasecmp(str, "enable") == 0)
						staWirelessParams->noAck[ackpolcnt] = 1;
					else if(strcasecmp(str, "disable") == 0)
						staWirelessParams->noAck[ackpolcnt] = 0;

					ackpolcnt++;
				}
			}
		}
	}

	wfaEncodeTLV(WFA_STA_SET_WIRELESS_TLV, sizeof(caStaSetWireless_t), (BYTE *)staWirelessParams, aBuf);
	*aLen = 4+sizeof(caStaSetWireless_t);

	return WFA_SUCCESS;

}
int xcCmdProcStaSendADDBA(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetSendADDBA_t *staSendADDBA = (caStaSetSendADDBA_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	DPRINT_INFO(WFA_OUT,"xcCmdProcStaSendADDBA Starts...");

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSendADDBA->intf, str, 15);
		}
		else if(strcasecmp(str, "tid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSendADDBA->tid = atoi(str);
			DPRINT_INFO(WFA_OUT, "\n TID -%i- \n", staSendADDBA->tid);
		}  
	}

	wfaEncodeTLV(WFA_STA_SEND_ADDBA_TLV, sizeof(caStaSetSendADDBA_t), (BYTE *)staSendADDBA, aBuf);
	*aLen = 4+sizeof(caStaSetSendADDBA_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetRIFS(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetRIFS_t *staSetRIFS = (caStaSetRIFS_t *)(aBuf+sizeof(wfaTLV));
	char *str;

	DPRINT_INFO(WFA_OUT, "xcCmdProcSetRIFS starts ...\n");

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSetRIFS->intf, str, 15);
		}
		else if(strcasecmp(str, "action") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSetRIFS->action = wfaStandardBoolParsing(str);
			DPRINT_INFO(WFA_OUT, "\n TID -%i- \n", staSetRIFS->action);
		}  
	}

	wfaEncodeTLV(WFA_STA_SET_RIFS_TEST_TLV, sizeof(caStaSetRIFS_t), (BYTE *)staSetRIFS, aBuf);
	*aLen = 4+sizeof(caStaSetRIFS_t);

	return WFA_SUCCESS;
}


int xcCmdProcStaSendCoExistMGMT(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSendCoExistMGMT_t *staSendMGMT = (caStaSendCoExistMGMT_t *)(aBuf+sizeof(wfaTLV));
	char *str;

	DPRINT_INFO(WFA_OUT, "xcCmdProcSendCoExistMGMT starts ...\n");

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSendMGMT->intf, str, 15);
		} 
		else if(strcasecmp(str, "type") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(staSendMGMT->type, str, 15);
		}
		else if(strcasecmp(str, "value") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			strncpy(staSendMGMT->value, str, 15);
		}
	}

	wfaEncodeTLV(WFA_STA_SEND_COEXIST_MGMT_TLV, sizeof(caStaSendCoExistMGMT_t), (BYTE *)staSendMGMT, aBuf);
	*aLen = 4+sizeof(caStaSendCoExistMGMT_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaResetDefault(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaResetDefault_t *reset = (caStaResetDefault_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	memset(reset, 0, sizeof(caStaResetDefault_t));
	DPRINT_INFO(WFA_OUT, "xcCmdProcStaResetDefault starts ...\n");
	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			if ((strlen(str) < 15) && (strlen(str) > 0)  )
				strncpy(reset->intf, str, 15);
			else
				DPRINT_ERR(WFA_OUT, "xcCmdProcStaResetDefault, interface valStr too long or NULL\n");
		}
		//commented out by ray because there is no parameter called "set", but "prog"
		/*if(strcasecmp(str, "set") == 0)
		{
		str = strtok_r(NULL, ",", &pcmdStr);
		strncpy(reset->set, str, 8);
		}*/
		if(strcasecmp(str, "prog") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			if( (strlen(str) < 8) && (strlen(str) > 0))
			{
				strncpy(reset->set, str, strlen(str)+1);
				DPRINT_INFO(WFA_OUT, "xcCmdProcStaResetDefault prog=%s \n", str);
			}
			else
				DPRINT_ERR(WFA_OUT, "xcCmdProcStaResetDefault, prog valStr too long or NULL\n");
		}
	}

	wfaEncodeTLV(WFA_STA_RESET_DEFAULT_TLV, sizeof(caStaResetDefault_t), (BYTE *)reset, aBuf);
	*aLen = 4+sizeof(caStaResetDefault_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaDisconnect(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	dutCommand_t *disc = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(disc->intf, str, 15);
		}
	}


	wfaEncodeTLV(WFA_STA_DISCONNECT_TLV, sizeof(dutCommand_t), (BYTE *)disc, aBuf);
	*aLen = 4+sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

/* Check for enable/disable and return WFA_ENABLE/WFA_DISABLE. WFA_INVALID_BOOL if invalid */
int wfaStandardBoolParsing (char *str)
{
	int rc;

	if(strcasecmp(str, "enable") == 0)
		rc=WFA_ENABLE;    
	else if(strcasecmp(str, "disable") == 0)
		rc=WFA_DISABLE; 
	else 
		rc=WFA_INVALID_BOOL;

	return rc;
}

int xcCmdProcStaReAssociate(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaReAssoc_t *reassoc = (caStaReAssoc_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(reassoc->intf, str, 15);
		}
		if(strcasecmp(str, "bssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(reassoc->bssid, str, 20);
		}
	}

	wfaEncodeTLV(WFA_STA_REASSOCIATE_TLV, sizeof(caStaReAssoc_t), (BYTE *)reassoc, aBuf);
	*aLen = 4+sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaCliCommand(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	DPRINT_INFO(WFA_OUT, "\n The CA CLI command to DUT is : %s",pcmdStr);
	DPRINT_INFO(WFA_OUT, "\n The CA CLI command to DUT Length : %d",strlen(pcmdStr));
	wfaEncodeTLV(WFA_STA_CLI_CMD_TLV, strlen(pcmdStr), (BYTE *)pcmdStr, aBuf);

	*aLen = 4+strlen(pcmdStr);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetPwrSave(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetPwrSave_t *setPwrSave = (caStaSetPwrSave_t *) (aBuf+sizeof(wfaTLV));
	char *str=NULL;

	if((aBuf == NULL) )
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setPwrSave->intf, str, WFA_IF_NAME_LEN - 1);
		}
		if(strcasecmp(str, "mode") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setPwrSave->mode, str, 20);
		}
	}


	wfaEncodeTLV(WFA_STA_SET_PWRSAVE_TLV, sizeof(caStaSetPwrSave_t), (BYTE *)setPwrSave, aBuf);

	*aLen = 4+sizeof(caStaSetPwrSave_t);
	return WFA_SUCCESS;
}

int xcCmdProcStaSetPowerSave(char *pcmdStr, BYTE *aBuf, int *aLen) /* legacy  */
{
	caStaSetPowerSave_t *setPowerSave = (caStaSetPowerSave_t *) (aBuf+sizeof(wfaTLV));
	char *str=NULL;

	if((aBuf == NULL) )
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setPowerSave->intf, str, WFA_IF_NAME_LEN - 1);
		}
		if(strcasecmp(str, "powersave") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(setPowerSave->powersave, str, 20);
		}
	}


	wfaEncodeTLV(WFA_STA_SET_POWER_SAVE_TLV, sizeof(caStaSetPowerSave_t), (BYTE *)setPowerSave, aBuf);

	*aLen = 4+sizeof(caStaSetPowerSave_t);
	return WFA_SUCCESS;
}



#ifdef WFA_P2P
int xcCmdProcStaGetP2pDevAddress(char * pcmdStr,BYTE * aBuf,int * aLen)
{
	dutCommand_t *getP2pDevAdd= (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(getP2pDevAdd->intf, str, 15);
		}
	}

	wfaEncodeTLV(WFA_STA_GET_P2P_DEV_ADDRESS_TLV, sizeof(dutCommand_t), (BYTE *)getP2pDevAdd, aBuf);
	*aLen = 4+sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetP2p(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSetP2p_t *staSetP2p= (caStaSetP2p_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSetP2p->intf, str, WFA_IF_NAME_LEN-1);
		}
		else if(strcasecmp(str, "oper_chn") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSetP2p->oper_chn= atoi(str);
			staSetP2p->oper_chn_flag =1;
		}
		else if(strcasecmp(str, "listen_chn") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSetP2p->listen_chn= atoi(str);
			staSetP2p->listen_chn_flag =1;
		}
		else if(strcasecmp(str, "intent_val") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSetP2p->intent_val= atoi(str);
			staSetP2p->intent_val_flag = 1;
		}
		else if(strcasecmp(str, "p2p_mode") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSetP2p->p2p_mode, str, 15);
			staSetP2p->p2p_mode_flag = 1;

		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSetP2p->ssid, str, WFA_SSID_NAME_LEN-1);
			staSetP2p->ssid_flag = 1;
		}
		else if(strcasecmp(str, "persistent") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSetP2p->presistent= atoi(str);
			staSetP2p->presistent_flag = 1;
		}
		else if(strcasecmp(str, "intra_bss") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSetP2p->intra_bss= atoi(str);
			staSetP2p->intra_bss_flag = 1;
		}
		else if(strcasecmp(str, "noa_duration") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSetP2p->noa_duration= atoi(str);
			staSetP2p->noa_duration_flag = 1;
		}
		else if(strcasecmp(str, "noa_interval") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSetP2p->noa_interval= atoi(str);
			staSetP2p->noa_interval_flag = 1;
		}
		else if(strcasecmp(str, "noa_count") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staSetP2p->noa_count= atoi(str);
			staSetP2p->noa_count_flag = 1;
		}   
	}

	wfaEncodeTLV(WFA_STA_SET_P2P_TLV, sizeof(caStaSetP2p_t), (BYTE *)staSetP2p, aBuf);
	*aLen = 4+sizeof(caStaSetP2p_t);

	return WFA_SUCCESS;
}


int xcCmdProcStaP2pConnect(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaP2pConnect_t *staP2pConnect= (caStaP2pConnect_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staP2pConnect->intf, str, (strlen(str) > WFA_IF_NAME_LEN) ? WFA_IF_NAME_LEN : strlen(str));
		}
		else if(strcasecmp(str, "p2pdevid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staP2pConnect->devId, str, (strlen(str) > WFA_P2P_DEVID_LEN) ? WFA_P2P_DEVID_LEN : strlen(str));
		}
	}

	wfaEncodeTLV(WFA_STA_P2P_CONNECT_TLV, sizeof(caStaP2pConnect_t), (BYTE *)staP2pConnect, aBuf);
	*aLen = 4+sizeof(caStaP2pConnect_t);

	return WFA_SUCCESS;
}



int xcCmdProcStaP2pStartGroupFormation(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaP2pStartGrpForm_t *staP2pStartGrpForm= (caStaP2pStartGrpForm_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staP2pStartGrpForm->intf, str, (strlen(str) > 15) ? 15 : strlen(str));
		}
		else if(strcasecmp(str, "p2pdevid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staP2pStartGrpForm->devId, str, (strlen(str) > 15) ? 15 : strlen(str));
		}
		else if(strcasecmp(str, "intent_val") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);
			staP2pStartGrpForm->intent_val= atoi(str);
		}
	}

	wfaEncodeTLV(WFA_STA_P2P_START_GRP_FORMATION_TLV, sizeof(caStaP2pStartGrpForm_t), (BYTE *)staP2pStartGrpForm, aBuf);
	*aLen = 4+sizeof(caStaP2pStartGrpForm_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaP2pDissolve(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaP2pDissolve_t *staP2pDissolve= (caStaP2pDissolve_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staP2pDissolve->intf, str, (strlen(str) > 15) ? 15 : strlen(str));
		}
		else if(strcasecmp(str, "groupid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staP2pDissolve->grpId, str, (strlen(str) > WFA_P2P_GRP_ID_LEN) ? WFA_P2P_GRP_ID_LEN : strlen(str));
		}
	}

	wfaEncodeTLV(WFA_STA_P2P_DISSOLVE_TLV, sizeof(dutCommand_t), (BYTE *)staP2pDissolve, aBuf);
	*aLen = 4+sizeof(caStaP2pDissolve_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSendP2pInvReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSendP2pInvReq_t *staSendP2pInvReq= (caStaSendP2pInvReq_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSendP2pInvReq->intf, str, 15);
		}
		else if(strcasecmp(str, "groupid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSendP2pInvReq->grpId, str, (strlen(str) > WFA_P2P_GRP_ID_LEN) ? WFA_P2P_GRP_ID_LEN : strlen(str));
		}
		else if(strcasecmp(str, "p2pdevid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSendP2pInvReq->devId, str, 15);
		}
	}

	wfaEncodeTLV(WFA_STA_SEND_P2P_INV_REQ_TLV, sizeof(caStaSendP2pInvReq_t), (BYTE *)staSendP2pInvReq, aBuf);
	*aLen = 4+sizeof(caStaSendP2pInvReq_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaAcceptP2pInvReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSendP2pInvReq_t *staAccceptP2pInvReq= (caStaSendP2pInvReq_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_SUCCESS;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staAccceptP2pInvReq->intf, str, 15);
		}
		else if(strcasecmp(str, "groupid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staAccceptP2pInvReq->grpId, str, (strlen(str) > WFA_P2P_GRP_ID_LEN) ? WFA_P2P_GRP_ID_LEN : strlen(str));
		}
		else if(strcasecmp(str, "p2pdevid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staAccceptP2pInvReq->devId, str, 15);
		}
	}

	wfaEncodeTLV(WFA_STA_ACCEPT_P2P_INV_REQ_TLV, sizeof(caStaSendP2pInvReq_t), (BYTE *)staAccceptP2pInvReq, aBuf);
	*aLen = 4+sizeof(caStaSendP2pInvReq_t);

	return WFA_SUCCESS;


}

int xcCmdProcStaSendP2pProvDisReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaSendP2pProvDisReq_t *staSendP2pProvDisReq= (caStaSendP2pProvDisReq_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSendP2pProvDisReq->intf, str, 15);
		}
		else if(strcasecmp(str, "configmethod") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSendP2pProvDisReq->confMethod, str, 15);
		}
		else if(strcasecmp(str, "p2pdevid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSendP2pProvDisReq->devId, str, 15);
		}
	}

	wfaEncodeTLV(WFA_STA_SEND_P2P_PROV_DIS_REQ_TLV, sizeof(caStaSendP2pProvDisReq_t), (BYTE *)staSendP2pProvDisReq, aBuf);
	*aLen = 4+sizeof(caStaSendP2pProvDisReq_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaSetWpsPbc(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	dutCommand_t *staSetWpsPbc= (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staSetWpsPbc->intf, str, 15);
		}
	}

	wfaEncodeTLV(WFA_STA_SET_WPS_PBC_TLV, sizeof(dutCommand_t), (BYTE *)staSetWpsPbc, aBuf);
	*aLen = 4+sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaWpsReadPin(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	dutCommand_t *staWpsReadPin= (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staWpsReadPin->intf, str, 15);
		}
	}

	wfaEncodeTLV(WFA_STA_WPS_READ_PIN_TLV, sizeof(dutCommand_t), (BYTE *)staWpsReadPin, aBuf);
	*aLen = 4+sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaWpsEnterPin(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaWpsEnterPin_t *staWpsEnterPin= (caStaWpsEnterPin_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_SUCCESS;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staWpsEnterPin->intf, str, 15);
		}
		else if(strcasecmp(str, "pin") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staWpsEnterPin->wpsPin, str, 15);
		}  
	}

	wfaEncodeTLV(WFA_STA_WPS_ENTER_PIN_TLV, sizeof(caStaWpsEnterPin_t), (BYTE *)staWpsEnterPin, aBuf);
	*aLen = 4+sizeof(caStaWpsEnterPin_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaGetPsk(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	dutCommand_t *staGetPsk= (dutCommand_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staGetPsk->intf, str, 15);
		}
	}

	wfaEncodeTLV(WFA_STA_GET_PSK_TLV, sizeof(dutCommand_t), (BYTE *)staGetPsk, aBuf);
	*aLen = 4+sizeof(dutCommand_t);

	return WFA_SUCCESS;
}

int xcCmdProcStaP2pJoin(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaP2pJoin_t *staP2pJoin= (caStaP2pJoin_t *) (aBuf+sizeof(wfaTLV));
	char *str;

	if(aBuf == NULL)
		return WFA_FAILURE;

	memset(aBuf, 0, *aLen);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
			break;

		if(strcasecmp(str, "interface") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staP2pJoin->intf, str, (strlen(str) > WFA_IF_NAME_LEN) ? WFA_IF_NAME_LEN : strlen(str));
		}
		else if(strcasecmp(str, "p2pdevid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staP2pJoin->devId, str, (strlen(str) > WFA_P2P_DEVID_LEN) ? WFA_P2P_DEVID_LEN : strlen(str));
		}
		else if(strcasecmp(str, "ssid") == 0)
		{
			str = strtok_r(NULL, ",", &pcmdStr);  
			strncpy(staP2pJoin->ssid, str, 15);
		}
	}

	wfaEncodeTLV(WFA_STA_P2P_JOIN_TLV, sizeof(caStaP2pJoin_t), (BYTE *)staP2pJoin, aBuf);
	*aLen = 4+sizeof(caStaP2pJoin_t);

	return WFA_SUCCESS;
}

#endif
