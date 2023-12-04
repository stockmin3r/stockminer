#ifndef __ALGO_H
#define __ALGO_H

#include <stdinc.h>

#define NO_COLOR          "class=empty"
#define ORANGE_COLOR      "class=orange"
#define GREEN_COLOR       "class=grn"
#define DARK_GREEN_COLOR  "class=green"
#define YELLOW_COLOR      "class=yellow"
#define GREY_COLOR        "class=grey"

#define YEAR_2026 0x36323032
#define YEAR_2025 0x35323032
#define YEAR_2024 0x34323032
#define YEAR_2023 0x33323032
#define YEAR_2022 0x32323032
#define YEAR_2021 0x31323032
#define YEAR_2020 0x30323032 // 0202
#define YEAR_2019 0x39313032
#define YEAR_2018 0x38313032
#define YEAR_2017 0x37313032
#define YEAR_2016 0x36313032
#define YEAR_2015 0x35313032
#define YEAR_2014 0x34313032

#define ESP_BLANK          0
#define ESP_NO2            1
#define ESP_STAY           2
#define ESP_ONE            3
#define ESP_1NO2           4
#define ESP_NONEG1         5
#define ESP_NONEG1_TWO     6
#define NEXT_DAY_ACTION    1
#define NEXT_DAY_ESP       2
#define ACTION_HOLD        1
#define ACTION_BUY         2
#define ACTION_WATCH       3
#define ACTION_EXIT        4
#define ACTION_NONE        5
#define ACTION_FAILED      0
#define ACTION_SUCCESS     1
#define ACTION_PENDING     2
#define TRUE_ESP           1

#define RANGING_MARKET  (1<<0)
#define TRENDING_MARKET (1<<1)
#define BULLISH_MACD    (1<<2)
#define BEARISH_MACD    (1<<3)
#define BULLISH_RSI     (1<<4)
#define BEARISH_RSI     (1<<5)
#define BULLISH_BB      (1<<6)
#define BEARISH_BB      (1<<7)
#define BULLISH_STOCH   (1<<8)
#define BEARISH_STOCH   (1<<9)
#define BULLISH_KCH     (1<<10)
#define BEARISH_KCH     (1<<11)
#define BULLISH_AROON   (1<<12)
#define BEARISH_AROON   (1<<13)
#define BULLISH_CCI     (1<<14)
#define BEARISH_CCI     (1<<15)

#define MAX_XLS 1764

struct stock;

struct action {
	double          a1_quarter;
	double          a1_quarter2;
	double          a4_quarter;
	double          a4_quarter2;
	double          three_pt;
	double          five_pt;
	double          ten_pt;
	unsigned short  nr_days;
	unsigned short  entry;
	unsigned char   status;
	unsigned char   status_21d;
	unsigned short  three_days;
	unsigned short  five_days;
	unsigned short  ten_days;
	unsigned char   a1esp;
	unsigned char   a4esp;
	unsigned char   type;
	unsigned short  a1total;
	unsigned short  a4total;
};

struct quarter {
	double          action1_q1;
	double          action1_q2;
	double          action4_q1;
	double          action4_q2;
	double          a1esp_q1;
	double          a1esp_q2;
	double          a4esp_q1;
	double          a4esp_q2;
	double          action1_total;
	double          action4_total;
	double          a1esp_total;
	double          a4esp_total;
	double          action1_q1r;
	double          action1_q2r;
	double          action4_q1r;
	double          action4_q2r;
	double          action1_failed;
	double          action4_failed;
	double          a1esp_failed;
	double          a4esp_failed;
	double          anyday_1q;
	double          anyday_2q;
	double          anyday_3q;
	double          anyday_4q;
	double          anyday_1r;
	double          anyday_2r;
	double          anyday_3r;
	double          anyday_4r;
	double          avgdays_a1;
	double          avgdays_a4;
	double          avgdays_a1esp;
	double          avgdays_a4esp;
	double          avgdays_anyday;
	unsigned short  action1_inprogress;
	unsigned short  action4_inprogress;
	unsigned short  a1esp_inprogress;
	unsigned short  a4esp_inprogress;
	unsigned short  avgdays_total_a1;
	unsigned short  avgdays_total_a4;
	unsigned short  avgdays_total_a1esp;
	unsigned short  avgdays_total_a4esp;
	unsigned short  avgdays_total_anyday;
	unsigned short  anyday_total;
	unsigned short  anyday_wait;
	unsigned short  anyday_fail;
};

