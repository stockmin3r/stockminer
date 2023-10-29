#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

struct board *lowcaps_gainer_board;
struct board *lowcaps_loser_board;
struct board *highcaps_gainer_board;
struct board *highcaps_loser_board;
int NR_DELTA_STOCKS  = 19;

/* Price Lowcaps */
struct board *price_15m;
struct board *price_5m;
struct board *price_1m;
struct board *price_15mL;
struct board *price_5mL;
struct board *price_1mL;
/* Price Highcaps */
struct board *price_15mH;
struct board *price_5mH;
struct board *price_1mH;
struct board *price_15mLH;
struct board *price_5mLH;
struct board *price_1mLH;
/* Volume Lowcaps */
struct board *volume_15m;
struct board *volume_5m;
struct board *volume_1m;
struct board *volume_15mH;
struct board *volume_5mH;
struct board *volume_1mH;
/* Indicators */
struct board *bull_board;
struct board *bear_board;

char *json_board(struct board *board, int *json_len) {
	int buffer = board->buffer;
	switch (buffer) {
		case 1: *json_len = board->json_size1; return board->buf1;
		case 2: *json_len = board->json_size2; return board->buf2;
	}
	return NULL;
}

static void get_ufostocks(struct stock **ufostocks, struct board **boards, int interval, int table) {
	switch (interval) {
		case 15:
			if (table == 1) memcpy(ufostocks, boards[LOWCAP_R15_BOARD]->stocks,   sizeof(void *) * 8);
			else            memcpy(ufostocks, boards[LOWCAP_R15L_BOARD]->stocks,  sizeof(void *) * 8);  break;
		case 5:
			if (table == 1) memcpy(ufostocks, boards[LOWCAP_R5_BOARD]->stocks,    sizeof(void *) * 8);
			else            memcpy(ufostocks, boards[LOWCAP_R5L_BOARD]->stocks,   sizeof(void *) * 8);  break;
		case 1:
			if (table == 1) memcpy(ufostocks, boards[LOWCAP_R1_BOARD]->stocks,    sizeof(void *) * 8);
			else            memcpy(ufostocks, boards[LOWCAP_R1L_BOARD]->stocks,   sizeof(void *) * 8);  break;
	}
}

static void get_hfostocks(struct stock **ufostocks, struct board **boards, int interval, int table) {
	switch (interval) {
		case 15:
			if (table == 1) memcpy(ufostocks, boards[HIGHCAP_R15_BOARD]->stocks,  sizeof(void *) * 8);
			else            memcpy(ufostocks, boards[HIGHCAP_R15L_BOARD]->stocks, sizeof(void *) * 8);  break;
		case 5:
			if (table == 1) memcpy(ufostocks, boards[HIGHCAP_R5_BOARD]->stocks,   sizeof(void *) * 8);
			else            memcpy(ufostocks, boards[HIGHCAP_R5L_BOARD]->stocks,  sizeof(void *) * 8);  break;
		case 1:
			if (table == 1) memcpy(ufostocks, boards[HIGHCAP_R1_BOARD]->stocks,   sizeof(void *) * 8);
			else            memcpy(ufostocks, boards[HIGHCAP_R1L_BOARD]->stocks,  sizeof(void *) * 8);  break;
	}
}

static void get_vfostocks(struct stock **vfostocks, struct board **boards, int interval, int table) {
	switch (interval) {
		case 15:
			if (table == 1) memcpy(vfostocks, boards[LOWCAP_V15_BOARD]->stocks,  sizeof(void *) * 8);
			else            memcpy(vfostocks, boards[HIGHCAP_V15_BOARD]->stocks, sizeof(void *) * 8); break;
		case 5:
			if (table == 1) memcpy(vfostocks, boards[LOWCAP_V5_BOARD]->stocks,   sizeof(void *) * 8);
			else            memcpy(vfostocks, boards[HIGHCAP_V5_BOARD]->stocks,  sizeof(void *) * 8); break;
		case 1:
			if (table == 1) memcpy(vfostocks, boards[LOWCAP_V1_BOARD]->stocks,   sizeof(void *) * 8);
			else            memcpy(vfostocks, boards[HIGHCAP_V1_BOARD]->stocks,  sizeof(void *) * 8); break;
	}
}

