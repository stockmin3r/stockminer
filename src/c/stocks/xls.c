#include <conf.h>
#include <stocks/stocks.h>

#define XLS_TABLE "XLS %s <table class=XLS style=width:100%%;margin-bottom:20px;>" \
					"<caption>%s (%d)</caption>" \
					"<span id=fullxls class=icon onclick=\"full_xls()\">FullXLS</span>" \
					"<thead><tr>"          \
				    "<th>Date</th>"        \
					"<th>Open</th>"        \
					"<th>High</th>"        \
					"<th>Low</th>"         \
					"<th>Close</th>"       \
					"<th>#days</th>"       \
					"<th>5r</th>"          \
					"<th>#days</th>"       \
					"<th>10r</th>"         \
					"<th>%%Pk</th>"        \
					"<th>Peak</th>"        \
					"<th>Up</th>"          \
					"<th>Down</th>"        \
					"<th>RSI</th>"         \
					"<th>mag1</th>"        \
					"<th>1</th>"           \
					"<th>mag2</th>"        \
					"<th>2</th>"           \
					"<th>-2</th>"          \
					"<th>Action1</th>"     \
					"<th>a1ESP</th>"       \
					"<th>Action4</th>"     \
					"<th>a4ESP</th>"       \
					"<th>ΔOpen</th>"       \
					"<th>ΔHigh</th>"       \
					"<th>ΔLow</th>"        \
					"<th>ΔClose</th>"      \
					"<th colspan=6 scope=colgroup></th>" \
					"</tr></thead><tbody>" \

#define XLS_DEF       "[{\"data\":\"DT\"},{\"data\":\"O\"},{\"data\":\"H\"},{\"data\":\"L\"},{\"data\":\"C\"}," \
                       "{\"data\":\"5D\"},{\"data\":\"5Dr\"},"                                                  \
                       "{\"data\":\"10D\"},{\"data\":\"10Dr\"},"                                                \
                       "{\"data\":\"PK\"},{\"data\":\"PKK\"},{\"data\":\"UP\"},{\"data\":\"DN\"},{\"data\":\"RSI\"},{\"data\":\"mag1\"},{\"data\":\"M1\"},{\"data\":\"mag2\"},{\"data\":\"M2\"}," \
                       "{\"data\":\"A1\"},{\"data\":\"A1ESP\"},{\"data\":\"A4\"},{\"data\":\"A4ESP\"},{\"data\":\"DO\"},{\"data\":\"DH\"},{\"data\":\"DL\"},{\"data\":\"DC\"}]" 

#define XLS_DATA      "[{\"data\":\"%s\"},{\"data\":\"%.2f\"},{\"data\":\"%.2f\"},{\"data\":\"%.2f\"},{\"data\":\"%.2f\"},"  \
                       "{\"data\":\"%d\"},{\"data\":\"%.2f\"},{\"data\":\"%d\"},{\"data\":\"%.2f\"},"                        \
                       "{\"data\":\"%s\"},{\"data\":\"%.2f\"},{\"data\":\"%.2f\"},{\"data\":\"%.2f\"},{\"data\":\"%.2f\"},"  \
					   "{\"data\":\"%d\"},{\"data\":\"%d\"},{\"data\":\"%d\"},{\"data\":\"%d\"},"                            \
                       "{\"data\":\"%s\"},{\"data\":\"%s\"},{\"data\":\"%s\"},{\"data\":\"%s\"},"                            \
					   "{\"data\":\"%.2f\"},{\"data\":\"%.2f\"},{\"data\":\"%.2f\"},{\"data\":\"%.2f\"}]"                    \

