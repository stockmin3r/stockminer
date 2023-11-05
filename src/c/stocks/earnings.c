/* License: Public Domain */

#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

#define MINI_EARN_TABLE "<table id=mini><caption id=CPL>Earnings</caption><thead><tr><th>Ticker</th><th>Days</th></tr></thead><tbody>"
#define MINI_ENTRY      "<tr class=mrow><td>%s</td><td>%d</td></tr>"

#define EARN_TABLE  "<table id=ETAB class=XTAB style=width:45%><caption id=ECP>Earnings Performance<caption><thead><tr><th>Days</th>"
//#define EARN_THEAD  "<th>%s</th>"
#define EARN_THEND  "</tr></thead>"

/*#define EARN_ENTRY1 "<tr><td>%d</td><td %s>%d%%</td></tr>"
#define EARN_ENTRY2 "<tr><td>%d</td><td %s>%d%%</td><td %s>%d%%</td></tr>"
#define EARN_ENTRY3 "<tr><td>%d</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td></tr>"
#define EARN_ENTRY4 "<tr><td>%d</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td></tr>"
#define EARN_ENTRY5 "<tr><td>%d</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td></tr>"
#define EARN_ENTRY6 "<tr><td>%d</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td><td %s>%d%%</td></tr>"
*/
#define EARN_ENTRY1 "%d:%d%% "
#define EARN_ENTRY2 "%d:%d%%:%d%% "
#define EARN_ENTRY3 "%d:%d%%:%d%%:%d%% "
#define EARN_ENTRY4 "%d:%d%%:%d%%:%d%%:%d%% "
#define EARN_ENTRY5 "%d:%d%%:%d%%:%d%%:%d%%:%d%% "
#define EARN_ENTRY6 "%d:%d%%:%d%%:%d%%:%d%%:%d%%:%d%% "
#define EARN_ENTRY7 "%d:%d%%:%d%%:%d%%:%d%%:%d%%:%d%%:%d%% "

char *MINI_TABLE(int *table_size, int table_id)
{
	struct stock *stock, **stock_table;
	struct earnings *e;
	struct XLS *XLS = CURRENT_XLS;
	char *table, *tptr, *p;
	int tsize = 0, rsize, x, y, count, nr_highcaps, nr_lowcaps;

	table = (char *)malloc(8 KB);
	if (!table)
		return NULL;
	strcpy(table, MINI_EARN_TABLE);
	tptr   = table+sizeof(MINI_EARN_TABLE)-1;
	tsize += sizeof(MINI_EARN_TABLE)-1;

	nr_highcaps = XLS->nr_highcaps;
	nr_lowcaps  = XLS->nr_lowcaps;
	if (table_id == HIGHCAPS_TABLE) {
		stock_table = XLS->HIGHCAPS;
		count       = nr_highcaps;
	} else {
		stock_table = XLS->LOWCAPS;
		count       = nr_lowcaps;
	}

	for (x=0; x<count; x++) {
		stock = stock_table[x];
		e = stock->EARNINGS;
		if (e->nr_earnings <= 0 || e->earning_days > 10 || e->earning_days <= 0)
			continue;
		rsize  = snprintf(tptr, 128, MINI_ENTRY, stock->sym, e->earning_days);
		tptr  += rsize;
		tsize += rsize;
	}
	strcpy(tptr, "</tbody></table>");
	tsize   += 16;
	*table_size = tsize;
	return (table);
}

