
#Windows DUT Porting Guidelines
July 17,2015
Version 1.0 Beta

##1.	Scope
This document provides guidance for implementing Wi-Fi Test Suite Device under Test (DUT) code on a Windows platform so that devices may be tested and certified for Wi-Fi certification programs. It is also servers as a baseline for the open source community to extend the functionality and improve the quality of the Windows DUT code.

The DUT code consists of two agents (control agent and DUT agent) and several modules, i.e., control module, configuration module, and traffic module. The DUT code currently supports an Ethernet connection between the device and test console; other interfaces may be incorporated in the future.
##2.	Overview of DUT code architecture
![alt tag](https://github.com/Wi-FiTestSuite/Wi-FiTestSuite-Win-DUT/blob/master/Docs/pics/DUT_architechture.png)

The sample Windows DUT source code includes the modules shown in blue:
  1. Control module in control agent: The code receives the CAPI command from test console and converts it to the struct which is then wrapped into TLV format and sent to the DUT agent. The control agent can reside on either a control PC which is detached from the underlying device or actually on the device. The communication interface between control agent and DUT agent in the sample code is TCP socket over Ethernet. Other interfaces can be developed to support a variety of connections such as USB, USB over serial, serial or wireless and so on.
  2. Config module in DUT agent: The DUT agent receives data packets in TLV format from the network over an Ethernet interface. The Config module will then invoke the function corresponding to the decoded command based on its tag value. The function will in turn call either the low level API or CLI batch to execute the command.
  3. Traffic module in DUT agent: The DUT agent will generate the UDP based traffic and start sending/receiving it to/from the peers. The data packets are generated at either a fixed or maximum rate that is supported by the underlying OS.


##3.Working with the source code
The overall source code structure comprises the following four subdirectories:

|Directory name|	Description|
|--------------|:--------------|
|/win_ca|	Contains files related to control agent  communication with DUT agent and test console|
|/win_dut|	Contains files relate to DUT agent initialization and communication with control agent|
|/win_inc|	Contains header files shared by control and DUT agent|
|/win_lib|	Contains various library files shared by control and DUT agent|
|/Tools/myping|	Contains myping utility source files|
The win_ca and win_dut directories contains the visual studio project file for building the win_ca.exe and win_dut.exe binary files, respectively.

The win_ca directory includes the following source files:

|Source file name|	Description|
|--------------|:--------------|
|/win_ca/win_ca.h|	Defines the global structures used by the control agent.|
|/win_ca/win_ca.c|	The main entry of the control agent.|

The win_dut directory includes the following source files:

|Source file name|	Description|
|--------------|:--------------|
|/win_dut/win_dut.h|	Defines the global structures and variables used by DUT agent.|
|/win_dut/win_dut.c|	The main entry of DUT agent.|
|/win_dut/win_dut_init.c|	Various buffer initializations.|

The win_inc directory includes the following source files:

|Source file name|	Description|
|--------------|:--------------|
|/iphdr.h|Various protocol header definitions used by the raw socket.|
|/resolve.h|Contains common name resolution and name printing routines.|
|/wfa_ca_resp.h|Contains the declaration of functions which return command execution results to test console.|
|/wfa_cmds.h|	The definition of command types.|
|/wfa_debug.h|	The declaration of functions for debugging and logging.|
|/wfa_main.h|	Contains the common headers used by control agent and DUT agent.|
|/wfa_miscs.h|	Contains the declaration of miscellaneous functions.|
|/wfa_rsp.h|	Defines the data structures which store the command response information.|
|/wfa_sock.h|	The socket function declaration.|
|/wfa_tg.h|	Defines the macros and structs used for traffic generation.|
|/wfa_tlv.h|	Defines the TLV tag and its encoding/decoding routines.|
|/wfa_types.h|	Defines the string and buffer length macros.|
|/wfa_ver.h|	Defines the system version of the DUT agent.|
|/wfa_wmmps.h|	Macros and structs definition, and function declaration for WMM-PS.|

The win_lib directory contains the following source files:

|Source file name|	Description|
|:-----------------|:--------------|
|/wfa_ca_resp.c  |	Contains the functions which return results to test console.|
|/wfa_cmdproc.c|	The functions which parse CAPI commands.|
|/wfa_cmdtbl.c|	The declaration of functions for implementing CAPI commands.|
|/wfa_cs.c|	The function implementations of CAPI commands.|
|/wfa_miscs.c|	Contains miscellaneous utility functions.|
|/wfa_sock.c|	C wrappers of winsock.|
|/wfa_tg.c|	Contains traffic-related CAPI command processing functions.|
|/wfa_thr.c|	Thread function for sending/receiving packets based on the priority of packets and the affiliated functions which create QoS flow using TC APIs.|
|/wfa_tlv.c|	TLV tag encoding and decoding routines.|
|/wfa_typestr.c|	The declaration of CAPI parsing functions in control agent.|
|/wfa_wmm_tc.c|	Contains the functions used for the programs where the user priority needs to be set for the outgoing packets. |
|/wfa_wmmps.c|	The functions used exclusively for the WMM-PS program.|


The Tools directory contains the following source files:

|Source file name|	Description|
|:-----------------|:--------------|
|/Tools/myping/iphdr.h|	Various protocol header definitions used by the raw socket.|
|/Tools/myping/resolve.h|	The declaration of network address resolution functions.|
|/Tools/myping/resolve.c|	Contains functions for print, formatting and resolving the specified address.|
|/Tools/myping/myping.c|	Contains functions for how to use raw sockets to send ICMP requests and receive response.|


##4.	Overview of porting process
The DUT is a device usually with the following two network interfaces:
- A Wi-Fi Interface is typically implemented via a Wi-Fi chipset within the DUT which provides 802.11x MAC and PHY functionality together with the driver and other software.
- A control interface is typically an Ethernet link or others to which the DUT can be connected and receives CAPI commands.

The sample Windows DUT code is designed with easy portability and modularity in mind. Conceptually it consists of the following three functions:
- Communication: The CAPI commands are received from and the response is generated and sent back to test console via the TCP control link.
- Device configuration and action: The command and associated parameters are used to configure the device in preparation for a given test or to execute the specified action.
- Traffic generation: The data packets are generated and sent over the wireless interface as per the parameters specified (payload size, framerate, and IP address and port etc.).


####OS dependent APIs:
The OS dependency includes the use of socket and thread, and other system APIs with regard to QoS and system time. These APIs have been tested on Windows 7, 8 and 10 platforms, and are expected to work on other Windows platforms. The following table shows the distribution of the OS dependent APIs in different source files.

|Windows API|	Files|
|:-----------------|:--------------|
|socket	|Mainly in wfa_sock.c, select() is used in wfa_dut.c, wfa_thr.c and wfa_wmmps.c|
|Thread/mutex/condition variable|	wfa_dut.c, wfa_thr.c, wfa_tg.c, wfa_wmm_tc.c, and wfa_wmmps.c|
|QoS TC API|	wfa_thr.c|
|System time|	wfa_dut.c and wfa_miscs.c|
|Registry|	wfa_cs.c|

####Configuration and action APIs:
These APIs are mainly located in the wfa_cs.c file and invoked by the main code to configure the device and execute the Wi-Fi specific actions.
These APIs can in turn invoke the vendor’s low level APIs which will manage and control the device via the device driver. Each function in the wfa_cs.c file corresponds to a CAPI command. 

CAPI commands may be implemented through the Windows netsh utility or through the Command Line Interface (CLI). To implement the CLI, follow the steps below.
  1. Add the CAPI command to be implemented to the wfa_cli.txt file according to the following format where TRUE means the command should return the result to the test console and FALSE means return null.
	- CAPI_command-TRUE
	- CAPI_command-FALSE

  2. Create the implementation as a batch file with exactly the same file name as the CAPI command, and write the result back to the Windows DUT using setx. Each CLI must return an error code status through the environment variable - WFA_CLI_STATUS.
 WFA_CLI_STATUS Code Definition:
 - WFA_CLI_COMPLETE(0)
 - WFA_CLI_ERROR(1)
 - WFA_CLI_INVALID(2)

If there is any return value from the CLI, it should be returned through the environment variable WFA_CLI_RETURN (String). The return value format is "var1_name, var1_value, var2_name, var2_value". For example, if CLI “STA_P2P_START_GROUP_FORMATION” returns two values “result” and “groupid” then environment variable WFA_CLI_RETURN should be set to “result,client,groupid,00:11:22:33:44:55 DIRECT-S2”.

Setx or other approaches can be used to write the result back to the Windows environment variables. For example,
- SETX WFA_CLI_STAUS 0
- SETX WFA_CLI_RETURN “result,client,groupid,00:11:22:33:44:55 DIRECT-S2”

##5.	API implementation for new command
When implementing a new CAPI command, it will be simpler to follow the steps above for the CLI implementation. If using an API is preferred, the following is the suggested procedure:
  1.	Define the new command’s TLV tag in wfa_tlv.h.
  2.	Declare the response function in wfa_ca_resp.h and add the function implementation in wfa_ca_resp.c.
  3.	Add a command parsing handling function to the function array of nameStr in wfa_typestr.c and implement it in wfa_cmdproc.c.
  4.	Define a new struct for the command in wfa_cmds.h if it is different from dutCommand_t and a response struct in wfa_rsp.h if it is not a generic one as dutCmdResponse_t.
  5.	Declare the function in gWfaCmdFuncTbl of wfa_cmdtbl.c and Implement it in wfa_cs.c.

##6.	Program specific build options
The source code defines a few Wi-Fi Alliance certification program specific compiling switches for building the executable for either an individual program or all the programs. When using these switches, only certification program specific CAPI commands will be included in the build. The relationship among the switches is illustrated as below:

|Build options|	Programs to build|
|:-----------------|:--------------|
|WFA_WMM_WPA2|	11n, VHT, TDLS|
|WFA_WMM_PS|	WMM-PS|
|WFA_WMM_AC|	WMM-AC|
|WFA_P2P|	P2P|
|WFA_WMM_WPA2 and WFA_VOICE_EXT| 	Voice Enterprise|
|Not specified|	PMF, WFD, WFDS, HS2|
|All specified|	All the programs as state above|

