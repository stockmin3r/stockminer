#!/bin/bash
# obsolete
mkdir /stockminer/backup
PATH="/stockminer/backup"
DATE=`/usr/bin/date +%F`
BACKUP="${PATH}/${DATE}.tar"
echo ${BACKUP}
/usr/bin/tar -cf "${BACKUP}" /stockminer/python/recursion/2020* /stockminer/python/recursion/forks.txt /stockminer/db/1m/* /stockminer/stockdb/csv/*.csv /stockminer/stockdb/mag2/*.csv /stockminer/stockdb/mag2/*.m2 /stockminer/stockdb/mag3/*