void rpc_stockpage_earnings(struct rpc *rpc)
{
	char            *table = rpc->packet;
	struct stock    *stock = search_stocks(rpc->argv[1]);
	char            *QGID  = rpc->argv[2];
	struct earnings *e;
	double           prior_close, price_open, close_high;
	double           ecount1_high, ecount2_high, ecount3_high, ecount4_high, ecount5_high, ecount6_high, ecount7_high;
	int              ecount, table_size, rsize, x, y;

	if (!stock || !(ecount=stock->EARNINGS->nr_earnings))
		return;

	table_size = snprintf(table, 32, "etab %s ", QGID);
	e          = stock->EARNINGS;
	for (x=0; x<ecount; x++) {
		if (!strcmp(e->earn[x].date, e->next_earnings))
			break;
		close_high  = 0;
		for (y=0; y<30; y++) {
			double close_price = e->earn[x].pc[y];
			if (close_price > close_high) {
				close_high = close_price;
//				printf("new close price high: %.2f ecount: %d y: %d\n", close_high, x, y);
				switch (x) {
					case 1:ecount1_high = y;break;
					case 2:ecount2_high = y;break;
					case 3:ecount3_high = y;break;
					case 4:ecount4_high = y;break;
					case 5:ecount5_high = y;break;
					case 6:ecount6_high = y;break;
					case 7:ecount7_high = y;break;
				}
			}
		}
	}
	switch (ecount) {
		case 1: table_size += sprintf(table+table_size, "%s ",                   e->earn[0].date); break;
		case 2: table_size += sprintf(table+table_size, "%s:%s ",                e->earn[0].date, e->earn[1].date); break;
		case 3: table_size += sprintf(table+table_size, "%s:%s:%s ",             e->earn[0].date, e->earn[1].date, e->earn[2].date); break;
		case 4: table_size += sprintf(table+table_size, "%s:%s:%s:%s ",          e->earn[0].date, e->earn[1].date, e->earn[2].date, e->earn[3].date); break;
		case 5: table_size += sprintf(table+table_size, "%s:%s:%s:%s:%s ",       e->earn[0].date, e->earn[1].date, e->earn[2].date, e->earn[3].date, e->earn[4].date); break;
		case 6: table_size += sprintf(table+table_size, "%s:%s:%s:%s:%s:%s ",    e->earn[0].date, e->earn[1].date, e->earn[2].date, e->earn[3].date, e->earn[4].date, e->earn[5].date); break;
		case 7: table_size += sprintf(table+table_size, "%s:%s:%s:%s:%s:%s:%s ", e->earn[0].date, e->earn[1].date, e->earn[2].date, e->earn[3].date, e->earn[4].date, e->earn[5].date, e->earn[6].date); break;
	}
	printf("ecount: %d table: %s ec1: %.2f ec2: %.2f ec3: %.2f\n", ecount, table, ecount1_high, ecount2_high, ecount3_high);
	for (x=29; x>=0; x--) {
		switch (ecount) {
			case 1:
				rsize = sprintf(table+table_size, EARN_ENTRY1, x+1, (int)round(e->earn[0].pc[x]));
				break;
			case 2:
				rsize = sprintf(table+table_size, EARN_ENTRY2, x+1, (int)round(e->earn[0].pc[x]), (int)round(e->earn[1].pc[x]));
				break;
			case 3:
				rsize = sprintf(table+table_size, EARN_ENTRY3, x+1, (int)round(e->earn[0].pc[x]), (int)round(e->earn[1].pc[x]), (int)round(e->earn[2].pc[x]));
				break;
			case 4:
				rsize = sprintf(table+table_size, EARN_ENTRY4, x+1, (int)round(e->earn[0].pc[x]), (int)round(e->earn[1].pc[x]), (int)round(e->earn[2].pc[x]), (int)round(e->earn[3].pc[x]));
				break;
			case 5:
				rsize = sprintf(table+table_size, EARN_ENTRY5, x+1, (int)round(e->earn[0].pc[x]), (int)round(e->earn[1].pc[x]), (int)round(e->earn[2].pc[x]), (int)round(e->earn[3].pc[x]), (int)round(e->earn[4].pc[x]));
				break;
			case 6:
				rsize = sprintf(table+table_size, EARN_ENTRY6, x+1,
				(int)round(e->earn[0].pc[x]), (int)round(e->earn[1].pc[x]), (int)round(e->earn[2].pc[x]),
				(int)round(e->earn[3].pc[x]), (int)round(e->earn[4].pc[x]), (int)round(e->earn[5].pc[x]));
				break;
			case 7:
				rsize = sprintf(table+table_size, EARN_ENTRY7, x+1,
				(int)round(e->earn[0].pc[x]), (int)round(e->earn[1].pc[x]), (int)round(e->earn[2].pc[x]),
				(int)round(e->earn[3].pc[x]), (int)round(e->earn[4].pc[x]), (int)round(e->earn[5].pc[x]), (int)round(e->earn[6].pc[x]));
				break;
		}
		table_size += rsize;
	}
	*(table+table_size-1) = '@';
	table[table_size]     = 0;
	printf(BOLDWHITE "%s" RESET "\n", table);
	websocket_send(rpc->connection, table, table_size);
}