void ufo_scan(struct XLS *XLS)
{
	struct stock  *stock;
	struct board **boards;
	uint64_t       current_volume,v1,v5,v15;
	int            x, y, nr_stocks;

	nr_stocks = XLS->nr_stocks;
	boards    = XLS->boards;
	for (x=0; x<nr_stocks; x++) {
		stock = &XLS->STOCKS_ARRAY[x];
		if (!stock->pr_percent || !stock->update || stock->current_price != 0.0 || stock->nr_ohlc < 16) {
//			debug_printf("failed ufo: %s curprice: %.2f prior: %.2f pr_per: %.2f nr_ohlc: %d\n", stock->sym, stock->current_price, stock->prior_close, stock->pr_percent, stock->nr_ohlc);
			continue;
		}
		stock->price_1min  = ((stock->current_price  / stock->ohlc[stock->nr_ohlc-2].close) -1)*100.0;
		stock->price_5m    = ((stock->current_price  / stock->ohlc[stock->nr_ohlc-6].close) -1)*100.0;
		stock->price_15m   = ((stock->current_price  / stock->ohlc[stock->nr_ohlc-16].close)-1)*100.0;
		if (Server.DEBUG_STOCK && !strcmp(stock->sym, Server.DEBUG_STOCK))
			printf(BOLDBLUE "ufo_scan(): [%s] current_price: %.2f prior_close: %.2f 15m: %.2f 5m: %.2f 1m: %.2f" RESET "\n", stock->sym, stock->current_price, stock->ohlc[stock->nr_ohlc-15].close, stock->price_15m, stock->price_5m, stock->price_1min);
		current_volume     = stock->current_volume;
		if (current_volume) {
			v1  = stock->ohlc[stock->nr_ohlc-2].volume;
			v5  = stock->ohlc[stock->nr_ohlc-6].volume;
			v15 = stock->ohlc[stock->nr_ohlc-16].volume;
			if (v1)
				stock->volume_1m  = ((current_volume / v1) -1)*100.0;
			if (v5)
				stock->volume_5m  = ((current_volume / v5) -1)*100.0;
			if (v15)
				stock->volume_15m = ((current_volume / v15)-1)*100.0;
		}
		if (stock->type == STOCK_TYPE_LOWCAPS) {
				add_delta_gainer(stock, boards[LOWCAP_GAINER_BOARD]);   // frontpage Lowcaps Gainer Table
				add_delta_loser (stock, boards[LOWCAP_LOSER_BOARD]);    // frontpage Lowcaps Loser  Table
			for (y=0; y<3; y++) {
				add_double_gainer(stock, boards[LOWCAP_BOARD_IDX+y]);    // UFO G 15m/5m/1m
				add_double_loser (stock, boards[LOWCAP_BOARD_IDX+y+3]);  // UFO L 15m/5m/1m
			}
			for (y=0; y<3; y++)
				add_double_gainer(stock, boards[VLOW_BOARD_IDX+y]);      // UFO L Volume 15m/5m/1m Tables
		} else if (stock->type == STOCK_TYPE_HIGHCAPS) {
				add_delta_gainer(stock, boards[HIGHCAP_GAINER_BOARD]);  // frontpage Highcaps Gainer Table
				add_delta_loser (stock, boards[HIGHCAP_LOSER_BOARD]);   // frontpage Highcaps Loser Table
			for (y=0; y<3; y++) {
				add_double_gainer(stock, boards[HIGHCAP_BOARD_IDX+y]);   // UFO G 15m/5m/1m
				add_double_loser (stock, boards[HIGHCAP_BOARD_IDX+y+3]); // UFO L 15m/5m/1m
			}
			for (y=0; y<NR_VHIGH_BOARDS; y++)
				add_double_gainer(stock, boards[VHIGH_BOARD_IDX+y]);     // UFO H Volume 15m/5m/1m Tables
		}
	}
}

