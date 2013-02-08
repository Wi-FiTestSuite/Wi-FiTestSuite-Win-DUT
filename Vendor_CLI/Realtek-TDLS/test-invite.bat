cls

call sta_p2p_reset /interface rtl8192s

call sta_start_autonomous_go /interface rtl8192s /oper_chn 36 /ssid -test-invite

call sta_send_p2p_invitation_req /interface rtl8192s /p2pdevid 00:e0:4c:92:01:cf /reinvoke 0