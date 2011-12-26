cls

call sta_p2p_reset /interface rtl8192s

call test-nego.bat

call sta_p2p_dissolve /interface rtl8192s

call rt_delay_sec 5

call sta_send_p2p_invitation_req /interface rtl8192s /p2pdevid 00:e0:4c:92:01:cf /reinvoke 1
