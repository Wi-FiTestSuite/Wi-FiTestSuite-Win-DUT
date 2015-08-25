#Windows DUT Build and Installation Guidelines
July 23, 2015
Version 1.0 Beta


##1.	Introduction
This document describes the procedure that users should follow to build, install and run the Windows control agent and DUT agent.
##2.	Code structure of Windows DUT code
The following tree shows the structure of windows DUT code for build and installation:
- win_ca
	- win_ca
- win_dut
	- win_dut
- win_inc
- win_lib
- installation
- Vendor_CLI
	- WMM_STAUT
	- Device Vendor Name such as XYZ-11N
	- Your CLI batch files
- Tools
	- myping
- External
	- pthreadlib
		- include
		- lib

	Note: WMM_STAUT folder is created to store the CLI batch files which are implemented for CAPI command in WMM-PS program.

##3.	Build Windows control agent and DUT agent
To build the windows control agent and DUT agent, the proper headers and libraries are required to be downloaded and linked.
###Pthreads for win32
As of now, the only external APIs referenced in the code-base is pthreads for win32. It can be downloaded from:
[ftp://sourceware.org/pub/pthreads-win32/dll-latest](ftp://sourceware.org/pub/pthreads-win32/dll-latest)
The relevant headers, libraries and dlls are:

|Headers	Libraries|	DLLs|
|--------------|:--------------|
|pthread.h|	pthreadVSE2.lib	pthreadVSE2.dll|
|sched.h||
|semaphore.h||

In the current code structure, the above header and library files are included in the\external\pthreadlib directory. As shown in the win_dut example below, the visual studio project files are pre-configured to include the header and library files.

![alt tag](https://github.com/Wi-FiTestSuite/Wi-FiTestSuite-Win-DUT/blob/master/Docs/pics/DUT_build_1.png)
![alt tag](https://github.com/Wi-FiTestSuite/Wi-FiTestSuite-Win-DUT/blob/master/Docs/pics/DUT_build_2.png)
![alt tag](https://github.com/Wi-FiTestSuite/Wi-FiTestSuite-Win-DUT/blob/master/Docs/pics/DUT_build_3.png)


##4.	Installation of Windows control and DUT agent
The following is the installation procedure for the Windows control agent and DUT agent:

1. Install Visual Studio C++ redistributable 2010 before starting the Windows control and DUT agent installation procedure.
https://www.microsoft.com/en-us/download/details.aspx?id=5555

2. The win_ca.exe and win_dut.exe binaries can be executed either through the console or as a Windows service. If you prefer to wrap win_ca.exe and win_dut.exe as Windows services, choose any third-party application such as XYNTService on GitHub.
https://github.com/mikeowens/XYNTService
After downloading the source code, build the binary, and then rename the binary as WfaControlAgent. Replicate the binary with the name WfaDutAgent. From these two binaries, use the WfaControlAgent.ini and WfaDutAgent.ini in the install folder as the default configuration ini files.
3. Make sure the WLAN AutoConfig service is running.
4. Copy the following dlls into the installation folders msvcp100.dll, msvcr100.dll, and pthreadVSE2.dll.

5.	Navigate to the installation folder and run install.bat. At the prompt, enter the name of your station.
6.	If your CAPI implementation is through the CLI, execute the install.bat file in the Vendor_CLI directory to install the device implemented batch files.
7.	Perform a system reboot.

##5.	Execution of Windows control and DUT agent
Make sure to put the CLI batch files which implement the CAPI commands in the C:\WFA\CLI\<Device Vendor Name> folder, where <Device Vendor Name> is the same name you selected during the installation for your device.

If you built the WfaControlAgent and WfaDutAgent binaries and ran the install.bat file, the Windows control agent and DUT agent should now be running as windows services. If you prefer to run two agents from the command line, manually start them as follows:
- win_ca.exe [control interface] [control port] 127.0.0.1 8000
- win_dut.exe 127.0.0.1 8000

where [control interface] can be a dummy interface such as eth0 and [control port] is the port number used to communicate with the test console such as 9000.

The logs for the Windows DUT may be retrieved from the folders C:\WFA\Logs\WFACA and C:\WFA\Logs\WFASTA.



