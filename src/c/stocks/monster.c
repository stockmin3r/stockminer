#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

#define CTABLE "<table id=CTAB class=\"SBTAB cell-border no-footer dataTable non-sortable\">" \
				"<thead>"                         \
					"<tr>"                    \
						"<th>Date</th>"   \
						"<th>Open</th>"   \
						"<th>High</th>"   \
						"<th>Low</th>"    \
						"<th>Close</th>"  \
						"<th>Daily</th>"  \
						"<th>3 Day</th>"  \
						"<th>5 Day</th>"  \
						"<th>8 Day</th>"  \
						"<th>13 Day</th>" \
						"<th>21 Day</th>" \
						"<th>Buy</th>"    \
						"<th>Buy Δ</th>"  \
						"<th>Sell</th>"   \
						"<th>Sell Δ</th>" \
					"</tr>"               \
				"</thead>"                \
					"<tbody>"


char *CTABLE_ENTRY =  "<tr>"
						 "<td>%s</td>"       /* Date   */
						 "<td>%.2f</td>"     /* Open   */
						 "<td %s>%.2f</td>"  /* High   */
						 "<td %s>%.2f</td>"  /* Low    */
						 "<td>%.2f</td>"     /* Close  */
						 "<td %s>%.2f</td>"  /* Daily  */
						 "<td %s>%.2f</td>"  /* 3 Day  */
						 "<td %s>%.2f</td>"  /* 5 Day  */
						 "<td %s>%.2f</td>"  /* 8 Day  */
						 "<td %s>%.2f</td>"  /* 13 Day */
						 "<td %s>%.2f</td>"  /* 21 Day */
						 "<td>%.2f</td>"     /* Buy    */
						 "<td>%.2f</td>"     /* Buy Δ  */
						 "<td>%.2f</td>"     /* Sell   */
						 "<td>%.2f</td>"     /* Sell Δ */
					"</tr>";

#define CTABLE_MAX_SIZE 128 KB

static struct stock **sb150_leaders;
static char          *cmaster_table;
static int            cmtable_size;

void sort_peak_stocks(struct XLS *XLS)
{
	struct stock   *stock, **peak_stocks;
	struct monster *monster;
	int i, j, nr_peak_stocks;
	double key;

	monster        = (struct monster *)XLS->MONSTER;
	peak_stocks    = monster->peak_stocks;
	nr_peak_stocks = monster->nr_peak_stocks;
	for (i=1; i<nr_peak_stocks; i++) {
		stock = peak_stocks[i];
		key   = stock->rank;
		j     = i;
		while (j > 0 && peak_stocks[j-1]->rank > key) {
			peak_stocks[j] = peak_stocks[j-1];
			j              = j-1;
		}
		peak_stocks[j] = stock;
	}
}

static __inline__ double MIN5(double a, double b, double c, double d, double e)
{
	double min_ab, min_cd, min4;
	min_ab = a < b ? a : b;
	min_cd = c < d ? c : d;
	min4   = (min_ab < min_cd ? min_ab : min_cd);
	return (min4 < e ? min4 : e);
}

char *day_color(double day)
{
	if (day > -5.0 && day < 5.0)
		return "class=w";
	if (day >= 5.0 && day < 10.0)
		return "class=L";
	if (day >= 10.0)
		return "class=G";
	if (day <= -5.0 && day > -10.0)
		return "class=O";
	if (day <= -10.0 && day > -15.0)
		return "class=Y";
	if (day <= -15.0)
		return "class=R";
	return "class=w";
}

int stock_color_table(struct stock *stock, char *row)
{
	struct mag *mag = stock->mag;
	char *jptr, *daily_color, *high_color, *low_color, *day3_color, *day5_color, *day8_color, *day13_color, *day21_color;
	int tsize = 0, nr_days, start, nbytes, x;
	double buy, buy_delta, sell, sell_delta, daily, day3, day5, day8, day13, day21, day_close, prior_close, high_pc, low_pc;
	double day3_price, day5_price, day8_price, day13_price, day21_price, min;

	jptr    = stock->ctable = (char *)malloc(CTABLE_MAX_SIZE);
	strcpy(jptr, CTABLE);
	jptr   += sizeof(CTABLE)-1;
	tsize   = sizeof(CTABLE)-1;
	start   = mag->year_2020+21;
	nr_days = mag->nr_entries-start;
	for (x=0; x<nr_days; x++) {
		day_close   = mag->close[start];
		prior_close = mag->close[start-1];
		high_pc     = ((mag->high[start]/prior_close)-1)*100.0;
		low_pc      = ((mag->low[start]/prior_close)-1)*100.0;
		daily       = ((day_close/prior_close)-1)*100.0;
		day3_price  = mag->close[start-3];
		day5_price  = mag->close[start-5];
		day8_price  = mag->close[start-8];
		day13_price = mag->close[start-13];
		day21_price = mag->close[start-21];
		day3        = ((day_close/day3_price)-1)*100.0;
		day5        = ((day_close/day5_price)-1)*100.0;
		day8        = ((day_close/day8_price)-1)*100.0;
		day13       = ((day_close/day13_price)-1)*100.0;
		day21       = ((day_close/day21_price)-1)*100.0;
		high_color  = day_color(high_pc);
		low_color   = day_color(low_pc);
		min         = MIN5(day3_price, day5_price, day8_price, day13_price, day21_price);
		buy         = min * 0.95;
		sell        = min * 1.05;
		buy_delta   = ((buy/day_close)-1)*100.0;
		sell_delta  = ((sell/day_close)-1)*100.0;
		daily_color = day_color(daily);
		day3_color  = day_color(day3);
		day5_color  = day_color(day5);
		day8_color  = day_color(day8);
		day13_color = day_color(day13);
		day21_color = day_color(day21);
		nbytes      = sprintf(jptr, CTABLE_ENTRY, mag->date[start], mag->open[start], high_color, high_pc, low_color, low_pc, mag->close[start],
						 	  daily_color, daily, day3_color, day3, day5_color, day5, day8_color, day8, day13_color, day13, day21_color, day21, buy, buy_delta, sell, sell_delta);
		jptr       += nbytes;
		tsize      += nbytes;
		start      += 1;
	}
	stock->ctable_size = tsize;
	start -= 1;
	if (!row)
		return 0;
	return sprintf(row, "{\"rank\":\"%d\",\"stock\":\"%s\",\"date\":\"%s\",\"two\":\"%d\",\"high\":\"%.2f\",\"low\":\"%.2f\",\"close\":\"%.2f\","
						 "\"daily\":\"%.2f\",\"day3\":\"%.2f\",\"day5\":\"%.2f\",\"day8\":\"%.2f\",\"day13\":\"%.2f\",\"day21\":\"%.2f\","
						 "\"buy\":\"%.2f\",\"dbuy\":\"%.2f\",\"sell\":\"%.2f\",\"dsell\":\"%.2f\"},",
					stock->rank, stock->sym, mag->date[start], mag->two[start], high_pc, low_pc, mag->close[start],
					daily, day3, day5, day8, day13, day21, buy, buy_delta, sell, sell_delta);
}

