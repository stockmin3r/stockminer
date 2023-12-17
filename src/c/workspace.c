#include <conf.h>
#include <extern.h>

struct qpage *quadverse_pages[4096];
mutex_t       qpage_lock;
int           nr_quadverse_pages;

static __inline__ struct quadverse *new_quadverse()
{
	struct quadverse *quadverse = (struct quadverse *)zmalloc(sizeof(struct quadverse));
	return (quadverse);
}

static __inline__ struct quadspace *new_quadspace()
{
	struct quadspace *quadspace = (struct quadspace *)zmalloc(sizeof(struct quadspace));
	return (quadspace);
}

static __inline__ struct quad *new_quad()
{
	struct quad *quad = (struct quad *)zmalloc(sizeof(struct quad));
	return (quad);
}

static __inline__ struct workspace *new_workspace()
{
	struct workspace *workspace = (struct workspace *)zmalloc(sizeof(struct workspace));
	return (workspace);
}

void new_chart(struct session *session, struct stock *stock)
{
	session->main_chart->stock      = stock;
	session->main_chart->nr_ohlc    = stock->nr_ohlc;
	session->main_chart->nr_updates = stock->nr_updates;
}

int workspace_chart_json(struct stock *stock, char *packet, char *div, char *cfunc, char *className)
{
	struct price *price;
	int           packet_len;

	price  = stock->price;
	packet_len = snprintf(packet, 256, "stockchart {\"type\":\"stock\",\"dtype\":\"array\",\"ticker\":\"%s\",\"div\":\"%s\",\"nr_1d\":\"%d\",\"cb\":\"%s\",\"class\":\"%s\",\"data\":",
						stock->sym, div, price->nr_points_1d==-1?0:price->nr_points_1d, cfunc?cfunc:"-", className?className:"");
	memcpy(packet+packet_len, price->price_1d, price->price_1d_len);
	packet_len += price->price_1d_len;
	packet[packet_len++] = '}';
	return (packet_len);
}

int pack_stockpage(struct stock *stock, char *packet, char *div, struct wsid *wsid, char *cfunc)
{
	struct price *price = stock->price;
	int packet_len;

	if (!price)
		return 0;
	packet_len = snprintf(packet, 256, "stockpage {\"type\":\"stock\",\"dtype\":\"array\",\"ticker\":\"%s\",\"div\":\"%s\",\"nr_1d\":\"%d\",\"cb\":\"%s\",\"QVID\":\"%d\",\"QSID\":\"%d\",\"data\":",
						  stock->sym, div, price->nr_points_1d, cfunc?cfunc:"-", wsid->QVID, wsid->QSID);
	memcpy(packet+packet_len, price->price_1d, price->price_1d_len);
	packet_len += price->price_1d_len;
	packet[packet_len++] = '}';
	return (packet_len);
}

/* main chart only - unused */
void pack_chart(struct rpc *rpc)
{
	char           *packet    = rpc->packet;
	char           *ticker    = rpc->argv[1];
	char           *chart_div = rpc->argv[2];
	struct stock   *stock;
	struct price   *price;
	int             packet_len;

	stock = search_stocks(ticker);
	if (!stock)
		return;

	price = stock->price;
	if (!price->price_1m)
		return;

	packet_len = snprintf(packet, 256, "mainchart {\"type\":\"stock\",\"dtype\":\"array\",\"ticker\":\"%s\",\"div\":\"%s\",\"nr_1d\":\"%d\",\"cb\":\"-\",\"data\":", stock->sym, chart_div, price->nr_points_1d);
	memcpy(packet+packet_len, price->price_1d, price->price_1d_len);
	packet_len += price->price_1d_len;
	packet[packet_len++] = '}';
	new_chart(rpc->session, stock);
	websocket_send(rpc->connection, packet, packet_len);
}

int pack_mini(struct stock *stock, char *packet, struct chart *chart, struct wsid *wsid, int limit, char *moveTo, int height, int interval)
{
	struct price *price;
	char         *ticker;
	int           nbytes, packet_len, mini_len;

	if (*stock->sym == '^')
		ticker = stock->sym+1;
	else
		ticker = stock->sym;
	if (chart) {
		snprintf(chart->div, MAX_DIVSIZE-1, "P%dQ%dq%dws%d-%s", wsid->QVID, wsid->QSID, wsid->QID, wsid->WSID, stock->sym);
		nbytes  = snprintf(packet, 32, "mini %s %s ", ticker, chart->div);
	} else {
		nbytes  = snprintf(packet, 64, "mini %s P%dQ%dq%dws%d-%s ", ticker, wsid->QVID, wsid->QSID, wsid->QID, wsid->WSID, ticker);
	}
	price       = stock->price;
	mini_len    = price->price_mini_len;
	if (mini_len <= 5)
		return 0;

	memcpy(packet+nbytes, price->price_mini, mini_len);
	packet_len  = nbytes + mini_len;
	printf("sym: %s curprice: %.2f pr per: %.2f addr: %p\n",  stock->sym, stock->current_price, stock->pr_percent, stock);
	packet_len += snprintf(packet+packet_len, 64, " %.2f %.2f %c %s %s %d %d", stock->current_price, stock->pr_percent, limit?'1':'0', stock->current_price>stock->price_open?"green" : "red", moveTo, height, interval);
	packet[packet_len] = 0;
	return (packet_len);
}

void rpc_chart(struct rpc *rpc)
{
	struct session     *session    = rpc->session;
	struct connection  *connection = rpc->connection;
	char               *packet     = rpc->packet+(rpc->internal ? 0 : 10);
	char               *QGID       = rpc->argv[1];
	char               *new_ticker = rpc->argv[2];
	char               *old_ticker = rpc->argv[3];
	char               *cfunc      = rpc->argv[4];
	char               *className  = rpc->argv[5];
	struct quadverse   *quadverse;
	struct workspace   *workspace;
	struct subscriber **subscribers;
	struct qpage       *qpage;
	struct stock       *stock;
	struct chart       *chart, *replacedChart = NULL;
	struct wsid         wsid;
	int                 packet_len = 0, chart_exists = 0, create_workspace = 0, nr_charts, nr_subscribers, x, y;
	int                 JQVID, QVID, QSID, QID, WSID;

	if (!(stock=search_stocks(new_ticker)))
		return;

//	mutex_lock(&session->watchlist_lock);
	if (!workspace_id(session, QGID, &wsid))
		goto out;
	QVID = wsid.QVID; QSID = wsid.QSID; QID = wsid.QID, WSID = wsid.WSID;
	workspace = get_workspace (session, &wsid, &create_workspace);
	if (!workspace || workspace->nr_charts >= MAX_CHARTS)
		goto out;

	quadverse = session->quadverse[QVID];
	if ((qpage=quadverse->qpage))
		JQVID = 0;
	else
		JQVID = QVID;

	// make sure this ticker hasn't already been loaded in this workspace
	nr_charts = workspace->nr_charts;
	for (x=0; x<nr_charts; x++) {
		chart = workspace->charts[x];
		if (!chart)
			continue;
		if (chart->stock == stock) {
			chart_exists = 1;
			break;
		}
		if (old_ticker && *old_ticker != '-' && !strcmp(old_ticker, chart->stock->sym)) {
			replacedChart = chart;
			break;
		}
	}
	if (!chart_exists && !replacedChart) {
		if (!workspace->nr_charts)
			qcache_setname(session, qpage, new_ticker, JQVID, QSID, QID, WSID); // XXX: SYNC

		// malloc new struct chart to wrap the stock struct it points to
		chart = workspace->charts[workspace->nr_charts++] = (struct chart *)zmalloc(sizeof(*chart));
		if (!chart)
			return;
		qcache_addchart(session, qpage, new_ticker, JQVID, QSID, QID, WSID); // XXX: SYNC
	}
	if (replacedChart) {
		chart = replacedChart;
		qcache_replace_chart(session, qpage, old_ticker, new_ticker, JQVID, QSID, QID, WSID); // XXX: SYNC
	}
	if (!chart)
		goto out;

	snprintf(chart->div, 32, "%s-%s", stock->sym, QGID); // create a Chart Global ID - CGID
	packet_len   = workspace_chart_json(stock, packet, chart->div, cfunc, className);
	chart->stock = stock;
	chart->type  = CHART_TYPE_OHLC;
	if (!packet_len || rpc->internal) {
		rpc->packet_size = packet_len;
		goto out;
	}

	/* Don't send the same chart to a browser or tab which has it already */
	if (chart_exists)
		websocket_sendx(connection, packet-10, packet_len);
	else
		websockets_sendall(session, packet-10, packet_len);

	quadverse = session->quadverse[session->current_quadverse[connection->websocket_id]];
	if (!quadverse)
		return;

	/*****************************
	 * Send chart to subscribers *
	 ****************************/
	qpage = quadverse->qpage;
	if (!qpage)
		goto out;

	nr_subscribers = qpage->nr_subscribers;
	subscribers    = qpage->subscribers;
	printf("nr subs: %d %d\n", nr_subscribers, session->current_quadverse[connection->websocket_id]);
	for (x=0; x<nr_subscribers; x++) {
		struct session *qsub_session = subscribers[x]->session;
		if (qsub_session == session)
			continue;
//		printf("sending packet to sub: %s\n", (char *)&qsub_session->connection->cookie);
		for (y=0; y<qsub_session->nr_websockets; y++) {
			connection = qsub_session->websockets[y];
			if (!connection || connection->fd == -1)
				continue;
			websocket_sendx(connection, packet-10, packet_len);
		}
	}
out:
	return;
//	mutex_unlock(&this_session->watchlist_lock);
}