int ufo_tables(struct session *session, char *packet, struct workspace *workspace)
{
	struct board *board;
	struct XLS   *XLS = CURRENT_XLS;
	char *json;
	int json_size, packet_len = 0, x;

	if (workspace->ufo_tables & UFO_LOWCAPS) {
		for (x=0; x<NR_LOWCAP_BOARDS; x++) {
			board = XLS->boards[LOWCAP_BOARD_IDX+x];
			strcpy(packet+packet_len, board->table);
			packet_len += board->tsize;
			json        = json_board(board, &json_size);
			memcpy(packet+packet_len, json, json_size);
			packet_len += json_size;
			*(packet+packet_len++) = '@';
		}
	}
	if (workspace->ufo_tables & UFO_HIGHCAPS) {
		for (x=0; x<NR_HIGHCAP_BOARDS; x++) {
			board = XLS->boards[HIGHCAP_BOARD_IDX+x];
			strcpy(packet+packet_len, board->table);
			packet_len += board->tsize;
			json        = json_board(board, &json_size);
			memcpy(packet+packet_len, json, json_size);
			packet_len += json_size;
			*(packet+packet_len++) = '@';
		}
	}	
	if (workspace->ufo_tables & UFO_VOLUME) {
		for (x=0; x<NR_VOLUME_BOARDS; x++) {
			board = XLS->boards[VOLUME_BOARD_IDX+x];
			strcpy(packet+packet_len, board->table);
			packet_len += board->tsize;
			json        = json_board(board, &json_size);
			memcpy(packet+packet_len, json, json_size);
			packet_len += json_size;
			*(packet+packet_len++) = '@';
		}
	}
	packet[packet_len] = 0;
	return (packet_len);
}

/* Pack UFO Charts */
int ufo_load_charts(struct session *session, char *packet, struct workspace *workspace, struct wsid *wsid, int interval, int table)
{
	struct stock     *ufostocks[20];
	struct chart     *chart;
	char              moveTo[128] = { '0', 0 };
	int               packet_len  = 0, nbytes, x;

	if (workspace->ufo_tables & UFO_LOWCAPS)
		get_ufostocks(ufostocks, CURRENT_XLS->boards, interval, table);
	else if (workspace->ufo_tables & UFO_HIGHCAPS)
		get_hfostocks(ufostocks, CURRENT_XLS->boards, interval, table);
	else if (workspace->ufo_tables & UFO_VOLUME)
		get_vfostocks(ufostocks, CURRENT_XLS->boards, interval, table);
	else
		return 0;

	workspace->watchtables = ufo_tables;
	workspace->nr_charts   = 0;

	for (x=0; x<8; x++) {
		struct stock *stock = ufostocks[x];
		chart = workspace->charts[workspace->nr_charts] = (struct chart *)zmalloc(sizeof(*chart));
		if (x >= 4)
			snprintf(moveTo, sizeof(moveTo)-1, "#P%dQ%dq%dws%d-%sbox", wsid->QVID, wsid->QSID, wsid->QID, wsid->WSID, ufostocks[x-4]->sym);
		nbytes = pack_mini(stock, packet+packet_len, chart, wsid, 1, moveTo, 250, interval);
		packet_len  += nbytes;
		*(packet+packet_len) = '@';
		packet_len  += 1;
		chart->stock = stock;
		chart->type  = CHART_TYPE_MINI;
		workspace->nr_charts++;
	}
	packet_len--;
	*(packet+packet_len) = 0;
	return (packet_len);
}

/* [ufoinit CMD ID TABLE QGID] */
void rpc_ufo(struct rpc *rpc)
{
	struct session    *session      = rpc->session;
	struct connection *connection   = rpc->connection;
	char              *packet       = rpc->packet;
	char               price_volume = *(rpc->argv[1]);
	char               cap          = *(rpc->argv[1]+1);
	int                table        = *(rpc->argv[1]+2)-48;
	int                interval     = atoi(rpc->argv[2]);
	char              *QGID         = rpc->argv[3];
	struct workspace  *workspace;
	struct wsid        wsid;
	int                packet_len;

	if (price_volume != 'P' && price_volume != 'V')
		return;
	if (cap != 'L' && cap != 'H' && cap != 'V')
		return;
	if (table != 1 && table != 2)
		return;
	if (interval != 15 && interval != 5 && interval != 1)
		return;

	if (!workspace_id(session, QGID, &wsid))
		return;
	if (!(workspace=get_workspace(session, &wsid, NULL)))
		return;

	/* First Call */
	if (price_volume == 'P') {
		if (cap == 'L') {
			printf("setting ufo lowcaps\n");
			workspace->ufo_tables = UFO_LOWCAPS;
		}
		else if (cap == 'H') {
			printf("setting ufo highcaps\n");
			workspace->ufo_tables = UFO_HIGHCAPS;
		}
	} else if (price_volume == 'V') {
		workspace->ufo_tables = UFO_VOLUME;
	}
	/* Tables */
	packet_len  = ufo_tables(session, packet, workspace);

	/* Charts */
	packet_len += ufo_load_charts(session, packet+packet_len, workspace, &wsid, interval, table);
	websocket_send(connection, packet, packet_len);
	rpc->packet[packet_len] = 0;
	printf("new ufo packet: %s\n", rpc->packet);
}

