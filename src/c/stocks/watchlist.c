#include <conf.h>
#include <stocks/stocks.h>

mutex_t           watchlist_lock;
struct watchlist *Watchlists[256];
int               nr_global_watchlists;

int watchlist_stocks(struct session *session, char *packet, char *watchlist_name)
{
	struct stock      *stock;
	struct watchlist  *watchlist;
	struct watchstock *watchstock;
	char              *ticker;
	int                x, packet_len = 0;

	watchlist = search_watchlist(session, watchlist_name);
	if (!watchlist || !watchlist->nr_stocks) {
		printf("no watchlist: %p \n", watchlist);
		return 0;
	}

	for (x=0; x<watchlist->nr_stocks; x++) {
		watchstock = &watchlist->stocks[x];
		stock      = watchstock->stock;
		if (!stock)
			continue;
		ticker = stock->sym;
		packet_len += snprintf(packet+packet_len, 8, "%s-", ticker);
	}
	return (packet_len-1);
}

// RPC_stageload var stocks = "", stage = av[1], watchlist_name = av[2], QGID = av[3], stocklist = av[4].split("-");
void rpc_webscript(struct rpc *rpc)
{
	struct session    *session        = rpc->session;
	struct connection *connection     = rpc->connection;
	char              *packet         = rpc->packet;
	char              *webscript      = rpc->argv[1];
	char              *watchlist_name = rpc->argv[2];
	char              *QGID           = rpc->argv[3];

	int packet_len = snprintf(packet, 256, "stage %s %s %s ", webscript, watchlist_name, QGID);
	int nbytes     = watchlist_stocks(session, packet+packet_len, watchlist_name);
	if (!nbytes)
		return;
	packet_len += nbytes;
	websocket_send(connection, packet, packet_len);
}

