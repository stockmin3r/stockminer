import pandas as pd
import time
import datetime as dt
import math
from bisect import bisect_left
import sys
import threading
sys.setrecursionlimit(10000000)

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

			#return['' if math.isnan(exit) else time.mktime(dt.datetime.strptime(exit, '%m/%d/%Y').timetuple()) for exit in list]
			return ['' if exit == '' else time.mktime(dt.datetime.strptime(exit, '%Y-%m-%d').timetuple()) for exit in list]
		else:
			return [time.mktime(dt.datetime.strptime(entry, '%Y-%m-%d').timetuple()) for entry in list]

	if direction == 'tohuman':
		# nan is for exit dates
		if special == 'nan':
			return [date if date == '' else dt.datetime.fromtimestamp(int(float(date))).strftime('%Y-%m-%d') for date in list]
		else:
			return [date if date == '' else dt.datetime.fromtimestamp(int(float(date))).strftime('%Y-%m-%d') for date in list]
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

# [1] READ IN DATA
def read_data():
	df               = pd.read_csv('2020.csv')
	rank_list        = df['Rank'].tolist()
	ticker_list      = df['Ticker'].tolist()
	entry_price_list = df['Entry Price'].tolist()
	exit_price_list  = df['Exit Price'].tolist()
	entry_date_list  = df['Entry Date'].tolist()
	exit_date_list   = df['Exit Date'].tolist()
	entry_date_list  = to_unix(entry_date_list, 'none', 'tounix')
	exit_date_list   = to_unix(exit_date_list, 'nan', 'tounix')
	return rank_list, ticker_list, entry_price_list, exit_price_list, entry_date_list, exit_date_list


rank_list, ticker_list, entry_price_list, exit_price_list, entry_date_list, exit_date_list = read_data()
datapoints = len(rank_list)

def count_occurences(list, exitdate):
	counter = 0
	for entrydate in list:
		if entrydate == exitdate:
			counter += 1
	return counter

next_list = []
fork_count = []
for date in exit_date_list:
	if date == '':
		next_list.append('')
	else:
		if date > entry_date_list[-1]:
			next_list.append('')
		else:
			next_list.append(take_closest(entry_date_list, date))


for idx, date in enumerate(exit_date_list):
	if date == '' or next_list[idx] == 0:
		fork_count.append(0)
	else:
		if date > entry_date_list[-1]:
			fork_count.append(0)
		else:
			fork_count.append(count_occurences(entry_date_list, entry_date_list[next_list[idx]]))

next_list = ['' if x == '' else int(x)+1 for x in next_list]
next_list.insert(0,0)
fork_count.insert(0,0)
i	   = 0

def newrow(i):
	i += 1
	steps[1] = i

def finish():
	sys.exit('~we done')

def backward(j, numfork):
	k = max(forks1)
	if k == 0:
		finish()

	steps[k+1] = steps[k+1] + 1
	forks2[numfork] = forks2[numfork]-1
	if forks2[numfork] == 0:
		forks1[numfork] = 0
		numfork -= 1

	if k + 2 <= j:
		for i in range(k + 2, j+1):
			steps[i] = 0
	j = k
	forward(j, numfork)

def forward(j, numfork):
	j += 1
	if next_list[steps[j]] == '':
		# print(steps[1:j+1])
		with open('forks.txt', 'a') as filehandle:
			filehandle.write(str(steps[1:j+1]))
			filehandle.write('\n')

		backward(j, numfork)
	else:
		steps[j+1] = next_list[steps[j]]

	if fork_count[steps[j]] > 1:
		numfork += 1
		forks1[numfork] = j
		forks2[numfork] = fork_count[steps[j]]-1
	forward(j, numfork)

# newrow(i)
# forward(j, numfork)

def thread_function(i):
	newrow(i)
	forward(j, numfork)

for run in range(0, datapoints):
	j	   = 0
	numfork = 0
	steps   = [0]*10000
	forks1  = [0]*10000
	forks2  = [0]*10000

	threading.Thread(target=thread_function, args=(run,)).start()
	time.sleep(1)