void rpc_stockpage_indicators(struct rpc *rpc)
{
	char         *table = rpc->argv[1];
	struct board *board;
	char         *json;
	int packet_len, json_size, idx;
	
	if (!strcmp(table, "bulls"))
		idx = BULL_BOARD_IDX;
	else if (!strcmp(table, "bears"))
		idx = BEAR_BOARD_IDX;
	else
		return;
	
	board = CURRENT_XLS->boards[idx];
	strcpy(rpc->packet, board->table);
	packet_len  = board->tsize;
	json        = json_board(board, &json_size);
	memcpy(rpc->packet+packet_len, json, json_size);
	packet_len += json_size;
	websocket_send(rpc->connection, rpc->packet, packet_len);
}

void rpc_ufo_megachart(struct rpc *rpc)
{
	char         *ticker = rpc->argv[1];
	char         *QGID   = rpc->argv[2];
	struct stock *stock  = search_stocks(ticker);
	struct wsid   WSID;
	int           packet_len;

	if (!workspace_id(rpc->session, QGID, &WSID))
		return;
	packet_len = pack_mini(stock, rpc->packet, NULL, &WSID, 0, "0", 805, 15);
	websocket_send(rpc->connection,  rpc->packet, packet_len);
}

int peakwatch_tables(struct session *session, char *packet, struct workspace *workspace)
{
	struct monster *monster;
	struct XLS     *XLS;
	struct stock   *stock;
	struct stock  **peak_stocks;
	struct peakwatch_workarea *workarea;
	int x, packet_len, nr_peak_stocks;

	if (market != DAY_MARKET)
		return 0;

	strcpy(packet, "peak ");
	packet_len = 5;
	workarea = (struct peakwatch_workarea *)workspace->workarea;
	XLS            = CURRENT_XLS;
	monster        = (struct monster *)XLS->MONSTER;
	if (!monster)
		return 0;
	nr_peak_stocks = monster->nr_peak_stocks;
	peak_stocks    = monster->peak_stocks;
	for (x=0; x<nr_peak_stocks; x++) {
		stock = peak_stocks[x];
/*		if (!workarea->last_price) {
			workarea->last_price = zmalloc(sizeof(double) * nr_peak_stocks);
			workarea->last_price[stock->id] = stock->current_price;
			continue;
		}
		if (workarea->last_price[stock->id] == stock->current_price)
			continue;
		workarea->last_price[stock->id] = stock->current_price;*/
		packet_len += sprintf(packet+packet_len, "%.2f:%.2f ", stock->current_price, stock->pr_percent);
	}
	packet[packet_len-1] = '@';
	return (packet_len);
}

/* convert board to JSON */
int lowcap_tables(struct session *session, char *packet, struct workspace *workspace)
{
	struct board *board;
	struct XLS   *XLS = CURRENT_XLS;
	char         *json;
	int           json_size, packet_len = 0, x;

	for (x=0; x<2; x++) {
		board       = XLS->boards[x];
		strcpy(packet+packet_len, board->table);
		packet_len += board->tsize;
		json        = json_board(board, &json_size);
		memcpy(packet+packet_len, json, json_size);
		packet_len += json_size;
		*(packet+packet_len++) = '@';
	}
	packet[packet_len] = 0;
	return (packet_len);
}