char *XLS_ENTRY =  "<tr>"
						"<td>%s</td>"         /* Date      */
						"<td>%.2f</td>"       /* Open      */
						"<td>%.2f</td>"       /* High      */
						"<td>%.2f</td>"       /* Low       */
						"<td>%.2f</td>"       /* Close     */
					    "<td>%d</td>"         /* #days 5%  */
						"<td>%.2f</td>"       /* return 5  */
						"<td>%d</td>"         /* #days 10% */
						"<td>%.2f</td>"       /* return 10 */
						"<td>%s</td>"         /* Peak %    */
						"<td>%.2f</td>"       /* Peak      */
						"<td>%.2f</td>"       /* Up        */
						"<td>%.2f</td>"       /* Down      */
						"<td %s>%.2f</td>"    /* RSI       */
						"<td>%d</td>"         /* mag1      */
						"<td %s>%d</td>"      /*   1       */
						"<td>%d</td>"         /* mag2      */
						"<td %s>%d</td>"      /*  2        */
						"<td %s>%d</td>"      /* -2        */
						"<td>%s</td>"         /* Action 1  */
						"<td>%s</td>"         /* a1ESP     */
						"<td>%s</td>"         /* Action 4  */
						"<td>%s</td>"         /* a4ESP     */
						"<td>%.2f</td>"       /* ΔOpen     */
						"<td %s>%.2f</td>"    /* ΔHigh     */
						"<td %s>%.2f</td>"    /* ΔLow      */
						"<td>%.2f</td>"       /* ΔClose    */
					"</tr>";

#define XLS_SBTABLE "<table class=XLS style=width:100%%;margin-bottom:20px;>" \
					"<caption>%s(%d) - %s</caption>" \
					"<span id=fullxls class=icon onclick=\"full_xls()\">Full XLS</span>" \
					"<thead>"\
					"<tr class=trx>"\
					"<th colspan=8></th>"\
					"<th colspan=12>5%% return - action 1</th>" \
					"<th colspan=12>5%% return - action 4</th>" \
					"</tr>"\
					"<tr class=trx>"\
					"<th></th>"\
					"<th colspan=7></th>"\
					"<th colspan=2 scope=colgroup>2020</th>"\
					"<th colspan=2 scope=colgroup>2019</th>"\
					"<th colspan=2 scope=colgroup>2018</th>"\
					"<th colspan=2 scope=colgroup>2017</th>"\
					"<th colspan=4 scope=colgroup>Total</th>"\
					"<th colspan=2 scope=colgroup>2020</th>"\
					"<th colspan=2 scope=colgroup>2019</th>"\
					"<th colspan=2 scope=colgroup>2018</th>"\
					"<th colspan=2 scope=colgroup>2017</th>"\
					"<th colspan=4 scope=colgroup>Total</th>"\
					"<th></th>"\
					"</tr>"\
					"<tr class=trx>"\
                    "<th name=ticker>Ticker</th>"\
                    "<th name=rank>Rank</th>"\
                    "<th name=price>Price</th>"\
                    "<th name=chgpc>%% Change</th>"\
                    "<th name=1yrpkpc>%% Pk</th>"\
					"<th name=1yrpk>Peak</th>"\
					"<th name=one>One</th>"\
					"<th name=two>Two</th>"\
					"<th name=a1q1_2020>Q1</th>"\
					"<th name=a1q2_2020>Q2</th>"\
					"<th name=a1q1_2019>Q1</th>"\
					"<th name=a1q2_2019>Q2</th>"\
					"<th name=a1q1_2018>Q1</th>"\
					"<th name=a1q2_2018>Q2</th>"\
					"<th name=a1q1_2017>Q1</th>"\
					"<th name=a1q2_2017>Q2</th>"\
					"<th name=total1>20</th>"\
					"<th name=total2>19</th>"\
					"<th name=total3>18</th>"\
					"<th name=total4>17</th>"\
					"<th name=a4q1_2020>1Q</th>"\
					"<th name=a4q2_2020>2Q</th>"\
					"<th name=a4q1_2019>1Q</th>"\
					"<th name=a4q2_2019>2Q</th>"\
					"<th name=a4q1_2018>1Q</th>"\
					"<th name=a4q2_2018>2Q</th>"\
					"<th name=a4q1_2017>1Q</th>"\
					"<th name=a4q2_2017>2Q</th>"\
					"<th name=total5>20</th>"\
					"<th name=total6>19</th>"\
					"<th name=total7>18</th>"\
					"<th name=total8>17</th>"\
					"<th>EDAY</th>"\
					"</tr>"\
					"<tr>"\
                    "<th name=ticker_td>%s</th>"\
                    "<th name=rank_td>%d</th>"\
                    "<th name=price_td>%.2f</th>"\
                    "<th name=chgpc_td>%.2f</th>"\
                    "<th name=1yrpkpc_td>%s</th>"\
                    "<th name=1yrpk_td>%.2f</th>"\
					"<th name=one_td %s>%d</th>"\
					"<th name=two_td %s>%d</th>"\
					"<th name=a1q1_2020_td %s>%s</th>"\
					"<th name=a1q2_2020_td %s>%s</th>"\
					"<th name=a1q1_2019_td %s>%s</th>"\
					"<th name=a1q2_2019_td %s>%s</th>"\
					"<th name=a1q1_2018_td %s>%s</th>"\
					"<th name=a1q2_2018_td %s>%s</th>"\
					"<th name=a1q1_2017_td %s>%s</th>"\
					"<th name=a1q2_2017_td %s>%s</th>"\
					"<th name=total1_td>%d</th>"\
					"<th name=total2_td>%d</th>"\
					"<th name=total3_td>%d</th>"\
					"<th name=total4_td>%d</th>"\
					"<th name=a4q1_2020_td %s>%s</th>"\
					"<th name=a4q2_2020_td %s>%s</th>"\
					"<th name=a4q1_2019_td %s>%s</th>"\
					"<th name=a4q2_2019_td %s>%s</th>"\
					"<th name=a4q1_2018_td %s>%s</th>"\
					"<th name=a4q2_2018_td %s>%s</th>"\
					"<th name=a4q1_2017_td %s>%s</th>"\
					"<th name=a4q2_2017_td %s>%s</th>"\
					"<th name=total5_td>%d</th>"\
					"<th name=total6_td>%d</th>"\
					"<th name=total7_td>%d</th>"\
					"<th name=total8_td>%d</th>"\
					"<th name=earn_td>%s</th>"\
					"</tr>"\
					"<tr>"\
				    "<th>Date</th>"        \
					"<th>Open</th>"        \
					"<th>High</th>"        \
					"<th>Low</th>"         \
					"<th>Close</th>"       \
					"<th>#days</th>"       \
					"<th>5r</th>"          \
					"<th>#days</th>"       \
					"<th>10r</th>"         \
					"<th>%% Pk</th>"       \
					"<th>Peak</th>"        \
					"<th>Up</th>"          \
					"<th>Down</th>"        \
					"<th>RSI</th>"         \
					"<th>mag1</th>"        \
					"<th>1</th>"           \
					"<th>mag2</th>"        \
					"<th>2</th>"           \
					"<th>-2</th>"          \
					"<th>Action1</th>"     \
					"<th>a1ESP</th>"       \
					"<th>Action4</th>"     \
					"<th>a4ESP</th>"       \
					"<th>ΔOpen</th>"       \
					"<th>ΔHigh</th>"       \
					"<th>ΔLow</th>"        \
					"<th>ΔClose</th>"      \
					"<th colspan=6 scope=colgroup></th>" \
					"</tr></thead><tbody>" \


