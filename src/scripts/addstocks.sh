#!/bin/bash
while read -r line; do /stockminer/scripts/addstock.sh $line; done < added-01-22-2022.txt
/stockminer/scripts/sync.sh