void build_color_table(struct XLS *XLS)
{
	struct stock *stock, **ranked_stocks;
	char *row;
	int nbytes, nr_stocks, x;

	cmaster_table = (char *)malloc(256 KB);
	strcpy(cmaster_table, "{\"data\":[");
	row = cmaster_table + 9;
	cmtable_size = 9;

	nr_stocks     = XLS->nr_stocks;
	ranked_stocks = XLS->ranked_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock  = sb150_leaders[x];
		if (!stock || !stock->mag || !(stock->type & STOCK_TYPE_HIGHCAPS))
			continue;
		if (stock->mag->nr_entries <= 21)
			continue;
		if (x <= 100) {
			nbytes = stock_color_table(stock, row);
			if (x == 100) {
				row[nbytes-1] = ']';
				row[nbytes] = '}';
				row[nbytes+1] = 0;
				cmtable_size += nbytes + 1;
			} else {
				row   += nbytes;
				cmtable_size += nbytes;
			}
		} else {
			stock_color_table(stock, NULL);
		}
	}
}

static void
update_json_sb150(struct XLS *XLS)
{
	struct stock   *stock;
	struct monster *monster;
	char            vstr[32];  char rbuf[64];  char pbuf[64];  char qbuf1[64];
	char            qbuf2[64]; char qbuf3[64]; char qbuf4[64]; char qbuf5[64];
	char            qbuf6[64]; char qbuf7[64]; char qbuf8[64]; char qbuf9[64];
	char            qbuf10[64];char qbuf11[64];char qbuf12[64];char qbuf13[64];
	char            qbuf14[64];char qbuf15[64];char qbuf16[64];
	int             max_size = 32 KB;

	monster              = (struct monster *)XLS->MONSTER;
	monster->sb150_table = (char *)malloc(max_size);
	strcpy(monster->sb150_table, "ctable SB150 [");
	monster->sb150_table_size = 14;
	for (int x=0; x<150; x++) {
		stock = sb150_leaders[x];
		if (!stock || !stock->mag)
			continue;
		struct quarter *quarter_2020 = &stock->mag->quarters[0];
		struct quarter *quarter_2019 = &stock->mag->quarters[1];
		struct quarter *quarter_2018 = &stock->mag->quarters[2];
		struct quarter *quarter_2017 = &stock->mag->quarters[3];
		int total1 = quarter_2020->action1_total;int total2 = quarter_2019->action1_total;
		int total3 = quarter_2018->action1_total;int total4 = quarter_2017->action1_total;int total5 = quarter_2020->action4_total;
		int total6 = quarter_2019->action4_total;int total7 = quarter_2018->action4_total;int total8 = quarter_2017->action4_total;

		monster->sb150_table_size += snprintf(monster->sb150_table+monster->sb150_table_size, 8191, "{\"ticker\":\"%s\",\"rank\":\"%d\",\"price\":\"%.2f\",\"chgpc\":\"%.2f\",\"1yrpkpc\":\"%s\",\"1yrpk\":\"%.2f\",\"one\":\"%d\",\"two\":\"%d\","
			"\"a1q1_2020\":\"%s\",\"a1q2_2020\":\"%s\",\"a1q1_2019\":\"%s\",\"a1q2_2019\":\"%s\","
			"\"a1q1_2018\":\"%s\",\"a1q2_2018\":\"%s\",\"a1q1_2017\":\"%s\",\"a1q2_2017\":\"%s\",\"total1\":\"%d\",\"total2\":\"%d\",\"total3\":\"%d\",\"total4\":\"%d\","
			"\"a4q1_2020\":\"%s\",\"a4q2_2020\":\"%s\",\"a4q1_2019\":\"%s\",\"a4q2_2019\":\"%s\","
			"\"a4q1_2018\":\"%s\",\"a4q2_2018\":\"%s\",\"a4q1_2017\":\"%s\",\"a4q2_2017\":\"%s\",\"total5\":\"%d\",\"total6\":\"%d\",\"total7\":\"%d\",\"total8\":\"%d\"},",
			stock->sym, stock->rank, stock->current_price, stock->pr_percent, stock_peak(stock->peak_1year_pc, pbuf), stock->peak_1year,stock->mag->one[stock->mag->nr_entries-1], stock->mag->two[stock->mag->nr_entries-1],
			quarter_value(quarter_2020->action1_q1r, qbuf1), quarter_value(quarter_2020->action1_q2r, qbuf2), quarter_value(quarter_2019->action1_q1r, qbuf3), quarter_value(quarter_2019->action1_q2r, qbuf4),
			quarter_value(quarter_2018->action1_q1r, qbuf5), quarter_value(quarter_2018->action1_q2r, qbuf6), quarter_value(quarter_2017->action1_q1r, qbuf7), quarter_value(quarter_2017->action1_q2r, qbuf8), total1, total2, total3, total4,
			quarter_value(quarter_2020->action4_q1r, qbuf9), quarter_value(quarter_2020->action4_q2r, qbuf10), quarter_value(quarter_2019->action4_q1r, qbuf11), quarter_value(quarter_2019->action4_q2r, qbuf12),
			quarter_value(quarter_2018->action4_q1r, qbuf13), quarter_value(quarter_2018->action4_q2r, qbuf14), quarter_value(quarter_2017->action4_q1r, qbuf15), quarter_value(quarter_2017->action4_q2r, qbuf16), total5, total6, total7, total8);
		if (monster->sb150_table_size + 2048 >= max_size) {
			max_size += 2048;
			monster->sb150_table = (char *)realloc(monster->sb150_table, max_size);
		}
	}
	monster->sb150_table[monster->sb150_table_size-1] = ']';
	monster->sb150_table[monster->sb150_table_size]   = '@';
	monster->sb150_table[monster->sb150_table_size+1] = 0;
	monster->sb150_table_size += 1;
}

