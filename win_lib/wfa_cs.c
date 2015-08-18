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
 * @file wfa_miscs.c
 * @brief File containing the miscellaneous utility rountines
*/

#include "wfa_dut.h"

#ifdef WFA_WMM_AC_EXT
extern int wfaTGSetPrio(int sockfd, int tgClass);
#endif

#define FW(x,y) FindWindowEx(x, NULL, y, L"")

char *wtsPath = "C:\\WFA";
char *capstr[] = {"Disable", "Enable"};

int wfaGetEnvVal(char * ,char * ,int);
int wfaClearEnvVal(char *);
int wfaExecuteCLI(char *CLI);

/** Respond "ca_get_version" command to controller
 * @param cmd not used.
 * @param valLen not used.
 * @param parms A buffer to store the version info response.
 * @return 0 as success.
 */
int agtCmdProcGetVersion(int len, BYTE *parms, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t *getverResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "entering agtCmdProcGetVersion ...\n");

	getverResp->status = STATUS_COMPLETE;
	strncpy(getverResp->cmdru.version, WFA_SYSTEM_VER, WFA_VERSION_LEN-1);

	wfaEncodeTLV(WFA_GET_VERSION_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)getverResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/** Force the station wireless I/F to re/associate with the AP
 * @param len not used.
 * @param caCmdBuf A buffer containing the command passed from control agent.
 * @param respLen The length of the command response string.
 * @param respBuf An already allocated buffer to store the command response.
 * @return 0 as success.
 */
