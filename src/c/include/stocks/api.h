#ifndef __API_H
#define __API_H

#define WSJ_API_STOCK_CLOSE    0
#define WSJ_API_STOCK_CLOSE2   1
#define WSJ_API_STOCK_ALLDAY   2
#define WSJ_API_STOCK_CURTICK  3

#define WSJ_API_INDEX_CLOSE    0
#define WSJ_API_INDEX_CLOSE2   1
#define WSJ_API_INDEX_ALLDAY   2
#define WSJ_API_INDEX_CURTICK  3

#define WSJ_API_FUND_CLOSE     0
#define WSJ_API_FUND_CLOSE2    1
#define WSJ_API_FUND_ALLDAY    2
#define WSJ_API_FUND_CURTICK   3

#define WSJ_API_CRYPTO_CLOSE   0
#define WSJ_API_CRYPTO_CLOSE2  1
#define WSJ_API_CRYPTO_ALLDAY  2
#define WSJ_API_CRYPTO_CURTICK 3

#define WSJ_US_INDEX_SPX       0
#define WSJ_US_INDEX_NDX       1
#define WSJ_US_INDEX_DOW       2
#define WSJ_US_INDEX_RUT       3
#define WSJ_UK_INDEX_FTSE100   4

#define NR_GLOBAL_INDEXES      4

#define DATA_FORMAT_WEBSOCKET_INTERNAL 1

#define WSJ_HDR    "Host: api.wsj.net\r\n" \
                   "Dylan2010.EntitlementToken: 57494d5ed7ad44af85bc59a51dd87c90\r\n" \
                   "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:77.0) Gecko/20100101 Firefox/77.0\r\n" \
                   "Accept-Encoding: gzip\r\n" \
                   "Content-Type: application/json, text/javascript\r\n\r\n"

/* 1m TWO TICKS without PRIOR CLOSE */
#define WSJ_CLOSE  "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"    \
				   "EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
				   "Slots%22%3Afalse%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"   \
				   "lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22"  \
				   "UseExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22"      \
				   "ResetTodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22STOCK%2FUS%2FXNAS%2F"

/* PRIOR CLOSE - Use only after a full day past EOD */
#define WSJ_CLOSE2 "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"    \
				   "EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
				   "Slots%22%3Afalse%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"   \
				   "lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22"  \
				   "UseExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Atrue%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22"       \
				   "ResetTodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22STOCK%2FUS%2FXNAS%2F"

/*
 * Period: 1m ALL TICKS FROM START OF DAY
 */
#define WSJ_ALL    "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
				   "EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
				   "Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
				   "lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Atrue%2C%22ShowAfterHours%22%3Atrue%2C%22U"   \
				   "seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Rese"   \
				   "tTodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22STOCK%2FUS%2FXNAS%2F"

/* 1m LAST TWO CURRENT TICKS */
#define WSJ_CUR    "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22PT1M%22%2C%22" \
				   "EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
				   "Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
				   "lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Atrue%2C%22ShowAfterHours%22%3Atrue%2C%22"    \
				   "UseExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22"      \
				   "ResetTodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22STOCK%2FUS%2FXNAS%2F"


/*
 * OHLC + VOLUME
 */
#define WSJ_OHLC   "%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
				   "%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"   \
				   "%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

/*
 * CRYPTO
 */

#define WSJ_CRYPTO_ALL "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"    \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"       \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"        \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U"     \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"      \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22CRYPTOCURRENCY%2FUS%2FCOINDESK%2F"  \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"       \
					"%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"       \
					"%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

/*
 * INDEX & FUND ALLDAY
 */

#define WSJ_SPX_ALL "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FS%26P%20US%2FSPX"  \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"   \
					"%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_NDX_ALL "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FXNAS%2FNDX"        \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"   \
					"%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_DOW_ALL "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
				    "TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FDOW%20JONES%20GLOBAL%2FDJIA" \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"   \
					"%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_RUT_ALL "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2F%2FRUT"            \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"   \
					"%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n" 


#define WSJ_FUN_ALL "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
				    "TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22FUND%2FUS%2FARCX%2F"

/*
 * INDEX & FUND CURRENT
 */

#define WSJ_SPX_CUR "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22PT1M%22%2C%22" \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FS%26P%20US%2FSPX"  \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"   \
					"%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_NDX_CUR "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22PT1M%22%2C%22" \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FXNAS%2FNDX"        \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"   \
					"%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_DOW_CUR "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22PT1M%22%2C%22" \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
				    "TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FDOW%20JONES%20GLOBAL%2FDJIA" \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"   \
					"%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_RUT_CUR "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22PT1M%22%2C%22" \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2F%2FRUT"            \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A"   \
					"%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_FUN_CUR "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22PT1M%22%2C%22" \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
				    "TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22FUND%2FUS%2FARCX%2F"

/*
 * INDEX & FUND CLOSE
 */