void update_json_esp()
{
	struct stock      *stock;
	struct esp_signal *esp, **esp_signals;
	struct rank_table *rtable;
	struct mag        *mag;
	char *jptr, *esp_string, *action, *nda_string, *a1esp, *a4esp, *a1s, *a4s;
	char pbuf[64];char abuf[32];char ebuf[32];char qbuf1[32],qbuf2[32],qbuf3[32],qbuf4[32];
	int x, r, nbytes = 0;
	double intraday_low, prior_close, close_pc;

	for (r=0; r<NR_RANKS; r++) {
		rtable = &rank_tables[r];
		jptr   = rtable->espbuf;
		strcpy(jptr, "table ESP [");
		nbytes = 11;
		for (x=0; x<rtable->nr_esp_signals; x++) {
			esp = rtable->esp_signals[x];
			if (!esp)
				continue;
			stock = esp->stock;
			if (!stock)
				continue;
			mag = stock->mag;
			if (esp->action == 5)
				action = "both";
			else if (esp->action == 0)
				action = "";
			else {
				sprintf(abuf, "%d", esp->action);
				action = abuf;
			}
			if (!esp->action) {
				if (esp->esp == 5)
					esp_string = "both";
				else {
					sprintf(ebuf, "%d", esp->esp);
					esp_string = ebuf;
				}
			} else
				esp_string = "";

			a1esp = (char *)esp_str(esp->a1esp);
			a1s   = (char *)esp_str(esp->a1s);
			a4esp = (char *)esp_str(esp->a4esp);
			a4s   = (char *)esp_str(esp->a4s);

			if (esp->next_day_action == NEXT_DAY_ACTION)
				nda_string = "Action";
			else if (esp->next_day_action == NEXT_DAY_ESP)
				nda_string = "ESP";
			else
				nda_string = "";

			prior_close = mag->close[mag->nr_entries-1];
			close_pc    = ((mag->close[mag->nr_entries-2]/prior_close)-1)*100.0;
			nbytes     += snprintf(jptr+nbytes, 8191, "{\"T\":\"%s\",\"DA\":\"%s\",\"R\":\"%d\",\"P\":\"%.2f\",\"D\":\"%.2f\",\"CL\":\"%.2f\",\"PK\":\"%s\",\"O\":\"%d\",\"W\":\"%d\","
				"\"a1q1\":\"%s\",\"a1q2\":\"%s\",\"a1T\":\"%d\",\"a4q1\":\"%s\",\"a4q2\":\"%s\",\"a4T\":\"%d\",\"A\":\"%s\",\"esp\":\"%s\",\"a1esp\":\"%s\",\"a1s\":\"%s\","
				"\"a4esp\":\"%s\",\"a4s\":\"%s\",\"nda\":\"%s\",\"next\":\"%.2f\",\"1\":\"%.2f\",\"2\":\"%.2f\",\"3\":\"%.2f\",\"4\":\"%.2f\",\"5\":\"%.2f\",\"6\":\"%.2f\","
				"\"7\":\"%.2f\",\"8\":\"%.2f\",\"9\":\"%.2f\",\"10\":\"%.2f\",\"11\":\"%.2f\",\"12\":\"%.2f\",\"13\":\"%.2f\",\"14\":\"%.2f\",\"15\":\"%.2f\"},",
				esp->ticker, esp->date, stock->rank, esp->price, esp->delta, close_pc, stock_peak2(esp->peak, pbuf), esp->one, esp->two,
				qval(esp->a1q1, qbuf1), qval(esp->a1q2, qbuf2), esp->a1total, qval(esp->a4q1, qbuf3), qval(esp->a4q2, qbuf4), esp->a4total, action, esp_string, a1esp, a1s, a4esp, a4s, nda_string, esp->next_cl,
				esp->day1, esp->day2, esp->day3, esp->day4, esp->day5, esp->day6, esp->day7, esp->day8, esp->day9, esp->day10,esp->day11,esp->day12,esp->day13,esp->day14,esp->day15);
		}
		jptr[nbytes-1]  = ']';
		jptr[nbytes]    = '@';
		jptr[nbytes+1]  = 0;
		nbytes         += 1;
		rtable->esp_len = nbytes;
	}
}

