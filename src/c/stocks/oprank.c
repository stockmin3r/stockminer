#include <conf.h>
#include <stocks/options.h>

struct opchain **ranked_opchains;
struct opstock **ranked_contracts;
int nr_ranked_opchains;
int nr_ranked_contracts;
int NR_OPTAB = 33;

char *toprank[] = { "SPY", "TSLA", "NOK", "NVDA", "AAPL", "AMD", "AAL", "AMZN", "MSFT", "BA", "BABA", "BAC", "BB", "CSCO", "GE", "GM", "F", "DKNG", "QCOM", "PTON", "MRNA", "XLF" };
#define NR_TOP_STOCKS sizeof(toprank)/sizeof(char *)

void oprank_add_opchain(struct opchain *opchain)
{
	int x, found = 0;

	if (nr_ranked_opchains > MAX_OPCHAINS)
		return;
	for (x=0; x<NR_TOP_STOCKS; x++) {
		if (!strcmp(opchain->ticker, toprank[x])) {
			found = 1;
			break;
		}
	}
	if (!found)
		return;
	if (!ranked_opchains)
		ranked_opchains = (struct opchain **)malloc(256 * sizeof(struct opchain *));
	ranked_opchains[nr_ranked_opchains++] = opchain;
}

void oprank_add_contract(struct opstock *opstock)
{
	if (nr_ranked_contracts > MAX_CONTRACTS)
		return;
	if (!ranked_contracts)
		ranked_contracts = (struct opstock **)malloc(256 * sizeof(struct opstock *));
	ranked_contracts[nr_ranked_contracts++] = opstock;
}

void load_ranks()
{
	struct stock  *stock;
	struct oprank *rank;
	char           path[256];
	char           buf[64 KB];
	char          *opchain_array[64];
	char          *contract_array[8192];
	char          *ticker, *op, *contract, *p, *p2;
	int            nr_opchains = 0, nr_contracts = 0, x;

	RANKED_OPCHAINS  = (struct oprank **)malloc(sizeof(struct oprank *) * 4096);
	RANKED_CONTRACTS = (struct oprank **)malloc(sizeof(struct oprank *) * 8192);

	if (!fs_readfile_str((char *)"RANKED_OPCHAINS.TXT", buf, sizeof(buf)))
		return;
	ticker = buf;
	while ((p=strchr(ticker, '\n'))) {
		*p++  = 0;
		rank  = (struct oprank *)malloc(sizeof(*rank));
		op    = strchr(ticker, ' ');
		*op++ = 0;
		stock = search_stocks(ticker);

		while ((p2=strchr(op, ' '))) {
			*p2++ = 0;
			opchain_array[nr_opchains++] = strdup(op);
			op = p2 + 1;
		}
		rank->opchains    = (struct opchain **)malloc(sizeof(struct opchain *) * (nr_opchains+32));
		rank->nr_opchains = nr_opchains;
		for (x=0; x<nr_opchains; x++)
			rank->opchains[x] = search_opchain(stock->options, opchain_array[x]);
		ticker = p + 1;
		RANKED_OPCHAINS[nr_ranked_opchains++] = rank;
	}

	if (!fs_readfile_str((char *)"RANKED_CONTRACTS.TXT", buf, sizeof(buf)))
		return;
	contract = buf;
	while ((p=strchr(contract, '\n'))) {
		*p++  = 0;
		rank  = (struct oprank *)malloc(sizeof(*rank));
		op    = strchr(ticker, ' ');
		*op++ = 0;
		stock = search_stocks(ticker);

		while ((p2=strchr(op, ' '))) {
			*p2++ = 0;
			contract_array[nr_contracts++] = strdup(op);
			op = p2 + 1;
		}
		rank->contracts    = (struct opstock **)malloc(sizeof(struct opstock *) * (nr_contracts+2048));
		rank->nr_contracts = nr_contracts;
		for (x=0; x<nr_opchains; x++)
			rank->contracts[x] = search_contract(stock->options, contract_array[x]);
		ticker = p + 1;
		RANKED_CONTRACTS[nr_ranked_contracts++] = rank;
	}
}

void sort_options_oi_calls(struct opstock *opstock)
{
	struct opstock *op;
	int x;

	if (nr_options_oi_calls < NR_OPTAB) {
		options_oi_calls[nr_options_oi_calls++] = opstock;
		return;
	}
	for (x=0; x<NR_OPTAB; x++) {
		op = options_oi_calls[x];
		if (opstock->option->openInterest > op->option->openInterest) {
			memmove(&options_oi_calls[x+1], &options_oi_calls[x], (NR_OPTAB-x)*8);
			options_oi_calls[x]  = opstock;
			break;
		}
	}
}

