#Windows DUT Build and Installation Guidelines
July 23, 2015
Version 1.0 Beta


##1.	Introduction
The document is to describe the steps and procedure that users should follow to build, install and run Windows control agent and DUT agent.
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

In the current code structure, the above header and library files are included in the\external\pthreadlib directory. As shown below, the visual studio project files are pre-configured to include them, taking win_dut for an example.

![alt tag](https://github.com/Wi-FiTestSuite/Wi-FiTestSuite-Win-DUT/blob/master/Docs/pics/DUT_build_1.png)
![alt tag](https://github.com/Wi-FiTestSuite/Wi-FiTestSuite-Win-DUT/blob/master/Docs/pics/DUT_build_2.png)
![alt tag](https://github.com/Wi-FiTestSuite/Wi-FiTestSuite-Win-DUT/blob/master/Docs/pics/DUT_build_3.png)


##4.	Installation of Windows control and DUT agent
The following is the installation procedure for Windows control agent and DUT agent:

1. Before the installation procedure starts, you need to install the visual studio C++ redistributable 2010 first.
https://www.microsoft.com/en-us/download/details.aspx?id=5555

2. The win_ca.exe and win_dut.exe binaries can be executed either through console or windows service. If you prefer to wrap win_ca.exe and win_dut.exe as windows services, you can choose any third-party application to do that, for example, the XYNTService on GitHub.
https://github.com/mikeowens/XYNTService
Once you download the source code, build the binary from it, and then rename it to WfaControlAgent. Replicate the binary with a different name WfaDutAgent. After having these two binaries, you can use the WfaControlAgent.ini and WfaDutAgent.ini in the install folder as the default configuration ini files.
3. Make sure WLAN AutoConfig service is running.
4. Copy the following dlls into the installation folder:
a. msvcp100.dll
b. msvcr100.dll
c. pthreadVSE2.dll

5.	Go to installation folder and run install.bat and then give the name of your station following the prompt.
6.	If your CAPI implementation is through CLI way, execute the install.bat file in the Vendor_CLI directory to install the device implemented batch files.
7.	Perform a system reboot.

##5.	Execution of Windows control and DUT agent
Make sure to put your CLI batch files which implement the CAPI commands in the C:\WFA\CLI\<Device Vendor Name> folder. <Device Vendor Name> is the same name you selected during the installation for your device.
If you already built the WfaControlAgent and WfaDutAgent binaries and run the install.bat file, the Windows control agent and DUT agent should be running as windows services now. Otherwise, if you like to run two agents from command line, you can manually start them as follows:
- win_ca.exe <control interface> <control port> 127.0.0.1 8000
- win_dut.exe 127.0.0.1 8000

where <control interface> can be a dummy one like eth0 and <control port> is the port number used to communicate with test console such as 9000.
The logs for Windows DUT can be retrieved from C:\WFA\Logs folder:
1. C:\WFA\Logs\WFACA
2. C:\WFA\Logs\WFASTA