/*

int earnings_table(struct stock *stock, char *table)
{
	struct earnings *e;
	char *ecolor1, *ecolor2, *ecolor3, *ecolor4, *ecolor5, *ecolor6;
	double prior_close, price_open, ecount1_high, ecount2_high, ecount3_high, ecount4_high, ecount5_high, ecount6_high, close_high;
	int ecount, table_size = 0, rsize, x, y;
	double close_high;

	ecount = stock->EARNINGS->nr_earnings;
	if (!ecount)
		return 0;

	table_size = 0;
	strcpy(table, EARN_TABLE);
	e = stock->EARNINGS;
	table_size += sizeof(EARN_TABLE)-1;
	table      += sizeof(EARN_TABLE)-1;
	for (x=0; x<ecount; x++) {
		if (!strcmp(e->earn[x].date, e->next_earnings))
			break;
		rsize  = sprintf(table, EARN_THEAD, e->earn[x].date);
		table += rsize;
		table_size += rsize;
		close_high  = 0;
		for (y=0; y<30; y++) {
			double close_price = e->earn[x].pc[y];
			if (close_price > close_high) {
				close_high = close_price;
//					printf("new close price high: %.2f ecount: %d y: %d\n", close_high, x, y);
				switch (x) {
					case 1:ecount1_high = y;break;
					case 2:ecount2_high = y;break;
					case 3:ecount3_high = y;break;
					case 4:ecount4_high = y;break;
					case 5:ecount5_high = y;break;
					case 6:ecount6_high = y;break;
				}
			}
		}
	}
	for (x=29; x>=0; x--) {
		switch (ecount) {
			case 1:
				if (ecount1_high == x)
					ecolor1 = "class=green2";
				else
					ecolor1 = "";
				rsize = sprintf(tptr, EARN_ENTRY1, x+1, ecolor1, (int)round(e->earn[0].pc[x]));
				break;
			case 2:
				if (ecount1_high == x)
					ecolor1 = "class=green2";
				else
					ecolor1 = "";
				if (ecount2_high == x)
					ecolor2 = "class=green2";
				else
					ecolor2 = "";
				rsize = sprintf(tptr, EARN_ENTRY2, x+1, ecolor1, (int)round(e->earn[0].pc[x]), ecolor2, (int)round(e->earn[1].pc[x]));
				break;
			case 3:
				if (ecount1_high == x)
					ecolor1 = "class=green2";
				else
					ecolor1 = "";
				if (ecount2_high == x)
					ecolor2 = "class=green2";
				else
					ecolor2 = "";
				if (ecount3_high == x)
					ecolor3 = "class=green2";
				else
					ecolor3 = "";
				rsize = sprintf(tptr, EARN_ENTRY3, x+1, ecolor1, (int)round(e->earn[0].pc[x]), ecolor2, (int)round(e->earn[1].pc[x]), ecolor3, (int)round(e->earn[2].pc[x]));
				break;
			case 4:
				if (ecount1_high == x)
					ecolor1 = "class=green2";
				else
					ecolor1 = "";
				if (ecount2_high == x)
					ecolor2 = "class=green2";
				else
					ecolor2 = "";
				if (ecount3_high == x)
					ecolor3 = "class=green2";
				else
					ecolor3 = "";
				if (ecount4_high == x)
					ecolor4 = "class=green2";
				else
					ecolor4 = "";
				rsize = sprintf(tptr, EARN_ENTRY4, x+1, ecolor1, (int)round(e->earn[0].pc[x]), ecolor2, (int)round(e->earn[1].pc[x]), ecolor3, (int)round(e->earn[2].pc[x]), ecolor4, (int)round(e->earn[3].pc[x]));
				break;
			case 5:
				if (ecount1_high == x)
					ecolor1 = "class=green2";
				else
					ecolor1 = "";
				if (ecount2_high == x)
					ecolor2 = "class=green2";
				else
					ecolor2 = "";
				if (ecount3_high == x)
					ecolor3 = "class=green2";
				else
					ecolor3 = "";
				if (ecount4_high == x)
					ecolor4 = "class=green2";
				else
					ecolor4 = "";
				if (ecount5_high == x)
					ecolor5 = "class=green2";
				else
					ecolor5 = "";
				rsize = sprintf(tptr, EARN_ENTRY5, x+1, ecolor1, (int)round(e->earn[0].pc[x]), ecolor2, (int)round(e->earn[1].pc[x]), ecolor3, (int)round(e->earn[2].pc[x]), ecolor4, (int)round(e->earn[3].pc[x]), ecolor5, (int)round(e->earn[4].pc[x]));
				break;
			case 6:
				if (ecount1_high == x)
					ecolor1 = "class=green2";
				else
					ecolor1 = "";
				if (ecount2_high == x)
					ecolor2 = "class=green2";
				else
					ecolor2 = "";
				if (ecount3_high == x)
					ecolor3 = "class=green2";
				else
					ecolor3 = "";
				if (ecount4_high == x)
					ecolor4 = "class=green2";
				else
					ecolor4 = "";
				if (ecount5_high == x)
					ecolor5 = "class=green2";
				else
					ecolor5 = "";
				if (ecount6_high == x)
					ecolor6 = "class=green2";
				else
					ecolor6 = "";
				rsize = sprintf(tptr, EARN_ENTRY6, x+1, ecolor1,
				(int)round(e->earn[0].pc[x]), ecolor2, (int)round(e->earn[1].pc[x]), ecolor3,
				(int)round(e->earn[2].pc[x]), ecolor4, (int)round(e->earn[3].pc[x]), ecolor5,
				(int)round(e->earn[4].pc[x]), ecolor6, (int)round(e->earn[5].pc[x]));
				break;
		}
		table      += rsize;
		table_size += rsize;
	}
	strcpy(table, "</tbody></table>");
	table_size += 16;
	return (table_size);
}
*/

