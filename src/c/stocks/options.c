#include <conf.h>
#include <stocks/api.h>
#include <stocks/options.h>

char **HIGHCAP_OPTIONS;
char **LOWCAP_OPTIONS;
char **OPTIONS;

struct stock  **opstocks;
struct opstock *options_oi_calls[35];
struct opstock *options_vol_calls[35];
struct opstock *options_oi_puts[35];
struct opstock *options_vol_puts[35];
struct opstock *options_trade_board[1024];
int nr_options_oi_calls;
int nr_options_vol_calls;
int nr_options_oi_puts;
int nr_options_vol_puts;
int nr_lastTraded;
int nr_opstocks;
int nr_global_puts;
int nr_global_calls;
int nr_lowInterest;
int nr_highcap_options;
int nr_lowcap_options;
int NR_OPTIONS;
struct oprank **RANKED_OPCHAINS;
struct oprank **RANKED_CONTRACTS;
int nr_opchain_stocks;
int nr_contract_stocks;

void option_probe(struct stock *stock, int caps)
{
	char url[512];
	char page[3072 KB];
	char buf[256];
	char *path;
	int nbytes, fd;

	sprintf(url, YAHOO_OPTION_CHECK, stock->sym);
	if (!curl_get(url, page) || strstr(page, "data is not available<")) {
		path = "data/stocks/OPTIONS_IGNORE.TXT";
		fd = open(path, O_RDWR|O_CREAT|O_APPEND, 0644);
		if (fd < 0)
			return;
		nbytes = snprintf(buf, sizeof(buf)-1, "%s\n", stock->sym);
		write(fd, buf, nbytes);
		close(fd);
		printf("No options: %s\n", stock->sym);
		return;
	}
	if (caps == LOWCAPS_TABLE)
		path = "data/stocks/OPTIONS_LOWCAPS.TXT";
	else
		path = "data/stocks/OPTIONS_HIGHCAPS.TXT";
	printf("STOCK HAS OPTIONS: %s %s\n", stock->sym, caps == LOWCAPS_TABLE ? "LOWCAPS" : "HIGHCAPS");
	fd = open(path, O_RDWR|O_CREAT|O_APPEND, 0644);
	nbytes = sprintf(buf, "%s\n", stock->sym);
	write(fd, buf, nbytes);
	close(fd);
}

void options_check(struct XLS *XLS)
{
	struct stock  *stock;
	struct stock **LOWCAPS;
	struct stock **HIGHCAPS;
	char           ticker[32];
	int            x, table = -1, nr_lowcaps, nr_highcaps, fd;

	LOWCAPS     = XLS->LOWCAPS;
	HIGHCAPS    = XLS->HIGHCAPS;
	nr_lowcaps  = XLS->nr_lowcaps;
	nr_highcaps = XLS->nr_highcaps;

	fd = open("data/stocks/OPTIONS_IGNORE.TXT", O_RDONLY);
	if (fd < 0)
		return;
	while (fs_readline(ticker, fd) != -1) {
		for (x=0; x<nr_lowcaps; x++) {
			stock = LOWCAPS[x];
			if (!strcmp(stock->sym, ticker)) {
				table = LOWCAPS_TABLE;
				break;
			}
		}
		if (table == -1) {
			for (x=0; x<nr_highcaps; x++) {
				stock = HIGHCAPS[x];
				if (!strcmp(stock->sym, ticker)) {
					table = HIGHCAPS_TABLE;
					break;
				}
			}
		}
		option_probe(stock, table);
		table = -1;
	}
}


void *options_thread(void *args)
{
	struct Option  *cop, *pop;
	struct opstock *opstock;
	struct opchain *opchain;
	struct stock   *stock;
	struct update   update;
	char            page[1024 KB];
	char            path[256];
	int             nr_expiry, nr_days, nr_calls, nr_puts, x, y, z;

	opstocks = (struct stock **)malloc(sizeof(struct stock *) * 10);
	while (1) {
		for (x=0; x<nr_opstocks; x++) {
			stock     = opstocks[x];
			opchain   = stock->options;
			nr_expiry = opchain->nr_expiry;
			nr_days   = opchain->nr_days-1;
			for (y=0; y<nr_expiry; y++) {
				if (opchain[y].expired || opchain[y].corrupt)
					continue;
				nr_calls = opchain[y].nr_calls;
				nr_puts  = opchain[y].nr_puts;
				cop      = opchain[y].call_options[nr_days];
				pop      = opchain[y].put_options[nr_days];

				if (!fetch_options(stock->sym, nr_calls ? cop->expiry : pop->expiry , page)) {
					opchain[y].nr_failed += 1;
					continue;
				}

				update.ticker  = stock->sym;
				update.page    = page;
				update.cmd     = OPCHAIN_LIVE;
				for (z=0; z<nr_calls; z++) {
					snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s", cop->contract);
					update.expiry_path = path;
					extract_options(&update);
					cop++;
				}
				for (z=0; z<nr_puts; z++) {
					snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s", pop->contract);
					update.expiry_path = path;
					extract_options(&update);
					pop++;
				}
			}
		}
		os_sleep(15*60);
	}
}