void update_json_msr(struct XLS *XLS)
{
	struct stock    *stock;
	struct stock   **STOCKS;
	struct monster  *monster;
	struct mag      *mag;
	char            *jptr;
	char             pbuf[64];
	char             abuf1[64], abuf2[64], abuf3[64], abuf4[64];
	char             abuf5[64], abuf6[64], abuf7[64], abuf8[64];
	char             abuf9[64], abuf10[64],abuf11[64],abuf12[64];
	char             qbuf1[64], qbuf2[64], qbuf3[64], qbuf4[64];
	char             qbuf5[64], qbuf6[64], qbuf7[64], qbuf8[64];
	char             qbuf9[64], qbuf10[64],qbuf11[64],qbuf12[64];
	char             qbuf13[64],qbuf14[64],qbuf15[64],qbuf16[64];
	char             qbuf17[64],qbuf18[64],qbuf19[64],qbuf20[64];
	char             qbuf21[64],qbuf22[64],qbuf23[64],qbuf24[64];
	int              nr_entries, nr_stocks, max_size = 32 KB;

	monster            = (struct monster *)XLS->MONSTER;
	monster->msr_table = (char *)malloc(max_size);
	STOCKS             = XLS->STOCKS_PTR;
	nr_stocks          = XLS->nr_stocks;
	strcpy(monster->msr_table, "ctable MSR [");
	monster->msr_table_size = 12;
	for (int x=0; x<nr_stocks; x++) {
		stock = STOCKS[x];
		if (!stock || !stock->mag)
			continue;
		mag        = stock->mag;
		nr_entries = stock->mag->nr_entries-1;
		struct quarter *quarter_2020   = &mag->quarters[0];
		struct quarter *quarter_2019   = &mag->quarters[1];
		struct quarter *quarter_2018   = &mag->quarters[2];
		struct quarter *quarter10_2020 = &mag->quarters10[0];
		struct quarter *quarter10_2019 = &mag->quarters10[1];
		struct quarter *quarter10_2018 = &mag->quarters10[2];
		int total1  = quarter_2020->action1_total;   int total2  = quarter_2019->action1_total;
		int total3  = quarter_2018->action1_total;   int total4  = quarter_2020->action4_total;
		int total5  = quarter_2019->action4_total;   int total6  = quarter_2018->action4_total;
		int total7  = quarter10_2020->action1_total; int total8  = quarter10_2019->action1_total;
		int total9  = quarter10_2018->action1_total; int total10 = quarter10_2020->action4_total;
		int total11 = quarter10_2019->action4_total; int total12 = quarter10_2018->action4_total;
		double avgdays_anyday2020   = quarter_2020->avgdays_anyday;  double avgdays_anyday2019 = quarter_2019->avgdays_anyday;
		double avgdays_anyday2018   = quarter_2018->avgdays_anyday;  double avgdays10_anyday2020 = quarter10_2020->avgdays_anyday;
		double avgdays10_anyday2019 = quarter10_2019->avgdays_anyday;double avgdays10_anyday2018 = quarter10_2018->avgdays_anyday;

		monster->msr_table_size += snprintf(monster->msr_table+monster->msr_table_size, 8191, "{\"T\":\"%s\",\"P\":\"%.2f\",\"D\":\"%.2f\",\"PK\":\"%s\",\"1\":\"%d\",\"2\":\"%d\","
			"\"a1esp\":\"%s\",\"a4esp\":\"%s\",\"YTD\":\"%.2f\",\"y1\":\"%.2f\",\"y2\":\"%.2f\",\"y3\":\"%.2f\",\"y4\":\"%.2f\","
			"\"a1q1_2020_any5\":\"%s\",\"a1q2_2020_any5\":\"%s\",\"a1q1_2019_any5\":\"%s\",\"a1q2_2019_any5\":\"%s\",\"a1q1_2018_any5\":\"%s\",\"a1q2_2018_any5\":\"%s\","
			"\"anyavg2020\":\"%.2f\",\"anyavg2019\":\"%.2f\",\"anyavg2018\":\"%.2f\","
			"\"a1q1_2020\":\"%s\",\"a1q2_2020\":\"%s\",\"a1q1_2019\":\"%s\",\"a1q2_2019\":\"%s\",\"a1q1_2018\":\"%s\",\"a1q2_2018\":\"%s\","
			"\"avg5a1_2020\":\"%.2f\",\"avg5a1_2019\":\"%.2f\",\"avg5a1_2018\":\"%.2f\",\"T1\":\"%d\",\"T2\":\"%d\",\"T3\":\"%d\","
			"\"a4q1_2020\":\"%s\",\"a4q2_2020\":\"%s\",\"a4q1_2019\":\"%s\",\"a4q2_2019\":\"%s\",\"a4q1_2018\":\"%s\",\"a4q2_2018\":\"%s\","
			"\"avg5a4_2020\":\"%.2f\",\"avg5a4_2019\":\"%.2f\",\"avg5a4_2018\":\"%.2f\",\"T4\":\"%d\",\"T5\":\"%d\",\"T6\":\"%d\","
			"\"a1q1_2020_any10\":\"%s\",\"a1q2_2020_any10\":\"%s\",\"a1q1_2019_any10\":\"%s\",\"a1q2_2019_any10\":\"%s\",\"a1q1_2018_any10\":\"%s\",\"a1q2_2018_any10\":\"%s\","
			"\"any10avg2020\":\"%.2f\",\"any10avg2019\":\"%.2f\",\"any10avg2018\":\"%.2f\","
			"\"a1q1_2020_10\":\"%s\",\"a1q2_2020_10\":\"%s\",\"a1q1_2019_10\":\"%s\",\"a1q2_2019_10\":\"%s\",\"a1q1_2018_10\":\"%s\",\"a1q2_2018_10\":\"%s\","
			"\"avg10a1_2020\":\"%.2f\",\"avg10a1_2019\":\"%.2f\",\"avg10a1_2018\":\"%.2f\",\"T7\":\"%d\",\"T8\":\"%d\",\"T9\":\"%d\","
			"\"a4q1_2020_10\":\"%s\",\"a4q2_2020_10\":\"%s\",\"a4q1_2019_10\":\"%s\",\"a4q2_2019_10\":\"%s\",\"a4q1_2018_10\":\"%s\",\"a4q2_2018_10\":\"%s\","
			"\"avg10a4_2020\":\"%.2f\",\"avg10a4_2019\":\"%.2f\",\"avg10a4_2018\":\"%.2f\",\"T10\":\"%d\",\"T11\":\"%d\",\"T12\":\"%d\"},",
			stock->sym, stock->current_price, stock->pr_percent, stock_peak(stock->peak_1year_pc, pbuf), stock->mag->one[stock->mag->nr_entries-1], stock->mag->two[stock->mag->nr_entries-1],
			esp_str(stock->a1esp), esp_str(stock->a4esp), mag->YTD, mag->year1, mag->year2, mag->year3, mag->year4,
			/* 5 % */
			qval(quarter_2020->anyday_1r,   abuf1),  qval(quarter_2020->anyday_2r,   abuf2),  qval(quarter_2019->anyday_1r,   abuf3), qval(quarter_2019->anyday_2r,   abuf4),  qval(quarter_2018->anyday_1r, abuf5),   qval(quarter_2018->anyday_2r,   abuf6),
			avgdays_anyday2020,  avgdays_anyday2019, avgdays_anyday2018,
			qval(quarter_2020->action1_q1r, qbuf1),  qval(quarter_2020->action1_q2r, qbuf2),  qval(quarter_2019->action1_q1r, qbuf3), qval(quarter_2019->action1_q2r, qbuf4),  qval(quarter_2018->action1_q1r, qbuf5), qval(quarter_2018->action1_q2r, qbuf6),
			quarter_2020->avgdays_a1, quarter_2019->avgdays_a1, quarter_2018->avgdays_a1, total1, total2, total3,
			qval(quarter_2020->action4_q1r, qbuf7),  qval(quarter_2020->action4_q2r, qbuf8),  qval(quarter_2019->action4_q1r, qbuf9), qval(quarter_2019->action4_q2r, qbuf10), qval(quarter_2018->action4_q1r, qbuf11), qval(quarter_2018->action4_q2r, qbuf12),
			quarter_2020->avgdays_a4, quarter_2019->avgdays_a4, quarter_2018->avgdays_a4, total4, total5, total6,
			/* 10% */
			qval(quarter10_2020->anyday_1r,   abuf7),  qval(quarter10_2020->anyday_2r,   abuf8),  qval(quarter10_2019->anyday_1r,   abuf9),  qval(quarter10_2019->anyday_2r,  abuf10),  qval(quarter10_2018->anyday_1r,   abuf11),  qval(quarter10_2018->anyday_1r,   abuf12),
			avgdays10_anyday2020,  avgdays10_anyday2019, avgdays10_anyday2018,
			qval(quarter10_2020->action1_q1r, qbuf13), qval(quarter10_2020->action1_q2r, qbuf14), qval(quarter10_2019->action1_q1r, qbuf15), qval(quarter10_2019->action1_q2r, qbuf16),  qval(quarter10_2018->action1_q1r, qbuf17),  qval(quarter10_2018->action1_q2r, qbuf18),
			quarter10_2020->avgdays_a1, quarter10_2019->avgdays_a1, quarter10_2018->avgdays_a1, total7, total8, total9,
			qval(quarter10_2020->action4_q1r, qbuf19),  qval(quarter10_2020->action4_q2r, qbuf20), qval(quarter10_2019->action4_q1r, qbuf21), qval(quarter10_2019->action4_q2r, qbuf22),qval(quarter10_2018->action4_q1r, qbuf23), qval(quarter10_2018->action4_q2r, qbuf24),
			quarter10_2020->avgdays_a4, quarter10_2019->avgdays_a4, quarter10_2018->avgdays_a4, total10, total11, total12);
		if (monster->msr_table_size + 2048 >= max_size) {
			max_size += 2048;
			monster->msr_table = (char *)realloc(monster->msr_table, max_size);
		}
	}
	monster->msr_table[monster->msr_table_size-1] = ']';
	monster->msr_table[monster->msr_table_size]   = '@';
	monster->msr_table[monster->msr_table_size+1] = 0;
	monster->msr_table_size                      += 1;
}

