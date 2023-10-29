import Stockdata
import pdb
from Stockdata import Stockdata
from Stockdata import Analysis
import datetime
from datetime import date
import pandas as pd
import time
import sys

import requests
import concurrent.futures
import cProfile,pstats
import os

#python_src_path=os.path.dirname(os.path.realpath(__file__))
#python_lib_path=python_src_path+"/../stockminer"
#sys.path.append(python_lib_path);
import stockminer
#config=stockminer.init(python_src_path)

#t1 = time.perf_counter()

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
	print("LKJSDLFKJ")
	open("asdf.txt", "a")
	config = stockminer.init()
	ticker_lists = config['ticker_lists']
	for ticker_list in ticker_lists:
		for ticker in ticker_list:
			print(ticker)
			try:
				process(ticker,config)
			except:
				print(ticker + " failed")

if __name__ == "__main__":
	main()

#def main(i):
#    ticks = stockminer.ticker_list[i]
#    for t in ticks:
#        try:
#            process(t)
#        except:
#            print(t + " failed");
#profiler = cProfile.Profile()
#profiler.enable()
#if int(sys.argv[1]) == -1:
#    process(sys.argv[2])
#else:
#    main(int(sys.argv[1]))
#profiler.disable()
#stats = pstats.Stats(profiler).sort_stats('cumtime')
#stats.print_stats()