void update_option_math(struct stock *stock, struct Option *option, struct opmath *opmath)
{
	double StockPrice = stock->current_price;
	int option_holding_period = 30;

	opmath->NetDebit        = StockPrice-option->bid;
	opmath->PctNetDebit     = (opmath->NetDebit/StockPrice-1)*100.0;
	opmath->ReturnCalled    = (option->strike/opmath->NetDebit-1)*100.0;
	opmath->APRCalled       = opmath->ReturnCalled/option_holding_period*365.0;
	opmath->ReturnUnchanged = (option->bid/opmath->NetDebit)*100.0;
	opmath->APRUnchanged    = opmath->ReturnUnchanged/option_holding_period*365.0;
	opmath->PctOTM          = (option->strike/StockPrice-1)*100.0;
}

int option_search(struct Option *op, int nr_op, char *contract)
{
	int x;
	for (x=0; x<nr_op; x++) {
		printf(BOLDYELLOW "search_contract: %s vs (requested:) %s" RESET "\n", op->contract, contract);
		if (!strcmp(contract, op->contract))
			return (x);
		op++;
	}
	return (-1);
}

struct opstock *search_contract(struct opchain *opchain, char *contract)
{
	struct Option  *op;
	struct stock   *stock;
	char           *p = contract;
	char          **names;
	char            ticker[8];
	char            expiry[12];
	char            optype;
	int             x = 0, nr_op, nr_days, nr_expiry, found = 0, count = 0, idx = -1;

	// eg: AMD230120P00060000
	expiry[0] = '2';
	expiry[1] = '0';
	while (!isdigit(*p))
		ticker[count++] = *p++;
	ticker[count] = 0;
	expiry[2] = *p;
	expiry[3] = *(p+1);
	expiry[4] = '-';
	expiry[5] = *(p+2);
	expiry[6] = *(p+3);
	expiry[7] = '-';
	expiry[8] = *(p+4);
	expiry[9] = *(p+5);

	while (*p != 'C' && *p != 'P')
		p++;
	if (*p == 'C')
		optype = OPTION_CALL;
	else if (*p == 'P')
		optype = OPTION_PUT;
	else {
		printf("search_contract error: %s\n", contract);
		return (NULL);
	}

	if (!opchain) {
		stock = search_stocks(ticker);
		if (!stock)
			return NULL;
		opchain   = stock->options;
		if (!opchain)
			return NULL;
	}
	nr_expiry = opchain->nr_expiry;
	nr_days   = opchain->nr_days-1;
	for (x=0; x<nr_expiry; x++) {
		if (strcmp(opchain[x].expiry_date, expiry))
			continue;
		if (optype == OPTION_CALL) {
			nr_op  = opchain[x].nr_calls;
			op     = opchain[x].call_options[nr_days];
			names  = opchain[x].cnames;
		} else {
			nr_op  = opchain[x].nr_puts;
			op     = opchain[x].put_options[nr_days];
			names  = opchain[x].pnames;
		}
		idx = option_search(op, nr_op, contract);
		if (idx != -1)
			break;
	}
	if (idx != -1) {
		printf(BOLDGREEN "FOUND CONTRACT: %s opstock: %p x: %d opchain: %p optype: %c" RESET "\n", contract, optype == OPTION_CALL ? opchain[x].call_opstocks[idx] : opchain[x].put_opstocks[idx], x, &opchain[x], optype);
		return (optype == OPTION_CALL ? opchain[x].call_opstocks[idx] : opchain[x].put_opstocks[idx]);
	}
	return NULL;
}

