import requests
import json
import time
from datetime import datetime as dt
from urllib.parse import urlencode 

data = {
        "Step":"P7D",
        "TimeFrame":"P1Y",
        "EntitlementToken":"57494d5ed7ad44af85bc59a51dd87c90",
        "IncludeMockTick":True,
        "FilterNullSlots":False,
        "FilterClosedPoints":True,
        "IncludeClosedSlots":False,
        "IncludeOfficialClose":True,
        "InjectOpen":True,
        "ShowPreMarket":True,
        "ShowAfterHours":False,
        "UseExtendedTimeFrame":True,
        "WantPriorClose":False,
        "IncludeCurrentQuotes":False,
        "ResetTodaysAfterHoursPercentChange":False,
        "Series":[{"Key":"STOCK/US/XNAS/AAPL","Dialect":"Charting","Kind":"Ticker","SeriesId":"s1","DataTypes":["Open","High","Low","Last"],"Indicators":[{"Parameters":[{"Name":"Period","Value":"50"}],"Kind":"SimpleMovingAverage","SeriesId":"i2"},{"Parameters":[],"Kind":"Volume","SeriesId":"i3"}]}]
    }

data = {
    'json' : json.dumps(data)
}

data = urlencode(data)

headers = {
    'Accept': 'application/json, text/javascript, */*; q=0.01',
    'Dylan2010.EntitlementToken': '57494d5ed7ad44af85bc59a51dd87c90',
    'Origin': 'https://quotes.wsj.com',
    'Referer': 'https://quotes.wsj.com/ABB/advanced-chart',
    'Sec-Fetch-Mode': 'cors',
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/76.0.3809.132 Safari/537.36'
}

url = 'https://api.wsj.net/api/michelangelo/timeseries/history?' + data + '&ckey=57494d5ed7'
print(url)
r = requests.get(url, headers = headers)
r.text
print(r.text);