struct qslide {
	double          a1q1;
	double          a4q1;
	double          avgdays_a1q1;
	double          avgdays_a4q1;
	double          a1ret;
	double          a4ret;
	double          a1std;
	double          a4std;
	double          anyday_avg;
	double          anyday_pc;
	double          anyday_ret;
	double          anyday_std;
	unsigned short  a1total;
	unsigned short  a4total;
	unsigned short  entry;
};

struct mag {
	char          **date;
	double         *open;
	double         *high;
	double         *low;
	double         *close;
	uint64_t       *volume;
	unsigned short  nr_entries;
	unsigned short  nr_candles;
	unsigned short  nr_action1;
	unsigned short  nr_action4;
	unsigned short  nr_a1esp;
	unsigned short  nr_a4esp;
	unsigned short  nr_action1_success;
	unsigned short  nr_action4_success;
	unsigned short  nr_a1esp_success;
	unsigned short  nr_a4esp_success;
	unsigned short  max_candles;
	unsigned short *days_5pc;
	unsigned short *days_10pc;
	double         *days_ret5;
	double         *days_ret10;
	double         *peak_yr;
	double         *peak_pc;
	double         *delta_open;
	double         *delta_high;
	double         *delta_low;
	double         *delta_close;
	double         *BIX;
	double         *up;
	double         *down;
	double         *ratio;
	char           *mag1;
	char           *mag2;
	char           *one;
	char           *two;
	char           *neg1;
	char           *neg2;
	double         *a1q1;
	double         *a1q2;
	double         *a4q1;
	double         *a4q2;
	double         *weeks;
	unsigned short *a1total;
	unsigned short *a4total;
	struct action  *action1;
	struct action  *action4;
	struct action  *a1esp;
	struct action  *a4esp;
	struct qslide  *qslide;
	struct CANDLE  *candles;
	struct quarter  quarters[8];
	struct quarter  quarters10[8];
	unsigned short  months[64];
	double          delta_high_peak;
	double          delta_low_peak;
	double          YTD;
	double          year1;
	double          year2;
	double          year3;
	double          year4;
	int16_t         year_2024;
	int16_t         year_2023;
	int16_t         year_2022;
	int16_t         year_2021;
	int16_t         year_2020;
	int16_t         year_2019;
	int16_t         year_2018;
	int16_t         year_2017;
	int16_t         year_2016;
	int16_t         year_2015;
	int16_t         year_2014;
	char            nr_weeks_up;
	char            nr_weeks_down;
};

struct mag2 {
	double         days_5pc;       // 0
	double         days_ret_5pc;
	double         days_max_5pc;
	double         days_10pc;
	double         days_ret_10pc;
	double         days_max_10pc;
	double         days_15pc;
	double         days_ret_15pc;
	double         days_max_15pc;
	double         days_20pc;
	double         days_ret_20pc;  // 10
	double         days_max_20pc;
	double         a1esp;
	double         action1;
	double         a1same;
	double         a1result;
	double         a4esp;
	double         action4;
	double         day1;           // 18
	double         day3;
	double         day5;
	double         day8;
	double         day13;
	double         day21;
	double         day42;
	double         day63;          // 25
	double         RTD;
	double         streak;
	double         dir;
	double         buy;            // 29
	double         buy_delta;      // 30
	double         sell;           // 31
	double         sell_delta;     // 32
	double         fib;            // 33
	double         fib_dir;        // 34
	double         buy_fib;        // 35
	double         buy_delta_fib;
	double         sell_fib;
	double         sell_delta_fib;
	double         mean;
	double         std;            // 40
	double         var_90;
	double         var_95;
	double         var_99;
	double         var_99pot;
	double         one_year_ago;   // 45
	double         one_year_pkpc;  // 46
	double         YTD;
	double         sig_21_a1;
	double         sig_42_a1;
	double         sig_63_a1;      // 50
	double         max_days_5pc_21_a1;
	double         max_days_5pc_42_a1;
	double         max_days_5pc_63_a1;
	double         success__5pc_21_a1;
	double         success__5pc_42_a1;
	double         success__5pc_63_a1;
	double         max_days_10pc_21_a1;
	double         max_days_10pc_42_a1;
	double         max_days_10pc_63_a1;
	double         success__10pc_21_a1; // 60
	double         success__10pc_42_a1;
	double         success__10pc_63_a1;
	double         sig_21_a4;
	double         sig_42_a4;
	double         sig_63_a4;
	double         max_days_5pc_21_a4;
	double         max_days_5pc_42_a4;
	double         max_days_5pc_63_a4;
	double         success__5pc_21_a4;
	double         success__5pc_42_a4;  // 70
	double         success__5pc_63_a4;
	double         max_days_10pc_21_a4;
	double         max_days_10pc_42_a4;
	double         max_days_10pc_63_a4;
	double         success__10pc_21_a4;
	double         success__10pc_42_a4;
	double         success__10pc_63_a4;
	double         one_year_pk_price;
	double         plimit;
	double         pdelta;             // 80
	double         one_yr_p10;
	double         one_yr_p5;
	double         last_peak;
	double         p10orp5;
	double         peak;
	double         sig;                // 86
} __attribute__((packed));

