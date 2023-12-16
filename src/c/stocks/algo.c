#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

int            current_month;
int            month_days_array[24];
int           *month_days;
int months[] = { 0x2d31302d, 0x2d32302d, 0x2d33302d, 0x2d34302d, 0x2d35302d, 0x2d36302d,
                 0x2d37302d, 0x2d38302d, 0x2d39302d, 0x2d30312d, 0x2d31312d, 0x2d32312d }; // -01-02-03-04-05...-12-

static void init_stockdata(struct mag *mag, int nr_entries)
{
	memset(mag, 0, sizeof(struct mag));

	mag->date         = (char  **)         malloc(sizeof(char *)         * nr_entries);
	mag->open         = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->high         = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->low          = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->close        = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->volume       = (uint64_t *)       malloc(sizeof(uint64_t)       * nr_entries);
	mag->peak_pc      = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->peak_yr      = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->weeks        = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->up           = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->down         = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->ratio        = (double *)         malloc(sizeof(double)         * nr_entries);
	mag->one          = (char *)          zmalloc(sizeof(char)           * nr_entries);
	mag->two          = (char *)          zmalloc(sizeof(char)           * nr_entries);
	mag->mag1         = (char *)          zmalloc(sizeof(char)           * nr_entries);
	mag->neg1         = (char *)          zmalloc(sizeof(char)           * nr_entries);
	mag->mag2         = (char *)          zmalloc(sizeof(char)           * nr_entries);
	mag->neg2         = (char *)          zmalloc(sizeof(char)           * nr_entries);
	mag->days_5pc     = (unsigned short *)zmalloc(sizeof(unsigned short) * nr_entries);
	mag->days_10pc    = (unsigned short *)zmalloc(sizeof(unsigned short) * nr_entries);
	mag->days_ret5    = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->days_ret10   = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->action1      = (struct action  *)zmalloc(sizeof(struct action)  * nr_entries);
	mag->action4      = (struct action  *)zmalloc(sizeof(struct action)  * nr_entries);
	mag->a1esp        = (struct action  *)zmalloc(sizeof(struct action)  * nr_entries);
	mag->a4esp        = (struct action  *)zmalloc(sizeof(struct action)  * nr_entries);
	mag->a1total      = (unsigned short *)zmalloc(sizeof(unsigned short) * nr_entries);
	mag->a4total      = (unsigned short *)zmalloc(sizeof(unsigned short) * nr_entries);
	mag->a1q1         = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->a1q2         = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->a4q1         = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->a4q2         = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->delta_open   = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->delta_high   = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->delta_low    = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->delta_close  = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->BIX          = (double *)        zmalloc(sizeof(double)         * nr_entries);
	mag->year_2015    = -1;
	mag->year_2016    = -1;
	mag->year_2017    = -1;
	mag->year_2018    = -1;
	mag->year_2019    = -1;
	mag->year_2020    = -1;
	mag->year_2021    = -1;
	mag->year_2022    = -1;
	mag->year_2023    = -1;
}

void do_trends(struct stock *stock, struct mag *mag)
{
	int x, max, max_entries = mag->nr_entries, entry, nr_days = 0, nr_green = 0;
	double day1,day3,day5,day8,day13,day21,day42,day63, x_close, prior_close;

	max = MIN(max_entries, 32);
	prior_close = mag->close[max_entries-1];

	/* #Days UP, #Days DOWN */
	for (x=0; x<max; x++) {
		x_close = mag->close[max_entries-x-1];
		if (x_close > 0.0)
			stock->nr_days_up++;
		else if (x_close < 0.0)
			stock->nr_days_down++;
	}

	if (max_entries < 63 || (short)mag->year_2022 == -1)
		return;
	day1           = mag->close[max_entries-1-1];
	day3           = mag->close[max_entries-1-3];
	day5           = mag->close[max_entries-1-5];
	day8           = mag->close[max_entries-1-8];
	day13          = mag->close[max_entries-1-13];
	day21          = mag->close[max_entries-1-21];
	day42          = mag->close[max_entries-1-42];
	day63          = mag->close[max_entries-1-63];
	stock->day1    = ((prior_close/day1)-1)*100.0;
	stock->day3    = ((prior_close/day3)-1)*100.0;
	stock->day5    = ((prior_close/day5)-1)*100.0;
	stock->day8    = ((prior_close/day8)-1)*100.0;
	stock->day13   = ((prior_close/day13)-1)*100.0;
	stock->day21   = ((prior_close/day21)-1)*100.0;
	stock->day42   = ((prior_close/day42)-1)*100.0;
	stock->day63   = ((prior_close/day63)-1)*100.0;
	if (market == NO_MARKET) {
		stock->open_pc = ((mag->open[max_entries-1]/prior_close)-1)*100.0;
		stock->high_pc = ((mag->high[max_entries-1]/prior_close)-1)*100.0;
		stock->low_pc  = ((mag->low[max_entries-1]/prior_close)-1)*100.0;
	} else {
		stock->open_pc = ((stock->price_open/prior_close)-1)*100.0;
		stock->high_pc = ((stock->intraday_high/prior_close)-1)*100.0;
		stock->low_pc  = ((stock->intraday_low/prior_close)-1)*100.0;
	}

	/* Green Days */
	entry = mag->year_2022;
	for (x_close = mag->close[entry]; entry < mag->nr_entries-1; entry++) {
		if (x_close > 0.0)
			nr_green += 1;
		nr_days++;
	}
	if (nr_green && nr_days)
		stock->green_days = ((nr_green/nr_days)-1)*100.0;

	/* Weeks UP/DOWN */
	for (x=0; x<current_week; x++) {
		struct week *week = &WEEKS[x];
		int start_entry   = date2entry(mag, week[x].start_date_string);
		int end_entry     = date2entry(mag, week[x].end_date_string);
		double chgpc      = delta(mag->close[start_entry], mag->close[end_entry]);
		mag->weeks[x]     = chgpc;
		if (chgpc < 0.0)
			mag->nr_weeks_down++;
		else
			mag->nr_weeks_up++;
	}
}

int stock_trend(struct stock *stock, char *qdiv, char *packet)
{
	struct mag2 *m2;
	int packet_size, x;

	if (!stock->mag2 || stock->mag->nr_entries < 10)
		return 0;
	m2 = &stock->mag2[stock->mag->nr_entries-1];

	packet_size = sprintf(packet, "trend %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f@", m2->day1, m2->day3, m2->day5, m2->day8, m2->day13, m2->day21, m2->day42, m2->day63);
	return (packet_size);
}