int wfaStaAssociate(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCommand_t *assoc = (dutCommand_t *)caCmdBuf;
	char *ifname = assoc->intf;
	char *ssid = assoc->cmdsu.ssid;
	dutCmdResponse_t *staAssocResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaAssociate ...\n");
	strcpy(wfaDutAgentData.gnetIf, ifname);

	if (wfaDutAgentCAPIData.geSupplicant == eWindowsZeroConfig)
	{   
		sprintf(wfaDutAgentData.gCmdStr, "netsh wlan connect name=%s ssid=%s", ssid,ssid);
		DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
		system(wfaDutAgentData.gCmdStr);
	}  
	/*
	* Then report back to control PC for completion.
	* This does not have failed/error status. The result only tells
	* a completion.
	*/
	staAssocResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_ASSOCIATE_RESP_TLV, 4, (BYTE *)staAssocResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + 4;

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* wfaStaIsConnected():
*    The function is to check whether the station's wireless I/F has 
*    already connected to an AP.
*/
int wfaStaIsConnected(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCommand_t *connStat = (dutCommand_t *)caCmdBuf;
	FILE *tmpfile = NULL;
	char result[32],filename[256], intfile[256];
	dutCmdResponse_t *staConnectResp = &wfaDutAgentData.gGenericResp;
	char *ifname = connStat->intf;
	char string[64],Interfacename[64];

	DPRINT_INFOL(WFA_OUT, "Entering isConnected ...\n"); 

	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s\\Temp\\iscon.txt", wtsPath);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);

	system(wfaDutAgentData.gCmdStr);
	if(wfaDutAgentCAPIData.geSupplicant == eWindowsZeroConfig)
	{
		sprintf(filename, "%s\\Temp\\iscon.txt", wtsPath);
		sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
		tmpfile = fopen(intfile, "r");
		if(tmpfile == NULL)
		{
			DPRINT_ERR(WFA_ERR, "\n Error opening the interface file \n");
			staConnectResp->status = STATUS_ERROR;
			wfaEncodeTLV(WFA_STA_IS_CONNECTED_RESP_TLV, 4, (BYTE *)staConnectResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + 4;

			return WFA_FAILURE;
		}
		else
		{
			for(;;)
			{
				if(fgets(string, 256, tmpfile) == NULL)
					break; 
			}
			fclose(tmpfile);

			if(strncmp(string, "IFNAME", 6) == 0)
			{
				char *str;
				str = strtok(string, "\"");
				str = strtok(NULL, "\"");
				if(str != NULL)
				{
					strcpy(&Interfacename[0],str);
				}
			}
		}
		sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s\\Temp\\temp.txt",wtsPath);
		DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
		system(wfaDutAgentData.gCmdStr);

		sprintf(wfaDutAgentData.gCmdStr, "netsh wlan show  interface name=\"%s\" > %s\\Temp\\temp.txt",&Interfacename[0], wtsPath);
		DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
		system(wfaDutAgentData.gCmdStr);

		sprintf(wfaDutAgentData.gCmdStr, "FOR /F \"tokens=2 delims=: \" %s IN ('findstr State %s\\Temp\\temp.txt') DO @echo %s > %s\\Temp\\iscon.txt","%i",wtsPath, "%i", wtsPath);
		DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);      
		system(wfaDutAgentData.gCmdStr);
	}

	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n",wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);
	/*
	* the status is saved in a file.  Open the file and check it.
	*/

	tmpfile = fopen(filename, "r+");
	if(tmpfile == NULL)
	{
		staConnectResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_IS_CONNECTED_RESP_TLV, 4, (BYTE *)staConnectResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;

		DPRINT_ERR(WFA_ERR, "wfa_cs.c,wfaStaIsConnected::file open failed\n");
		return WFA_FAILURE;
	}

	if(wfaDutAgentCAPIData.geSupplicant == eWindowsZeroConfig)
	{
		fscanf(tmpfile, "%s", result); 
		if(strncmp(result, "connected", 9) == 0)
			staConnectResp->cmdru.connected = 1;
		else
			staConnectResp->cmdru.connected = 0;
	} 

	/*
	* Report back the status: Complete or Failed.
	*/
	staConnectResp->status = STATUS_COMPLETE;   
	wfaEncodeTLV(WFA_STA_IS_CONNECTED_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)staConnectResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	if(tmpfile)
		fclose(tmpfile);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* wfaStaGetIpConfig():
* This function is to retriev the ip info including
*     1. dhcp enable
*     2. ip address
*     3. mask 
*     4. primary-dns
*     5. secondary-dns
*
*     The current implementation is to use a script to find these information
*     and store them in a file. 
*/
int wfaStaGetIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	int i = 0;
	dutCmdResponse_t *ipconfigResp = &wfaDutAgentData.gGenericResp; 
	caStaGetIpConfigResp_t *ifinfo = &ipconfigResp->cmdru.getIfconfig;

	dutCommand_t *getIpConf = (dutCommand_t *)caCmdBuf;
	char *ifname = getIpConf->intf;
	FILE *tmpfd;
	char string[256],Interfacename[64], intfile[256], confile[256];
	char *str;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaGetIpConfig ...\n");

	strcpy(ifinfo->dns[0], "0");
	strcpy(ifinfo->dns[1], "0");

	sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
	tmpfd = fopen(intfile, "r");
	if(tmpfd == NULL)
	{
		ipconfigResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_GET_IP_CONFIG_RESP_TLV, 4, (BYTE *)ipconfigResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;

		DPRINT_ERR(WFA_ERR, "%s open failed\n", intfile);
		return WFA_FAILURE;
	}
	else
	{
		for(;;)
		{
			if(fgets(string, 256, tmpfd) == NULL)
				break; 
		}
		fclose(tmpfd);

		if(strncmp(string, "IFNAME", 6) == 0)
		{
			char *str;
			str = strtok(string, "\"");
			str = strtok(NULL, "\"");
			if(str != NULL)
			{
				strcpy(&Interfacename[0],str);
			}
		}
	}
	if(getIpConf->cmdsu.ipTypeV4V6 == 2)
	{
		sprintf(wfaDutAgentData.gCmdStr, "netsh interface ipv6 show addresses interface=\"%s\"  > %s\\Temp\\getcon.txt",&Interfacename[0], wtsPath);
	}
	else
	{
		sprintf(wfaDutAgentData.gCmdStr, "netsh interface ip show addresses name=\"%s\"  > %s\\Temp\\getcon.txt",&Interfacename[0], wtsPath);
	}
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	sprintf(confile, "%s\\Temp\\getcon.txt", wtsPath);
	tmpfd = fopen(confile, "r+");

	if(tmpfd == NULL)
	{
		ipconfigResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_GET_IP_CONFIG_RESP_TLV, 4, (BYTE *)ipconfigResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;

		DPRINT_ERR(WFA_ERR, "File open failed\n");
		return WFA_FAILURE;
	}

	for(;;)
	{
		if(fgets(string, 256, tmpfd) == NULL)
			break; 

		str = strtok(string, " ");
		if ( str != NULL)
		{
			/* check dhcp enabled */
			if(strncmp(str, "DHCP", 4) ==0)
			{
				str = strtok(NULL, " ");
				str = strtok(NULL, " ");
				if(strncmp(str, "Yes", 3) ==0)
					ifinfo->isDhcp = 1;
				else
					ifinfo->isDhcp = 0;
			}

			/* find out the ip address */
			if(strncmp(str, "IP", 2) ==0)
			{
				str = strtok(NULL, " ");
				str = strtok(NULL, " ");
				memset(ifinfo->ipaddr,'\0',16);
				if(str != NULL)
				{
					DPRINT_INFOL(WFA_OUT, "\nThe IP length %dEnd\n",strlen(str));
					DPRINT_INFOL(WFA_OUT, "\nThe IP  %sEnd\n",str);

					strncpy(ifinfo->ipaddr, str, 15);
					ifinfo->ipaddr[strlen(str)-1]='\0';
				}
				else
					strncpy(ifinfo->ipaddr, "none", 15);
			}

			/* ip v6 addresss */
			if(strncmp(str, "Address", 7) ==0)
			{
				ifinfo->ipTypeV6 = 1;
				str = strtok(NULL, " ");
				memset(ifinfo->ipV6addr,'\0',48);
				if(str != NULL)
				{
					DPRINT_INFOL(WFA_OUT, "\nThe IP length %dEnd\n",strlen(str));
					DPRINT_INFOL(WFA_OUT, "\nThe IP  %sEnd\n",str);

					strncpy(ifinfo->ipV6addr, str, strlen(str));
					ifinfo->ipaddr[strlen(str)-1]='\0';
				}
				else
					strncpy(ifinfo->ipaddr, "none", 15);
				break;
			}

			/* check the mask */
			if(strncmp(str, "Subnet", 6) ==0)
			{
				str = strtok(NULL, " ");
				str = strtok(NULL, " ");
				if(str != NULL)
				{
					strcpy(ifinfo->mask, str);
				}
				else
					strcpy(ifinfo->mask, "none");
			}
		}
	}

	fclose(tmpfd);

	/*
	* Report back the results
	*/
	ipconfigResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_GET_IP_CONFIG_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)ipconfigResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	DPRINT_INFOL(WFA_OUT, "%i %i %s %s %s %s %i\n", ipconfigResp->status, 
		ifinfo->isDhcp, ifinfo->ipaddr, ifinfo->mask, 
		ifinfo->dns[0], ifinfo->dns[1], *respLen);

	fclose(tmpfd);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* wfaStaSetIpConfig():
*   The function is to set the ip configuration to a wireless I/F.
*   1. IP address
*   2. Mac address
*   3. default gateway
*   4. dns nameserver (pri and sec).  
*/
int wfaStaSetIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	char *saveptr1, *saveptr2, *saveptr3;

	dutCommand_t *setIpConf = (dutCommand_t *)caCmdBuf;
	caStaSetIpConfig_t *ipconfig = &setIpConf->cmdsu.ipconfig;
	dutCmdResponse_t *staSetIpResp = &wfaDutAgentData.gGenericResp;

	char string[256],Interfacename[64], intfile[256];
	FILE *tmpfd;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetIpConfig ...\n");

	Sleep(2000);
	sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
	tmpfd = fopen(intfile, "r");

	if(tmpfd == NULL)
	{
		DPRINT_ERR(WFA_ERR, "\n Error opening the interface file \n");
		staSetIpResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_SET_IP_CONFIG_RESP_TLV, 4, (BYTE *)staSetIpResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;
		return WFA_FAILURE;
	}
	else
	{
		for(;;)
		{
			if(fgets(string, 256, tmpfd) == NULL)
				break; 
		}
		fclose(tmpfd);

		if(strncmp(string, "IFNAME", 6) == 0)
		{
			char *str;
			str = strtok(string, "\"");
			str = strtok(NULL, "\"");
			if(str != NULL)
			{
				strcpy(&Interfacename[0],str);
			}
		}
	}

	if(ipconfig->isDhcp == 1)
	{
		sprintf(wfaDutAgentData.gCmdStr, "netsh interface ip set address name=\"%s\" source=dhcp", &Interfacename[0]);
		DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
		system(wfaDutAgentData.gCmdStr);
	}
	else
	{
		sprintf(wfaDutAgentData.gCmdStr, "netsh interface ip set address name=\"%s\" static %s %s ", &Interfacename[0],ipconfig->ipaddr,ipconfig->mask);
		DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
		system(wfaDutAgentData.gCmdStr);

		saveptr3 = strtok_r(ipconfig->pri_dns, ".", &saveptr1);
		if (saveptr3 == NULL)
			goto endcleanup;

		if (strtok_r(NULL, ".", &saveptr1) != NULL)        
		{
			sprintf(wfaDutAgentData.gCmdStr, "netsh interface ip set dns name=\"%s\" static addr=%s", &Interfacename[0],ipconfig->pri_dns);
			DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
			system(wfaDutAgentData.gCmdStr);
			strtok_r(ipconfig->sec_dns, ".", &saveptr2);
			if (strtok_r(NULL, ".", &saveptr2) != NULL)	        
			{
				sprintf(wfaDutAgentData.gCmdStr, "netsh interface ip add dns name=\"%s\" addr=%s", &Interfacename[0],ipconfig->sec_dns);
				DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
				system(wfaDutAgentData.gCmdStr);
			}
		}
	}

	/*
	* report status
	*/
endcleanup:
	staSetIpResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_IP_CONFIG_RESP_TLV, 4, (BYTE *)staSetIpResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + 4;

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* wfaStaVerifyIpConnection():
* The function is to verify if the station has IP connection with an AP by
* send ICMP/pings to the AP.
*/ 
int wfaStaVerifyIpConnection(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCommand_t *verip = (dutCommand_t *)caCmdBuf;
	dutCmdResponse_t *verifyIpResp = &wfaDutAgentData.gGenericResp;

#ifndef WFA_PING_UDP_ECHO_ONLY
	char strout[64], *pcnt;
	FILE *tmpfile;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaVerifyIpConnection ...\n");

	/* set timeout value in case not set */
	if(verip->cmdsu.verifyIp.timeout <= 0)
	{
		verip->cmdsu.verifyIp.timeout = 10;
	}

	/* execute the ping command  and pipe the result to a tmp file */
	sprintf(wfaDutAgentData.gCmdStr, "ping %s -c 3 -W %u | grep loss | cut -f3 -d, 1>& /tmp/pingout.txt", verip->cmdsu.verifyIp.dipaddr, verip->cmdsu.verifyIp.timeout);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr); 

	/* scan/check the output */
	tmpfile = fopen("/tmp/pingout.txt", "r+");
	if(tmpfile == NULL)
	{
		verifyIpResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV, 4, (BYTE *)verifyIpResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;

		DPRINT_ERR(WFA_ERR, "File open failed\n");
		return WFA_FAILURE;
	}

	verifyIpResp->status = STATUS_COMPLETE;
	if(fscanf(tmpfile, "%s", strout) == EOF)
		verifyIpResp->cmdru.connected = 0;
	else
	{
		pcnt = strtok(strout, "%"); 

		/* if the loss rate is 100%, not able to connect */
		if(atoi(pcnt) == 100)
			verifyIpResp->cmdru.connected = 0;
		else
			verifyIpResp->cmdru.connected = 1;
	}

	fclose(tmpfile);
#else
	int btSockfd;
	struct pollfd fds[2];
	int timeout = 2000;
	char anyBuf[64];
	struct sockaddr_in toAddr;
	int done = 1, cnt = 0, ret, nbytes;

	verifyIpResp->status = STATUS_COMPLETE;
	verifyIpResp->cmdru.connected = 0;

	btSockfd = wfaCreateUDPSock("127.0.0.1", WFA_UDP_ECHO_PORT);

	if(btSockfd == -1)
	{
		verifyIpResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV, 4, (BYTE *)verifyIpResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;      
		return WFA_FAILURE;      
	}

	toAddr.sin_family = AF_INET;
	toAddr.sin_addr.s_addr = inet_addr(verip->cmdsu.verifyIp.dipaddr);
	toAddr.sin_port = htons(WFA_UDP_ECHO_PORT);

	while(done)
	{
		wfaTrafficSendTo(btSockfd, (char *)anyBuf, 64, (struct sockaddr *)&toAddr);
		cnt++;

		fds[0].fd = btSockfd;
		fds[0].events = POLLIN | POLLOUT; 

		ret =  WSAPoll(fds, 1, timeout);
		switch(ret)
		{
		case 0:
			/* it is time out, count a packet lost*/
			break;
		case -1:
			/* it is an error */
		default:
			{
				switch(fds[0].revents)
				{
				case POLLIN:
				case POLLPRI:
				case POLLOUT:
					nbytes = wfaTrafficRecv(btSockfd, (char *)anyBuf, (struct sockaddr *)&toAddr);
					if(nbytes != 0)
						verifyIpResp->cmdru.connected = 1;
					done = 0;
					break;
				default:
					/* errors but not care */
					;
				} 
				break;
			}
		} 
		if(cnt == 3)
		{
			done = 0;
		}
	}       
#endif

	wfaEncodeTLV(WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)verifyIpResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* wfaStaGetMacAddress()
*    This function is to retrieve the MAC address of a wireless I/F.
*/
int wfaStaGetMacAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCommand_t *getMac = (dutCommand_t *)caCmdBuf;
	dutCmdResponse_t *getmacResp = &wfaDutAgentData.gGenericResp;
	char *str;
	char *ifname = getMac->intf;
	FILE *tmpfd;
	char string[128],Interfacename[128], intfile[128];
	char filename[50];

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaGetMacAddress ...\n");

	sprintf(filename,"%s\\Temp\\ipconfig.txt", wtsPath);
	sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
	DPRINT_INFOL(WFA_OUT, "filename %s intfile %s\n", filename, intfile);

	tmpfd = fopen(intfile, "r");
	if(tmpfile == NULL)
	{
		DPRINT_ERR(WFA_ERR, "\n Error opening the interface file \n");
		getmacResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_GET_MAC_ADDRESS_RESP_TLV, 4, (BYTE *)getmacResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;

		return WFA_FAILURE;
	}
	else
	{
		for(;;)
		{
			if(fgets(string, 256, tmpfd) == NULL)
				break; 
		}
		fclose(tmpfd);

		if(strncmp(string, "IFNAME", 6) == 0)
		{
			char *str;
			str = strtok(string, "\"");
			str = strtok(NULL, "\"");
			if(str != NULL)
			{
				strcpy(&Interfacename[0],str);
			}
		}
	}
	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s\\Temp\\temp.txt", wtsPath);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);

	sprintf(wfaDutAgentData.gCmdStr, "netsh wlan show  interface state name=\"%s\" > %s\\Temp\\temp.txt",&Interfacename[0], wtsPath);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	sprintf(wfaDutAgentData.gCmdStr, "FOR /F \"tokens=3,4,5,6,7,8 delims=: \" %s IN ('findstr Physical %s\\Temp\\temp.txt') DO @echo %s:%s:%s:%s:%s:%s > %s\\Temp\\ipconfig.txt","%i",wtsPath, "%i","%j","%k","%l","%m","%n", wtsPath);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	Sleep(1000);

	tmpfd = fopen(filename, "r+");
	if(tmpfd == NULL)
	{
		getmacResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_GET_MAC_ADDRESS_RESP_TLV, 4, (BYTE *)getmacResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;

		DPRINT_ERR(WFA_ERR, "wfaStaGetMacAddress; file open failed\n");
		return WFA_FAILURE;
	}

	if(fgets(string, 256, tmpfd) == NULL)
	{
		getmacResp->status = STATUS_ERROR;
	}
	str = strtok(string, " ");

	if(str)
	{        
		strcpy(getmacResp->cmdru.mac, str);
		getmacResp->status = STATUS_COMPLETE;
	}
	else
	{
		getmacResp->status = STATUS_ERROR;
	}

	wfaEncodeTLV(WFA_STA_GET_MAC_ADDRESS_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)getmacResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	fclose(tmpfd);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* wfaStaGetStats():
* The function is to retrieve the statistics of the I/F's layer 2 txFrames, 
* rxFrames, txMulticast, rxMulticast, fcsErrors/crc, and txRetries.
* Currently there is not definition how to use these info. 
*/
int wfaStaGetStats(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t *statsResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaGetStats ...\n");

	statsResp->status = STATUS_ERROR;
	wfaEncodeTLV(WFA_STA_GET_STATS_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)statsResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
*  Since WEP is optional, this function could be used to replace
*  wfaSetEncryption() if necessary. 
*/
int wfaSetEncryptionZeroConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEncryption_t *setEncryp = (caStaSetEncryption_t *)caCmdBuf;
	dutCmdResponse_t *setEncrypResp = &wfaDutAgentData.gGenericResp;

	FILE *file,*tmpfd;
	char string[256],Interfacename[64], pfile[128], intfile[128];

	DPRINT_INFOL(WFA_OUT, "Entering wfaSetEncryptionZeroConfig ...\n"); 

	sprintf(pfile, "%s\\Temp\\tmp.xml", wtsPath);
	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s", pfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	file = fopen(pfile, "w+");
	if(file==NULL) 
	{
		DPRINT_ERR(WFA_ERR, "Error: can't create file.\n");
		setEncrypResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_SET_ENCRYPTION_RESP_TLV, 4, (BYTE *)setEncrypResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;

		return WFA_FAILURE;
	}
	else 
	{
		sprintf(wfaDutAgentData.gCmdStr,"<?xml version=\"1.0\"?>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<name>%s</name>\n",setEncryp->ssid);
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<SSIDConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<SSID>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<name>%s</name>\n",setEncryp->ssid);
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t</SSID>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t</SSIDConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<connectionType>ESS</connectionType>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<connectionMode>auto</connectionMode>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<autoSwitch>true</autoSwitch>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<MSM>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<security>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<authEncryption>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<authentication>open</authentication>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		if(setEncryp->encpType == 1)
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<encryption>WEP</encryption>\n");
		else
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<encryption>none</encryption>\n");

		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t <useOneX>false</useOneX>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t</authEncryption>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		if(setEncryp->encpType == 1)
		{
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<sharedKey>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t<keyType>networkKey</keyType>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t<protected>false</protected>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t<keyMaterial>%s</keyMaterial>\n",setEncryp->keys[0]);
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t</sharedKey>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
		}

		sprintf(wfaDutAgentData.gCmdStr,"\t\t</security>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t</MSM>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"</WLANProfile>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
	}
	fclose(file);

	sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
	tmpfd = fopen(intfile, "r");
	if(tmpfd == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Error opening the interface file \n");
		setEncrypResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_SET_ENCRYPTION_RESP_TLV, 4, (BYTE *)setEncrypResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;

		return WFA_FAILURE;
	}
	else
	{
		for(;;)
		{
			if(fgets(string, 256, tmpfd) == NULL)
				break; 
		}
		fclose(tmpfd);

		if(strncmp(string, "IFNAME", 6) == 0)
		{
			char *str;
			str = strtok(string, "\"");
			str = strtok(NULL, "\"");
			if(str != NULL)
			{
				strcpy(&Interfacename[0],str);
			}
		}
	}

	sprintf(wfaDutAgentData.gCmdStr, "netsh wlan add profile filename=\"%s\" interface=\"%s\" user=all", pfile,&Interfacename[0]);

	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	setEncrypResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_ENCRYPTION_RESP_TLV, 4, (BYTE *)setEncrypResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}


int wfaSetEncryption(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEncryption_t *setEncryp = (caStaSetEncryption_t *)caCmdBuf;
	dutCmdResponse_t *setEncrypResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaSetEncryption ...\n");

	switch(wfaDutAgentCAPIData.geSupplicant)
	{
	case eWindowsZeroConfig:
		wfaSetEncryptionZeroConfig(len,caCmdBuf,respLen,respBuf);			
		break;

	default:
		setEncrypResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_ENCRYPTION_RESP_TLV, 4, (BYTE *)setEncrypResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;		    
	}

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}


/*
* wfaStaSetEapTLSZeroConfig(): For ZeroConfig
*   This is to set
*   1. ssid
*   2. encrypType - tkip or aes-ccmp
*   3. keyManagementType - wpa or wpa2
*   4. trustedRootCA
*   5. clientCertificate
*  The path for the certificate thumbprint file is c:\WFA\Sigma\Certificates\<username>.txt
*/
int wfaStaSetEapTLSZeroConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEapTLS_t *setTLS = (caStaSetEapTLS_t *)caCmdBuf;
	char *ifname = setTLS->intf;
	dutCmdResponse_t *setEapTlsResp = &wfaDutAgentData.gGenericResp;
	FILE *file,*tmpfd;
	char string[256],Interfacename[64], pfile[128], intfile[128];

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetEapTLSZeroConfig...\n");

	sprintf(pfile, "%s\\Temp\\tmp.xml", wtsPath);
	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s", pfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	file = fopen(pfile, "w+");
	if(file==NULL) 
	{
		DPRINT_ERR(WFA_ERR, "Can't create file.\n");
		setEapTlsResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_SET_EAPTLS_RESP_TLV, 4, (BYTE *)setEapTlsResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;

		return WFA_FAILURE;
	}
	else 
	{
		sprintf(wfaDutAgentData.gCmdStr,"<?xml version=\"1.0\"?>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<name>%s</name>\n",setTLS->ssid);
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<SSIDConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<SSID>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<name>%s</name>\n",setTLS->ssid);
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t</SSID>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<nonBroadcast>false</nonBroadcast>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t</SSIDConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<connectionType>ESS</connectionType>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<connectionMode>auto</connectionMode>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<autoSwitch>true</autoSwitch>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<MSM>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<security>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<authEncryption>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		if((strcmp(setTLS->keyMgmtType, "WPA2") == 0) || (strcmp(setTLS->keyMgmtType, "wpa2") == 0))
		{
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<authentication>WPA2</authentication>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<encryption>AES</encryption>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
		}
		else if((strcmp(setTLS->keyMgmtType, "WPA") == 0) || (strcmp(setTLS->keyMgmtType, "wpa") == 0))
		{
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<authentication>WPA</authentication>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<encryption>TKIP</encryption>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
		}

		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<useOneX>true</useOneX>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t</authEncryption>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<OneX xmlns=\"http://www.microsoft.com/networking/OneX/v1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<EAPConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t<EapHostConfig xmlns=\"http://www.microsoft.com/provisioning/EapHostConfig\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t<EapMethod>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t<Type xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">13</Type>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t<VendorId xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">0</VendorId>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t<VendorType xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">0</VendorType>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t<AuthorId xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">0</AuthorId>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t</EapMethod>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t<ConfigBlob>020000002A00000007000000000000000000000000000000000000000000000000000000000000000000</ConfigBlob>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t</EapHostConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t</EAPConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t</OneX>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t</security>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t</MSM>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"</WLANProfile>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
	}
	fclose(file);

	sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
	tmpfd = fopen(intfile, "r");
	if(tmpfd == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Error opening the interface file \n");
		setEapTlsResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_SET_EAPTLS_RESP_TLV, 4, (BYTE *)setEapTlsResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;

		return WFA_FAILURE;
	}
	else
	{
		for(;;)
		{
			if(fgets(string, 256, tmpfd) == NULL)
				break; 
		}
		fclose(tmpfd);

		if(strncmp(string, "IFNAME", 6) == 0)
		{
			char *str;
			str = strtok(string, "\"");
			str = strtok(NULL, "\"");
			if(str != NULL)
			{
				strcpy(&Interfacename[0],str);
			}
		}
	}

	sprintf(wfaDutAgentData.gCmdStr, "netsh wlan add profile filename=\"%s\" interface=\"%s\" user=all",pfile, &Interfacename[0]);

	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	setEapTlsResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_EAPTLS_RESP_TLV, 4, (BYTE *)setEapTlsResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* wfaStaSetEapTLS():
*   This is to set
*   1. ssid
*   2. encrypType - tkip or aes-ccmp
*   3. keyManagementType - wpa or wpa2
*   4. trustedRootCA
*   5. clientCertificate
*/
int wfaStaSetEapTLS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEapTLS_t *setTLS = (caStaSetEapTLS_t *)caCmdBuf;
	char *ifname = setTLS->intf;
	dutCmdResponse_t *setEapTlsResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetEapTLS...\n");

	switch(wfaDutAgentCAPIData.geSupplicant)
	{
	case eWindowsZeroConfig:
		wfaStaSetEapTLSZeroConfig(len,caCmdBuf,respLen,respBuf);			
		break;	   
	default:
		setEapTlsResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_EAPTLS_RESP_TLV, 4, (BYTE *)setEapTlsResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;		    
	}

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* The function is to set 
*   1. ssid
*   2. passPhrase
*   3. keyMangementType - wpa/wpa2
*   4. encrypType - tkip or aes-ccmp
* Using Windows WpaSupplicant supplicant  
* 
*/


/*
* The function is to set 
*   1. ssid
*   2. passPhrase
*   3. keyMangementType - wpa/wpa2
*   4. encrypType - tkip or aes-ccmp
*/
int wfaStaSetPSKZeroConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetPSK_t *setPSK = (caStaSetPSK_t *)caCmdBuf;
	dutCmdResponse_t *setPskResp = &wfaDutAgentData.gGenericResp;

	FILE *file,*tmpfd;
	char string[256],Interfacename[64], pfile[128], intfile[128];

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetPSKZeroConfig...\n");

	sprintf(pfile, "%s\\Temp\\tmp.xlm", wtsPath);
	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s", pfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	file = fopen(pfile, "w+");

	/* we create a file for reading and writing */

	if(file==NULL) 
	{
		DPRINT_ERR(WFA_ERR, "Can't create file.\n");
		setPskResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_SET_PSK_RESP_TLV, 4, (BYTE *)setPskResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;

		return WFA_FAILURE;
	}
	else 
	{
		sprintf(wfaDutAgentData.gCmdStr,"<?xml version=\"1.0\"?>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<name>%s</name>\n",setPSK->ssid);
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<SSIDConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<SSID>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<name>%s</name>\n",setPSK->ssid);
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t</SSID>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t</SSIDConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<connectionType>ESS</connectionType>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<connectionMode>auto</connectionMode>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<autoSwitch>true</autoSwitch>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<MSM>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<security>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<authEncryption>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		if(setPSK->encpType == ENCRYPT_TKIP)
		{
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<authentication>WPAPSK</authentication>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<encryption>TKIP</encryption>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
		}
		else if(setPSK->encpType == ENCRYPT_AESCCMP)
		{
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<authentication>WPA2PSK</authentication>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<encryption>AES</encryption>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
		}
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t <useOneX>false</useOneX>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t</authEncryption>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<sharedKey>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t<keyType>passPhrase</keyType>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t<protected>false</protected>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t<keyMaterial>%s</keyMaterial>\n",setPSK->passphrase);
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t</sharedKey>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t</security>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t</MSM>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"</WLANProfile>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
	}
	fclose(file);


	sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
	tmpfd = fopen(intfile, "r");
	if(tmpfd == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Error opening the interface file \n");
		setPskResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_SET_PSK_RESP_TLV, 4, (BYTE *)setPskResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;

		return WFA_FAILURE;
	}
	else
	{
		for(;;)
		{
			if(fgets(string, 256, tmpfd) == NULL)
				break; 
		}
		fclose(tmpfd);

		if(strncmp(string, "IFNAME", 6) == 0)
		{
			char *str;
			str = strtok(string, "\"");
			str = strtok(NULL, "\"");
			if(str != NULL)
			{
				strcpy(&Interfacename[0],str);
			}
		}
	}

	sprintf(wfaDutAgentData.gCmdStr, "netsh wlan add profile filename=\"%s\" interface=\"%s\" user=all",pfile,&Interfacename[0]);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	setPskResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_PSK_RESP_TLV, 4, (BYTE *)setPskResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + 4;

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS; 
}

int wfaStaSetPSK(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetPSK_t *setPSK = (caStaSetPSK_t *)caCmdBuf;
	dutCmdResponse_t *setPskResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetPSK...\n");

	switch(wfaDutAgentCAPIData.geSupplicant)
	{
	case eWindowsZeroConfig:
		wfaStaSetPSKZeroConfig(len,caCmdBuf,respLen,respBuf);			
		break;	   
	default:
		setPskResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_PSK_RESP_TLV, 4, (BYTE *)setPskResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;		
	}

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS; 
}


/*
* wfaStaGetInfo(): 
* Get vendor specific information in name/value pair by a wireless I/F.
*/
int wfaStaGetInfo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	dutCommand_t *getInfo = (dutCommand_t *)caCmdBuf;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaGetInfo...\n");

	/*
	* Normally this is called to retrieve the vendor information
	* from a interface, no implement yet
	*/
	sprintf(infoResp.cmdru.info, "interface,%s,vendor,XXX,cardtype,802.11", getInfo->intf);

	infoResp.status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_GET_INFO_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");

	return WFA_SUCCESS;
}

