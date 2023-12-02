#include <conf.h>
#include <stocks/stocks.h>

/*
 * GSPC: INDEX%2FUS%2FS%26P%20US%2FSPX             INDEX/US/S&P US/SPX              SPX
 * IXIC: INDEX%2FUS%2FXNAS%2FNDX                   INDEX/US/XNAS/NDX                NDX
 * DJI:  INDEX%2FUS%2FDOW%20JONES%20GLOBAL%2FDJIA  INDEX/US/DOW JONES GLOBAL/DJIA   DJIA
 * RUT:  INDEX%2FUS%2F%2FRUT                       INDEX/US/RUT                     RUT
 * SPY:  FUND%2FUS%2FARCX%2FSPY                    FUND/US/ARCX/SPY                 SPY
 * XLB:  FUND%2FUS%2FARCX%2FXLB                    FUND/US/ARCX/XLB                 XLB
 * XLC:  FUND%2FUS%2FARCX%2FXLC                    FUND/US/ARCX/XLC                 XLC
 * XLE:  FUND%2FUS%2FARCX%2FXLE                    FUND/US/ARCX/XLE                 XLE
 * XLF:  FUND%2FUS%2FARCX%2FXLF                    FUND/US/ARCX/XLF                 XLF
 * XLI:  FUND%2FUS%2FARCX%2FXLI                    FUND/US/ARCX/XLI                 XLI
 * XLK:  FUND%2FUS%2FARCX%2FXLK                    FUND/US/ARCX/XLK                 XLK
 * XLP:  FUND%2FUS%2FARCX%2FXLP                    FUND/US/ARCX/XLP                 XLP
 * XLU:  FUND%2FUS%2FARCX%2FXLU                    FUND/US/ARCX/XLU                 XLU
 * XLV:  FUND%2FUS%2FARCX%2FXLV                    FUND/US/ARCX/XLV                 XLV
 * XLY:  FUND%2FUS%2FARCX%2FXLY                    FUND/US/ARCX/XLY                 XLY
 */
char  *WSJ_US_IDX_SPX[] = { WSJ_SPX_CLO,    WSJ_SPX_CLO2,   WSJ_SPX_ALL,    WSJ_SPX_CUR     };
char  *WSJ_US_IDX_DOW[] = { WSJ_DOW_CLO,    WSJ_DOW_CLO2,   WSJ_DOW_ALL,    WSJ_DOW_CUR     };
char  *WSJ_US_IDX_NDX[] = { WSJ_NDX_CLO,    WSJ_NDX_CLO2,   WSJ_NDX_ALL,    WSJ_NDX_CUR     };
char  *WSJ_US_IDX_RUT[] = { WSJ_RUT_CLO,    WSJ_RUT_CLO2,   WSJ_RUT_ALL,    WSJ_RUT_CUR     };
char **WSJ_INDEXES[]    = { WSJ_US_IDX_SPX, WSJ_US_IDX_NDX, WSJ_US_IDX_DOW, WSJ_US_IDX_RUT  };