void rpc_stockpage(struct rpc *rpc)
{
	struct session    *session    = rpc->session;
	struct connection *connection = rpc->connection;
	char              *packet     = rpc->packet;
	char              *ticker     = rpc->argv[1];
	int                QVID       = atoi(rpc->argv[2]);
	int                QSID       = atoi(rpc->argv[3]);
	struct stock      *stock      = search_stocks(ticker);
	struct quadverse  *quadverse;
	struct workspace  *workspace;
	struct chart      *chart;
	struct wsid        WSID;
	char               id[24];
	int                packet_len;

	if (!stock || (QVID == 0 && QSID < 5) || QVID == 2)
		return;

/*	if (stockpage_exists(session, stock)) {
		return;
	}*/

	sprintf(id, "P%dQ%dq2ws0", QVID, QSID);
	if (!workspace_id(session, id, &WSID))
		return;
	workspace = get_workspace(session, &WSID, NULL);
	if (!workspace || workspace->nr_charts >= MAX_CHARTS)
		return;
	quadverse = session->quadverse[WSID.QVID];
	quadverse->current_quadspace[connection->websocket_id] = WSID.QSID;
	quadverse->quadspace[WSID.QSID]->stockpage = ticker;  // QuadSpace Title
	chart = workspace->charts[workspace->nr_charts++] = (struct chart *)zmalloc(sizeof(*chart));
	snprintf(chart->div, MAX_DIVSIZE-1, "%s-%s", stock->sym, id);
	packet_len   = pack_stockpage(stock, packet, chart->div, &WSID, NULL);
	chart->stock = stock;
	chart->type  = CHART_TYPE_OHLC;

	if (!session->qcache) {
		printf(BOLDCYAN "stock_page: CREATING QCACHE" RESET "\n");
		session->qcache = qcache_create(session, QVID, QSID, ticker, 1);
	} else {
		qcache_new_quadspace(session, quadverse->qpage, QVID, QSID, NULL, ticker, 1);
		printf(BOLDYELLOW "stock_page: qcache new quadspace QVID: %d QSID: %d" RESET "\n", QVID, QSID);
	}
	websocket_send(connection, packet, packet_len);
	packet[packet_len++] = ' ';
	packet[packet_len++] = '1';
	printf(BOLDGREEN "%s" RESET "\n", packet);
	websockets_sendall_except(session, connection, packet, packet_len);
}

void rpc_mini_charts(struct rpc *rpc)
{
	char             *packet = rpc->packet;
	char             *QGID   = rpc->argv[1];
	struct workspace *workspace;
	struct chart     *chart;
	struct wsid       wsid;
	int               x, nbytes, count, packet_len = 0;

	if (!workspace_id(rpc->session, QGID, &wsid))
		return;
	workspace = get_workspace(rpc->session, &wsid, NULL);
	count = MIN(NR_DELTA_STOCKS, 14);
	for (x=0; x<count; x++) {
		if (workspace->nr_charts >= MAX_CHARTS)
			break;
		chart        = workspace->charts[workspace->nr_charts++];
		if (!chart || !chart->stock)
			continue;
//		stock        = get_stock();
		nbytes       = pack_mini(chart->stock, packet+packet_len, chart, &wsid, 0, "0", 250, 0);
		packet_len  += nbytes;
//		chart->stock = stock;
		chart->type  = CHART_TYPE_OHLC;
		*(packet+packet_len++) = '@';
	}
	websocket_sendx(rpc->connection, packet, packet_len-1);
	packet[packet_len-1] = 0;
	printf("packet: %s len: %d strlen: %d\n", packet, packet_len, (int)strlen(packet));
}

static __inline__ struct workspace *workspace_lookup(struct session *session, int QVID, int QSID, int QID, int WSID)
{
	struct quadverse *quadverse;
	struct quadspace *quadspace;
	struct quad      *quad;
	struct workspace *workspace;

	if (QVID < 0 || QVID >= MAX_QUADVERSES || QSID < 0 || QSID >= MAX_QUADSPACES)
		return NULL;
	if (QID < 0  || QID >= MAX_QUADS || WSID < 0 || WSID >= MAX_WORKSPACES) 
		return NULL;

	if (!(quadverse=session->quadverse[QVID]))
		return NULL;

	if (!(quadspace=quadverse->quadspace[QSID]))
		return NULL;

	if (!(quad=quadspace->quad[QID]))
		return NULL;
	if (!(workspace=quad->workspace[WSID]))
		return NULL;
	return (workspace);
}

int workspace_id(struct session *session, char *id, struct wsid *wsid)
{
	char *p = id;

	if (!id)
		return 0;

	if (*id == '#')
		id++;
	wsid->WSID  = -1;
	wsid->QID   = 0;
	wsid->QVID  = atoi(id+1);
	if (wsid->QVID < 0 || wsid->QVID >= MAX_QUADVERSES)
		return 0;
	while (*p != 'Q') {
		p++;
		if (*p == '\0')
			return 0;
	}
	wsid->QSID = atoi(p+1);
	if (wsid->QSID < 0 || wsid->QSID >= MAX_QUADSPACES)
		return 0;

	while (*p != 'q') {
		p++;
		if (*p == '\0')
			return 1;
	}
	wsid->QID = atoi(p+1);
	if (wsid->QID < 0 || wsid->QID >= MAX_QUADS)
		return 0;
	while (*p != 's') {
		p++;
		if (*p == '\0')
			return 0;
	}
	wsid->WSID = atoi(p+1);
	if (wsid->WSID < 0 || wsid->WSID >= MAX_WORKSPACES)
		return 0;
	return 1;
}

struct workspace *get_workspace(struct session *session, struct wsid *wsid, int *create_workspace)
{
	struct quadverse *quadverse;
	struct quadspace *quadspace;
	struct quad      *quad;
	struct workspace *workspace;
	int QVID = wsid->QVID, QSID = wsid->QSID, QID = wsid->QID, WSID = wsid->WSID;

	if (!(quadverse = session->quadverse[QVID])) {
		quadverse   = session->quadverse[QVID]    = new_quadverse();
		session->nr_quadverses++;
	}
	if (!(quadspace = quadverse->quadspace[QSID])) {
		quadspace   = quadverse->quadspace[QSID] = new_quadspace();
		quadverse->nr_quadspaces++;
	}
	if (!(quad      = quadspace->quad[QID])) {
		quad        = quadspace->quad[QID]       = new_quad();
		quadspace->nr_quads++;
	}
	if (!(workspace = quad->workspace[WSID])) {
		workspace   = quad->workspace[WSID]      = new_workspace();
		quad->nr_workspaces++;
		if (create_workspace)
			*create_workspace = 1;
	}
	return (workspace);
}

// unused,dead,to be removed
int workspace_charts(struct session *session, char *packet, int argc, char **argv)
{
	struct stock     *stock;
	struct quadverse *quadverse;
	struct workspace *workspace;
	struct qpage     *qpage;
	struct chart     *chart;
	struct wsid       WSID;
	int x, nbytes, JQVID, packet_len = 0, create_workspace = 0;

	for (x=0; x<argc; x+=2) {
		stock = search_stocks(argv[x+1]);
		if (!stock)
			continue;
		if (!workspace_id(session, argv[x], &WSID))
			return 0;
		workspace  = get_workspace(session, &WSID, &create_workspace);
		if (!workspace || workspace->nr_charts >= MAX_CHARTS)
			return 0;
		quadverse = session->quadverse[WSID.QVID];
		if ((qpage=quadverse->qpage))
			JQVID = 0;
		else
			JQVID = WSID.QVID;
		if (create_workspace)
			qcache_new_workspace(session, qpage, "Charts", JQVID, WSID.QSID, WSID.QID, WSID.WSID);
		chart        = workspace->charts[workspace->nr_charts++] = (struct chart *)zmalloc(sizeof(*chart));
		snprintf(chart->div, 32, "%s-%s", stock->sym, argv[x]);
		nbytes       = workspace_chart_json(stock, packet+packet_len, chart->div, NULL, NULL);
		packet_len  += nbytes;
		chart->stock = stock;
		chart->type  = CHART_TYPE_OHLC;
		packet[packet_len++] = '@';
	}
	packet[--packet_len] = 0;
	return (packet_len);
}