/*
* wfaStaSetEapTTLS():
*   This is to set
*   1. ssid
*   2. username
*   3. passwd
*   4. encrypType - tkip or aes-ccmp
*   5. keyManagementType - wpa or wpa2
*   6. trustedRootCA
*/
int wfaStaSetEapTTLS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEapTTLS_t *setTTLS = (caStaSetEapTTLS_t *)caCmdBuf;
	char *ifname = setTTLS->intf;
	dutCmdResponse_t *setEapTtlsResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetEapTTLS...\n");

	switch(wfaDutAgentCAPIData.geSupplicant)
	{
	case eWindowsZeroConfig:
		setEapTtlsResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_EAPTTLS_RESP_TLV, 4, (BYTE *)setEapTtlsResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;			
		break;

	default:
		setEapTtlsResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_PSK_RESP_TLV, 4, (BYTE *)setEapTtlsResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;
		break;
	}

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");

	return WFA_SUCCESS;
}


/*
* wfaStaSetEapSIM():
*   This is to set
*   1. ssid
*   2. user name
*   3. passwd
*   4. encrypType - tkip or aes-ccmp
*   5. keyMangementType - wpa or wpa2
*/
int wfaStaSetEapSIM(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEapSIM_t *setSIM = (caStaSetEapSIM_t *)caCmdBuf;
	char *ifname = setSIM->intf;
	dutCmdResponse_t *setEapSimResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetEapSIM...\n");

	switch(wfaDutAgentCAPIData.geSupplicant)
	{
	case eWindowsZeroConfig:
		setEapSimResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_EAPSIM_RESP_TLV, 4, (BYTE *)setEapSimResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;			
		break;

	default:
		setEapSimResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_EAPSIM_RESP_TLV, 4, (BYTE *)setEapSimResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;
		break;
	}

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");

	return WFA_SUCCESS;
}

/*
* wfaStaSetPEAP(): For ZeroConfig
*   This is to set
*   1. ssid
*   2. user name
*   3. passwd
*   4. encryType - tkip or aes-ccmp
*   5. keyMgmtType - wpa or wpa2
*   6. trustedRootCA
*   7. innerEAP
*   8. peapVersion
*/
int wfaStaSetPEAPZeroConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEapPEAP_t *setPEAP = (caStaSetEapPEAP_t *)caCmdBuf;
	char *ifname = setPEAP->intf;
	dutCmdResponse_t *setPeapResp = &wfaDutAgentData.gGenericResp;

	FILE *file,*tmpfd;
	char string[256],Interfacename[64], pfile[128], intfile[128];

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetPEAPZeroConfig...\n");

	sprintf(pfile, "%s\\Temp\\tmp.xml", wtsPath);
	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s", pfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	file = fopen(pfile, "w+");

	/* we create a file for reading and writing */
	if(file==NULL) 
	{
		DPRINT_ERR(WFA_ERR, "Can't create file.\n");
		setPeapResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_SET_PEAP_RESP_TLV, 4, (BYTE *)setPeapResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;

		return WFA_FAILURE;
	}
	else 
	{
		sprintf(wfaDutAgentData.gCmdStr,"<?xml version=\"1.0\"?>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<name>%s</name>\n",setPEAP->ssid);
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<SSIDConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<SSID>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<name>%s</name>\n",setPEAP->ssid);
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t</SSID>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<nonBroadcast>false</nonBroadcast>\n");
		fputs(wfaDutAgentData.gCmdStr, file);	
		sprintf(wfaDutAgentData.gCmdStr,"\t</SSIDConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<connectionType>ESS</connectionType>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<connectionMode>auto</connectionMode>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<autoSwitch>true</autoSwitch>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t<MSM>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t<security>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<authEncryption>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		if((strcmp(setPEAP->keyMgmtType, "WPA2") == 0) || (strcmp(setPEAP->keyMgmtType, "wpa2") == 0))
		{
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<authentication>WPA2</authentication>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<encryption>AES</encryption>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
		}
		else if((strcmp(setPEAP->keyMgmtType, "WPA") == 0) || (strcmp(setPEAP->keyMgmtType, "wpa") == 0))
		{
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<authentication>WPA</authentication>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
			sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<encryption>TKIP</encryption>\n");
			fputs(wfaDutAgentData.gCmdStr, file);
		}


		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<useOneX>true</useOneX>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t</authEncryption>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t<OneX xmlns=\"http://www.microsoft.com/networking/OneX/v1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<cacheUserData>true</cacheUserData>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<authMode>machineOrUser</authMode>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<singleSignOn>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<type>preLogon</type>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<maxDelay>10</maxDelay>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<allowAdditionalDialogs>true</allowAdditionalDialogs>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<userBasedVirtualLan>false</userBasedVirtualLan>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t</singleSignOn>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t<EAPConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t<EapHostConfig xmlns=\"http://www.microsoft.com/provisioning/EapHostConfig\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t<EapMethod>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t<Type xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">25</Type>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t<VendorId xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">0</VendorId>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t<VendorType xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">0</VendorType>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t<AuthorId xmlns=\"http://www.microsoft.com/provisioning/EapCommon\">0</AuthorId>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t</EapMethod>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<Config xmlns=\"http://www.microsoft.com/provisioning/EapHostConfig\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<Eap xmlns=\"http://www.microsoft.com/provisioning/BaseEapConnectionPropertiesV1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<Type>25</Type>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<EapType xmlns=\"http://www.microsoft.com/provisioning/MsPeapConnectionPropertiesV1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<ServerValidation>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<DisableUserPromptForServerValidation>false</DisableUserPromptForServerValidation>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<ServerNames></ServerNames>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t</ServerValidation>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<FastReconnect>false</FastReconnect>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<InnerEapOptional>false</InnerEapOptional>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<Eap xmlns=\"http://www.microsoft.com/provisioning/BaseEapConnectionPropertiesV1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<Type>26</Type>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<EapType xmlns=\"http://www.microsoft.com/provisioning/MsChapV2ConnectionPropertiesV1\">\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<UseWinLogonCredentials>false</UseWinLogonCredentials>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t</EapType>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t</Eap>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<EnableQuarantineChecks>false</EnableQuarantineChecks>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<RequireCryptoBinding>false</RequireCryptoBinding>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<PeapExtensions>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<PerformServerValidation xmlns=\"http://www.microsoft.com/provisioning/MsPeapConnectionPropertiesV2\">false</PerformServerValidation>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t<AcceptServerName xmlns=\"http://www.microsoft.com/provisioning/MsPeapConnectionPropertiesV2\">false</AcceptServerName>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t</PeapExtensions>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t</EapType>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t</Eap>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t\t\t\t</Config>\n");
		fputs(wfaDutAgentData.gCmdStr, file);

		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t\t</EapHostConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t\t</EAPConfig>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t\t</OneX>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t\t</security>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"\t</MSM>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
		sprintf(wfaDutAgentData.gCmdStr,"</WLANProfile>\n");
		fputs(wfaDutAgentData.gCmdStr, file);
	}
	fclose(file);

	sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
	tmpfd = fopen(intfile, "r");
	if(tmpfd == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Error opening the interface file \n");
		setPeapResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_SET_PEAP_RESP_TLV, 4, (BYTE *)setPeapResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;

		return WFA_FAILURE;
	}
	else
	{
		for(;;)
		{
			if(fgets(string, 256, tmpfd) == NULL)
				break; 
		}
		fclose(tmpfd);

		if(strncmp(string, "IFNAME", 6) == 0)
		{
			char *str;
			str = strtok(string, "\"");
			str = strtok(NULL, "\"");
			if(str != NULL)
			{
				strcpy(&Interfacename[0],str);
			}
		}
	}

	sprintf(wfaDutAgentData.gCmdStr, "netsh wlan add profile filename=\"%s\" interface=\"%s\" user=all",pfile,&Interfacename[0]);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);


	setPeapResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_PEAP_RESP_TLV, 4, (BYTE *)setPeapResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");

	return WFA_SUCCESS;
}


