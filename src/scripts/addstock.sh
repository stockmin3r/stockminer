#!/bin/bash
TICKER=$1
python3 /stockminer/scripts/fetch.py $1
python3 /stockminer/scripts/earn.py $1
STOCK="/stockminer/data/stocks/stockdb/"
STOCK+=$1
STOCK+=".edates"
sed -i '/report_date/d' $STOCK
python3 /stockminer/python/colgen/colgen.py -1 $1
/stockminer/stockminer -mag $1
/stockminer/stockminer -d $1