int highcap_tables(struct session *session, char *packet, struct workspace *workspace)
{
	struct board *board;
	struct XLS   *XLS = CURRENT_XLS;
	char *json;
	int json_size, packet_len = 0, x;

	for (x=0; x<2; x++) {
		board = XLS->boards[x+2];
		strcpy(packet+packet_len, board->table);
		packet_len += board->tsize;
		json        = json_board(board, &json_size);
		memcpy(packet+packet_len, json, json_size);
		packet_len += json_size;
		*(packet+packet_len++) = '@';
	}
	return (packet_len);
}

void send_highcaps(struct rpc *rpc)
{
	int packet_len = highcap_tables(rpc->session, rpc->packet, NULL);
	if (packet_len)
		websocket_send(rpc->connection, rpc->packet, packet_len-1);
}

int stock_present(struct stock **bstocks, struct stock *stock, int max) {
	int x;
	for (x=0; x<max; x++)
		if (bstocks[x] == stock)
			return 1;
	return 0;
}

/* **********************************
 *
 *     DOUBLE GAINERS/LOSERS (UFO)
 *
 ***********************************/
void add_double_gainer(struct stock *stock, struct board *board)
{
	if (stock->type == STOCK_TYPE_INDEX)
		return;
	if (stock_present(board->stocks, stock, board->nr_stocks)) {
		board->dirty = 1;
		return;
	}
	if (insert_double_gainer(stock, board)) {
		sort_double_gainers(board);
		board->dirty = 1;
	}
}
int insert_double_gainer(struct stock *stock, struct board *board) {
	struct stock *sp, **bstocks;
	double key;
	int x, nstocks, offset;

	nstocks = board->nr_stocks;
	bstocks = board->stocks;
	offset  = board->offset;
	key     = *(double *)((char *)stock+offset);
	for (x=0; x<nstocks; x++) {
		sp = bstocks[x];
		if (key > *(double *)((char *)sp+offset)) {
			memmove(&bstocks[x+1], &bstocks[x], (nstocks-x)*8);
			bstocks[x]  = stock;
			return 1;
		}
	}
	return 0;
}
void sort_double_gainers(struct board *board)
{
	struct stock *stock, **bstocks;
	int i, j, nstocks, offset;
	double key;

	nstocks   = board->nr_stocks;
	bstocks   = board->stocks;
	offset    = board->offset;
	for (i=1; i<nstocks; i++) {
		stock = bstocks[i];
		key   = *(double *)((char *)stock+offset);
		j     = i;
		while (j > 0 && *(double *)((char *)bstocks[j-1]+offset) < key) {
			bstocks[j]     = bstocks[j-1];
			j              = j-1;
			board->dirty = 1;
		}
		bstocks[j] = stock;
	}
}

int insert_double_loser(struct stock *stock, struct board *board) {
	struct stock *sp, **bstocks;
	double key;
	int x, nstocks, offset;

	nstocks = board->nr_stocks;
	bstocks = board->stocks;
	offset  = board->offset;
	key     = *(double *)((char *)stock+offset);
	for (x=0; x<nstocks; x++) {
		sp = bstocks[x];
		if (key < *(double *)((char *)sp+offset)) {
			memmove(&bstocks[x+1], &bstocks[x], (nstocks-x)*8);
			bstocks[x]  = stock;
			return 1;
		}
	}
	return 0;
}

void sort_double_losers(struct board *board)
{
	struct stock *stock, **bstocks;
	int i, j, nstocks, offset;
	double key;

	nstocks   = board->nr_stocks;
	bstocks   = board->stocks;
	offset    = board->offset;
	for (i=1; i<nstocks; i++) {
		stock = bstocks[i];
		key   = *(double *)((char *)stock+offset);
		j     = i;
		while (j > 0 && *(double *)((char *)bstocks[j-1]+offset) > key) {
			bstocks[j]     = bstocks[j-1];
			j              = j-1;
			board->dirty = 1;
		}
		bstocks[j] = stock;
	}
}

void add_double_loser(struct stock *stock, struct board *board)
{
	if (stock->type == STOCK_TYPE_INDEX)
		return;
	if (stock_present(board->stocks, stock, board->nr_stocks)) {
		board->dirty = 1;
		return;
	}
	if (insert_double_loser(stock, board)) {
		sort_double_losers(board);
		board->dirty = 1;
	}
}