void wsj_stock_api(struct stock *stock)
{
	struct WSJ  *WSJ;
	char        *wsj_api_src,**wsj_api_dst,   *ticker,     *buf;
	int          wsj_api_size, type, exchange, ticker_size, total_size, x;

	WSJ         = &stock->API.WSJ;
	ticker      = stock->sym;
	exchange    = stock->market;
	ticker_size = strlen(ticker);
	type        = stock->type;
	if (ticker_size > 7)
		return;

	for (x=0; x<4; x++) {
		switch (x) {
			case WSJ_API_STOCK_CLOSE:
				wsj_api_src  = WSJ_CLOSE;
				wsj_api_dst  = &WSJ->CLOSE;
				wsj_api_size = sizeof(WSJ_CLOSE)-1;
				break;
			case WSJ_API_STOCK_CLOSE2:
				wsj_api_src  = WSJ_CLOSE2;
				wsj_api_dst  = &WSJ->CLOSE2;
				wsj_api_size = sizeof(WSJ_CLOSE2)-1;
				break;
			case WSJ_API_STOCK_ALLDAY:
				wsj_api_src  = WSJ_ALL;
				wsj_api_dst  = &WSJ->ALL;
				wsj_api_size = sizeof(WSJ_ALL)-1;
				break;
			case WSJ_API_STOCK_CURTICK:
				wsj_api_src  = WSJ_CUR;
				wsj_api_dst  = &WSJ->CLOSE;
				wsj_api_size = sizeof(WSJ_CUR)-1;
				break;
		}
		total_size   = wsj_api_size + ticker_size + sizeof(WSJ_OHLC)-1 + sizeof(WSJ_HDR)-1;
		*wsj_api_dst = buf = (char *)malloc(total_size+1);
		strcpy(buf, wsj_api_src);
		if (exchange == MARKET_NYSE) {
			buf[wsj_api_size-6] = 'Y';
			buf[wsj_api_size-5] = 'S';
		} else if (exchange == MARKET_OTC) {
			buf[wsj_api_size-8] = 'O';
			buf[wsj_api_size-7] = 'O';
			buf[wsj_api_size-6] = 'T';
			buf[wsj_api_size-5] = 'C';
		} else if (exchange == MARKET_ASE) {
			buf[wsj_api_size-7] = 'A';
			buf[wsj_api_size-6] = 'S';
			buf[wsj_api_size-5] = 'E';
		}
		strcat(buf+wsj_api_size, ticker);
		memcpy(buf+wsj_api_size+ticker_size,                    WSJ_OHLC, sizeof(WSJ_OHLC));
		memcpy(buf+wsj_api_size+ticker_size+sizeof(WSJ_OHLC)-1, WSJ_HDR,  sizeof(WSJ_HDR)-1);
	}
}