struct mag3 {
	double        volume_adi;
	uint64_t      volume_obv;
	double        volume_cmf;
	double        volume_fi;
	double        volume_mfi;
	double        volume_em;
	double        volume_sma_em;
	double        volume_vpt;
	double        volume_nvi;
	double        volume_vwap;
	double        vol_atr;
	double        vol_bbm;
	double        vol_bbh;
	double        vol_bbl;
	double        vol_bbw;
	double        vol_bbp;
	double        vol_bbhi;
	double        vol_bbli;
	double        vol_kcc;
	double        vol_kch;
	double        vol_kcl;
	double        vol_kcw;
	double        vol_kcp;
	double        vol_kchi;
	double        vol_kcli;
	double        vol_dcl;
	double        vol_dch;
	double        vol_dcm;
	double        vol_dcw;
	double        vol_dcp;
	double        vol_ui;
	double        tr_macd;
	double        tr_macd_sig;
	double        tr_macd_diff;
	double        tr_sma_fast;
	double        tr_sma_slow;
	double        tr_ema_fast;
	double        tr_ema_slow;
	double        tr_adx;
	double        tr_adx_pos;
	double        tr_adx_neg;
	double        tr_vtx_pos;
	double        tr_vtx_neg;
	double        tr_vtx_diff;
	double        tr_trix;
	double        tr_mass_idx;
	double        tr_cci;
	double        tr_dpo;
	double        tr_kst;
	double        tr_kst_sig;
	double        tr_kst_diff;
	double        tr_ICH_conv;
	double        tr_ICH_base;
	double        tr_ICH_a;
	double        tr_ICH_b;
	double        tr_vICH_a;
	double        tr_vICH_b;
	double        tr_aroon_up;
	double        tr_aroon_down;
	double        tr_aroon_ind;
	double        psar_up;
	double        psar_down;
	double        psar_up_ind;
	double        psar_down_ind;
	double        tr_stc;
	double        mom_rsi;
	double        stoch_rsi;
	double        stoch_rsi_k;
	double        stoch_rsi_d;
	double        mom_tsi;
	double        mom_uo;
	double        mom_stoch_k;
	double        mom_stoch_d;
	double        mom_wr;
	double        mom_ao;
	double        mom_kama;
	double        mom_roc;
	double        mom_ppo;
	double        mom_ppo_sig;
	double        mom_ppo_hist;
	double        others_dr;
	double        others_dlr;
	double        others_cr;
};

struct mag4 {
	double        sma20;
	double        sma50;
	double        sma100;
	double        sma150;
	double        sma200;
	double        ema10;
	double        ema20;
	double        ema50;
	double        ema100;
	double        ema200;
	double        mfi;
};


