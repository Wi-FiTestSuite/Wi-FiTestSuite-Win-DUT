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
 * @file wfa_wmmps.c
 * @brief File containing the various rountines used in wmm-ps testing
*/

#include "wfa_dut.h"

#ifdef WFA_WMM_PS

/* external functions used in this file  */
extern int wfaExecuteCLI(char *CLI);  /* in wfa_cs.c */
extern int wfaTGSetPrio(int sockfd, int tgClass, int streamid);

/* APTS messages*/
struct apts_msg apts_msgs[] ={
	{0, -1},
	{"B.D", B_D},	
	{"B.H", B_H},
	{"B.B", B_B},
	{"B.M", B_M},	
	{"M.D", M_D},	
	{"B.Z", B_Z},	
	{"M.Y", M_Y},	
	{"L.1", L_1},	
	{"A.Y", A_Y},	
	{"B.W", B_W},	
	{"A.J", A_J},	
	{"M.V", M_V},	
	{"M.U", M_U},	
	{"A.U", A_U},	
	{"M.L", M_L},	
	{"B.K", B_K},	
	{"M.B", M_B},	
	{"M.K", M_K},	
	{"M.W", M_W},	
#ifdef WFA_WMM_AC
	{"ATC1", ATC1},	
	{"ATC2", ATC2},	
	{"ATC3", ATC3},	
	{"ATC4", ATC4},	
	{"ATC5", ATC5},	
	{"ATC6", ATC6},	
	{"ATC7", ATC7},	
	{"ATC8", ATC8},	
	{"ATC9", ATC9},	
	{"ATC10", ATC10},	
	{"ATC11", ATC11},	
	{"STC1", STC1},	
	{"STC2", STC2},	
	{"STC3", STC3},	
#endif
	{"APTS TX         ", APTS_DEFAULT },
	{"APTS Hello      ", APTS_HELLO },
	{"APTS Broadcast  ", APTS_BCST },
	{"APTS Confirm    ", APTS_CONFIRM},
	{"APTS STOP       ", APTS_STOP},
	{"APTS CK BE      ", APTS_CK_BE },
	{"APTS CK BK      ", APTS_CK_BK },
	{"APTS CK VI      ", APTS_CK_VI },
	{"APTS CK VO      ", APTS_CK_VO },
	{"APTS RESET      ", APTS_RESET },
	{"APTS RESET RESP ", APTS_RESET_RESP },
	{"APTS RESET STOP ", APTS_RESET_STOP },
	{0, 0 }		// APTS_LAST
};

/* The DUT recv table for each of the test cases*/
StationRecvProcStatetbl_t stationRecvProcStatetbl[LAST_TEST+1][6] = 
{
	/* {WfaRcvStop,0,0,0,0},  original */
	{WfaRcvProc,0,0,0,0,0}, 
	/*B.D*/ {WfaRcvProc,WfaRcvVO,WfaRcvStop,0,0,0},
	/*B.H*/ {WfaRcvProc,WfaRcvVO,WfaRcvVO,WfaRcvStop,0,0},
	/*B.B*/ {WfaRcvProc,WfaRcvStop,0,0,0,0},
	/*B.M*/ {WfaRcvProc,WfaRcvStop,0,0,0,0},
	/*M.D*/ {WfaRcvProc,WfaRcvBE,WfaRcvBK,WfaRcvVI,WfaRcvVO,WfaRcvStop},
	/*B.Z*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvStop,0,0},
	/*M.Y*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvBE,WfaRcvStop,0},
	/*L.1*/ {WfaRcvProc,WfaRcvVOCyclic,0,0,0,0},
	/*A.Y*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvBE,WfaRcvStop,0},
	/*B.W*/ {WfaRcvProc,WfaRcvBE,WfaRcvVI,WfaRcvBE,WfaRcvVI,WfaRcvStop},
	/*A.J*/ {WfaRcvProc,WfaRcvVO,WfaRcvVI,WfaRcvBE,WfaRcvBK,WfaRcvStop},
	/*M.V*/ {WfaRcvProc,WfaRcvBE,WfaRcvVI,WfaRcvStop,0,0},
	/*M.U*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvVO,WfaRcvVO,WfaRcvStop},
	/*A.U*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvVO,WfaRcvStop,0},
	/*M.L*/ {WfaRcvProc,WfaRcvBE,WfaRcvStop,0,0,0},
	/*B.K*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvStop,0,0},
	/*M.B*/ {WfaRcvProc,WfaRcvStop,0,0,0,0},
	/*M.K*/ {WfaRcvProc,WfaRcvBE,WfaRcvVI,WfaRcvStop,0,0},
	/*M.W*/ {WfaRcvProc,WfaRcvBE,WfaRcvBE,WfaRcvBE,WfaRcvVI,WfaRcvStop}
#ifdef WFA_WMM_AC
	/*ATC1*/ ,{WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvStop},
		/*ATC2*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvBE,WfaRcvStop},
		/*ATC3*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvBE,WfaRcvStop},
		/*ATC4*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvVO,WfaRcvVO,WfaRcvStop},
		/*ATC5*/ {WfaRcvProc,WfaRcvVI,WfaRcvVO,WfaRcvStop},
		/*ATC6*/ {WfaRcvProc,WfaRcvVO,WfaRcvVI,WfaRcvVI,WfaRcvStop},
		/*ATC7*/ {WfaRcvProc,WfaRcvVO,WfaRcvVI,WfaRcvBE,WfaRcvBK,WfaRcvStop},
		/*ATC8*/ {WfaRcvProc,WfaRcvVO,WfaRcvVI,WfaRcvStop},
		/*ATC9*/ {WfaRcvProc,WfaRcvVO,WfaRcvVI,WfaRcvBE,WfaRcvBK,WfaRcvStop},
		/*ATC10*/{WfaRcvProc,WfaRcvVO,WfaRcvVI,WfaRcvBE,WfaRcvBK,WfaRcvStop},
		/*ATC11*/{WfaRcvProc,WfaRcvVO,WfaRcvVO,WfaRcvVI,WfaRcvVO,WfaRcvStop},
		/*STC1*/ {WfaRcvProc,WfaRcvVI,WfaRcvBE,WfaRcvStop},
		/*STC2*/ {WfaRcvProc,WfaRcvBE,WfaRcvVI,WfaRcvStop},
		/*STC3*/ {WfaRcvProc,WfaRcvBE,WfaRcvBK,WfaRcvVI,WfaRcvVO,WfaRcvStop}
#endif
};