void workspace_broadcast(struct session *session, struct connection *connection, char *packet, int packet_len)
{
	struct quadverse   *quadverse = session->quadverse[session->current_quadverse[connection->websocket_id]];
	struct qpage       *qpage;
	struct subscriber **subscribers;
	struct session     *qsub_session;
	int                 nr_subscribers, x, y;

	websockets_sendall_except(session, connection, packet, packet_len);
	if (!quadverse)
		return;
	qpage = quadverse->qpage;
	if (!qpage)
		return;

	nr_subscribers = qpage->nr_subscribers;
	subscribers    = qpage->subscribers;
	printf("workspace_boardcast nr_subs: %d %d\n", nr_subscribers, session->current_quadverse[connection->websocket_id]);
	for (x=0; x<nr_subscribers; x++) {
		struct subscriber *qsub = subscribers[x];
		if (!qsub)
			continue;
		qsub_session = qsub->session;
		if (qsub_session == session)
			continue;
//		printf("workspace_broadcast: sending packet to sub: %.8s\n", (char *)&qsub_session->connection->cookie);
//		websocket_sendall(qsub_session, packet, packet_len);
		for (y=0; y<qsub_session->nr_websockets; y++) {
			connection = qsub_session->websockets[y];
			if (!connection || connection->fd == -1)
				continue;
			websocket_sendx(connection, packet, packet_len);
		}
	}
}

int chart_indicator_add(struct session *session, struct qpage *qpage, char *packet, char *params, int JQVID, int QVID, int QSID, int QID, int WSID)
{
	struct chart *chart;
	char *ticker = params, *div, *indi;

	/* TSLA-P0Q5q0ws0&atr */
	printf("chart_indicator_add: %s\n", params);
	div = strchr(params, '-');
	if (!div)
		return 0;

	*div++ = 0;
	indi = strchr(div, '&');
	if (!indi || strlen(indi) > 84)
		return 0;
	*indi++ = 0;

	/* 1) Add Indicator to Workspace Chart */
	chart = search_chart(session, ticker, QVID, QSID, QID, WSID);
	if (!chart)
		return 0;
	if (!chart->indicators) {
		chart->indicators    = (char **)malloc(sizeof(char *) * 16);
		chart->nr_indicators = 0;
	} else if (chart->nr_indicators >= 16)
		return 0;
	chart->indicators[chart->nr_indicators++] = strdup(indi);

	/* 2) Add Indicator to QCache */
	if (!qcache_chart_indicator_add(session, qpage, ticker, indi, JQVID, QSID, QID, WSID))
		return 0;
	return snprintf(packet, 256, "qupdate indi 1 %s-%s %s", ticker, div, indi);
}

int chart_indicator_remove(struct session *session, struct qpage *qpage, char *packet, char *params, int JQVID, int QVID, int QSID, int QID, int WSID)
{
	struct chart *chart;
	char *indicator, *div, *ticker = params;
	int nr_indicators, x;

	div = strchr(params, '-');
	if (!div)
		return 0;

	*div++ = 0;
	indicator = strchr(div, '&');
	if (!indicator || strlen(indicator) > 32)
		return 0;
	*indicator++ = 0;

	chart = search_chart(session, ticker, QVID, QSID, QID, WSID);
	if (!chart)
		return 0;

	nr_indicators = chart->nr_indicators;
	if (!chart->indicators || !nr_indicators)
		return 0;

	for (x=0; x<nr_indicators; x++) {
		if (chart->indicators[x] && !strcmp(chart->indicators[x], indicator)) {
			if (x == (nr_indicators-1))
				chart->indicators[x] = NULL;
			else {
				memmove(&chart->indicators[x+1], &chart->indicators[x], (nr_indicators-x-1)*sizeof(void *));
				chart->indicators[nr_indicators-1] = NULL;
			}
			chart->nr_indicators--;
			break;
		}
	}
	qcache_chart_indicator_remove(session, qpage, ticker, indicator, JQVID, QSID, QID, WSID);
	return snprintf(packet, 256, "qupdate indi 0 %s-%s %s", ticker, div, indicator);
}

struct chart *search_chart(struct session *session, char *ticker, int QVID, int QSID, int QID, int WSID)
{
	struct workspace *workspace;
	struct chart     *chart;
	int               nr_charts, x;

	workspace = workspace_lookup(session, QVID, QSID, QID, WSID);
	if (!workspace)
		return (NULL);

	nr_charts = workspace->nr_charts;
	for (x=0; x<nr_charts; x++) {
		chart = workspace->charts[x];
		if (!chart)
			continue;
		if (!strcmp(chart->stock->sym, ticker))
			return (chart);
	}
	return (NULL);
}

int qpage_load_qcache(struct qpage *qpage)
{
	yyjson_doc *doc;

	printf("qpage loading qcache: %s len: %d\n", qpage->qcache_json, qpage->qsize);
	doc = yyjson_read(qpage->qcache_json, qpage->qsize, 0);
	if (!doc)
		return 0;
	qpage->qcache = yyjson_doc_mut_copy(doc, NULL);
	if (!qpage->qcache)
		return 0;
	return 1;
}


void new_profile_qpage(struct session *session)
{
	struct qpage *qpage     = (struct qpage *)zmalloc(sizeof(*qpage));
	qpage->quadverse        = session->quadverse[QUADVERSE_PROFILE];
	qpage->quadverse->qpage = qpage;
	qpage->subscribers      = (struct subscriber **)zmalloc(sizeof(struct subscriber *) * MAX_SUBSCRIBERS);
	strncpy(qpage->url, session->user->uname, MAX_USERNAME_SIZE);
	printf("new profile qpage: %p url: %s\n", qpage, qpage->url);
}

void load_quadverse_pages()
{
	struct list_head *session_list;
	struct session   *session;
	struct qpage     *qpage, *qpage_file;
	char              qbuf[sizeof(struct qpage) * MAX_QPAGES];
	char              path[256];
	int               qsize, nr_qpages;

	session_list = get_session_list();
	DLIST_FOR_EACH_ENTRY(session, session_list, list) {
		session->qpages    = NULL;
		session->nr_qpages = 0;
		if (*session->user->uname == '\0')
			continue;
		sprintf(path, QPAGE_PATH, session->user->uid);
		qsize = fs_readfile(path, qbuf, sizeof(qbuf));
		printf("qpage path: %s qsize: %d\n", path, qsize);
		if (!qsize) {
//			new_profile_qpage(session);
			continue;
		}
		nr_qpages          = qsize/sizeof(struct qpage);
		qpage_file         = (struct qpage *)qbuf;
		session->nr_qpages = nr_qpages;
		session->qpages    = (struct qpage **)malloc(sizeof(struct qpage *) * (MAX(32, nr_qpages)));
		for (int x=0; x<nr_qpages; x++) {
			qpage              = (struct qpage *)malloc(sizeof(*qpage));
			quadverse_pages[x] = qpage;
			session->qpages[x] = qpage;
			memcpy(qpage, qpage_file, sizeof(*qpage_file));
			memset(&qpage->qpage_lock, 0, sizeof(qpage->qpage_lock));
			qpage->subscribers = (struct subscriber **)zmalloc(sizeof(struct subscriber *) * MAX_SUBSCRIBERS);
			if (qpage->flags  &= QUADVERSE_PROFILE) {
				qpage->quadverse        = session->quadverse[QUADVERSE_PROFILE];
				qpage->quadverse->qpage = qpage;
				strncpy(qpage->url, session->user->uname, MAX_USERNAME_SIZE);
			} else {
				session->quadverse[session->nr_quadverses++] = qpage->quadverse = new_quadverse();
				qpage->quadverse->qpage = qpage;
			}
			qpage_load_qcache(qpage);
			nr_quadverse_pages++;
			qpage_file++;
		}
//		if (!session->quadverse[QUADVERSE_PROFILE]->qpage)
//			new_profile_qpage(session);
	}
}