/* ************************************************
 *
 * Produce *.e binary files from *.earn text files
 *
 **************************************************/
int build_earnings(struct stock *stock)
{
	struct earn     *earn;
	struct earnings  stock_earnings;
	struct mag      *mag;
	char             path[256];
	char             buf[1024];
	char             dbuf[1024];
	struct tm        tm;
	char            *date, *pDate;
	long             next_earnings, prior_earnings;
	time_t           etimestamp, last_earnings, current_time = time(NULL);
	int              fd, x, y, z, nr_entries, nr_stock_earnings = 0;

	if (!stock->mag)
		return 0;
	memset(&stock_earnings, 0, sizeof(stock_earnings));
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.edates", stock->sym);
	if (!fs_readfile_str(path, buf, sizeof(buf))) {
		unlink(path);
		return 0;
	}
	date = buf;
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.e", stock->sym);
	unlink(path);
	fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0644);
	for (x=0; x<10; x++) {
		pDate = strchr(date, '\n');
		if (!pDate)
			break;
		*pDate++ = 0;
		date += 5;

		memset(&tm, 0, sizeof(tm));
		tm.tm_isdst = -1;
		strptime(date, "%Y-%m-%d", &tm);
		etimestamp = mktime(&tm);
		if (etimestamp < current_time) {
			mag = stock->mag;
			nr_entries = mag->nr_entries;
			earn = &stock_earnings.earn[nr_stock_earnings++];
			prior_earnings = etimestamp;
			strcpy(earn->date, date);
			for (y=0; y<nr_entries-2; y++) {
				if (!strcmp(mag->date[nr_entries-y-1], date)) {
					double earnings_close = mag->close[nr_entries-y-1];
					for (z=0; z<30; z++) {
						double prev_earnings = mag->close[nr_entries-y-2-z];
						earn->pc[z] = ((earnings_close/prev_earnings)-1)*100;
					}
				}
			}
			date = pDate;
			last_earnings = etimestamp;
			continue;
		}
		break;
	}
	next_earnings  = (etimestamp-current_time)/(24*3600)+1;
	prior_earnings = (prior_earnings-current_time)/(24*3600)+1;
	if (labs(prior_earnings) < next_earnings)
		stock_earnings.earning_days = prior_earnings;
	else
		stock_earnings.earning_days = next_earnings;

	stock_earnings.next_ts = etimestamp;
	strcpy(stock_earnings.next_earnings, date);
	if (etimestamp > current_timestamp) {
		*(stock_earnings.next_earnings+4) = 0;
		*(stock_earnings.next_earnings+7) = 0;
		snprintf(dbuf, sizeof(dbuf)-1, "%s/%s/%s", stock_earnings.next_earnings+5, stock_earnings.next_earnings+8, stock_earnings.next_earnings);
		if (Server.DEBUG_MODE)
			printf("%s\n", dbuf);
		strcpy(stock_earnings.next_earnings, dbuf);
	}

	if (nr_stock_earnings <= 0) {
		printf(BOLDRED "no earnings: %s" RESET "\n", stock->sym);
		return 0;
	}
	stock_earnings.nr_earnings = nr_stock_earnings;
	stock->EARNINGS = &stock_earnings;
	memcpy(&stock->earnings, &stock_earnings, sizeof(stock_earnings));
	stock->EARNINGS = &stock->earnings;

	if (Server.DEBUG_MODE) {
		printf(BOLDWHITE "[%s] Next Earnings: %s" RESET "\n", stock->sym, stock->EARNINGS->next_earnings);
		close(fd);
		goto out;
	}
	write(fd, (void *)&stock_earnings, sizeof(stock_earnings));
	close(fd);