void init_BIX(struct XLS *XLS)
{
	struct stock *GSPC, *stock, **stocks;
	int nr_entries, nr_stocks, x, y;

	if (!XLS->config->production)
		return;

	stocks    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	GSPC      = XLS->GSPC;
	for (x=0; x<nr_stocks; x++) {
		stock = stocks[x];
		if (!stock->mag || !stock->mag2)
			continue;
		nr_entries = stock->nr_mag2_entries;
		if (nr_entries != stock->mag->nr_entries) {
//			printf(BOLDRED "init_BIX mismatch: [%s] %d vs %d" RESET "\n", stock->sym, stock->mag->nr_entries, stock->nr_mag2_entries);
			continue;
		}
		for (y=0; y<nr_entries; y++) {
			struct mag2 *m2 = &stock->mag2[y];
			if (m2->YTD > GSPC->mag2[y].YTD)
				stock->mag->BIX[y] = m2->YTD - GSPC->mag2[y].YTD;
			else
				stock->mag->BIX[y] = GSPC->mag2[y].YTD - m2->YTD;
		}
	}
}

static __inline__ void qslide_anyday(struct stock *stock, struct mag *mag, struct qslide *qslide, int start_entry, int end_entry)
{
	unsigned short days, nr_days = 0;
	double anyret[256];
	double anyday_ret = 0, anyret_5pc;
	int x, nr_success = 0, nr_failed = 0, total;

	for (x=start_entry; x<end_entry; x++) {
		days = mag->days_5pc[x];
		if (days <= 21) {
			anyret_5pc         = mag->days_ret5[x];
			nr_success        += 1;
			nr_days           += days;
			anyday_ret        += anyret_5pc;
			anyret[nr_success] = anyret_5pc;
		} else {
			nr_failed  += 1;
		}
	}
	qslide->anyday_avg = (double)nr_days/(double)(end_entry-start_entry);
	total              = nr_success + nr_failed;
	qslide->anyday_pc  = 100.0+((((double)nr_success)/(double)total)-1)*100.0;
	qslide->anyday_ret = anyday_ret/nr_success;
	qslide->anyday_std = stdev(anyret, nr_success);
}

static __inline__ void qslide_actions(struct stock *stock, struct mag *mag, struct qslide *qslide, int start_entry, int end_entry)
{
	struct action *action1 = mag->action1;
	struct action *action4 = mag->action4;
	double total, nr_ret_a1 = 0, nr_ret_a4 = 0, ret_5pc;;
	double a1ret[256];
	double a4ret[256];
	int nr_success1 = 0, nr_pending1 = 0, nr_failed1 = 0;
	int nr_success4 = 0, nr_pending4 = 0, nr_failed4 = 0;
	int nr_days_a1  = 0, nr_days_a4  = 0;
	int x;

	for (x=0; x<mag->nr_action1; x++) {
		if (action1[x].entry >= start_entry && action1[x].entry <= end_entry) {
			switch (action1[x].status_21d) {
				case ACTION_SUCCESS:
					ret_5pc     = mag->days_ret5[action1[x].entry];
					nr_days_a1  += action1[x].nr_days;
					nr_ret_a1   += ret_5pc;
					a1ret[nr_success1] = ret_5pc;
					nr_success1 += 1;
					break;
				case ACTION_PENDING:
					nr_pending1 += 1;
					break;
				case ACTION_FAILED:
					nr_failed1  += 1;
					break;
			}
		}
	}
	for (x=0; x<mag->nr_action4; x++) {
		if (action4[x].entry >= start_entry && action4[x].entry <= end_entry) {
			switch (action4[x].status_21d) {
				case ACTION_SUCCESS:
					ret_5pc      = mag->days_ret5[action4[x].entry];
					nr_days_a4  += action4[x].nr_days;
					nr_ret_a4   += ret_5pc;
					a4ret[nr_success4] = ret_5pc;
					nr_success4 += 1;
					break;
				case ACTION_PENDING:
					nr_pending4 += 1;
					break;
				case ACTION_FAILED:
					nr_failed4  += 1;
					break;
			}
		}
	}
	qslide->a1total      = (short)(nr_success1+nr_pending1+nr_failed1);
	qslide->a4total      = (short)(nr_success4+nr_pending4+nr_failed4);
	qslide->avgdays_a1q1 = (double)nr_days_a1/(double)nr_success1;
	qslide->avgdays_a4q1 = (double)nr_days_a4/(double)nr_success4;
	qslide->a1ret        = (double)nr_ret_a1 /(double)nr_success1;
	qslide->a4ret        = (double)nr_ret_a4 /(double)nr_success4;
	qslide->a1std        = (double)stdev(a1ret, nr_success1);
	qslide->a4std        = (double)stdev(a4ret, nr_success4);

	if (nr_success1) {
		total = (double)((double)nr_success1+(double)nr_pending1+(double)nr_failed1);
		qslide->a1q1 = 100.0+((((double)nr_success1)/total)-1)*100.0;
	} else
		qslide->a1q1 = 0.0;

	if (nr_success4) {
		total = (double)((double)nr_success4+(double)nr_pending4+(double)nr_failed4);
		qslide->a4q1 = 100.0+((((double)nr_success4)/total)-1)*100.0;
	} else
		qslide->a4q1 = 0.0;

}


static __inline__ void algorithm_actions_esp(struct mag *mag, int esp, int signal_day, int nr_days, int inprogress, struct quarter *quarter)
{
	switch (esp) {
		case 1: /* ESP a1 */
			quarter->avgdays_total_a1esp += (nr_days+1);
			if (nr_days+1 < 63) {
				if (!inprogress) {
					mag->nr_a1esp_success++;
					quarter->a1esp_q1++;
				}
			} else if (nr_days+1 < 126) {
				quarter->a1esp_q2++;
			} else {
				mag->a1esp[signal_day].status = ACTION_FAILED;
			}
			break;
		default: /* ESP a4 */
			quarter->avgdays_total_a4esp += (nr_days+1);
			if (nr_days+1 < 63) {
				if (!inprogress) {
					mag->nr_a4esp_success++;
					quarter->a4esp_q1++;
				}
			} else if (nr_days+1 < 126) {
				mag->a4esp[signal_day].status = ACTION_SUCCESS;
				quarter->a4esp_q2++;
			} else {
				mag->a4esp[signal_day].status = ACTION_FAILED;
			}
	}
}

