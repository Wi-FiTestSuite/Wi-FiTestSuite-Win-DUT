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

#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <io.h>
#include "wfa_ca.h"
#include "wfa_ca_resp.h"

#define WFA_ENV_AGENT_IPADDR "WFA_ENV_AGENT_IPADDR"

extern int xcCmdProcGetVersion(unsigned char *parms);
extern dutCommandRespFuncPtr wfaCmdRespProcFuncTbl[];
extern typeNameStr_t nameStr[];

/*
* The output format can be redefined for file output.
*/
int main(int argc, char *argv[])
{
	int nfds;
	struct sockaddr_in servAddr;
	unsigned short servPort, myport; 
	char *servIP=NULL, *tstr=NULL;
	int bytesRcvd;                   
	fd_set sockSet;
	char cmdName[WFA_BUFF_32];
	int i, isFound = 0, nbytes, tag, ret_status, slen,dut_con_error=0;
	int tmsockfd, cmdLen = WFA_BUFF_1K;
	int maxfdn1;
	BYTE xcCmdBuf[WFA_BUFF_4K];
	BYTE caCmdBuf[WFA_BUFF_4K];
	BYTE pcmdBuf[WFA_BUFF_1K];
	char *pcmdStr;
	char respStr[WFA_BUFF_512];
	int gSock = -1;

	//start of Windows CLI handling variables
	char wfaCliBuff[128];
	FILE *wfaCliFd;
	char * cliCmd,*tempCmdBuff;

	// for setting socket option to tcp_nodelay
	int iResult = 0;
	int iOptVal = 0;
	int iOptLen = sizeof (int);

	WFA_OUT = stdout;
	WFA_ERR = stderr;
	WFA_WNG = stdout;

	DPRINT_INFOL(WFA_OUT, "%c[%d;%d;%dm", 0x1B, 1,32,40);
	if(argc < 3)
	{
		DPRINT_ERR(WFA_ERR, "Usage: %s <control interface> <local control agent port> [<DUT IP> <DUT PORT> [<Logfile Path>]]\n", argv[0]);
		exit(1);
	}

	myport = atoi(argv[2]);

	if(argc > 3)
	{
		if(argc < 5)
		{
			DPRINT_ERR(WFA_ERR, "Usage: %s <control interface> <local control agent port> <DUT IP ADDRESS> <DUT PORT>\n", argv[0]);
			exit(1);
		}
		
		servIP = argv[3];
		if(isIpV4Addr(argv[3])== WFA_FAILURE)
		{
			return FALSE;
		}
		if(isNumber(argv[4])== WFA_FAILURE)
		{
			return FALSE;
		}
		
		servPort = atoi(argv[4]);
		if(argc > 5)
		{
			FILE *logfile;
			int fd;
			logfile = fopen(argv[5],"a");
			if(logfile != NULL)
			{
				WFA_OUT = WFA_ERR = WFA_WNG = logfile;
				//fd = _fileno(logfile);
				DPRINT_INFO(WFA_OUT,"redirecting the output to %s\n",argv[5]);
				//_dup2(fd,1);
				//_dup2(fd,2);
			}
			else
			{
				DPRINT_ERR(WFA_ERR, "Cant open the log file continuing without redirecting\n");
			}
		}
	}
	else
	{
		if((tstr = getenv("WFA_ENV_AGENT_IPADDR")) == NULL)
		{
			DPRINT_ERR(WFA_ERR, "Environment variable WFA_ENV_AGENT_IPADDR not set or specify DUT IP/PORT\n");
			exit(1);
		}
		if(isIpV4Addr(tstr)== WFA_FAILURE)
		{
			return FALSE;
		}
		servIP= tstr; 
		if((tstr = getenv("WFA_ENV_AGENT_PORT")) == NULL)
		{
			DPRINT_ERR(WFA_ERR, "Environment variable WFA_ENV_AGENT_PORT not set or specify DUT IP/PORT\n");
			exit(1);
		}
		if(isNumber(tstr)== WFA_FAILURE)
		{
			return FALSE;
		}
		servPort = atoi(tstr); 
	}

	wfa_defined_debug = WFA_DEBUG_ERR | WFA_DEBUG_WARNING | WFA_DEBUG_INFO;
	wfaCAAgetData.gCaSockfd = -1;
	strcpy(wfaCAAgetData.gnetIf, "any");
	tmsockfd = wfaCreateTCPServSock(myport);
	maxfdn1 = tmsockfd + 1;
	FD_ZERO(&sockSet);

	if(gSock == -1)
	{
		if ((gSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		{
			DPRINT_ERR(WFA_ERR, "socket() failed: %i", errno);
			exit(1);
		}

		memset(&servAddr, 0, sizeof(servAddr)); 
		servAddr.sin_family      = AF_INET;
		servAddr.sin_addr.s_addr = inet_addr(servIP);
		servAddr.sin_port        = htons(servPort);
		if (connect(gSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		{
			DPRINT_ERR(WFA_ERR, "connect() failed: %i, continuing\n", errno);
			dut_con_error = 1;
		}
		else
		{
			DPRINT_INFO(WFA_OUT, "Connected to the DUT/TGEN %s:%d\n",servIP,servPort);
		}
	}

	for(;;)
	{
		fflush(stdout);
		FD_ZERO(&sockSet);
		FD_SET(tmsockfd, &sockSet);
		maxfdn1 = tmsockfd + 1;

		if(wfaCAAgetData.gCaSockfd != -1)
		{
			FD_SET(wfaCAAgetData.gCaSockfd, &sockSet);
			if(maxfdn1 < wfaCAAgetData.gCaSockfd)
			{
				maxfdn1 = wfaCAAgetData.gCaSockfd +1; 
			}
		}
		if(gSock == -1)
		{
			if ((gSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
			{
				DPRINT_ERR(WFA_ERR, "socket() failed: %i", errno);
				exit(1);
			}
			memset(&servAddr, 0, sizeof(servAddr)); 
			servAddr.sin_family      = AF_INET;
			servAddr.sin_addr.s_addr = inet_addr(servIP);
			servAddr.sin_port        = htons(servPort);
			if (connect(gSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
			{
				DPRINT_ERR(WFA_ERR, "connect() failed: %i,continuing\n", errno);
				dut_con_error = 1;
			}
			else
			{
				dut_con_error = 0;
				DPRINT_INFO(WFA_OUT, "Connected to the DUT/TGEN %s:%d\n",servIP,servPort);
			}
		}
		else
		{
			if(dut_con_error)
			{
				DPRINT_INFO(WFA_OUT, "Trying to Connect to %s:%d ...\n",servIP,servPort);
				if (connect(gSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
				{
					DPRINT_ERR(WFA_ERR, "connect() failed: %i,continuing\n", errno);
				}
				else
				{
					dut_con_error = 0;
					DPRINT_INFO(WFA_OUT, "Connected to the DUT/TGEN %s:%d\n",servIP,servPort);
				}
			}
			else
			{
				FD_SET(gSock, &sockSet);
				if(maxfdn1 < gSock)
				{
					maxfdn1 = gSock +1; 
				}
			}
		}

		if((nfds = select(maxfdn1, &sockSet, NULL, NULL, NULL)) < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}
			else
			{
				DPRINT_WARNING(WFA_WNG, "select error %i", errno);
			}
		}

		DPRINT_INFO(WFA_OUT, "new event \n");
		if(FD_ISSET(tmsockfd, &sockSet))
		{
			wfaCAAgetData.gCaSockfd = wfaAcceptTCPConn(tmsockfd);
			DPRINT_INFO(WFA_OUT, "accept new connection\n");
			continue;
		}

		if(wfaCAAgetData.gCaSockfd > 0 && FD_ISSET(wfaCAAgetData.gCaSockfd, &sockSet))
		{
			memset(xcCmdBuf, '\0', WFA_BUFF_4K);
			memset(wfaCAAgetData.gRespStr, '\0', WFA_BUFF_512);

			nbytes = wfaCtrlRecv(wfaCAAgetData.gCaSockfd, xcCmdBuf, WFA_BUFF_4K - 4); 
			DPRINT_INFO(WFA_OUT, "message %s %i\n", xcCmdBuf, nbytes);
			if(nbytes <=0)
			{
				closesocket(wfaCAAgetData.gCaSockfd);
				wfaCAAgetData.gCaSockfd = -1;
				continue;
			}

			/*
			* send back to command line or TM.
			*/
			memset(respStr, '\0', WFA_BUFF_512);
			if(dut_con_error) //Retry Once more for the DUT to come up
			{
				if(gSock != -1)
				{
					if (connect(gSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
					{
						DPRINT_ERR(WFA_ERR, "connect() failed: %i", errno);
					}
					else
					{
						dut_con_error = 0;
						DPRINT_INFO(WFA_OUT, "Connected to the DUT/TGEN %s:%d\n",servIP,servPort);
					}
				}
			}
			if(dut_con_error)
			{
				sprintf(respStr, "status,RUNNING\r\n");
				wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)respStr, strlen(respStr));
				sprintf(respStr, "status,ERROR,DUT_%s:%d_NOT_READY\r\n",servIP,servPort);
				wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)respStr, strlen(respStr));
			}
			else
			{
				sprintf(respStr, "status,RUNNING\r\n");
				wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)respStr, strlen(respStr));
				DPRINT_INFO(WFA_OUT, "%s\n", respStr);

				slen = (int )strlen((char *)xcCmdBuf);
				DPRINT_INFO(WFA_OUT, "last %x last-1  %x last-2 %x last-3 %x\n", cmdName[slen], cmdName[slen-1], cmdName[slen-2], cmdName[slen-3]);

				xcCmdBuf[slen-3] = '\0';
				isFound = 0;
				tempCmdBuff=(char* )malloc(sizeof(xcCmdBuf));
				memcpy(tempCmdBuff,xcCmdBuf,sizeof(xcCmdBuf));

				memcpy(cmdName, strtok_r((char *)tempCmdBuff, ",", (char **)&pcmdStr), 32);
				DPRINT_INFO(WFA_OUT,"\nInside the new block \n");

				wfaCliFd=fopen("c:\\WFA\\WfaEndpoint\\wfa_cli.txt","r");
				DPRINT_INFO(WFA_OUT,"\nAfter File open \n");
				if(wfaCliFd!= NULL)
				{
					while(fgets(wfaCliBuff, 128, wfaCliFd) != NULL)
					{
						if(ferror(wfaCliFd))
						{
							break;
						}
						cliCmd=strtok(wfaCliBuff,"-");
						if(strcmp(cliCmd,cmdName) == 0)
						{
							strcpy(cmdName,"wfa_cli_cmd");
							pcmdStr = (char*)&xcCmdBuf[0]; // fix: casting to char* to remove warning
							break;
						}
					}
					fclose(wfaCliFd);

				}
				DPRINT_INFO(WFA_OUT,"\nOutside the new block \n");
				free(tempCmdBuff);
				if(strcmp(cmdName,"wfa_cli_cmd") != 0)
				{
					memcpy(cmdName, strtok_r((char *)xcCmdBuf, ",", (char **)&pcmdStr), 32);
				}
				i = 0;
				while(nameStr[i].type != -1)
				{
					if(strcmp(nameStr[i].name, cmdName) == 0)
					{
						isFound = 1;
						break;
					}
					i++;
				}
				DPRINT_INFO(WFA_OUT, "Got CMD: %s\n", cmdName);
				if(isFound == 0)
				{
					Sleep(1);
					sprintf(respStr, "status,INVALID\r\n");
					wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)respStr, strlen(respStr));
					DPRINT_WARNING(WFA_WNG, "Command not valid, check the name: %s\n", cmdName);
					continue;
				}

				memset(pcmdBuf, '\0', WFA_BUFF_1K); 
				if(nameStr[i].cmdProcFunc(pcmdStr, pcmdBuf, &cmdLen)==WFA_FAILURE)
				{
					Sleep(1);
					sprintf(respStr, "status,INVALID\r\n");
					wfaCtrlSend(wfaCAAgetData.gCaSockfd, (BYTE *)respStr, strlen(respStr));
					DPRINT_WARNING(WFA_WNG, "Incorrect command syntax!\n");
					continue;
				}
				// for setting the socket option to tcp_nodelay
				iResult = setsockopt(gSock, IPPROTO_TCP, TCP_NODELAY, (char *) &iOptVal, iOptLen);
				if (iResult == SOCKET_ERROR) {
					DPRINT_INFOL(WFA_OUT, "setsockopt for TCP_NODELAY failed with error: %u\n", WSAGetLastError());
				} 
				else
				{
					DPRINT_INFOL(WFA_OUT, "Set TCP_NODELAY: ON\n");
				}
				/*
				* send to DUT.
				*/
				if(send(gSock, (const char*)pcmdBuf, cmdLen, 0) != cmdLen)
				{
					DPRINT_WARNING(WFA_WNG, "Incorrect sending ...\n");
					continue;
				}
				DPRINT_INFO(WFA_OUT, "sent to DUT\n");
			} /* done with wfaCAAgetData.gCaSockfd */
		} // end of for gCaSock

		if(gSock > 0 && FD_ISSET(gSock, &sockSet))
		{
			DPRINT_INFO(WFA_OUT, "received from DUT\n");

			Sleep(1);
			memset(respStr, 0, WFA_BUFF_512);
			memset(caCmdBuf, 0, WFA_BUFF_4K);
			if ((bytesRcvd = recv(gSock, (char*)caCmdBuf, WFA_BUFF_4K, 0)) <= 0)
			{
				DPRINT_WARNING(WFA_WNG, "recv() failed or connection closed prematurely\n");
				FD_CLR(gSock, &sockSet);
				closesocket(gSock);
				gSock=-1;
				continue;
			}
			tag = ((wfaTLV *)caCmdBuf)->tag;     
			memcpy(&ret_status, caCmdBuf+4, 4);
			DPRINT_INFO(WFA_OUT, "bytes=%i, %i,%i,%x %x %x %x \n", bytesRcvd, ((wfaTLV *)caCmdBuf)->tag,((wfaTLV *)caCmdBuf)->len, *(caCmdBuf+4), *(caCmdBuf+5), *(caCmdBuf+6), *(caCmdBuf+7));
			DPRINT_INFO(WFA_OUT, "tag %i \n", tag-WFA_STA_COMMANDS_END);
			// for downward compatibility 
			if((tag != 0 && tag > WFA_STA_NEW_CMD_RESPONSE_START && tag < WFA_STA_NEW_CMD_RESPONSE_END) && wfaCmdRespProcFuncTbl[tag-WFA_STA_NEW_CMD_RESPONSE_START + (WFA_STA_RESPONSE_END - WFA_STA_COMMANDS_END - 1)] != NULL)
			{
				wfaCmdRespProcFuncTbl[tag-WFA_STA_NEW_CMD_RESPONSE_START + (WFA_STA_RESPONSE_END - WFA_STA_COMMANDS_END - 1)](caCmdBuf);
			}
			else if((tag != 0 && tag < WFA_STA_RESPONSE_END) && wfaCmdRespProcFuncTbl[tag-WFA_STA_COMMANDS_END] != NULL)
			{
				wfaCmdRespProcFuncTbl[tag-WFA_STA_COMMANDS_END](caCmdBuf);
			}
			else
			{
				DPRINT_WARNING(WFA_WNG, "wfa_ca main: function not defined\n");            
			}
		} 
	} 
	closesocket(gSock);
	exit(0);
}