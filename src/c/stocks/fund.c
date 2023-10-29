#include <conf.h>
#include <stocks/stocks.h>
#include <curl/curl.h>

#define YAHOO_STATS "https://finance.yahoo.com/quote/%s/key-statistics?p=%s"


char *FUNDAMENTALS[] = { "Market Cap", "Enterprise Value", "Trailing P/E", "Forward P/E", "PEG Ratio", "Price/Sales", "Price/Book", "Enterprise Value/Revenue", "Enterprise Value/EBITDA",
						 "Avg Vol (3 month)", "Avg Vol (10 day)", "Shares Outstanding", "Float", "% Held by Insiders", "% Held by Institutions",
						 "Shares Short", "Short Ratio", "Short % of Float", "Short % of Shares Outstanding", "Shares Short",
						 /* Dividends */
						 "Forward Annual Dividend Rate", "Forward Annual Dividend Yield", "Trailing Annual Dividend Rate", "Trailing Annual Dividend Yield", "5 Year Average Dividend Yield",
						 "Payout Ratio", "Dividend Date", "Ex-Dividend Date", "Last Split Factor", "Last Split Date",
						 /* Financial Highlights */
						 "Profit Margin", "Operating Margin", "Return on Assets", "Return on Equity", "Revenue (ttm)", "Revenue Per Share", "Quarterly Revenue Growth",
						 "Gross Profit (ttm)", "EBITDA", "Net Income Avi to Common", "Diluted EPS (ttm)", "Quarterly Earnings Growth",
						 "Total Cash (mrq)", "Total Cash Per Share", "Total Debt (mrq)", "Total Debt/Equity", "Current Ratio (mrq)", "Book Value Per Share",
						 "Operating Cash Flow", "Levered Free Cash Flow"};

#define NR_FUND (sizeof(FUNDAMENTALS)/8)

/*
void update_fundb()
{
	struct stock   *stock;
	struct fund     fund;
	struct fund_int fund_int;
	char            path[256], *p;
	double         *FIPI, *FIPS;
	int x, y, fd, done = 0;

	memset(&fund_int, 0, sizeof(fund_int));
	for (x=0; x<nr_allcaps; x++) {
		stock = ALLCAPS[x];
//		stock_fund(stock);
		sprintf(path, "data/stocks/stockdb/%s.f", stock->sym);
		printf("[%s] %d\n", stock->sym, x);
		fd = open(path, O_RDONLY, 0644);
		read(fd, (void *)&fund, sizeof(fund));
		FIPS = (double *)&fund;
		FIPI = (double *)&fund_int;
		for (y=0; y<49; y++) {
			p = (char *)FIPS;
			*FIPI = strtod((char *)FIPS, NULL);
			while (*p != 0) {
				switch (*p) {
					case 'T':*FIPI *= 1000000000000.0; done = 1;break;
					case 'B':*FIPI *= 1000000000.0;    done = 1;break;
					case 'M':*FIPI *= 1000000.0;       done = 1;break;
					case 'k':*FIPI *= 1000.0;          done = 1;break;
				}
				if (done) {
					done = 0;
					break;
				}
				p++;
				if (p-(char *)FIPS >= 12)
					break;
			}
			FIPI++;
			FIPS = (double *)((char *)FIPS+12);
		}
		fund_int.timestamp = 1614693600;
		close(fd);
		unlink(path);
		fd = open(path, O_RDWR|O_CREAT, 0644);
		write(fd, &fund_int, sizeof(fund_int));
		close(fd);

		strcat(path, "s");
		fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0644);
		write(fd, &fund, sizeof(fund));
		close(fd);
	}
}*/

int stock_fund(struct stock *stock)
{
	char     url[256];
	char     page[3072 KB];
	char    *p, *p2, c;
	char    *FPS; // Fundamental Pointer to String
	double  *FPD; // Fundamental Pointer to Double
	int      x;

	sprintf(url, YAHOO_STATS, stock->sym, stock->sym);
	FPS = (char *)&stock->fund_str;
	stock->fund_int = (struct fund_int *)realloc(stock->fund_int, (stock->nr_fundamentals+1) * sizeof(struct fund_int));
	FPD = (double *)&stock->fund_int[stock->nr_fundamentals];

	if (!curl_get(url, page))
		return 0;
	p = page;
	for (x=0; x<NR_FUND; x++) {
		p = strstr(p, FUNDAMENTALS[x]);
		if (!p)
			return 1;
		p2 = strstr(p, "</td></tr>");
		if (!p2)
			return 0;
		*p2++ = 0;
		p = p2-2;
		c = *p;
		while (*p != '>') p--;
		strncpy(FPS, p+1, 11);
		*FPD = strtod(p+1, NULL);

		switch (c) {
			case 'T':*FPD *= 1000000000000.0;break;
			case 'B':*FPD *= 1000000000.0;   break;
			case 'M':*FPD *= 1000000.0;      break;
			case 'k':*FPD *= 1000.0;         break;
		};
		printf("FUND: %s value: %s (value: %.2f)\n", FUNDAMENTALS[x], FPS, *FPD);
		FPS  = FPS + 12;
		FPD += 1;
		p    = p2 + 2;
	}
	stock->fund_int[stock->nr_fundamentals++].timestamp = time(NULL);
	return 1;
}


void update_fundb(char *ticker, struct XLS *XLS)
{
	struct stock  *stock;
	struct stock **stocks;
	char           path[256];
	int            x, y, nr_stocks, pathsize;

	stocks    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = stocks[x];
		if (ticker && strcmp(stock->sym, ticker))
			continue;
		for (y=0; y<3; y++) {
			if (stock_fund(stock))
				break;
			printf(BOLDRED "%s failed" RESET "\n", stock->sym);
			os_sleep(60*5);
		}
		pathsize = snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.f", stock->sym);
		printf("[%s] %d\n", stock->sym, x);
		fs_appendfile(path, &stock->fund_int[stock->nr_fundamentals], sizeof(struct fund_int));
		path[pathsize++] = 's';
		path[pathsize]   = 0;
		fs_newfile(path, &stock->fund_str, sizeof(stock->fund_str));
		os_sleep(10);
	}
}