static __inline__ void algorithm_actions_a1(struct mag *mag, int y, int z, int inprogress, int ten_pc, struct quarter *quarter)
{
	int nr_days = z+1;

	quarter->avgdays_total_a1 += nr_days; // called once after 5% reached, whether 1Q or 2Q
	if (!ten_pc)
		mag->action1[mag->nr_action1-1].nr_days = nr_days;
	if (nr_days < 21)
		if (!inprogress)
			mag->action1[mag->nr_action1-1].status_21d = ACTION_SUCCESS;

	if (nr_days < 63) {
		if (!inprogress) {
			mag->action1[mag->nr_action1-1].status = ACTION_SUCCESS;
			mag->nr_action1_success++;
			quarter->action1_q1++;
		}
	} else if (nr_days < 126) {
		mag->action1[mag->nr_action1-1].status = ACTION_SUCCESS;
		quarter->action1_q2++;
	} else {
		mag->action1[mag->nr_action1-1].status = ACTION_FAILED;
	}
}

static __inline__ void algorithm_actions_a4(struct mag *mag, int y, int z, int inprogress, int ten_pc, struct quarter *quarter)
{
	int nr_days = (z+1);

	quarter->avgdays_total_a4              += nr_days;
	if (!ten_pc)
		mag->action4[mag->nr_action4-1].nr_days = nr_days;
	if (nr_days <= 21)
		if (!inprogress)
			mag->action4[mag->nr_action4-1].status_21d = ACTION_SUCCESS;

	if (nr_days < 63) {
		if (!inprogress) {
			mag->action4[mag->nr_action4-1].status = ACTION_SUCCESS;
			mag->nr_action4_success++;
			quarter->action4_q1++;
		}
	} else if (nr_days < 126) {
		mag->action4[mag->nr_action4-1].status = ACTION_SUCCESS;
		quarter->action4_q2++;
	} else {
		mag->action4[mag->nr_action4-1].status = ACTION_FAILED;
	}
}


double action_q1_result(double action_q1, double action_total)
{
    if (action_q1 != 0) {
		if (action_q1 == action_total)
			return 100.0;
		else
			return round(((action_q1/action_total))*100);
    }
	return 0;
}

double action_q2_result(double action_q1, double action_q2, double action_q1r, double action_total, double actions_failed)
{
	if (action_q2 == 0 && action_total == 0)
		return 0;
	if (action_q2 == action_q1 && action_q2 == action_total)
		return 100;
	if (action_q2 == action_total)
		return 100;
	return round(((action_q1+action_q2)/action_total)*100);
}

static __inline__ void action_failed(struct mag *mag, struct quarter *quarter, struct quarter *quarter10, int action1, int action4, int a1esp, int a4esp, int y, int z)
{
	if (action1) {
		mag->action1[mag->nr_action1-1].status = ACTION_FAILED;
		quarter->action1_failed++;
		quarter10->action1_failed++;
		quarter->action1_inprogress--;
		quarter10->action1_inprogress--;
		algorithm_actions_a1(mag, y, z, 0, 1, quarter);
	} else if (a1esp) {
		mag->a1esp[y].status = ACTION_FAILED;
		quarter->a1esp_failed++;
		quarter10->a1esp_failed++;
		quarter->a1esp_inprogress--;
		quarter10->a1esp_inprogress--;
		algorithm_actions_esp(mag, 1, y, z, 0, quarter);
	}
	if (action4) {
		mag->action4[mag->nr_action4-1].status = ACTION_FAILED;
		quarter->action4_failed++;
		quarter10->action4_failed++;
		quarter->action4_inprogress--;
		quarter10->action4_inprogress--;
		algorithm_actions_a4(mag, y, z, 0, 1, quarter);
	} else if (a4esp) {
		mag->a4esp[y].status = ACTION_FAILED;
		quarter->a4esp_failed++;
		quarter10->a4esp_failed++;
		quarter->a4esp_inprogress--;
		quarter10->a4esp_inprogress--;
		algorithm_actions_esp(mag, 4, y, z, 0, quarter);
	}
}

static __inline__ void action_10pc(struct mag *mag, double pc_diff, struct quarter *quarter10, int action1, int action4, int a1esp, int a4esp, int failed_signal, int y, int z)
{
	if (action1) {
		mag->action1[mag->nr_action1-1].ten_pt      = pc_diff;
		mag->action1[mag->nr_action1-1].ten_days    = z+1;
		quarter10->action1_inprogress--;
		if (!failed_signal)
			algorithm_actions_a1(mag, y, z, 0, 1, quarter10);
	} else if (a1esp) {
		mag->a1esp[y].ten_pt         = pc_diff;
		mag->a1esp[y].ten_days       = z+1;
		quarter10->a1esp_inprogress--;
		if (!failed_signal)
			algorithm_actions_esp(mag, 1, y, z, 0, quarter10);
	}
	if (action4) {
		mag->action4[mag->nr_action4-1].ten_pt      = pc_diff;
		mag->action4[mag->nr_action4-1].ten_days    = z+1;
		quarter10->action4_inprogress--;
		if (!failed_signal)
			algorithm_actions_a4(mag, y, z, 0, 1, quarter10);
	} else if (a4esp) {
		mag->a4esp[y].ten_pt         = pc_diff;
		mag->a4esp[y].ten_days       = z+1;
		quarter10->a4esp_inprogress--;
		if (!failed_signal)
			algorithm_actions_esp(mag, 4, y, z, 0, quarter10);
	}
}

static __inline__ void action_5pc(struct mag *mag, double pc_diff, struct quarter *quarter, int action1, int action4, int a1esp, int a4esp, int failed_signal, int y, int z)
{
	if (action1) {
		mag->action1[mag->nr_action1-1].five_pt     = pc_diff;
		mag->action1[mag->nr_action1-1].five_days   = z+1;
		mag->action1[mag->nr_action1-1].status      = ACTION_SUCCESS;
		mag->action1[mag->nr_action1-1].a1_quarter  = action_q1_result(quarter->action1_q1, quarter->action1_total);
		mag->action1[mag->nr_action1-1].a1_quarter2 = action_q2_result(quarter->action1_q1, quarter->action1_q2, quarter->action1_q1r, quarter->action1_total, quarter->action1_failed);
		quarter->action1_inprogress--;
		algorithm_actions_a1(mag, y, z, 0, 0, quarter);
	}
	if (action4) {
		mag->action4[mag->nr_action4-1].five_pt     = pc_diff;
		mag->action4[mag->nr_action4-1].five_days   = z+1;
		mag->action4[mag->nr_action4-1].status      = ACTION_SUCCESS;
		mag->action4[mag->nr_action4-1].a4_quarter  = action_q1_result(quarter->action4_q1, quarter->action4_total);
		mag->action4[mag->nr_action4-1].a4_quarter2 = action_q2_result(quarter->action1_q1, quarter->action1_q2, quarter->action1_q1r, quarter->action1_total, quarter->action1_failed);
		quarter->action4_inprogress--;
		algorithm_actions_a4(mag, y, z, 0, 0, quarter);
	}
	if (a1esp && !action1) {
		mag->a1esp[y].five_pt     = pc_diff;
		mag->a1esp[y].five_days   = z+1;
		mag->a1esp[y].status      = ACTION_SUCCESS;
		quarter->a1esp_inprogress--;
		algorithm_actions_esp(mag, 1, y, z, 0, quarter);
	}
	if (a4esp && !action4) {
		mag->a4esp[y].five_pt = pc_diff;
		mag->a4esp[y].five_days = z+1;
		mag->a4esp[y].status = ACTION_SUCCESS;
		quarter->a4esp_inprogress--;
		algorithm_actions_esp(mag, 4, y, z, 0, quarter);
	}
}