char *XPAGE = "<html><head>"
			  "<meta charset=\"utf-8\"/>"
			  "<link rel=icon href=\"data:;base64,=\">"
			  "<link rel=\"shortcut icon\" href=\"data:image/x-icon;,\" type=image/x-icon>"
			  "<title>%s</title>"
				"<style>"
					".grn{background-color:green}.orange{background-color:orange}.yellow{background-color:yellow}"
					"h3{text-align:center;color:brown;margin-top:10px}"
					"caption{display:none}"
					"span{display:none}"
					"th{"
						"color:white;"
						"padding:8px;"
						"text-align:center;"
						"border:1px outset grey;"
						"background-color:#23234F}"
					"table{"
						"border-collapse:collapse;"
						"font-size:12.5px;"
						"font-family:Arial;"
						"line-height:0.8;"
						"color:black;"
						"border-bottom-width:2px;"
						"border-bottom-style:solid;"
						"border:2px solid #2f3136;"
						"border-left-color:rgb(47, 49, 54);"
						"border-left-style:solid;"
						"border-left-width: 2px;"
						"border-right-style:solid;"
						"border-right-color:rgb(47, 49, 54);"
						"border-right-width:2px;"
						"border-spacing:2px 2px;"
						"border-top-color:rgb(47, 49, 54);"
						"border-top-style:solid;"
						"border-top-width:2px;"
						"border-bottom-color:rgb(47, 49, 54)}"
					"thead th{position:sticky;top:0}"
					"td{"
						"border:1px solid rgb(128, 128, 128, 0.2) !important;"
						"font-size:16px;"
						"line-height:normal;"
						"padding-bottom:8px;"
						"padding-left:5px;"
						"padding-right:5px;"
						"border-collapse:collapse;"
						"border-spacing:2px 2px;"
						"padding-top:8px;"
						"text-align:center;"
						"border-spacing:0}"
					"</style>"
					"</head>"
					"<body><h3>%s</h3>%s</body></html>";

