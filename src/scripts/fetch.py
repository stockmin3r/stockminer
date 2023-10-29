from pandas_datareader import data as pdr
import yfinance as yf
import pandas as pd
import sys

path='/stockminer/data/stocks/stockdb/csv/'

print (sys.argv[1]);
yf.pdr_override()
# dataframe
data = pdr.get_data_yahoo(sys.argv[1],period="max")
data["Date"]=data.index
data=data.sort_index(axis=0,ascending=False)
data=data.iloc[0:1764, :-1]
data=data.sort_index(axis=0,ascending=True)
data.drop(columns='Adj Close', axis=1, inplace=True);
data.to_csv(path+sys.argv[1]+'.csv', header=False,float_format="%.3f")
