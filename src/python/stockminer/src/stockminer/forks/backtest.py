import pandas as pd
from pandas_datareader import data as pdr
import numpy as np
import yfinance
import yfinance as yf
import xlsxwriter
import xlwt
from scipy import stats
from scipy.stats import norm
from datetime import timedelta
import time
import math
import sys
import datetime as dt
import os.path

if os.path.isfile('/production'):
	is_server = True
	fullpath = '/stockminer/python/recursion/'
else:
	is_server = True
	fullpath = 'C:/'

def to_unix(list, special, direction):
	# special gets converted to ''
	if direction == 'tounix':
		if special == 'nan':
			for idx, date in enumerate(list):
				if isinstance(date, str):
					continue
				else:
					if math.isnan(date):
						list[idx] = ''

			return ['' if exit == '' else time.mktime(dt.datetime.strptime(exit, '%Y-%m-%d').timetuple()) for exit in list]
		else:
			return [time.mktime(dt.datetime.strptime(entry, '%Y-%m-%d').timetuple()) for entry in list]

	if direction == 'tohuman':
		# nan is for exit dates
		if special == 'nan':
			return [date if date == '' else dt.datetime.fromtimestamp(int(float(date))).strftime('%Y-%m-%d') for date in list]
		else:
			return [date if date == '' else dt.datetime.fromtimestamp(int(float(date))).strftime('%Y-%m-%d') for date in list]

class Sequences:
	def __init__(self):
		"""
		[1] import sequences from single_fork_threaded.py
		[1, 115, 171, 174, 242, 244, 253, 254, 255, 257, 262, 296, 321, 328, 350, 356]
		[1, 115, 171, 174, 242, 244, 253, 254, 255, 257, 262, 296, 321, 328, 350, 357, 418]
		...
		sequences
		-- [sequence number][buy opportunity row number]
		"""
		with open(fullpath + 'forks.txt') as f:
			sequences = [line.strip('][\n').split(', ') for line in f]
			self.sequences = [list(map(int, line)) for line in sequences]
		self.num_sequences = len(sequences)

class BackTest:
	def __init__(self, filename):
		self.filename         = filename
		df                    = pd.read_csv(fullpath + self.filename + '.csv')
		self.rank_list        = df['Rank'].tolist()
		self.ticker_list      = df['Ticker'].tolist()
		self.entry_price_list = df['Entry Price'].tolist()
		self.exit_price_list  = df['Exit Price'].tolist()
		entry_date_list       = df['Entry Date'].tolist()
		exit_date_list        = df['Exit Date'].tolist()
		self.entry_date_list  = to_unix(entry_date_list, 'none', 'tounix')
		self.exit_date_list   = to_unix(exit_date_list, 'nan', 'tounix')
		self.length           = len(df.index)
		self.xls              = df.copy()

	def number_of_signals(self):
		return self.length

	def printxls(self):
	   # self.xls.to_csv(self.symbol + '_hist.csv', float_format="%.2f")
	   print(self.xls)

	def get_row(self, date):
		# dates, datetime to string
		dateslist = data['Date'].dt.strftime('%Y-%m-%d').tolist()
		return data.loc[[dateslist.index(date)]]

	def get_close_by_index(self, idx):
		return self.xls.loc[[idx]].iloc[0]['Close']