/* **********************************
 *
 * MAIN PAGE GAINER/LOSER TABLE SORT
 *
 ***********************************/
int insert_delta_gainer(struct stock *stock, struct board *board) {
	struct stock *sp, **bstocks;
	double key;
	int x, nstocks;

	nstocks = board->nr_stocks;
	bstocks = board->stocks;
	key     = stock->pr_percent;
//	if (!strncmp(board->table, "table HG", 8) && key > 2.0)
//		printf(BOLDGREEN "board: %s stock: %s nstocks: %d key: %.2f" RESET "\n", board->table, stock->sym, nstocks, key);
	for (x=0; x<nstocks; x++) {
		sp = bstocks[x];
		if (key > sp->pr_percent) {
			if (stock->thread_id == Server.DEBUG_THREAD && !strncmp(board->table, "table HG", 8))
				printf(BOLDGREEN "sp: %s pr: %.2f vs key: %.2f" RESET "\n", sp->sym, sp->pr_percent, key);
			memmove(&bstocks[x+1], &bstocks[x], (nstocks-x)*8);
			bstocks[x]  = stock;
			return 1;
		}
	}
	return 0;
}
int insert_delta_loser(struct stock *stock, struct board *board) {
	struct stock *sp, **bstocks;
	double key;
	int x, nstocks;

	nstocks = board->nr_stocks;
	bstocks = board->stocks;
	key     = stock->pr_percent;
//	if (!strncmp(board->table, "table HL", 8) && key > 2.0)
//		printf(BOLDRED "board: %s stock: %s nstocks: %d key: %.2f" RESET "\n", board->table, stock->sym, nstocks, key);
	for (x=0; x<nstocks; x++) {
		sp = bstocks[x];
		if (key < sp->pr_percent) {
//		if (!strncmp(board->table, "table HL", 8) && key > 2.0)
//			printf(BOLDRED "sp: %s pr: %.2f vs key: %.2f" RESET "\n", sp->sym, sp->pr_percent, key);
			memmove(&bstocks[x+1], &bstocks[x], (nstocks-x)*8);
			bstocks[x]  = stock;
			return 1;
		}
	}
	return 0;
}
void sort_delta_gainers(struct board *board)
{
	struct stock *stock, **bstocks;
	int i, j, nstocks;
	double key;

	nstocks   = board->nr_stocks;
	bstocks   = board->stocks;
	for (i=1; i<nstocks; i++) {
		stock = bstocks[i];
		key   = stock->pr_percent;
		j     = i;
		while (j > 0 && bstocks[j-1]->pr_percent < key) {
			bstocks[j]     = bstocks[j-1];
			j              = j-1;
			board->dirty = 1;
		}
		bstocks[j] = stock;
	}
}
void sort_delta_losers(struct board *board)
{
	struct stock *stock, **bstocks;
	int i, j, nstocks;
	double key;

	nstocks   = board->nr_stocks;
	bstocks   = board->stocks;
	for (i=1; i<nstocks; i++) {
		stock = bstocks[i];
		key   = stock->pr_percent;
		j     = i;
		while (j > 0 && bstocks[j-1]->pr_percent > key) {
			bstocks[j]     = bstocks[j-1];
			j              = j-1;
			board->dirty = 1;
		}
		bstocks[j] = stock;
	}
}
void add_delta_loser(struct stock *stock, struct board *board)
{
	if (stock->type == STOCK_TYPE_INDEX)
		return;
	if (stock_present(board->stocks, stock, board->nr_stocks)) {
		board->dirty = 1;
		return;
	}
	if (insert_delta_loser(stock, board)) {
//		if (!strncmp(board->table, "table HL", 8))
	//		printf("got highcaps low %s stock: %s board's nr_stocks: %d curprice: %.2f\n", board->table, stock->sym, board->nr_stocks, stock->current_price);
		sort_delta_losers(board);
		board->dirty = 1;
	}
}
void add_delta_gainer(struct stock *stock, struct board *board)
{
	if (stock->type == STOCK_TYPE_INDEX)
		return;
	if (stock_present(board->stocks, stock, board->nr_stocks)) {
		board->dirty = 1;
		return;
	}
	if (insert_delta_gainer(stock, board)) {
		if (stock->thread_id == Server.DEBUG_THREAD && !strncmp(board->table, "table HG", 8))
			printf("got highcaps high %s stock: %s board's nr_stocks: %d curprice: %.2f\n", board->table, stock->sym, board->nr_stocks, stock->current_price);
		sort_delta_gainers(board);
		board->dirty = 1;
	}
}