void sort_options_oi_puts(struct opstock *opstock)
{
	struct opstock *op;
	int x;

	if (nr_options_oi_puts < NR_OPTAB) {
		options_oi_puts[nr_options_oi_puts++] = opstock;
		return;
	}
	for (x=0; x<NR_OPTAB; x++) {
		op = options_oi_puts[x];
		if (opstock->option->openInterest > op->option->openInterest) {
			memmove(&options_oi_puts[x+1], &options_oi_puts[x], (NR_OPTAB-x)*8);
			options_oi_puts[x]  = opstock;
			break;
		}
	}
}


void sort_options_vol_calls(struct opstock *opstock)
{
	struct opstock *op;
	int x;

	if (nr_options_vol_calls < NR_OPTAB) {
		options_vol_calls[nr_options_vol_calls++] = opstock;
		return;
	}
	for (x=0; x<NR_OPTAB; x++) {
		op = options_vol_calls[x];
		if (opstock->option->volume > op->option->volume) {
			memmove(&options_vol_calls[x+1], &options_vol_calls[x], (NR_OPTAB-x)*8);
			options_vol_calls[x] = opstock;
			break;
		}
	}
}

void sort_options_vol_puts(struct opstock *opstock)
{
	struct opstock *op;
	int x;

	if (nr_options_vol_puts < NR_OPTAB) {
		options_vol_puts[nr_options_vol_puts++] = opstock;
		return;
	}
	for (x=0; x<NR_OPTAB; x++) {
		op = options_vol_puts[x];
		if (opstock->option->volume > op->option->volume) {
			memmove(&options_vol_puts[x+1], &options_vol_puts[x], (NR_OPTAB-x)*8);
			options_vol_puts[x] = opstock;
			break;
		}
	}
}


void sort_lastTraded(struct opstock *opstock, int rank)
{
	struct opstock *op;
	int x;

	if (nr_lastTraded < 1023) {
		options_trade_board[nr_lastTraded++] = opstock;
		return;
	}
	for (x=0; x<1023; x++) {
		op = options_trade_board[x];
		if (rank == RANK_LAST_TRADED_LOW) {
			if (opstock->option->lastTrade > op->option->lastTrade)
				continue;
		} else if (rank == RANK_LAST_TRADED_HIGH) {
			if (opstock->option->lastTrade < op->option->lastTrade)
				continue;
		}
//		printf("moving: %s to %s's position and adding: %s\n", options_trade_board[x]->stock->sym,options_trade_board[x+1]->stock->sym, opstock->stock->sym);
		memmove(&options_trade_board[x+1], &options_trade_board[x], (1023-x)*8);
		options_trade_board[x]  = opstock;
		break;
	}
}


int RankOption(char *ticker, int rank_type, void *rank, struct Option **option)
{
	struct Option  *opt, *cop, *pop;
	struct opchain *opchain;
	struct stock   *stock;
	char            expbuf[32];
	int             y, xp_index, nr_days, nr_expiry, nr_call_options, nr_put_options;
	unsigned long   lastTradedHigh        = 0;
	unsigned long   lastTradedLow         = -1;
	double          impliedVolatilityHigh = 0.0;
	int             openInterestHigh      = 0;

	stock = search_stocks(ticker);
	if (!load_opchains(stock))
		return 0;
	opchain   = stock->options;
	nr_expiry = opchain->nr_expiry;

	for (xp_index=0; xp_index<nr_expiry; xp_index++) {
		nr_days = opchain[xp_index].nr_days-1;
		cop = opchain[xp_index].call_options[nr_days];
		pop = opchain[xp_index].put_options[nr_days];
		nr_call_options = opchain[xp_index].nr_calls;
		nr_put_options  = opchain[xp_index].nr_puts;
		for (y=0; y<nr_call_options; y++) {
			switch (rank_type) {
				case RANK_OPEN_INTEREST:
					if (cop->openInterest > openInterestHigh) {
						openInterestHigh = cop->openInterest;
						*option = cop;
					}
					cop++;
					break;
				case RANK_LAST_TRADED_HIGH:
					if (cop->lastTrade && cop->lastTrade > lastTradedHigh) {
						lastTradedHigh = cop->lastTrade;
						*option = cop;
//						printf("call: checking expiry: %s lastTraded: %lu\n", opchain[xp_index].expiry_date, cop->lastTrade);
					}
					cop++;
					break;
				case RANK_LAST_TRADED_LOW:
					if (cop->lastTrade && cop->lastTrade < 1609872723UL)
						nr_lowInterest++;
					if (cop->lastTrade && cop->lastTrade < lastTradedLow) {
						lastTradedLow = cop->lastTrade;
						*option = cop;
//						printf("call: checking expiry: %s lastTraded: %lu strike: %.2f\n", unix2str(expiry[xp_index], expbuf), cop->lastTrade, cop->strike);
					}
					cop++;
					break;
			}
		}
		for (y=0; y<nr_put_options; y++) {
			switch (rank_type) {
				case RANK_OPEN_INTEREST:
					if (pop->openInterest > openInterestHigh) {
						openInterestHigh = pop->openInterest;
						*option = pop;
					}
					pop++;
					break;
				case RANK_LAST_TRADED_HIGH:
					if (pop->lastTrade && pop->lastTrade > lastTradedHigh) {
						lastTradedHigh = pop->lastTrade;
						*option = pop;
//						printf("put: checking expiry: %s lastTraded: %lu strike: %.2f\n", unix2str(expiry[xp_index], expbuf), pop->lastTrade, pop->strike);
					}
					pop++;
					break;
				case RANK_LAST_TRADED_LOW:
					if (pop->lastTrade && pop->lastTrade < 1592405589UL)
						nr_lowInterest++;
					if (pop->lastTrade && pop->lastTrade < lastTradedLow) {
						lastTradedLow = pop->lastTrade;
						*option = pop;
//						printf("put: checking expiry: %s lastTraded: %lu strike: %.2f\n", unix2str(expiry[xp_index], expbuf), pop->lastTrade, pop->strike);
					}
					pop++;
					break;
			}
		}
	}
	switch (rank_type) {
		case RANK_OPEN_INTEREST:
			*(int *)rank = openInterestHigh;
			break;
		case RANK_LAST_TRADED:
			*(unsigned long *)rank = lastTradedHigh;
			break;
		case RANK_LAST_TRADED_LOW:
			*(unsigned long *)rank = lastTradedLow;
			break;
		case RANK_LAST_TRADED_HIGH:
			*(unsigned long *)rank = lastTradedHigh;
			break;
	}
	return 1;
}