class Analysis(BackTest):
	def Portfolio(self, indexrun, sequence):
		# self.xls is a dataframe of buy opportunities
		rows_with_nan = [index for index, row in self.xls.iterrows() if row.isnull().any()]
		list_of_labels = ['ticker: ', 'qty: ', 'buy amt: ', 'sell amt: ', 'buy date: ', 'sell date: ', 'cash: ', 'capital: ', 'returns: ']

		if sequence[0] in rows_with_nan:
			tickers     = self.ticker_list
			entryprices = self.entry_price_list
			entrydates  = self.entry_date_list
			cash        = 2000
			ticker      = tickers[sequence[0]]
			shares      = int(cash/entryprices[sequence[0]])
			basis       = np.float32(shares*entryprices[sequence[0]])
			cash        = cash - basis
			startdate   = entrydates[sequence[0]]
			startdate   = dt.datetime.fromtimestamp(int(float(startdate))).strftime('%m/%d/%y')
			enddate	    = 'n/a'
			totalcash   = 'n/a'
			returns	    = 'n/a'
			closebasis  = 'n/a'

			singular_records = [ticker,shares,basis,closebasis,startdate,enddate,cash,totalcash,returns]
			with open(fullpath + '2020.txt', 'a') as filehandle:
				filehandle.write('\n' + 'Portfolio No. ' + str(indexrun) + '\n')
				for label, listitem in zip(list_of_labels, singular_records):
					filehandle.write(f"{label}{listitem}\n")
		else:
			# [1] Create lists
			tickers     = self.ticker_list
			entryprices = self.entry_price_list
			exitprices  = self.exit_price_list
			entrydates  = self.entry_date_list
			exitdates   = self.exit_date_list

			# convert nan to blank in exitdates list
			for idx, date in enumerate(exitdates):
				if isinstance(date, str):
					continue
				else:
					if math.isnan(date):
						exitdates[idx] = ''

			cash          = 2000
			sequence_path = sequence # [1, 115, 171, 174, 242, 244, 253, 254, 255, 257, 262, 296, 321, 328, 350, 356]
			footstep      = 0
			row           = sequence[footstep]

			position_rows         = []
			position_shares       = []
			position_cash         = []
			position1_ticker      = tickers[sequence[footstep]]
			position1_shares      = int(cash/entryprices[sequence[footstep]])
			position1_basis       = np.float32(position1_shares*entryprices[sequence[footstep]])
			position1_close_basis = np.float32(position1_shares*exitprices[sequence[footstep]])
			position1_start       = entrydates[sequence[footstep]]
			position1_end         = exitdates[sequence[footstep]]
			cash                  = np.float32(cash - position1_basis)

			position_rows.append(sequence[footstep])
			position_shares.append(position1_shares)
			position_cash.append(np.float32(cash))
			footstep += 1
			# row += 1

			recordticker, recordshares, recordbasis, recordclosebasis, recordstart, recortend, recordcash, recordtotal = ([] for i in range(8))

			# appends values of run to lists
			def status(type):
				recordticker.append(str(position1_ticker))
				recordshares.append(str(position1_shares))
				recordbasis.append(str(np.float32(position1_basis)))
				recordclosebasis.append(str(np.float32(position1_close_basis)))
				recordstart.append(str(position1_start))
				recortend.append(str(position1_end))
				recordcash.append(str(np.float32(cash)))
				if position1_close_basis == "":
					recordtotal.append("")
				else:
					recordtotal.append(np.float32(position1_close_basis + cash))

			status(0)
			record = 0

			for ticker in range(1,len(sequence)): # loop for all signals, first signal already considered
				position1_ticker = tickers[sequence[footstep]]
				position1_shares = int(cash / entryprices[sequence[footstep]])
				position1_basis  = np.float32(position1_shares * entryprices[sequence[footstep]])
				position1_start  = entrydates[sequence[footstep]]

				if exitdates[sequence[footstep]] == "":
					position1_end         = 'in progress'
					position1_close_basis = 0
				else:
					position1_end         = exitdates[sequence[footstep]] # unix dates
					position1_close_basis = np.float32(position1_shares * exitprices[sequence[footstep]])

				if exitdates[position_rows[record]] == '':
					footstep += 1
					continue
				else:
					cash = np.float32(position_cash[record] + exitprices[position_rows[record]]*position_shares[record])

					position1_ticker      = tickers[sequence[footstep]]
					position1_shares      = int(cash / entryprices[sequence[footstep]])
					position1_basis       = np.float32(position1_shares * entryprices[sequence[footstep]])
					position1_close_basis = np.float32(position1_shares * exitprices[sequence[footstep]])
					position1_start       = entrydates[sequence[footstep]]
					cash                  = np.float32(cash - position1_basis)

					position_rows.append(sequence[footstep])
					position_shares.append(position1_shares)
					position_cash.append(np.float32(cash))
					record += 1
					footstep += 1
					status(0)

			# POST - CALCULATIONS
			# [1] INCEPTION RETURN
			returns	= []
			if math.isnan(recordtotal[-1]):
				returns.append(float(recordtotal[-2]) / 2000 - 1)
				unixdiff = (float(recortend[-2])-float(recordstart[0]))/86400

				if unixdiff < 365:
					returns.append('N/A')
				else:
					returns.append((recordtotal[-2]/2000)**(365.25/(unixdiff))-1)
			else:
				returns.append(float(recordtotal[-1]) / 2000 - 1)

			# [2] APY
			# PERCENT OF STOCKS IN PROGRESS FROM FIRST ENTRY
			first_entry_stuck = round(len(rows_with_nan)/len(tickers)*100,2)

			# [3] PRINT RECORDS TO TXT FILE
			recordstart   = [date if date == '' else dt.datetime.fromtimestamp(int(float(date))).strftime('%m/%d/%y') for date in recordstart]
			recortend     = [date if date == 'in progress' else dt.datetime.fromtimestamp(int(float(date))).strftime('%m/%d/%y') for date in recortend]
			list_of_lists = [recordticker, recordshares, recordbasis,recordclosebasis, recordstart, recortend, recordcash, recordtotal, returns]

			with open(fullpath + '2020.txt','a') as filehandle:
				# if startrowinput == 0:
				#	 filehandle.write(str(first_entry_stuck) + '% of portfolios in-progress from first entry: ')
				#	 filehandle.write('\n')
				#	 filehandle.write(str(rows_with_nan))
				#	 filehandle.write('\n')
				filehandle.write('\n' + 'Portfolio No. ' + str(indexrun) + '\n')
				for label, listitem in zip(list_of_labels, list_of_lists):
					filehandle.write(f"{label}{listitem}\n")
