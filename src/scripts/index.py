from pandas_datareader import data as pdr
import yfinance as yf
import pandas as pd

path='/stockminer/data/stocks/stockdb/csv/'

ticker_list = ['^GSPC', '^DJI', '^IXIC', '^RUT' ]
for t in ticker_list:
	yf.pdr_override()
	data = pdr.get_data_yahoo(t,period="max")
	data["Date"]=data.index
	data=data.sort_index(axis=0,ascending=False)
	data=data.iloc[0:1764, :-1]
	data=data.sort_index(axis=0,ascending=True)
	data.drop(columns='Adj Close', axis=1, inplace=True);
	data.to_csv(path+t+'.csv', header=False,float_format="%.2f")
