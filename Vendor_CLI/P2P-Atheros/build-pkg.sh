#!/bin/bash

git archive --format=tar --prefix=Atheros-P2P/ HEAD |
    gzip > Atheros-P2P.tar.gz

cd ..
tar czf Atheros-P2P/Atheros-P2P-drv.tar.gz \
    Atheros-P2P/newma/{adf.ko,ath_dev.ko,umac.ko,asf.ko,ath_hal.ko,ath_rate_atheros.ko,wlanconfig}

tar czf Atheros-P2P/Atheros-P2P-sigma.tar.gz \
    Atheros-P2P/{wfa_dut,wfa_ca} Atheros-P2P/scripts/*.sh

tar czf Atheros-P2P/Atheros-P2P-wpas.tar.gz \
    Atheros-P2P/{wpa_supplicant,wpa_cli}

cd Atheros-P2P
ls -l Atheros-P2P*.tar.gz