/*
void workspace_close(struct session *session)
{
	int nr_quadverses, x, y, z;

//	mutex_lock(&session->watchlist_lock)
	nr_quadverses = session->nr_quadverses;
	if (!(quadverse=session->quadverse[q]))
		continue;
	nr_quadspaces = quadverse->nr_quadspaces;
	for (x=0; x<MAX_QUADSPACES; x++) {
		if (!(quadspace=quadverse->quadspace[x]))
			continue;
		nr_quads = quadspace->nr_quads;
		for (y=0; y<MAX_QUADS; y++) {
			if (!(quad=quadspace->quad[y]))
				continue;
			nr_workspaces = quad->nr_workspaces;
			for (z=0; z<MAX_WORKSPACES; z++) {
				if (!(workspace=quad->workspace[z]))
					continue;


}*/

int addPoint_page(struct session *session, char *packet)
{
	struct quadspace *quadspace;
	struct quadverse *quadverse;
	struct workspace *workspace;
	struct quad      *quad;
	struct stock     *stock;
	struct chart     *chart;
	struct tick      *tick;
	int packet_len = 0, nbytes, c, q, x, y, z;
	int nr_quadverses, nr_quadspaces, nr_quads, nr_workspaces, nr_charts;

	nr_quadverses = session->nr_quadverses;
	if (nr_quadverses > MAX_QUADVERSES)
		return 0;
	for (q=0; q<MAX_QUADVERSES; q++) {
		if (!(quadverse=session->quadverse[q]))
			continue;
		nr_quadspaces = quadverse->nr_quadspaces;
		for (x=0; x<MAX_QUADSPACES; x++) {
			if (!(quadspace=quadverse->quadspace[x]))
				continue;
			nr_quads = quadspace->nr_quads;
			for (y=0; y<MAX_QUADS; y++) {
				if (!(quad=quadspace->quad[y]))
					continue;
				nr_workspaces = quad->nr_workspaces;
				for (z=0; z<MAX_WORKSPACES; z++) {
					if (!(workspace=quad->workspace[z]))
						continue;
					nr_charts = workspace->nr_charts;
					for (c=0; c<nr_charts; c++) {
						chart = workspace->charts[c];
						if (!chart || !(stock=chart->stock))
							continue;
						if (chart->nr_ohlc == stock->nr_ohlc)
							continue;
						chart->nr_ohlc = stock->nr_ohlc;
						tick           = stock->current_tick;
//						printf("chart ohlc: %d stock ohlc: %d\n", chart->nr_ohlc, stock->nr_ohlc);
						if (chart->type == CHART_TYPE_OHLC) {
							nbytes = snprintf(packet+packet_len, 512, "addPoint %s %s@", chart->div, tick->current_ohlc);
							printf(BOLDWHITE "addPoint: %s" RESET "\n", packet);
						} else if (chart->type == CHART_TYPE_MINI) {
							nbytes = snprintf(packet+packet_len, 512, "upmini %s %s %s %.2f %.2f add@",
							stock->sym, chart->div, tick->current_mini, stock->current_price, stock->pr_percent);
							printf(BOLDWHITE "addMini: %s" RESET "\n", packet);
						} else
							continue;
						packet_len += nbytes;
					}
				}
			}
		}
	}
	return (packet_len);
}

void workspace_set_watchtable(struct session *session, struct watchlist *watchtable, char *QGID)
{
	struct quadverse *quadverse;
	struct quadspace *quadspace;
	struct workspace *workspace;
	struct watchlist *origin;
	struct wsid       wsid;
	char              wstab[512];
	int               QVID, QSID, QID, WSID;

	printf(BOLDYELLOW "workspace_set_watchtable: %s" RESET "\n", QGID);
	if (!workspace_id(session, QGID, &wsid))
		return;
	QVID = wsid.QVID;
	QSID = wsid.QSID;
	QID  = wsid.QID;
	WSID = wsid.WSID;
	workspace = get_workspace(session, &wsid, NULL);
	if (!workspace)
		return;
	quadverse = session->quadverse[QVID];
	if (!quadverse)
		return;
	origin = watchtable->origin;
	if (!origin)
		return;
	workspace->watchlists[workspace->nr_watchtables++] = watchtable;
	snprintf(wstab, 256, "%s:%s", watchtable->name, origin->name);
	qcache_add_object(session, quadverse->qpage, "wstab", wstab, QVID, QSID, QID, WSID);
	printf(BOLDBLUE "adding table: watchtable->name: %s watchlist_id: %s watchtable: %p workspace: %p qbuf: %s" RESET "\n", watchtable->name, (char *)&watchtable->watchlist_id, watchtable, workspace, wstab);
}

/*
 * Update all dynamic charts & tables in a single Workspace
 */
int process_workspace(struct session *session, char *packet, struct workspace *workspace, int PID, int QSID, int QID, int WSID)
{
	struct stock *stock;
	struct chart *chart;
	struct ohlc  *ohlc;
	int           nbytes, packet_len = 0;

	/* 1) Update Charts */
	for (int x=0; x<workspace->nr_charts; x++) {
		if (!(chart=workspace->charts[x]))
			continue;
		stock = chart->stock;
		if (chart->dead || !stock)
			continue;
		if (chart->nr_updates == stock->nr_updates)
			continue;
		chart->nr_updates = stock->nr_updates;
		ohlc = &stock->ohlc[stock->nr_ohlc-1];
		if (chart->type == CHART_TYPE_OHLC) {
			nbytes = sprintf(packet+packet_len, "update %s [%.3f,%.3f,%.2f,%.2f,%llu]@", chart->div, ohlc->open, ohlc->high, ohlc->low, ohlc->close, ohlc->volume);
			printf(BOLDGREEN "CHART_TYPE_OHLC: %s" RESET "\n", packet+packet_len);
		} else if (chart->type == CHART_TYPE_MINI) {
			nbytes = sprintf(packet+packet_len, "upmini %s %s %s %.2f %.2f@",
			stock->sym, chart->div, stock->current_tick->current_mini, stock->current_price, stock->pr_percent);
			printf(BOLDMAGENTA "CHART_TYPE_MINI: %s" RESET "\n", packet+packet_len);
		} else
			continue;
		packet_len += nbytes;
	}

	/* 2) Special WatchTables (UFO) */
	if (workspace->watchtables) {
		packet_len += workspace->watchtables(session, packet+packet_len, workspace);
		return (packet_len);
	}

	/* 3) Generic WatchTables */
//	printf(BOLDYELLOW "updating watchtables? QSID: %d QID: %d WSID: %d nr_watchtables: %d" RESET "\n", QSID, QID, WSID, workspace->nr_watchtables);
	for (int x=0; x<workspace->nr_watchtables; x++) {
		if (workspace->watchlists[x]) {
			int size = watchtable_packet(session, workspace->watchlists[x], packet+packet_len);
			if (size > 0) {
				packet_len += size;
				packet[packet_len++] = '@';
			}
		}
	}
	return (packet_len);
}

/*
 * Update the dynamic charts & tables belonging to the current QuadSpace
 *   - For each Quad in a QuadSpace:
 *     - Calls process_workspace() for each Quad's Workspace
 */
void update_page(struct session *session, struct connection *connection, char *packet)
{
	struct quadspace *quadspace;
	struct quadverse *quadverse;
	struct workspace *workspace;
	struct quad      *quad;
	int               packet_len = 0, QID;
	int               current_quadverse, current_quadspace, current_workspace;

	packet_len        = addPoint_page(session, packet);
	current_quadverse = session->current_quadverse[connection->websocket_id];
	quadverse         = session->quadverse[current_quadverse];
	if (!quadverse)
		return;
	current_quadspace = quadverse->current_quadspace[connection->websocket_id];
	quadspace         = quadverse->quadspace[current_quadspace];
	if (!quadspace)
		return;
	for (QID=0; QID<MAX_QUADS; QID++) {
		quad = quadspace->quad[QID];
		if (!quad)
			continue;
		current_workspace = quad->current_workspace[connection->websocket_id];
		workspace         = quad->workspace[current_workspace];
		if (!workspace)
			continue;
		packet_len += process_workspace(session, packet+packet_len, workspace, current_quadverse, current_quadspace, QID, current_workspace);
	}
	strcpy(packet+packet_len, "index ");
	packet_len += 6;
	packet_len += stocks_get_indexes(packet+packet_len);
	packet[packet_len] = 0;
	websocket_send(connection, packet, packet_len);
}

struct quadverse *qpage_quadverse2(struct session *session, char *URL, int *qindex)
{
	for (int x=0; x<MAX_QUADVERSES; x++) {
		struct quadverse *quadverse = session->quadverse[x];
		if (!quadverse || !quadverse->qpage)
			continue;
		if (!strcmp(quadverse->qpage->url, URL)) {
			*qindex = x;
			return (quadverse);
		}
	}
	return NULL;
}

