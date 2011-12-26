************************************************************************************
(1) Usage for RtlUI2.exe:
    a. "RtlUI2.exe -?" : get all support commands.
    b. "RtlUI2.exe -[main argument] ?" get the main argument help
	such as typing "RtlUI2.exe -set_p2p ?".
====================================================================================
************************************************************************************
(2) devleopment notes:
[2010-09-10, By Haich]
	a. Add reset power state in sta_p2p_reset as suggested by Bruce.
	b. Fix sta_add_arp_table_entry.bat where "macaddress" should be "/macaddress" in the arg list.
	c. In sta_set_opportunistic_ps.bat, set ctwindow as: RtlUI2.exe -sta_set_opportunistic_ps -ctwindow %ctwindow%
[2010-09-06, By Haich]
	a. Fix sta_block_icmp_rsp
	b. Fix sta_p2p_reset
	c. Fix sta_set_opportunistic_ps
[2010-04-22, By Bruce]
    a. Fix the parameter [device] to [discovery] in "rt_ui_enable_p2p.bat".
    b. Update RtlUI2.exe from version 20100415 to 20100420.
====================================================================================
************************************************************************************
(3) Related information:
[2010-09-06, By Haich]
	Enable firewall:
	netsh advfirewall set allprofiles state on

	Disable firewall:
	netsh advfirewall set allprofiles state off

	Delete rule:
	netsh advfirewall firewall delete rule name="yyy"

	Show rule:
	netsh advfirewall firewall show rule name="yyy"

	Add rule:
	netsh advfirewall firewall add rule name="yyy" dir=in action=block profile=any remoteip=172.21.72.124 protocol=icmpv4:8,any interfacetype=wireless

	Delete all rules:
	netsh advfirewall firewall delete rule name=all

	How to Enable Windows 7 Ping Response in Firewall:
	http://www.sysprobs.com/enable-ping-reply-windows-7

	Netsh AdvFirewall Firewall Commands:
	http://technet.microsoft.com/en-us/library/dd734783(WS.10).aspx

	How to use the "netsh advfirewall firewall" context instead of the "netsh firewall" context to control Windows Firewall behavior in Windows Server 2008 and in Windows Vista:
	http://support.microsoft.com/kb/947709	
	http://support.microsoft.com/kb/947709	
====================================================================================
