#include <conf.h>
#include <stocks/stocks.h>

int                rmonth[100] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
int                NR_RANKS    = 19;
int                MAX_RANKS   = 200;
struct rank_table *rank_tables;
bool               ranks_initialized = false;

int date_to_rank(struct stock *stock, char *date)
{
	int year  = atoi(date+2);
	int month = atoi(date+5);

	switch (year) {
		case 19:
			if (month == 12)
				return stock->ranks_2020[0];
			return stock->ranks_2019[month-1];
		case 20:
			if (month == 12)
				return stock->ranks_2021[0];
			return stock->ranks_2020[month-1];
		case 21:
			if (month == 12)
				return stock->ranks_2022[0];
			return stock->ranks_2021[month-1];
		case 22:
			if (month == 12)
				return stock->ranks_2022[0];
			return stock->ranks_2022[month-1];
	}
	return 0;
}

void rpc_set_max_ranks(struct rpc *rpc)
{
	int max = atoi(rpc->argv[1]);

	if (max <= 0 || max >= 10000)
		return;
	MAX_RANKS = max;
}

// ranks.csv has been obsolete for years & will have to be replaced with new modular ranking system
void upload_ranks(struct XLS *XLS, struct connection *connection, char *rankfile, int filesize)
{
	char *status;

	/* Backup Previous Ranks */
	rename("data/stocks/ranks/ranks.txt", "data/stocks/ranks/.backup_ranks.txt");
	rename("data/stocks/ranks/ranks.csv", "data/stocks/ranks/.backup_ranks.csv");

	/* Generate New Ranks */
	fs_newfile("data/stocks/ranks/ranks.csv", rankfile, filesize);
	system("dos2unix --force data/stocks/ranks/ranks.csv");
	system("mac2unix --force data/stocks/ranks/ranks.csv");
	status = "admin Converting-new-ranks.csv-to-ranks.txt";
	websocket_send(connection, status, strlen(status));
	system("cd data/stocks/ranks && ./ranks.sh");
	status = "admin New-ranks.txt-generated";
	websocket_send(connection, status, strlen(status));

	/* Reload new Ranks */
	init_ranks(XLS);
	/* Create a new 2020.csv */
	init_signals(XLS);
	status = "admin New-signals.csv-generated";
	websocket_send(connection, status, strlen(status));

	/* Sort 2020.csv */
	system("scripts/ranksort.sh");
	status = "admin New-signals.csv-sorted";
	websocket_send(connection, status, strlen(status));

	/* Generate forks.txt */
	unlink("python/recursion/forks.txt");
	system("cd python/recursion && python3 single_fork_threaded.py");
	status = "admin New-forks.txt-generated";
	websocket_send(connection, status, strlen(status));

	/* Generate 2020.txt */
	system("cd python && python/run recursion/recursion.py 2>/dev/shm/recursion");
	status = "admin New-signals.txt-created";
	websocket_send(connection, status, strlen(status));

	/* Update QuickLook/PeakWatch */
	update_quicklook(XLS);
	update_peakwatch(XLS);

	/* Reload forks */
	load_forks(XLS);

	/* Regenerate Portfolio/Flyport tables */
	generate_monster_db(XLS);
	status = "admin Ranks-Updated";
	websocket_send(connection, status, strlen(status));

}

void init_ranks(struct XLS *XLS)
{
	struct stock *stock;
	char          buf[1024 KB];
	char         *ticker, *newline, *map, *p, *p2;
	int           x, nr_ranks, rank, year, month;

	if (ranks_initialized)
		return;

	fs_readfile_str((char *)"data/stocks/ranks/ranks.txt", buf, sizeof(buf));
	ticker = buf;
	XLS->ranked_stocks = (struct stock **)malloc(XLS->nr_stocks * sizeof(struct stock *));
	XLS->max_ranks = MAX_RANKS;
	while ((newline=strchr(ticker, '\n'))) {
		nr_ranks   = 0;
		*newline++ = 0;
		p = strchr(ticker, ',');
		if (!p) {
			ticker = newline;
			continue;
		}
		*p = 0;
		if ((p2=(char *)memchr(ticker, 'd', 6)))
			*p2 = '-';
		if (!strcmp(ticker, "GSPC"))
			ticker = "^GSPC";
		else if (!strcmp(ticker, "IXIC"))
			ticker = "^IXIC";
		else if (!strcmp(ticker, "RUT"))
			ticker = "^RUT";
		else if (!strcmp(ticker, "DJI"))
			ticker = "^DJI";
		stock  = search_stocks_XLS(XLS, ticker);
		if (!stock) {
			ticker = newline;
			continue;
		}
		ticker = newline;
		if (!stock)
			continue;
		*p++ = ',';

		stock->nr_ranks = 0;
		while ((p2=strchr(p, ','))) {
			rank = atoi(p);
			p2 = strchr(p, ',');
			if (!p2)
				break;
			p2++;
			if (*(p2+7) == ',' || *(p2+7) == '\0') {
				month   = *(p2+4) - 48;
				*(p2+4) = 0;
				p       = p2 + 8;
			} else {
				*(p2+6) = 0;
				month   = atoi(p2+4);
				*(p2+4) = 0;
				p       = p2 + 9;
			}
			year = atoi(p2+2);

			switch (year) {
				case 19:
					if (month == 12)
						stock->ranks_2020[0] = rank;
					stock->ranks_2019[month] = rank;
					break;
				case 20:
					if (month == 12)
						stock->ranks_2021[0] = rank;
					stock->ranks_2020[month] = rank;
					break;
				case 21:
					if (month == 12)
						stock->ranks_2021[0] = rank;
					stock->ranks_2021[month] = rank;
					break;
				case 22:
					if (month == 12)
						stock->ranks_2022[0] = rank;
					stock->ranks_2022[month] = rank;
			}
			/* Current Rank */
			if (stock->nr_ranks == 0) {
				stock->rank = rank;
				stock->rankstr[cstring_itoa(stock->rankstr, rank)-1] = 0;
				XLS->ranked_stocks[rank-1] = stock;
			} else if (stock->nr_ranks == 1)
				stock->prev_rank = rank;
			stock->nr_ranks++;
			if (*(p-1) == '\0')
				break;
		}
	}
	ranks_initialized = false;
}