struct quadverse *qpage_quadverse(struct session *session, struct qpage *qpage, int *qindex)
{
	for (int x=0; x<MAX_QUADVERSES; x++) {
		struct quadverse *quadverse = session->quadverse[x];
		if (!quadverse || quadverse->qpage != qpage)
			continue;
		*qindex = x;
		return (quadverse);
	}
	return NULL;
}

struct qpage *get_qpage(char *URL)
{
	struct qpage *qpage;
	int x;

	for (x=0; x<nr_quadverse_pages; x++) {
		qpage = quadverse_pages[x];
		if (!strcmp(qpage->url, URL))
			return (qpage);
	}
	return NULL;
}

static __inline__ int qpage_is_owner(int uid, struct qpage *qpage)
{
	int x, nr_owners = qpage->nr_owners;

	for (x=0; x<nr_owners; x++) {
		if (qpage->owners[x].uid == uid)
			return 1;
	}
	return 0;
}

static __inline__ char *qpage_username(struct qpage *qpage)
{
	struct session *session = session_by_uid(qpage->owners[0].uid);
	if (!session || *session->user->uname == '\0')
		return NULL;
	return session->user->uname;
}

int remove_quadspace(struct session *session, struct qpage *qpage, int JQVID, int QVID, int QSID)
{
	struct quadverse *quadverse = session->quadverse[QVID];

	if (!quadverse)
		return 0;
	quadverse->quadspace[QSID] = NULL;
	qcache_remove_quadspace(session, qpage, JQVID, QSID);
	return 1;
}

int remove_workspace(struct session *session, struct qpage *qpage, int JQVID, int QVID, int QSID, int QID, int WSID)
{
	struct quadverse *quadverse;
	struct quadspace *quadspace;
	struct quad      *quad;

	quadverse = session->quadverse[QVID];
	if (!quadverse)
		return 0;
	quadspace = quadverse->quadspace[QSID];
	if (!quadspace)
		return 0;
	quad      = quadspace->quad[QID];
	if (!quad)
		return 0;
	quad->workspace[WSID] = NULL;
	qcache_remove_workspace(session, qpage, JQVID, QSID, QID, WSID);
	return 1;
}

int workspace_name(struct session *session, char *packet, struct qpage *qpage, char *title, int JQVID, int QVID, int QSID, int QID, int WSID)
{
	int packet_len;

	printf("workspace name: %s\n", title);
	qcache_setname(session, qpage, title, JQVID, QSID, QID, WSID);
	if (WSID != -1)
		packet_len = snprintf(packet, 255, "qupdate wsname {\"QGID\":\"#P%dQ%dq%d^.ws%d\",\"title\":\"%s\"}", QVID, QSID, QID, WSID, title);
	else
		packet_len = snprintf(packet, 255, "qupdate wsname {\"QGID\":\"#quadverse%d^.chrome-tab:nth-child(%d)\",\"title\":\"%s\"}", QVID, QSID, title);
	return (packet_len);
}

int remove_chart(struct session *session, char *ticker, int QVID, int QSID, int QID, int WSID)
{
	struct wsid       wsid;
	struct chart     *chart;
	struct workspace *workspace;
	int               nr_charts, x;

	wsid.QVID = QVID;
	wsid.QSID = QSID;
	wsid.QID  = QID;
	wsid.WSID = WSID;
	workspace = get_workspace (session, &wsid, NULL);
	if (!workspace)
		goto out;

	nr_charts = workspace->nr_charts;
	for (x=0; x<nr_charts; x++) {
		chart = workspace->charts[x];
		if (chart && !strcmp(chart->stock->sym, ticker)) {
			if (x == (nr_charts-1))
				workspace->charts[x] = NULL;
			else {
				memmove(&workspace->charts[x+1], &workspace->charts[x], (nr_charts-x-1)*sizeof(void *));
				workspace->charts[nr_charts-1] = NULL;
			}
			workspace->nr_charts--;
//			free(chart);
			break;
		}
	}
	return 1;
out:
	return 0;
}


/*
 * qupdate QSP_NAME 
 * qupdate QSP_DEL
 */
int quadverse_update_packet(struct session *session, struct qpage *qpage, char *packet, int operation, int JQVID, int QVID, int QSID, int QID, int WSID, char *args)
{
	char QGID[64];
	char *blob_url, *blob_type, *grid, *ticker, *URL = args;
	int packet_len;

	snprintf(QGID, sizeof(QGID)-1, "#P%dQ%dq%dws%d", QVID, QSID, QID, WSID);
	switch (operation) {
		case QUPDATE_WS_NAME:
			packet_len = workspace_name(session, packet, qpage, args, JQVID, QVID, QSID, QID, WSID);
			break;
		case QUPDATE_GRID:
			grid = args;
			packet_len = snprintf(packet, 256, "qupdate grid {\"QGID\":\"%s\",\"grid\":\"%s\"}", QGID, grid);
			qcache_setgrid(session, qpage, grid, JQVID, QSID, QID, WSID);
			break;
		case QUPDATE_POSITION:
			blob_url = strchr(args, ':');
			if (!blob_url)
				return 0;
			*blob_url++ = 0;
			blob_type = args;
			qcache_set_position(session, qpage, blob_url, blob_type, JQVID, QSID, QID, WSID);
			*(blob_url-1) = ':';
			packet_len = snprintf(packet, 256, "qupdate pos {\"URL\":\"%s\"}", blob_url);
			break;
		case QUPDATE_ADD_CHART_INDI:
			if (!(packet_len=chart_indicator_add   (session, qpage, packet, args, JQVID, QVID, QSID, QID, WSID)))
				return 0;
			break;
		case QUPDATE_DEL_CHART_INDI:
			if (!(packet_len=chart_indicator_remove(session, qpage, packet, args, JQVID, QVID, QSID, QID, WSID)))
				return 0;
			break;
		case QUPDATE_QSP_DEL:
			if (!remove_quadspace(session, qpage, JQVID, QVID, QSID))
				return 0;
			packet_len = snprintf(packet, 256, "qupdate qsp_del {\"QVID\":\"%d\",\"QSID\":\"%d\"}", QVID, QSID);
			break;
		case QUPDATE_WS_DEL:
			if (!remove_workspace(session, qpage, JQVID, QVID, QSID, QID, WSID))
				return 0;
			packet_len = snprintf(packet, 256, "qupdate ws_del {\"QVID\":\"%d\",\"QSID\":\"%d\",\"QID\":\"%d\",\"WSID\":\"%d\"}", QVID, QSID, QID, WSID);
			break;
		case QUPDATE_DEL_CHART:
			ticker = args;
			if (!remove_chart(session, ticker, QVID, QSID, QID, WSID))
				return 0;
			qcache_remove_chart(session, qpage, ticker, JQVID, QSID, QID, WSID);
			packet_len = snprintf(packet, 128, "qupdate del_chart {\"QGID\":\"%s\",\"ticker\":\"%s\"}", QGID, ticker);
			break;
		case QUPDATE_DEL_IMG:
			if (!remove_object(session, URL))
				return 0;
			qcache_remove_object(session, qpage, "img", URL, JQVID, QSID, QID, WSID);
			packet_len = snprintf(packet, 256, "qupdate del_blob {\"QGID\":\"%s\",\"URL\":\"%s\",\"type\":\"img\"}", QGID, URL);
			break;
		case QUPDATE_DEL_WSTAB:
			qcache_remove_wstab(session, qpage, "wstab", URL, JQVID, QSID, QID, WSID);
			packet_len = snprintf(packet, 256, "qupdate del_wstab {\"QGID\":\"%s\",\"TID\":\"%s\"}", QGID, URL);
			break;
		case QUPDATE_ADD_WSTAB:
			if (!watchtable_create(session, URL, NULL))
				return 0;
			printf("qupdate add watchtable\n");
			qcache_add_object2(session, qpage, "wstab", URL, JQVID, QSID, QID, WSID);
			packet_len = snprintf(packet, 256, "qupdate add_wstab {\"QGID\":\"%s\",\"TID\":\"%s\"}", QGID, URL);
			break;
	}
	printf("QUPDATE: %s\n", packet);
	return (packet_len);
}

/**
 * RPC: Quadverse Update
 *
 * operation: {int}
 * QVID:      {int}
 * QSID:      {int}
 * QID:       {int}
 * WSID:      {int}
 * args:      {char *}
 */