/*
* wfaStaSetPEAP()
*   This is to set
*   1. ssid
*   2. user name
*   3. passwd
*   4. encryType - tkip or aes-ccmp
*   5. keyMgmtType - wpa or wpa2
*   6. trustedRootCA
*   7. innerEAP
*   8. peapVersion
*/
int wfaStaSetPEAP(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEapPEAP_t *setPEAP = (caStaSetEapPEAP_t *)caCmdBuf;
	char *ifname = setPEAP->intf;
	dutCmdResponse_t *setPeapResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetPEAP...\n");


	switch(wfaDutAgentCAPIData.geSupplicant)
	{
	case eWindowsZeroConfig:
		wfaStaSetPEAPZeroConfig(len,caCmdBuf,respLen,respBuf);
		break;

	default:
		setPeapResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_PEAP_RESP_TLV, 4, (BYTE *)setPeapResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;
		break;
	}

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");

	return WFA_SUCCESS;
}

/*
* wfaStaSetUAPSD()
*    This is to set
*    1. acBE
*    2. acBK
*    3. acVI
*    4. acVO
*/
int wfaStaSetUAPSD(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetUAPSD_t *setUAPSD = (caStaSetUAPSD_t *)caCmdBuf;
	char *ifname = setUAPSD->intf;    
	dutCmdResponse_t *setUAPSDResp = &wfaDutAgentData.gGenericResp;
	BYTE acBE=1;
	BYTE acBK=1;
	BYTE acVO=1;
	BYTE acVI=1;
	int st = 0;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetUAPSD...\n");    

	if(setUAPSD->acBE != 1)
		acBE=setUAPSD->acBE = 0;
	if(setUAPSD->acBK != 1)
		acBK=setUAPSD->acBK = 0;
	if(setUAPSD->acVO != 1)
		acVO=setUAPSD->acVO = 0;
	if(setUAPSD->acVI != 1)
		acVI=setUAPSD->acVI = 0;

	setUAPSDResp->status = STATUS_ERROR;
	strcpy(wfaDutAgentCAPIData.gStaSSID, setUAPSD->ssid); 

	switch(wfaDutAgentData.vend)
	{
	case WMM_ATHEROS:      
		setUAPSDResp->status = STATUS_COMPLETE;            
		break;
	case WMM_BROADCOMM:
		setUAPSDResp->status = STATUS_COMPLETE;
		break;
	case WMM_INTEL:
		DPRINT_ERR(WFA_ERR, "Intel not support yet on wmmps UAPSD\n");
		setUAPSDResp->status = STATUS_INVALID;
		goto end_cleanup; /*  tmp code */
		break;
	case WMM_MARVELL:
		DPRINT_ERR(WFA_ERR, "Marvell not support yet on wmmps UAPSD\n");
		setUAPSDResp->status = STATUS_INVALID;
		goto end_cleanup; /*  tmp code */
		break;
	case WMM_RALINK:
		setUAPSDResp->status = STATUS_COMPLETE;          
		break;
	case WMM_STAUT:
		sprintf(wfaDutAgentData.gCmdStr, "cd %s &  sta_set_uapsd /interface staut /ssid %s /acBE %d /acBK %d /acVI %d /acVO %d",  wfaDutAgentData.WFA_CLI_CMD_DIR, /*WFA_CLI_CMD_DIR,*/setUAPSD->ssid, setUAPSD->acBE, setUAPSD->acBK, setUAPSD->acVI, setUAPSD->acVO);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		DPRINT_INFOL(WFA_OUT,"\nWMM_STAUT set UAPSD:%s status=%d\n",wfaDutAgentData.gCmdStr, st);
		break;
	default:
		DPRINT_ERR(WFA_ERR, "Unknown Vendor, vendorIdex=%d\n", wfaDutAgentData.vend);
		setUAPSDResp->status = STATUS_INVALID;
		goto end_cleanup;
	}

	switch(st)
	{
	case 0:
		setUAPSDResp->status = STATUS_COMPLETE;
		break;
	case 1:
		setUAPSDResp->status = STATUS_ERROR;
		break;
	case 2:
		setUAPSDResp->status = STATUS_INVALID;
		break;
	}

end_cleanup:    
	wfaEncodeTLV(WFA_STA_SET_UAPSD_RESP_TLV, 4, (BYTE *)setUAPSDResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	DPRINT_INFOL(WFA_OUT, "wfaStaSetUAPSD Completing ...\n");
	return WFA_SUCCESS;
}

int wfaDeviceGetInfo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t *infoResp = &wfaDutAgentData.gGenericResp;
	FILE *tmpfile;
	/*a vendor can fill in the proper info or anything non-disclosure */
	caDeviceGetInfoResp_t dinfo;
	int i;
	char string[512];
	char tfile[128], resfile[128];
	char *str;

	DPRINT_INFOL(WFA_OUT, "Entering wfaDeviceGetInfo() ...\n");

	sprintf(tfile, "%s\\Temp\\temp.txt", wtsPath);
	sprintf(resfile, "%s\\Temp\\result.txt", wtsPath);

	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s",tfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);
	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s",resfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	sprintf(wfaDutAgentData.gCmdStr, "netsh wlan show  drivers > %s", tfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	sprintf(wfaDutAgentData.gCmdStr, "FOR /F \"tokens=2 delims=:\" %s IN ('findstr Vendor %s') DO @echo %s > %s","%i",tfile, "%i", resfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	tmpfile = fopen(resfile, "r");
	if(tmpfile == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Error opening the result.txt file \n");
		infoResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_DEVICE_GET_INFO_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)infoResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

		return WFA_FAILURE;
	}
	else
	{
		for(;;)
		{
			if(fgets(string, 256, tmpfile) == NULL)
				break; 
		}
		fclose(tmpfile);
		i=strlen(string);
		string[i]='\0';
	}

	str = strtok(string, " ");
	if(str != NULL)
	{
		strncpy(dinfo.vendor, str,16);		
		DPRINT_INFOL(WFA_OUT, "The Vendor %s", str);
	}

	sprintf(wfaDutAgentData.gCmdStr, "FOR /F \"tokens=2 delims=:\" %s IN ('findstr Version %s') DO @echo %s > %s","%i",tfile, "%i", resfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);


	tmpfile = fopen(resfile, "r");
	if(tmpfile == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Error opening the result.txt file \n");
		infoResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_DEVICE_GET_INFO_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)infoResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

		return WFA_FAILURE;
	}
	else
	{
		for(;;)
		{
			if(fgets(string, 256, tmpfile) == NULL)
				break; 
		}
		fclose(tmpfile);
		i=strlen(string);
		string[i]='\0';
	}

	strncpy(dinfo.version, string,16);		
	DPRINT_INFOL(WFA_OUT, "The Version %s", string);

	strncpy(dinfo.model, wfaDutAgentData.WFA_CLI_VERSION,16);		
	DPRINT_INFOL(WFA_OUT, "The model %s", string);

	memcpy(&infoResp->cmdru.devInfo, &dinfo, sizeof(caDeviceGetInfoResp_t));

	infoResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_DEVICE_GET_INFO_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* This funciton is to retrieve a list of interfaces and return
