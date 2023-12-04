<h1 align="center">
Build your own Decentralized Social/Tech Media
-----------------------------------------------
</h1>

[![Discord](https://badgen.net/badge/icon/discord?icon=discord&label)](https://discord.gg/pfHADMGz)

The goals of this project are to give more digital power to individuals and businesses by allowing
them to create their own websites without having to pay for services/hosting/infrastructure/admining.
Currently, the website is able to generate charts and tables for stock algorithmic analysis
(for the US market only at the moment). The website layout is defined by a Website.json file
which resides in src/website/(mainpage|backpage)/name/website.json

<!-- mainpage -->
<h1 align="center">
	<img src="https://i.imgur.com/YOqdQUX.png"" width="256"/>
	<br/>
	<br/>
</h1>

<h1 align="center">
	Demo: <a href="https://www.stockminer.org"/>
</h1>

Currently this version is running only on Ubuntu 22.04 but other systems will be added
in time eg: *BSD, Windows, MacOSX

To install all dependencies use the "make linux" target otherwise make j will do a make -j 12

```
make linux
./configure
make npm
cd pkg/npm
npm pack
npm install stockminer-*.tgz
npm start
```

It is also possible to just run ./stockminer and connect to https://localhost:4443

On boot, The stockminer daemon will start fetching data for the tickers in data/stocks/STOCKS.TXT
The electron npm BrowserWindow app connects to the stockminer node (localhost:4443).
An update workspace will be loaded to show the progress of the data fetching.
Currently, the standard 1D OHLC data is fetched from yahoo and takes approximately ~1h40m for ~2.6k tickers

<h1 align="center">
	<img src="https://i.imgur.com/ygdjD3Q.png" width="256"/>
	<br/>
	<br/>
</h1>

<!-- profile -->
<h1 align="center">
	<img src="https://i.imgur.com/NtB6aIj.png" width="256"/>
	<br/>
	<br/>
</h1>
