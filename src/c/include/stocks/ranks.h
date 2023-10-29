#ifndef __RANKS_H
#define __RANKS_H

#include <stocks/stocks.h>

struct rank_table {
	struct stock      **ranked_stocks;
	struct esp_signal **esp_signals;
	int                 nr_esp_signals;
	char                espbuf[4096 KB];
	int                 esp_len;
	int                 main_ranks;
	int                 rank_month;
};
extern struct rank_table *rank_tables;

struct XLS;

void init_ranks   (struct XLS *XLS);
int  date_to_rank (struct stock *stock, char *date);
#endif