void algorithm_actions(struct mag *mag, int nr_entries)
{
	struct quarter *quarter, *quarter10;
	int five_pt, ten_pt, failed_signal, action1, action4, a1esp, a4esp, esp, y, z;

	/* [2] #Days Return 5% & 10% */
	for (y=0; y<nr_entries; y++) {
		double current_close = mag->close[y];
		five_pt        = 0;
		ten_pt         = 0;
		failed_signal  = 0;
		quarter        = get_quarter(mag->date[y], mag);
		quarter10      = get_quarter10(mag->date[y], mag);
		action1        = 0;
		action4        = 0;

        if (quarter && is_action1(mag, y)) {
			action1                        = 1;
			quarter->action1_total        += 1;
			quarter10->action1_total      += 1;
			quarter->action1_inprogress   += 1;
			quarter10->action1_inprogress += 1;
			mag->action1[mag->nr_action1].status     = ACTION_PENDING;
			mag->action1[mag->nr_action1].status_21d = ACTION_PENDING;
			mag->action1[mag->nr_action1].entry      = y;
			mag->nr_action1++;
        }
		if (quarter && (esp=is_a1esp(mag, y))) {
			if (!action1) {
				mag->a1esp[y].status         = ACTION_PENDING;
				quarter->a1esp_total        += 1;
				quarter10->a1esp_total      += 1;
				quarter->a1esp_inprogress   += 1;
				quarter10->a1esp_inprogress += 1;
				mag->nr_a1esp++;
				mag->a1esp[y].type = TRUE_ESP;
			}
			mag->a1esp[y].a1esp = esp;
			mag->a1esp[y].entry = y;
			a1esp               = 1;
		}
        if (quarter && is_action4(mag, y)) {
			action4                        = 1;
			quarter->action4_total        += 1;
			quarter10->action4_total      += 1;
			quarter->action4_inprogress   += 1;
			quarter10->action4_inprogress += 1;
			mag->action4[mag->nr_action4].status     = ACTION_PENDING;
			mag->action4[mag->nr_action4].status_21d = ACTION_PENDING;
			mag->action4[mag->nr_action4].entry      = y;
            mag->nr_action4++;
        }
		if (quarter && (esp=is_a4esp(mag, y))) {
			if (!action4) {
				mag->a4esp[y].status         = ACTION_PENDING;
				quarter->a4esp_total        += 1;
				quarter10->a4esp_total      += 1;
				quarter->a4esp_inprogress   += 1;
				quarter10->a4esp_inprogress += 1;
				mag->a4esp[y].type           = TRUE_ESP;
				mag->nr_a4esp++;
			}
			mag->a4esp[y].a4esp = esp;
			mag->a4esp[y].entry = y;
			a4esp               = 1;
		}
		if (quarter) {
			mag->a1total[y] = (unsigned short)quarter->action1_total;
			mag->a4total[y] = (unsigned short)quarter->action4_total;
			mag->a1q1[y]    = action_q1_result(quarter->action1_q1, quarter->action1_total);
			mag->a1q2[y]    = action_q2_result(quarter->action1_q1, quarter->action1_q2, quarter->action1_q1r, quarter->action1_total, quarter->action1_failed);
			mag->a4q1[y]    = action_q1_result(quarter->action4_q1, quarter->action4_total);
			mag->a4q2[y]    = action_q2_result(quarter->action4_q1, quarter->action4_q2, quarter->action4_q1r, quarter->action4_total, quarter->action4_failed);
		}

		/* Forward Scan for 5% & 10% returns */
		for (z=0; z<252; z++) {
			/* Action Failed */
			if (z+1 == 126 && !five_pt) {
				failed_signal = 1;
				if (quarter)
					action_failed(mag, quarter, quarter10, action1, action4, a1esp, a4esp, y, z);
			} else if (z+1 == 22 && !five_pt && quarter) {
				if (action1)
					mag->action1[mag->nr_action1-1].status_21d = ACTION_FAILED;
				if (action4)
					mag->action4[mag->nr_action4-1].status_21d = ACTION_FAILED;
			}
			/* Action In Progress */
			if (z+y >= mag->nr_entries-1) {
				if (!failed_signal && !five_pt) {
					if (quarter) {
						if (action1)
							algorithm_actions_a1(mag, y, z, 1, 0, quarter);
						else if (a1esp)
							algorithm_actions_esp(mag, 1, y, z, 1, quarter);
						if (action4)
							algorithm_actions_a4(mag, y, z, 1, 0, quarter);
						else if (a4esp)
							algorithm_actions_esp(mag, 4, y, z, 1, quarter);
					}
				}
				break;
			}
			double next_close = mag->close[y+z+1];
			double pc_diff    = ((next_close/current_close)-1)*100.0;
			if (pc_diff >= 5.0) {
				if (!five_pt) {
					mag->days_5pc[y]  = (z+1);
					mag->days_ret5[y] = pc_diff;
					if (quarter)
						action_5pc(mag, pc_diff, quarter, action1, action4, a1esp, a4esp, failed_signal, y, z);
				}
				five_pt = 1;
				/* *************************************
				 *              10% REACHED
				 **************************************/
				if (pc_diff >= 10.0 && !ten_pt) {
					mag->days_10pc[y]  = (z+1);
					mag->days_ret10[y] = pc_diff;
					if (!ten_pt)
						ten_pt = 1;
					if (quarter)
						action_10pc(mag, pc_diff, quarter10, action1, action4, a1esp, a4esp, failed_signal, y, z);
				}
			}
		}
	}
}

/* *******************
 *                   *
 * SARAH'S ALGORITHM *
 *                   *
 ********************/
