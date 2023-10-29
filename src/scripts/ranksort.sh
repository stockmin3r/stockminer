#!/bin/bash
tr "," "-" </stockminer/python/recursion/2020.csv | sort -n -t "-" -k3 -k4 -k5|gawk -F"-" '{print $1","$2","$3"-"$4"-"$5","$6","$7"-"$8"-"$9","$10}'| sed 's/--//g' | sed 's/Rank.*,,/Rank,Ticker,Entry Date,Entry Price,Exit Date,Exit Price/g'|grep -v ,,,, > /dev/shm/2.csv
mv /dev/shm/2.csv /stockminer/python/recursion/2020.csv