void rpc_option_chain(struct rpc *rpc)
{
	struct session *session  = rpc->session;
	char           *packet   = rpc->packet;
	char           *contract = rpc->argv[1];
	struct opstock *opstock;
	struct opchain *opchain;
	struct stock   *stock;
	struct Option  *cop, *pop;
	int packet_len, nbytes, x, nr_op, nr_calls, nr_puts, nr_days;

	printf("option chain: %s\n", contract);
	if (!(opstock=search_contract(NULL, contract))) {
		printf("option_chain: failed to locate contract: %s\n", contract);
		return;
	}
	opchain  = opstock->opchain;
	stock    = opstock->stock;
	nr_days  = opchain->nr_days-1;
	nr_calls = opchain->nr_calls;
	nr_puts  = opchain->nr_puts;
	cop      = opchain->call_options[nr_days];
	pop      = opchain->put_options[nr_days];
	nr_op    = nr_calls >= nr_puts ? nr_calls : nr_puts;

	strcpy(packet, "optab OPCH [");
	packet_len = 12;

	for (x=0; x<nr_op; x++) {
		if (nr_calls <= 0 || (nr_puts > 0 && cop->strike > pop->strike)) {
			nbytes = sprintf(packet+packet_len, "{\"oP\":\"-\",\"oB\":\"-\",\"oA\":\"-\",\"oOI\":\"-\",\"oIV\":\"-\",\"oVOL\":\"-\","
												 "\"oS\":\"%.2f\",\"oPP\":\"%.2f\",\"oBP\":\"%.2f\",\"oAP\":\"%.2f\",\"oOIP\":\"%d\",\"oIVP\":\"%.2f\",\"oVOLP\":\"%d\"},",
			pop->strike, pop->lastPrice, pop->bid, pop->ask, pop->openInterest, pop->impliedVolatility, pop->volume);
			nr_puts  -= 1;
		} else if (nr_puts <= 0 ||(nr_calls > 0 && pop->strike > cop->strike)) {
			nbytes = sprintf(packet+packet_len, "{\"oP\":\"%.2f\",\"oB\":\"%.2f\",\"oA\":\"%.2f\",\"oOI\":\"%d\",\"oIV\":\"%.2f\",\"oVOL\":\"%d\","
												 "\"oS\":\"%.2f\",\"oPP\":\"-\",\"oBP\":\"-\",\"oAP\":\"-\",\"oOIP\":\"-\",\"oIVP\":\"-\",\"oVOLP\":\"-\"},",
			cop->lastPrice, cop->bid, cop->ask, cop->openInterest, cop->impliedVolatility, cop->volume, cop->strike);
			nr_calls -= 1;
		} else if (cop->strike == pop->strike) {
			nbytes = sprintf(packet+packet_len, "{\"oP\":\"%.2f\",\"oB\":\"%.2f\",\"oA\":\"%.2f\",\"oOI\":\"%d\",\"oIV\":\"%.2f\",\"oVOL\":\"%d\","
												 "\"oS\":\"%.2f\",\"oPP\":\"%.2f\",\"oBP\":\"%.2f\",\"oAP\":\"%.2f\",\"oOIP\":\"%d\",\"oIVP\":\"%.2f\",\"oVOLP\":\"%d\"},",
			cop->lastPrice, cop->bid, cop->ask, cop->openInterest, cop->impliedVolatility, cop->volume,
			cop->strike, pop->lastPrice, pop->bid, pop->ask, pop->openInterest, pop->impliedVolatility, pop->volume);
			nr_calls -= 1;
			nr_puts  -= 1;
		}
		printf("cop strike: %.2f pop strike: %.2f nr_calls: %d nr_puts: %d\n", cop->strike, pop->strike, nr_calls, nr_puts);
		packet_len += nbytes;
		cop++;
		pop++;
	}
	*(packet+packet_len-1) = ']';
	*(packet+packet_len)   = ' ';
	packet_len += 1;
	packet_len += sprintf(packet+packet_len, "%.2f", stock->current_price);
	printf(BOLDYELLOW "%s" RESET "\n", packet);
	websocket_send(rpc->connection, packet, packet_len);
}

void rpc_option_chart(struct rpc *rpc)
{
	char           *packet   = rpc->packet;
	char           *contract = rpc->argv[1];
	struct opstock *opstock;
	char           *cdiv;
	int             packet_len, nbytes;

	cdiv = strchr(contract, ' ');
	if (!cdiv)
		return;
	*cdiv++ = 0;
	printf("contract: %s div: %s\n", contract, cdiv);
	if (!(opstock=search_contract(NULL, contract))) {
		printf("failed opstock\n");
		return;
	}

	if (!opstock->csv) {
		printf("failed at csv\n");
		return;
	}

	nbytes = sprintf(packet, "chart %s %s %d ", opstock->option->contract, cdiv, opstock->csv_nr_points);
	memcpy(packet+nbytes, opstock->csv, opstock->csv_len);
	packet_len  = nbytes + opstock->csv_len;
	*(packet+packet_len) = 0;
	printf(BOLDGREEN "%s" RESET "\n", packet);
	websocket_send(rpc->connection, packet, packet_len);
}

