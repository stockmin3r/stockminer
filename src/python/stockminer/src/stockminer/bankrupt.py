'''
Sarah's bankrupcy code
'''
import json
import urllib.request
import pandas as pd
from pandas.io.json import json_normalize
from bs4 import BeautifulSoup
import requests

path = 'stockdb/8k'

# API Key
TOKEN = 'XXX' # replace YOUR_API_KEY with the API key you got from sec-api.io after sign up
# API endpoint
API = "https://api.sec-api.io?token=" + TOKEN

# define the filter parameters you want to send to the API
payload = {
  "query": { "query_string": { "query": "formType:\"8-K\"" } },
  "from": "0",
  "size": "10",
  "sort": [{ "filedAt": { "order": "desc" } }]
}

# format your payload to JSON bytes
jsondata = json.dumps(payload)
jsondataasbytes = jsondata.encode('utf-8')   # needs to be bytes

# instantiate the request
req = urllib.request.Request(API)

# set the correct HTTP header: Content-Type = application/json
req.add_header('Content-Type', 'application/json; charset=utf-8')
# set the correct length of your request
req.add_header('Content-Length', len(jsondataasbytes))
# send the request to the API
response = urllib.request.urlopen(req, jsondataasbytes)

# read the response
res_body = response.read()
# transform the response into JSON
filings = json.loads(res_body.decode("utf-8"))

filing_list = filings['filings']

data = pd.DataFrame(filing_list)

filing_data = data[['ticker','linkToFilingDetails']]

bankrupt_text = []
ch11_text = []

links = filing_data['linkToFilingDetails'].to_list()

for l in links:
    html_data = urllib.request.urlopen(l)
    soup = BeautifulSoup(html_data, 'html.parser')
    text = soup.get_text()

    if "bankruptcy" in text:
        bankrupt_text.append("True")
    else:
        bankrupt_text.append("False")

    if "Chapter 11" in text:
        ch11_text.append("True")
    else:
        ch11_text.append("False")

filing_data = filing_data.copy()

filing_data["Bankruptcy"] = bankrupt_text
filing_data["Chapter_11"] = ch11_text

filing_data.to_csv('stockdb/8k/8k.csv')