/* The DUT send table for each of the test cases*/
StationProcStatetbl_t stationProcStatetbl[LAST_TEST+1][12] = {
	/* Dummy*/{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}},
	/* B.D*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVO,P_ON,LII / 2},{WfaStaSndVO,P_ON,LII / 2} ,{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* B.H*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVO,P_ON,LII / 2},{WfaStaSndVO,P_ON,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* B.B*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVO,P_ON,LII / 2},{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndBE,P_ON,LII / 2} ,{WfaStaSndBK,P_ON,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0}},

	/* B.M*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,30000000},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 

	/* M.D*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{WfaStaSndStop,P_ON,LII / 2},{0,0,0}},

	/* B.Z*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVO,P_ON,LII / 2 },{WfaStaSndStop,P_ON,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* M.Y*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndVO,P_ON,LII / 2}   ,{WfaStaSndBE,P_ON,LII / 2},{WfaStaSndBE,P_ON,LII / 2},{ WfaStaSndStop,P_ON,LII/2}, {0,0,0},{0,0,0},{0,0,0}},

	/* L.1*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVOCyclic,P_ON,20000},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2 },*/ {0,0,0}},

	/* A.Y*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndVO,P_ON,LII / 2},{WfaStaSndBE,P_ON,LII / 1},{WfaStaSndBE,P_OFF,LII / 2},{WfaStaSndBE,P_ON,LII /1},{WfaStaSndStop,P_ON,LII / 1},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0}}, 

	/* B.W*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,LII / 2}   ,{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* A.J*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVO,P_ON,LII / 2}   ,{WfaStaSndVO,P_OFF,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop ,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* M.V*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,LII / 2}   ,{WfaStaSndBE,P_ON,LII / 2},{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop ,P_ON,LII / 2} ,*/{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* M.U*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,LegacyPS_PSPoll, LII / 2},
	{WfaStaSndVI,P_ON,LII / 2}   ,{WfaStaSndBE,P_ON,LII / 2},{WfaStaSnd2VO,P_ON,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop ,P_ON,LII / 2} ,*/{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* original version A.U as 05/22/2013 */
	/* use "RaOID -s apsd_ay 1" to enable PSnonPoll mode or "RaOID -s apsd_ay 0" to disable PSnonPoll mode. */
	{{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,LegacyPS_PSNonPoll, LII / 2},
	{WfaStaSndVI,LegacyPM_ON,LII / 2}  ,{WfaStaSndBE,LegacyPM_OFF,LII / 2},{WfaStaSndBE,LegacyPM_ON,LII / 2},{WfaStaSndBE,LegacyPM_OFF,LII / 2}   ,{WfaStaSndVO,LegacyPM_ON,LII / 2}   ,{WfaStaSndVO,LegacyPM_OFF,LII / 2} ,{WfaStaSndStop,LegacyPM_OFF,LII / 2},/*{WfaStaWaitStop ,P_ON,LII / 2},*/{0,0,0}},

	/* M.L*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndBE,P_ON,LII / 2}   ,{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* B.K*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,LII / 2}   ,{WfaStaSndBE,P_ON,LII / 2} ,{WfaStaSndVI,P_ON,LII / 2} ,{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* M.B*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVO,P_ON,LII / 2}   ,{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndBE,P_ON,LII / 2} ,{WfaStaSndBK,P_ON,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2} ,*/{0,0,0},{0,0,0},{0,0,0}},

	/* in the following two cases, in linux sta version sleep time if LII/8 benz  */
	/* M.K*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,LII / 2}   ,{WfaStaSndBE,P_ON,LII / 2}        ,{WfaStaSndVI,P_ON,LII / 2},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII / 2},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0}},

	/* M.W*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,LII / 8}   ,{WfaStaSndBE,P_ON,LII / 8} ,{WfaStaSndVI,P_ON,LII / 8},{WfaStaSndStop,P_ON,LII / 2},/*{WfaStaWaitStop,P_ON,LII /8},*/{0,0,0},{0,0,0},{0,0,0},{0,0,0}}
