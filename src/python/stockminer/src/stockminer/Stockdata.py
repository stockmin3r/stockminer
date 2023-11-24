#########################
# Sarah's Stockdata     #
# License: Public Domain#
#########################
import pandas as pd
from pandas_datareader import data as pdr
import numpy as np
import yfinance
import yfinance as yf
import xlsxwriter
import xlwt
import pdb
from scipy import stats
from scipy.stats import norm
import datetime
import holidays
from dateutil.relativedelta import relativedelta
import datedelta
import time
from bisect import bisect_left
class Stockdata:
	def __init__(self,ticker,csv_path,colgen_path):
		self.symbol      = ticker
		self.colgen_path = colgen_path
		self.xls         = pd.read_csv(csv_path + '/' + self.symbol + '.csv', names=['Date','Open','High','Low','Close', 'Volume'])
		self.xls['Date'] = pd.to_datetime(self.xls['Date'])
		self.last_close  = self.xls.loc[self.xls.index[-1], 'Close']

	def printxls(self):
		self.xls.replace(r'^\s*$', np.nan, regex=True, inplace=True)
		self.xls.replace('0', np.nan)
		self.xls.fillna(0, inplace=True)
		self.xls.drop(['1 yr pk % price old','# days -10%','return -10%','max -10%','Date', 'Open', 'High', 'Low', 'Close', 'Volume', 'next Open%', 'next Daily%','next Close%', 'Open%', 'Daily%','Close%','mag1','one','mag2','two','1 yr index', '1 yr index - 21','1 yr index - 42','1 yr index - 63', 'day_of_week','next day','1 yr ago P','1 yr index P','1 yr P%','nxtday_month','nxtday_day','nxtday_year'], axis=1, inplace=True)
		self.xls.to_csv(self.colgen_path + '/' + self.symbol + '.csv', float_format="%.3f", index=False, header=False)

	def get_row(self, date):
		dateslist = self.xls['Date'].dt.strftime('%Y-%m-%d').tolist()
		return self.xls.loc[[dateslist.index(date)]]

	def get_close_by_index(self, idx):
		return self.xls.loc[[idx]].iloc[0]['Close']

	# returns list of close prices from start index to end index
	def get_close_by_range(self, start, end):
		closeprices = self.xls['Close'].tolist()
		if type(start) == list or type(end) == list:
			# list_of_close_prices = []
			return []
		else:
			if start > end:
				start, end = end, start
			# list_of_close_prices = self.xls.iloc[start:end+1]['Close'].tolist()
			return closeprices[start:end + 1]

		# return list_of_close_prices

	def get_max_by_range(self, start, end):
		# returns maximum close price in region bounded
		# by start index to end index
		temp = self.get_close_by_range(start, end)
		if not temp:
			return "empty"
		else:
			return max(temp)

	def get_signals_by_range(self, start, end, column, filter1):
		# returns list of type string with: 'pot' and ''
		# from start index to end index
		# element is str
		list_of_a1 = self.xls[str(column)].tolist()
		atpeaklist = self.xls[str(filter1)].tolist()

		if type(start) == list or type(end) == list:
			list_of_a1 = []
		elif start < 0 or end < 0:
			list_of_a1 = []
		else:
			if start > end:
				start, end = end, start

			list_of_a1 = list_of_a1[start:end + 1]
			atpeaklist = atpeaklist[start:end + 1]

			for index, item in enumerate(atpeaklist):
				if item == 0.0:
					list_of_a1[index] = ''

		return list_of_a1

	def get_signals_by_range2(self, start, end, column, filter1, filter2):
		# returns list of type string with: 'pot' and ''
		# from start index to end index
		# element is str
		list_of_a1 = self.xls[str(column)].tolist()
		atpeaklist = self.xls[str(filter1)].tolist()
		maxdays = self.xls[str(filter2)].tolist()

		if type(start) == list or type(end) == list:
			list_of_a1 = []
		elif start < 0 or end < 0:
			list_of_a1 = []
		else:
			if start > end:
				start, end = end, start

			list_of_a1 = list_of_a1[start:end + 1]
			atpeaklist = atpeaklist[start:end + 1]

			for index, item in enumerate(atpeaklist):
				if item == 0.0:
					list_of_a1[index] = ''

			# 'max 5%'
			maxdays = maxdays[start:end + 1]
			for index, item in enumerate(list_of_a1):
				if item != 'pot':
					maxdays[index] = 0

		return maxdays

	def get_signals_by_range3(self, start, end, column, filter1, filter2):
		# returns list of type int with: '# days 5%'
		# from start index to end index
		# element is int
		list_of_a1 = self.xls[str(column)].tolist()
		atpeaklist = self.xls[str(filter1)].tolist()
		days = self.xls[str(filter2)].tolist()

		if type(start) == list or type(end) == list:
			list_of_a1 = []
		elif start < 0 or end < 0:
			list_of_a1 = []
		else:
			if start > end:
				start, end = end, start
			# end should be +1? for iloc
			list_of_a1 = list_of_a1[start:end + 1]
			atpeaklist = atpeaklist[start:end + 1]

			for index, item in enumerate(atpeaklist):
				if item == 0.0:
					list_of_a1[index] = ''

			# '# days 5%'
			days = days[start:end + 1]
			for index, item in enumerate(list_of_a1):
				if item != 'pot':
					days[index] = 1000 # so that it fails to be counted

		return days

	def take_closest(self, myList, myNumber):
		"""
		Assumes myList is sorted. Returns closest value to myNumber.

		If two numbers are equally close, return the smallest number.
		"""
		pos = bisect_left(myList, myNumber)  # position to insert myNumber to keep list sorted

		if pos == 0:
			return myList.index(myList[0])
		if pos == len(myList):
			return myList.index(myList[-1])
		before = myList.index(myList[pos - 1])
		after = myList.index(myList[pos])

		if after - myNumber < myNumber - before:
			return after
		else:
			return before