void algorithm_mag1(struct stock *stock, struct mag *mag)
{
	double prior_close, open_price, close_price, current_close;
	int q, y, z, nr_days, nr_entries = mag->nr_entries;
	int start_entry, end_entry, max_entries;

	/* Set stock->current price points */
	stock->day_volume  = mag->volume[nr_entries-1];
	stock->prior_close = mag->close[nr_entries-1];
	stock->pr_percent  = ((stock->prior_close/mag->close[nr_entries-2])-1)*100;
	if (market != DAY_MARKET && market != PRE_MARKET) {
		stock->current_close = stock->prior_close;
		stock->current_price = stock->prior_close;
		stock->current_open  = stock->prior_close;
		stock->price_open    = stock->prior_close;
		stock->prior_close   = mag->close[nr_entries-2];
		stock->intraday_high = stock->mag->high[stock->mag->nr_entries-1];
		stock->intraday_low  = stock->mag->low [stock->mag->nr_entries-1];
	}
	if (stock->pr_percent <= -100.0 || stock->pr_percent > 7000.0) {
		printf(BOLDRED "CORRUPT PRIOR CLOSE: %s %.2f pr_percent: %.2f" RESET "\n", stock->sym, stock->prior_close, stock->pr_percent);
		stock->pr_percent = 0.0;
	}

	/* [2] Algorithm mag1 && mag2 */
	for (y=0; y<nr_entries; y++) {
		open_price  = mag->open[nr_entries-y-1];
		close_price = mag->close[nr_entries-y-1];
		prior_close = mag->close[nr_entries-y-2];
		mag->mag2[nr_entries-y-1] = (open_price>=prior_close)-(open_price<prior_close)+(close_price>=open_price)-(close_price<open_price);
		mag->mag1[nr_entries-y-1] = (close_price >= prior_close) ? 1 : -1;
		/* Calculate Up */
		if (close_price > prior_close)
			mag->up[nr_entries-y-1] = (close_price-prior_close);
		else
			mag->up[nr_entries-y-1] = 0;
		/* Calculate Down */
		if (prior_close > close_price)
			mag->down[nr_entries-y-1] = (prior_close-close_price);
		else
			mag->down[nr_entries-y-1] = 0;
		/* Delta Price Targets */
		mag->delta_open[nr_entries-y-1]  = ((open_price/prior_close)-1)*100.0;
		mag->delta_high[nr_entries-y-1]  = ((mag->high[nr_entries-y-1]/prior_close)-1)*100.0;
		mag->delta_low[nr_entries-y-1]   = ((mag->low[nr_entries-y-1]/prior_close)-1)*100.0;
		mag->delta_close[nr_entries-y-1] = ((close_price/prior_close)-1)*100.0;
	}

	/* [3] Count ONEs & TWOs */
	for (y=0; y<nr_entries-16; y++) {
		mag->two[nr_entries-1-y]   = count_twos(&mag->mag2[nr_entries-15-y]);
		mag->neg2[nr_entries-1-y]  = count_neg_twos(&mag->mag2[nr_entries-15-y]);
		mag->one[nr_entries-1-y]   = count_ones(&mag->mag1[nr_entries-15-y]);
		mag->neg1[nr_entries-1-y]  = count_neg_ones(&mag->mag1[nr_entries-15-y]);
		mag->ratio[nr_entries-1-y] = rsi_ratio(&mag->up[nr_entries-15-y], &mag->down[nr_entries-15-y]);
	}

	/* [5] Calculate PEAK */
	for (y=0; y<nr_entries-16; y++) {
		double peak = mag->close[nr_entries-y-1];
		if (nr_entries-y >= 252)
			nr_days = 252;
		else
			nr_days = nr_entries-y;
		for (z=1; z<nr_days; z++) {
			double price = mag->close[nr_entries-y-1-z];
			if (price > peak)
			 	peak = price;
		}
		mag->peak_yr[nr_entries-y-1] = peak;
		mag->peak_pc[nr_entries-y-1] = ((mag->close[nr_entries-y-1]/peak)-1)*100;
		if (y==0) {
			stock->peak_1year        = peak;
			stock->peak_1year_pc     = mag->peak_pc[nr_entries-y-1];
		}
	}

	/* [6] Delta Low/Delta High */
	for (y=0; y<252; y++) {
		double delta_high_peak = mag->delta_high[nr_entries-y-1];
		double delta_low_peak  = mag->delta_low[nr_entries-y-1];
		if (nr_entries-y >= 252)
			nr_days = 252;
		else
			nr_days = nr_entries-y;
		for (z=1; z<nr_days; z++) {
			double delta_high = mag->delta_high[nr_entries-y-1-z];
			double delta_low  = mag->delta_low[nr_entries-y-1-z];
			if (delta_high > delta_high_peak)
			 	delta_high_peak = delta_high;
			if (delta_low < delta_low_peak)
				delta_low_peak  = delta_low;
		}
		mag->delta_high_peak = delta_high_peak;
		mag->delta_low_peak  = delta_low_peak;
	}

	/* [7] Core Algorithm */
	algorithm_actions(mag, nr_entries);

	/* [8] Quarter Results */
	for (q=0; q<4; q++) {
		struct quarter *quarter   = &mag->quarters[q];
		struct quarter *quarter10 = &mag->quarters10[q];

		/*
		 * Quarter Percentages (Action 1 and Action 4)
		 */
		quarter->action1_q1r = action_q1_result(quarter->action1_q1, quarter->action1_total);
		quarter->action1_q2r = action_q2_result(quarter->action1_q1, quarter->action1_q2, quarter->action1_q1r, quarter->action1_total, quarter->action1_failed);
		quarter->action4_q1r = action_q1_result(quarter->action4_q1, quarter->action4_total);
		quarter->action4_q2r = action_q2_result(quarter->action4_q1, quarter->action4_q2, quarter->action4_q1r, quarter->action4_total, quarter->action4_failed);
		quarter10->action1_q1r = action_q1_result(quarter10->action1_q1, quarter10->action1_total);
		quarter10->action1_q2r = action_q2_result(quarter10->action1_q1, quarter10->action1_q2, quarter10->action1_q1r, quarter10->action1_total, quarter10->action1_failed);
		quarter10->action4_q1r = action_q1_result(quarter10->action4_q1, quarter10->action4_total);
		quarter10->action4_q2r = action_q2_result(quarter10->action4_q1, quarter10->action4_q2, quarter10->action4_q1r, quarter10->action4_total, quarter10->action4_failed);
		if (quarter->action1_q1 == 0 && quarter->action1_q2 > 0)
			quarter->action1_q1r = -1;
		if (quarter->action4_q1 == 0 && quarter->action4_q2 > 0)
			quarter->action4_q1r = -1;
		if (quarter10->action1_q1 == 0 && quarter10->action1_q2 > 0)
			quarter10->action1_q1r = -1;
		if (quarter10->action4_q1 == 0 && quarter10->action4_q2 > 0)
			quarter10->action4_q1r = -1;
		/*
		 * Quarter Avgdays (Action 1 and Action 4)
		 */
		if (quarter->action1_total)
			quarter->avgdays_a1   = (quarter->avgdays_total_a1/quarter->action1_total);
		else
			quarter->avgdays_a1   = 0;
		if (quarter->action4_total)
			quarter->avgdays_a4   = (quarter->avgdays_total_a4/quarter->action4_total);
		else
			quarter->avgdays_a4   = 0;
		if (quarter10->action1_total)
			quarter10->avgdays_a1 = (quarter10->avgdays_total_a1/quarter10->action1_total);
		else
			quarter10->avgdays_a1 = 0;

		if (quarter10->action4_total)
			quarter10->avgdays_a4 = (quarter10->avgdays_total_a4/quarter10->action4_total);
		else
			quarter10->avgdays_a4 = 0;
	}

	/* Trend Table, Candles */
	do_trends(stock, mag);
	stock->price->nr_points_1d = mag->nr_entries-mag->year_2022-1;
	if (nr_entries < 252)
		return;
	candle_scan(stock, mag);
	return;

	prior_close   = mag->close[nr_entries-1-252];
	current_close = mag->close[mag->nr_entries-1];
	mag->YTD      = ((current_close/prior_close)-1)*100;
	if (nr_entries < 252*2 || !mag->nr_action1 || !mag->nr_action4 || mag->year_2021 == -1 || mag->year_2022 == -1)
		return;
	/*
	end_entry   = mag->year_2021+1;
	start_entry = end_entry-252;
	max_entries = nr_entries-end_entry-1-21;
	mag->qslide = (struct qslide *)malloc(254 * sizeof(struct qslide));
	stock->nr_qslide = max_entries;

	for (x=0; x<max_entries; x++) {
		qslide_actions(stock, mag, &mag->qslide[x], start_entry+x, end_entry+x);
		qslide_anyday (stock, mag, &mag->qslide[x], start_entry+x, end_entry+x);
	}*/
}