struct esp_signal {
	char               ticker[16];
	char               date[16];
	char               date2[16];
	int                next_day_action;
	double             next_day_low;
	double             next_cl;
	double             day1;
	double             day2;
	double             day3;
	double             day4;
	double             day5;
	double             day6;
	double             day7;
	double             day8;
	double             day9;
	double             day10;
	double             day11;
	double             day12;
	double             day13;
	double             day14;
	double             day15;
	double             price;
	double             delta;
	double             peak;
	double             a1q1;
	double             a1q2;
	double             a4q1;
	double             a4q2;
	int                a1total;
	int                a4total;
	int                day_index;
	unsigned short     day;
	unsigned short     a1esp;
	unsigned short     a4esp;
	unsigned short     a1s;
	unsigned short     a4s;
	struct stock      *stock;
	struct esp_signal *next_esp;
	time_t             timestamp;
	int                one;
	int                two;
	int                mag1;
	int                mag2;
	int                action;
	int                year_index;
	int                esp;
	int                bought;
};

struct week {
	char              *start_date_string;
	char              *end_date_string;
	time_t             start_date_unix;
	time_t             end_date_unix;
};

extern struct week WEEKS[52];
extern int *month_days;

//(open>=prior close)-(open<priorclose)+(close>=open)-(close<open)

static __inline__ const char *esp_str(int esp)
{
	switch (esp) {
		case ESP_STAY:
			return "stay";
		case ESP_NO2:
			return "no2";
		case ESP_BLANK:
			return "";
		case ESP_ONE:
			return "one";
		case ESP_1NO2:
			return "1no2";
		case ESP_NONEG1:
			return "no-1";
		case ESP_NONEG1_TWO:
			return "no-1-2";
	}
	return NULL;
}

static __inline__ int is_a1esp(struct mag *mag, int entry)
{
	int two,mag2,esp;

	if (entry < 14)
		return 0;
	mag2 = mag->mag2[entry-14];
	two  = mag->two[entry];

	if (two == 3 && mag2 == 2)
		esp = ESP_NO2;
	else
		esp = ESP_BLANK;
	return (esp);
}

static __inline__ int is_a1esp_same(struct mag *mag, int entry)
{
	int two,mag2;

	if (entry < 15)
		return 0;
	two  = mag->two[entry];
	mag2 = mag->mag2[entry-14];

	if (mag2 == 0 || mag2 == 1)
		return ESP_STAY;
	if (two == 2) {
		if (mag2 == 0 || mag2 == -2)
			return ESP_NO2;
		else
			return ESP_STAY;
	}
	return ESP_BLANK;
}

static __inline__ int is_a4esp(struct mag *mag, int entry)
{
	int one,two,mag1,mag2,esp;

	if (entry < 15)
		return 0;
	one  = mag->one[entry];
	two  = mag->two[entry];
	mag1 = mag->mag1[entry-14];
	mag2 = mag->mag2[entry-14];

	if (one >= 10 && mag2 == 2 && two == 4)
		esp = ESP_NO2;
	else if (one == 9 && mag1 == -1 && mag2 == 2 && two == 4)
		esp = ESP_NO2;
	else
		esp = ESP_BLANK;
	return (esp);
}

static __inline__ int is_a4esp_same(struct mag *mag, int entry)
{
	int one,two,mag1,mag2;

	if (entry < 15)
		return 0;
	one  = mag->one[entry];
	two  = mag->two[entry];
	mag1 = mag->mag1[entry-14];
	mag2 = mag->mag2[entry-14];

	if (one >= 10) {
		if (two == 0 || two == 1 || two == 2)
			return (ESP_STAY);
		else {
			if (two == 3) {
				if (mag2 == 0 || mag2 == -2)
					return (ESP_NO2);
				else
					return (ESP_STAY);
			}
		}
	} else if (one == 9) {
		if (mag1 == -1) {
			if (two == 0 || two == 1 || two == 2)
				return (ESP_STAY);
			else {
				if (two == 3) {
					if (mag2 == 0 || mag2 == -2)
						return (ESP_NO2);
					else
						return (ESP_STAY);
				}
			}
		}
	}
	return (ESP_BLANK);
}

static __inline__ int is_action1(struct mag *mag, int entry)
{
	int two;

	two = mag->two[entry];
	if (two <= 2 && entry > 15)
		return 1;
	return 0;
}