void rpc_quadverse_update(struct rpc *rpc)
{
	struct session    *session   = rpc->session;
	struct connection    *connection   = rpc->connection;
	char              *packet    = rpc->packet;
	int                operation = atoi(rpc->argv[1]);
	int                QVID      = atoi(rpc->argv[2]);
	int                QSID      = atoi(rpc->argv[3]);
	int                QID       = atoi(rpc->argv[4]);
	int                WSID      = atoi(rpc->argv[5]);
	char              *args      = rpc->argv[6];
	struct qpage      *qpage;
	struct quadverse  *quadverse;
	struct session    *qsession;
	struct subscriber *qsubscriber;
	int x, packet_len, JSON_QVID = 0;

	if (QVID < 0 || QVID >= MAX_QUADVERSES || QSID >= MAX_QUADSPACES || QSID < 0 || !(quadverse=session->quadverse[QVID]))
		return;
	if (QID  < 0 || QID  >= MAX_QUADS      || WSID >= MAX_WORKSPACES || ((WSID < 0) && (operation != QUPDATE_WS_NAME))) 
		return;

	/* PRIVATE QuadVerses */
	qpage = quadverse->qpage;
	if (!qpage) {
		packet_len = quadverse_update_packet(session, NULL, packet, operation, QVID, QVID, QSID, QID, WSID, args);
		if (packet_len)
			websockets_sendall_except(session, connection, packet, packet_len);
		return;
	}

	if (!qpage_is_owner(session->user->uid, qpage))
		return;

	/* PUBLIC QuadVerses stockminer.org/myQuadverse Pages */
	for (x=0; x<qpage->nr_subscribers; x++) {
		qsubscriber = qpage->subscribers[x];
		if (!qsubscriber)
			continue;
		qsession = qsubscriber->session;
//		if (qsubscriber == session)
//			continue;
		quadverse   = qpage_quadverse(qsession, qpage, &QVID);
		if (!quadverse)
			continue;
		packet_len = quadverse_update_packet(session, qpage, packet, operation, JSON_QVID, QVID, QSID, QID, WSID, args);
		if (!packet_len)
			continue;
		printf("quadverse_update_live: %s\n", packet);
		websockets_sendall_except(qsession, connection, packet, packet_len);
	}
}

/* unused */
void qpage_send_subscribers(struct qpage *qpage, char *packet, int packet_len)
{
	int x;

	for (x=0; x<qpage->nr_subscribers; x++) {
		struct session *qsubscriber = qpage->subscribers[x]->session;
		websockets_sendall(qsubscriber, packet, packet_len);
	}
}

struct qpage *quadverse_owner(struct session *session, char *url)
{
	struct qpage *qpage;
	int x, owner = 0;

	mutex_lock(&qpage_lock);
	for (x=0; x<nr_quadverse_pages; x++) {
		qpage = quadverse_pages[x];
		if (!strcmp(qpage->url, url) && qpage_is_owner(session->user->uid, qpage))
			owner = 1;
			break;
	}
	mutex_unlock(&qpage_lock);
	if (owner)
		return qpage;
	return (NULL);
}

void quadverse_unsubscribe(struct session *session)
{
	struct qpage *qpage;
	struct quadverse *quadverse;
	int x, y;

	for (x=0; x<MAX_QUADVERSES; x++) {
		quadverse = session->quadverse[x];
		if (!quadverse || !(qpage=quadverse->qpage))
			continue;
		for (y=0; y<qpage->nr_subscribers; y++) {
			if (qpage->subscribers[y]->session == session) {
				if (y == (qpage->nr_subscribers-1))
					qpage->subscribers[y] = NULL;
				else {
					memmove(&qpage->subscribers[y+1], &qpage->subscribers[y], (qpage->nr_subscribers-y-1)*sizeof(void *));
					qpage->subscribers[qpage->nr_subscribers-1] = NULL;
				}
				qpage->nr_subscribers--;
				printf("unsubscribed session: %p\n", session);
				break;
			}
		}
	}
}

void quadverse_subscribe(struct session *session, struct connection *connection, struct qpage *qpage, int QVID)
{
	struct qlive      *qlive;
	struct subscriber *qsub;
	int x, nr_subscribers, nr_subs = 0;

	if (!qpage) {
		printf(BOLDRED "quadveres_subscribe critical error: session: %p uname: %s" RESET "\n", session, session->user->uname);
		return;
	}

	qlive           = (struct qlive      *)zmalloc(sizeof(*qlive));
	qsub            = (struct subscriber *)zmalloc(sizeof(*qsub));
	qsub->session   = session;
	qsub->client_id = connection->websocket_id;
	qsub->QVID      = QVID;

	mutex_lock(&qpage->qpage_lock);
	nr_subscribers = qpage->nr_subscribers;
	for (x=0; x<nr_subscribers; x++) {
		if (qpage->subscribers[x]->session == session)
			nr_subs++;
	}
	if (nr_subs >= 4) {
		free(qsub);
		return;
	}
	if ((nr_subscribers+1) >= qpage->max_subscribers) {
		qpage->subscribers     = (struct subscriber **)realloc(qpage->subscribers, nr_subscribers+MAX_SUBSCRIBERS);
		qpage->max_subscribers = nr_subscribers+MAX_SUBSCRIBERS;
	}
	qpage->subscribers[qpage->nr_subscribers++] = qsub;

	qlive->key = connection->fd;
	HASH_ADD_INT(qpage->quadverse->current_quadspace_live, key, qlive);
	mutex_unlock(&qpage->qpage_lock);
}

void qpage_set_quadverse(struct session *session, struct qpage *qpage)
{
	int x;
	for (x=0; x<MAX_QUADVERSES; x++) {
		if (!session->quadverse[x]) {
			session->quadverse[x] = qpage->quadverse;
			return;
		}
	}
}

struct qpage *qpage_alloc()
{
	struct qpage *qpage = (struct qpage       *)zmalloc(sizeof(*qpage));
	qpage->subscribers  = (struct subscriber **)zmalloc(sizeof(struct subscriber *) * MAX_SUBSCRIBERS);
	return (qpage);
}

int qpage_packet(struct session *session, char *packet)
{
	struct qpage *qpage;
	char *username;
	int packet_len, nr_qpages, x;

	nr_qpages = session->nr_qpages;
	strcpy(packet, "qpage ");
	packet_len = 6;
	for (x=0; x<nr_qpages; x++) {
		qpage = session->qpages[x];
		if (!qpage)
			continue;
		username = qpage_username(qpage);
		if (!username)
			username = "none";
		packet_len += sprintf(packet+packet_len, "%s:%s:%c ", username, qpage->url, qpage->background_image);
	}
	if (packet_len == 6)
		return 0;
	packet[packet_len-1] = '@';
	return (packet_len);
}

void rpc_quadverse_export(struct rpc *rpc)
{
	struct session *session      =      rpc->session;
	char           *url          =      rpc->argv[1];
	char            qpage_bg_img =     *rpc->argv[2];
	int             QVID         = atoi(rpc->argv[3]);
	int             QSID         = atoi(rpc->argv[4]);
	yyjson_mut_doc *qcache       = session->qcache;
	yyjson_mut_val *qcache_root, *qcache_oldroot;
	struct qpage   *qpage;
	char            path[256];
	char           *qcache_json;
	int             qcache_json_size, packet_len, reload = (qpage_bg_img=='0'?0:1);

	if (!session->user || !session->user->logged_in || session->nr_quadverses >= MAX_QUADVERSES || session->nr_qpages >= 16 || !qcache)
		return;

	qpage = qpage_alloc();
	if (!qpage)
		return;

	qcache_oldroot = qcache->root;
	sprintf(path, "/%d/%d", QVID, QSID);
	qcache_root = yyjson_mut_doc_get_pointer(qcache, path);
	if (!qcache_root)
		return;
	yyjson_mut_doc_set_root(session->qcache, qcache_root);
	qcache_json = (char *)yyjson_mut_write(qcache, 0, NULL);
	if (!qcache_json)
		return;

	yyjson_mut_doc_set_root(qcache, qcache_oldroot);
	qpage->qcache_json[0] = '[';
	qpage->qcache_json[1] = '[';
	qcache_json_size      = strlen(qcache_json);
	sprintf(path, QPAGE_PATH, session->user->uid);
	strncpy(qpage->qcache_json+2, qcache_json, sizeof(qpage->qcache_json)-6);
	strncpy(qpage->url, url, sizeof(qpage->url)-1);
	qpage->qcache_json[qcache_json_size+2] = ']';
	qpage->qcache_json[qcache_json_size+3] = ']';
	qpage->qcache_json[qcache_json_size+4] = '\0';
	qpage->quadverse        = new_quadverse();
	qpage->nr_owners        = 1;
	qpage->owners[0].uid    = session->user->uid;
	qpage->usize            = strlen(url);
	qpage->qsize            = qcache_json_size+4;
	qpage->quadverse->qpage = qpage;
	qpage->background_image = qpage_bg_img;
	if (!qpage_load_qcache(qpage))
		return;
	fs_appendfile(path, (char *)qpage, sizeof(*qpage));
	qpage_set_quadverse(session, qpage);

	if (!session->qpages)
		session->qpages = (struct qpage **)malloc(sizeof(struct qpage *) * MAX_QPAGES);
	session->qpages[session->nr_qpages++] = qpage;

	mutex_lock(&qpage_lock);
	quadverse_pages[nr_quadverse_pages++] = qpage;
	mutex_unlock(&qpage_lock);

	if (reload) {
		packet_len = snprintf(rpc->packet, 128, "qreload %s/%s", session->user->uname, qpage->url);
		printf("qreload: %s\n", rpc->packet);
		websocket_send(rpc->connection, rpc->packet, packet_len);
	}
}

