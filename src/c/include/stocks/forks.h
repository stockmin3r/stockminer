#ifndef __FORKS_H
#define __FORKS_H

#include <stocks/stocks.h>

#define SIGNALS_CSV "/stockminer/python/recursion/2020.csv"
#define SIGNALS_DB  "/stockminer/db/signals.db"
#define FORKS_DB    "/stockminer/db/forks.db"

struct portfolio {
	char          *hypersonic;
	char          *supersonic;
	char          *business;
	char          *economy;
	char          *hyper_port_scatter;
	char          *super_port_scatter;
	char          *business_port_scatter;
	char          *economy_port_scatter;
	uint32_t       hypersonic_size;
	uint32_t       supersonic_size;
	uint32_t       business_size;
	uint32_t       economy_size;
	uint32_t       nr_hypersonic_ports;
	uint32_t       nr_supersonic_ports;
	uint32_t       nr_business_ports;
	uint32_t       nr_economy_ports;
	uint32_t       hyper_port_scatter_size;
	uint32_t       super_port_scatter_size;
	uint32_t       business_port_scatter_size;
	uint32_t       economy_port_scatter_size;
	uint32_t       hyper_port_scatter_max;
	uint32_t       super_port_scatter_max;
	uint32_t       business_port_scatter_max;
	uint32_t       economy_port_scatter_max;
	double         avgdays;
};

struct monster {
	struct portfolio *portfolios;
	struct sig       *signals;
	struct stock    **peak_stocks;
	char             *table;                  // MONSTER TABLES (ALL)
	char             *gztable;                // MONSTER TABLES (ALL) GZIPPED
	char             *stockpage_sigmon;       // stockpage algo screener table
	char             *quicklook;              // table 1
	char             *peakwatch;              // table 2
	char             *table3;                 // allsignals/flightInfo
	char             *table5;                 // QF Flight Saftey Table
	char             *sb150_table;            // Top 150 Stocks Table
	char             *msr_table;
	char             *msr2_table;
	uint32_t          size;                   // MONSTER TABLES SIZE
	uint32_t          gzsize;                 // MONSTER TABLES SIZE (GZIPPED)
	uint32_t          stockpage_sigmon_size;
	uint32_t          peakwatch_size;
	uint32_t          quicklook_size;
	uint32_t          table3_size;
	uint32_t          table5_size;
	uint32_t          sb150_table_size;
	uint32_t          msr_table_size;
	uint32_t          msr2_table_size;
	unsigned short    nr_signals_63d;
	unsigned short    nr_signals_128d;
	unsigned short    nr_signals_dead;
	unsigned short    nr_signals;             // ranks   0-200
	unsigned short    nr_pending;             // pending 0-200
	unsigned short    nr_signals_rank50;      // ranks   0-50
	unsigned short    nr_signals_rank100;     // ranks   51-100
	unsigned short    nr_signals_rank150;     // ranks   101-150
	unsigned short    nr_signals_rank200;     // ranks   151-200
	unsigned short    nr_pending50;
	unsigned short    nr_pending100;
	unsigned short    nr_pending150;
	unsigned short    nr_pending200;
	unsigned short    nr_peak_stocks;
};

struct forkmem {
    double        *buy_amount;
    double        *sell_amount;
    char         **buy_date;
    char         **sell_date;
    double        *cash;
    double        *capital;
    int           *nr_days;
    int           *qty;
    char         **tickers;
    double         RTD;
    double         APY;
    double         avgdays;
    char          *boarding_passes;
    char          *fork_scatter;
    char          *fork_table;
    int            boarding_passes_len;
    int            fork_tsize;
    int            fork_scatter_size;
    int            port_id;
    unsigned short nr_positions;
    unsigned short fork_class;
};

#define NR_EXP 8

struct forkdb {
	double         buy_amount[NR_EXP];
	double         sell_amount[NR_EXP];
	double         cash[NR_EXP];
	double         capital[NR_EXP];
	time_t         buy_date[NR_EXP];
	time_t         sell_date[NR_EXP];
	int            nr_days[NR_EXP];
	int            qty[NR_EXP];
	unsigned short tickers[NR_EXP];
	double         RTD;
	double         APY;
	double         avgdays;
	int            boarding_passes_len;
	int            fork_tsize;
	int            fork_scatter_size;
	int            port_id;
	unsigned short nr_positions;
	unsigned short fork_class;
};

