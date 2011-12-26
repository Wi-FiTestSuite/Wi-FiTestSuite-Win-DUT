# Run wfa_dut at the DUT PC

echo ===============================================================
echo . Before starting this script, remember to run 'screen' first.
echo . After starting this script, press ctrl-A, H to capture screen
echo   output to the log file screenlog.0.
echo . After exiting this script, rename screenlog.0 to dut-log.txt.
echo ===============================================================

date
./wfa_dut lo 8000 
