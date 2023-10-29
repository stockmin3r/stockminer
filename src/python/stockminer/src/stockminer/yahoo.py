import sys
from pandas_datareader import data as pdr
import yfinance as yf
import pandas as pd
from io import StringIO
import stockminer

def usage():
	print ("--csv_path output_directory - directory where to download CSVs to");

def yahoo_earnings(ticker, stockdb_path):
	from datetime import datetime
	from datetime import timedelta
	from yahoo_earnings_calendar import YahooEarningsCalendar
	import dateutil.parser
	start_date    = datetime.now().date()  - timedelta(days=400)
	end_date      = (datetime.now().date() + timedelta(days=400))
	report_date   = datetime.now().date()
	yec           = YahooEarningsCalendar()
	earnings_list = yec.get_earnings_of(sys.argv[1])
	earnings_df   = pd.DataFrame(earnings_list)
	earnings_df['report_date'] = earnings_df['startdatetime'].apply(lambda x: dateutil.parser.isoparse(x).date())
	earnings_df = earnings_df.loc[earnings_df['report_date'].between(start_date, end_date)] .sort_values('report_date')
	file = open(stockdb_path + ticker + ".edates", "w")
	file.write(str(earnings_df.report_date))
	sys.exit()

def stockminer_yahoo(argv,config):
	csv  = StringIO()
	args = stockminer.get_options(argv, "hc:r:t:", ["help", "csv_path=", "maxrows=", "ticker="])

	# command line arguments take precedence over the config file
	if "csv_path" in args:
		csv_path=args['csv_path']
	else:
		csv_path=config['csv_path']
	if "maxrows" in args:
		maxrows=int(args['maxrows'])
	elif "maxrows" in config:
		maxrows=int(config['maxrows'])
	else:
		maxrows=stockminer.MAX_TICKER_ROWS

	if "ticker" in args:
		tickers = [args['ticker']]
	else:
		tickers = config['ticker_list']

	for ticker in tickers:
		yf.pdr_override()
		df = pdr.get_data_yahoo(ticker,period="max")
		if df.empty:
			continue
		df["Date"]=df.index
		df=df.sort_index(axis=0,ascending=False)
		df=df.iloc[0:maxrows, :-1]
		df=df.sort_index(axis=0,ascending=True)
		df.drop(columns='Adj Close', axis=1, inplace=True);
		csv=df.to_csv(header=False,float_format="%.3f")
		fp = open(csv_path+'/'+ticker+'.csv', "w")
		for line in csv:
			if ",,," not in line:
				fp.write(line)

if __name__ == "__main__":
	config = stockminer.init()
	stockminer_yahoo(sys.argv[1:],config)