void rpc_option_covered_calls(struct rpc *rpc)
{
	char           *packet   = rpc->packet;
	char           *contract = rpc->argv[1];
	struct stock   *stock;
	struct opstock *opstock;
	struct Option  *option, *cop, *pop;
	struct opchain *opchain;
	struct opmath   opmath;
	int             packet_len, x, y, nr_op, nr_days, nbytes;

	if (!(opstock=search_contract(NULL, contract))) {
		printf("failed to locate contract: %s\n", contract);
		return;
	}
	strcpy(packet, "table OPCC [");
	packet_len = 12;
	stock      = opstock->stock;
	nr_op      = opstock->nr_op;
	option     = opstock->option;
	opchain    = opstock->opchain;
	nr_days    = opchain->nr_days-1;
	cop        = opchain->call_options[nr_days];
	for (y=0; y<nr_op; y++) {
		update_option_math(stock, cop, &opmath);
		nbytes = sprintf(packet+packet_len, "{\"oP\":\"%.2f\",\"oS\":\"%.2f\",\"oB\":\"%.2f\",\"oH\":\"%d\",\"oRC\":\"%.2f\",\"oAPR\":\"%.2f\",\"oRU\":\"%.2f\",\"oARPu\":\"%.2f\",\"oOTM\":\"%.2f\",\"oND\":\"%.2f\",\"oNDP\":\"%.2f\"},",
		stock->current_price, cop->strike, cop->bid, 30, opmath.ReturnCalled, opmath.APRCalled, opmath.ReturnUnchanged, opmath.APRUnchanged, opmath.PctOTM, opmath.NetDebit, opmath.PctNetDebit);
		packet_len += nbytes;
		cop++;
	}
	*(packet+packet_len-1) = ']';
	*(packet+packet_len)   = 0;
	printf(BOLDCYAN "covered calls: %s" RESET "\n", packet);
	websocket_send(rpc->connection, packet, packet_len);
}

void rpc_option_page(struct rpc *rpc)
{
	struct session *session = rpc->session;
	char           *packet  = rpc->packet;
	struct opstock *opstock;
	struct stock   *stock;
	char           *oi_call_leaders,    *vol_call_leaders,    *oi_put_leaders,    *vol_put_leaders;
	int             oi_call_leaders_len, vol_call_leaders_len, oi_put_leaders_len, vol_put_leaders_len;
	int             nbytes, packet_len = 0;

	session->option_contract = opstock = options_oi_calls[0];
	if (!opstock)
		return;
	/* [1] Chart */
	stock = opstock->stock;
	printf(BOLDGREEN "option session: %p opstock: %p contract: %s stock: %s" RESET "\n", session, opstock, opstock->option->contract, opstock->stock->sym);
	if (opstock->csv) {
		nbytes      = sprintf(packet, "chart %s OPCHART %d 1 1 ", opstock->option->contract, opstock->csv_nr_points);
		memcpy(packet+nbytes, opstock->csv, opstock->csv_len);
		packet_len  = nbytes + opstock->csv_len;
		*(packet+packet_len) = '@';
		packet_len += 1;
	}

	/* [2] OI CALL Leaders */
	oi_call_leaders = json_option_oi_calls_leaders(&oi_call_leaders_len);
	strcpy(packet+packet_len, "table OIC ");
	packet_len += 10;
	memcpy(packet+packet_len, oi_call_leaders, oi_call_leaders_len);
	packet_len += oi_call_leaders_len;
	*(packet+packet_len) = '@';
	packet_len += 1;

	/* [3] Volume CALL Leaders */
	strcpy(packet+packet_len, "table VOLC ");
	vol_call_leaders  = json_option_vol_calls_leaders(&vol_call_leaders_len);
	packet_len += 11;
	memcpy(packet+packet_len, vol_call_leaders, vol_call_leaders_len);
	packet_len += vol_call_leaders_len;
	*(packet+packet_len) = '@';
	packet_len += 1;

	/* [2] OI PUT Leaders */
	oi_put_leaders = json_option_oi_puts_leaders(&oi_put_leaders_len);
	strcpy(packet+packet_len, "table OIP ");
	packet_len += 10;
	memcpy(packet+packet_len, oi_put_leaders, oi_put_leaders_len);
	packet_len += oi_put_leaders_len;
	*(packet+packet_len) = '@';
	packet_len += 1;

	/* [3] Volume PUT Leaders */
	strcpy(packet+packet_len, "table VOLP ");
	vol_put_leaders  = json_option_vol_puts_leaders(&vol_put_leaders_len);
	packet_len += 11;
	memcpy(packet+packet_len, vol_put_leaders, vol_put_leaders_len);
	packet_len += vol_put_leaders_len;

	/* [6] Send Websocket Packet */
	websocket_send(rpc->connection, packet, packet_len);
	*(packet+packet_len) = 0;
	printf("%s len: %d strlen: %d\n", packet, packet_len, (int)strlen(packet));
}

