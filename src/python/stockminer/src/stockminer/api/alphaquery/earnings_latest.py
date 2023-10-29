import requests
import os
import pandas as pd
from requests_html import HTMLSession
tickerlistpath = os.path.join(os.sep, 'media', 'share', 'Stockland','raw_stock_data', 'tickerlists', 'tickerlist_validated.txt')
temp = open(tickerlistpath, 'r').read()
ticker_list = [x.strip() for x in temp.split('\n')]

earninghistfilepath = os.path.join(os.sep, 'media', 'share', 'Stockland','earnings', 'earningshistory.txt')
earningshistfile = open(earninghistfilepath, 'a')

# ---- [Scrape for ticker in ticker_list] ----
for t in ticker_list:
    #print(t)
    header = {
    "User-Agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.75 Safari/537.36",
    "X-Requested-With": "XMLHttpRequest"
    }

    # ---- [URLs] ----
    #nextestimatedate = 'https://www.zacks.com/stock/quote/' + t + '/detailed-estimates'
    earningshist = 'https://www.alphaquery.com/stock/' + t + '/earnings-history'

    # ---- [Next Earnings Date] ----
    #try:
    #    session = HTMLSession()
    #    response = session.get(nextestimatedate, headers=header)
    #except requests.exceptions.RequestException as e:
    #    print(e)

    #try:
    #    next_earnings = response.html.find('#detail_estimate', first=True).text
    #    nextearningsdatesfile.write(t + ' ' + next_earnings.splitlines()[2] + '\n')
    #except:
    #    pass

    # ---- [Earnings History] ----
    try:
        session = HTMLSession()
        response2 = requests.get(earningshist, headers=header)
    except requests.exceptions.RequestException as e:
        print(e)
        #x = pd.read_html(response2.text) # error when table is empty

        # fails before this
        #earningshistfile.write(t + ' ' + str(x) + '\n')

    try:
        x = pd.read_html(response2.text) # error when table is empty
    # fails before this
        earningshistfile.write(t + ' ' + str(x) + '\n')
    except ValueError as v:
        print('empty alphaquery')


# ---- [Other useful URLs] ----
# url2 = 'https://www.zacks.com/stock/research/AAPL/earnings-calendar'
# url2 = 'https://www.nasdaq.com/market-activity/stocks/aapl/earnings'
