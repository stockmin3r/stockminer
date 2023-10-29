# Thanks to: https://stackoverflow.com/questions/51970543/python-how-to-scrape-stock-key-data-from-marketwatch-com-where-the-data-is-d

import requests
import json

req_url = 'https://api-secure.wsj.net/api/michelangelo/timeseries/history?json={"Step":"PT1M","TimeFrame":"D1",' \
          '"EntitlementToken":"cecc4267a0194af89ca343805a3e57af","IncludeMockTick":true,"FilterNullSlots":false,' \
          '"FilterClosedPoints":true,"IncludeClosedSlots":false,"IncludeOfficialClose":true,"InjectOpen":false,' \
          '"ShowPreMarket":true,"ShowAfterHours":false,"UseExtendedTimeFrame":false,"WantPriorClose":true,' \
          '"IncludeCurrentQuotes":false,"ResetTodaysAfterHoursPercentChange":false,' \
          '"Series":[{"Key":"STOCK/US/XNYS/SUZ","Dialect":"Charting","Kind":"Ticker","SeriesId":"s1",' \
          '"DataTypes":["Last"],"Indicators":[{"Parameters":[{"Name":"ShowOpen"},{"Name":"ShowHigh"},' \
          '{"Name":"ShowLow"},{"Name":"ShowPriorClose","Value":true},{"Name":"Show52WeekHigh"},' \
          '{"Name":"Show52WeekLow"}],"Kind":"OpenHighLowLines","SeriesId":"i2"}]}]}&ckey=cecc4267a0'

r = requests.get(req_url, headers={"User-Agent": "Mozilla/5.0 (X11; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0",
                                   "Content-Type": "application/json, text/javascript, */*; q=0.01",
                                   "Dylan2010.EntitlementToken": "cecc4267a0194af89ca343805a3e57af"})
# Full Return
print(r)

# Stock UNIX Dates
print(json.loads(r.content)['TimeInfo']['Ticks'])

# Stock Prices
print(json.loads(r.content)['Series'][0]['DataPoints'])
