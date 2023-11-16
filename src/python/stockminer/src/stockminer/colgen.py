import Stockdata
import pdb
import time
import sys
import os
import datetime
import requests
import pandas as pd
import concurrent.futures
from Stockdata import Stockdata
from Stockdata import Analysis
from datetime import date

#python_src_path=os.path.dirname(os.path.realpath(__file__))
#python_lib_path=python_src_path+"/../stockminer"
#sys.path.append(python_lib_path);
import stockminer
#config=stockminer.init(python_src_path)

def process(ticker,config):
	stock = Analysis(ticker,config['csv_path'],config['colgen_path'])
	stock.deltas()
	stock.days(.05)
	stock.days(.1)
	stock.days(.15)
	stock.days(.2)
	stock.days(-.1)
	stock.mag1()
	stock.trend21d()
	stock.RTD()
	stock.streakdir(ticker)
	stock.fib()
	stock.valueatrisk()
	stock.oneyearago()
	stock.oneyearindex()
	stock.oneyear_lag()
	stock.peak()
	stock.peakprice()
	stock.YTD()

	def run(type):
		stock.totalsignals      (21, type)
		stock.totalsignals      (42, type)
		stock.totalsignals      (63, type)
		stock.signalsmaxdays    (21, type, '5%')
		stock.signalsmaxdays    (42, type, '5%')
		stock.signalsmaxdays    (63, type, '5%')
		stock.signalssuccessrate(21, type, '5%')
		stock.signalssuccessrate(42, type, '5%')
		stock.signalssuccessrate(63, type, '5%')
		stock.signalsmaxdays    (21, type, '10%')
		stock.signalsmaxdays    (42, type, '10%')
		stock.signalsmaxdays    (63, type, '10%')
		stock.signalssuccessrate(21, type, '10%')
		stock.signalssuccessrate(42, type, '10%')
		stock.signalssuccessrate(63, type, '10%')

	run('action 1')
	run('action 4')

	stock.nextday()
	stock.oneyearago_percentile()
	stock.oneyearindex_percentile() # '1 yr index P'
	stock.peakP()                   # '1 yr pk Price'
	stock.oneyearP(ticker)          # '1 yr P%'
	stock.limit_and_delta_P(ticker) # Plimit and Pdelta
	stock.p10temppercentpeak()
	stock.p5temppercentpeak()
	stock.last_peak()
	stock.P10orP5()
	stock.peakfm()
	stock.sig(ticker)
	stock.printxls()

def main():
	config = stockminer.init()
	ticker_lists = config['ticker_lists']
	for ticker_list in ticker_lists:
		for ticker in ticker_list:
			try:
				process(ticker,config)
			except:
				pass

if __name__ == "__main__":
	main()