void update_json_msr2(struct XLS *XLS)
{
	struct monster *monster;
	struct stock   *stock;
	struct stock  **STOCKS;
	struct mag     *mag;
	struct qslide  *qslide;
	char pbuf[64];char q1[64],q2[64],q3[64],q4[64];char s1[64],s2[64],s3[64];
	double a1q1, a4q1, avgdays_a1q1, avgdays_a4q1, anyday_avg, anyday_pc, anyday_ret, a1ret, a4ret;
	double anyday_std, a1std, a4std;
	int x, nr_entries, a1total, a4total, nr_stocks, max_size = 32 KB;

	monster             = (struct monster *)XLS->MONSTER;
	nr_stocks           = XLS->nr_stocks;
	STOCKS              = XLS->STOCKS_PTR;
	monster->msr2_table = (char *)malloc(max_size);
	strcpy(monster->msr2_table, "ctable MSR2 [");
	monster->msr2_table_size = 13;
	for (x=0; x<nr_stocks; x++) {
		stock = STOCKS[x];
		if (!stock || !stock->mag || !stock->mag->qslide)
			continue;
		mag          = stock->mag;
		qslide       = &mag->qslide[stock->nr_qslide-1];
		anyday_pc    = qslide->anyday_pc;
		anyday_ret   = qslide->anyday_ret;
		anyday_avg   = qslide->anyday_avg;
		anyday_std   = qslide->anyday_std;
		a1q1         = qslide->a1q1;
		a4q1         = qslide->a4q1;
		a1total      = qslide->a1total;
		a4total      = qslide->a4total;
		a1ret        = qslide->a1ret;
		a4ret        = qslide->a4ret;
		avgdays_a1q1 = qslide->avgdays_a1q1;
		avgdays_a4q1 = qslide->avgdays_a4q1;
		a1std        = qslide->a1std;
		a4std        = qslide->a4std;

		if (!a1q1)
			continue;
		monster->msr2_table_size += snprintf(monster->msr2_table+monster->msr2_table_size, 8191,
			"{\"T\":\"%s\",\"2\":\"%d\",\"any\":\"%s\",\"anyret\":\"%.2f\",\"anystd\":\"%s\",\"anyavg\":\"%.2f\","
			"\"a1q1\":\"%s\",\"a1ret\":\"%.2f\",\"a1std\":\"%s\",\"avgA1q1\":\"%.2f\",\"a1T\":\"%d\","
			"\"a4q1\":\"%s\",\"a4ret\":\"%.2f\",\"a4std\":\"%s\",\"avgA4q1\":\"%.2f\",\"a4T\":\"%d\"},",
			stock->sym, stock->mag->two[stock->mag->nr_entries-1], qval(anyday_pc, q1), anyday_ret, standard_deviation(anyday_std, s1), anyday_avg,
			qval(a1q1, q2), a1ret, standard_deviation(a1std, s2), avgdays_a1q1, a1total, qval(a4q1, q3), a4ret, standard_deviation(a4std, s3), avgdays_a4q1, a4total);

		if (monster->msr2_table_size + 2048 >= max_size) {
			max_size += 2048;
			monster->msr2_table = (char *)realloc(monster->msr2_table, max_size);
			if (!monster->msr2_table)
				return;
		}
	}
	monster->msr2_table[monster->msr2_table_size-1] = ']';
	monster->msr2_table[monster->msr2_table_size]   = '@';
	monster->msr2_table[monster->msr2_table_size+1] = 0;
	monster->msr2_table_size        += 1;
}

char *ranksym(struct stock *stock, char *outbuf)
{
	int current_rank = stock->rank;
	int prev_rank    = stock->prev_rank;

	if (current_rank < prev_rank)
		sprintf(outbuf, "%d<i>▲</i>", current_rank);
	else if (current_rank > prev_rank)
		sprintf(outbuf, "%d<h3>▼</h3>", current_rank);
	else
		sprintf(outbuf, "%d", current_rank);
	return outbuf;
}

void update_quicklook(struct XLS *XLS)
{
	struct stock   *stock, **STOCKS;
	struct monster *monster;
	struct mag2    *m2;
	char           *dir1, *dir2;
	char            rbuf[64];
	double          delta;
	int             nr_stocks, nr_entries, max_ranks, max_size = 32 KB;

	monster = (struct monster *)XLS->MONSTER;
	monster->quicklook = (char *)malloc(32 KB);
	strcpy(monster->quicklook, "ctable QL [");
	monster->quicklook_size = 11;

	nr_stocks = XLS->nr_stocks;
	STOCKS    = XLS->STOCKS_PTR;
	max_ranks = XLS->max_ranks;
	for (int x=0; x<nr_stocks; x++) {
		stock = STOCKS[x];
		if (!stock->mag || !stock->rank || stock->rank > max_ranks)
			continue;
		nr_entries = stock->mag->nr_entries;
		if (!stock->mag2 || stock->nr_mag2_entries != nr_entries)
			continue;
		m2 = &stock->mag2[nr_entries-1];
		/*	RANK:902,T:900,DATE:915,ED:313,PRICE:901,PEAK:913,
			1d:218,3d:219,5d:220,8d:221,13d:222,21d:223
			BUY:229,BuyDelta:230,Sell:231,SellDelta:232
			BuyFIB:235,BuyDeltaFIB:236,SellFIB:237,SellDeltaFIB:238
		 */
		delta   = ((stock->prior_close/stock->mag->close[nr_entries-2])-1)*100.0;
		if (m2->dir == 1.0)
			dir1 = "up";
		else if (m2->dir == 0.0)
			dir1 = "down";
		else
			dir1 = "";

		if (m2->fib_dir == 1.0)
			dir2 = "up";
		else if (m2->fib_dir == 0.0)
			dir2 = "down";
		else
			dir2 = "";

		monster->quicklook_size += sprintf(monster->quicklook+monster->quicklook_size,
								"{\"902\":\"%s\",\"900\":\"%s\",\"313\":\"%d\",\"901\":\"%.2f\",\"913\":\"%.2f\","
								 "\"218\":\"%.2f\",\"219\":\"%.2f\",\"220\":\"%.2f\",\"221\":\"%.2f\",\"222\":\"%.2f\",\"223\":\"%.2f\","
								 "\"245\":\"%.2f\",\"227\":\"%.2f\",\"228\":\"%s\","
							 	 "\"229\":\"%.2f\",\"230\":\"%.2f\",\"231\":\"%.2f\",\"232\":\"%.2f\","
								 "\"233\":\"%.2f\",\"234\":\"%s\","
							 	 "\"235\":\"%.2f\",\"236\":\"%.2f\",\"237\":\"%.2f\",\"238\":\"%.2f\"},",
						 ranksym(stock, rbuf), stock->sym, stock->earnings.earning_days, stock->prior_close, stock->mag->peak_pc[nr_entries-1], 
						 m2->day1,    m2->day3,         m2->day5,    m2->day8, m2->day13, m2->day21,
						 m2->YTD,     m2->streak,       dir1,
						 m2->buy,     m2->buy_delta,    m2->sell,    m2->sell_delta, m2->fib, dir2,
						 m2->buy_fib, m2->buy_delta_fib,m2->sell_fib,m2->sell_delta_fib);
		if (monster->quicklook_size + 2048 >= max_size) {
			max_size += 2048;
			monster->quicklook = (char *)realloc(monster->quicklook, max_size);
		}
	}
	if (monster->quicklook_size == 11) {
		monster->quicklook_size = 0;
		return;
	}
	monster->quicklook[monster->quicklook_size-1]  = ']';
	monster->quicklook[monster->quicklook_size]    = '@';
	monster->quicklook[monster->quicklook_size+1]  = 0;
	monster->quicklook_size                       += 1;
}