void cmd_options(int argc, char **argv, struct server *server)
{
	struct XLS *XLS = server->XLS;

	/* *****************
	 *
	 *   OPTION UPDATE
	 *
	 ******************/
	if (!strcmp(argv[1], "-Oupdate"))
		stock_update(argv[2], argc == 4 ? argv[3] : NULL, server);
	if (!strcmp(argv[1], "-Og"))
		stock_update_options(argv[2], argc == 4 ? argv[3] : NULL, 0);
	else if (!strcmp(argv[1], "-OH"))
		get_opchain_history(argv[2], NULL);
	else if (!strcmp(argv[1], "-OHc"))
		get_opchain_history(argv[2], argv[3]);
	else if (!strcmp(argv[1], "-OP"))
		update_options(argv[2], 0, 0);
	else if (!strcmp(argv[1], "-OP1"))
		update_options(argv[2], 1, 0);
	else if (!strcmp(argv[1], "-OPlow"))
		update_lowcap_options(argv[2], 0, 0);
	else if (!strcmp(argv[1], "-Ofix"))
		opchains_fix(argv[2]);
	else if (!strcmp(argv[1], "-Ofixoi"))
		opchains_fixoi(argv[2], argc == 4 ? argv[3] : NULL, server);
	/* *************
	 *
	 * OPTION RANK
	 *
	 **************/
	else if (!strcmp(argv[1], "-Oi"))
		RankOpenInterest(argv[2]);
	else if (!strcmp(argv[1], "-Odatelow"))
		RankLastTradedLow(argv[2]);
	else if (!strcmp(argv[1], "-Odatehigh"))
		RankLastTradedHigh(argv[2]);
	else if (!strcmp(argv[1], "-O2"))
		options_check(XLS);
	/* *************
	 *
	 * OPTION SHOW
	 *
	 **************/
	else if (!strcmp(argv[1], "-Ocorrupt"))
		show_corrupt(argv[2]);
	else if (!strcmp(argv[1], "-Odblist"))
		db_query(0);
	else if (!strcmp(argv[1], "-Ochain"))
		opchain_query(argv[2], argv[3], 0);
	else if (!strcmp(argv[1], "-OSQ"))
		stock_query(argv[2], 0);
	else if (!strcmp(argv[1], "-Os"))
		show_options(argv[2], argc == 4 ? argv[3] : NULL);
	else if (!strcmp(argv[1], "-Oexp"))
		show_expiry(argv[2], 0);
	else if (!strcmp(argv[1], "-Oexp2"))
		verify_expiry(argv[2]);
	else if (!strcmp(argv[1], "-Omissing"))
		show_missing(argv[2]);
	else if (!strcmp(argv[1], "-Otime"))
		show_chainstamp(argv[2], server);
	else if (!strcmp(argv[1], "-Ostats"))
		show_stats(argv[2], argc == 4 ? argv[3] : NULL);
	else if (!strcmp(argv[1], "-Odays"))
		show_days(argv[2], argc == 4 ? argv[3] : NULL);
	else if (!strcmp(argv[1], "-Oq")) {
		init_options(server);
		option_query(argv[2]);
	}
	else if (!strcmp(argv[1], "-OC"))
		op_query(argv[2]);
	else if (!strcmp(argv[1], "-OC1")) {
		if (argc == 2)
			op_query_ticker(argv[2]);
		else
			op_query_opchain(argv[2], argv[3]);
	}

	/* *************
	 *
	 * OPTION CSV
	 *
	 **************/
	else if (!strcmp(argv[1], "-O1d"))
		show_contract_1d(argv[2], 0);
	else if (!strcmp(argv[1], "-Olast"))
		show_contract_1d(argv[2], 1);
	else if (!strcmp(argv[1], "-Odiff"))
		show_csv_points(argv[2], argv[3]);
	else if (!strcmp(argv[1], "-O1dupdate")) {
		update_1d(argv[2], argv[3]);
	}

	/* ***************
	 *
	 * OPTION BACKUP
	 *
	 ****************/
	else if (!strcmp(argv[1], "-Obackup"))
		backup_stocks(argv[2]);
	else if (!strcmp(argv[1], "-Orestore"))
		backup_restore(argv[2], argc >= 4 ? argv[3] : NULL, argc == 5 ? argv[4] : NULL);
	exit(0);
}