* the list back to Agent control.
* ********************************************************************
* Note: We intend to make this WLAN interface name as a hardcode name.
* Therefore, for a particular device, you should know and change the name
* for that device while doing porting. The MACRO "WFA_STAUT_IF" is defined in 
* the file "inc/wfa_ca.h". If the device OS is not linux-like, this most 
* likely is hardcoded just for CAPI command responses.
* *******************************************************************
* 
*/
int wfaDeviceListIF(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t *infoResp = &wfaDutAgentData.gGenericResp;
	dutCommand_t *ifList = (dutCommand_t *)caCmdBuf;
	caDeviceListIFResp_t *ifListResp = &infoResp->cmdru.ifList;

	DPRINT_INFOL(WFA_OUT, "Entering wfaDeviceListIF...\n");
	switch(ifList->cmdsu.iftype)
	{
	case IF_80211:
		infoResp->status = STATUS_COMPLETE;
		ifListResp->iftype = IF_80211; 
		strcpy(ifListResp->ifs[0], wfaDutAgentData.intfname);
		strcpy(ifListResp->ifs[1], "NULL");
		strcpy(ifListResp->ifs[2], "NULL");
		break;

	case IF_ETH:
		infoResp->status = STATUS_COMPLETE;
		ifListResp->iftype = IF_ETH; 
		strcpy(ifListResp->ifs[0], "eth0");
		strcpy(ifListResp->ifs[1], "NULL");
		strcpy(ifListResp->ifs[2], "NULL");
		break;

	default:      
		infoResp->status = STATUS_ERROR; 
	}

	wfaEncodeTLV(WFA_DEVICE_LIST_IF_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

int wfaStaDebugSet(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t *debugResp = &wfaDutAgentData.gGenericResp;
	dutCommand_t *debugSet = (dutCommand_t *)caCmdBuf;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaDebugSet ...\n");

	if(debugSet->cmdsu.dbg.state == 1) /* enable */
		wfa_defined_debug |= debugSet->cmdsu.dbg.level;
	else
		wfa_defined_debug = (~debugSet->cmdsu.dbg.level & wfa_defined_debug);

	debugResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_GET_INFO_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)debugResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
*   wfaStaGetBSSID():
*     This function is to retrieve BSSID of a specific wireless I/F.
*/ 
int wfaStaGetBSSID(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	char string[64],Interfacename[64];
	FILE *tmpfd;
	dutCmdResponse_t *bssidResp = &wfaDutAgentData.gGenericResp;
	dutCommand_t *getbssid= (dutCommand_t *)caCmdBuf;
	char *ifname = getbssid->intf;
	char bfile[128], intfile[128], tfile[128];

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaGetBSSID...\n");
	/* retrieve the BSSID */

	sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
	sprintf(bfile, "%s\\Temp\\bssid.txt", wtsPath);
	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s", bfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);
	if(wfaDutAgentCAPIData.geSupplicant == eWindowsZeroConfig)
	{
		tmpfd = fopen(intfile, "r");
		if(tmpfd == NULL)
		{
			DPRINT_INFOL(WFA_OUT, "\n Error opening the interface file \n");
		}
		else
		{
			for(;;)
			{
				if(fgets(string, 256, tmpfd) == NULL)
					break; 
			}
			fclose(tmpfd);

			if(strncmp(string, "IFNAME", 6) == 0)
			{
				char *str;
				str = strtok(string, "\"");
				str = strtok(NULL, "\"");
				if(str != NULL)
				{
					strcpy(&Interfacename[0],str);
				}
			}
		}

		sprintf(tfile, "%s\\Temp\\temp.txt", wtsPath);
		sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s",tfile);
		sprintf(wfaDutAgentData.gCmdStr, "netsh wlan show  interface name=\"%s\" > %s",&Interfacename[0], tfile);
		DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
		system(wfaDutAgentData.gCmdStr);

		sprintf(wfaDutAgentData.gCmdStr, "FOR /F \"tokens=2,3,4,5,6,7 delims=:\" %s IN ('findstr BSSID %s') DO @echo %s:%s:%s:%s:%s:%s > %s","%i",tfile, "%i","%j","%k","%l","%m","%n", bfile);
		DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
		system(wfaDutAgentData.gCmdStr);
	}

	system(wfaDutAgentData.gCmdStr);

	tmpfd = fopen(bfile, "r+");
	if(tmpfd == NULL)
	{
		bssidResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_GET_BSSID_RESP_TLV, 4, (BYTE *)bssidResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;

		DPRINT_ERR(WFA_ERR, "File open failed\n");
		return WFA_FAILURE;
	}

	if(wfaDutAgentCAPIData.geSupplicant == eWindowsZeroConfig)
	{
		if(fscanf(tmpfd, "%s", string) == EOF)
		{
			bssidResp->status = STATUS_COMPLETE; 
			strcpy(bssidResp->cmdru.bssid, "00:00:00:00:00:00");
		}
		else
		{
			strcpy(bssidResp->cmdru.bssid, string);
			bssidResp->status = STATUS_COMPLETE;
		}
	}  

	wfaEncodeTLV(WFA_STA_GET_BSSID_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)bssidResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	fclose(tmpfd);

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

/*
* wfaStaSetIBSS()
*    This is to set
*    1. ssid
*    2. channel
*    3. encrypType - none or wep
*    optional
*    4. key1
*    5. key2
*    6. key3
*    7. key4
*    8. activeIndex - 1, 2, 3, or 4
*/
int wfaStaSetIBSS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetIBSS_t *setIBSS = (caStaSetIBSS_t *)caCmdBuf;
	dutCmdResponse_t *setIbssResp = &wfaDutAgentData.gGenericResp;

	setIbssResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_IBSS_RESP_TLV, 4, (BYTE *)setIbssResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}

/*
*  wfaSetMode():
*  The function is to set the wireless interface with a given mode (possible 
*  adhoc)
*  Input parameters:
*    1. I/F
*    2. ssid
*    3. mode adhoc or managed
*    4. encType
*    5. channel
*    6. key(s)
*    7. active  key
*/ 
int wfaStaSetMode(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetMode_t *setmode = (caStaSetMode_t *)caCmdBuf;
	dutCmdResponse_t *SetModeResp = &wfaDutAgentData.gGenericResp;

	SetModeResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_MODE_RESP_TLV, 4, (BYTE *)SetModeResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}

int wfaStaSetPwrSave(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetPwrSave_t *setps = (caStaSetPwrSave_t *)caCmdBuf;
	dutCmdResponse_t *SetPSResp = &wfaDutAgentData.gGenericResp;
	int st = 2;
	strncpy(wfaDutAgentData.gnetIf, setps->intf, WFA_BUFF_32 -1); /*  save for future use */
	switch(wfaDutAgentData.vend)
	{
	case WMM_ATHEROS:      
		break;
	case WMM_BROADCOMM:
		break;
	case WMM_MARVELL:
		break;
	case WMM_RALINK:
		break;
	case WMM_STAUT:
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_pwrsave /interface staut /mode %s",  wfaDutAgentData.WFA_CLI_CMD_DIR, /*WFA_CLI_CMD_DIR,*/  setps->mode);

		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		DPRINT_INFOL(WFA_OUT,"\nwfaStaSetPwrSave:WMM_STAUT WMMPS:%s status=%d\n",wfaDutAgentData.gCmdStr, st);
		break;
	default:
		DPRINT_ERR(WFA_ERR, "wfaStaSetPwrSave::Unknown Vendor, vendorIdex=%d\n", wfaDutAgentData.vend);
		goto end_cleanup;
	}
end_cleanup:    
	switch(st)
	{
	case 0:
		SetPSResp->status = STATUS_COMPLETE;
		break;
	case 1:
		SetPSResp->status = STATUS_ERROR;
		break;
	case 2:
		SetPSResp->status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_SET_PWRSAVE_RESP_TLV, 4, (BYTE *)SetPSResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}

int wfaStaSetPowerSave(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{/* This is legacy power save mode setup related to capi: STA_SET_POWER_SAVE  */
	caStaSetPowerSave_t *setps = (caStaSetPowerSave_t *)caCmdBuf;
	dutCmdResponse_t *SetPSResp = &wfaDutAgentData.gGenericResp;
	int st = 2;

	DPRINT_INFOL(WFA_OUT, "\nwfaStaSetPowerSave::LegacyPS:mode=%s\n",setps->powersave);
	strncpy(wfaDutAgentData.gnetIf, setps->intf, WFA_BUFF_32 -1); /*  save for future use */
	switch(wfaDutAgentData.vend)
	{
	case WMM_ATHEROS:      
		break;
	case WMM_BROADCOMM:
		break;
	case WMM_INTEL:
		break;
	case WMM_MARVELL:
		break;
	case WMM_RALINK:
		break;
	case WMM_STAUT:/* if not need to be in test bed for ch4 test, no need this one  */
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_power_save /interface staut /powersave %s",  wfaDutAgentData.WFA_CLI_CMD_DIR, /*WFA_CLI_CMD_DIR,*/  setps->powersave);

		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		DPRINT_INFOL(WFA_OUT,"\nwfaStaSetPowerSave:WMM_STAUT Legacy PS:%s status=%d\n",wfaDutAgentData.gCmdStr, st);
		break;
	default:
		DPRINT_ERR(WFA_ERR, "wfaStaSetPowerSave, legacyPS::Unknown Vendor, vendorIdex=%d\n", wfaDutAgentData.vend);
		goto end_cleanup;
	}
end_cleanup:    
	switch(st)
	{
	case 0:
		SetPSResp->status = STATUS_COMPLETE;
		break;
	case 1:
		SetPSResp->status = STATUS_ERROR;
		break;
	case 2:
		SetPSResp->status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_SET_POWER_SAVE_RESP_TLV, 4, (BYTE *)SetPSResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}


int wfaStaUpload(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaUpload_t *upload = &((dutCommand_t *)caCmdBuf)->cmdsu.upload;
	dutCmdResponse_t *upLoadResp = &wfaDutAgentData.gGenericResp;

#ifdef WFA_WMM_WPA2
	caStaUploadResp_t *upld = &upLoadResp->cmdru.uld;
#endif

	if(upload->type == WFA_UPLOAD_VHSO_RPT)
	{
#ifdef WFA_VOICE_EXT
		int rbytes;
		/*
		* if asked for the first packet, always to open the file
		*/
		if(upload->next == 1)
		{
			if(e2efp != NULL)
			{
				fclose(e2efp);
				e2efp = NULL;
			}

			e2efp = fopen(e2eResults, "r");
		}

		if(e2efp == NULL)
		{
			upLoadResp->status = STATUS_ERROR;
			wfaEncodeTLV(WFA_STA_UPLOAD_RESP_TLV, 4, (BYTE *)upLoadResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + 4;
			return WFA_FAILURE;
		}

		rbytes = fread(upld->bytes, 1, 256, e2efp); 

		if(rbytes < 256)  
		{
			/* 
			* this means no more bytes after this read
			*/
			upld->seqnum = 0;
		}
		else
		{ 
			upld->seqnum = upload->next;
		}
		fclose(e2efp);
		e2efp=NULL;

		upld->nbytes = rbytes;
		upLoadResp->status = STATUS_COMPLETE;
#else
		DPRINT_INFOL(WFA_OUT, "The command not supported\n");
		upLoadResp->status = STATUS_ERROR;
#endif
		wfaEncodeTLV(WFA_STA_UPLOAD_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)upLoadResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
	}
	else
	{
		upLoadResp->status = STATUS_ERROR;
		wfaEncodeTLV(WFA_STA_UPLOAD_RESP_TLV, 4, (BYTE *)upLoadResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + 4;
	}

	return WFA_SUCCESS;
}

/*
* wfaStaSetWMM()
*  TO be ported on a specific plaform for the DUT
*  This is to set the WMM related parameters at the DUT.
*  Currently the function is used for GROUPS WMM-AC and WMM general configuration for setting RTS Threshhold, Fragmentation threshold and wmm (ON/OFF)
*  It is expected that this function will set all the WMM related parametrs for a particular GROUP .
*/
int wfaStaSetWMM(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetWMM_t *setwmm = (caStaSetWMM_t *)caCmdBuf;
	char *ifname = setwmm->intf;
	dutCmdResponse_t *setwmmResp = &wfaDutAgentData.gGenericResp;

	switch(setwmm->group)
	{
#ifdef WFA_WMM_AC
	case GROUP_WMMAC:
		if(setwmm->action == WMMAC_ADDTS)
		{
			//wmmacadd_t *addts= &(setwmm->actions.addts);
			/* This needs to be ported to the specific platform with these TSPEC details*/		         
			DPRINT_INFOL(WFA_OUT, "ADDTS AC PARAMS: dialog id: %d, TID: %d, DIRECTION: %d, PSB: %d, UP: %d,\
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

			sprintf(wfaDutAgentData.gCmdStr, "iwpriv %s acm %d 1 1",ifname,setwmm->actions.addts.accesscat);
			DPRINT_INFOL(WFA_OUT, "cmd is  %s\n",wfaDutAgentData.gCmdStr);
			system(wfaDutAgentData.gCmdStr);
			if(setwmm->send_trig)
			{
				int Sockfd;
				struct sockaddr_in psToAddr;
				int TxMsg[512];
				Sockfd = wfaCreateUDPSock(setwmm->dipaddr, 12346);
				memset(&psToAddr, 0, sizeof(psToAddr));
				psToAddr.sin_family = AF_INET;
				psToAddr.sin_addr.s_addr = inet_addr(setwmm->dipaddr);
				psToAddr.sin_port = htons(12346);
				usleep(becon_int);
				wfaTGSetPrio(Sockfd, setwmm->trig_ac);
				DPRINT_INFOL(WFA_OUT, "\r\nSending dummy Hello to %s\n",setwmm->dipaddr);
				create_apts_msg(APTS_HELLO, TxMsg,0);
				sendto(Sockfd, TxMsg, 256, 0, (struct sockaddr *)&psToAddr, sizeof(struct sockaddr));
				close(Sockfd);
			}
			//system(wfaDutAgentData.gCmdStr);
		}
		else
			DPRINT_INFOL(WFA_OUT, "DELTS AC PARAMS: TID: %d\n",setwmm->actions.delts);

		setwmmResp->status = STATUS_COMPLETE;
		break;
#endif
	case GROUP_WMMCONF:
		switch(wfaDutAgentData.vend)
		{
		case WMM_BROADCOMM: 
			break;

		case WMM_INTEL: 
			break;

		case WMM_ATHEROS: 
			break;
		default:
			DPRINT_INFOL(WFA_OUT, "Unknown station %d\n",wfaDutAgentData.vend);
		}  /* switch (vend) */

		setwmmResp->status = STATUS_COMPLETE;
		break; /* case GROUP_WMMCONF */

	default:
		DPRINT_ERR(WFA_ERR, "The group %d is not supported\n",setwmm->group);
		setwmmResp->status = STATUS_ERROR; 
	}

	wfaEncodeTLV(WFA_STA_SET_WMM_RESP_TLV, 4, (BYTE *)setwmmResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;
	return WFA_SUCCESS;
}


int wfaStaPresetParameters(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaPresetParameters_t *presetParams = (caStaPresetParameters_t *)caCmdBuf;
	dutCmdResponse_t *PresetParamsResp = &wfaDutAgentData.gGenericResp;
	char *intfname = presetParams->intf;
	BYTE presetDone=0;
	int st = 0;

	DPRINT_INFOL(WFA_OUT, "Inside wfaStaPresetParameters function ...\n");
	DPRINT_INFOL(WFA_OUT,"\nCurrent Supplicant value: %d\n",wfaDutAgentCAPIData.geSupplicant);
	DPRINT_INFOL(WFA_OUT,"\nReceived Supplicant value: %d\n",presetParams->supplicant);

	// For Win 7 - DUT the supplicant is always ZeroConfig and preset =1
	if (presetParams->supplicant)
	{
		presetDone = 1;
	}

#ifndef TGN_TB_STATION
	if(presetParams->modeFlag)
	{
		if( vend == WMM_INTEL )
			;
		else
			DPRINT_INFOL(WFA_OUT, "Not an Intel Sta for setting wirelessmode");
	}
#else
	if(presetParams->reset != 0)
	{
		switch(presetParams->reset)
		{
		case eResetProg11n:
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & reset_default /interface %s /set 11n", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);	        
			break;
		default:
			DPRINT_INFOL(WFA_OUT, "The program not supported.\n");
		}

		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		switch(st)
		{
		case 0:
			presetDone = 1;
			break;
		case 1:
			presetDone = 0;
			break;
		case 2:
			presetDone = 0;
			break;
		}
	}
	if(presetParams->fragFlag == 1)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & SET_FRAG /interface %s /value %d", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname,presetParams->fragThreshold);
		DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		switch(st)
		{
		case 0:
			presetDone = 1;
			break;
		case 1:
			presetDone = 0;
			break;
		case 2:
			presetDone = 0;
			break;
		}
	}

	if(presetParams->rtsFlag == 1)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & SET_RTS /interface %s /value %d", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname,presetParams->rtsThreshold);
		DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);

		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		switch(st)
		{
		case 0:
			presetDone = 1;
			break;
		case 1:
			presetDone = 0;
			break;
		case 2:
			presetDone = 0;
			break;
		}
	}

	if(presetParams->preambleFlag == 1)
	{
		if(presetParams->preamble == eLong)
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & STA_SET_PREAMBLE /interface %s /value long", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
		else
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & STA_SET_PREAMBLE /interface %s /value short", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);

		DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		switch(st)
		{
		case 0:
			presetDone = 1;
			break;
		case 1:
			presetDone = 0;
			break;
		case 2:
			presetDone = 0;
			break;
		}
	}

	if(presetParams->wmmFlag)
	{
		if(presetParams->wmmState == 1)
		{
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & STA_SET_WMM /interface %s /action on", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
			DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
		}
		else
		{
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & STA_SET_WMM /interface %s /action off", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
			DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
		}

		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		switch(st)
		{
		case 0:
			presetDone = 1;
			break;
		case 1:
			presetDone = 0;
			break;
		case 2:
			presetDone = 0;
			break;
		}
	}

	if(presetParams->modeFlag != 0)
	{
		switch(presetParams->wirelessMode)
		{
		case eModeB:
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_mode /interface %s /mode BO",wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
			DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
			break;
		case eModeBG:
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_mode /interface %s /mode BG", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
			DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
			break;
		case eModeA:
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_mode /interface %s /mode AO", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
			DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
			break;
		case eModeAN:
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_mode /interface %s /mode NA", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
			DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
			break;
		case eModeGN:
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_mode /interface %s /mode NG", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
			DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
			break;
		case eModeNL:
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_mode /interface %s /mode NL", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
			DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);
			break;
		default:
			DPRINT_INFOL(WFA_OUT, "other mode does not need to support\n");
		}

		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		switch(st)
		{
		case 0:
			presetDone = 1;
			break;
		case 1:
			presetDone = 0;
			break;
		case 2:
			presetDone = 0;
			break;
		}
	}
#endif

#ifndef TGN_TB_STATION	
	if(presetParams->psFlag)
	{
		if( vend == WMM_INTEL )
			;//setIntelPowerSave(presetParams->legacyPowerSave  );
		else if(vend == WMM_BROADCOMM)
		{
			if(presetParams->legacyPowerSave)
			{
			}
			else
			{
			}
		}
		else
			DPRINT_INFOL(WFA_OUT, "Not an Intel /Broadcom Sta for setting powersave ");
	}
#else
	if(presetParams->psFlag)
	{
		if(presetParams->legacyPowerSave == 1)
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_power_save /interface %s /powersave pspoll", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
		else if(presetParams->legacyPowerSave == 2)
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_power_save /interface %s /powersave fast", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
		else if(presetParams->legacyPowerSave == 3)
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_power_save /interface %s /powersave psnonpoll ", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);
		else
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_power_save /interface %s /powersave off", wfaDutAgentData.WFA_CLI_CMD_DIR, wfaDutAgentData.intfname);

		DPRINT_INFOL(WFA_OUT, "%s\n", wfaDutAgentData.gCmdStr);        
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);

		switch(st)
		{
		case 0:
			presetDone = 1;
			break;
		case 1:
			presetDone = 0;
			PresetParamsResp->status = STATUS_ERROR;
			break;
		case 2:
			presetDone = 0;
			PresetParamsResp->status = STATUS_INVALID;
			break;
		}
	}
#endif

	if(presetParams->noack_be != 0 || presetParams->noack_bk != 0 || presetParams->noack_vi != 0 || presetParams->noack_vo != 0)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_noack /interface %s /mode %x %x %x %x",wfaDutAgentData.WFA_CLI_CMD_DIR,wfaDutAgentData.intfname,presetParams->noack_be-1,presetParams->noack_bk-1,presetParams->noack_vi-1,presetParams->noack_vo-1);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
	}

	if (presetDone)
	{
		PresetParamsResp->status = STATUS_COMPLETE;
		wfaEncodeTLV(WFA_STA_PRESET_PARAMETERS_RESP_TLV, 4, (BYTE *)PresetParamsResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;
	}
	else
	{
		PresetParamsResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_PRESET_PARAMETERS_RESP_TLV, 4, (BYTE *)PresetParamsResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;
	}

	return WFA_SUCCESS;
}


int wfaStaSetEapFAST(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEapFAST_t *setFAST= (caStaSetEapFAST_t *)caCmdBuf;
	char *ifname = setFAST->intf;
	dutCmdResponse_t *setEapFastResp = &wfaDutAgentData.gGenericResp;

	switch(wfaDutAgentCAPIData.geSupplicant)
	{
	case eWindowsZeroConfig:
		setEapFastResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_EAPFAST_RESP_TLV, 4, (BYTE *)setEapFastResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;			
		break;

	default:
		setEapFastResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_EAPFAST_RESP_TLV, 4, (BYTE *)setEapFastResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;	
		break;
	}

	return WFA_SUCCESS;
}

int wfaStaSetEapAKA(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetEapAKA_t *setAKA= (caStaSetEapAKA_t *)caCmdBuf;
	char *ifname = setAKA->intf;
	dutCmdResponse_t *setEapAkaResp = &wfaDutAgentData.gGenericResp;

	switch(wfaDutAgentCAPIData.geSupplicant)
	{
	case eWindowsZeroConfig:
		setEapAkaResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_EAPAKA_RESP_TLV, 4, (BYTE *)setEapAkaResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;			
		break;

	default:
		setEapAkaResp->status = STATUS_INVALID;
		wfaEncodeTLV(WFA_STA_SET_EAPAKA_RESP_TLV, 4, (BYTE *)setEapAkaResp, respBuf);   
		*respLen = WFA_TLV_HDR_LEN + 4;	
		break;
	}

	return WFA_SUCCESS;
}


int wfaStaSetSystime(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetSystime_t *systime = (caStaSetSystime_t *)caCmdBuf;
	dutCmdResponse_t *setSystimeResp = &wfaDutAgentData.gGenericResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaSetSystime ...\n"); 

	sprintf(wfaDutAgentData.gCmdStr, "date %d-%d-%d",systime->month,systime->date,systime->year);
	system(wfaDutAgentData.gCmdStr);

	sprintf(wfaDutAgentData.gCmdStr, "time %d:%d:%d", systime->hours,systime->minutes,systime->seconds);
	system(wfaDutAgentData.gCmdStr);

	setSystimeResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_SYSTIME_RESP_TLV, 4, (BYTE *)setSystimeResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}


void RefreshTaskbarNotificationArea()
{
	HWND hNotificationArea;
	RECT r;
	LONG x ;
	LONG y ;

	GetClientRect(hNotificationArea = FindWindowEx(FW(FW(FW(NULL, L"Shell_TrayWnd"), L"TrayNotifyWnd"), L"SysPager"),NULL,L"ToolbarWindow32",L"Notification Area"),&r);    
	for ( x = 0; x < r.right; x += 5)
		for ( y = 0; y < r.bottom; y += 5)
			SendMessage(
			hNotificationArea,
			WM_MOUSEMOVE,
			0,
			(y << 16) + x);
} 

int wfaStaSet11n(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf) 
{

	caSta11n_t * v11nParams = (caSta11n_t *)caCmdBuf;
	dutCmdResponse_t *v11nParamsResp = &wfaDutAgentData.gGenericResp;

	char *intf = wfaDutAgentData.intfname;

	int st=0, status = STATUS_ERROR;

	DPRINT_INFOL(WFA_OUT, "START - wfaStaSet11n at interface %s\n", intf); 
	//TODO

	if(v11nParams->addba_reject != 0xFF && v11nParams->addba_reject < 2)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_addba_reject /interface %s /action %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, capstr[v11nParams->addba_reject]);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_addba_reject failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}
	}

	if(v11nParams->ampdu != 0xFF && v11nParams->ampdu < 2)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_ampdu /interface %s /action %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, capstr[v11nParams->ampdu]);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_ampdu failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}
	}

	if(v11nParams->amsdu != 0xFF && v11nParams->amsdu < 2)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_amsdu /interface %s /action %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, capstr[v11nParams->amsdu]);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_amsdu failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}
	}

	if(v11nParams->greenfield != 0xFF && v11nParams->greenfield < 2)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_greenfield /interface %s /action %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, capstr[v11nParams->greenfield]);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "_set_greenfield failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}
	}

	if(v11nParams->mcs32!= 0xFF && v11nParams->mcs32 < 2 && v11nParams->mcs_fixedrate[0] != '\0')
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_mcs /interface %s /fixedRate %s /mcs32 %s /BW %d", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, v11nParams->mcs_fixedrate, capstr[v11nParams->mcs32], wfaDutAgentCAPIData.g11nChnlWidth);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_mcs failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}
	} 
	else if (v11nParams->mcs32!= 0xFF && v11nParams->mcs32 < 2 && v11nParams->mcs_fixedrate[0] == '\0')
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_mcs /interface %s  /fixedRate null /mcs32 %s /BW %d", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, capstr[v11nParams->mcs32],wfaDutAgentCAPIData.g11nChnlWidth);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_mcs32 failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}

	} 
	else if (v11nParams->mcs32 == 0xFF && v11nParams->mcs_fixedrate[0] != '\0')
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_mcs /interface %s /fixedRate %s /mcs32 disable /BW %d", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, v11nParams->mcs_fixedrate,wfaDutAgentCAPIData.g11nChnlWidth);
	}

	if(v11nParams->rifs_test != 0xFF && v11nParams->rifs_test < 2)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_rifs_test /interface %s /action %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, capstr[v11nParams->rifs_test]);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_rifs_test failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}
	}

	if(v11nParams->sgi20 != 0xFF && v11nParams->sgi20 < 2)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_sgi20 /interface %s /action %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, capstr[v11nParams->sgi20]);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_sgi20 failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}
	}

	if(v11nParams->smps != 0xFFFF)
	{
		if(v11nParams->smps == 0)
		{
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_smps /interface %s /mode dynamic",wfaDutAgentData.WFA_CLI_CMD_DIR, intf);
		}
		else if(v11nParams->smps == 1)
		{
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_smps /interface %s /mode static", wfaDutAgentData.WFA_CLI_CMD_DIR, intf);
		}
		else if(v11nParams->smps == 2)
		{
			sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_smps /interface %s /mode nolimit", wfaDutAgentData.WFA_CLI_CMD_DIR, intf);
		}
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_smps failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}
	}

	if(v11nParams->stbc_rx != 0xFFFF)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_stbc_rx /interface %s /streams %i", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, v11nParams->stbc_rx);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_stbc_rx failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return FALSE;
		}
	}

	if(v11nParams->width[0] != '\0')
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_11n_channel_width /interface %s /width %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, v11nParams->width);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		wfaDutAgentCAPIData.g11nChnlWidth = atoi(v11nParams->width);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_11n_channel_width failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}

	}

	if(v11nParams->_40_intolerant != 0xFF && v11nParams->_40_intolerant < 2)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_40_intolerant /interface %s /action %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, capstr[v11nParams->_40_intolerant]);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		DPRINT_INFOL(WFA_OUT, "st %i\n", st);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_40_intolerant failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}

	}

	if(v11nParams->txsp_stream != 0 && v11nParams->txsp_stream <4)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_txsp_stream /interface %s /value %u", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, v11nParams->txsp_stream);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		DPRINT_INFOL(WFA_OUT, "st %i\n", st);
		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_txsp_stream failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}

	}

	if(v11nParams->rxsp_stream != 0 && v11nParams->rxsp_stream < 4)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_rxsp_stream /interface %s /value %u", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, v11nParams->rxsp_stream);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", wfaDutAgentData.gCmdStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);

		if(st != 0)
		{
			v11nParamsResp->status = STATUS_ERROR;
			strcpy(v11nParamsResp->cmdru.info, "set_rxsp_stream failed");
			wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
			return WFA_FAILURE;
		}
	}

	v11nParamsResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, 4, (BYTE *)v11nParamsResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;
	return WFA_SUCCESS;
}

