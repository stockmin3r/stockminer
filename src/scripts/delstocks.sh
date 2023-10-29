#!/bin/bash
while read -r line; do /stockminer/scripts/delstock.sh $line; done < $1
/stockminer/scripts/sync.sh