void RankOpenInterest(char *ticker)
{
	char *stock;
	struct Option *option;
	int x, openInterestHigh = 0;
	int high;

	for (x=0; x<nr_highcap_options; x++) {
		stock = HIGHCAP_OPTIONS[x];
		if (ticker && strcmp(stock, ticker))
			continue;
		if (!RankOption(stock, RANK_OPEN_INTEREST, (int *)&high, &option))
			continue;
		if (high > openInterestHigh) {
			printf("%s overtake (%d vs %d)\n", stock, high, openInterestHigh);
			openInterestHigh = high;
		}
	}
}

void RankLastTradedLow(char *ticker)
{
	struct opstock *opstock;
	struct Option  *option;
	char           *stock;
	char            expbuf[32];
	char            expbuf2[32];
	int             x;
	uint64_t       low, lastTradeLow = -1;

	for (x=0; x<nr_highcap_options; x++) {
		stock = HIGHCAP_OPTIONS[x];
		if (ticker && strcmp(stock, ticker))
			continue;
		if (!RankOption(stock, RANK_LAST_TRADED_LOW, (uint64_t *)&low, &option))
			continue;
		if (low < lastTradeLow) {
			printf("%s (%s) overtake (%s vs %s)\n", stock, option->contract, unix2str(low, expbuf), unix2str(lastTradeLow, expbuf2));
			lastTradeLow = low;
		}
		opstock         = (struct opstock *)malloc(sizeof(*opstock));
		opstock->stock  = search_stocks(stock);
		opstock->option = option;
		sort_lastTraded(opstock, RANK_LAST_TRADED_LOW);
	}
	for (x=0; x<nr_lastTraded; x++) {
		opstock = options_trade_board[x];
		printf("[%s] [%s] %s\n", opstock->stock->sym, opstock->option->contract, unix2str(opstock->option->lastTrade, expbuf));
	}
	printf("nr_lowInterest: %d\n", nr_lowInterest);
}

void RankLastTradedHigh(char *ticker)
{
	struct opstock *opstock;
	struct Option  *option;
	char            expbuf[32];
	char            expbuf2[32];
	char           *stock;
	int             x;
	unsigned long   high, lastTradeHigh = 0;

	for (x=0; x<nr_highcap_options; x++) {
		stock = HIGHCAP_OPTIONS[x];
		if (ticker && strcmp(stock, ticker))
			continue;
		if (!RankOption(stock, RANK_LAST_TRADED_HIGH, (unsigned long *)&high, &option))
			continue;
		if (high > lastTradeHigh) {
			printf("%s overtake (%s vs %s)\n", stock, unix2str(high, expbuf), unix2str2(lastTradeHigh, expbuf2));
			lastTradeHigh = high;
		}
		opstock         = (struct opstock *)malloc(sizeof(*opstock));
		opstock->stock  = search_stocks(stock);
		opstock->option = option;
		sort_lastTraded(opstock, RANK_LAST_TRADED_HIGH);
	}
	for (x=0; x<nr_lastTraded; x++) {
		opstock = options_trade_board[x];
		printf("[%-5s] [%-24s] %s\n", opstock->stock->sym, opstock->option->contract, unix2str2(opstock->option->lastTrade, expbuf));
	}
}