void update_peakwatch(struct XLS *XLS)
{
	struct stock   *stock, **STOCKS;
	struct monster *monster;
	struct mag     *mag;
	struct mag2    *m2;
	char           *sig;
	char            rbuf[64];
	double          delta, price;
	int             nr_entries, nr_days, nr_stocks;
	int             found_peak = 0, max_size = 32 KB, max_ranks, day_added; 

	monster                 = (struct monster *)XLS->MONSTER;
	nr_stocks               = XLS->nr_stocks;
	STOCKS                  = XLS->STOCKS_PTR;
	max_ranks               = XLS->max_ranks;
	monster->peak_stocks    = (struct stock **)malloc(sizeof(struct stock *) * max_ranks);
	monster->nr_peak_stocks = 0;
	monster->peakwatch      = (char *)malloc(32 KB);
	monster->peakwatch_size = 14;
	strcpy(monster->peakwatch, "peakwatch PW [");
	for (int x=0; x<nr_stocks; x++) {
		stock      = STOCKS[x];
		mag        = stock->mag;
		if (!mag || !stock->rank || stock->rank > max_ranks || monster->nr_peak_stocks >= max_ranks || stock->nr_mag2_entries != mag->nr_entries)
			continue;
		nr_entries = mag->nr_entries;
		if (!stock->mag2)
			continue;
		m2 = &stock->mag2[nr_entries-1];
		/*	RANK:902,T:900,DATE:915,ED:313,PRICE:901,PEAK_PC:913,
			1d:218,3d:219,5d:220,8d:221,13d:222,21d:223
			BUY:229,BuyDelta:230,Sell:231,SellDelta:232
			BuyFIB:235,BuyDeltaFIB:236,SellFIB:237,SellDeltaFIB:238
		 */
		nr_days = stock->mag->nr_entries-1-stock->mag->months[NR_RANKS];
		found_peak = 0;
		for (int y=0; y<nr_days; y++) {
			m2 = &stock->mag2[nr_entries-1-y];
			if (m2->sig) {
				found_peak = 0;
				break;
			}
			if (m2->peak) {
				for (int z=0; z<(nr_days-y); z++) {
					day_added = nr_entries-2-z-y;
					if (date_to_rank(stock, mag->date[day_added]) > 200)
						continue;
					m2 = &stock->mag2[day_added];
					if (!m2->peak) {
//						printf("%s found peak at: %s\n", stock->sym, mag->date[day_added]);
						found_peak = day_added;
						break;
					}
				}
			}
			if (found_peak)
				break;
		}
		if (!found_peak)
			continue;
		m2 = &stock->mag2[nr_entries-1];
		if (market == DAY_MARKET) {
			price = stock->current_price;
			delta = stock->pr_percent;
			if (delta <= (m2->pdelta*100 || delta <= m2->buy_delta_fib))
				sig = "1";
			else
				sig = "0";
		} else {
			price = stock->prior_close;
			delta = ((stock->prior_close/mag->close[nr_entries-2])-1)*100.0;
			if (m2->sig)
				sig = "1";
			else
				sig = "";
		}
		monster->peak_stocks[monster->nr_peak_stocks++] = stock;
		monster->peakwatch_size += sprintf(monster->peakwatch+monster->peakwatch_size,
								"{\"902\":\"%s\",\"900\":\"%s\",\"915\":\"%s\",\"313\":\"%d\",\"901\":\"%.2f\",\"903\":\"%.2f\",\"913\":\"%.2f\","
								 "\"218\":\"%.2f\",\"219\":\"%.2f\",\"220\":\"%.2f\",\"221\":\"%.2f\",\"222\":\"%.2f\",\"223\":\"%.2f\","
							 	 "\"229\":\"%.2f\",\"230\":\"%.2f\",\"231\":\"%.2f\",\"232\":\"%.2f\","
							 	 "\"235\":\"%.2f\",\"236\":\"%.2f\",\"237\":\"%.2f\",\"238\":\"%.2f\",\"247\":\"%.2f\",\"253\":\"%s\"},",
						 ranksym(stock, rbuf), stock->sym, mag->date[day_added], stock->earnings.earning_days, price, delta, mag->peak_pc[nr_entries-1],
						 m2->day1,    m2->day3,         m2->day5,    m2->day8, m2->day13, m2->day21,
						 m2->buy,     m2->buy_delta,    m2->sell,    m2->sell_delta,
						 m2->buy_fib, m2->buy_delta_fib,m2->sell_fib,m2->sell_delta_fib, m2->pdelta*100, sig);
		if (monster->peakwatch_size + 2048 >= max_size) {
			max_size += 2048;
			monster->peakwatch = (char *)realloc(monster->peakwatch, max_size);
		}
	}
	if (monster->peakwatch_size == 11) {
		monster->peakwatch_size = 0;
		return;
	}
	monster->peakwatch[monster->peakwatch_size-1] = ']';
	monster->peakwatch[monster->peakwatch_size]   = '@';
	monster->peakwatch[monster->peakwatch_size+1] = 0;
	monster->peakwatch_size                      += 1;

	/* sort peak_stocks by rank */
	sort_peak_stocks(XLS);
}