int wfaStaSetWireless(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf){

	caStaSetWireless_t * staWirelessParams = (caStaSetWireless_t *)caCmdBuf;
	dutCmdResponse_t *staWirelessResp = &wfaDutAgentData.gGenericResp;

	char *intf = wfaDutAgentData.intfname;
	int st = 0;

	DPRINT_INFOL(WFA_OUT, "START - wfaStaSetWireless \n");
	if((staWirelessParams->noAck[NOACK_BE] == 0 || staWirelessParams->noAck[NOACK_BE] == 1) && 
		(staWirelessParams->noAck[NOACK_BK] == 0 ||  staWirelessParams->noAck[NOACK_BK] == 1) &&
		(staWirelessParams->noAck[NOACK_VI] == 0 || staWirelessParams->noAck[NOACK_VI] == 1) &&
		(staWirelessParams->noAck[NOACK_VO] == 0 || staWirelessParams->noAck[NOACK_VO] == 0) )
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_noack /interface %s /mode %x %x %x %x", staWirelessParams->noAck[NOACK_BE], staWirelessParams->noAck[NOACK_BK], staWirelessParams->noAck[NOACK_VI], staWirelessParams->noAck[NOACK_VO]);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		if(st != 0)
		{
			staWirelessResp->status = STATUS_ERROR;
			strcpy(staWirelessResp->cmdru.info, "set_noack failed");
		}
	}

	staWirelessResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_SET_WIRELESS_RESP_TLV, 4, (BYTE *)staWirelessResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}

int wfaStaSendADDBA(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetSendADDBA_t *staSendADDBA = (caStaSetSendADDBA_t *)caCmdBuf;
	dutCmdResponse_t *staSendADDBAResp = &wfaDutAgentData.gGenericResp;

	char *intf = wfaDutAgentData.intfname;
	int st;

	DPRINT_INFOL(WFA_OUT, "START - wfaStaSendADDBA \n"); 

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & send_addba /interface %s /tid %i", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, staSendADDBA->tid);
	st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);

	switch(st)
	{
	case 0:
		staSendADDBAResp->status = STATUS_COMPLETE;
		break;
	case 1:
		staSendADDBAResp->status = STATUS_ERROR;
		break;
	case 2:
		staSendADDBAResp->status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_SET_SEND_ADDBA_RESP_TLV, 4, (BYTE *)staSendADDBAResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}

int wfaStaSetRIFS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSetRIFS_t *setRIFS = (caStaSetRIFS_t *)caCmdBuf;

	char *intf = wfaDutAgentData.intfname;

	dutCmdResponse_t *staSetRIFSResp = &wfaDutAgentData.gGenericResp;
	int st;

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & set_rifs_test /interface %s /action %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, capstr[setRIFS->action]);
	st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);

	switch(st)
	{
	case 0:
		staSetRIFSResp->status = STATUS_COMPLETE;
		break;
	case 1:
		staSetRIFSResp->status = STATUS_ERROR;
		break;
	case 2:
		staSetRIFSResp->status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_SET_RIFS_TEST_RESP_TLV, 4, (BYTE *)staSetRIFSResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}

int wfaStaSendCoExistMGMT(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaSendCoExistMGMT_t *sendMGMT = (caStaSendCoExistMGMT_t *)caCmdBuf;

	char *intf = wfaDutAgentData.intfname;
	dutCmdResponse_t *staSendMGMTResp = &wfaDutAgentData.gGenericResp;
	int st = 0;

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & send_coexist_mgmt /interface %s /type %s /value %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, sendMGMT->type, sendMGMT->value);
	st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);

	switch(st)
	{
	case 0:
		staSendMGMTResp->status = STATUS_COMPLETE;
		break;
	case 1:
		staSendMGMTResp->status = STATUS_ERROR;
		break;
	case 2:
		staSendMGMTResp->status = STATUS_INVALID;
		break;
	}
	wfaEncodeTLV(WFA_STA_SEND_COEXIST_MGMT_RESP_TLV, 4, (BYTE *)staSendMGMTResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}