#define LBX_TABLE  "<table id=LBX class=\"LBX_%s xls\" style=width:100%%;margin-bottom:20px;>" \
					"<caption class=sigcap>%s (%d)</caption>" \
					"<span id=fullxls class=icon onclick=\"full_xls()\">Full XLS</span>" \
					"<span id=daybutton class=icon onclick=\"daytrade()\" title=\"Daytrading Algorithm\">Daytrade</span>" \
					"<span id=xaction1 class=icon onclick=\"full_action1()\" title=\"Action 1 XLS\">Action 1</span>" \
					"<span id=xaction4 class=icon onclick=\"full_action4()\" title=\"Action 4 XLS\">Action 4</span>" \
					"<thead><tr>"          \
				    "<th>Date</th>"        \
					"<th>Open</th>"        \
					"<th>High</th>"        \
					"<th>Low</th>"         \
					"<th>Close</th>"       \
					"<th>#days</th>"       \
					"<th>5r</th>"          \
					"<th>#days</th>"       \
					"<th>10r</th>"         \
					"<th>%% Pk</th>"       \
					"<th>Peak</th>"        \
					"<th>Up</th>"          \
					"<th>Down</th>"        \
					"<th>RSI</th>"         \
					"<th>mag1</th>"        \
					"<th>1</th>"           \
					"<th>mag2</th>"        \
					"<th>2</th>"           \
					"<th>-2</th>"          \
					"<th>Action1</th>"     \
					"<th>a1ESP</th>"       \
					"<th>Action4</th>"     \
					"<th>a4ESP</th>"       \
					"<th>ΔOpen</th>"       \
					"<th>ΔHigh</th>"       \
					"<th>ΔLow</th>"        \
					"<th>ΔClose</th>"      \
					"</tr></thead><tbody>" \


static __inline__ int XLS_ADD_ENTRY(char *tptr, struct mag *mag, int entry)
{
	char *action1, *action4, *date, *one_color, *two_color, *rsi_color;
	char *dlow_color, *dhigh_color;
	double dopen, dhigh, dlow, dclose;
	double day_open, day_high, day_low, day_close, prior_close, up, down, rsi, peak_pc, peak_yr;
	double days_ret5, days_ret10;
	int nr_entries, rsize, one, two, neg1, neg2, mag1, mag2, days_5pc, days_10pc, a1esp,a4esp;
	char buf[256];

	date        = mag->date[entry];
	day_open    = mag->open[entry];
	day_close   = mag->close[entry];
	prior_close = mag->close[entry-1];
	day_high    = mag->high[entry];
	day_low     = mag->low[entry];
	mag1        = mag->mag1[entry];
	one         = mag->one[entry];
	neg1        = mag->neg1[entry];
	peak_yr     = mag->peak_yr[entry];
	peak_pc     = mag->peak_pc[entry];
	up          = mag->up[entry];
	down        = mag->down[entry];
	rsi         = mag->ratio[entry];
	mag2        = mag->mag2[entry];
	two         = mag->two[entry];
	neg2        = mag->neg2[entry];
	days_5pc    = mag->days_5pc[entry];
	days_ret5   = mag->days_ret5[entry];
	days_10pc   = mag->days_10pc[entry];
	days_ret10  = mag->days_ret10[entry];
	dopen       = mag->delta_open[entry];
	dhigh       = mag->delta_high[entry];
	dlow        = mag->delta_low[entry];
	dclose      = mag->delta_close[entry];
	a1esp       = mag->a1esp[entry].a1esp;
	a4esp       = mag->a4esp[entry].a4esp;

	/* ********
	 * ACTION1
	 *********/
	if (two <= 2 && (entry > 15))
		action1 = "Buy";
	else
		action1 = "";

	/**********
	 * ACTION4
	 *********/
	if (one >= 9 && two <= 3)
		action4 = "Buy";
	else
		action4 = "";

	if (is_delta_high(&mag->delta_high[entry-9], dhigh))
		dhigh_color = GREEN_COLOR;
	else
		dhigh_color = NO_COLOR;

	if (is_delta_low(&mag->delta_low[entry-9], dlow))
		dlow_color = ORANGE_COLOR;
	else
		dlow_color = NO_COLOR;

	/* Cell Colors */
	if (one == 9)
		one_color = YELLOW_COLOR;
	else if (one >= 10)
		one_color = GREEN_COLOR;
	else
		one_color = NO_COLOR;

	if (two == 3)
		two_color = YELLOW_COLOR;
	else if (two <= 2 && entry > 15)
		two_color = ORANGE_COLOR;
	else
		two_color = NO_COLOR;

	if (rsi >= 2)
		rsi_color = GREEN_COLOR;
	else
		rsi_color = NO_COLOR;
	rsize = sprintf(tptr, XLS_DATA,  date, day_open, day_high, day_low, day_close,
									 days_5pc, days_ret5, days_10pc, days_ret10,
								     stock_peak(peak_pc, buf), peak_yr, up, down, rsi,
									 mag1, one ,mag2, two,
									 action1, esp_str(a1esp), action4, esp_str(a4esp),
									 dopen, dhigh, dlow, dclose);

/*	rsize = sprintf(tptr, XLS_ENTRY, date, day_open, day_high, day_low, day_close,
									 days_5pc, days_ret5, days_10pc, days_ret10,
								     stock_peak(peak_pc, buf), peak_yr, up, down, rsi_color, rsi,
									 mag1, one_color, one,
									 mag2, two_color, two, (neg2>=6) ? GREEN_COLOR:NO_COLOR, neg2, action1, esp_str(a1esp), action4, esp_str(a4esp),
									 dopen, dhigh_color, dhigh, dlow_color, dlow, dclose);*/
	return (rsize);
}