void rpc_stock_scatter(struct rpc *rpc)
{
	char           *packet = rpc->packet;
	char           *QGID   = rpc->argv[1];
	char           *ticker = rpc->argv[2];
	struct stock   *stock;
	struct price   *price;
	struct mag2    *m2;
	struct mag     *mag;
	int             packet_size, nr_entries, year, x;

	stock = search_stocks(ticker);
	if (!stock || !stock->mag || !stock->mag2)
		return;
	mag = stock->mag;

	nr_entries = mag->nr_entries;
	if (nr_entries < 10)
		return;

	year = mag->year_2021;
	if (year == -1) {
		year = mag->year_2022;
		if (year == -1)
			return;
	}
	strcpy(packet, "sp-signal-scatter [");
	packet_size = 19;
	if (nr_entries != stock->nr_mag2_entries) {
		printf(BOLDRED "sigmon: mismatch in %s mag(%d)/mag2(%d)" RESET "\n", stock->sym, nr_entries, stock->nr_mag2_entries);
		return;
	}
	for (x=year; x<nr_entries; x++) {
		m2 = &stock->mag2[x];
		if (m2->sig)
			packet_size += sprintf(packet+packet_size, "{\"x\":%lu,\"y\":%.2f},", str2unix(mag->date[x])*1000, mag->close[x]);
	}
	packet[packet_size-1] = ']';
	packet[packet_size++] = ' ';
	price = stock->price;
	memcpy(packet+packet_size, price->price_1d_close, price->price_1d_close_len);
	packet_size += price->price_1d_close_len;
	packet[packet_size++] = ' ';
	packet_size += snprintf(packet+packet_size, 64, "%s-%s", QGID, stock->sym);
	packet[packet_size]= 0;
	printf("%s\n", packet);
	websocket_send(rpc->connection, packet, packet_size);
}

/* STOCK PAGE MIDDLE QUAD SIGNAL CHART */
void build_stockpage_sigmon(struct XLS *XLS)
{
	struct stock   *stock, **STOCKS;
	struct monster *monster;
	struct mag2    *m2;
	struct mag     *mag;
	char            D1[16],D3[16],D5[16],D8[16],D13[16],D21[16];
	char           *d1,*d3,*d5,*d8,*d13,*d21;
	char            rbuf[64];
	int             nr_entries, year, x, y, nr_stocks, max_size, max_ranks;
	double          day1, day3, day5, day8, day13, day21, price, signal_close;

	monster  = (struct monster *)XLS->MONSTER;
	max_size = 32 KB;
	monster->stockpage_sigmon = (char *)malloc(32 KB);
	strcpy(monster->stockpage_sigmon, "sigmon sigmon [");
	monster->stockpage_sigmon_size = 15;

	STOCKS    = XLS->STOCKS_PTR;
	max_ranks = XLS->max_ranks;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = STOCKS[x];
		if (!(mag=stock->mag))
			continue;

		nr_entries = mag->nr_entries;
		if (!stock->mag2 || nr_entries < 10)
			continue;

		year = mag->year_2021;
		if (year == -1)
			continue;
		if (nr_entries != stock->nr_mag2_entries) {
			printf(BOLDRED "sigmon: mismatch in %s mag(%d)/mag2(%d)" RESET "\n", stock->sym, nr_entries, stock->nr_mag2_entries);
			continue;
		}
		for (y=year; y<nr_entries; y++) {
			m2 = &stock->mag2[y];
			if (!m2->sig)
				continue;
			if (date_to_rank(stock, mag->date[y]) > 200)
				continue;
			signal_close = mag->close[y];
			if (y + 1 < nr_entries) {
				sprintf(D1, "%.2f", delta(mag->close[y+1], signal_close));
				d1 = D1;
			} else
				d1 = "-";
			if (y + 3 < nr_entries) {
				sprintf(D3, "%.2f", delta(mag->close[y+3], signal_close));
				d3 = D3;
			} else
				d3 = "-";
			if (y + 5 < nr_entries) {
				sprintf(D5, "%.2f", delta(mag->close[y+5], signal_close));
				d5 = D5;
			} else
				d5 = "-";
			if (y + 8 < nr_entries) {
				sprintf(D8, "%.2f", delta(mag->close[y+8], signal_close));
				d8 = D8;
			} else
				d8 = "-";
			if (y + 13 < nr_entries) {
				sprintf(D13, "%.2f", delta(mag->close[y+13], signal_close));
				d13 = D13;
			} else
				d13 = "-";
			if (y + 21 < nr_entries) {
				sprintf(D21, "%.2f", delta(mag->close[y+21], signal_close));
				d21 = D21;
			} else
				d21 = "-";

			if (market == DAY_MARKET)
				price = stock->current_price;
			else
				price = stock->prior_close;

			monster->stockpage_sigmon_size += sprintf(monster->stockpage_sigmon+monster->stockpage_sigmon_size, "{\"902\":\"%s\",\"900\":\"%s\",\"915\":\"%s\",\"901\":\"%.2f\",\"913\":\"%.2f\","
														   			   "\"850\":\"%s\",\"851\":\"%s\",\"852\":\"%s\",\"853\":\"%s\",\"854\":\"%s\",\"855\":\"%s\"},",
						 ranksym(stock, rbuf), stock->sym, mag->date[y], price, mag->peak_pc[nr_entries-1],
						 d1,d3,d5,d8,d13,d21);
			if (monster->stockpage_sigmon_size + 2048 >= max_size) {
				max_size += 2048;
				monster->stockpage_sigmon = (char *)realloc(monster->stockpage_sigmon, max_size);
			}
		}
	}
	if (monster->stockpage_sigmon_size < 100) {
		monster->stockpage_sigmon_size = 0;
		return;
	}
	monster->stockpage_sigmon[monster->stockpage_sigmon_size-1] = ']';
	monster->stockpage_sigmon[monster->stockpage_sigmon_size++] = '@';
}

/* Stock Page entry point for unregistered */
void rpc_stockpage_signals(struct rpc *rpc)
{
	char           *packet     = rpc->packet;
	char           *ticker     = rpc->argv[1];
	struct XLS     *XLS        = CURRENT_XLS;
	struct monster *monster    = (struct monster *)XLS->MONSTER;
	int             packet_len = monster->stockpage_sigmon_size;

	if (!packet_len || !ticker)
		return;

	memcpy(packet, monster->stockpage_sigmon, monster->stockpage_sigmon_size);
	packet[packet_len-1] = ' ';
	strncpy(packet+packet_len, ticker, 7);
	websocket_send(rpc->connection, packet, packet_len);
}

/*
 * init_monster(boot):
 *    - called on boot (boot==1) (but BEFORE load_forks() is called at EOD)
 *    - called at EOD  (boot==0)
 * init_forks():
 *    - called by init_monster() (when boot==1)
 * load_forks():
 *    - called by init_forks() IF forks.db is missing
 *    - called at EOD to create a new forks.db
 * table3:
 *    - built by build_flight_info()
 * build_flight_info():
 *    - called by init_signals()
 * init_signals():
 *    - called on boot by init_monster()
 *    - called at EOD (BEFORE load_forks() at EOD + BEFORE generate_monster_db())
 * table5:
 *    - built by load_forks()
 */

