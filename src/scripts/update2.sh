#!/bin/bash
script_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
root_path=$( cd "$(dirname ../)" ; pwd -P )

/stockminer/admin --update-WSJ
python3 /stockminer/scripts/index.py
sed -i '/,,,,/d' data/stocks/stockdb/csv/*.csv
#/stockminer/admin --ebuild
# mag2
cd /stockminer/python && /stockminer/python/run colgen/colgen.py 2>/dev/shm/colgen
# mag3
# cd /stockminer/python && /stockminer/python/run indi/indi.py 2>/dev/shm/indi
# process mag CSVs into binary structs
cd /stockminer && /stockminer/www --mag 2>/dev/null
# reload stocks with new EOD data
/stockminer/admin --update-EOD
tr "," "-" < /stockminer/python/recursion/2020.csv | sort -n -t "-" -k3 -k4 -k5|gawk -F"-" '{print $1","$2","$3"-"$4"-"$5","$6","$7"-"$8"-"$9","$10}'| sed 's/--//g' | sed 's/Rank.*,,/Rank,Ticker,Entry Date,Entry Price,Exit Date,Exit Price/g'|grep -v ,,,, > /dev/shm/2.csv
mv /dev/shm/2.csv /stockminer/python/recursion/2020.csv
rm -f /stockminer/python/recursion/*.txt
cd /stockminer/python/recursion && python3 single_fork_threaded.py
cd /stockminer/python && /stockminer/python/run recursion/recursion.py 2>/dev/shm/recursion
/stockminer/admin --update-FORKS
#/stockminer/scripts/backup.sh