static __inline__ int is_action4(struct mag *mag, int entry)
{
	double one, two;

	one = mag->one[entry];
	two = mag->two[entry];
	if (one >= 9 && two <= 3)
		return 1;
	return 0;
}

static __inline__ int is_delta_high(double *array, double high)
{
	int x;

	for (x=0; x<9; x++) {
		if (*array++ >= high)
			return 0;
	}
	return 1;
}

static __inline__ int is_delta_low(double *array, double low)
{
	int x;

	for (x=0; x<9; x++) {
		if (*array++ <= low)
			return 0;
	}
	return 1;
}

static __inline__ double delta(double current_close, double prior_close)
{
	return ((current_close/prior_close)-1)*100.0;
}

static __inline__ char count_twos(char *array)
{
	char nr_twos = 0;
	int x;

	for (x=0; x<15; x++) {
		if (*array == 2)
			nr_twos++;
		array++;
	}
	return (nr_twos);
}

static __inline__ char count_neg_twos(char *array)
{
	char nr_twos = 0;
	int x;

	for (x=0; x<15; x++) {
		if (*array == -2)
			nr_twos++;
		array++;
	}
	return (nr_twos);
}


static __inline__ char count_ones(char *array)
{
	char nr_ones = 0;
	int x;

	for (x=0; x<15; x++) {
		if (*array == 1)
			nr_ones++;
		array++;
	}
	return (nr_ones);
}

static __inline__ char count_neg_ones(char *array)
{
	char nr_ones = 0;
	int x;

	for (x=0; x<15; x++) {
		if (*array == -1)
			nr_ones++;
		array++;
	}
	return (nr_ones);
}

static __inline__ double rsi_ratio(double *up, double *down)
{
	double up_total = 0.0, down_total = 0.0;
	int x;

	for (x=0; x<15; x++) {
		up_total   += *up++;
		down_total += *down++;
	}
	return (up_total/down_total);
}

static __inline__ const char *stock_peak(double peak, char *pbuf)
{
	if (peak == 0) {
		strcpy(pbuf, "Peak");
		return (pbuf);
	}
	if (peak == -100)
		return "";
	sprintf(pbuf, "%.2f", peak);
	return (pbuf);
}

static __inline__ const char *stock_peak2(double peak, char *pbuf)
{
	if (peak == 0) {
		strcpy(pbuf, "Peak");
		return (pbuf);
	}
	if (peak == -100)
		return "";
	sprintf(pbuf, "%.1f", peak);
	return (pbuf);
}

static __inline__ const char *year_peak(double peak, char *pbuf) 
{
	if (peak == 0) {
		strcpy(pbuf, "Peak");
		return (pbuf);
	}
	if (peak > 4000)
		return "";
	sprintf(pbuf, "%.2f", peak);
	return (pbuf);
}

static __inline__ const char *SIGNAL(double sig)
{
	if (sig == 1.0)
		return "sig";
	return "";
}

static __inline__ const char *PEAK(double peak)
{
	if (peak == 1.0)
		return "peak";
	return "";
}

static __inline__ double stdev(double *input, int nr_days)
{
	double sum = 0, mean = 0;
	int x;

	for (x=0; x<nr_days; x++)
		sum += input[x];
	mean = sum / (nr_days);
	sum  = 0;
	for (x=0; x<nr_days; x++)
		sum += (input[x]-mean) * (input[x]-mean);
	return sqrt(sum/nr_days);
}

static __inline__ struct quarter *get_quarter(char *date, struct mag *mag)
{
	switch (*(date+3)) {
		case '2': // 2022
			if (*(date+2) != '2')
				return NULL;
			return &mag->quarters[0];
		case '1': // 2021
			return &mag->quarters[1];
		case '0': // 2020
			return &mag->quarters[2];
		case '9': // 2019
			return &mag->quarters[3];
		case '8': // 2018
			return &mag->quarters[4];
		case '7': // 2017
			return &mag->quarters[5];
		case '6': // 2016
			return &mag->quarters[6];
		case '5': // 2015
			return &mag->quarters[7];
	}
	return NULL;
}

