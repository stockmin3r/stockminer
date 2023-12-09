import pandas as pd
import pandas_market_calendars as mcal

nyse = mcal.get_calendar('24/7')
sched = nyse.schedule(start_date='2023-01-01', end_date='2023-12-31')
calendar = ''
for x in range(len(sched)):
	calendar = calendar + str(sched.iloc[x][1]).split(" ")[0] + "\n"
file = open("/stockminer/data/stocks/CALENDAR-2023.TXT", "w")
file.write(calendar)