void session_new_quadspace(struct session *session, struct connection *connection, struct qpage *qpage, int QSID)
{
	char packet[64];
	int QVID, packet_len, broadcast = 0;

	QVID = session->current_quadverse[connection->websocket_id];
	printf(BOLDWHITE "session_new_quadspace: QVID: %d QSID: %d" RESET "\n", QVID, QSID);
	/* No custom modifications for the first 6 QuadSpaces of QVID 0 OR on QVID 1,2,5 which have predefined functions */
	if ((QVID == 0 && QSID < 5) || QVID == 1 || QVID == 2 || QVID == 5)
		return;

	packet_len = snprintf(packet, sizeof(packet)-1, "qspace %d", QVID);
	session->quadverse[QVID]->quadspace[QSID] = new_quadspace();

	if (!session->qcache) {
		printf(BOLDCYAN "session_new_quadspace: CREATING QCACHE %d:%d" RESET "\n", connection->websocket_id, connection->fd);
		session->qcache = qcache_create(session, QVID, QSID, NULL, 0);
		broadcast = 1;
	} else {
		printf(BOLDYELLOW "session_new_quadspace: qcache new quadspace QVID: %d QSID: %d broadcast: %d %d:%d" RESET "\n", QVID, QSID, broadcast, connection->websocket_id, connection->fd);
		fs_printfile("db/uid/0/qcache");
		qcache_new_quadspace(session, qpage, QVID, QSID, &broadcast, NULL, 0);
	}

	if (!broadcast)
		return;
	workspace_broadcast(session, connection, packet, packet_len);
}

struct workspace *session_new_workspace(struct session *session, struct connection *connection, struct qpage *qpage, struct quad *quad, int QVID, int QSID, int QID, int WSID)
{
	struct workspace *workspace;
	char              packet[64];
	int               packet_len;

	if (!session->qcache)
		return NULL;
	packet_len = snprintf(packet, sizeof(packet)-1, "wspace %d %d %d", QVID, QSID, QID);
	if (!(workspace=new_workspace()))
		return NULL;
	quad->workspace[WSID] = workspace;
	qcache_new_workspace(session, qpage, "NewTab", QVID, QSID, QID, WSID);
	qcache_save(session, qpage, QVID, QSID);
	workspace_broadcast(session, connection, packet, packet_len);
	return workspace;
}

static void quadverse_switch(struct session *session, struct connection *connection, char *QVID)
{
	struct quadverse *quadverse;
	uint16_t          current_quadverse;
	uint16_t          websocket_id = connection->websocket_id;

	current_quadverse = strtoul(QVID, NULL, 10);
	printf("current quadverse: %d\n", current_quadverse);
	if (current_quadverse >= MAX_QUADVERSES)
		return;
	if (!(quadverse=session->quadverse[current_quadverse])) {
		session->quadverse[current_quadverse]  = quadverse = new_quadverse();
		session->nr_quadverses++;
	}
	session->current_quadverse[websocket_id]   = current_quadverse;
//	if (!session->qcache && current_quadverse != QUADVERSE_ALGO)
	if (!session->qcache)
		session->qcache = qcache_create(session, current_quadverse, 0, NULL, 0);
}

static void quadspace_switch(struct session *session, struct connection *connection, char *QGID)
{
	struct quadverse *quadverse;
	uint16_t          current_quadverse, current_quadspace;
	uint16_t          websocket_id = connection->websocket_id;

	printf("quadverse_switch QGID: %s\n", QGID);
	if ((current_quadverse = strtoul(QGID+1, NULL, 10)) >= MAX_QUADVERSES)
		return;

	if (!(quadverse=session->quadverse[current_quadverse])) {
		session->quadverse[current_quadverse] = quadverse = new_quadverse();
		session->nr_quadverses++;
	}

	char *p = QGID+2;
	while (*p != 'Q') {
		p++;
		if (*p == '\0')
			return;
	}
	current_quadspace = strtoul(p+1, NULL, 10);
	if (current_quadspace >= MAX_QUADSPACES)
		return;
	if (!quadverse->quadspace[current_quadspace])
		session_new_quadspace(session, connection, quadverse->qpage, current_quadspace); // creates session->qcache if empty
	quadverse->current_quadspace[websocket_id] = current_quadspace;
	session->current_quadverse[websocket_id]   = current_quadverse;
}

static void workspace_switch(struct session *session, struct connection *connection, char *QGID)
{
	struct quadverse  *quadverse;
	struct quadspace  *quadspace;
	struct workspace  *workspace;
	struct quad       *quad;
	uint16_t           current_quadverse, current_quadspace, current_workspace;
	uint16_t           QID, websocket_id = connection->websocket_id;

	// set workspace
	printf("workspace_switch: QGID: %s\n", QGID);
	current_quadverse = session->current_quadverse[websocket_id];
	if (!(quadverse=session->quadverse[current_quadverse]))
		return;
	current_quadspace = quadverse->current_quadspace[websocket_id];
	if (current_quadspace >= MAX_QUADSPACES)
		return;
	if (!(quadspace=quadverse->quadspace[current_quadspace]))
		return;
	QID = strtoul(QGID, NULL, 10);
	if (QID >= MAX_QUADS)
		return;
	quad = quadspace->quad[QID];
	if (!quad)
		return;
	if (*(QGID+1) != ' ')
		QGID++;
	current_workspace = strtoul(QGID+2, NULL, 10);

	if (current_workspace >= MAX_WORKSPACES)
		return;
//	if (current_workspace >= MAX_WORKSPACES || current_quadverse == QUADVERSE_ALGO)
//		return;
	if (!(workspace=quad->workspace[current_workspace])) {
		printf("calling session_new workspace: websocket_id: %d\n", websocket_id);
		workspace = session_new_workspace(session, connection, quadverse->qpage, quad, current_quadverse, current_quadspace, QID, current_workspace);
		if (!workspace)
			return;
	}
	printf("[set workspace]: current_workspace: %d websocket_id: %d\n", current_workspace, websocket_id);
	quad->current_workspace[websocket_id] = current_workspace;
	process_workspace(session, connection->packet, workspace, current_quadverse, current_quadspace, QID, current_workspace);
}

/*
 * Tripple use RPC for handling onclick events for switching between:
 *   - QuadVerses (from the drop down menu)
 *   - QuadSpaces (by clicking on a Quadspace ChromeTab)
 *   - WorkSpaces (by clicking on a Workspace ChromeTab)
 */
void rpc_qswitch(struct rpc *rpc)
{
	struct session    *session    =  rpc->session;
	struct connection *connection =  rpc->connection;
	char               qswitch    = *rpc->argv[1];
	char              *QGID       =  rpc->argv[2];
	char              *p;
	int                current_quadverse, current_quadspace, QID, current_workspace, websocket_id = connection->websocket_id;

	// set quadverse
	switch (qswitch) {
		case RPC_QUADVERSE_SWITCH:
			printf("set_quadverse qswitch: %s\n", QGID);
			quadverse_switch(session, connection, QGID);
			break;
		case RPC_QUADSPACE_SWITCH:
			quadspace_switch(session, connection, QGID);
			break;
		case RPC_WORKSPACE_SWITCH:
			workspace_switch(session, connection, QGID);
			break;
	}
}

#define MAX_BOOTPARAM_OBJECTS 4

/*
 * GET /ws/action/objtype/filetype/filesize/filename/arg1/arg2
 * GET /ws/ACTION_BOOT/bootstring
 * GET /ws/ACTION_RELOAD
 */