/* Live Table Events */
struct btab {
	char          *table;
	char          *table_id;
	uint64_t       btype;
	void         (*update)(struct board *board);
	void         (*add)(struct stock *stock, struct board *board);
	int            nr_stocks;
	int            max_stocks;
	int            bufsize;
	uint64_t       offset;
	int            buffer;
};

struct btab board_table[] = {
		/* Gainer/Loser Tables */
		{ "table LG ",    "LG",    BLOWCAPS_DELTA |BTYPE_LOWCAPS,  update_lowcaps,         add_delta_gainer,  NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, 0 },
		{ "table LL ",    "LL",    BLOWCAPS_DELTA |BTYPE_LOWCAPS,  update_lowcaps,         add_delta_loser,   NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, 0 },
		{ "table HG ",    "HG",    BHIGHCAPS_DELTA|BTYPE_HIGHCAPS, update_highcaps,        add_delta_gainer,  NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, 0 },
		{ "table HL ",    "HL",    BHIGHCAPS_DELTA|BTYPE_HIGHCAPS, update_highcaps,        add_delta_loser,   NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, 0 },
		/* UFO Price Lowcaps */
		{ "table R15 ",   "R15",   BLOWCAPS_PRICE|BTYPE_LOWCAPS,   update_lowcaps_double,  add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_15m)  },
		{ "table R5 ",    "R5",    BLOWCAPS_PRICE|BTYPE_LOWCAPS,   update_lowcaps_double,  add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_5m)   },
		{ "table R1 ",    "R1",    BLOWCAPS_PRICE|BTYPE_LOWCAPS,   update_lowcaps_double,  add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_1min) },
		{ "table R15mL ", "R15mL", BLOWCAPS_PRICE|BTYPE_LOWCAPS,   update_lowcaps_double,  add_double_loser,  NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_15m)  },
		{ "table R5mL ",  "R5mL",  BLOWCAPS_PRICE|BTYPE_LOWCAPS,   update_lowcaps_double,  add_double_loser,  NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_5m)   },
		{ "table R1mL ",  "R1mL",  BLOWCAPS_PRICE|BTYPE_LOWCAPS,   update_lowcaps_double,  add_double_loser,  NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_1min) },
		/* UFO Price Highcaps */
		{ "table R15 ",   "R15",   BHIGHCAPS_PRICE|BTYPE_HIGHCAPS, update_highcaps_double, add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_15m)  },
		{ "table R5 ",    "R5",    BHIGHCAPS_PRICE|BTYPE_HIGHCAPS, update_highcaps_double, add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_5m)   },
		{ "table R1 ",    "R1",    BHIGHCAPS_PRICE|BTYPE_HIGHCAPS, update_highcaps_double, add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_1min) },
		{ "table R15mL ", "R15mL", BHIGHCAPS_PRICE|BTYPE_HIGHCAPS, update_highcaps_double, add_double_loser,  NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_15m)  },
		{ "table R5mL ",  "R5mL",  BHIGHCAPS_PRICE|BTYPE_HIGHCAPS, update_highcaps_double, add_double_loser,  NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_5m)   },
		{ "table R1mL ",  "R1mL",  BHIGHCAPS_PRICE|BTYPE_HIGHCAPS, update_highcaps_double, add_double_loser,  NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, price_1min) },
		/* UFO Volume Lowcaps */
		{ "table V15 ",   "V15",   BLOWCAPS_VOL|BTYPE_LOWCAPS,     update_lowcaps_volume,  add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, volume_15m) },
		{ "table V5 ",    "V5",    BLOWCAPS_VOL|BTYPE_LOWCAPS,     update_lowcaps_volume,  add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, volume_5m)  },
		{ "table V1 ",    "V1",    BLOWCAPS_VOL|BTYPE_LOWCAPS,     update_lowcaps_volume,  add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, volume_1m)  },
		/* UFO Volume Highcaps */
		{ "table V15H ",  "V15H",  BLOWCAPS_VOL|BTYPE_HIGHCAPS,    update_highcaps_volume, add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, volume_15m) },
		{ "table V5H ",   "V5H",   BLOWCAPS_VOL|BTYPE_HIGHCAPS,    update_highcaps_volume, add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, volume_5m)  },
		{ "table V1H ",   "V1H",   BLOWCAPS_VOL|BTYPE_HIGHCAPS,    update_highcaps_volume, add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, volume_1m)  },
		/* IndiBoard */
		{ "table bulls ", "CBULL", BULLCAPS,                       update_indicators,      add_double_gainer, NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, nr_bullish_indicators) },
		{ "table bears ", "CBEAR", BEARCAPS,                       update_indicators,      add_double_loser,  NR_DEF_STOCKS, MAX_DELTA_STOCKS, 4 KB, OFF(stock, nr_bearish_indicators) }
};

