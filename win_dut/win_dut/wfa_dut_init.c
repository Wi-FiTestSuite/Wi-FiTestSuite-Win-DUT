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
 * @file wfa_dut_init.c
 * @brief File containing the initial configuration of buffers and test environment variables
*/

#include "wfa_dut.h"

#define WFA_WTS_SETTING_FILE_NAME "wts_settings.txt"

/** 
 * Buffer initialization
 * @param tBuf a buffer used to carry receive and send test traffic
 * @param rBuf a buffer used for response of control command
 * @param paBuf parameters buffer
 * @param cBuf control command buffer
 * @param timerp timer used in select call
*/
void wfa_dut_init(BYTE **tBuf, BYTE **rBuf, BYTE **paBuf, BYTE **cBuf, struct timeval **timerp)
{
	int ncnt = 0;
	FILE *tmpfd;
	char string[WFA_BUFF_256];
	char *ch;
	int comprslt = -1;

#ifdef WFA_WMM_WPA2
	/* allocate the traffic stream table */
	for (ncnt = 0; ncnt < WFA_MAX_WMM_STREAMS; ncnt++)
	{
		wfaTGWMMData.tgSockfds[ncnt] = -1;
	}
#endif /* WFA_WMM_WPA2 */

	/* a buffer used to carry receive and send test traffic */
	*tBuf = (BYTE *) malloc(MAX_UDP_LEN+1); /* alloc a traffic buffer */
	if(*tBuf == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Failed to malloc traffic buffer\n");
		WFA_EXIT(1);
	}
	/* a buffer used for response of control command */
	*rBuf = (BYTE *)malloc(WFA_BUFF_4K);
	if(*rBuf == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Failed to malloc response buffer\n");
		WFA_EXIT(1);
	}
	/* timer used in select call */
	*timerp = (struct timeval *) malloc(sizeof(struct timeval)); //fix: using the casting of (struct timeval *)
	if(*timerp == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Failed to malloc timer val\n");
		WFA_EXIT(1);
	}
	/* control command buf */
	*cBuf = (BYTE *) malloc(WFA_BUFF_1K); // be consistent to the use of this buffer with 1k size
	if(*cBuf == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Failed to malloc control command buf\n");
		WFA_EXIT(1);
	}

	/* parameters buff */
	*paBuf = (BYTE *)malloc(MAX_PARMS_BUFF);
	if(*paBuf == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Failed to malloc parms value buff\n");
		WFA_EXIT(1);
	}

#ifdef WFA_VOICE_EXT
	/* allocate for End2End stats */
	e2eStats = (tgE2EStats_t *) malloc(6144*sizeof(tgE2EStats_t));
	if(e2eStats == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Failed to malloc e2eStats\n");
		WFA_EXIT(1);
	}
#endif /* WFA_VOICE_EXT */

	wfaDutAgentCAPIData.geSupplicant = eWindowsZeroConfig;
	DPRINT_INFOL(WFA_OUT,"New Supplicant value: %d\n", wfaDutAgentCAPIData.geSupplicant);
	wfaDutAgentData.vend= WMM_BROADCOMM;

	/*  use wts_settings.txt file to detect which vendor is  */
	tmpfd = fopen(WFA_WTS_SETTING_FILE_NAME, "r+");
	if(tmpfd == NULL)
	{
		DPRINT_ERR(WFA_ERR, "file open failed for %s\n", WFA_WTS_SETTING_FILE_NAME);
		return ;
	}

	memset(string, 0, WFA_BUFF_256);
	memset(string, 0, WFA_BUFF_256);

	while (fgets(string, WFA_BUFF_256 -1, tmpfd) != NULL) 
	{        
		ch = strtok(string, "=");

		comprslt = strncmp(ch, "WFA_CLI_STA_DEVICE", strlen("WFA_CLI_STA_DEVICE"));            
		if (comprslt == 0) 
		{
			ch = strtok(NULL, "=");
			if (ch == NULL)     
				comprslt = -1;
			break;                    
		}        
	}

	if(comprslt != 0)
	{
		DPRINT_INFOL(WFA_OUT, "No vendor device found\n");
		return ;
	}
	fclose(tmpfd);

	if(strstr(ch,"Broadcom") != NULL)
	{
		wfaDutAgentData.vend= WMM_BROADCOMM; /* will be removed later Nov 2012 */        
	}
	else if(strstr(ch,"Intel"))
	{
		wfaDutAgentData.vend= WMM_INTEL;
	}
	else if(strstr(ch,"Marvell"))
	{
		wfaDutAgentData.vend= WMM_MARVELL;        
	}
	else if(strstr(ch,"Atheros"))
	{
		wfaDutAgentData.vend= WMM_ATHEROS;
	}
	else if(strstr(ch,"Ralink"))
	{
		wfaDutAgentData.vend= WMM_RALINK;        
	}
	else if(strstr(ch,"WMM_STAUT"))
	{
		wfaDutAgentData.vend= WMM_STAUT;
	}
	else
	{
		DPRINT_ERR(WFA_ERR, "Unknown vendor\n");  
		wfaDutAgentData.vend = WMM_UNKNOWN_VENDOR;
	}

	DPRINT_INFOL(WFA_OUT, "vendorIdex=%d\n", wfaDutAgentData.vend);
}

/** 
 * Read the settings in wts_setting.txt file
*/
void wfa_set_envs()
{
	char *name, *dest;
	FILE *envfile = NULL;
	char line[129];

	envfile = fopen(WFA_WTS_SETTING_FILE_NAME, "r");
	if(envfile == NULL)
	{
		DPRINT_ERR(WFA_ERR, "can't open setting file, exiting ...\n");
		exit(1);
	}

	memset(wfaDutAgentData.WFA_CLI_CMD_DIR, 0, 64);

	while(fgets(line, 128, envfile) != NULL)
	{
		if(*line == '#' || *line == 'a')
		{
			memset(line, 0, 129);
			continue;
		}

		name = (char *)strtok(line, "=");
		dest = (char *)strtok(NULL, "=");

		if(strcmp(name, "WFA_CLI_STA_DEVICE") == 0)
		{
			dest[strlen(dest)-1] = '\0';
			DPRINT_INFOL(WFA_OUT, "value %s\n", dest);
			strcpy(wfaDutAgentData.WFA_CLI_CMD_DIR, dest);
			DPRINT_INFOL(WFA_OUT, " %s is %s\n", name, wfaDutAgentData.WFA_CLI_CMD_DIR);
		}
		else if(strcmp(name, "DEVICE") == 0)
		{
			dest[strlen(dest)-1] = '\0';
			strcpy(wfaDutAgentData.intfname, dest);
			DPRINT_INFOL(WFA_OUT, "%s is %s\n", name, wfaDutAgentData.intfname);
		}
		else if(strcmp(name, "VERSION") == 0)
		{
			dest[strlen(dest)-1] = '\0';
			strcpy(wfaDutAgentData.WFA_CLI_VERSION, dest);
			DPRINT_INFOL(WFA_OUT, "%s is %s\n", name, wfaDutAgentData.WFA_CLI_VERSION);
		}
		memset(line, 0, 129);
	}
	//reset the env variable status
	_putenv("WFA_CLI_STATUS=0");
}

