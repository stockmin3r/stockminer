import requests
import pandas as pd
from requests_html import HTMLSession
import os

tickerlistpath = os.path.join(os.sep, 'media', 'share', 'Stockland','raw_stock_data', 'tickerlists', 'tickerlist_validated.txt')
temp = open(tickerlistpath, 'r').read()
ticker_list = [x.strip() for x in temp.split('\n')]

zacks_earnings_api="https://www.zacks.com/stock/research/{}/earnings-calendar"
nasdaq_earnings_api="https://www.nasdaq.com/market-activity/stocks/{}/earnings"

nxtpath = os.path.join(os.sep, 'media', 'share', 'Stockland','earnings', 'next_earnings_dates.txt')
nextearningsdatesfile = open(nxtpath, 'a')

# ---- [Scrape for ticker in ticker_list] ----
for t in ticker_list:
    print(t)
    header = {
    "User-Agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.75 Safari/537.36",
    "X-Requested-With": "XMLHttpRequest"
    }

    url = 'https://www.zacks.com/stock/quote/' + t + '/detailed-estimates'
    try:
        session = HTMLSession()
        response = session.get(url, headers=header)
    except requests.exceptions.RequestException as e:
        print(e)

    try:
        next_earnings = response.html.find('#detail_estimate', first=True).text
        nextearningsdatesfile.write(t + ' ' + next_earnings.splitlines()[4] + '\n')
    except:
        pass

# ---- [Other useful URLs] ----
# url2 = 'https://www.zacks.com/stock/research/AAPL/earnings-calendar'
# url2 = 'https://www.nasdaq.com/market-activity/stocks/aapl/earnings'
