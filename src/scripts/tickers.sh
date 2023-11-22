#!/bin/bash
# obsolete
HIGHCAPS=`cat /stockminer/data/stocks/HIGHCAPS.TXT  | tr "\n" ","|sed  "s/,/','/g"`
LOWCAPS=`cat /stockminer/data/stocks/LOWCAPS.TXT    | tr "\n" ","|sed  "s/,/','/g"`
STOCKS="tickers=['"
STOCKS+=$HIGHCAPS
STOCKS+=$LOWCAPS
# ALLCAPS
echo "import numpy as np" > /stockminer/python/lib/tickers.py
echo $STOCKS >> /stockminer/python/lib/tickers.py
sed -i "s/','$/'\]/g" /stockminer/python/lib/tickers.py
# END OF FILE
sed -i 's/\./-/g' /stockminer/python/lib/tickers.py
echo "ticker_list = np.array_split(tickers, 32)" >> /stockminer/python/lib/tickers.py

#HIGHCAPS
STOCKS="highcaps=['"
STOCKS+=$HIGHCAPS
echo $STOCKS >> /stockminer/python/lib/tickers.py
sed -i "s/','$/'\]/g" /stockminer/python/lib/tickers.py
sed -i 's/\./-/g' /stockminer/python/lib/tickers.py

#LOWCAPS
STOCKS="lowcaps=['"
STOCKS+=$LOWCAPS
echo $STOCKS >> /stockminer/python/lib/tickers.py
sed -i "s/','$/'\]/g" /stockminer/python/lib/tickers.py
sed -i 's/\./-/g' /stockminer/python/lib/tickers.py
sed -i s'/np-array/np\.array/g' /stockminer/python/lib/tickers.py