/* unused */
char *stock_xls(char *ticker, char *div, int *xls_len)
{
	struct stock *stock;
	struct mag   *mag;
	char         *table, *tptr, *one_color, *two_color;
	int           tsize = 0, nr_entries, rsize, x, one, two;
	char pbuf[64];   char qbuf1[64];  char qbuf2[64];
	char qbuf3[64];  char qbuf4[64];  char qbuf5[64];  char qbuf6[64];
	char qbuf7[64];  char qbuf8[64];  char qbuf9[64];  char qbuf10[64];
	char qbuf11[64]; char qbuf12[64]; char qbuf13[64]; char qbuf14[64];
	char qbuf15[64]; char qbuf16[64]; char ebuf[64];

	table = (char *)malloc(2048 KB);
	stock = search_stocks(ticker);
	if (!stock || !(mag=stock->mag))
		goto out_error;
	tptr = table;

	if (stock->rank != -1) {
		struct quarter *quarter_2020 = &stock->mag->quarters[0];
		struct quarter *quarter_2019 = &stock->mag->quarters[1];
		struct quarter *quarter_2018 = &stock->mag->quarters[2];
		struct quarter *quarter_2017 = &stock->mag->quarters[3];
		int total1 = quarter_2020->action1_total;
		int total2 = quarter_2019->action1_total;
		int total3 = quarter_2018->action1_total;
		int total4 = quarter_2017->action1_total;
		int total5 = quarter_2020->action4_total;
		int total6 = quarter_2019->action4_total;
		int total7 = quarter_2018->action4_total;
		int total8 = quarter_2017->action4_total;
		/* Cell Colors */
		one = stock->mag->one[stock->mag->nr_entries-1];
		if (one == 9)
			one_color = YELLOW_COLOR;
		else if (one >= 10)
			one_color = GREEN_COLOR;
		else
			one_color = NO_COLOR;
	
		two = stock->mag->two[stock->mag->nr_entries-1];
		if (two == 3)
			two_color = YELLOW_COLOR;
		else if (two <= 2)
			two_color = ORANGE_COLOR;
		else
			two_color = NO_COLOR;

		tsize  = snprintf(table, 8192*4, XLS_SBTABLE, stock->sym, stock->rank, stock->name, stock->sym, stock->rank, stock->current_price, stock->pr_percent, stock_peak(stock->peak_1year_pc, pbuf), stock->peak_1year,
			one_color, stock->mag->one[stock->mag->nr_entries-1], two_color, stock->mag->two[stock->mag->nr_entries-1],
			quarter_color(quarter_2020->action1_q1r), quarter_value(quarter_2020->action1_q1r, qbuf1),  quarter_color(quarter_2020->action1_q2r), quarter_value(quarter_2020->action1_q2r, qbuf2),
			quarter_color(quarter_2019->action1_q1r), quarter_value(quarter_2019->action1_q1r, qbuf3),  quarter_color(quarter_2019->action1_q2r), quarter_value(quarter_2019->action1_q2r, qbuf4),
			quarter_color(quarter_2018->action1_q1r), quarter_value(quarter_2018->action1_q1r, qbuf5),  quarter_color(quarter_2018->action1_q2r), quarter_value(quarter_2018->action1_q2r, qbuf6),
			quarter_color(quarter_2017->action1_q1r), quarter_value(quarter_2017->action1_q1r, qbuf7),  quarter_color(quarter_2017->action1_q2r), quarter_value(quarter_2017->action1_q2r, qbuf8), total1, total2, total3, total4,
			quarter_color(quarter_2020->action4_q1r), quarter_value(quarter_2020->action4_q1r, qbuf9),  quarter_color(quarter_2020->action4_q2r), quarter_value(quarter_2020->action4_q2r, qbuf10),
			quarter_color(quarter_2019->action4_q1r), quarter_value(quarter_2019->action4_q1r, qbuf11), quarter_color(quarter_2019->action4_q2r), quarter_value(quarter_2019->action4_q2r, qbuf12),
			quarter_color(quarter_2018->action4_q1r), quarter_value(quarter_2018->action4_q1r, qbuf13), quarter_color(quarter_2018->action4_q2r), quarter_value(quarter_2018->action4_q2r, qbuf14),
			quarter_color(quarter_2017->action4_q1r), quarter_value(quarter_2017->action4_q1r, qbuf15), quarter_color(quarter_2017->action4_q2r), quarter_value(quarter_2017->action4_q2r, qbuf16), total5, total6, total7, total8, earning_days(stock->earnings.earning_days, ebuf));
	} else {
		tsize  = sprintf(tptr, XLS_TABLE, div, stock->sym, stock->rank);
	}
	tptr  += tsize;
	nr_entries = mag->nr_entries;
	if (nr_entries <= 0 || nr_entries < 17)
		goto out_error;
	for (x=0; x<16; x++) {
		rsize = XLS_ADD_ENTRY(tptr, mag, nr_entries-16);
//		if (!strcmp(ticker, "MSFT")) {
//			printf("TICKER: %s date: %s x: %d mag2: %d two: %d addr: %p\n", ticker, mag->date[nr_entries-16]?mag->date[nr_entries-16]:"no date",x,  mag->mag2[nr_entries-16], mag->two[nr_entries-16], &mag->two[nr_entries-16]);
//		}
		tptr  += rsize;
		tsize += rsize;
		nr_entries += 1;
	}
	strcpy(tptr, "</tbody></table>");
	tsize   += 16;
	*xls_len = tsize;
	return (table);
out_error:
	return NULL;
}