#define WSJ_SPX_CLO "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"    \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FS%26P%20US%2FSPX"  \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%5D%2C%22Kind%22%3A%22Volume%2"        \
					"2%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_NDX_CLO "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"    \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FXNAS%2FNDX"        \
            	    "%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%5D%2C%22Kind%22%3A%22Volume%2"        \
					"2%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_DOW_CLO "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"    \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
				    "TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FDOW%20JONES%20GLOBAL%2FDJIA" \
                    "%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
                    "%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%5D%2C%22Kind%22%3A%22Volume%2"        \
                    "2%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_RUT_CLO "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"    \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2F%2FRUT"            \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTy"      \
					"pes%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%5D%2C%22Kind%22%3A%22Volume%2"     \
					"2%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_FUN_CLO "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"    \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"  \
				    "TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22FUND%2FUS%2FARCX%2F"


#define WSJ_SPX_CLO2 "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Atrue%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"   \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FS%26P%20US%2FSPX"  \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%5D%2C%22Kind%22%3A%22Volume%2"        \
					"2%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_NDX_CLO2 "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Atrue%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"   \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FXNAS%2FNDX"        \
            	    "%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
					"%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%5D%2C%22Kind%22%3A%22Volume%2"        \
					"2%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_DOW_CLO2 "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Atrue%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"   \
				    "TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2FDOW%20JONES%20GLOBAL%2FDJIA" \
                    "%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes"   \
                    "%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%5D%2C%22Kind%22%3A%22Volume%2"        \
                    "2%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_RUT_CLO2 "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Atrue%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"   \
					"TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22INDEX%2FUS%2F%2FRUT"            \
					"%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTy"      \
					"pes%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%5D%2C%22Kind%22%3A%22Volume%2"     \
					"2%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1\r\n"

#define WSJ_FUN_CLO2 "GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22P1D%22%2C%22TimeFrame%22%3A%22D1%22%2C%22"   \
					"EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNull"   \
					"Slots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialC"    \
					"lose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22U" \
					"seExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Atrue%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22Reset"   \
				    "TodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22FUND%2FUS%2FARCX%2F"

/*
 * INDICATORS
 */
#define WSJ_VTX "https://api.wsj.net/api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1" \
				"%22%2C%22StartDate%22%3A1593820800000%2C%22EndDate%22%3A1593820800000%2C%22EntitlementToken%22%3A%2257494d5ed7ad4" \
				"4af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNullSlots%22%3Afalse%2C%22FilterClosedPoints%22%" \
				"3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialClose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22Sho" \
				"wPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22UseExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3" \
				"Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22ResetTodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%" \
				"5B%7B%22Key%22%3A%22STOCK%2FUS%2FXNAS%2F"

#define WSJ_VTX2 "%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%2" \
				 "2SeriesId%22%3A%22s1%22%2C%22DataTypes%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%7B" \
				 "%22Name%22%3A%22ShowOpen%22%7D%2C%7B%22Name%22%3A%22ShowHigh%22%7D%2C%7B%22Name%22%3A%22ShowLow%22%7D%2C%7B%22Nam" \
				 "e%22%3A%22ShowPriorClose%22%2C%22Value%22%3Atrue%7D%2C%7B%22Name%22%3A%22Show52WeekHigh%22%7D%2C%7B%22Name%22%3A%" \
				 "22Show52WeekLow%22%7D%5D%2C%22Kind%22%3A%22OpenHighLowLines%22%2C%22SeriesId%22%3A%22i4%22%7D%2C%7B%22Parameters%" \
				 "22%3A%5B%7B%22Name%22%3A%22Period%22%2C%22Value%22%3A14%7D%5D%2C%22Kind%22%3A%22VortexIndicator%22%2C%22SeriesId%" \
				 "22%3A%22i5%22%7D%5D%7D%5D%7D&ckey=57494d5ed7"

#define MACD     "https://api.wsj.net/api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1" \
				 "%22%2C%22StartDate%22%3A1595030400000%2C%22EndDate%22%3A1595030400000%2C%22EntitlementToken%22%3A%2257494d5ed7ad4" \
				 "4af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNullSlots%22%3Afalse%2C%22FilterClosedPoints%22%" \
				 "3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialClose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22Sho" \
				 "wPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22UseExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3" \
				 "Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22ResetTodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A"  \
				 "%5B%7B%22Key%22%3A%22STOCK%2FUS%2FXNAS%2F"