int ufo_stock_tables(char *packet, int table_type)
{
	struct board *board;
	struct XLS   *XLS = CURRENT_XLS;
	char         *json;
	int           json_size, packet_len = 0, x;

	board       = XLS->boards[table_type];
	strcpy(packet+packet_len, board->table);
	packet_len += board->tsize;
	json        = json_board(board, &json_size);
	memcpy(packet+packet_len, json, json_size);
	packet_len += json_size;
	*(packet+packet_len++) = '@';
	packet[packet_len] = 0;
	return (packet_len);
}

int ufo_table_json(char *table, char *packet)
{
	for (int x=0; x<sizeof(board_table)/sizeof(struct btab); x++) {
		if (!strcmp(board_table[x].table_id, table))
			return ufo_stock_tables(packet, x);
	}
	return 0;
}

void init_ufo(struct XLS *XLS)
{
	struct stock **bstocks, *stock, **stocks;
	struct board *board, **boards;
	int x, y, bufsize, nr_stocks;

	stocks = XLS->LOWCAPS;
	XLS->boards = boards = (struct board **)zmalloc(sizeof(struct board *) * NR_BOARDS);
	for (x=0; x<sizeof(board_table)/sizeof(struct btab); x++) {
		board             = (struct board *)zmalloc(sizeof(*board));
		bstocks           = (struct stock **)malloc((MAX_DELTA_STOCKS+1) * sizeof(struct stock *));
		memcpy(bstocks, stocks,    (MAX_DELTA_STOCKS+1) * sizeof(struct stock *));
		board->stocks     = bstocks;
		board->update     = board_table[x].update;
		board->add        = board_table[x].add;
		board->nr_stocks  = board_table[x].nr_stocks;
		board->max_stocks = board_table[x].max_stocks;
		board->buf1       = (char *)malloc(board_table[x].bufsize);
		board->buf2       = (char *)malloc(board_table[x].bufsize);
		board->offset     = board_table[x].offset;
		board->table      = board_table[x].table;
		board->buffer     = 1;
		board->tsize      = strlen(board_table[x].table);
		boards[x]         = board;
		if (holiday || market == NO_MARKET || board_table[x].btype == BULLCAPS || board_table[x].btype == BEARCAPS)
			board->dirty = 1;
	}

	stocks    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = stocks[x];
		for (y=0; y<NR_BOARDS; y++) {
			if (stock->type == STOCK_TYPE_LOWCAPS         && board_table[y].btype & BTYPE_LOWCAPS) {
				boards[y]->add(stock, boards[y]);
			} else if (stock->type == STOCK_TYPE_HIGHCAPS && board_table[y].btype & BTYPE_HIGHCAPS) {
				boards[y]->add(stock, boards[y]);
			} else if (board_table[y].btype == BULLCAPS   || board_table[y].btype == BEARCAPS) {
				boards[y]->add(stock, boards[y]);
			}
		}
	}

	/* The market may or may not be open when ufo_scan() is run:
	 *   - if the market is closed then ufo_scan() fills the tables based on yesterday's (EOD) values
	 *   - if the market is open then ufo_scan() fills the tables based on the current prices and volumes
	 */
	ufo_scan(XLS);
}