void load_stock_csv(struct XLS *XLS, struct stock *stock, struct mag *mag, unsigned int year)
{
	struct price *price;
	int entry, start_entry, nbytes, month = 0, count_months = 0, ohlc_len = 1, close_len = 1, nr_entries = mag->nr_entries;
	time_t timestamp;

	if (stock == XLS->GSPC) {
		count_months = 1;
		month_days = &month_days_array[0];
	}

	switch (year) {
		case YEAR_2014: start_entry = mag->year_2014; break;
		case YEAR_2015: start_entry = mag->year_2015; break;
		case YEAR_2016: start_entry = mag->year_2016; break;
		case YEAR_2017: start_entry = mag->year_2017; break;
		case YEAR_2018: start_entry = mag->year_2018; break;
		case YEAR_2019: start_entry = mag->year_2019; break;
		case YEAR_2020: start_entry = mag->year_2020; break;
		case YEAR_2021: start_entry = mag->year_2021; break;
		case YEAR_2022: start_entry = mag->year_2022; break;
		case YEAR_2023: start_entry = mag->year_2023; break;
		case YEAR_2024: start_entry = mag->year_2024; break;
		default: return;
	}

	if (start_entry == -1) {
		if (mag->year_2021 == -1)
			start_entry = mag->year_2023;
		else
			start_entry = mag->year_2022;
	}
	price                    = stock->price;
	price->price_1d          = (char *)malloc(64 KB);
	price->price_1d_close    = (char *)malloc(32 KB);
	price->price_1d_close[0] = '[';
	price->price_1d[0]       = '[';

	mag->months[month++] = start_entry;
	for (entry=start_entry; entry<nr_entries; entry++) {
		year = *(unsigned int *)mag->date[entry];
		if (year == YEAR_2022) {
			if (year == months[month])
				mag->months[month++] = entry;
			if (count_months)
				month_days_array[month-1]++;
		}
		if (entry == YEAR_2023)
			month_days_array[month-1]++;

		timestamp  = ny_time(mag->date[entry])*1000;
		nbytes     = sprintf(price->price_1d+ohlc_len, "[%lu,%.2f,%.2f,%.2f,%.2f,%llu],", timestamp, mag->open[entry], mag->high[entry], mag->low[entry], mag->close[entry], mag->volume[entry]);
		ohlc_len   += nbytes;
		nbytes     = sprintf(price->price_1d_close+close_len, "[%lu,%.2f],", timestamp, mag->close[entry]);
		close_len  += nbytes;
	}
	price->price_1d_len                 = ohlc_len;
	price->price_1d_close_len           = close_len;
	price->price_1d[price->price_1d_len-1] = ']';

	*(price->price_1d_close+close_len-1) = ']';
	if (count_months) {
		current_month = month;
		printf("current_month: %d\n", current_month);
	}
}

// use for stack buffers in synchronous code
bool readfile_csv(char *ticker, char *csvbuf, int64_t max)
{
	struct stat sb;
	char        path[256];
	char       *p = NULL;

	if (max <= 0)
		return false;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/csv/%s.csv", ticker);
	if (strchr(ticker, '.')) {
		p = strchr(path, '.');
		if (p)
			*p = '-';
	}
	if (fs_stat(path, &sb) < 0 || sb.st_size < 200 || (sb.st_size >= 1024 KB))
		return 0;
	fs_readfile_str(path, csvbuf, max);
	if (p && *p == '-')
		*p = '.';
	return true;
}