int lightbox_xls(char *ticker, char *table)
{
	struct stock *stock;
	struct mag   *mag;
	char         *tptr;
	int           tsize = 0, nr_entries, rsize, x;

	stock = search_stocks(ticker);
	if (!stock || !(mag=stock->mag) || ((nr_entries=(mag->nr_entries)) <= 17))
		return 0;

	tptr   = table;
	tsize  = sprintf(tptr, LBX_TABLE, stock->sym, stock->sym, stock->rank);
	tptr  += tsize;
	for (x=0; x<16; x++) {
		rsize  = XLS_ADD_ENTRY(tptr, mag, nr_entries-16);
		tptr  += rsize;
		tsize += rsize;
		nr_entries += 1;
	}
	strcpy(tptr, "</tbody></table>");
	return (tsize+16);
}

#define SEND_VIA_WEBSOCKET 0x7377
#define SEND_VIA_HTTP      0x7078

/* no longer useful */
void xls_mag2(struct connection *connection, char *table, char *div, char *ticker, int nr_entries, int via)
{
	struct stock *stock;
	struct mag *mag;
	char *tptr;
	int tsize = 0, rsize, x;

	stock = search_stocks(ticker);
	if (!stock || !(mag=stock->mag) || ((nr_entries=(mag->nr_entries-1)) <= 0))
		return;

	tptr   = table;
	strcpy(table, "newtab mag2 ");
	tsize = 12;
/*	tsize  = sprintf(tptr, XLS_TABLE, div, stock->sym, stock->rank);*/
	tptr  += tsize;
	for (x=nr_entries; x>=0; x--) {
		rsize = XLS_ADD_ENTRY(tptr, mag, x);
		tptr  += rsize;
		tsize += rsize;
	}
	strcpy(table+tsize-1, "</tbody></table>");
	tsize   += 15;
	if (via == SEND_VIA_WEBSOCKET)
		websocket_send(connection, table, tsize);
}

