#!/bin/sh
cd /media/share/Stockland/earnings
rm earningshistory.txt
#rm /media/share/Stockland/earnings/earningshistory.txt
#python3 /media/share/Stockand/earnings/earnings_latest.py
python3 earnings_latest.py
# pull next earning dates
rm next_earningsdates.txt
python3 next_earnings_dates.py