void init_algo(struct XLS *XLS, struct stock *stock)
{
	struct mag   *mag;
	char          csvbuf[2048 KB];
	char         *p, *p2, *line;
	int           nr_entries, entry = 0;

	if (!readfile_csv(stock->sym, csvbuf, sizeof(csvbuf))) {
		stock->dead = 1;
		printf(BOLDRED "dead stock: %s" RESET "\n", stock->sym);
		return;
	}

	// ^GSPC
	if (*(unsigned int *)(stock->sym) == 0x5053475e)
		XLS->GSPC = stock;

	nr_entries = cstring_line_count(csvbuf);
	stock->mag = mag = (struct mag *)malloc(sizeof(struct mag));
	if (!mag)
		return;
	init_stockdata(mag, nr_entries);
	mag->nr_entries = nr_entries;

	/* [1] Extract CSV */
	line = csvbuf;
	char buf[256];
	while ((p2=strchr(line, '\n'))) {
		unsigned int year = *(unsigned int *)line;
		switch (year) {
			case YEAR_2023: if (mag->year_2023 == -1) mag->year_2023 = entry; break;
			case YEAR_2022: if (mag->year_2022 == -1) mag->year_2022 = entry; break;
			case YEAR_2021: if (mag->year_2021 == -1) mag->year_2021 = entry; break;
			case YEAR_2020: if (mag->year_2020 == -1) mag->year_2020 = entry; break;
			case YEAR_2019: if (mag->year_2019 == -1) mag->year_2019 = entry; break;
			case YEAR_2018: if (mag->year_2018 == -1) mag->year_2018 = entry; break;
			case YEAR_2017: if (mag->year_2017 == -1) mag->year_2017 = entry; break;
			case YEAR_2016: if (mag->year_2016 == -1) mag->year_2016 = entry; break;
			case YEAR_2015: if (mag->year_2015 == -1) mag->year_2015 = entry; break;
			case YEAR_2014: if (mag->year_2014 == -1) mag->year_2014 = entry; break;
		}
		p = strchr(line, ',');
		if (!p)
			break;
		*p++ = 0;
		// Open
		mag->date[entry] = strdup(line);
		mag->open[entry] = strtod(p, NULL);
		p = strchr(p+1, ',');
		if (!p)
			break;
		*p++ = 0;
		// High
		mag->high[entry] = strtod(p, NULL);
		p = strchr(p+1, ',');
		if (!p)
			break;
		*p++ = 0;
		// Low
		mag->low[entry]  = strtod(p, NULL);
		p = strchr(p+1, ',');
		if (!p)
			break;
		*p++ = 0;
		// Close
		mag->close[entry] = strtod(p, NULL);
		while (*p != ',') p++;
		p += 1;
		// AdjClose
//		mag->adj_close[entry] = strtod(p, NULL);
		while (*p != ',') p++;
		p += 1;
		// Volume
		mag->volume[entry] = strtoul(p, NULL, 10);
		while (*p != '\n') p++;
		*p++   = 0;
		entry += 1;
		line   = p2 + 1;
	}
	load_stock_csv(XLS, stock, mag, YEAR_2022);
	algorithm_mag1(stock, mag);
}

void init_anyday(struct XLS *XLS)
{
	struct stock   *stock,   **STOCKS;
	struct quarter *quarter, *quarter10;
	struct mag     *mag;
	int             days_5pc, days_10pc, idx, nr_stocks, nr_entries, nr_days;
	int             year_2023=0,year_2022=0, year_2021=0;
	int             year_2020=-1,year_2019=-1,year_2018=-1,year_2017=-1,year_2016=-1,year_2015=-1;
	double          current_close,prior_close;

	nr_stocks = XLS->nr_stocks;
	STOCKS    = XLS->STOCKS_PTR;
	for (int x=0; x<nr_stocks; x++) {
		stock = STOCKS[x];
		if (!stock || !stock->mag)
			continue;
		mag = stock->mag;
		nr_entries = mag->nr_entries;
		for (int y=0; y<8; y++) {
			switch (y) {
				case 0:
					idx = mag->year_2015;
					year_2015 = idx;
					if (idx == -1)
						continue;
					nr_days   = mag->year_2016-year_2015;
					quarter   = get_quarter("2015", mag);
					quarter10 = get_quarter10("2015", mag);
					break;
				case 1:
					idx       = mag->year_2016;
					year_2016 = idx;
					if (idx == -1)
						continue;
					nr_days   = mag->year_2017-year_2016;
					quarter   = get_quarter("2016", mag);
					quarter10 = get_quarter10("2016", mag);
					break;
				case 2:
					idx       = mag->year_2017;
					year_2017 = idx;
					if (idx == -1)
						continue;
					nr_days   = mag->year_2018-year_2017;
					quarter   = get_quarter("2017", mag);
					quarter10 = get_quarter10("2017", mag);
					break;
				case 3:
					idx       = mag->year_2018;
					year_2018 = idx;
					if (idx == -1)
						continue;
					nr_days = mag->year_2019-year_2018;
					if (year_2017 != -1) {
						prior_close   = mag->close[year_2017];
						current_close = mag->close[year_2018];
						mag->year4 = ((current_close/prior_close)-1)*100;
					}
					quarter = get_quarter("2018", mag);
					quarter10 = get_quarter10("2018", mag);
					break;
				case 4:
					idx       = mag->year_2019;
					year_2019 = idx;
					if (idx == -1)
						continue;
					nr_days = mag->year_2020-year_2019;
					if (year_2018 != -1) {
						prior_close   = mag->close[year_2018];
						current_close = mag->close[year_2019];
						mag->year3 = ((current_close/prior_close)-1)*100;
					}
					quarter = get_quarter("2019", mag);
					quarter10 = get_quarter10("2019", mag);
					break;
				case 5:
					idx = mag->year_2020;
					year_2020 = idx;
					if (idx == -1)
						continue;
					if (year_2019 != -1)
						mag->year2 = ((mag->close[year_2020]/mag->close[year_2019])-1)*100;
					nr_days   = mag->year_2021-year_2020;
					quarter   = get_quarter("2020", mag);
					quarter10 = get_quarter10("2020", mag);
					break;
				case 6:
					idx = mag->year_2021;
					year_2021 = idx;
					if (idx == -1)
						continue;
					nr_days   = year_2022-year_2021;
					if (year_2021 != -1)
						mag->year1 = ((mag->close[year_2021]/mag->close[year_2020])-1)*100;
					quarter   = get_quarter("2021", mag);
					quarter10 = get_quarter10("2021", mag);
					break;
				case 7:
					idx = mag->year_2022;
					year_2022 = idx;
					if (idx == -1)
						continue;
					nr_days   = nr_entries-year_2022;
					quarter   = get_quarter("2022", mag);
					quarter10 = get_quarter10("2022", mag);
					break;
				case 8:
					idx = mag->year_2023;
					year_2023 = idx;
					if (idx == -1)
						continue;
					nr_days   = nr_entries-year_2023;
					quarter   = get_quarter("2023", mag);
					quarter10 = get_quarter10("2023", mag);
					break;
			}

			for (int z=0; z<nr_days; z++) {
				if (z+idx >= nr_entries)
					break;
				days_5pc  = mag->days_5pc[z+idx];
				days_10pc = mag->days_10pc[z+idx];
				quarter->anyday_total++;
				quarter->avgdays_total_anyday += days_5pc;
				quarter10->anyday_total++;
				quarter10->avgdays_total_anyday += days_10pc;
				/*
				 * 5%
				 */
				if (days_5pc == 0) {
					if (year_2022)
						quarter->avgdays_total_anyday += (nr_entries-year_2022)-z;
					else
						quarter->avgdays_total_anyday += (252-z);
				}
				if (days_5pc == 0 && (z+63 < 252))
					quarter->anyday_wait++;
				else if (days_5pc < 63)
					quarter->anyday_1q++;
				else if (days_5pc < 126)
					quarter->anyday_2q++;
				else if (days_5pc < 190)
					quarter->anyday_3q++;
				else if (days_5pc <= 252)
					quarter->anyday_4q++;
				else
					quarter->anyday_fail++;

				/*
				 * 10%
				 */
				if (days_10pc == 0) {
					if (year_2022)
						quarter10->avgdays_total_anyday += (nr_entries-year_2022)-z;
					else
						quarter10->avgdays_total_anyday += (252-z);
				}
				if (days_10pc == 0 && (z+63 < 252))
					quarter10->anyday_wait++;
				else if (days_10pc < 63)
					quarter10->anyday_1q++;
				else if (days_10pc < 126)
					quarter10->anyday_2q++;
				else if (days_10pc < 190)
					quarter10->anyday_3q++;
				else if (days_10pc <= 252)
					quarter10->anyday_4q++;
				else
					quarter10->anyday_fail++;
			}
			quarter->anyday_1r        = (quarter->anyday_1q/(quarter->anyday_1q+quarter->anyday_2q+quarter->anyday_3q+quarter->anyday_4q+quarter->anyday_fail+quarter->anyday_wait))*100;
			quarter10->anyday_1r      = (quarter10->anyday_1q/(quarter10->anyday_1q+quarter10->anyday_2q+quarter10->anyday_3q+quarter10->anyday_4q+quarter10->anyday_fail+quarter10->anyday_wait))*100;
			quarter->anyday_2r        = ((quarter->anyday_1q+quarter->anyday_2q)/(quarter->anyday_1q+quarter->anyday_2q+quarter->anyday_3q+quarter->anyday_4q+quarter->anyday_fail+quarter->anyday_wait))*100;
			quarter10->anyday_2r      = ((quarter10->anyday_1q+quarter10->anyday_2q)/(quarter10->anyday_1q+quarter10->anyday_2q+quarter10->anyday_3q+quarter10->anyday_4q+quarter10->anyday_fail+quarter10->anyday_wait))*100;
			quarter->avgdays_anyday   = (quarter->avgdays_total_anyday/(double)quarter->anyday_total);
			quarter10->avgdays_anyday = (quarter10->avgdays_total_anyday/(double)quarter10->anyday_total);

/*			if (!strcmp(stock->sym, "HNP")) {
				printf("AVGDAYS ANYDAY: %.2f avgdays10: %.2f\n", quarter->avgdays_anyday, quarter10->avgdays_anyday);
				printf("QUARTER   anyday_1r: %.2f anyday_2r: %.2f (1q: %.2f 2q: %.2f 3q: %.2f 4q: %.2f) wait: %d fail: %d total: %d\n", quarter->anyday_1r, quarter->anyday_2r, quarter->anyday_1q, quarter->anyday_2q,
				quarter->anyday_3q, quarter->anyday_4q, quarter->anyday_wait, quarter->anyday_fail, (int)(quarter->anyday_1q+quarter->anyday_2q+quarter->anyday_3q+quarter->anyday_4q+quarter->anyday_wait+quarter->anyday_fail));
				printf("QUARTER10 anyday_1r: %.2f anyday_2r: %.2f (1q: %.2f 2q: %.2f 3q: %.2f 4q: %.2f) wait: %d fail: %d total: %d\n", quarter10->anyday_1r, quarter10->anyday_2r, quarter10->anyday_1q, quarter10->anyday_2q,
				quarter10->anyday_3q, quarter10->anyday_4q, quarter10->anyday_wait, quarter10->anyday_fail,(int)(quarter10->anyday_1q+quarter10->anyday_2q+quarter10->anyday_3q+quarter10->anyday_4q+quarter10->anyday_wait+quarter10->anyday_fail));
			}*/
		}
	}
}