void rpc_xls(struct rpc *rpc)
{
	struct session *session     = rpc->session;
	char           *packet      = rpc->packet;
	char           *ticker      = rpc->argv[1];
	unsigned short 	via         = *(unsigned short *)(rpc->argv[2]);
	char           *preset_name = rpc->argv[4];
	char           *QGID        = rpc->argv[5];
	char           *TID         = rpc->argv[6];
	struct stock   *stock       = search_stocks(ticker);
	struct wtab    *preset;
	char           *p;
	char            table[2048 KB];
	int             x, len, nr_rows = 0, packet_len = 0;

	/* XLS 1:ABIO 2:ws|xp 3:rowsize 4:PresetName 5:QGID */
	if (!stock || !stock->mag)
		return;

	if (*rpc->argv[3] == 'm')
		nr_rows = stock->mag->nr_entries-1;
	else
		nr_rows = atoi(rpc->argv[3]);

	len = strlen(ticker);
	p   = ticker;
	for (x=0; x<len; x++) {
		char c = *p;
		*p++   = toupper(c);
	}

	// no longer used
	if (!strcmp(preset_name, "mag2")) {
		xls_mag2(rpc->connection, table, QGID, ticker, nr_rows, via);
		return;
	}
	preset  = search_watchtable_preset(session, preset_name, NULL);
	if (!preset)
		return;
	if (nr_rows <= 0 || nr_rows >= stock->mag->nr_entries)
		nr_rows = 16;

	packet = XLS_packet(stock, TID, preset, nr_rows, &packet_len);
	if (!packet || !packet_len)
		return;
	websocket_send(rpc->connection, packet, packet_len);
	free(packet);
}

void xls_mag2_action(char *ticker, char *div, int action, int sockfd)
{
	struct stock *stock;
	struct mag   *mag;
	char          table[2048 KB];
	char         *tptr;
	int           tsize = 0, rsize, nr_entries, buy_signal, x;

	stock = search_stocks(ticker);
	if (!stock || !(mag=stock->mag) || ((nr_entries=(mag->nr_entries)) <= 0))
		return;

	tptr   = table;
	tsize  = sprintf(tptr, XLS_TABLE, div, stock->sym, stock->rank);
	tptr  += tsize;
	for (x=0; x<nr_entries; x++) {
		buy_signal = 0;
		switch (action) {
			case 1:
				if (is_action1(mag, x))
					buy_signal = 1;
				break;
			case 4:
				if (is_action4(mag, x))
					buy_signal = 1;
				break;
		}
		if (!buy_signal)
			continue;
		rsize = XLS_ADD_ENTRY(tptr, mag, x);
		tptr  += rsize;
		tsize += rsize;
	}
	strcpy(table+tsize-1, "</tbody></table>");
	tsize   += 15;
}

void HTTP_XLS(char *req, struct connection *connection)
{
	struct rpc  rpc;
	char        *p;
	char        *argv[16];
	char         packet[1024 KB];
	int          argc;

	/* GET /XLS/ABIO/src/rowsize/type/args */
	p = strchr(req+11, ' ');
	if (!p)
		return;
	*p = 0;
	argc = cstring_split(req+5, argv, 7, '/');
	if (argc <= 0)
		return;

	rpc.session    = NULL;
	rpc.connection = connection;
	rpc.packet     = packet;
	rpc.argc       = argc;
	rpc.argv       = argv;
	rpc_xls(&rpc);
}