###########################
#
#	 CLASS: ANALYSIS
#	 ---------------
#
###########################
class Analysis(Stockdata):

	def get_close_by_index(self, idx):
		return super().get_close_by_index(idx)

	# open%, daily%, close%, next open%, next daily%, next close%
	def deltas(self):
		self.xls['Open%']	    = (self.xls['Open']  / self.xls['Close'].shift(1) - 1) * 100
		self.xls['Daily%']	    = (self.xls['Close'] / self.xls['Open']           - 1) * 100
		self.xls['Close%']	    = (self.xls['Close'] / self.xls['Close'].shift(1) - 1) * 100
		self.xls['next Open%']  =  self.xls['Open%'].shift(-1)
		self.xls['next Daily%'] =  self.xls['Daily%'].shift(-1)
		self.xls['next Close%'] =  self.xls['Close%'].shift(-1)
		return self.xls

	# mag1, one, mag2, two, a1 ESP, action 1, a1 same, a1 ESP result, a4 ESP, action 4
	def mag1(self):
		self.xls['mag1'] = np.where(self.xls['Close'] >= self.xls['Close'].shift(1), 1, -1)
		self.xls['mag1'] = pd.to_numeric(self.xls["mag1"])
		self.xls['mag1'] = self.xls['mag1'].replace([-1], 0)
		self.xls['one']  = self.xls['mag1'].rolling(15).sum()

		self.xls['mag2']  = np.where(self.xls['Open'] >= self.xls['Close'].shift(1), 1, 0) - np.where(
		self.xls['Open']  < self.xls['Close'].shift(1), 1, 0) + np.where(self.xls['Close'] >= self.xls['Open'],1,0) - np.where(
		self.xls['Close'] < self.xls['Open'], 1, 0)

		self.xls['mag2']  = pd.to_numeric(self.xls["mag2"])
		self.xls['mag2']  = self.xls['mag2'].replace([-2], 0)
		self.xls['two']   = (self.xls['mag2'].rolling(15).sum()) / 2

		self.xls['a1 ESP']   = np.where((self.xls['two'] == 3) & (self.xls['mag2'].shift(14) == 2), "no 2", "")
		self.xls['action 1'] = np.where((self.xls['two'] <= 2), "pot", "")

		self.xls['a1 same']  = np.where((self.xls['two'] == 0) | (self.xls['two'] == 1), "stay",
									   np.where((self.xls['two'] == 2), np.where(
										   (self.xls['mag2'].shift(14) == 0) | (self.xls['mag2'].shift(14) == -2), "no 2", "stay"), ""))

		self.xls['a1 ESP result'] = np.where((self.xls['a1 ESP'] != ""),
											 np.where((self.xls['action 1'].shift(-1) == "pot"), "action",
													  np.where((self.xls['a1 ESP'].shift(-1) != ""), "ESP", "none")), "")

		self.xls['a4 ESP'] = np.where(
		(self.xls['one'] >= 10) & (self.xls['mag2'].shift(14) == 2) & (self.xls['two'] == 4), "no 2",
			np.where((self.xls['one'] == 9) & (self.xls['mag1'].shift(14) == -1) & (
					self.xls['mag2'].shift(14) == 2) & (self.xls['two'] == 4), "no 2", ""))

		self.xls['action 4'] = np.where((self.xls['one'] >= 9) & (self.xls['two'] <= 3), "pot", "")
		return self.xls

	# universal # days, return, max
	def days(self, pt):
		# percent gain in decimal form
		ptdecimal = pt
		# label for headers
		labelp = ' ' + str(int(round(pt * 100, 0))) + '%'
		# gain multiplier
		pt = 1 + ptdecimal
		self.xls['nearest_pt' + labelp] = self.xls['Close'] * pt

		maxindex = len(self.xls) - 1

		# indices = []
		# closeabovept = []
		indices, closeabovept = ([] for i in range(2))

		for row in range(len(self.xls)):
			# nearest value to pt
			x = self.xls['Close'].sub(self.xls['nearest_pt' + labelp][row]).abs().idxmin()
			# all indexes greater than pt, checks all rows, we only want to start at current row
			y = self.xls[self.xls['Close'].gt(self.xls['nearest_pt' + labelp][row])].index

			try:
				# first indexes of value greater than pt
				index = next(x for x, val in enumerate(y) if val > row)
				# first close price greater than pt
				output = y[index]
				closeabove = self.xls['Close'][output]
			except:
				output = maxindex
				closeabove = self.xls['Close'][maxindex]

			indices.append(output) # idx column
			closeabovept.append(closeabove) # close % column

		self.xls['original_index' + labelp] = self.xls.index
		self.xls['idx'	  + labelp]  = indices
		self.xls['close'	+ labelp]  = closeabovept
		self.xls['# days'   + labelp]  = np.where(
			(100 * (self.xls['close' + labelp] / self.xls['Close'] - 1)) >= ((pt - 1) * 100),
			(self.xls['idx' + labelp] - self.xls.index), '')
		self.xls[['# days'  + labelp]] = self.xls[['# days' + labelp]].apply(pd.to_numeric)
		self.xls['return'   + labelp]  = 100 * (self.xls['close' + labelp] / self.xls['Close'] - 1)

		# self.xls['max' + labelp] = self.xls['idx' + labelp] - self.xls.index
		# above was changed to the npwhere version below

		# self.xls['max' + labelp] = np.where(
		#	 (100 * (self.xls['close' + labelp] / self.xls['Close'] - 1)) >= ((pt - 1) * 100),
		#	 (self.xls['idx' + labelp] - self.xls.index), (self.xls['idx' + labelp] - self.xls.index - 3))

		self.xls['max' + labelp] = np.where(
			(100 * (self.xls['close' + labelp] / self.xls['Close'] - 1)) >= ((pt - 1) * 100),
			(self.xls['idx' + labelp] - self.xls.index), (self.xls['idx' + labelp] - self.xls.index))

		labels = [str('original_index' + labelp), str('idx' + labelp), str('close' + labelp),
				  str('nearest_pt' + labelp)]

		self.xls.drop(labels, axis=1, inplace=True)

		# label = str('original_index' + str(pt))
		# self.xls.drop(label, axis=1, inplace=True)
		return self.xls

	# 1d, 3d, 5, 8d, 13d, 21d, 42d, 63d
	def trend21d(self):
		trend_list = [1, 3, 5, 8, 13, 21, 42, 63]
		for int in trend_list:
			self.xls[str(int) + 'd'] = 100 * (self.xls['Close'].pct_change(int))
		return self.xls

	# RTD
	def RTD(self):
		self.xls['RTD'] = (self.xls['Close'].iloc[-1] / self.xls['Close'] - 1) * 100
		return self.xls

	# YTD, dependent of oneyearindex()
	def YTD(self):
		closelist = self.xls['Close'].tolist()
		oneyridx = self.xls['1 yr index'].tolist()
		ytdlist = []

		for index, item in enumerate(oneyridx):
			# item is 1 year ago index
			# index is current row

			if item < 0:
				# maximum = ""
				ytdlist.append("")
			else:
				yragoclose = self.get_close_by_index(item)
				ytdlist.append(100 * (closelist[index] / yragoclose - 1))

		self.xls['YTD'] = ytdlist

		return self.xls

	# mean, std_dev, VaR_90, VaR_95, VaR_99
	def valueatrisk(self):

		self.xls['mean'] = self.xls['Close%'].rolling(252).mean()
		self.xls['std_dev'] = self.xls['Close%'].rolling(252).std()
		self.xls['VaR_90'] = norm.ppf(1 - 0.9, self.xls['mean'], self.xls['std_dev'])
		self.xls['VaR_95'] = norm.ppf(1 - 0.95, self.xls['mean'], self.xls['std_dev'])
		self.xls['VaR_99'] = norm.ppf(1 - 0.99, self.xls['mean'], self.xls['std_dev'])
		self.xls['VaR_99pot'] = np.where((self.xls['Close%']) <= (self.xls['VaR_99']),"pot","")
		return self.xls

	# 1 year ago date, in some cases these is 1 less than the template
	def oneyearago(self):
		datelist = self.xls['Date'].tolist()
		updateddates = []
		for i in datelist:
			updateddates.append(i - datedelta.YEAR + datedelta.DAY)
		self.xls['1 yr ago'] = updateddates

		# days_per_year = 365.24 - 1
		#
		# self.xls['1 yr ago'] = (self.xls['Date'] - datetime.timedelta(days=days_per_year)).dt.strftime('%Y-%m-%d')
		return self.xls

	# 1 year index
	def oneyearindex(self):

		temp = pd.DataFrame()
		temp['Date'] = self.xls['Date'].dt.strftime('%Y-%m-%d')  # datetime to string
		temp['1 yr ago'] = self.xls['1 yr ago']
		temp['1 yr ago'] = self.xls['1 yr ago'].dt.strftime('%Y-%m-%d')  # datetime to string

		dateslist = temp['Date'].tolist()
		# convert to unix time
		dateslist = [time.mktime(datetime.datetime.strptime(date, "%Y-%m-%d").timetuple()) for date in dateslist]

		searchlist = temp['1 yr ago'].tolist()  # df column to list, this has been to_pydatetime already
		searchlist = [time.mktime(datetime.datetime.strptime(searchvalue, "%Y-%m-%d").timetuple()) for searchvalue in searchlist]
		# idx = dateslist.index(date) # index of search value within list

		def take_closest(myList, myNumber):
			"""
			Assumes myList is sorted. Returns closest value to myNumber.

			If two numbers are equally close, return the smallest number.
			"""
			pos = bisect_left(myList, myNumber)  # position to insert myNumber to keep list sorted

			if pos == 0:
				return myList.index(myList[0])
			if pos == len(myList):
				return myList.index(myList[-1])
			before = myList.index(myList[pos - 1])
			after = myList.index(myList[pos])

			if after - myNumber < myNumber - before:
				return after
			else:
				return before

		yrindex = []
		# for searchvalue in (searchlist):
		#	 if searchvalue in dateslist:
		#		 yrindex.append(take_closest(dateslist,searchvalue))
		#	 else:
		#		 yrindex.append(-1)
		for searchvalue in (searchlist):
			yrindex.append(take_closest(dateslist,searchvalue))

		self.xls['1 yr index'] = yrindex

		return self.xls

	# # 1 year index
	# def oneyearindex(self):
	#
	#	 temp = pd.DataFrame()
	#	 temp['Date'] = self.xls['Date'].dt.strftime('%Y-%m-%d')  # datetime to string
	#	 temp['1 yr ago'] = self.xls['1 yr ago']
	#	 # temp['1 yr ago'] = self.xls['1 yr ago'].dt.strftime('%Y-%m-%d')  # datetime to string
	#
	#	 dateslist = temp['Date'].tolist()
	#	 searchlist = temp['1 yr ago'].tolist()  # df column to list, this has been to_pydatetime already
	#	 # idx = dateslist.index(date) # index of search value within list
	#
	#	 yrindex = []
	#	 for index, searchvalue in enumerate(searchlist):
	#		 if searchvalue in dateslist:
	#			 yrindex.append(dateslist.index(searchvalue))
	#		 else:
	#			 # tempobj = datetime.datetime.strptime(searchvalue, '%Y-%m-%d')
	#			 # newvalue = tempobj + datetime.timedelta(days=1)
	#			 # newvalue = searchvalue + datetime.timedelta(days=1)
	#			 newvalue = searchvalue + datedelta.DAY
	#			 # newvalue = newvalue.date()
	#			 newvalue = newvalue.date().strftime('%Y-%m-%d')
	#
	#			 if newvalue in dateslist:
	#				 yrindex.append(dateslist.index(newvalue))
	#			 else:
	#				 # tempobj2 = datetime.datetime.strptime(searchvalue, '%Y-%m-%d')
	#				 # newvalue2 = tempobj2 + datetime.timedelta(days=2)
	#				 newvalue2 = searchvalue + datetime.timedelta(days=2)
	#				 newvalue2 = newvalue2.date().strftime('%Y-%m-%d')
	#
	#				 if newvalue2 in dateslist:
	#					 yrindex.append(dateslist.index(newvalue2))
	#				 else:
	#					 # tempobj3 = datetime.datetime.strptime(searchvalue, '%Y-%m-%d')
	#					 # newvalue3 = tempobj3 + datetime.timedelta(days=3)
	#					 newvalue3 = searchvalue + datetime.timedelta(days=3)
	#					 newvalue3 = newvalue3.date().strftime('%Y-%m-%d')
	#
	#					 if newvalue3 in dateslist:
	#						 yrindex.append(dateslist.index(newvalue3))
	#					 else:
	#						 # tempobj4 = datetime.datetime.strptime(searchvalue, '%Y-%m-%d')
	#						 # newvalue4 = tempobj4 + datetime.timedelta(days=4)
	#						 newvalue4 = searchvalue + datetime.timedelta(days=4)
	#						 newvalue4 = newvalue4.date().strftime('%Y-%m-%d')
	#
	#						 if newvalue4 in dateslist:
	#							 yrindex.append(dateslist.index(newvalue4))
	#						 else:
	#							 # yrindex.append(0)
	#							 yrindex.append(-1)
	#
	#	 self.xls['1 yr index'] = yrindex
	#
	#	 return self.xls

	# 1 year index - 21, -42, -63
	def oneyear_lag(self):

		self.xls['1 yr index - 21'] = self.xls['1 yr index'].shift(21)
		self.xls['1 yr index - 42'] = self.xls['1 yr index'].shift(42)
		self.xls['1 yr index - 63'] = self.xls['1 yr index'].shift(63)
		self.xls['1 yr index - 21'].fillna(-1, inplace=True)
		self.xls['1 yr index - 42'].fillna(-1, inplace=True)
		self.xls['1 yr index - 63'].fillna(-1, inplace=True)
		self.xls['1 yr index - 21'] = self.xls['1 yr index - 21'].astype(int)
		self.xls['1 yr index - 42'] = self.xls['1 yr index - 42'].astype(int)
		self.xls['1 yr index - 63'] = self.xls['1 yr index - 63'].astype(int)

		return self.xls

	# 1 year pk %, dependent of oneyearindex()
	def peak(self):
		closelist = self.xls['Close'].tolist()
		oneyridx = self.xls['1 yr index'].tolist()
		peaklist = []

		for index, item in enumerate(oneyridx):
			# item is 1 year ago index
			# index is current row

			if item < 0:
				# maximum = ""
				# offpeak = ""
				peaklist.append("")
			else:
				maximum = self.get_max_by_range(item, index)
				# offpeak = 100 * (closelist[index] / maximum - 1)
				peaklist.append(100 * (closelist[index] / maximum - 1))

			# peaklist.append(offpeak)

		self.xls['1 yr pk %'] = peaklist
		return self.xls

	def peakprice(self):
		closelist = self.xls['Close'].tolist()
		oneyridx  = self.xls['1 yr index'].tolist()
		peaklist  = []

		for index, item in enumerate(oneyridx):
			# item is 1 year ago index
			# index is current row
			if item < 0:
				# maximum = ""
				# offpeak = ""
				peaklist.append("")
			else:
				maximum = self.get_max_by_range(item, index)
				# offpeak = 100 * (closelist[index] / maximum - 1)
				peaklist.append(maximum)

			# peaklist.append(offpeak)
		self.xls['1 yr pk % price old'] = peaklist
		return self.xls

	# statistic for a1, a1 success rate
	def signalssuccessrate(self, offset, actiontype, benchmark):
		oneyearindex_list  = self.xls['1 yr index - ' + str(offset)].tolist()
		total_signals_list = self.xls['# signals {} {}'.format(offset, actiontype)].tolist()
		total21            = []
		total21_row        = ""

		for index, item in enumerate(oneyearindex_list):
			if item < 0:
				total21_row = ""
				total21.append(total21_row)
			else:
				list_days = self.get_signals_by_range3(index - offset, item, actiontype, '1 yr pk %', '# days ' + str(benchmark))

				count = 0
				for i in list_days:
					if i < offset:
						count += 1
				if count == 0:
					# rate = ""
					total21.append("")
				else:
					# rate = 100*(count/total_signals_list[index])
					total21.append(100*(count/total_signals_list[index]))

				# total21.append(rate)

		self.xls['success rate {} {} {}'.format(benchmark, offset, actiontype)] = total21
		return self.xls

	# statistic for a1, a1 max days
	def signalsmaxdays(self, offset, actiontype, benchmark):
		oneyearindex_list = self.xls['1 yr index - ' + str(offset)].tolist()
		total21           = []
		total21_row       = ""

		for index, item in enumerate(oneyearindex_list):
			if item < 0:
				# total21_row = ""
				# total21.append(total21_row)
				total21.append("")
			else:
				# list_maxdays = self.get_signals_by_range2(index - offset, item, actiontype, '1 yr pk %', 'max ' + str(benchmark))
				# total21.append(max(list_maxdays))
				total21.append(max(self.get_signals_by_range2(index - offset, item, actiontype, '1 yr pk %', 'max ' + str(benchmark))))

		self.xls['max days {} {} {}'.format(benchmark, offset, actiontype)] = total21
		return self.xls

	# statistic for a1, a1 total signals
	def totalsignals(self, offset, actiontype):
		oneyearindex_list = self.xls['1 yr index - ' + str(offset)].tolist()
		total21           = []
		total21_row       = ""

		for index, item in enumerate(oneyearindex_list):
			if item < 0:
				# total21_row = ""
				# total21.append(total21_row)
				total21.append("")
			else:
				# list_signals = self.get_signals_by_range(index-offset,item, actiontype,'1 yr pk %')
				# total21_row = list_signals.count('pot')
				# total21.append(total21_row)
				total21.append(self.get_signals_by_range(index-offset,item, actiontype,'1 yr pk %').count('pot'))

		self.xls['# signals {} {}'.format(offset, actiontype)] = total21
		return self.xls

	# NOT DONE cat, dependent of oneyearindex()
	# wait, status if a signal has not been
	# given enough time to play out,
	# was not implemented
	def cat(self):
		self.xls['1 yr pk %'] = pd.to_numeric(self.xls['1 yr pk %'])
		self.xls['cat'] = np.where(self.xls['1 yr pk %'] == 0,
								   'peak',
						np.where(self.xls['1 yr pk %'] <= -20,
								 'bear',
						np.where(self.xls['# days 5%'] <= 63,
								 '1Q',
						np.where(self.xls['# days 5%'] <= 126,
								 '2Q',
						np.where(self.xls['# days 5%'] <= 190,
								 '3Q',
						np.where(self.xls['# days 5%'] <= 253,
								 '4Q', 'fail'))))))
		return self.xls

	def get_row(self, date):
		return super().get_row(date)

	def return_long(self, start, end):
		x = super().get_row(start)
		y = super().get_row(end)

		return round((y.iloc[0]['Close'] / x.iloc[0]['Close'] - 1) * 100, 3)

	# 21d trend (-2%, +3%)
	def streakdir(self, t):
		self.xls['streak'] = np.where(self.xls['Close'].shift(1).rolling(21).min() >= self.xls['Close'],
									  100 * (self.xls['Close'] / (self.xls['Close'].shift(1).rolling(21).min()) - 1),
									  np.where(self.xls['Close'].shift(1).rolling(21).max() <= self.xls['Close'],
											   100 * (self.xls['Close'] / (
												   self.xls['Close'].shift(1).rolling(21).max()) - 1), ""))

		self.xls['dir'] = np.where(self.xls['Close'].rolling(21).min() >= self.xls['Close'],
								   "0.0",
								   np.where(self.xls['Close'].rolling(21).max() <= self.xls['Close'],"1.0", "2.0"))

		self.xls['streak']                = pd.to_numeric(self.xls["streak"])
		self.xls['buy(streakdir)']        = 0.98 *  self.xls['Close'].rolling(21).min()
		self.xls['buy delta(streakdir)']  = 100  * (self.xls['buy(streakdir)'] / self.xls['Close'] - 1)
		self.xls['sell(streakdir)']       = 1.03 *  self.xls['Close'].rolling(21).max()
		self.xls['sell delta(streakdir)'] = 100  * (self.xls['sell(streakdir)'] / self.xls['Close'] - 1)

		return self.xls

	# Fibonacci trend (+/- 5%)
	def fib(self):

		self.xls['temp3']   = self.xls['Close'].shift(3)
		self.xls['temp5']   = self.xls['Close'].shift(5)
		self.xls['temp8']   = self.xls['Close'].shift(8)
		self.xls['temp13']  = self.xls['Close'].shift(13)
		self.xls['temp21']  = self.xls['Close'].shift(21)
		self.xls['tempmin'] = self.xls[['temp3', 'temp5', 'temp8', 'temp13', 'temp21']].min(axis=1)
		self.xls['tempmax'] = self.xls[['temp3', 'temp5', 'temp8', 'temp13', 'temp21']].max(axis=1)
		self.xls['temp2']   = self.xls['Close'].shift(2)
		self.xls['temp4']   = self.xls['Close'].shift(4)
		self.xls['temp7']   = self.xls['Close'].shift(7)
		self.xls['temp12']  = self.xls['Close'].shift(12)
		self.xls['temp20']  = self.xls['Close'].shift(20)
		self.xls['tempminesp'] = self.xls[['temp2', 'temp4', 'temp7', 'temp12', 'temp20']].min(axis=1)
		self.xls['tempmaxesp'] = self.xls[['temp2', 'temp4', 'temp7', 'temp12', 'temp20']].max(axis=1)

		self.xls['fib'] = np.where((self.xls['tempmin'] >= self.xls['Close']),
								   100*(self.xls['Close'] / self.xls['tempmin'] - 1),
								   np.where((self.xls['tempmax'] <= self.xls['Close']),
											100*(self.xls['Close'] / self.xls['tempmax'] - 1), ""))

		self.xls['fib'] = pd.to_numeric(self.xls["fib"])
		self.xls['dir(fib)'] = np.where(
			(self.xls['tempmin'] >= self.xls['Close']),
			"0.0",
			np.where(
				(self.xls['tempmax'] <= self.xls['Close']),
				"1.0",
				"2.0"))

		self.xls['buy(fib)'] = 0.95 * self.xls['tempminesp']
		self.xls['buy delta(fib)'] = 100 * (self.xls['buy(fib)'] / self.xls['Close'] - 1)

		self.xls['sell(fib)'] = 1.05 * self.xls['tempmaxesp']
		self.xls['sell delta(fib)'] = 100 * (self.xls['sell(fib)'] / self.xls['Close'] - 1)

		columnlist = ['temp3', 'temp5', 'temp8', 'temp13', 'temp21', 'temp2', 'temp4', 'temp7', 'temp12', 'temp20',
					  'tempminesp', 'tempmaxesp', 'tempmin', 'tempmax']

		self.xls.drop(columnlist, axis=1, inplace=True)

		return self.xls

	# remove pot by criteria
	def pot_override(self):

		filter1 = 'VaR_99'
		filter2 = 'VaR_95'
		filter3 = 'VaR_90'

		self.xls['past_99'] = self.xls['Close%'] - self.xls[filter1]
		self.xls['past_95'] = self.xls['Close%'] - self.xls[filter2]
		self.xls['past_90'] = self.xls['Close%'] - self.xls[filter3]

		removal_list = ['a1 ESP', 'action 1', 'a1 same', 'a1 ESP result', 'a4 ESP', 'action 4']

		for column in removal_list:
			# check if daily move was worse than the value at risk 90%
			self.xls[str(column)] = np.where(self.xls['past_99'] < 0, "", self.xls[str(column)])
			# self.xls[str(column)] = np.where(self.xls['past_95'] < 0, "", self.xls[str(column)])
			# self.xls[str(column)] = np.where(self.xls['past_90'] < 0, "", self.xls[str(column)])
		return self.xls

	# next day
	def nextday(self):
		# ONE_DAY  = datetime.timedelta(days=1)
		# HOLIDAYS_US = holidays.US()
		# def next_buisness_day():
		#	 next_day = (self.xls['Date'] + ONE_DAY).dt.strftime('%Y-%m-%d')
		#	 next_day = next_day.to_datetime(self.xls['next day'], format = '%Y-%m-%d')
		#	 while next_day.weekday() in holidays.WEEKEND or next_day in HOLIDAYS_US:
		#		 next_day += ONE_DAY
		#	 return next_day.dt.strftime('%Y-%m-%d')

		# self.xls['next day'] = self.xls['Date'].apply(next_buisness_day())
		self.xls['day_of_week'] = self.xls['Date'].dt.day_name()
		self.xls['next day'] = np.where(self.xls['day_of_week'] == 'Friday',
							(self.xls['Date'] + datetime.timedelta(days=3)).dt.strftime('%Y-%m-%d'),
							(self.xls['Date'] + datetime.timedelta(days=1)).dt.strftime('%Y-%m-%d'))
		# self.xls['next day'] = (self.xls['Date'] + datetime.timedelta(days=1)).dt.strftime('%Y-%m-%d')
		# if FRIDAY, add 3 days
		self.xls['next day'] = pd.to_datetime(self.xls['next day'], format = '%Y-%m-%d')
		return self.xls

	# 1 year ago date, in some cases these is 1 less than the template
	def oneyearago_percentile(self):
		self.xls['nxtday_month'] = pd.DatetimeIndex(self.xls['next day']).month
		self.xls['nxtday_day'] = pd.DatetimeIndex(self.xls['next day']).day
		self.xls['nxtday_year'] = pd.DatetimeIndex(self.xls['next day']).year-1

		# days_per_year = 365.24 - 2
		days_per_year = 365.25 - 1 # subtract 1 year, add 1 day
		days_per_year_leap = 365.25  # subtract 1 year
		# self.xls['1 yr ago P'] = np.where((self.xls['nxtday_month'] == 2) & (self.xls['nxtday_day'] == 29),
		#						  (self.xls['next day'] - datetime.timedelta(days=days_per_year_leap)).dt.strftime('%Y-%m-%d'),
		#						  (self.xls['next day'] - datetime.timedelta(days=days_per_year)).dt.strftime('%Y-%m-%d'))

		# self.xls['1 yr ago P364'] = (self.xls['next day'] - datetime.timedelta(days=364)).dt.strftime('%Y-%m-%d')
		# self.xls['1 yr ago P365'] = (self.xls['next day'] - datetime.timedelta(days=365)).dt.strftime('%Y-%m-%d')

		# self.xls['newdatetemp'] = datetime.date(self.xls['nxtday_year'],self.xls['nxtday_month'],self.xls['nxtday_day'])

		nextdaylist = self.xls['next day'].tolist()

		# convert text dates to datetime
		nextdaylist2 = []
		for i in nextdaylist:
			nextdaylist2.append(i.to_pydatetime())

		# subtract a year, add a day
		nextdaylist3 = []
		for i in nextdaylist2:
			nextdaylist3.append(i - datedelta.YEAR + datedelta.DAY)


		self.xls['1 yr ago P'] = nextdaylist3
		# self.xls['1 yr ago new'] = (self.xls['newdatetemp'] + datetime.timedelta(days=1)).dt.strftime('%Y-%m-%d')

		# self.xls['1 yr ago P'] = np.where((self.xls['nxtday_month'] == 2) & (self.xls['nxtday_day'] == 29),
		#								   self.xls['1 yr ago P'],
		#								   (self.xls['next day'] + datetime.timedelta(days=1)).dt.strftime(
		#									   '%Y-%m-%d'))
		# self.xls['1 yr ago P'] = (self.xls['next day'] - datetime.timedelta(days=days_per_year)).dt.strftime('%Y-%m-%d')
		return self.xls

	# 1 year index
	def oneyearindex_percentile(self):
		temp = pd.DataFrame()
		temp['Date'] = self.xls['Date'].dt.strftime('%Y-%m-%d')  # datetime to string
		temp['1 yr ago P'] = self.xls['1 yr ago P']
		temp['1 yr ago P'] = self.xls['1 yr ago P'].dt.strftime('%Y-%m-%d')  # datetime to string

		dateslist = temp['Date'].tolist()
		dateslist = [time.mktime(datetime.datetime.strptime(date, "%Y-%m-%d").timetuple()) for date in dateslist]
		searchlist = temp['1 yr ago P'].tolist()  # df column to list
		searchlist = [time.mktime(datetime.datetime.strptime(searchvalue, "%Y-%m-%d").timetuple()) for searchvalue in
					  searchlist]

		yrindex = []
		for searchvalue in (searchlist):
			yrindex.append(self.take_closest(dateslist, searchvalue))

		self.xls['1 yr index P'] = yrindex
		return self.xls

	# 1 year P
	def oneyearP(self, t):
		#print ("oneyearP: " + t)
		# LIST IMPLEMENTATION
		# list of percentage off from peaks									 float
		oneyearpeaks = self.xls['1 yr pk %'].tolist()
		# list of indexes for range to take percentile for '1 yr pk %'		  int
		oneyrindexp = self.xls['1 yr index P'].tolist()
		# closesformax = self.xls['Close'].tolist()

		oneyrP = []

		# for index, row in enumerate(oneyrindexp):
		#	 if row == 0:
		#		 oneyrP.append("")
		#	 else:
		#		 if type(oneyearpeaks[row]) == str:
		#			 oneyrP.append("")
		#		 else:
		#			 x = oneyearpeaks[row:index+1]
		#			 x = sorted(x, reverse=False)
		#			 oneyrP.append(x[24])
		for index, row in enumerate(oneyrindexp):
			if index < 24:
				oneyrP.append(0)
			else:
				x = oneyearpeaks[row:index+1]
				x = sorted(x, reverse=False)
				oneyrP.append(x[24])

		self.xls['1 yr P%'] = oneyrP

		return self.xls

	# Peak in dollars helper column
	def peakP(self):

		# list of percentage off from peaks									 float
		oneyearpeaks = self.xls['1 yr pk %'].tolist()
		closelist = self.xls['Close'].tolist()
		oneyridx = self.xls['1 yr index P'].tolist()

		peaklist = []

		for index, item in enumerate(oneyridx):
			# item is 1 year ago index
			# index is current row

			if item == -1:
				# maximum = ""
				# offpeak = ""
				peaklist.append("")
			else:
				if type(oneyearpeaks[item]) == str:
					peaklist.append("")
				else:
					maximum = self.get_max_by_range(item, index)

					peaklist.append(maximum)

		self.xls['1 yr pk Price'] = peaklist

		return self.xls

	def limit_and_delta_P(self,t):
		yrpkhelper = self.xls['1 yr P%'].tolist()
		yrpricehelper = self.xls['1 yr pk Price'].tolist()
		closeprices = self.xls['Close'].tolist()
		#print("limitdelta: " + t)
		plimit = []
		pdelta = []
		for pk, price, closeprice in zip(yrpkhelper, yrpricehelper, closeprices):
			if price == -1:
				# maximum = ""
				# offpeak = ""
				plimit.append("")
				pdelta.append("")
			else:
				if type(price) == str:
					plimit.append("")
					pdelta.append("")
				else:
					plimit.append(price*(1+pk/100))
					pdelta.append(price*(1+pk/100)/closeprice-1)

		self.xls['Plimit'] = plimit
		self.xls['Pdelta'] = pdelta
		return self.xls

	def p10temppercentpeak(self):
		oneyearpeaks = self.xls['1 yr pk %'].tolist()
		# list of indexes for range to take percentile for '1 yr pk %'		  int
		oneyrindexp = self.xls['1 yr index'].tolist()

		oneyrP = []

		for index, row in enumerate(oneyrindexp):
			if index < 24:
				oneyrP.append(0)
			else:
				x = oneyearpeaks[row:index + 1]
				x = sorted(x, reverse=False)
				oneyrP.append(x[24])

		self.xls['1 yr P10%'] = oneyrP

		return self.xls

	def p5temppercentpeak(self):
		oneyearpeaks = self.xls['1 yr pk %'].tolist()
		# list of indexes for range to take percentile for '1 yr pk %'		  int
		oneyrindexp = self.xls['1 yr index'].tolist()

		oneyrP = []

		for index, row in enumerate(oneyrindexp):
			if index < 11:
				oneyrP.append(0)
			else:
				x = oneyearpeaks[row:index + 1]
				x = sorted(x, reverse=False)
				oneyrP.append(x[11])

		self.xls['1 yr P5%'] = oneyrP

		return self.xls

	def P10orP5(self):
		# all doubles
		olist = self.xls['1 yr pk %'].tolist()
		dvlist = self.xls['1 yr P5%'].tolist()
		delist = self.xls['1 yr P10%'].tolist()

		p10orp5 = []
		for o, dv, de in zip(olist, dvlist, delist):
			if o <= dv:
				p10orp5.append('5')
			elif o <= de:
				p10orp5.append('10')
			else:
				p10orp5.append(0)

		self.xls['P10orP5'] = p10orp5

	def last_peak(self):
		oneyearpeaks = self.xls['1 yr pk %'].tolist()

		temp = pd.DataFrame()
		temp['Date'] = self.xls['Date'].dt.strftime('%Y-%m-%d')  # datetime to string
		dateslist = temp['Date'].tolist()

		last_peak = []

		for index, value in enumerate(oneyearpeaks):
			if value == 0:
				last_peak.append(time.mktime(datetime.datetime.strptime(dateslist[index], "%Y-%m-%d").timetuple()))
			elif value == "":
				last_peak.append(0)
			else:
				last_peak.append(last_peak[-1])

		self.xls['last peak'] = last_peak

	def peakfm(self):
		o_list =	self.xls['1 yr pk %'].tolist()
		ew_list =   self.xls['streak'].tolist()
		fc_list =   self.xls['fib'].tolist()
		fk_list =   self.xls['P10orP5'].tolist() # str
		fl_list =   self.xls['last peak'].tolist() # date
		fm_list = [] # 'peak'

		for o, ew, fc, fk, fl in zip(o_list, ew_list, fc_list, fk_list, fl_list):
			if o == 0:
				fm_list.append(1)
			elif fk == '5' or fk == '10':
				fm_list.append(0)
			elif ew <= -2 and fc <= -5:
				fm_list.append(0)
			else:
				fm_list.append(fm_list[-1])

		self.xls['peak'] = fm_list

	def sig(self, t):
		o_list =	self.xls['1 yr pk %'].tolist()
		ew_list =   self.xls['streak'].tolist()
		fc_list =   self.xls['fib'].tolist()
		fk_list =   self.xls['P10orP5'].tolist() # str
		fl_list =   self.xls['last peak'].tolist() # date
		fm_list =   self.xls['peak'].tolist()

		sig_list = []
		for index, (o, ew, fc, fk, fl, fm) in enumerate(zip(o_list, ew_list, fc_list, fk_list, fl_list, fm_list)):
			if (fm_list[index-1] == 1) and (sig_list and (sig_list[-1] == 0)) and (fk == '5' or fk == '10' or (ew <= -2 and fc <= -5)):
				sig_list.append(1)
			else:
				sig_list.append(0)
		self.xls['sig'] = sig_list

	def concatdate(self):
		# you can drop these 3 later
		self.xls['yeartemp'] = pd.DatetimeIndex(self.xls['Date']).year
		self.xls['monthtemp'] = pd.DatetimeIndex(self.xls['Date']).month  # we want previous month
		self.xls['daytemp'] = pd.DatetimeIndex(self.xls['Date']).day

		yearlist = self.xls['yeartemp'].tolist()
		monthlist = self.xls['monthtemp'].tolist()
		daylist = self.xls['daytemp'].tolist()

		def get_month_end(dt):
			first_of_month = datetime.datetime(dt.year, dt.month, 1)
			next_month_date = first_of_month + datetime.timedelta(days=32)
			new_dt = datetime.datetime(next_month_date.year, next_month_date.month, 1)
			return new_dt - datetime.timedelta(days=1)

		tempconcatlist = []
		for year, month, day in zip(yearlist, monthlist, daylist):
			tempdate = datetime.datetime(year, month, day)
			temp = get_month_end(tempdate) - relativedelta(months=1)

			yr = temp.year
			mth = temp.month
			dy = temp.day

			output = str(yr) + str(mth) + str(dy) + ' ' + str(self.symbol)

			tempconcatlist.append(output)

		self.xls['concat'] = tempconcatlist