void rpc_stockpage_anyday(struct rpc *rpc)
{
	char         *QGID   = rpc->argv[1];
	char         *ticker = rpc->argv[2];
	struct stock *stock  = search_stocks(ticker);
	struct mag   *mag;
	int           packet_len;

	if (!stock || !(mag=stock->mag))
		return;

	char abuf1[64],abuf2[64],abuf3[64];
	char abuf4[64],abuf5[64],abuf6[64];
	struct quarter *quarter_2021   = &mag->quarters[0];
	struct quarter *quarter_2020   = &mag->quarters[1];
	struct quarter *quarter_2019   = &mag->quarters[2];
	struct quarter *quarter10_2021 = &mag->quarters10[0];
	struct quarter *quarter10_2020 = &mag->quarters10[1];
	struct quarter *quarter10_2019 = &mag->quarters10[2];
	double avgdays_anyday2021      = quarter_2021->avgdays_anyday;
	double avgdays_anyday2020      = quarter_2020->avgdays_anyday;
	double avgdays_anyday2019      = quarter_2019->avgdays_anyday;
	double avgdays10_anyday2021    = quarter10_2021->avgdays_anyday;
	double avgdays10_anyday2020    = quarter10_2020->avgdays_anyday;
	double avgdays10_anyday2019    = quarter10_2019->avgdays_anyday;

	packet_len = snprintf(rpc->packet, 4096, "anyday %s %s %s %s %s %s %s %.2f %.2f %.2f %s %s %s %s %s %s %.2f %.2f %.2f@",
			QGID, qval3(quarter_2021->anyday_1r,abuf1),qval3(quarter_2021->anyday_2r,abuf2),qval3(quarter_2020->anyday_1r,abuf3),qval3(quarter_2020->anyday_2r,abuf4),qval3(quarter_2019->anyday_1r,abuf5),qval3(quarter_2019->anyday_2r,abuf6),
			avgdays_anyday2021, avgdays_anyday2020,avgdays_anyday2019,
			qval3(quarter10_2021->anyday_1r,abuf1),qval3(quarter10_2021->anyday_2r,abuf2),qval3(quarter10_2020->anyday_1r,abuf3),qval3(quarter10_2020->anyday_2r,abuf4),qval3(quarter10_2019->anyday_1r,abuf5),qval3(quarter10_2019->anyday_2r,abuf6),
			avgdays10_anyday2021, avgdays10_anyday2020,avgdays10_anyday2019);
	websocket_send(rpc->connection, rpc->packet, packet_len);
}
