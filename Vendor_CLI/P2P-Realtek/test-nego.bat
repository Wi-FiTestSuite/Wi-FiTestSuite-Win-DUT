cls

call sta_p2p_reset /interface rtl8192s

call sta_send_p2p_provision_dis_req /interface rtl8192s /configmethod pbc /p2pdevid 00:e0:4c:92:01:cf

call rt_delay_sec 5

call sta_set_wps_pbc /interface rtl8192s

call sta_p2p_start_group_formation /interface rtl8192s /p2pdevid 00:e0:4c:92:01:cf /intent_val 15 /init_go_neg 1