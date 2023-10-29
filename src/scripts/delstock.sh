#!/bin/bash
sed_delete(){
	STR="/"
	STR+=$1
	STR+="/d"
	sed -i "$STR" $2
}
backup(){
	cp /stockminer/data/stocks/LOWCAPS.TXT  /stockminer/backup
	cp /stockminer/data/stocks/HIGHCAPS.TXT /stockminer/backup
}

yahoo_delete(){
	FILE=$2
	STR="s/'"
	STR+=$1
	STR+="',//g"
	sed -i "$STR" $FILE
}

backup

do_lowcaps(){
	if grep -Fxq $1 /stockminer/data/stocks/LOWCAPS.TXT
	then
		echo "excising dead stock from LOWCAPS: " + $1
		sed_delete $1 "/stockminer/data/stocks/LOWCAPS.TXT"
		sed_delete $1 "/stockminer/data/stocks/OPTIONS-LOWCAPS"
	fi
}

do_highcaps(){
	if grep -Fxq $1 /stockminer/data/stocks/HIGHCAPS.TXT
	then
		echo "excising dead stock from HIGHCAPS: " + $1
		sed_delete $1 "/stockminer/data/stocks/HIGHCAPS.TXT"
		sed_delete $1 "/stockminer/data/stocks/OPTIONS-HIGHCAPS"
	fi
}

do_lowcaps  $1
do_highcaps $1
