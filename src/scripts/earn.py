import sys
import pandas as pd
from datetime import datetime
from datetime import timedelta
from yahoo_earnings_calendar import YahooEarningsCalendar
import dateutil.parser
start_date = datetime.now().date() - timedelta(days=400)
end_date = (datetime.now().date() + timedelta(days=400))
report_date = datetime.now().date()
yec = YahooEarningsCalendar()
earnings_list = yec.get_earnings_of(sys.argv[1])
earnings_df = pd.DataFrame(earnings_list)
earnings_df['report_date'] = earnings_df['startdatetime'].apply(lambda x: dateutil.parser.isoparse(x).date())
earnings_df = earnings_df.loc[earnings_df['report_date'].between(start_date, end_date)] .sort_values('report_date')

file = open("/stockminer/data/stocks/stockdb/" + sys.argv[1] + ".edates", "w")
file.write(str(earnings_df.report_date))
