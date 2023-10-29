#!/bin/bash
python3 /stockminer/scripts/allcaps.py /stockminer/data/stocks/stockdb/yahoo/ 1>/dev/shm/stocks.out
grep "found, symbol may be delisted" /dev/shm/stocks.out|cut -d':' -f1|cut -d' ' -f2 > /dev/shm/delisted.stocks
/stockminer/admin --delisted /dev/shm/delisted.stocks
/stockminer/scripts/delstocks.sh /dev/shm/delisted.stocks  # will call sync.sh
node /stockminer/scripts/verify.js 1>/dev/shm/verify.txt