#define MACD2    "%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C"                                               \
				 "%22SeriesId%22%3A%22s1%22%2C%22DataTypes%22%3A%5B%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B"  \
				 "%7B%22Name%22%3A%22ShowOpen%22%7D%2C%7B%22Name%22%3A%22ShowHigh%22%7D%2C%7B%22Name%22%3A%22ShowLow%22%7D%2C%7B%22" \
				 "Name%22%3A%22ShowPriorClose%22%2C%22Value%22%3Atrue%7D%2C%7B%22Name%22%3A%22Show52WeekHigh%22%7D%2C%7B%22Name%22%" \
				 "3A%22Show52WeekLow%22%7D%5D%2C%22Kind%22%3A%22OpenHighLowLines%22%2C%22SeriesId%22%3A%22i4%22%7D%2C%7B%22Paramete" \
				 "rs%22%3A%5B%7B%22Name%22%3A%22EMA1%22%2C%22Value%22%3A12%7D%2C%7B%22Name%22%3A%22EMA2%22%2C%22Value%22%3A26%7D%2C" \
				 "%7B%22Name%22%3A%22SignalLine%22%2C%22Value%22%3A9%7D%5D%2C%22Kind%22%3A%22MovingAverageConvergenceDivergence%22%" \
				 "2C%22SeriesId%22%3A%22i5%22%7D%5D%7D%5D%7D&ckey=57494d5ed7"


/* **********
 *
 * Yahoo
 *
 ***********/
/* Yahoo Stocks API */
#define YWATCH             "https://query1.finance.yahoo.com/v7/finance/quote?formatted=true&crumb=PExTakFX2zL&symbols="
#define YWATCH2            "&fields=longName%2CpriceHint%2Csymbol%2CregularMarketPrice%2CregularMarketChange%2CregularMarketChangePercent%2Ccurrency%2CregularMarketTime%2CgmtOffSetMilliseconds%2CtimeZoneShortName%2CexchangeTimezoneShortName%2CregularMarketVolume%2Cquantity%2CaverageDailyVolume3Month%2CregularMarketDayHigh%2CregularMarketDayLow%2CregularMarketPrice%2CregularMarketOpen%2CfiftyTwoWeekHigh%2CfiftyTwoWeekLow%2CregularMarketPrice%2CregularMarketOpen%2Csparkline%2CmarketCap%2CtradeButton&corsDomain=finance.yahoo.com"
#define YAHOO_OHLC         "https://query1.finance.yahoo.com/v8/finance/chart/%s?range=1d&interval=1m&includePrePost=False"
#define YAHOO_EOD          "https://query1.finance.yahoo.com/v8/finance/chart/%s?interval=1d"
#define YAHOO_CLOSE        "https://query2.finance.yahoo.com/v8/finance/chart/%s?formatted=true&crumb=tAPc%2FmxouN0&lang=en-US&interval=1d&period1=%d&period2=%d&events=div%7Csplit&corsDomain=finance.yahoo.com"
#define YAHOO_FIVE_YEAR    "https://finance.yahoo.com/quote/%s/history?period1=1438387200&period2=1596240000&interval=1d&filter=history&frequency=1d"
#define YAHOO_HISTORY      "https://query1.finance.yahoo.com/v7/finance/download/%s?period1=%lu&period2=%lu&interval=1d&events=history"

#define YAHOO_HISTORY2      "GET /v7/finance/download/%s?period1=%lu&period2=%lu&interval=1d&events=history HTTP/1.0\r\n"     \
                           "Host: query1.finance.yahoo.com\r\n"                                                               \
                           "Accept: */*\r\n" \
                           "Accept-Encoding: gzip,deflate\r\n"                                                                \
                           "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:77.0) Gecko/20100101 Firefox/77.0\r\n\r\n"

/* Yahoo Options API */
#define YAHOO_OPTIONS      "https://query1.finance.yahoo.com/v7/finance/options/"
#define YAHOO_OPTIONS_DATE "https://query1.finance.yahoo.com/v7/finance/options/%s?date=%lu"
#define YAHOO_OPTIONS_1D   "https://query1.finance.yahoo.com/v8/finance/chart/%s?period1=%lu&period2=%lu&interval=1d"
#define YAHOO_OPTIONS_15M  "https://query1.finance.yahoo.com/v8/finance/chart/%s?period1=%lu&period2=%lu&interval=15m"
#define YAHOO_OPTIONS_1M   "https://query1.finance.yahoo.com/v8/finance/chart/%s?period1=%lu&period2=%lu&interval=1m"
#define YAHOO_OPTION_CHECK "https://finance.yahoo.com/quote/%s/options/"

/* **********
 *
 * Crypto
 *
 ***********/
#define WEBSOCKET_CLIENT                "GET /v2?format=streamer HTTP/1.1\r\n" \
										"Host: streamer.cryptocompare.com\r\n" \
										"Sec-WebSocket-Version: 13\r\n" \
										"Sec-WebSocket-Key: ez+qrkibwLq1++fta7rJcQ==\r\n" \
										"Sec-Fetch-Dest: websocket\r\n"   \
										"Sec-Fetch-Mode: websocket\r\n"   \
										"Sec-Fetch-Site: same-origin\r\n" \
										"Pragma: no-cache\r\n"            \
										"Cache-Control: no-cache\r\n"     \
										"Upgrade: websocket\r\n\r\n"      \

/* *************
 *
 * defense.gov
 *
 **************/
#define DEFENSE_GOV_PAGE     "https://www.defense.gov/News/Contracts/?Page=%d"
#define DEFENSE_GOV_CONTRACT "https://www.defense.gov/News/Contracts/Article/%s/"

#endif