out:
	if (*stock->EARNINGS->next_earnings == '/')
		return 0;
	return 1;
}

void build_stock_earnings()
{
	struct stock **stocks;
	struct XLS    *XLS = CURRENT_XLS;
	int            nr_stocks;

	stocks    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (int x=0; x<nr_stocks; x++)
		build_earnings(stocks[x]);
}

void *get_earnings(void *args)
{
	char cmd[256];
	char *stock = (char *)args;

	snprintf(cmd, sizeof(cmd)-1, "python3 earn.py %s", stock);
	system(cmd);
	snprintf(cmd, sizeof(cmd)-1, "sed -i '/report_date/d' data/stocks/stockdb/%s.edates", stock);
	system(cmd);
	return (NULL);
}

void get_earnings_dates(char *ticker, struct XLS *XLS)
{
	struct stat sb;
	char        path[256];
	char      **stocks, *p;
	int         x, nr_stocks;

	if (ticker) {
		get_earnings(ticker);
		exit(0);
	}

	stocks    = XLS->STOCKS_STR;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		ticker = stocks[x];
		if ((p=strchr(ticker, '.')))
			*p = '-';
		snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.edates", ticker);
		if (!stat(path, &sb))
			continue;
		printf(BOLDGREEN "EARNINGS DATES FOR: %s" RESET "\n", ticker);
		thread_create(get_earnings, ticker);
		os_sleep(4);
	}
	os_sleep(1000000);
}

void update_earnings(struct XLS *XLS)
{
	struct stock *stock, **stocks;
	int x, nr_unsynced = 0, nr_stocks;

	stocks    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = stocks[x];
		if (stock->EARNINGS->earning_days <= 0 || *stock->EARNINGS->next_earnings == '/') {
			if (!build_earnings(stock)) {
				printf(BOLDRED "stock has no next earnings: %s" RESET "\n", stock->sym);
				get_earnings(stock->sym);
			}
			nr_unsynced++;
		}
	}
	printf("unsynchronized: %d\n", nr_unsynced);
}

void print_earnings(char *ticker)
{
	struct earnings *e;
	char             buf[4096];
	char             path[256];
	int              x, y;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.e", ticker);
	if (!fs_readfile(path, buf, sizeof(buf))) {
		printf(BOLDRED "[-] Failed to read earnings file: %s" RESET "\n", path);
		return;
	}
	e = (struct earnings *)buf;
	printf("nr earnings: %d\n", e->nr_earnings);
	printf("Next Earnings: %s\n", e->next_earnings);
	for (x=0; x<e->nr_earnings; x++) {
		printf("-------------------------\n");
		printf(BOLDWHITE "%s" RESET "\n", e->earn[x].date);
		for (y=0; y<30; y++) {
			printf("%.2f\n", e->earn[x].pc[y]);
		}
		printf("-------------------------\n");
	}
	printf("etimestamp: %d\n", (int)e->next_ts);
}

void cmd_dump_earnings(int sockfd)
{
	struct stock     *stock;
	struct stock    **stocks;
	struct XLS       *XLS = CURRENT_XLS;
	struct earnings  *e;
	int               days, nr_stocks;

	recv(sockfd, (void *)&days, 4, 0);
	stocks    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (int x=0; x<nr_stocks; x++) {
		stock = stocks[x];
		if (!stock || stock->EARNINGS->earning_days > days)
			continue;
		e = stock->EARNINGS;
		printf(BOLDWHITE "---------------%s (Next Earnings: %d)-------------" RESET "\n", stock->sym, e->earning_days);
		printf(BOLDBLUE "---------------------------------" RESET "\n");
	}
}