void _setProg(char *progname)
{
	if(strcmp(progname, "VHT") ==0)
		wfaDutAgentCAPIData.progSet = eDEF_VHT;
	else if(strcmp(progname, "11n") ==0)
		wfaDutAgentCAPIData.progSet = eDEF_11N;
	else
		wfaDutAgentCAPIData.progSet = 0;
}

int wfaStaResetDefault(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaResetDefault_t *reset = (caStaResetDefault_t *)caCmdBuf;
	char *intf = wfaDutAgentData.intfname;
	char *prog = reset->set;
	dutCmdResponse_t *staResetResp = &wfaDutAgentData.gGenericResp;
	int st;

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaResetDefault ...\n");


	if (wfaDutAgentData.vend == WMM_RALINK)
	{
		staResetResp->status = STATUS_COMPLETE;
		goto end;
	}
	else if (wfaDutAgentData.vend == WMM_BROADCOMM)
	{
		staResetResp->status = STATUS_COMPLETE;
		goto end;
	}
	else if (wfaDutAgentData.vend == WMM_STAUT)
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_reset_default /interface staut /prog %s /type DUT", wfaDutAgentData.WFA_CLI_CMD_DIR, prog);
	}
	else
	{
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & reset_default /interface %s /set %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, prog);
	}
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);

	_setProg(prog);

	switch(st)
	{
	case 0:
		staResetResp->status = STATUS_COMPLETE;
		break;
	case 1:
		staResetResp->status = STATUS_ERROR;
		break;
	case 2:
		staResetResp->status = STATUS_INVALID;
		break;
	}
end:
	wfaEncodeTLV(WFA_STA_RESET_DEFAULT_RESP_TLV, 4, (BYTE *)staResetResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}

int wfaStaDisconnect(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCommand_t *disc = (dutCommand_t *)caCmdBuf;
	char *intf = disc->intf;
	dutCmdResponse_t *staDiscResp = &wfaDutAgentData.gGenericResp;
	FILE *tmpfile = NULL;
	char string[64],Interfacename[64], intfile[128], tfile[128];

	DPRINT_INFOL(WFA_OUT, "Entering wfaStaDisconnect ...\n");

	sprintf(intfile, "%s\\WfaEndpoint\\Interface.txt", wtsPath);
	switch(wfaDutAgentCAPIData.geSupplicant)
	{
	case  eWindowsZeroConfig :
		tmpfile = fopen(intfile, "r");
		if(tmpfile == NULL)
		{
			DPRINT_ERR(WFA_ERR, "\n Error opening the interface file \n");
		}
		else
		{
			for(;;)
			{
				if(fgets(string, 256, tmpfile) == NULL)
					break; 
			}
			fclose(tmpfile);

			if(strncmp(string, "IFNAME", 6) == 0)
			{
				char *str;
				str = strtok(string, "\"");
				str = strtok(NULL, "\"");
				if(str != NULL)
				{
					strcpy(&Interfacename[0],str);
				}
			}
		}
		sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s", tfile);

		sprintf(wfaDutAgentData.gCmdStr, "netsh wlan delete profile name=\"*\"");
		system(wfaDutAgentData.gCmdStr);

		DPRINT_INFOL(WFA_OUT, "Executing %s\n",wfaDutAgentData.gCmdStr);
		break;

	default :
		DPRINT_ERR(WFA_ERR, " \n Unknown Supplicant in Disconnect function");
	}

	staDiscResp->status = STATUS_COMPLETE;
	wfaEncodeTLV(WFA_STA_DISCONNECT_RESP_TLV, 4, (BYTE *)staDiscResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	DPRINT_INFOL(WFA_OUT, "Completing ...\n");
	return WFA_SUCCESS;
}


/* Execute CLI, read the status from Environment variable */
int wfaExecuteCLI(char *CLI)
{
	int st = 0;
	char *retstr;

	system(CLI);

	Sleep(1000);

	retstr = getenv("WFA_CLI_STATUS");
	DPRINT_INFOL(WFA_OUT, "wfaExecuteCLI::CLICmd=%s; cli status %s\n", CLI, retstr);
	return atoi(retstr);
}

int wfaStaReassociate(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	caStaReAssoc_t *reassoc = (caStaReAssoc_t *)caCmdBuf;
	char *intf = wfaDutAgentData.intfname;

	char *bssid = reassoc->bssid;
	dutCmdResponse_t *staReAssocResp = &wfaDutAgentData.gGenericResp;
	int st;

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_send_reassoc /interface %s /bssid %s", wfaDutAgentData.WFA_CLI_CMD_DIR, intf, bssid);
	st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);

	switch(st)
	{
	case 0:
		staReAssocResp->status = STATUS_COMPLETE;
		break;
	case 1:
		staReAssocResp->status = STATUS_ERROR;
		break;
	case 2:
		staReAssocResp->status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_REASSOCIATE_RESP_TLV, 4, (BYTE *)staReAssocResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4;

	return WFA_SUCCESS;
}

int wfaStaCliCommand(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	char cmdName[32];
	char * pcmdStr,*str;
	int st;
	char CmdStr[WFA_CMD_STR_SZ];
	FILE *wfaCliFd=NULL;
	char wfaCliBuff[64];
	char retstr[32], clfile[128];
	int CmdReturnFlag;
	int ret = 0;
	int ckcnt = 5; //fix: moved to here
    int i = 0;
    int paramFlag = 0;
	
	char prog[16];
	char *result;
	char cmdStr1[WFA_CMD_STR_SZ];
	char *arryPtr;	

	caStaCliCmdResp_t infoResp;

	DPRINT_INFOL(WFA_OUT, "\nEntry wfaStaCliCommand... \n");

	DPRINT_INFOL(WFA_OUT, "\nThe command Received: %s\n",caCmdBuf);
	
	if (strstr((char *)caCmdBuf, "sta_reset_default") != NULL)
	{
		strncpy(cmdStr1, (char *)caCmdBuf, WFA_CMD_STR_SZ);
		arryPtr = cmdStr1;
		for(;;)
		{
			result = strtok_r(NULL, ",", &arryPtr);
			if(result == NULL || result[0] == '\0')
				break;

			if(strcasecmp(result, "prog") == 0)
			{
				result = strtok_r(NULL, ",", &arryPtr);
				strncpy(prog, result, 16);
				break;
			}
		}

		_setProg(prog);

		DPRINT_INFOL(WFA_OUT, "progSet value=%d\n", wfaDutAgentCAPIData.progSet);
	}
	
	memcpy(cmdName, strtok_r((char *)caCmdBuf, ",", (char **)&pcmdStr), 32);
	sprintf(CmdStr, "%s",cmdName);

	sprintf(clfile, "%s\\WfaEndpoint\\wfa_cli.txt", wtsPath);

	DPRINT_INFOL(WFA_OUT, "===================cli file %s ==========================\n", clfile);

	for(;;)
	{
		str = strtok_r(NULL, ",", &pcmdStr);
		if(str == NULL || str[0] == '\0')
		{
			DPRINT_INFOL(WFA_OUT, "No more string\n");
			break;
		}
		else
		{
			sprintf(CmdStr, "%s /%s",CmdStr,str);
            if (strstr((char *)str, _strlwr("ssid")) != NULL || strstr((char *)str, _strlwr("passphrase")) != NULL)
            {
                paramFlag = 1;
            }

			str = strtok_r(NULL, ",", &pcmdStr);

            if (paramFlag)
            {
                while(str[i] != '\0' && isalnum(str[i]))
                {
                    i++;
                }
                if (i < (int)strlen(str))
                    sprintf(CmdStr, "%s \"%s\"",CmdStr,str);
                else
                    sprintf(CmdStr, "%s %s",CmdStr,str);

                paramFlag = 0;
            }
            else
            {
			    sprintf(CmdStr, "%s %s",CmdStr,str);
            }
		}
	}

	DPRINT_INFOL(WFA_OUT, "CMDSTR ===========  %s ==================\n", CmdStr);
	// try
	Sleep(3000);

	CmdReturnFlag =0;

	// check the return process
	wfaCliFd=fopen(clfile,"r");
	if(wfaCliFd!= NULL)
	{
		DPRINT_INFOL(WFA_OUT, "Searching CLI Command %s\n", cmdName);
		while(fgets(wfaCliBuff, 64, wfaCliFd) != NULL)
		{
			if(ferror(wfaCliFd))
				break;

			str=strtok(wfaCliBuff,"-");
			if(strcmp(str,cmdName) == 0)
			{
				str=strtok(NULL,",");
				if(strcmp(str,"TRUE") == 0)
				{
					DPRINT_INFOL(WFA_OUT, "Found supported CLI Command %s\n",cmdName); 
					CmdReturnFlag =1;
					break;
				}
				else
				{
					// even with FALSE setting, still excused in the following code
					DPRINT_INFOL(WFA_OUT, "Found supported CLI Command, but set to FALSE: %s\n",cmdName); 
				}
			}

		}
		fclose(wfaCliFd);
	}

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");
	/* should excused with CmdReturnFlag setting, but in fact it is NOT, why, bug?  benz */
	sprintf(wfaDutAgentData.gCmdStr, "cd %s & echo CLI: %s & %s", wfaDutAgentData.WFA_CLI_CMD_DIR, CmdStr, CmdStr);
	ret = system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "\nRUN-> %s system call retVale=%i\n", wfaDutAgentData.gCmdStr, ret);

	Sleep(2000);

	memset(&retstr[0],'\0',32);
	while(retstr[0] =='\0' && ckcnt > 0)
	{
		wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
		ckcnt--;
		Sleep(2000);
	}

	DPRINT_INFOL(WFA_OUT, "\nCLI CmdStr %s retrived WFA_CLI_STATUS=%s\n", CmdStr, retstr);
	if(strlen(retstr) > 0)
		st = atoi(retstr);

	infoResp.resFlag=CmdReturnFlag;

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;
		if (CmdReturnFlag)
		{
			memset(&retstr[0],'\0',32);
			DPRINT_INFOL(WFA_OUT, "cli status beforoe %s**** len%d**** \n", retstr,strlen(retstr));
			wfaGetEnvVal("WFA_CLI_RETURN",&retstr[0],sizeof(retstr));
			DPRINT_INFOL(WFA_OUT, "cli status %s**** len%d**** \n", retstr,strlen(retstr));
			memset(&infoResp.result[0],'\0',WFA_CLI_CMD_RESP_LEN);
			if(retstr != NULL)
			{
				strncpy(&infoResp.result[0], retstr,(strlen(retstr) < WFA_CLI_CMD_RESP_LEN ) ? strlen(retstr) : (WFA_CLI_CMD_RESP_LEN-1) );
				DPRINT_INFOL(WFA_OUT, "Return CLI result to CA: %s****\n", &infoResp.result[0]);			
			}
			else
				strcpy(&infoResp.result[0], "ENV_VAR_NOT_DEFINED");
		}
		break;

	case 1:
		infoResp.status = STATUS_ERROR;
		DPRINT_INFOL(WFA_OUT, "CLI STATUS ERROR\n");
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		DPRINT_INFOL(WFA_OUT, "CLI STATUS INVALID\n");
		break;
	}

	wfaEncodeTLV(WFA_STA_CLI_CMD_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}

int wfaClearEnvVal(char * in_value)
{
	sprintf(wfaDutAgentData.gCmdStr, "reg delete HKEY_CURRENT_USER\\Environment /v %s /f",in_value);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);    

	return WFA_SUCCESS;
}

int wfaGetEnvVal(char * in_value,char * out_value,int size)
{
	FILE *file_hd;
	char *str;
	char tfile[128], envfile[128];

	DPRINT_INFOL(WFA_OUT, "\nEntry wfaGetEnvVal.. ");

	sprintf(tfile, "%s\\Temp\\temp.txt", wtsPath);
	sprintf(envfile, "%s\\Temp\\env_val.txt", wtsPath);
	// get the status
	sprintf(wfaDutAgentData.gCmdStr, "del /F /Q %s && del /F /Q %s", tfile, envfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);

	sprintf(wfaDutAgentData.gCmdStr, "reg query HKEY_CURRENT_USER\\Environment /v %s > %s",in_value, tfile);
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);    

	sprintf(wfaDutAgentData.gCmdStr, "FOR /F \"tokens=3,* delims= \" %s in ('findstr \"%s\" %s') do @echo %s %s > %s","%i",in_value,tfile, "%i","%j", envfile );
	DPRINT_INFOL(WFA_OUT, "RUN-> %s\n", wfaDutAgentData.gCmdStr);
	system(wfaDutAgentData.gCmdStr);    

	// get the status
	file_hd = fopen(envfile,"r");
	if(file_hd != NULL && !ferror(file_hd))
	{
		fgets(wfaDutAgentData.gCmdStr,WFA_CMD_STR_SZ,file_hd);
		str=strtok(wfaDutAgentData.gCmdStr," ");
		strcpy(out_value,str);
		DPRINT_INFOL(WFA_OUT, "In GetEnv token %s*** The return value:%s***\n",str,out_value);
		str=strtok(NULL," ");
		if(strlen(str) >2)
			sprintf(&out_value[strlen(out_value)]," %s",str);

		DPRINT_INFOL(WFA_OUT, "In GetEnv token %s**** The return value:%s***\n",str,out_value);
		fclose(file_hd);
	}
	else
	{
		out_value = NULL;
	}

	DPRINT_INFOL(WFA_OUT, "Completing ... ");
	return WFA_SUCCESS;
}