bool rpc_boot(struct rpc *rpc)
{
	struct session   *session = rpc->session;
	char             *packet  = rpc->connection->packet+rpc->connection->packet_size;
	char             *request = rpc->argv[1];
	struct XLS       *XLS     = CURRENT_XLS;
	struct stock     *stock;
	struct quadverse *quadverse;
	struct quadspace *quadspace;
	struct quad      *quad;
	struct workspace *workspace;
	char             *argv  [MAX_BOOT_OBJECTS];
	char             *pargv [MAX_WORKSPACE_OBJECTS];
	char             *params[MAX_BOOTPARAM_OBJECTS];
	char             *p = request, *p2;
	uint64_t          action, argc, packet_size = 0;
	uint16_t          quadverse_profile_index;

	if (stockdata_checkpoint != SD_CHECKPOINT_COMPLETE)
		return true;

	action = strtoul(request, NULL, 10);
	if (action >= MAX_WEBSOCKET_ACTIONS)
		return false;

	while (*p != '/') {
		p++;
		if (*p == '\0')
			return false;
	}
	p++;
	p2 = p;
	while (*p2 != '\r') {
		p2++;
		if (*p2 == '\0')
			return false;
	}
	*p2 = 0;

	argc = cstring_split(p, argv, MAX_BOOT_OBJECTS, '/');
	if (!argc)
		return false;

	if (!(quadverse=session->quadverse[0])) {
		session->quadverse[0] = quadverse = new_quadverse();
		session->nr_quadverses++;
	}

	for (int x = 0; x<argc; x++) {
		int      nr_params  = cstring_split(argv[x],   params, MAX_BOOTPARAM_OBJECTS, ':');
		int      nr_objects = cstring_split(params[1], pargv,  MAX_WORKSPACE_OBJECTS, ',');
		char    *obj_id     = params[1]; // one or more objects of type: chart/table/etc
		char    *QGID       = params[2];
		uint16_t QID, WSID; // Quad ID & Workspace ID (QVID & QSID are always 0 on boot)

		if (!(p=strchr(QGID+1, 'q')))
			return false;
		QID = strtoul(p+1, NULL, 10);
		if (!(p=strchr(p+1, 'w')))
			return false;
		WSID = strtoul(p+2, NULL, 10);

		if (QID >= MAX_QUADS || WSID >= MAX_WORKSPACES)
			return false;

		if (!(quadspace=quadverse->quadspace[0])) {
			quadverse->quadspace[0] = quadspace = new_quadspace();
			quadverse->nr_quadspaces++;
		}

		if (!(quad=quadspace->quad[QID])) {
			quadspace->quad[QID] = quad = new_quad();
			quadspace->nr_quads++;
		}

		if (!(workspace=quad->workspace[WSID])) {
			quad->workspace[WSID] = workspace = new_workspace();
			quad->nr_workspaces++;
		}

		if (!nr_params || !nr_objects)
			return false;

		if (!strncmp(params[0], "addTable", 8)) {
			/****************
			 *     Tables
			 ***************/
			for (int y = 0; y<nr_objects; y++)
				packet_size += ufo_table_json(pargv[y], packet+packet_size);
			/***********************************************************/
		} else if (!strncmp(params[0], "addChart", 8)) {
			/****************
			 *     Charts
			 ***************/
			char *cargv[6] = {0};

			if (nr_params != 4)
				continue;

			// pick a "random" stock if a '?' is supplied instead of the ticker
			if (*obj_id == '?')
				stock = XLS->boards[LOWCAP_GAINER_BOARD]->stocks[0];
			else
				stock = search_stocks(obj_id);

			if (!stock)
				continue;

			cargv[1]        = QGID;
			cargv[2]        = stock->sym;
			cargv[5]        = params[3]; // class name of an existing <div> where the chart will reside (meaning it was created already rather than by the rpc_chart() js handler
			rpc->packet     = packet+packet_size;
			rpc->argv       = cargv;
			rpc->internal   = true;
			/*
			 * rpc_chart() is called internally to pack chart config JSON(s) into connection->packet
			 * it will also create quadspaces/workspaces necessary to hold those charts
			 */
			rpc_chart(rpc);

			packet_size += rpc->packet_size;
			*(packet+packet_size++) = '@';
//			printf("params: %s packet_size: %lld\n", params[3], packet_size);
		} else if (!strncmp(params[0], "addProfile", 10)) {
			if (!params[1])
				return false;
			quadverse_profile_index = strtoul(params[1], NULL, 10);
			if (quadverse_profile_index >= MAX_QUADVERSES)
				return false;
			if (!(quadverse=session->quadverse[quadverse_profile_index]))
				quadverse     = new_quadverse();
			quadverse->flags |= QUADVERSE_PROFILE;
			quadverse->nr_quadspaces = 1;
			session->nr_quadverses++;
		}
//		printf("argv: %s params: %d\n", argv[x], nr_params);
	}
	rpc->connection->packet_size = packet_size;
	return true;
}



void session_load_quadverse(struct session *session)
{
	struct quadverse *quadverse;
	struct quadspace *quadspace;
	struct quad      *quad;
	struct workspace *workspace;

//	session->nr_quadverses     = 6;
	/* QVID 0: STOCKS QUADVERSE */
//	memset(&session->quadverse[0],      0, sizeof(struct quadverse *) * MAX_QUADVERSES);
//	memset(&session->current_quadverse, 0, sizeof(char)*MAX_WEBSOCKETS);

//	session->quadverse[0]      = quadverse = new_quadverse();
/*	quadverse->nr_quadspaces   = 4;
	quadverse->quadspace[0]    = quadspace = new_quadspace(); // Stocks
	quadverse->quadspace[1]    = new_quadspace();             // Screener
	quadverse->quadspace[2]    = new_quadspace();             // Price Action
	quadverse->quadspace[3]    = new_quadspace();             // Volume
	quadverse->quadspace[4]    = new_quadspace();             // Candles

	// QVID 0: Gainer Tables
	quadspace->quad[0]         = quad = new_quad();           // Gainer Tables
	quad->workspace[0]         = workspace = new_workspace(); // Lowcaps
	workspace->watchtables     = lowcap_tables;
	quad->nr_workspaces        = 2;
	workspace->nr_watchtables  = 2;
	quad->workspace[1]         = workspace = new_workspace(); // Highcaps
	workspace->watchtables     = highcap_tables;
	workspace->nr_watchtables  = 2;*/

	// QVID 0: Main Chart
/*	quadspace->quad[1]         = quad      = new_quad();      // Main Chart
	quadspace->nr_quads        = 2;
	quad->workspace[0]         = workspace = new_workspace();
	quad->nr_workspaces        = 1;
//	workspace->nr_charts       = 1;

	// QVID 0: QSID 1
	quadverse->quadspace[1]->quad[2] = quad = new_quad();            // morphtab
	quad->workspace[0]               = workspace = new_workspace();  // morphtab
	workspace->watchlists[0]         = session->morphtab;
	workspace->nr_watchtables        = 1;

	// QVID 1: OPTIONS QUADVERSE
/*	session->quadverse[1]      = quadverse = new_quadverse();
	quadverse->nr_quadspaces   = 2;
	quadverse->quadspace[0]    = quadspace = new_quadspace(); // Options
	quadverse->quadspace[1]    = new_quadspace();             // Screener

	// QVID 2: CRYPTO QUADVERSE
	session->quadverse[2]      = quadverse = new_quadverse();
	quadverse->quadspace[0]    = quadspace = new_quadspace(); // Crypto
	quadverse->nr_quadspaces   = 1;*/

	// QVID 3: CHARTSPACE QUADVERSE
/*	session->quadverse[3]      = quadverse = new_quadverse();
	quadverse->nr_quadspaces   = 1;
	quadspace->quad[0]         = quad = new_quad();
	quad->workspace[0]         = workspace = new_workspace();
	quad->nr_workspaces        = 1;*/

	// QVID 4: PROFILE QUADVERSE
/*	session->quadverse[4]      = quadverse = new_quadverse();
	quadverse->flags          |= QUADVERSE_PROFILE;
	quadverse->nr_quadspaces   = 1;

	// QVID 5: MONSTER
	session->quadverse[5]      = quadverse = new_quadverse();
	quadverse->quadspace[0]    = quadspace = new_quadspace();
	quadspace->quad[0]         = quad      = new_quad();
	quad->workspace[0]         = new_workspace();
	quad->workspace[1]         = workspace = new_workspace();
	quad->workspace[2]         = new_workspace();
	quad->workspace[3]         = new_workspace();
	quad->workspace[4]         = new_workspace();
	quad->workspace[5]         = new_workspace();
	quad->workspace[6]         = new_workspace();
	workspace->watchtables     = peakwatch_tables;
	workspace->workarea        = (struct peakwatch_workarea *)zmalloc(sizeof(struct peakwatch_workarea));*/

}