static __inline__ struct quarter *get_quarter10(char *date, struct mag *mag)
{
	switch (*(date+3)) {
		case '2': // 2022
			if (*(date+2) != '2')
				return NULL;
			return &mag->quarters[0];
		case '1': // 2021
			return &mag->quarters[1];
		case '0': // 2020
			return &mag->quarters10[2];
		case '9': // 2019
			return &mag->quarters10[3];
		case '8': // 2018
			return &mag->quarters10[4];
		case '7': // 2017
			return &mag->quarters10[5];
		case '6': // 2016
			return &mag->quarters10[6];
		case '5': // 2015
			return &mag->quarters10[7];
	}
	return NULL;
}

static __inline__ int date2entry(struct mag *mag, char *target_date)
{
	int start_entry, nr_days, x;

	switch (*(target_date+3)) {
		case '3': // 2023
			if (*(target_date+2) != '2')
				return 0.0;
			start_entry = mag->year_2023;
			break;
		case '2': // 2022
			if (*(target_date+2) != '2')
				return 0.0;
			start_entry = mag->year_2022;
			break;
		case '1': // 2021
			start_entry = mag->year_2021;
			break;
		case '0': // 2020
			start_entry = mag->year_2020;
			break;
		case '9': // 2019
			start_entry = mag->year_2019;
			break;
		case '8': // 2018
			start_entry = mag->year_2018;
			break;
	}
	nr_days = mag->nr_entries-start_entry;
	for (x=0; x<nr_days; x++) {
		if (!strcmp(mag->date[start_entry+x]+5, target_date+5))
			return (start_entry+x);
	}
	return -1;
}

static __inline__ int count_market_days(struct mag *mag, char *start_date, char *end_date)
{
	int start_entry = date2entry(mag, start_date);
	int end_entry   = date2entry(mag, end_date);
	return (end_entry-start_entry);
}

static __inline__ const char *quarter_color(double val)
{
	if (val == 100.0)
		return "class=darkgreen";
	if (val >= 90)
		return "class=lightgreen";
	if (val >= 80)
		return "class=blue";
	if (val >= 70)
		return "class=yellow";
	return "class=empty";
}

static __inline__ void set_month(char *dst_date, char *src_date)
{
	int d_index = 0;

	if (*(src_date+5) == '0') {
		dst_date[d_index++] = src_date[6];
		dst_date[d_index++] = '/';
	} else {
		dst_date[d_index++] = src_date[5];
		dst_date[d_index++] = src_date[6];
		dst_date[d_index++] = '/';
	}
	if (*(src_date+8) == '0') {
		dst_date[d_index++] = src_date[9];
		dst_date[d_index]   = 0;
	} else {
		dst_date[d_index++] = src_date[8];
		dst_date[d_index++] = src_date[9];
		dst_date[d_index]   = 0;
	}
}

#define qval2(v,b) quarter_value_css(v,b)
#define qval       quarter_value
#define qval3      quarter_value3

static __inline__ const char *quarter_value(double value, char *buf)
{
	if (!value) {
		strcpy(buf, "none");
		return (buf);
	} else if (value == -1)
		return "0%";
	sprintf(buf, "%d%%", (int)value);
	return (buf);
}

static __inline__ const char *quarter_value3(double value, char *buf)
{
	if (!value) {
		strcpy(buf, "-");
		return (buf);
	} else if (value == -1)
		return "0%";
	sprintf(buf, "%d%%", (int)value);
	return (buf);
}

static __inline__ const char *quarter_value_css(double value, char *buf)
{
	if (!value) {
		return ">none";
	} else if (value == -1)
		return "class=w>0%";
	if (value == 100.0)
		return "class=G>100%";
	else if (value >= 80.0)
		sprintf(buf, "class=B>%d%%", (int)value);
	else if (value >= 70.0)
		sprintf(buf, "class=Y>%d%%", (int)value);
	else
		sprintf(buf, "class=w>%d%%", (int)value);
	return (buf);
}

static __inline__ const char *standard_deviation(double value, char *buf)
{
	if (value >= 1.0)
		sprintf(buf, "%.1f", value);
	else
		sprintf(buf, "%.2f", value);
	return (buf);
}

static __inline__ const char *earning_days(int days, char *buf)
{
	if (days <= 0 || days > 300)
		return "";
	sprintf(buf, "%d", days);
	return (buf);
}

#endif