#ifdef WFA_P2P
/*
* wfaStaGetP2pDevAddress(): 
*/
int wfaStaGetP2pDevAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	dutCommand_t *getInfo = (dutCommand_t *)caCmdBuf;
	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];

	st = 1;

	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaGetP2pDevAddress... ");

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_get_p2p_dev_address /interface %s", WFA_CLI_CMD_DIR, intf);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
	if(retstr !=NULL)
		st = atoi(retstr);


	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;

		wfaGetEnvVal("WFA_CLI_RETURN",&retstr[0],sizeof(retstr));
		DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
		memset(&infoResp.cmdru.devid[0],0,WFA_P2P_DEVID_LEN);
		if(retstr != NULL)
		{
			strncpy(&infoResp.cmdru.devid[0], retstr,(strlen(retstr) < WFA_P2P_DEVID_LEN ) ? strlen(retstr) : (WFA_P2P_DEVID_LEN-1) );
			DPRINT_INFOL(WFA_OUT, "Device ID : %s\n", &infoResp.cmdru.devid[0]);			
		}
		else
			strcpy(&infoResp.cmdru.devid[0], "ENV_VAR_NOT_DEFINED");
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_GET_P2P_DEV_ADDRESS_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}

/*
* wfaStaSetP2p(): 
*/
int wfaStaSetP2p(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	caStaSetP2p_t *getStaSetP2p = (caStaSetP2p_t *)caCmdBuf;

	char *intf = wfaDutAgentData.intfname;

	int st;
	char cmd[512];
	char retstr[128];


	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaSetP2p... ");

	memset(cmd,0,512);

	if(getStaSetP2p->oper_chn_flag == 1)
		sprintf(cmd+strlen(cmd)," /oper_chn %d",getStaSetP2p->oper_chn);
	if(getStaSetP2p->intent_val_flag == 1)
		sprintf(cmd+strlen(cmd)," /intent_val %d",getStaSetP2p->intent_val);
	if(getStaSetP2p->listen_chn_flag == 1)
		sprintf(cmd+strlen(cmd)," /listen_chn %d",getStaSetP2p->listen_chn);
	if(getStaSetP2p->p2p_mode_flag == 1)
		sprintf(cmd+strlen(cmd)," /p2p_mode %s",getStaSetP2p->p2p_mode);
	if(getStaSetP2p->ssid_flag == 1)
		sprintf(cmd+strlen(cmd)," /ssid %s",getStaSetP2p->ssid);
	if(getStaSetP2p->presistent_flag == 1)
		sprintf(cmd+strlen(cmd)," /presistent %d",getStaSetP2p->presistent);
	if(getStaSetP2p->intra_bss_flag == 1)
		sprintf(cmd+strlen(cmd)," /intra_bss %d",getStaSetP2p->intra_bss);
	if(getStaSetP2p->noa_duration_flag == 1)
		sprintf(cmd+strlen(cmd)," /noa_duration %d",getStaSetP2p->noa_duration);
	if(getStaSetP2p->noa_interval_flag == 1)
		sprintf(cmd+strlen(cmd)," /noa_interval %d",getStaSetP2p->noa_interval);
	if(getStaSetP2p->noa_count_flag == 1)
		sprintf(cmd+strlen(cmd)," /noa_count %d",getStaSetP2p->noa_count);

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_p2p /interface %s %s", WFA_CLI_CMD_DIR, intf, cmd);
	st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	// Implement the function and this does not return any thing back.

	wfaEncodeTLV(WFA_STA_SET_P2P_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}
/*
* wfaStaP2pConnect(): 
*/
int wfaStaP2pConnect(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	caStaP2pConnect_t *getStaP2pConnect = (caStaP2pConnect_t *)caCmdBuf;

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];


	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaP2pConnect... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_p2p_connect /interface %s /p2pdevid %s", WFA_CLI_CMD_DIR, intf,getStaP2pConnect->devId);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;

		wfaGetEnvVal("WFA_CLI_RETURN",&retstr[0],sizeof(retstr));
		DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
		memset(&infoResp.cmdru.grpid[0],0,WFA_P2P_GRP_ID_LEN);
		if(retstr != NULL)
		{
			strncpy(&infoResp.cmdru.grpid[0], retstr,(strlen(retstr) < WFA_P2P_GRP_ID_LEN ) ? strlen(retstr) : (WFA_P2P_GRP_ID_LEN-1) );
			DPRINT_INFOL(WFA_OUT, "Device ID : %s\n", &infoResp.cmdru.grpid[0]);			
		}
		else
			strcpy(&infoResp.cmdru.grpid[0], "ENV_VAR_NOT_DEFINED");
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	// Implement the function and this  return the grpid.
	wfaEncodeTLV(WFA_STA_P2P_CONNECT_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}

/*
* wfaStaP2pJoin(): 
*/
int wfaStaP2pJoin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	caStaP2pJoin_t *getStaP2pJoin = (caStaP2pJoin_t *)caCmdBuf;

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];


	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaP2pJoin... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");


	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_p2p_join /interface %s /p2pdevid %s /ssid %s", WFA_CLI_CMD_DIR, intf,getStaP2pJoin->devId,getStaP2pJoin->ssid);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}


	// Implement the function and this does not return any thing back.
	wfaEncodeTLV(WFA_STA_P2P_JOIN_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}


/*
* wfaStaP2pStartGrpFormation(): 
*/
int wfaStaP2pStartGrpFormation(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	caStaP2pStartGrpForm_t *getStaP2pStartGrpForm = (caStaP2pStartGrpForm_t *)caCmdBuf;

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];


	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaP2pStartGrpFormation... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_p2p_start_group_formation /interface %s /p2pdevid %s /intent_val %d", WFA_CLI_CMD_DIR, intf,getStaP2pStartGrpForm->devId,getStaP2pStartGrpForm->intent_val);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;

		wfaGetEnvVal("WFA_CLI_RETURN",&retstr[0],sizeof(retstr));
		DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
		memset(&infoResp.cmdru.grpid[0],0,WFA_P2P_GRP_ID_LEN);
		if(retstr != NULL)
		{
			strncpy(&infoResp.cmdru.p2presult[0], retstr,(strlen(retstr) < WFA_P2P_GRP_ID_LEN ) ? strlen(retstr) : (WFA_P2P_GRP_ID_LEN-1) );
			DPRINT_INFOL(WFA_OUT, "Device ID : %s\n", &infoResp.cmdru.grpid[0]);			
		}
		else
			strcpy(&infoResp.cmdru.grpid[0], "ENV_VAR_NOT_DEFINED");
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_P2P_START_GRP_FORM_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}


/*
* wfaStaP2pDissolve(): 
*/
int wfaStaP2pDissolve(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	caStaP2pDissolve_t *getStap2pDissolve= (caStaP2pDissolve_t *)caCmdBuf;	

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];

	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaP2pDissolve... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");


	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_p2p_dissolve /interface %s /groupid %s", WFA_CLI_CMD_DIR, intf,getStap2pDissolve->grpId);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_P2P_DISSOLVE_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}

/*
* wfaStaSendP2pInvReq(): 
*/
int wfaStaSendP2pInvReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	caStaSendP2pInvReq_t *getStaP2pInvReq= (caStaSendP2pInvReq_t *)caCmdBuf;	

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];

	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaSendP2pInvReq... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	if(getStaP2pInvReq->grpId_flag == 1)
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_send_p2p_invitation_req /interface %s /p2pdevid %s /groupid %s", WFA_CLI_CMD_DIR, intf,getStaP2pInvReq->devId,getStaP2pInvReq->grpId);
	else
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_send_p2p_invitation_req /interface %s /p2pdevid %s", WFA_CLI_CMD_DIR, intf,getStaP2pInvReq->devId);

	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);

	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}


	wfaEncodeTLV(WFA_STA_SEND_P2P_INV_REQ_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}


/*
* wfaStaAcceptP2pReq(): 
*/
int wfaStaAcceptP2pReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	caStaSendP2pInvReq_t *getStaP2pInvReq= (caStaSendP2pInvReq_t *)caCmdBuf;	

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];

	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaAcceptP2pReq... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	if(getStaP2pInvReq->grpId_flag == 1)
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_accept_p2p_invitation_req /interface %s /p2pdevid %s /groupid %s", WFA_CLI_CMD_DIR, intf,getStaP2pInvReq->devId,getStaP2pInvReq->grpId);
	else
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_accept_p2p_invitation_req /interface %s /p2pdevid %s", WFA_CLI_CMD_DIR, intf,getStaP2pInvReq->devId);

	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_ACCEPT_P2P_REQ_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}


/*
* wfaStaSendP2pProvDisReq(): 
*/
int wfaStaSendP2pProvDisReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	caStaSendP2pProvDisReq_t *getStaP2pProvDisReq= (caStaSendP2pProvDisReq_t *)caCmdBuf;

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];

	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaSendP2pProvDisReq... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_send_p2p_provision_dis_req /interface %s /configmethod %s /p2pdevid %s", WFA_CLI_CMD_DIR, intf,getStaP2pProvDisReq->confMethod,getStaP2pProvDisReq->devId);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);

	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_SEND_P2P_PROV_DIS_REQ_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}

/*
* wfaStaSetWpsPbc(): 
*/
int wfaStaSetWpsPbc(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	dutCommand_t *getStaSetWpsPbc= (dutCommand_t *)caCmdBuf;

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];

	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaSetWpsPbc... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_wps_pbc /interface %s", WFA_CLI_CMD_DIR, intf);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);

	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}
	wfaEncodeTLV(WFA_STA_SET_WPS_PBC_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}

/*
* wfaStaWpsReadPin(): 
*/
int wfaStaWpsReadPin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	dutCommand_t *getStaWpsReadPin= (dutCommand_t *)caCmdBuf;	

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[WFA_WPS_PIN_LEN];


	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaWpsReadPin... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_wps_read_pin /interface %s", WFA_CLI_CMD_DIR, intf);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);

	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;

		wfaGetEnvVal("WFA_CLI_RETURN",&retstr[0],sizeof(retstr));
		DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
		memset(&infoResp.cmdru.wpsPin[0],0,WFA_WPS_PIN_LEN);
		if(retstr != NULL)
		{
			strncpy(&infoResp.cmdru.wpsPin[0], retstr,(strlen(retstr) < WFA_WPS_PIN_LEN ) ? strlen(retstr) : (WFA_WPS_PIN_LEN-1) );
			DPRINT_INFOL(WFA_OUT, "Device ID : %s\n", &infoResp.cmdru.wpsPin[0]);			
		}
		else
			strcpy(&infoResp.cmdru.grpid[0], "ENV_VAR_NOT_DEFINED");
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_WPS_READ_PIN_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}


/*
* wfaStaWpsEnterPin(): 
*/
int wfaStaWpsEnterPin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	caStaWpsEnterPin_t *getStaWpsEnterPin= (caStaWpsEnterPin_t *)caCmdBuf;
	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[128];

	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaWpsEnterPin... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_wps_enter_pin /interface %s /pin ", WFA_CLI_CMD_DIR, intf,getStaWpsEnterPin->wpsPin);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);

	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_WPS_ENTER_PIN_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}


/*
* wfaStaGetPsk(): 
*/
int wfaStaGetPsk(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t infoResp;
	dutCommand_t *getStaGetPsk= (dutCommand_t *)caCmdBuf;   	

	char *intf = wfaDutAgentData.intfname;

	int st;
	char retstr[WFA_WPS_PIN_LEN];

	DPRINT_INFOL(WFA_OUT, "\n Entry wfaStaGetPsk... ");

	st = 1;

	wfaClearEnvVal("WFA_CLI_STATUS");
	wfaClearEnvVal("WFA_CLI_RETURN");

	sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_get_psk /interface %s", WFA_CLI_CMD_DIR, intf);
	system(wfaDutAgentData.gCmdStr);
	DPRINT_INFOL(WFA_OUT, "CLI Command %s\n", wfaDutAgentData.gCmdStr);

	wfaGetEnvVal("WFA_CLI_STATUS",&retstr[0],sizeof(retstr));
	DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
	if(retstr !=NULL)
		st = atoi(retstr);

	switch(st)
	{
	case 0:
		infoResp.status = STATUS_COMPLETE;

		wfaGetEnvVal("WFA_CLI_RETURN",&retstr[0],sizeof(retstr));
		DPRINT_INFOL(WFA_OUT, "cli status %s\n", retstr);
		memset(&infoResp.cmdru.pskInfo.passPhrase[0],0,WFA_WPS_PIN_LEN);
		if(retstr != NULL)
		{
			strncpy(&infoResp.cmdru.pskInfo.passPhrase[0], retstr,(strlen(retstr) < WFA_WPS_PIN_LEN ) ? strlen(retstr) : (WFA_WPS_PIN_LEN-1) );
			DPRINT_INFOL(WFA_OUT, "Device ID : %s\n", &infoResp.cmdru.pskInfo.passPhrase[0]);			
		}
		else
			strcpy(&infoResp.cmdru.grpid[0], "ENV_VAR_NOT_DEFINED");
		break;
	case 1:
		infoResp.status = STATUS_ERROR;
		break;
	case 2:
		infoResp.status = STATUS_INVALID;
		break;
	}

	wfaEncodeTLV(WFA_STA_GET_PSK_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);   
	*respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

	return WFA_SUCCESS;
}

#endif