void rpc_watchlist_alert(struct rpc *rpc)
{
	struct session   *session = rpc->session;
	struct stock     *stock;
	struct watchlist *watchlist;
	struct watchcond *watchcond;
	char             *argv[10];
	char             *ticker, *watchname, *condarg, *periods;
	int               cmd, condition, condclass, argc, update = 0, nr_periods = 0;
	unsigned long     id;

	/* id 0/1/2 [] WatchlistName Ticker cond condlass condarg */
	argc      = cstring_split(rpc->argv[1], argv, 10, ' ');
	id        = *(unsigned long *)rpc->argv[1];
	cmd       = *argv[1] - 48;
	watchname =  argv[2];
	ticker    =  argv[3];
	periods   =  argv[4] + 1;
	condition = *argv[5] - 48;

	if (strlen(watchname) > WATCHLIST_NAME_MAX)
		return;
	if (condition != -3) {
		condclass = *argv[6]-48;
		condarg   =  argv[7];
		if (argc >= 8 && *argv[8] == 'U')
			update    = 1;
	}
	if (!(watchlist=search_watchlist(session, watchname)) || !(stock=search_stocks(ticker)))
		return;

//	mutex_lock(&session->watchlist_lock);
	if (watchlist->nr_conditions >= MAX_CONDITIONS)
		goto ret;

	if (update && !(watchcond = session_watchcond(watchlist, id))) {
		goto ret;
	} else if (!update) {
		watchcond            = &watchlist->conditions[watchlist->nr_conditions++];
		watchcond->stock     = stock;
		watchcond->condclass = condclass;
		*(unsigned long *)(watchcond->ticker) = 0;
		*(unsigned long *)(watchcond->ticker) = *(unsigned long *)ticker;
	}
	watchcond->exec_len = snprintf(watchcond->exec, MAX_EXEC_LEN, "%s^%s^%s^%s^%s^%s^%s^%s", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
//	printf("memcpy exec len: %d strlen: %d memexec: %s\n", exec_len, (int)strlen(watchcond->exec), watchcond->exec);
	watchcond->id  = id;
	watchcond->cmd = cmd;

	while (*periods != ']' || *periods != ' ') {
		watchcond->periods[nr_periods++] = *periods++-48;
		if (nr_periods > 5)
			break;
	}

	if (condition == CONDITION_PRICE) {
		watchcond->price = strtod(condarg, NULL);
		if (strchr(condarg, '%'))
			watchcond->price = 100.0 + (100.0*(watchcond->price/100.0));
		if (stock->current_price > watchcond->price) {
			printf("GOT PRICE LESSER THAN CONDITION\n");
			watchcond->condition = PRICE_LESSER_THAN;
		} else {
			printf("PRICE GREATER THAN\n");
			watchcond->condition = PRICE_GREATER_THAN;
		}
	} else if (condition == CONDITION_VOLUME) {
		watchcond->volume  = strtoul(condarg, NULL, 10);
		if (stock->current_volume > watchcond->volume)
			watchcond->condition = VOLUME_GREATER_THAN;
		else
			watchcond->condition = VOLUME_LESSER_THAN;
	}
	if (update)
		watchcond_update(session, watchlist, watchcond);
	else {
		watchtable_add(session, watchlist, stock);
		watchcond_create(session, watchlist, watchcond);
	}
ret:
//	mutex_unlock(&session->watchlist_lock);
	return;
}

void send_watchtable(struct session *session, struct connection *connection, char *packet, struct watchlist *watchtable)
{
	int packet_len;

	packet_len = watchtable_packet(session, watchtable, packet);
	if (packet_len <= 0)
		return;
	websocket_send(connection, packet, packet_len);
	*(packet+packet_len) = 0;
	if (market == NO_MARKET)
		workspace_broadcast(session, connection, packet, packet_len);
	printf(BOLDGREEN "%s len: %d strlen: %d" RESET "\n", packet, packet_len, (int)strlen(packet));
}

struct watchlist *search_watchlist(struct session *session, char *name)
{
	int x, nr_watchlists = session->nr_watchlists;
	for (x=0; x<nr_watchlists; x++) {
		struct watchlist *watchlist = session->watchlists[x];
		if (watchlist && !strcmp(name, watchlist->name))
			return watchlist;
	}
	return (NULL);
}

struct watchlist *watchlist_add(struct session *session, struct watchlist *watchlist, struct stock *stock)
{
	struct watchlist *wp;
	struct filemap    filemap;
	char             *map;
	char              path[256];

	/* Save stock to Watchlist in memory */
	memwatch_addstock(watchlist, stock);

	/* Save Watchlist to Disk if new */
	watchlist_path(session, path);
	wp = (struct watchlist *)map_watchlist(session, path, watchlist->name, &map, &filemap);
	if (!wp)
		return NULL;
	memwatch_addstock(wp, stock);
	printf(BOLDYELLOW "watchlist_add(): Written watchlist to file: %s wp->name: %s wp->basetabe: %s ticker: %s" RESET "\n", watchlist->name, wp->name, wp->basetable, stock->sym);
	UNMAP_FILE(map, &filemap);
	return (watchlist);
}

struct watchlist *watchtable_add(struct session *session, struct watchlist *watchtable, struct stock *stock)
{
	if (session->nr_watchlists >= MAX_WATCHLISTS || watchtable->nr_stocks >= MAX_STOCKS || !stock)
		return NULL;

	/* Check if watchtable now holds a loaded Watchlist */
	if (watchtable->origin) {
		if (get_watchstock(watchtable->origin, stock))
			return NULL;
		printf(BOLDGREEN "wchtable_add() adding to ORIGIN: %s" RESET "\n", watchtable->origin->name);
		watchlist_add(session, watchtable->origin, stock);
	} else if (get_watchstock(watchtable, stock))
			return NULL;

	/* Add Stock to the WatchTable Container in Memory */
	memwatch_addstock(watchtable, stock);
	printf(BOLDGREEN "watchtable_add(): adding to watchtable: %s nr_stocks: %d stock: %s session: %p watchtable: %p" RESET "\n",watchtable->name, watchtable->nr_stocks, stock->sym, session, watchtable);
	return (watchtable);
}

/**
 ****************************************************************
 * rpc_watchtable_addStock():
 * - add stock(s) to a watchlist and/or its container watchtable
 * @watchtable_name  {char *}  watchtable HTML <table> ID
 **************************************************************/
void rpc_watchtable_addstocks(struct rpc *rpc)
{
	struct session    *session         = rpc->session;
	struct connection *connection      = rpc->connection;
	char              *packet          = rpc->packet;
	char              *watchtable_name = rpc->argv[1];
	char              *tickers         = rpc->argv[2];
	struct watchlist  *watchtable      = search_watchtable(session, watchtable_name);
	int                packet_len, nr_stocks, argc, added = 0;

	if (!watchtable || !(*tickers))
		return;

	nr_stocks = cstring_count_chars(tickers, ',');
	if (nr_stocks >= MAX_BULK_ADD)
		return;

	/* Only one ticker is being added */
	if (nr_stocks == 0)
		nr_stocks = 1;

	char *ticker_argv[nr_stocks];

	if (nr_stocks > 1) {
		argc = cstring_split(tickers, ticker_argv, nr_stocks, ',');
		if (!argc)
			return;
	}

//	if (watchtable->origin)
//		watchtable = watchtable->origin;
	printf("nr_stocks; %d argc: %d\n", nr_stocks, argc);
	for (int x=0; x<nr_stocks; x++)
		if (watchtable_add(session, watchtable, search_stocks(ticker_argv[x])))
			added++;
	if (!added)
		return;

	packet_len = watchtable_packet(session, watchtable, packet);
	printf("addstocks3\n");
	if (packet_len > 0) {
		printf("sending watchtables: %s\n", packet);
		websocket_send(connection, packet, packet_len);
		workspace_broadcast(session, connection, packet, packet_len);
	}
}

/* session->watchlist locked by watchlist_load() */
struct watchlist *watchlist_import(struct session *session, struct connection *connection, char *packet, struct watchlist *src_watchlist, struct watchlist *destination_watchtable)
{
	struct stock *stock;
	int packet_len, x;

	for (x=0; x<src_watchlist->nr_stocks; x++) {
		stock = src_watchlist->stocks[x].stock;
//		stock->update = 1;
		watchtable_add(session, destination_watchtable, stock);
	}

	packet_len = watchtable_packet(session, destination_watchtable, packet);
	websocket_send(connection, packet, packet_len);
	*(packet+packet_len) = 0;
	printf(BOLDGREEN "%s len: %d strlen: %d" RESET "\n", packet, packet_len, (int)strlen(packet));
	return (destination_watchtable);
}

void rpc_watchlist_load(struct rpc *rpc)
{
	struct session    *session        = rpc->session;
	struct connection *connection     = rpc->connection;
	char              *packet         = rpc->packet;
	char              *watchlist_name = rpc->argv[1];
	char              *dst_watchtable = rpc->argv[2];
	char              *QVID           = rpc->argv[3];
	struct watchlist  *gwatch, *src_watchlist, *dwatchtable;
	int                x, nr_watchlists, new_wtab = 0;

	printf(BOLDBLUE "watchlist load name: %s destination watchtable: %s QVID: %s" RESET "\n", watchlist_name, dst_watchtable, QVID);
	/* Get WatchTable Container */
	dwatchtable = search_watchtable(session, dst_watchtable);
	if (!dwatchtable) {
		new_wtab = 1;
		if (!(dwatchtable = watchtable_create(session, dst_watchtable, dst_watchtable)))
			return;
	} else
		watchtable_clear(dwatchtable);

	/* Private Watchlists */
//	mutex_lock(&session->watchlist_lock);
	src_watchlist = search_watchlist(session, watchlist_name);
	if (src_watchlist) {
		watchlist_import(session, connection, packet, src_watchlist, dwatchtable);
		dwatchtable->origin       = src_watchlist;
		dwatchtable->watchlist_id = src_watchlist->watchlist_id;
		if (new_wtab)
			workspace_set_watchtable(session, dwatchtable, QVID);
//		mutex_unlock(&session->watchlist_lock);
		send_watchtable(session, connection, packet, dwatchtable);
		return;
	}

	/* Public Watchlists */
	for (x=0; x<nr_global_watchlists; x++) {
		gwatch = Watchlists[x];
		if (!strcmp(gwatch->name, watchlist_name)) {
			watchlist_import(session, connection, packet, gwatch, dwatchtable);
			dwatchtable->origin       = gwatch;
			dwatchtable->watchlist_id = gwatch->watchlist_id;
			if (new_wtab)
				workspace_set_watchtable(session, dwatchtable, QVID);
//			mutex_unlock(&session->watchlist_lock);
			send_watchtable(session, connection, packet, dwatchtable);
			return;
		}
	}
//	mutex_unlock(&session->watchlist_lock);
}

/* Locked by caller */
struct watchlist *watchtable_create(struct session *session, char *watchtable_id, char *watchlist_name)
{
	struct watchlist *watchlist;

	if (session->nr_watchlists >= MAX_WATCHLISTS)
		return NULL;
	watchlist = (struct watchlist *)zmalloc(sizeof(*watchlist));
	if (!watchlist)
		return NULL;

	printf(BOLDCYAN "wchtable_create(): %s" RESET "\n", watchtable_id);
	strncpy(watchlist->basetable, watchtable_id, BASETABLE_NAME_MAX);
	watchlist->owner = session;
	if (watchlist_name)
		strncpy(watchlist->name, watchlist_name, WATCHLIST_NAME_MAX);
	watchlist->config = IS_MORPHTAB;
	session->watchlists[session->nr_watchlists++] = watchlist;
	return (watchlist);
}

struct watchlist *search_watchtable(struct session *session, char *watchtable_name)
{
	int x, nr_watchlists;

	nr_watchlists = session->nr_watchlists;
	for (x=0; x<nr_watchlists; x++) {
		struct watchlist *watchlist = session->watchlists[x];
		if (!watchlist)
			continue;
		if (!strcmp(watchtable_name, watchlist->basetable)) {
			if (verbose)
				printf(BOLDMAGENTA "search_watchtable(): %s vs %s" RESET "\n", watchtable_name, watchlist->basetable);
			return (watchlist);
		}
	}
	return (NULL);
}

void watchtable_clear(struct watchlist *watchtable)
{
	if (!watchtable) {
		printf(BOLDRED "watchtable_clear: empty watchtable!" RESET "\n");
		return;
	}
	watchtable->origin = NULL;
	if (watchtable->nr_stocks > 0) {
		memset(watchtable->stocks, 0, MIN(watchtable->nr_stocks, MAX_STOCKS)*sizeof(struct watchstock));
		watchtable->nr_stocks     = 0;
		watchtable->nr_conditions = 0;
	}
}

void rpc_watchlist_clear(struct rpc *rpc)
{
	struct session   *session        = rpc->session;
	char             *watchlist_name = rpc->argv[1];
	struct watchlist *watchlist      = search_watchlist(session, watchlist_name);

	if (!watchlist)
		return;

//	mutex_lock(&session->watchlist_lock);
	watchlist->nr_stocks     = 0;
	watchlist->nr_conditions = 0;
	watchlist->origin        = NULL;
//	mutex_unlock(&session->watchlist_lock);
}

double stock_price_target(struct session *session, struct watchlist *watchlist, struct stock *stock)
{
	struct watchcond *watchcond;

	if (!session)
		return 0.0;
//	mutex_lock(&session->watchlist_lock);
	if (!watchlist) {
		if (!session->nr_watchlists)
			goto out;
		if (session->nr_watchlists == 2)
			watchlist = session->watchlists[1];
			if (!watchlist)
				watchlist = session->watchlists[0];
		if (!watchlist)
			goto out;
	}
	watchcond = get_watchcond(watchlist, stock);
	if (!watchcond)
		goto out;
//	mutex_unlock(&session->watchlist_lock);
	return watchcond->price;
out:
//	mutex_unlock(&session->watchlist_lock);
	return 0.0;
}

/* unused */
struct watchlist *watchlist_addstocks(struct session *session, int nstocks, char **s, char *watchlist_name)
{
	struct watchlist *watchlist = search_watchlist(session, watchlist_name);
	int x;

	if (!watchlist)
		return NULL;
	for (x=0; x<nstocks; x++) {
		if (!s[x] || *s[x] == '\0')
			continue;
		watchlist = watchlist_add(session, watchlist, search_stocks(s[x]));
	}
	return (watchlist);
}