void wsj_index_api(struct stock *stock)
{
	struct WSJ  *WSJ;
	char        *wsj_api_src, **wsj_api_dst,  *ticker,     *buf;
	int          wsj_api_size, type, exchange, ticker_size, total_size, x, y;

	for (x=0; x<NR_GLOBAL_INDEXES; x++) {
		switch (x) {
			case WSJ_US_INDEX_SPX: {
				for (y=0; y<4; y++) {
					switch (y) {
						case WSJ_API_INDEX_CLOSE:
							wsj_api_src     = WSJ_INDEXES[x][0];
							wsj_api_dst     = &WSJ->CLOSE;
							wsj_api_size    = sizeof(WSJ_SPX_CLO)-1;
							break;
						case WSJ_API_INDEX_CLOSE2:
							wsj_api_src     = WSJ_INDEXES[x][2];
							wsj_api_dst     = &WSJ->CLOSE2;
							wsj_api_size    = sizeof(WSJ_SPX_CLO2)-1;
							break;
						case WSJ_API_INDEX_ALLDAY:
							wsj_api_src     = WSJ_INDEXES[x][3];
							wsj_api_dst     = &WSJ->ALL;
							wsj_api_size    = sizeof(WSJ_SPX_ALL)-1;
							break;
						case WSJ_API_INDEX_CURTICK:
							wsj_api_src     = WSJ_INDEXES[x][4];
							wsj_api_dst     = &WSJ->CUR;
							wsj_api_size    = sizeof(WSJ_SPX_CUR)-1;
							break;
					}
				}
			};
			case WSJ_US_INDEX_DOW: {
				for (y=0; y<4; y++) {
					switch (y) {
						case WSJ_API_INDEX_CLOSE:
							wsj_api_src     = WSJ_INDEXES[x][0];
							wsj_api_dst     = &WSJ->CLOSE;
							wsj_api_size    = sizeof(WSJ_DOW_CLO)-1;
							break;
						case WSJ_API_INDEX_CLOSE2:
							wsj_api_src     = WSJ_INDEXES[x][1];
							wsj_api_dst     = &WSJ->CLOSE2;
							wsj_api_size    = sizeof(WSJ_DOW_CLO2)-1;
							break;
						case WSJ_API_INDEX_ALLDAY:
							wsj_api_src     = WSJ_INDEXES[x][2];
							wsj_api_dst     = &WSJ->ALL;
							wsj_api_size    = sizeof(WSJ_DOW_ALL)-1;
							break;
						case WSJ_API_INDEX_CURTICK:
							wsj_api_src     = WSJ_INDEXES[x][3];
							wsj_api_dst     = &WSJ->CUR;
							wsj_api_size    = sizeof(WSJ_DOW_CUR)-1;
							break;
					}
				}
			};
			case WSJ_US_INDEX_NDX: {
				for (y=0; y<4; y++) {
					switch (y) {
						case WSJ_API_INDEX_CLOSE:
							wsj_api_src     = WSJ_INDEXES[x][0];
							wsj_api_dst     = &WSJ->CLOSE;
							wsj_api_size    = sizeof(WSJ_NDX_CLO)-1;
							break;
						case WSJ_API_INDEX_CLOSE2:
							wsj_api_src     = WSJ_INDEXES[x][1];
							wsj_api_dst     = &WSJ->CLOSE2;
							wsj_api_size    = sizeof(WSJ_NDX_CLO2)-1;
							break;
						case WSJ_API_INDEX_ALLDAY:
							wsj_api_src     = WSJ_INDEXES[x][2];
							wsj_api_dst     = &WSJ->ALL;
							wsj_api_size    = sizeof(WSJ_NDX_ALL)-1;
							break;
						case WSJ_API_INDEX_CURTICK:
							wsj_api_src     = WSJ_INDEXES[x][3];
							wsj_api_dst     = &WSJ->CUR;
							wsj_api_size    = sizeof(WSJ_NDX_CUR)-1;
							break;
					}
				}
			};
			case WSJ_US_INDEX_RUT: {
				for (y=0; y<4; y++) {
					switch (y) {
						case WSJ_API_INDEX_CLOSE:
							wsj_api_src     = WSJ_INDEXES[x][0];
							wsj_api_dst     = &WSJ->CLOSE;
							wsj_api_size    = sizeof(WSJ_RUT_CLO)-1;
							break;
						case WSJ_API_INDEX_CLOSE2:
							wsj_api_src     = WSJ_INDEXES[x][1];
							wsj_api_dst     = &WSJ->CLOSE2;
							wsj_api_size    = sizeof(WSJ_RUT_CLO2)-1;
							break;
						case WSJ_API_INDEX_ALLDAY:
							wsj_api_src     = WSJ_INDEXES[x][2];
							wsj_api_dst     = &WSJ->ALL;
							wsj_api_size    = sizeof(WSJ_RUT_ALL)-1;
							break;
						case WSJ_API_INDEX_CURTICK:
							wsj_api_src     = WSJ_INDEXES[x][3];
							wsj_api_dst     = &WSJ->CUR;
							wsj_api_size    = sizeof(WSJ_RUT_CUR)-1;
							break;
					}
				}
			};
		} // switch
	} // for

	wsj_api_size += sizeof(WSJ_HDR)-1;
	*wsj_api_dst  = buf = (char *)malloc(wsj_api_size+1);
	strcpy(buf, wsj_api_src);
	strcat(buf, WSJ_HDR);
}

void load_WSJ(struct XLS *XLS)
{
	struct stock *stock, **STOCKS;
	int           nr_stocks, type, x;

	STOCKS    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = STOCKS[x];
		switch (stock->type) {
			case STOCK_TYPE_STOCK:
				wsj_stock_api(stock);
				break;
			case STOCK_TYPE_INDEX:
				wsj_index_api(stock);
				break;
			case STOCK_TYPE_FUND:
				break;
		}
	}
}