#ifdef WFA_WMM_AC
	/* ATC1*/  ,{{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVO,P_ON,1000000}   ,{WfaStaWaitStop,P_ON,LII / 2}     ,{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
	/* ATC2*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,1000000}   ,{WfaStaSndVO,P_ON,1000000}        ,{WfaStaSndBE,P_ON,1000000}    ,{WfaStaSndBE,P_ON,1000000}  ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0}},
	/* ATC3*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,becon_int}   ,{WfaStaSndVO,P_ON,1000000}      ,{WfaStaSndBE,P_ON,1000000}    ,{WfaStaSndBE,P_ON,1000000}  ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0}},
	/* ATC4*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,lis_int}   ,{WfaStaSndBE,P_ON,1000000}        ,{WfaStaSndVO,P_ON,1000000}    ,{WfaStaSndVO,P_ON,1}        ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0}},
	/* ATC5*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, 1},
	{WfaStaSndVI,P_ON,lis_int}   ,{WfaStaSndVO,P_ON,lis_int+2*becon_int}        ,{WfaStaSndVO,P_ON,becon_int}    ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
	/* ATC6*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,becon_int}   ,{WfaStaSndVO,P_ON,lis_int}        ,{WfaStaSndVI,P_ON,becon_int}  ,{WfaStaSndVO,P_ON,becon_int}  ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0}},
	/* ATC7*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,becon_int}   ,{WfaStaSndVO,P_ON,lis_int}        ,{WfaStaSndVI,P_ON,becon_int}  ,{WfaStaSndVO,P_ON,becon_int}  ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0}},
	/* ATC8*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVO,P_ON,lis_int}   ,{WfaStaSndVI,P_ON,lis_int+2*becon_int}        ,{WfaStaSndVI,P_ON,becon_int}  ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
	/* ATC9*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndBE,P_ON,becon_int}   ,{WfaStaSndVO,P_ON,lis_int}        ,{WfaStaSndBE,P_ON,becon_int}  ,{WfaStaSndVI,P_ON,becon_int}  ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0}},
	/* ATC10*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,becon_int}   ,{WfaStaSndBE,P_ON,lis_int}        ,{WfaStaSndVI,P_ON,becon_int}  ,{WfaStaSndBE,P_ON,becon_int}  ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0}},
	/* ATC11*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndBE,P_ON,becon_int}   ,{WfaStaSndVI,P_ON,lis_int}        ,{WfaStaSndBE,P_ON,becon_int}  ,{WfaStaSndVO,P_ON,lis_int}  ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0}},
	/* STC1*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,becon_int}   ,{WfaStaSndBE,P_ON,1000000}        ,{WfaStaSndVI,P_ON,1000000}    ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
	/* STC2*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,becon_int}   ,{WfaStaSndBE,P_ON,1000000}        ,{WfaStaSndVO,P_ON,1000000}    ,{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
	/* STC3*/  {{WfaStaSndHello,P_OFF, 1000000},{WfaStaSndConfirm,P_ON, LII / 2},
	{WfaStaSndVI,P_ON,becon_int}   ,{WfaStaSndVI,P_ON,becon_int}        ,{WfaStaSndVI,P_ON,becon_int}    ,{WfaStaSndVI,P_ON,becon_int},{WfaStaWaitStop,P_ON,LII / 2}  ,{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
#endif

};


int ac_seq[APTS_LAST][6] ={
	{0,      0,      0,      0,      0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0}, // APTS_TESTS
	{0}, // B.D
	{0}, // B.2
	{0}, // B.H
	{0}, // B.4
	{0}, // B_5
	{0, 0, 0, 0, 0}, // B_6
	{TG_WMM_AC_VO, TG_WMM_AC_VI, TG_WMM_AC_BE, TG_WMM_AC_BK, 0}, // B.B B_B - 4 exchanges: 1 uplink, 0 downlink
	{0}, // B.E
	{0}, // B.G
	{0}, // B.I
	{0}, // M.D
	{0}, // M.G
	{0}, // M.I
	{0}, // B.Z  1, 1, 1, 0},	// 1 special exchange for Broadcast testing
	{TG_WMM_AC_VI, TG_WMM_AC_VO, TG_WMM_AC_BE, TG_WMM_AC_BE, 0}, //  M.Y  M_Y 2 special exchange for Broadcast testing
	{0}, // L.1
	{0}, // DLOAD
	{0}, // ULOAD
	{0}, // "APTS PASS"
	{0}, // "APTS FAIL"
	//{TOS_VI, TOS_VO, TOS_BE, TOS_BE, 0}, //  A.Y A_Y special exchange for Broadcast testing
	{TG_WMM_AC_VI, TG_WMM_AC_VO, TG_WMM_AC_BE, TG_WMM_AC_BE, TG_WMM_AC_BE}, //  A.Y A_Y special exchange for Broadcast testing
	{0}, //  B.W  2 special exchange for Broadcast testing
	{0}, //  A.J
	{TG_WMM_AC_VI, TG_WMM_AC_BE, TG_WMM_AC_VI, TG_WMM_AC_VI, TG_WMM_AC_VI}, //  M.V M_V
	{TG_WMM_AC_VI, TG_WMM_AC_BE, TG_WMM_AC_VO, TG_WMM_AC_VO, TG_WMM_AC_VO}, //  M.U M_U
	{TG_WMM_AC_VI, TG_WMM_AC_BE, TG_WMM_AC_BE, TG_WMM_AC_BE, TG_WMM_AC_VO, TG_WMM_AC_VO},  //  A.U A_U
	{0}, //  M.L M_L
	{TG_WMM_AC_VI, TG_WMM_AC_BE, TG_WMM_AC_VI, TG_WMM_AC_VI, 0}, // B.K B_K
	{TG_WMM_AC_VO, TG_WMM_AC_VI, TG_WMM_AC_BE, TG_WMM_AC_BK, 0}, // M.B M_B - 4 exchanges: 1 uplink, 0 downlink
	{TG_WMM_AC_VI, TG_WMM_AC_BE, TG_WMM_AC_VI, TG_WMM_AC_VI, 0}, // M.K M_K
	{TG_WMM_AC_VI, TG_WMM_AC_BE, TG_WMM_AC_VI, TG_WMM_AC_VI, 0} //  M.W M_W   special exchange for Broadcast testing
};

/* Generic function to create a meassage, it also fills in the AC as part of
** the payload
** */
void create_apts_msg(int msg, unsigned int txbuf[],int id)
{
	struct apts_msg *t;

	t = &apts_msgs[msg];
	txbuf[ 0] = wfaTGWMMPSData.wmmps_info.my_cookie;
	txbuf[ 1] = wfaTGWMMPSData.wmmps_info.dscp;
	txbuf[ 2] = 0;
	txbuf[ 3] = 0;
	txbuf[ 4] = 0;
	txbuf[ 5] = 0; 
	txbuf[ 9] = id;
	txbuf[ 10] = t->cmd; 
	strcpy((char *)&txbuf[11], t->name);
	DPRINT_INFOL(WFA_OUT,"create_apts_msg (%s) %d\n", t->name,t->cmd);
}

void print_hex_string(char* buf, int len)
{
	int i;

	if (len==0) 
	{ 
		DPRINT_INFOL(WFA_OUT, "<empty string>"); 
		return; 
	}

	for (i = 0; i < len; i++) 
	{
		DPRINT_INFOL(WFA_OUT, "%02x ", *((unsigned char *)buf + i));
		if ((i&0xf)==15) DPRINT_INFOL(WFA_OUT, "\n   ");
	}
	if ((i&0xf))
		DPRINT_INFOL(WFA_OUT, "\n");
}

/* trace print*/
void mpx(char *m, void *buf_v, int len)
{
	char *buf = (char*)buf_v;

	DPRINT_INFOL(WFA_OUT, "%s   MSG: %s\n   ", m, &buf[44] );
	print_hex_string(buf, len);
}

/* function to validate the AC of the payload recd to ensure the correct
** message sequence*/
int receiver(unsigned int *rmsg,int length,int tos,unsigned int type)
{
	int r=1;
#ifndef WFA_WMM_AC
	int new_dscp=rmsg[1];

	if((new_dscp != tos)||(rmsg[10] != type))
	{
		DPRINT_INFOL(WFA_OUT,"\r\nERR dscp rcv is miss match:rcv dscp=0x%x expect tos=0x%x rcv msg type=%d expect type=%d\n",new_dscp,tos, rmsg[10], type);
		r=-6;
	}
#else
	if(rmsg[10] != type)
	{
		DPRINT_INFOL(WFA_OUT,"\r\n dscp recd is %d msg type is %d\n",new_dscp,rmsg[10]);
		r=-6;
	}
#endif

	return r;
}
/* WfaRcvProc: This function receives the test case name
** after sending the initial hello packet, on receiving a
** valid test case it advances onto the next receive state
*/
int WfaRcvProc(unsigned int *rmsg,int length,int *state)
{
	int sta_test;
	wfaTGWMMPSData.num_hello=0;
	DPRINT_INFOL(WFA_OUT,"WfaRcvProc entering ...");
	sta_test = rmsg[10];
	mpx("STA recv \n", rmsg, 64);
	if(!((sta_test >=B_D)&&(sta_test <= LAST_TEST)))
		return -1;
	if ( rmsg[10] > 0)
	{
		wfaTGWMMPSData.wmmps_info.sta_test = rmsg[10];
	}
	wfaTGWMMPSData.wmmps_info.my_sta_id = rmsg[9];
	pthread_mutex_lock(&wfaTGWMMPSData.wmmps_mutex_info.thr_flag_mutex);
	wfaTGWMMPSData.wmmps_mutex_info.thr_flag = wfaTGWMMPSData.wmmps_info.streamid;
	pthread_mutex_unlock(&wfaTGWMMPSData.wmmps_mutex_info.thr_flag_mutex);
	(*state)++;
	DPRINT_INFOL(WFA_OUT,"WfaRcvProc sta_test=%d next state=%d\n", sta_test, *state);
	return 0;
}

/* 
* WfaStaResetAll: This function resets the whole communication with
* the console (in the event of a wrong message received for the test case)
* resulting into resending of all the packets from the scratch, there is an
* upper bound for the resets a max of three
*/
void WfaStaResetAll()
{
	int r;
	DPRINT_INFOL(WFA_OUT,"Entering WfaStaResetAll\n");
	wfaTGWMMPSData.num_retry++;
	if(wfaTGWMMPSData.num_retry > MAXRETRY)
	{
		create_apts_msg(APTS_RESET_STOP, wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.wmmps_info.my_sta_id);
		wfaTGSetPrio(wfaTGWMMPSData.psSockfd, TG_WMM_AC_BE, wfaTGWMMPSData.wmmps_info.streamid);
		r = sendto(wfaTGWMMPSData.psSockfd, (char*)wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.msgsize, 0, (struct sockaddr *)&wfaTGWMMPSData.wmmps_info.psToAddr, sizeof(struct sockaddr));
		mpx("STA msg RESET_STOP sent",wfaTGWMMPSData.psTxMsg,64);
		DPRINT_INFOL(WFA_OUT, "Too many retries on reset\n");
		wfaTGWMMPSData.num_retry = 0;
		exit(-8);
	}
	if(!wfaTGWMMPSData.reset_recd)
	{
		create_apts_msg(APTS_RESET, wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.wmmps_info.my_sta_id);
		wfaTGSetPrio(wfaTGWMMPSData.psSockfd, TG_WMM_AC_BE, wfaTGWMMPSData.wmmps_info.streamid);
		wfaTGWMMPSData.psTxMsg[1] = TOS_BE;
		r = sendto(wfaTGWMMPSData.psSockfd, (char*)wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.msgsize, 0, (struct sockaddr *)&wfaTGWMMPSData.wmmps_info.psToAddr, sizeof(struct sockaddr));
		mpx("STA msg RESET sent",wfaTGWMMPSData.psTxMsg,64);
	}
	else
	{
		create_apts_msg(APTS_RESET_RESP, wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.wmmps_info.my_sta_id);
		wfaTGSetPrio(wfaTGWMMPSData.psSockfd, TG_WMM_AC_BE, wfaTGWMMPSData.wmmps_info.streamid);
		r = sendto(wfaTGWMMPSData.psSockfd, (char*)wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.msgsize, 0, (struct sockaddr *)&wfaTGWMMPSData.wmmps_info.psToAddr, sizeof(struct sockaddr));
		mpx("STA msg RESET_RESP sent", wfaTGWMMPSData.psTxMsg,64);
		wfaTGWMMPSData.reset_recd=0;
	}
	wfaTGWMMPSData.num_hello=0;
	wfaTGWMMPSData.resetsnd=1;
	wfaTGWMMPSData.resetrcv=1;
}

/*
* WfaRcvVO: A function expected to receive a AC_VO packet from
* the console, if does not reeive a valid VO resets the communication wit
* h the console
*/
int WfaRcvVO(unsigned int *rmsg,int length,int *state)
{
	int r;

	if ((r=receiver(rmsg,length,TOS_VO,APTS_DEFAULT))>=0)
	{
		DPRINT_INFOL(WFA_OUT,"WfaRcvVO, state=%d\n", *state);
		(*state)++;
	}
	else
	{
		DPRINT_INFOL(WFA_OUT,"\nBAD RCV in VO, ret=%d, call ResetAll\n",r);
		WfaStaResetAll();
	}
	return 0;
}
/* 
* WfaRcvVI: A function expected to receive a AC_VI packet from
* the console, if does not reeive a valid VI resets the communication wit
* h the console
*/
int WfaRcvVI(unsigned int *rmsg,int length,int *state)
{
	int r;
	if ((r=receiver(rmsg,length,TOS_VI,APTS_DEFAULT))>=0)
	{
		DPRINT_INFOL(WFA_OUT,"WfaRcvVI, state=%d\n", *state);
		(*state)++;
	}
	else
	{
		DPRINT_INFOL(WFA_OUT,"\nBAD RCV in VI, ret=%d\n",r);
	}
	return 0;
}

/* 
* WfaRcvBE: A function expected to receive a AC_BE packet from
* the console, if does not reeive a valid BE resets the communication wit
* h the console
*/
int WfaRcvBE(unsigned int *rmsg,int length,int *state)
{

	int r;
	if ((r=receiver(rmsg,length,TOS_BE,APTS_DEFAULT))>=0)
	{
		DPRINT_INFOL(WFA_OUT,"WfaRcvBE, state=%d\n", *state);
		(*state)++;
	}
	else
	{
		DPRINT_INFOL(WFA_OUT,"\nBAD RCV in BE ret=%d\n",r);
	}
	return 0;
}

/*
* WfaRcvBK: A function expected to receive a AC_BK packet from
* the console, if does not reeive a valid BK resets the communication wit
* h the console
*/
int WfaRcvBK(unsigned int *rmsg,int length,int *state)
{
	int r;
	if ((r=receiver(rmsg,length,TOS_BK,APTS_DEFAULT))>=0)
	{
		DPRINT_INFOL(WFA_OUT,"WfaRcvBK, state=%d\n", *state);
		(*state)++;
	}
	else
	{
		DPRINT_INFOL(WFA_OUT,"\nBAD RCV in BK ret=%d\n",r);
	}
	return 0;
}

/*
* WfaRcvVOCyclic: This is meant for the L1 test case. The function
* receives the VO packets from the console
*/
int WfaRcvVOCyclic(unsigned int *rmsg,int length,int *state)
{
	int r;  
	if(rmsg[10] != APTS_STOP)
	{
		if ((r=receiver(rmsg,length,TOS_VO,APTS_DEFAULT))>=0)
		{
			;
		}
		else
		{
			DPRINT_INFOL(WFA_OUT,"\nWfaRcvVOCyclic::BAD RCV in VO, ret=%d\n",r);
		}
	}
	else
	{
		DPRINT_INFOL(WFA_OUT,"\nWfaRcvVOCyclic in...\n");
		wfaTGWMMPSData.gtgWmmPS = 0;
		closesocket(wfaTGWMMPSData.psSockfd);
		wfaTGWMMPSData.psSockfd = -1;
		//signal(SIGALRM, SIG_IGN);
		wfaSetDUTPwrMgmt(PS_OFF);
		Sleep(1000);
	}
	return 0;
}

/*
* WfaRcvStop: This function receives the stop message from the
* console, it waits for the sending thread to have sent the stop before
* quitting
*/
int WfaRcvStop(unsigned int *rmsg,int length,int *state)
{
	DPRINT_INFOL(WFA_OUT,"\r\nEnterring WfaRcvStop\n");
	if((rmsg[10] == APTS_RESET_RESP) || (rmsg[10] !=APTS_STOP))
	{
		DPRINT_INFOL(WFA_OUT,"\nRCV APTS_RESET_RESP, ResetAll sent rmsg[10]=%d\n",rmsg[10]);
		WfaStaResetAll();
	}
	else
	{
		if (wfaTGWMMPSData.psSockfd <=0)
		{
			DPRINT_INFOL(WFA_OUT,"\r\nWfaRcvStop:: UDP sock closed too early\n");
		}
		WfaStaSndStop(P_ON, LII/2, state);
		wfaSetDUTPwrMgmt(PS_OFF);
		DPRINT_INFOL(WFA_OUT,"\r\nWfaRcvStop end, but not close UDP sock yet\n");
	}
	return 0;
}
void BUILD_APTS_MSG(int msg, unsigned long *txbuf)
{   
	struct apts_msg *t;
	t = &apts_msgs[msg];
	txbuf[0] = wfaTGWMMPSData.wmmps_info.msgno++;
	txbuf[1] = 0;
	txbuf[2] = 0;
	txbuf[3] = 0;
	txbuf[4] = 0;
	txbuf[5] = 0;
	txbuf[6] = t->param0;
	txbuf[7] = t->param1;
	txbuf[8] = t->param2;
	txbuf[9] = t->param3;
	txbuf[10] = t->cmd;
	strcpy((char *)&txbuf[11], t->name); 
	DPRINT_INFOL(WFA_OUT,"BUILD_APTS_MSG::%s\n",t->name);
}

void send_txmsg(int new_prio_class)
{
	int r;
	int new_dscp = 0;

	if(new_prio_class > -1)
		new_dscp = wfaTGSetPrio(wfaTGWMMPSData.psSockfd, new_prio_class, wfaTGWMMPSData.wmmps_info.streamid);

	wfaTGWMMPSData.psTxMsg[0] = wfaTGWMMPSData.wmmps_info.msgno++;
	wfaTGWMMPSData.psTxMsg[1] = new_dscp;
	wfaTGWMMPSData.psTxMsg[2] = wfaTGWMMPSData.wmmps_info.my_group_cookie;
	wfaTGWMMPSData.psTxMsg[3] = wfaTGWMMPSData.wmmps_info.my_cookie;
	wfaTGWMMPSData.psTxMsg[4] = wfaTGWMMPSData.wmmps_info.my_sta_id;

	if(wfaTGWMMPSData.psTxMsg[10] == APTS_DEFAULT)
	{
		wfaTGWMMPSData.psTxMsg[13] = (wfaTGWMMPSData.wmmps_info.msgno%10) + 0x20202030; 
	}

	r = wfaTrafficSendTo(wfaTGWMMPSData.psSockfd, (char *)wfaTGWMMPSData.psTxMsg, 200+(wfaTGWMMPSData.wmmps_info.msgno%200), (struct sockaddr *) &wfaTGWMMPSData.wmmps_info.psToAddr);

	wfaTGWMMPSData.wmmps_info.nsent++;
}

/*
* This needs to adopt to the specific platform you port to.
*/
void wfaSetDUTPwrMgmt(int mode)
{
	static int curr_mode = -1;
	int st = -1;
	char modeStr[16] ;
	if(curr_mode == mode)
	{
		DPRINT_INFOL(WFA_OUT,"wfaSetDUTPwrMgmt current mode=%d pass-in mode=%d, no action\n",curr_mode,mode );
		return;
	}
	memset(modeStr, 0, 16);
	DPRINT_INFOL(WFA_OUT,"In wfaSetDUTPwrMgmt\n");

	switch (mode)
	{
	case P_ON: 
		strcpy(modeStr, "on");
		break;
	case P_OFF:
		strcpy(modeStr, "off");
		break;
	case LegacyPS_OFF:
		strcpy(modeStr, "off");
		break;
	case LegacyPS_PSPoll:
		strcpy(modeStr, "PSPoll");
		break;
	case LegacyPS_FAST:
		strcpy(modeStr, "Fast");
		break;
	case LegacyPS_PSNonPoll:
		strcpy(modeStr, "PSNonPoll");
		break;
	default:
		strcpy(modeStr, "off");
		DPRINT_ERR(WFA_ERR, "PS mode set error, set to 'off' as default, mode=%d\n", mode);
	}

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
	case WMM_STAUT:
		sprintf(wfaDutAgentData.gCmdStr, "cd %s & sta_set_pwrsave /interface staut /action %s", wfaDutAgentData.WFA_CLI_CMD_DIR, modeStr);
		st = wfaExecuteCLI(wfaDutAgentData.gCmdStr);
		DPRINT_INFOL(WFA_OUT,"\nWMM_STAUT WMMPS CMD:%s status=%d mode=%d\n", wfaDutAgentData.gCmdStr, st, mode);
		break;
	case WMM_RALINK:            
		break;
	default:
		DPRINT_ERR(WFA_ERR, "Unknown Vendor, vendorIdex=%d\n", wfaDutAgentData.vend);
		goto end_clean;
	}
	curr_mode = mode;
end_clean:
	return;
}

int timeout_recvfrom(int sock,char* data,int length,int flags,struct sockaddr* sockfrom,int* fromlen,int timeout){
	fd_set socks;
	struct timeval t;
	int bytes=0, ret = 0;
	FD_ZERO(&socks);
	FD_SET(sock,&socks);
	t.tv_sec=timeout;
	if(ret = select(sock+1,&socks,NULL,NULL,&t)){
		bytes=recvfrom(sock,data,length,0,sockfrom,fromlen);
		return bytes;
	}
	else
	{
		if ( ret == SOCKET_ERROR)
		{
			DPRINT_ERR(WFA_ERR,"timeout_recvfrom got SOCKET_ERROR\n");
		}
		return -1;
	}
}
void wfaWmmpsInitFlag(void)
{
	wfaTGWMMPSData.num_retry=0;
	wfaTGWMMPSData.state_num = 1;/* send state, skip first hello send */
	wfaTGWMMPSData.resetsnd = 0;
	wfaTGWMMPSData.num_hello=0;
	wfaTGWMMPSData.num_stops=0;

	wfaTGWMMPSData.wmmps_info.sta_test=1; /* test case index,   */
	wfaTGWMMPSData.wmmps_info.rcv_state=0;
	wfaTGWMMPSData.wmmps_info.sta_state = 0;
	wfaTGWMMPSData.wmmps_info.wait_state = WFA_WAIT_STAUT_00;
	wfaTGWMMPSData.wmmps_info.resetWMMPS = 0;

	DPRINT_INFOL(WFA_OUT,"wfaWmmpsInitFlag::reset all flags\n");
}

/* following routine replace wfaWmmPowerSaveProcess  */
void* wfa_wmmps_thread(void* input)
{
	int rbytes=0;
	int sta_test=1;
	struct sockaddr from;
	int len;
	StationRecvProcStatetbl_t *rcvstatarray;
	StationRecvProcStatetbl_t func;
	HANDLE sendThreadHandle = 0;
	DWORD  sendThreadId = 0;
	len=sizeof(from);

	while(1){
		if (sendThreadHandle != 0)
		{
			CloseHandle(sendThreadHandle);
			sendThreadHandle = 0;
		}
		pthread_mutex_lock(&wfaTGWMMPSData.wmmps_mutex_info.thr_flag_mutex);
		DPRINT_INFOL(WFA_OUT, "wfa_wmmps_thread::wait...\n");
		pthread_cond_wait(&wfaTGWMMPSData.wmmps_mutex_info.thr_flag_cond,&wfaTGWMMPSData.wmmps_mutex_info.thr_flag_mutex);
		pthread_mutex_unlock(&wfaTGWMMPSData.wmmps_mutex_info.thr_flag_mutex);
		sendThreadHandle = CreateThread(NULL, 0, wfa_wmmps_send_thread, (PVOID)&wfaTGWMMPSData.wmmps_info, 0,&sendThreadId);
		Sleep(1000);

		DPRINT_INFOL(WFA_OUT, "wfa_wmmps_thread::run...gtgWmmPS=%d psSockfd=%d\n", wfaTGWMMPSData.gtgWmmPS, wfaTGWMMPSData.psSockfd);
		while(wfaTGWMMPSData.gtgWmmPS > 0)
		{
			if(wfaTGWMMPSData.resetsnd)//When the testcase ends, WfaStaResetAll() sets the resetsnd to 1
			{
				DPRINT_INFOL(WFA_OUT, "wfa_wmmps_thread::got resetsnd on, set off of it\n");
				wfaTGWMMPSData.resetsnd = 0;
			}
			else
			{
				/* then receive  */
				if (wfaTGWMMPSData.psSockfd > 0)
				{
					memset(wfaTGWMMPSData.psRxMsg, 0, 512);
					rbytes = 0;
					rbytes=timeout_recvfrom(wfaTGWMMPSData.psSockfd,(char*)wfaTGWMMPSData.psRxMsg,WMMPS_MSG_BUF_SIZE-4,0,&from,&len,3);
				}

				if(rbytes>0) /*  only when we really get data from PC_Endpoint */
				{
					DPRINT_INFOL(WFA_OUT, "wfa_wmmps_thread::sta_test(tst case#)=%d rbytes=%d rcv_state=%d rmsg[10]=%d\n",sta_test, rbytes, wfaTGWMMPSData.wmmps_info.rcv_state, wfaTGWMMPSData.psRxMsg[10]);

					wfaTGWMMPSData.wmmps_info.my_cookie = wfaTGWMMPSData.psRxMsg[0];
					sta_test = wfaTGWMMPSData.wmmps_info.sta_test;
					if((sta_test!=L_1))
					{
						mpx("RX msg", wfaTGWMMPSData.psRxMsg, 64);
					}
					if(wfaTGWMMPSData.psRxMsg[10]==APTS_STOP)
					{
						if (wfaTGWMMPSData.wmmps_info.rcv_state > 0)
						{
							DPRINT_INFOL(WFA_OUT, "\r\nwfa_wmmps_thread::stop rcv rcv_state=%d\n", (wfaTGWMMPSData.wmmps_info.rcv_state));
							wfaTGWMMPSData.resetsnd = 1; // by Qiuming word, benz, use STOP as super state to stop test case.
							WfaRcvStop(wfaTGWMMPSData.psRxMsg,rbytes,&(wfaTGWMMPSData.wmmps_info.rcv_state));
							rbytes =0;
							continue;
						}/* else will ignore */
						else
						{
							DPRINT_INFOL(WFA_OUT, "\r\nwfa_wmmps_thread::stop rcv rcv_state=%d ERR\n", (wfaTGWMMPSData.wmmps_info.rcv_state));
						}
					}
					if(wfaTGWMMPSData.psRxMsg[10]==APTS_RESET){
						wfaTGWMMPSData.reset_recd=1;
						DPRINT_INFOL(WFA_OUT, "\r\nwfa_wmmps_thread:: rcv RESET call WfaStaResetAll\n");
						WfaStaResetAll();
						rbytes = 0;
						continue; 
					}

					rcvstatarray=stationRecvProcStatetbl[sta_test];//receive functions
					func=rcvstatarray[(wfaTGWMMPSData.wmmps_info.rcv_state)];
					if (func.statefunc != NULL) {
						DPRINT_INFOL(WFA_OUT, "wfa_wmmps_thread:: call state func in state=%d\n", wfaTGWMMPSData.wmmps_info.rcv_state);
						func.statefunc(wfaTGWMMPSData.psRxMsg,rbytes, &(wfaTGWMMPSData.wmmps_info.rcv_state));
					}
					else
					{
						DPRINT_INFOL(WFA_OUT, "wfa_wmmps_thread::Detected NULL func pt in rcv func table vcs_state=%d\n", wfaTGWMMPSData.wmmps_info.rcv_state);
					}
					rbytes=0;
				}
			}

			if(wfaTGWMMPSData.num_hello > MAXHELLO)
			{
				wfaTGWMMPSData.resetsnd=1;
				DPRINT_INFOL(WFA_OUT,"wfa_wmmps_thread::sent hello too many %d. Restart test script or dut/ca in STA\n", wfaTGWMMPSData.num_hello);
				wfaTGWMMPSData.wmmps_info.resetWMMPS = 1;

				WfaStaResetAll();
				closesocket(wfaTGWMMPSData.psSockfd);
				wfaTGWMMPSData.psSockfd = -1;
				wfaSetDUTPwrMgmt(PS_OFF);
				wfaTGWMMPSData.gtgWmmPS = 0; /* exit this internal loop */
			}
			Sleep(5);

		}/* while(gtgWmmPS>0)  */
	}/*  while loop */
	return NULL;
}/* wfa_wmmps_thread */

DWORD WINAPI wfa_wmmps_send_thread(void* input)
{
	StationProcStatetbl_t *sendstate,curr_state;
	tgProfile_t *pTGProfile = findTGProfile(wfaTGWMMPSData.wmmps_info.streamid);
	int iOptVal=0, iOptLen =sizeof(int);

	DPRINT_INFOL(WFA_OUT,"wfa_wmmps_send_thread Start\n");
	if ( pTGProfile == NULL)
	{
		DPRINT_ERR(WFA_ERR, "wfa_wmmps_send_thread: TG frofile is NULL, exit\n");
		return 0;
	}
	if (wfaTGWMMPSData.psSockfd > 0)
	{
		DPRINT_INFOL(WFA_OUT, "wmmps_send_thread: WMMPS early socket not closed\n");
		closesocket(wfaTGWMMPSData.psSockfd);
		wfaTGWMMPSData.psSockfd = 0;
	}

	// used for receive data and send hello only
	wfaTGWMMPSData.psSockfd = wfaCreateUDPSock(pTGProfile->dipaddr, WFA_WMMPS_UDP_PORT);
	if (wfaTGWMMPSData.psSockfd > 0)
	{
		DPRINT_INFOL(WFA_OUT, "wfaCreateUDPSock: WMMPS profile create UDP socket for RCV OK psSockfd=%d\n", wfaTGWMMPSData.psSockfd);
	}
	else
	{
		DPRINT_ERR(WFA_ERR, "wfaCreateUDPSock: WMMPS profile create UDP RCV socket failed\n");
	}

	if (getsockopt(wfaTGWMMPSData.psSockfd, SOL_SOCKET, SO_SNDBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
	{
		DPRINT_INFOL(WFA_OUT, "wfa_wmmps_send_thread:: get SO_SNDBUF Value=%ld\n", iOptVal);
	}

	Sleep(1000);
	while(wfaTGWMMPSData.gtgWmmPS > 0 && wfaTGWMMPSData.psSockfd > 0)
	{
		if (wfaTGWMMPSData.wmmps_info.rcv_state > 0)/*  check state forwarded */
		{
			DPRINT_INFOL(WFA_OUT,"1-send state_num=%d sta_test=%d\n", wfaTGWMMPSData.state_num, wfaTGWMMPSData.wmmps_info.sta_test); 
			sendstate =  stationProcStatetbl[wfaTGWMMPSData.wmmps_info.sta_test];
			curr_state = sendstate[wfaTGWMMPSData.state_num];
			if ( curr_state.statefunc != NULL)
			{
				curr_state.statefunc(curr_state.pw_offon,curr_state.sleep_period,&(wfaTGWMMPSData.state_num));//send functions
			}
			else
			{
				DPRINT_ERR(WFA_ERR, "wfa_wmmps_send_thread, state Func is NULL, exit loop\n");
				break;
			}
		}
		else
		{
			/* For WMM-Power Save
			* periodically send HELLO to Console for initial setup if not rcv pkt from PCE as rcv_state==0.
			*/
			if((wfaTGWMMPSData.psSockfd != -1) && (wfaTGWMMPSData.num_hello <= MAXHELLO) && (wfaTGWMMPSData.wmmps_info.rcv_state <=0))
			{
				memset(wfaTGWMMPSData.psTxMsg, 0, WMMPS_MSG_BUF_SIZE);
				BUILD_APTS_MSG(APTS_HELLO, (unsigned long *)wfaTGWMMPSData.psTxMsg);
				wfaTrafficSendTo(wfaTGWMMPSData.psSockfd, (char *)wfaTGWMMPSData.psTxMsg, WMMPS_MSG_BUF_SIZE/4, (struct sockaddr *) &wfaTGWMMPSData.wmmps_info.psToAddr);
				wfaTGWMMPSData.num_hello++;
				DPRINT_INFOL(WFA_OUT,"wmmps_send_thread::sent hello \n");
				wfaTGWMMPSData.wmmps_info.sta_state = 0;
				wfaTGWMMPSData.wmmps_info.wait_state = WFA_WAIT_STAUT_00;
				Sleep(1000);
			}
			Sleep(100);
		}
		if (wfaTGWMMPSData.wmmps_info.resetWMMPS == 1)
		{
			DPRINT_WARNING(WFA_OUT,"wfa_wmmps_send_thread end with resetWMMPS on\n");
			break;
		}
	}/*  while(gtgWmmPS>0 && psSockfd>0)  */

	Sleep(300);
	DPRINT_INFOL(WFA_OUT,"wfa_wmmps_send_thread end\n");
	return 0;
}

#endif /* WFA_WMM_PS */

