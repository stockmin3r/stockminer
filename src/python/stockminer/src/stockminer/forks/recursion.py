import backtest
from backtest import BackTest
from backtest import Analysis
from backtest import Sequences
import datetime
from datetime import date
import pandas as pd
import time
import sys
import requests

t1 = time.perf_counter()

backtest = Sequences()
num_sequences = backtest.num_sequences

stock = Analysis('2020')

workers = int(num_sequences/30)
index = int(sys.argv[1])
start_index = index*workers
end_index = (index+1)*workers

# if end_index > num_sequences:
#	 end_index = num_sequences-1

if end_index > num_sequences:
	end_index = num_sequences

for i in range(start_index, end_index):
	sequence = backtest.sequences[i]
	sequence = [x - 1 for x in sequence]
	stock.Portfolio(i, sequence)

# for i in range(0,num_sequences):
#	 print(i)
#	 sequence = backtest.sequences[i]
#	 sequence = [x-1 for x in sequence]
#	 stock.Portfolio(i, sequence)

t2 = time.perf_counter()
print(f'Finished in {t2 - t1} seconds')
