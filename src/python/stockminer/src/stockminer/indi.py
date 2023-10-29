import pandas as pd
from pandas_datareader import data as pdr
import numpy as np
import yfinance
import yfinance as yf
from scipy import stats
from scipy.stats import norm
from ta import add_all_ta_features
import datetime
from datetime import date
import pandas as pd
import time
import sys

python_src_path=os.path.dirname(os.path.realpath(__file__))
import tickers

t1 = time.perf_counter()

class Indicator:
	def __init__(self, ticker):
		self.symbol      = ticker
		self.xls         = pd.read_csv('/stockminer/stockdb/csv/' + self.symbol + '.csv', names=['Date','Open','High','Low','Close', 'Volume'])
		self.xls         = add_all_ta_features(self.xls, open="Open", high="High", low="Low", close="Close", volume="Volume")
		self.xls.replace('0', np.nan)
		self.xls.fillna(0, inplace=True)
		self.xls.drop(['Date','Open', 'High', 'Low', 'Close', 'Volume'], axis=1, inplace=True)


def main(i):
	ticks = tickers.ticker_list[i]
	for ticker in ticks:
		indi = Indicator(ticker)
		indi.xls.to_csv("/stockminer/stockdb/mag3/" + indi.symbol + ".csv", float_format="%.3f", header=False, index=False)

main(int(sys.argv[1]))