struct sig {
	struct stock  *stock;
	char           ticker[8];
	char           entry_date[12];
	char           exit_date[12];
	double         entry_price;
	double         exit_price;
	double         ret;
	unsigned short rank;
	unsigned short nr_days;
	unsigned short status;
	unsigned short entry;       // stockdb/csv OHLC[row]
};

struct fquarter {
	char           *start;
	char           *end;
	int             total;
	int             nr_APY_SP500_5pc;
	int             nr_APY_SP500;
	int             nr_APY_10pc;
	int             nr_RTD_SP500_5pc;
	int             nr_RTD_SP500;
	int             nr_RTD_10pc;
	unsigned short  nr_signals_63d;
	unsigned short  nr_signals_128d;
	unsigned short  nr_signals_dead;
	unsigned short  nr_signals;              // ranks   0-200
	unsigned short  nr_pending;              // pending 0-200
	unsigned short  nr_pending_rank50;
	unsigned short  nr_pending_rank100;
	unsigned short  nr_pending_rank150;
	unsigned short  nr_pending_rank200;
	unsigned short  nr_signals_rank50;       // ranks   0-50     total
	unsigned short  nr_signals_rank100;      // ranks   51-100   total
	unsigned short  nr_signals_rank150;      // ranks   101-150  total
	unsigned short  nr_signals_rank200;      // ranks   151-200  total
	unsigned short  nr_signals_rank50_63d;   // ranks   0-50     <= 63d
	unsigned short  nr_signals_rank100_63d;  // ranks   51-100   <= 63d
	unsigned short  nr_signals_rank150_63d;  // ranks   101-150  <= 63d
	unsigned short  nr_signals_rank200_63d;  // ranks   151-200  <= 63d
	unsigned short  nr_signals_rank50_128d;  // ranks   0-50     <= 63d
	unsigned short  nr_signals_rank100_128d; // ranks   51-100   <= 63d
	unsigned short  nr_signals_rank150_128d; // ranks   101-150  <= 63d
	unsigned short  nr_signals_rank200_128d; // ranks   151-200  <= 63d
	unsigned short  nr_signals_rank50_dead;  // ranks   0-50     <= 63d (PENDING)
	unsigned short  nr_signals_rank100_dead; // ranks   51-100   <= 63d (PENDING)
	unsigned short  nr_signals_rank150_dead; // ranks   101-150  <= 63d (PENDING)
	unsigned short  nr_signals_rank200_dead; // ranks   151-200  <= 63d (PENDING)
	double          APY_SP500_5pc;
	double          APY_SP500;
	double          APY_10pc;
	double          RTD_SP500_5pc;
	double          RTD_SP500;
	double          RTD_10pc;
};

#define NR_QUARTER_YEARS  3

struct fq {
	struct fquarter  fquarter[4];
};

static __inline__ int fork_list(char *ptr, char **list, char **end)
{
	char *p;
	int nr_items = 0, x;

	for (x=0; x<128; x++) {
		*list++ = ptr;
		p = strchr(ptr, '\'');
		if (!p)
			return 0;
		*p = 0;
		nr_items++;
		if (*(p+1) == ']') {
			*end = p+3;
			break;
		}
		ptr = p + 4;
	}
	return (nr_items);
}

static __inline__ int fork_list2(char *ptr, char **list, char **end)
{
	char *p;
	int nr_items = 0, x;

	p = strchr(ptr, ']');
	if (!p)
		return 0;
	*p = 0;
	*end = p+2;
	for (x=0; x<128; x++) {
		*list++ = ptr;
		nr_items++;
		p = strchr(ptr, ',');
		if (!p)
			break;
		*p = 0;
		if (*(p+1) == ']') {
			*end = p+3;
			break;
		}
		ptr = p + 2;
	}
	return (nr_items);
}

#endif