void generate_monster_db(struct XLS *XLS)
{
	struct monster *new_monster = (struct monster *)malloc(sizeof(struct monster));
	struct monster *monster     = (struct monster *)XLS->MONSTER;
	struct forkmem *fork;
	int             zbytes;

	memcpy(new_monster, monster, sizeof(*monster));
	new_monster->table = (char *)malloc(12000 KB);
	new_monster->size  = 0;

	if (!monster->quicklook) {
		build_stockpage_sigmon(XLS);
		update_quicklook(XLS);
		update_peakwatch(XLS);
		build_flight_info(new_monster);
	}

	/* Monster 1.0 */
	memcpy(new_monster->table, monster->msr_table, monster->msr_table_size);
	new_monster->size += monster->msr_table_size;

	/* StockPage Algo Screener Table */
	memcpy(new_monster->table, monster->stockpage_sigmon, monster->stockpage_sigmon_size);
	new_monster->size += monster->stockpage_sigmon_size;

	/* Quicklook (Table 1) */
	memcpy(new_monster->table+new_monster->size, monster->quicklook, monster->quicklook_size);
	new_monster->size += monster->quicklook_size;

	/* PeakWatch (Table 2) */
	memcpy(new_monster->table+new_monster->size, monster->peakwatch, monster->peakwatch_size);
	new_monster->size += monster->peakwatch_size;

	/* Table 3  (FDT/All Signals)*/
	memcpy(new_monster->table+new_monster->size, new_monster->table3, new_monster->table3_size);
	new_monster->size += new_monster->table3_size;

	/*
	 * Airstocks main page
 	 */
	fork = &XLS->forks[0];
	if (fork) {
		/* Table #4.1 First Fork */
		memcpy(new_monster->table+new_monster->size, fork->fork_table, fork->fork_tsize);
		new_monster->size += fork->fork_tsize;
		*(new_monster->table+new_monster->size++) = '@';
	
		/* Chart #4.1 */
		memcpy(new_monster->table+new_monster->size, fork->fork_scatter, fork->fork_scatter_size);
		new_monster->size += fork->fork_scatter_size;
	
		/* Table #4.2 (Portfolios List) + Hypersonic Port Scatter (copied to portfolios->hypersonic prior to this) */
		memcpy(new_monster->table+new_monster->size, monster->portfolios->hypersonic, monster->portfolios->hypersonic_size);		
		new_monster->size += monster->portfolios->hypersonic_size;
	
		/* Table 5  (Saftey) */
		memcpy(new_monster->table+new_monster->size, monster->table5, monster->table5_size);
		new_monster->size += monster->table5_size;
	//	fs_newfile("table5.txt", monster->table5, monster->table5_size);
		*(new_monster->table+new_monster->size-1) = ']';
	
	//	fs_newfile("table.txt", new_monster->table, new_monster->size);
		// add old monster to garbage collection;
	//	memory_add_garbage(MONSTER);
	}

	new_monster->gztable = (char *)malloc(12000 KB);
	strcpy(new_monster->gztable, HTTP_GZIP);
	zbytes = zip_compress(new_monster->table, new_monster->gztable+sizeof(HTTP_GZIP)-1, new_monster->size);
	cstring_itoa(new_monster->gztable+GZIP_OFFSET, zbytes);
//	new_monster->gztable = (char *)realloc(new_monster->gztable, zbytes);
	new_monster->gzsize  = zbytes+sizeof(HTTP_GZIP)-1;

	// set the Current Monster
	XLS->MONSTER = new_monster;
	new_monster->quicklook = NULL;

	// save monster tables & portfolio+monster structs
	store_monster_db(new_monster);
}

void store_monster_db(struct monster *monster)
{
	struct portfolio *portfolios = monster->portfolios;
	int fd;

	if (!portfolios)
		return;

	fd = open("db/monster.db", O_RDWR|O_CREAT|O_TRUNC, 0644);
	write(fd, portfolios,             sizeof(*portfolios));
	write(fd, monster,                sizeof(*monster));
	write(fd, monster->gztable,       monster->gzsize);
	write(fd, portfolios->supersonic, portfolios->supersonic_size);
	write(fd, portfolios->business,   portfolios->business_size);
	write(fd, portfolios->economy,    portfolios->economy_size);
	close(fd);
}

// boot code only
int load_monster_db(struct XLS *XLS)
{
	struct portfolio *PORTFOLIOS;
	struct monster   *MONSTER, *m = (struct monster *)XLS->MONSTER;
	char             *file;

	if (!(file=fs_mallocfile("db/monster.db", NULL)))
		return 0;
	PORTFOLIOS             = (struct portfolio *)(file);
	MONSTER                = (struct monster   *)(file+sizeof(*PORTFOLIOS));
	MONSTER->gztable       = (char *)(file+sizeof(*PORTFOLIOS)+sizeof(*MONSTER));
	MONSTER->portfolios    = PORTFOLIOS;
	MONSTER->quicklook     = NULL;
	MONSTER->signals       = m->signals;
	MONSTER->nr_signals    = m->nr_signals;
	PORTFOLIOS->hypersonic = NULL; // included in MONSTER->gztable
	PORTFOLIOS->supersonic = (char *)(MONSTER->gztable+MONSTER->gzsize);
	PORTFOLIOS->business   = (char *)(PORTFOLIOS->supersonic+PORTFOLIOS->supersonic_size);
	PORTFOLIOS->economy    = (char *)(PORTFOLIOS->economy   +PORTFOLIOS->supersonic_size);
	XLS->MONSTER           = (void *)MONSTER;
	return 1;
}

void init_monster(struct XLS *XLS, int do_forks)
{
	XLS->MONSTER = (void *)zmalloc(sizeof(struct monster));

	init_signals(XLS);
	build_candle_monster(XLS);

//	build_color_table();
//	update_json_esp();
	update_json_msr(XLS);
	update_json_msr2(XLS);
//	update_json_sb150(XLS);

	if (!XLS->config->production)
		return;

	// update_EOD() doesn't touch fork code, that is done separately
	if (!do_forks)
		return;
	// boot code only from here
	init_forks(XLS);
	if (!load_monster_db(XLS)) {
		printf("generating monster\n");
		build_stockpage_sigmon(XLS);
		update_quicklook(XLS);
		update_peakwatch(XLS);
		generate_monster_db(XLS);
	}
}

void http_send_monster(struct connection *connection)
{
	struct XLS *XLS         = CURRENT_XLS;
	struct monster *monster = (struct monster *)XLS->MONSTER;

	if (!monster)
		return;

	SSL_write(connection->ssl, monster->gztable, monster->gzsize);
}

void rpc_send_monster(struct rpc *rpc)
{
	http_send_monster(rpc->connection);
}
