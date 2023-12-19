#include <conf.h>
#include <extern.h>

#define BACKTEST_INVESTMENT     10000.0
#define BACKTEST_INVESTMENT_ESP  1000.0

struct backtest  esp_passive_backtest;
struct backtest  gspc_backtest;
struct backtest  ixic_backtest;
struct backtest  rut_backtest;
struct backtest  dji_backtest;
struct backtest  ultra_backtest;
struct backtest  agro_backtest;
struct backtest  moderate_backtest;
struct backtest *backtest_list[4096];

struct port      port_ultra;
struct portdb    ultra_portdb;
struct backdb    ultra_backdb;

struct port      port_ultra;
struct backdb    ultra_backdb;
mutex_t          port_lock;
DLIST_HEAD(port_list);

#define NR_POSITIONS  1
#define NR_SECTORS    11
#define PROFIT_TARGET 5.0
#define MAX_RANKS     100

int BACKTEST_INITIALIZED;

void backtest_long      (char *ticker, struct backtest *backtest);
int  backtest_linechart (char **json_out, char *buf, int nr_entries, double *sum);
void session_load_ports (struct session *session);

#define ITABLE   "<table class=UTAB>"   \
				 "<caption>Current Positions</caption>" \
				 "<thead>"              \
					"<th>Ticker</th>"   \
					"<th>Date</th>"     \
					"<th>Rank</th>"     \
					"<th>%Chg</th>"     \
					"<th>Industry</th>" \
					"<th>Sector</th>"   \
				"</thead>"              \
				"<tbody>"               \

char *ITAB_ENTRY =  "<tr>"
					"<td>%s</td>"
					"<td>%s</td>"
					"<td>%d</td>"
					"<td>%.2f</td>"
					"<td>%s</td>"
					"<td>%s</td>"
					"</tr>";

#define HTABLE      "<table class=UTAB style=width:100%;height:100%>" \
					"<caption>Current and Closed Positions</caption>" \
					"<thead>"                    \
					"<th>Ticker</th>"            \
					"<th>Date</th>"              \
					"<th>Rank</th>"              \
					"<th>%Chg</th>"              \
					"<th>#days</th>"             \
					"<th>1d</th>"                \
					"<th>2d</th>"                \
					"<th>3d</th>"                \
					"<th>4d</th>"                \
					"<th>5d</th>"                \
					"<th>6d</th>"                \
					"<th>7d</th>"                \
					"<th>8d</th>"                \
					"<th>9d</th>"                \
					"<th>10d</th>"               \
					"<th>11d</th>"               \
					"<th>12d</th>"               \
					"</thead>"                   \
					"<tbody>"                    \

char *HTAB_ENTRY =  "<tr>"
					"<td>%s</td>"
					"<td>%s</td>"
					"<td>%d</td>"
					"<td %s</td>"
					"<td>%s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"<td %s</td>"
					"</tr>";

#define PORT_DIV "<div id=%s class=portfolio onclick=Q.port_load()>" \
				 	"<div class=minichart id=%s-chart></div>"    \
				 	"<div class=port-mini-title>%s</div>"          \
				 	"<div class=port-mini-desc>%s</div>"           \
				 	"<span %s</span>"                              \
				 "</div>"                                          \

char *sector_colors[] = { "green", "#F066AC", "#FE3739", "orange", "yellow", "#821eda", "dodgerblue", "#00001a", "black", "brown", "#196e89" };
char *esp_stocks[]    = { "RNG", "PAYC", "SHOP", "COUP", "HUBS", "PCTY", "PODD", "EPAM", "NVCR", "MA", "FIVN" };

struct sector {
	char    name[96];
	char   *color;
	double  total;
	int     count;
};

char *daystr(double value, char *buf)
{
	char *color;

	if (value == 0)
		return ">";
	if (value > 0)
		sprintf(buf, "class=%s>+%.2f%%", "g", value);
	else
		sprintf(buf, "class=%s>-%.2f%%", "r", value);
	return (buf);
}
/*
void sort_esp_rank(struct esp_signal **esp_signals, int nr_esp_signals, int current_month)
{
	struct esp_signal *esp, *prev_esp;
	uint64_t key;
	int i, j, x;

	for (x=0; x<nr_esp_signals-1; x++) {
		for (i = 1; i < nr_esp_signals; i++) {
			esp = esp_signals[i];
			key = esp->stock->ranks[current_month];
			j = i - 1;
			prev_esp = esp_signals[j];
			while (j >= 0 && prev_esp->stock->ranks[current_month] > key) {
				esp_signals[j+1] = esp_signals[j];
				j = j - 1;
			}
			esp_signals[j+1] = esp;
		}
	}
}*/

#define CTRL_JSON "@{\"ultra-capital\":\"%d\",\"backbox-avgdays\":\"%d\",\"backbox-pt\":\"%.2f\",\"backbox-maxpos\":\"%d\"," \
				  "\"backbox-a1q1\":\"%d\",\"backbox-a4q1\":\"%d\",\"backbox-select-capital\":\"%d\",\"backbox-select-avgdays\":\"%d\"," \
				  "\"backbox-select-pt\":\"%d\",\"backbox-select-maxpos\":\"%d\",\"backbox-select-a1q1\":\"%d\",\"backbox-select-a4q1\":\"%d\",\"rank\":\"%d\",\"peak\":\"%d\"}@" \

double backtest_avgdays(struct backtest *backtest)
{
	struct position *position;
	int x, nr_days = 0, nr_positions = backtest->nr_positions;

	for (x=0; x<nr_positions; x++) {
		position = &backtest->positions[x];
		nr_days += position->nr_days;
	}
	return (nr_days/(backtest->nr_positions));
}

void backlist_table(struct port *port)
{
	int json_len, nbytes, nr_backtests, x;
	char buf[256];
	char *jptr = port->backlist_table;

	strcpy(jptr, BTABLE);
	jptr += sizeof(BTABLE)-1;
	json_len = sizeof(BTABLE)-1;

	nr_backtests = port->nr_backtests;
	for (x=0; x<nr_backtests; x++) {
		struct backjson *bj = port->backtest[x];
		nbytes = sprintf(jptr, BTABLE_ENTRY, (char *)&bj->gid, bj->name, daystr(bj->gain, buf), bj->avgdays_pos);
		printf("BJ ID: %s bjname: %s uid: %x\n", (char *)&bj->gid, bj->name, bj->uid);
		jptr  += nbytes;
		json_len += nbytes;
	}
	strcpy(jptr, "</tbody></table>");
	json_len += 16;
	port->backlist_table_len = json_len;
}

void build_port_json(struct session *session, struct port *port, struct backtest *backtest)
{
	struct position *position;
	struct mag *mag;
	char *jptr;
	char buf[16 KB];
	char n_days[8] = {0};
	int x, y, nr_positions, day_index, nbytes, json_len = 0, nr_days, nr_backtests;
	double current_close, position_close, current_chgpc, day_close, pc_diff, total;
	double chgpc[12];
	uint64_t *backtest_id;
	char cbuf1[32];char cbuf2[32];char cbuf3[32];char cbuf4[32];char cbuf5[32];char cbuf6[32];char cbuf[32];
	char cbuf7[32];char cbuf8[32];char cbuf9[32];char cbuf10[32];char cbuf11[32];char cbuf12[32];

	/* [0] Summary */
	jptr = backtest->json;
	total = backtest->money_available+backtest->money_invested;
	if (!session) {
		backtest->uid = 0;
		backtest_id   =  (uint64_t *)"Default";
		backtest->gid = *(uint64_t *)backtest_id;
//		printf(BOLDCYAN "BUILDING DEFAULT PROFILE" RESET "\n");
	} else {
		random_string_len((char *)&backtest->gid, 8);
		printf(BOLDMAGENTA "NEW BACKTEST ID: %s session: %p" RESET "\n", (char *)&backtest->gid, session);
		if (!session->user->logged_in)
			backtest->uid = -1;
		else
			backtest->uid = session->user->uid;
	}
	backtest->avgdays_pos = backtest_avgdays(backtest);
	if (total > backtest->money_initial)
		json_len = sprintf(jptr, "+%.2f%% %.2f %s @", ((total/backtest->money_initial)-1)*100, total, (char *)&backtest->gid);
	else
		json_len = sprintf(jptr, "-%.2f%% %.2f %s @", ((total/backtest->money_initial)-1)*100, total, (char *)&backtest->gid);

	jptr += json_len;

	/* [1] Line Chart */
	strcpy(jptr, backtest->json_linechart);
	jptr      += backtest->json_linechart_len;
	json_len  += backtest->json_linechart_len;
	*jptr++   = '@';
	json_len += 1;

	/* [2] Investment Table */
	strcpy(jptr, ITABLE);
	jptr      += sizeof(ITABLE)-1;
	json_len  += sizeof(ITABLE)-1;

	nr_positions = backtest->nr_positions;
	for (x=0; x<nr_positions; x++) {
		position = &backtest->positions[x];
		if (position->status == POSITION_CLOSED)
			continue;
		day_index      = position->day_index;
		position_close = position->stock->mag->close[day_index];
		current_close  = position->stock->mag->close[position->esp->stock->mag->nr_entries-1];
//		printf("day index: %d nrent: %d\n", day_index, position->esp->stock->mag->nr_entries-1);
		pc_diff        = ((current_close/position_close)-1)*100;
//		nbytes         = sprintf(jptr, ITAB_ENTRY, position->stock->sym, position->esp->date2, position->stock->ranks[position->month], pc_diff, position->esp->stock->industry, position->esp->stock->sector);
		jptr          += nbytes;
		json_len      += nbytes;
	}
	strcpy(jptr, "</tbody></table>@");
	jptr     += 17;
	json_len += 17;

	/* [3] Transaction History */
	strcpy(jptr, HTABLE);
	jptr      += sizeof(HTABLE)-1;
	json_len  += sizeof(HTABLE)-1;

	for (x=0; x<nr_positions; x++) {
		position       = &backtest->positions[x];
		mag            = position->stock->mag;
		nr_days        = MIN(position->nr_days, 12);
		day_index      = position->day_index;
		position_close = mag->close[day_index];
		memset(chgpc, 0, sizeof(chgpc));
		for (y=0; y<nr_days; y++) {
			day_close  = mag->close[day_index+y+1];
			chgpc[y] = ((day_close/position_close)-1) * 100;
		}
		if (position->status == POSITION_CLOSED) {
			sprintf(n_days, "%d", position->nr_days);
			current_close  = mag->close[position->day_index+position->nr_days];
		} else {
			n_days[0] = '-';
			n_days[1] = 0;
			current_close  = mag->close[mag->nr_entries-1];
		}
//		printf("[%s] (%s) current_close: %.2f position_close: %.2f day_index: %d  nr_days: %d\n", position->esp->date2, position->esp->stock->sym, current_close, position_close, day_index, position->nr_days);
		current_chgpc  = ((current_close/position_close)-1)*100;
/*		nbytes         = sprintf(jptr, HTAB_ENTRY, position->stock->sym, position->esp->date2, position->stock->ranks[position->month], daystr(current_chgpc, cbuf), n_days,
								daystr(chgpc[0], cbuf1),daystr(chgpc[1], cbuf2),daystr(chgpc[2], cbuf3),daystr(chgpc[3], cbuf4),daystr(chgpc[4], cbuf5),daystr(chgpc[5], cbuf6),
								daystr(chgpc[6], cbuf7),daystr(chgpc[7], cbuf8),daystr(chgpc[8], cbuf9),daystr(chgpc[9], cbuf10),daystr(chgpc[10], cbuf11),daystr(chgpc[11], cbuf12));*/
		jptr          += nbytes;
		json_len      += nbytes;
	}
	strcpy(jptr, "</tbody></table>@");
	jptr     += 17;
	json_len += 17;

	/* [4] Sector Pie */
	strcpy(jptr, backtest->sector_json);
	json_len += backtest->sector_json_len;
	jptr     += backtest->sector_json_len;

	/* [5] CTRL JSON */
	nbytes = sprintf(jptr, CTRL_JSON, (int)backtest->money_initial, backtest->avgdays, backtest->pt, backtest->max_positions, backtest->a1q1, backtest->a4q1,
					 backtest->capital_dynamic, backtest->avgdays_year, backtest->pt_dynamic, backtest->maxpos_dynamic, backtest->a1q1_year, backtest->a4q1_year,
					 backtest->max_rank, (int)backtest->peak);
	json_len += nbytes;
	jptr     += nbytes;
	backtest->json_len = json_len;
}

void close_position(struct backtest *backtest, struct position *position)
{
	backtest->nr_open_positions -= 1;
	backtest->money_available   += position->profit;
	backtest->money_invested    -= position->size;
	position->status             = POSITION_CLOSED;
}

int pt_reached(struct position *position)
{
	int current_index = (position->day_index + position->nr_days);
	double prior_close   = position->stock->mag->close[position->day_index];
	double current_close = position->stock->mag->close[current_index];
	double pc_diff;

	pc_diff = (((current_close/prior_close)-1)*100);
	position->profit = position->nr_shares * current_close;
	position->pc_diff = pc_diff;
	if (pc_diff > position->target) {
//		printf("PT REACHED [%s] - [%s] current_close: %.2f prior_close: %.2f pc_diff: +%.2f%%  #days: %d\n",
//		position->stock->mag->date[current_index], position->stock->mag->date[position->day_index],current_close, prior_close, pc_diff, position->nr_days);
		position->gain   = pc_diff;
		return 1;
	}
//	printf(BOLDRED "[date: %s esp->day: %d pos->day_index: %d nrdays: %d]%s prior: %.2f current: %.2f percentage: %.2f date: %s" RESET "\n",
//	position->esp->stock->mag->date[position->esp->day], position->esp->day, position->day_index, position->nr_days, position->stock->sym, prior_close, current_close, pc_diff, position->esp->date2/*position->stock->mag->date[position->nr_days+position->day_index]*/);
	return 0;
}

int backtest_close_positions(struct backtest *backtest)
{
	struct position *position;
	double real_value = 0, new_val = 0;
	int x;

	if (backtest->nr_open_positions == 0) {
		backtest->sum[backtest->current_day] = (backtest->money_invested + backtest->money_available);
		return backtest->max_positions;
	}
	for (x=0; x<backtest->nr_positions; x++) {
		position = &backtest->positions[x];
		if (position->status == POSITION_CLOSED)
			continue;
		if (pt_reached(position)) {
			close_position(backtest, position);
//			printf(BOLDGREEN "POSITION CLOSED: %s (%.2f profit: $%.2f) nr_days: %d idx: %d date: %s Money Available: %.2f (%d)" RESET "\n",
//			position->stock->sym, position->gain, position->profit-position->size,
//			position->nr_days, position->day_index, position->esp->date, backtest->money_available,backtest->max_positions-backtest->nr_open_positions);
			continue;
		}
		real_value += position->profit;
		new_val    += position->size;
		position->nr_days += 1;
	}
	backtest->sum[backtest->current_day] = (backtest->money_invested + backtest->money_available);
//	backtest->sum[backtest->current_day] = (real_value + backtest->money_available);
//	printf("BACKTEST SUM DAY: %d total money: %.2f real: %.2f new: %.2f full: %.2f	\n",
//	backtest->current_day, backtest->sum[backtest->current_day], real_value, new_val, backtest->money_available+backtest->money_invested);
	return (backtest->max_positions-backtest->nr_open_positions);
}

void backtest_open_position(struct backtest *backtest, struct esp_signal *esp)
{
	struct position   position;
	struct stock     *stock = esp->stock;
	struct mag       *mag = stock->mag;
	double amount;
	int x;

	memset(&position, 0, sizeof(position));
	position.stock     = stock;
	position.day_index = esp->day_index;
	position.nr_days   = 1;
	position.target    = backtest->pt;
	position.status    = POSITION_OPEN;
	position.esp       = esp;
	position.month     = backtest->current_month;
	position.size      = backtest->money_available/(backtest->max_positions-backtest->nr_open_positions);
	position.stock_price = mag->close[position.day_index];
	position.date      = *(uint64_t *)(mag->date[position.day_index]);
	position.nr_shares = (position.size/position.stock_price);
//	printf(BOLDWHITE "[esp->day: %d day_index: %d {%.2f} [%s] Opening Position (%s) %.2f shares (total: $%.2f) idx: %d price: %.2f" RESET "\n",esp->day, position.day_index,
//	backtest->money_available, esp->date, stock->sym, position.nr_shares, position.size, position.day_index, position.stock_price);

	position.profit              = 0;
	position.status              = 0;
	position.invested            = backtest->money_available+backtest->money_invested;
	memcpy(&backtest->positions[backtest->nr_positions++], &position, sizeof(position));
	backtest->money_available   -= position.size;
	backtest->money_invested    += position.size;
	backtest->nr_open_positions += 1;
//	printf(BOLDYELLOW "POSITION: %.2f addr: %p" RESET "\n", position.invested, &backtest->positions[backtest->nr_positions-1]);
}

/* Make sure to check if you still have enough funds to buy the stock */
int scan_esp(struct backtest *backtest, struct esp_signal **esp_signals, int current_month)
{
	struct esp_signal *esp, *entry_esp;
	struct stock *stock;
	unsigned short day, target_day, nr_esp_signals, nr_esp = 0, nr_sigs = 0;

	nr_esp_signals = backtest->current_ranks->nr_esp_signals-1;
//	if (backtest->esp_index>= nr_esp_signals)
//		return 0;
	day = backtest->esp_signals[backtest->esp_index]->day;
 	for (; backtest->esp_index<nr_esp_signals-1; backtest->esp_index++) {
		esp = backtest->esp_signals[backtest->esp_index];
		nr_sigs++;
//		printf("%s date: %s esp_index: %d day: %d vs: %d\n", esp->stock->sym, esp->date2, backtest->esp_index, day, esp->day);
//		printf("next delta: %.2f target_day: %d day: %d stock: %s\n", next_esp->delta, next_esp->day, day, next_esp->stock->sym);
		if (day != esp->day)
			break;
		if (!esp->esp)
			continue;
//		if (esp->stock->ranks[current_month] > backtest->max_rank)
//			continue;
		if (esp->delta >= 0 || esp->next_day_low >= 0)
			continue;
		if (esp->next_day_action != NEXT_DAY_ACTION)
			continue;
//		if (esp->peak > backtest->peak)
//			continue;
		entry_esp = esp->next_esp;
		if (!entry_esp)
			continue;
/*		if (entry_esp->action == 1 && entry_esp->a1q1 < backtest->a1q1)
			continue;
		if (entry_esp->action == 4 && entry_esp->a4q1 < backtest->a4q1)
			continue;*/
		if (entry_esp->stock != esp->stock)
			continue;
//		printf(BOLDBLUE "CAUGHT ESP ENTRY: %s [%s] (%s) a1q1: %.2f a4q1: %.2f (%.2f %.2f) action1: %d backtest->a1q1: %d" RESET "\n",
//		esp->stock->sym, entry_esp->stock->sym, entry_esp->date, esp->a1q1, esp->a4q1, entry_esp->a1q1, entry_esp->a4q1, esp->action, backtest->a1q1);
		esp_signals[nr_esp++] = entry_esp;
	}
//	backtest->esp_index += nr_sigs;
	return (nr_esp);
}

struct backjson *get_backtest(struct session *session, uint64_t port_id, uint64_t backtest_id)
{
	struct backjson *bj;
	struct port *port;

	port = get_portfolio(session, port_id);
	if (!port)
		return NULL;

//	mutex_lock(&session->backtest_lock);
	for (int x=0; x<session->nr_ports; x++) {
		bj = port->backtest[x];
		if (bj->gid == backtest_id) {
//			mutex_unlock(&session->backtest_lock);
			return (bj);
		}
	}
//	mutex_unlock(&session->backtest_lock);
	return (NULL);
}

struct port *get_portfolio(struct session *session, uint64_t port_id)
{
	struct port *port;

//	mutex_lock(&session->backtest_lock);
	for (int x=0; x<session->nr_ports; x++) {
		port = session->portlist[x];
		if (!port)
			printf("PORT IS FUCING NULL\n");
		printf("SESSION PORT: %-8s vs: %-8s\n", (char *)&port->port_id, (char *)&port_id);
		if (port->port_id == port_id) {
//			mutex_unlock(&session->backtest_lock);
			return (port);
		}
	}
//	mutex_unlock(&session->backtest_lock);
	return (NULL);
}

void portfolio_save(struct session *session, struct port *port)
{
	

}

void backtest_save(struct session *session, uint64_t port_id, char *name, int do_export)
{
	struct filemap   filemap;
	struct backdb    backdb;
	struct portdb    portdb;
	struct backtest *backtest;
	struct backjson *bj;
	struct port     *port;
	struct portdb   *pdb;
	struct stat      sb;
	char             path[256];
	char             cookie[12];
	char            *map;
	int              fd;

	bj = session->current_backtest;
	if (!bj)
		return;
	port = get_portfolio(session, port_id);
	if (do_export)
		sprintf(path, "db/ports/%.8s.bpub", (char *)&port_id);
	else
		sprintf(path, "db/ports/%.8s.bpriv.%d", (char *)&port_id, session->user->uid);

	memset(&backdb, 0, sizeof(backdb));

	printf(BOLDRED "BACKTEST_SAVE ID: %s current json: %s" RESET "\n", (char *)&bj->gid, session->current_backtest2->json);
	strcpy(bj->name, name);
//	mutex_lock(&session->backtest_lock);
	port->backtest[port->nr_backtests++] = bj;
//	mutex_unlock(&session->backtest_lock);

	build_port_json(session, port, session->current_backtest2);

	strcpy(backdb.main_json, session->current_backtest2->json);
	backdb.main_json_len = session->current_backtest2->json_len;
	backdb.uid  = bj->uid;
	backdb.gid  = bj->gid;
	backdb.gain = bj->gain;
	backdb.avgdays_pos = bj->avgdays_pos;
	strcpy(backdb.name, name);
	backlist_table(port);
	printf("BACKTEST SAVE PORT ID: %s BJID: %s path: %s prior backtests: %d bj: %s backlist len: %d\n", (char *)&port_id, (char *)&bj->gid, path, port->nr_backtests, port->backlist_table, port->backlist_table_len);

	if (stat(path, &sb) == -1) {
		fd = open(path, O_RDWR|O_CREAT|O_APPEND, 0644);
		write(fd, (void *)&ultra_backdb, sizeof(struct backdb));
	} else {
		fd = open(path, O_RDWR|O_CREAT|O_APPEND, 0644);
	}
	write(fd, (void *)&backdb, sizeof(backdb));
	close(fd);

	/* Clone Portfolio if saving a Backtest while not owning the Portfolio */
	if (!session->user->logged_in) {
		snprintf(path, sizeof(path)-1, "db/ports/cookie/%s.port", session->cookie);
	} else {
		snprintf(path, sizeof(path)-1, "db/ports/%d.port", session->user->uid);
	}
	if (stat(path, &sb) == -1) {
		portdb.id            = port_id;
		portdb.gain          = port->gain;
		portdb.nr_backtests  = port->nr_backtests;
		portdb.json_mini_len = port->json_mini_len;
		strcpy(portdb.name, port->name);
		strcpy(portdb.desc, port->desc);
		strcpy(portdb.json_mini, port->json_mini);

		fd = open(path, O_RDWR|O_CREAT|O_APPEND, 0644);
		write(fd, (void *)&portdb, sizeof(portdb));
		close(fd);
	} else {
		sprintf(path, "db/ports/%d.port", session->user->uid);
		map = MAP_FILE_RW(path, &filemap);
		if (map) {
			struct portdb *pdb = (struct portdb *)map;
			pdb->nr_backtests += 1;
			UNMAP_FILE(map, &filemap);
		}
	}
}

void backtest_ultra(struct session *session, uint64_t port_id, struct backtest *backtest, double money_initial, char **stocklist, int nstocks)
{
	struct esp_signal *esp;
	struct position   *position;
	struct backjson   *bj;
	struct port       *port;
	struct esp_signal *esp_list[4096];
	struct stock      *stock;
	char               minichart[256];
	char               port_div[1024];
	char               buf[256];
	struct mag        *mag;
	double             miniport[4];
	double             profit = 0;
	int start_month = 0, custom_backtest = 1, range, day_index, days_in_month, nr_free_positions, nr_esp_signals, x, y, z;

	if (backtest) {
		start_month               = backtest->start_month;
		port                      = get_portfolio(session, port_id);
		backtest->money_initial   = backtest->capital;
		backtest->money_available = backtest->capital;        
		printf("INITIAL: %.2f %d %d pt: %.2f\n", backtest->money_initial, backtest->max_positions, backtest->a1q1, backtest->pt);
	} else {
		backtest                  = &ultra_backtest;
		port                      = &port_ultra;
		backtest->money_initial   = money_initial;
		backtest->money_available = money_initial;
		backtest->max_positions   = NR_POSITIONS;
		backtest->a1q1            = 70;
		backtest->a4q1            = 70;
		backtest->capital         = money_initial;
		backtest->capital_dynamic = 1;
		backtest->maxpos_dynamic  = 0;
		backtest->pt_dynamic      = 0;
		backtest->avgdays_year    = 2;
		backtest->pt              = PROFIT_TARGET;
		backtest->peak            = 0;
		backtest->max_rank        = MAX_RANKS;
		custom_backtest           = 0;
	}

	// printf("START: %d\n", backtest->a1q1);
	/*
	 * ESP Scan/Buy/Sell Loop
	 */
	for (x=start_month; x<current_month; x++) {
		backtest->current_ranks = &rank_tables[x];
		backtest->current_month = x;
		backtest->esp_signals   = rank_tables[x].esp_signals;
		days_in_month           = month_days[x];
		backtest->esp_index     = 0;
//		printf("NR SIGNALS: %d month: %d\n", rank_tables[x].nr_esp_signals, days_in_month);
//		printf(BOLDGREEN "MONTH: %d days_in_month: %d backtest: %p" RESET "\n", x, days_in_month, backtest);
		for (y=0; y<days_in_month; y++) {
			nr_free_positions = backtest_close_positions(backtest);
			nr_esp_signals = scan_esp(backtest, esp_list, x);
			if (!nr_esp_signals) {
				backtest->current_day++;
				continue;
			}
//			sort_esp_rank(esp_list, nr_esp_signals, backtest->current_month);
//			printf("FOUND: %d SIGS\n", nr_esp_signals);
			for (z=0; z<nr_esp_signals; z++) {
				if (z >= nr_free_positions)
					break;
				backtest_open_position(backtest, esp_list[z]);
//				printf(BOLDCYAN "opened position (free positions: %d) z: %d" RESET "\n", backtest->max_positions-backtest->nr_open_positions, z);
			}
			backtest->current_day++;
		}
	}

/*	for (x=0; x<backtest->nr_positions; x++) {
		position = &backtest->positions[x];
		if (position->status == POSITION_CLOSED) {
			printf("**`  %-6s  `** (bought on: **`  %s  `**) DAYS: **`   %-2d  `**   PSIZE: ` $%.2f `   PROFIT: **`   $%.2f   `**\n",
			position->esp->stock->sym, position->esp->date2, position->nr_days, position->size, position->profit-position->size);
			profit += (position->profit-position->size);
		} else
			printf("**`  %-6s  `** (bought on: **`  %s  `**) DAYS: **`   %-2d  `**   PSIZE: ` $%.2f `  [IN PROGRESS]\n",
			position->esp->stock->sym, position->esp->date2, position->nr_days, position->size);
	}*/
	for (x=0; x<backtest->nr_positions; x++) {
		position = &backtest->positions[x];
/*		if (position->status == POSITION_CLOSED) {
			printf("%-6s (bought on: %s) DAYS: %-2d PSIZE: $%.2f PROFIT: $%.2f\n",
			position->esp->stock->sym, position->esp->date2, position->nr_days, position->size, position->profit-position->size);
		} else
			printf("%-6s (bought on: %s) DAYS: %-2d PSIZE: $%.2f [IN PROGRESS]\n",
			position->esp->stock->sym, position->esp->date2, position->nr_days, position->size);*/
	}
//	printf(BOLDGREEN "** TOTAL PROFIT: $%.2f    ** [month: %d] $%.2f maxpos: %d" RESET "\n", profit, start_month, backtest->money_available+backtest->money_invested, backtest->max_positions);
	backtest->json_linechart_len = linechart_positions(&backtest->json_linechart, backtest->nr_positions, &backtest->positions[0]);

	backtest->json = malloc(96 KB);
	backtest_sectors(backtest);
	bj                = malloc(sizeof(*bj));
	bj->gain          = (((backtest->money_available+backtest->money_invested)/backtest->money_initial)-1)*100;
	backtest->gain    = bj->gain;

	build_port_json(session, port, backtest);
	bj->gid           = backtest->gid;
	bj->main_json     = backtest->json;
	bj->main_json_len = backtest->json_len;
	bj->avgdays_pos   = backtest->avgdays_pos;

	if (!custom_backtest) {
		range                     = (backtest->nr_positions*30)/100;
		miniport[0]               = backtest->money_initial;
		miniport[1]               = backtest->positions[range].invested;
		miniport[2]               = backtest->positions[range*2].invested;
		miniport[3]               = backtest->money_available+backtest->money_invested;
		backtest_linechart(NULL, minichart, 4, &miniport[0]);
		port_ultra.backtest[0]    = bj;
		port_ultra.gain           = bj->gain;
		port_ultra.nr_backtests   = 1;
		strcpy((char *)&port_ultra.port_id, "ULTRADI");
		strcpy(port_ultra.name, "Ultra Aggressive");
		strcpy(port_ultra.desc, "Swing Portfolio simulated by Radar Signals");

		/* [ {port_id}, {port_div}, {minichart} ] */
		sprintf(port_div, PORT_DIV, (char *)&port_ultra.port_id, (char *)&port_ultra.port_id, port_ultra.name, port_ultra.desc, daystr(port_ultra.gain, buf));
		port_ultra.json_mini = malloc(JSON_MINI_MAX+1);
		port_ultra.json_mini_len = snprintf(port_ultra.json_mini, JSON_MINI_MAX, "%s@%s@%s", "ULTRADI", port_div, minichart);
		list_add_portfolio(&port_ultra);

		/* Copy Core Portdb */
		ultra_portdb.id           = port_ultra.port_id;
		ultra_portdb.gain         = port_ultra.gain;
		ultra_portdb.nr_backtests = 1;
		strcpy(ultra_portdb.name, port_ultra.name);
		strcpy(ultra_portdb.desc, port_ultra.desc);
		strcpy(ultra_portdb.json_mini, port_ultra.json_mini);
		ultra_portdb.json_mini_len = port_ultra.json_mini_len;

		/* Copy Core Backdb */
		strcpy(ultra_backdb.main_json, bj->main_json);
		ultra_backdb.main_json_len = bj->main_json_len;
		ultra_backdb.gain          = bj->gain;
		ultra_backdb.avgdays_pos   = bj->avgdays_pos;
		ultra_backdb.uid           = 0;
		strcpy((char *)&ultra_backdb.gid, "Default");
		strcpy(ultra_backdb.name, "Default");
	} else {
		session->current_backtest = bj;
		if (!session->user->logged_in)
			bj->uid = -1;
		else
			bj->uid = session->user->uid;
		strcpy(bj->name, "Temp");
		session->current_backtest2 = backtest;
		memcpy(backtest->json+backtest->json_len, port->backlist_table, port->backlist_table_len);
		backtest->json[backtest->json_len+port->backlist_table_len] = 0;
//		printf(BOLDBLUE "DONE NEW BACKTEST JSON: %s ID: %s backtest gid: %s bj: %p strlen: %d len: %d" RESET "\n", backtest->json, (char *)&bj->gid, (char *)&backtest->gid, bj, (int)strlen(backtest->json), backtest->json_len);
		net_send(backtest->http_fd, backtest->json, backtest->json_len+port->backlist_table_len);
	}
}

int add_sector(struct sector *sector, int nr_sectors, char *name, char *color)
{
	int x;

	if (!nr_sectors) {
		strcpy(sector->name, name);
		sector->count = 1;
		sector->color = color;
		return 1;
	}
	for (x=0; x<nr_sectors; x++) {
		if (!strcmp(sector->name, name)) {
			sector->count += 1;
			return (nr_sectors);
		}
		sector++;
	}
	sector->color = color;
	strcpy(sector->name, name);
	sector->count = 1;
	return (nr_sectors+1);
}

void backtest_sectors(struct backtest *backtest)
{
	struct position *position;
	struct sector sectors[NR_SECTORS];
	struct sector *sector;
	char  *tptr;
	int x, nbytes, sector_json_len = 0;
	int biggest_sector = 0, total_sectors = 0, biggest_sector_index = 0, nr_sectors = 0, sector_count;
	int nr_positions, nr_open_positions = 0, max_json_len = 512;

	if (backtest->nr_open_positions == 0) {
		backtest->sector_json = "none";
		backtest->sector_json_len = 4;
		return;
	}
	memset(&sectors[0], 0, sizeof(sectors));
	nr_positions = backtest->nr_positions;
	for (x=0; x<nr_positions; x++) {
		position = &backtest->positions[x];
//		printf("sector: %s stock: %s\n", SECTORS[position->esp->stock->sector_index], position->esp->stock->sym);
		if (position->status == POSITION_CLOSED)
			continue;
		backtest->sectors[position->esp->stock->sector_index]++;
		nr_sectors = add_sector(&sectors[0], nr_sectors, SECTORS[position->esp->stock->sector_index], sector_colors[position->esp->stock->sector_index]);
	}

	for (x=0; x<nr_sectors; x++) {
		sector = &sectors[x];
		sector_count = sector->count;
 		if (sector_count > biggest_sector) {
			biggest_sector_index = x;
			biggest_sector = sector_count;
		}
		total_sectors += sector_count;
	}
	for (x=0; x<nr_sectors; x++) {
		sector = &sectors[x];
		sector->total = (sector->count*100.0)/total_sectors;
	}
	backtest->sector_json = tptr = malloc(max_json_len);
	if (nr_sectors > 1) {
		sector = &sectors[biggest_sector];
		sector_json_len = sprintf(tptr, "[{\"name\":\"%s\",\"y\":%.2f,\"selected\":\"true\",\"color\":\"%s\"},", sector->name, sector->total, sector->color);
		tptr += sector_json_len;
	} else {
		nbytes = sprintf(tptr, "[{\"name\":\"%s\",\"y\":%.2f,\"selected\":\"true\",\"color\":\"%s\"}]", sector->name, sector->total, sector->color);
		backtest->sector_json_len = nbytes;
		return;
	}
	for (x=0; x<nr_sectors; x++) {
		if (x == biggest_sector)
			continue;
		sector = &sectors[x];
		nbytes = sprintf(tptr, "{\"name\":\"%s\",\"y\":%.2f,\"color\":\"%s\"},", sector->name, sector->total, sector->color);
		sector_json_len += nbytes;
//		printf("sector names: %s\n", sector->name);
		if (sector_json_len+64 > max_json_len) {
			max_json_len += 256;
			backtest->sector_json = realloc(backtest->sector_json, max_json_len);
			tptr = backtest->sector_json + sector_json_len;
		}
		tptr += nbytes;
	}
	*(tptr-1) = ']';
	backtest->sector_json_len = sector_json_len;
}

int get_port(struct session *session, uint64_t port_id, int http_fd)
{
	struct port *port;
	struct backjson *core_bj;
	char table[16 KB];
	char *json;
	int table_size, json_len;

	if (!BACKTEST_INITIALIZED)
		return 0;
	port = get_portfolio(session, port_id);
	printf("PORT: %p\n", port);
	if (!port || !port->backtest[0])
		return 0;
	core_bj       = port->backtest[0];
	json          = core_bj->main_json;
	json_len      = core_bj->main_json_len;
	memcpy(json+json_len, port->backlist_table, port->backlist_table_len);
	json[json_len+port->backlist_table_len] = 0;
	printf("backlist_table: %s full json: %s table_len: %d total len: %d strlen: %lu\n", port->backlist_table, json, port->backlist_table_len, port->backlist_table_len+json_len, strlen(json));
	net_send(http_fd, json, json_len+port->backlist_table_len);
	return 1;
}

/*
 *
 * BACKTEST LONG
 *
 */
void init_backtest()
{
	struct stock     *stock;
	struct stockfund *stockfund;
	struct backtest  *backtest;
	struct mag       *mag;
	struct session   *session;
	struct list_head *session_list;
	int year_2023, nr_entries, x, y;
	double diff, close_price, prior_close;

	backtest_ultra(NULL, 0, NULL, 10000, NULL, 0);

	backtest_long("^GSPC", &gspc_backtest);
	backtest_long("^IXIC", &ixic_backtest);
	backtest_long("^RUT",  &rut_backtest);
	backtest_long("^DJI",  &dji_backtest);

	/* Passive ESP */
	backtest = &esp_passive_backtest;
	for (x=0; x<10; x++) {
		stock                     = search_stocks(esp_stocks[x]);
		mag                       = stock->mag;
		stockfund                 = &backtest->stockfunds[x];
		stockfund->stock          = stock;
		year_2023                 = mag->year_2023;
		nr_entries                = mag->nr_entries-year_2023;
		backtest->money_invested  = BACKTEST_INVESTMENT_ESP;
		stockfund->nr_shares      = (BACKTEST_INVESTMENT_ESP/mag->close[year_2023]);
		stockfund->funds[0]       = BACKTEST_INVESTMENT_ESP;
		year_2023++;
		for (y=1; y<nr_entries; y++) {
			prior_close = mag->close[year_2023-1];
			close_price = mag->close[year_2023++];
			diff = ((close_price/prior_close)-1)*100.0;
			stockfund->funds[y] = (stockfund->funds[y-1] * (1+(diff/100)));
		}
//		printf(BOLDMAGENTA"[%s] (close: %.2f) profit: %.2f" RESET "\n", stock->sym, mag->close[year_2020-1], stockfund->funds[nr_entries-1]);
	}
	for (x=0; x<nr_entries; x++) {
		stockfund = &backtest->stockfunds[0];
		for (y=0; y<10; y++) {
			backtest->sum[x] += stockfund->funds[x]; 
			stockfund++;
		}
	}
	backtest->json_linechart_len = backtest_linechart(&backtest->json_linechart, NULL, nr_entries, &backtest->sum[0]);
	BACKTEST_INITIALIZED = 1;
	SESSION_LOCK();
	session_list = get_session_list();
	DLIST_FOR_EACH_ENTRY(session, session_list, list) {
		session_load_ports(session);
	}
	SESSION_UNLOCK();
}

int get_json_backtest(struct session *session, char *backname, int http_fd)
{
	if (!strcmp(backname, "ESP")) {
		net_send(http_fd, gspc_backtest.json, gspc_backtest.json_len);
		net_send(http_fd, ixic_backtest.json, ixic_backtest.json_len);
		net_send(http_fd, rut_backtest.json,  rut_backtest.json_len);
		net_send(http_fd, dji_backtest.json,  dji_backtest.json_len);
		net_send(http_fd, esp_passive_backtest.json, esp_passive_backtest.json_len);
		return 1;
	}
	return 0;
}

int backtest_linechart(char **json_out, char *buf, int nr_entries, double *sum)
{
	char *jptr, *json;
	int x, nbytes, json_len = 0;

	if (!buf) {
		if (nr_entries < 10)
			jptr = json = malloc(256);
		else
			jptr = json = malloc(16 KB);
	} else {
		jptr = json = buf;
	}
	for (x=0; x<nr_entries; x++) {
		if (x == (nr_entries-1))
			nbytes = sprintf(jptr, "%.2f]",  sum[x]);
		else if (x == 0)
			nbytes = sprintf(jptr, "[%.2f,", sum[x]);
		else
			nbytes = sprintf(jptr, "%.2f,",  sum[x]);
		json_len += nbytes;
		jptr     += nbytes;
	}
	if (!buf)
		*json_out = json;
	return (json_len);
}

int linechart_positions(char **json_out, int nr_positions, struct position *position)
{
	char *jptr, *json;
	int x, nbytes, json_len = 0;

	if (nr_positions < 10)
		jptr = json = malloc(512);
	else
		jptr = json = malloc(16 KB);
	for (x=0; x<nr_positions; x++) {
		if (x == (nr_positions-1))
			nbytes = sprintf(jptr, "%.2f]",  position->invested);
		else if (x == 0)
			nbytes = sprintf(jptr, "[%.2f,", position->invested);
		else
			nbytes = sprintf(jptr, "%.2f,",  position->invested);
		json_len += nbytes;
		jptr     += nbytes;
		position++;
	}
	*json_out = json;
	return (json_len);
}

void backtest_long(char *ticker, struct backtest *backtest)
{
	struct stock     *stock = search_stocks(ticker);
	struct stockfund *stockfund;
	struct mag       *mag;
	int year_2020, nr_entries, x;
	double prior_close, close_price, diff;

	if (!stock)
		return;
	mag                      = stock->mag;
	year_2020                = mag->year_2020;
	stockfund                = &backtest->stockfunds[0];
	nr_entries               = mag->nr_entries-year_2020;
	backtest->money_invested = BACKTEST_INVESTMENT;
	stockfund->nr_shares     = (BACKTEST_INVESTMENT/mag->close[year_2020]);
	stockfund->funds[0]      = BACKTEST_INVESTMENT;
	year_2020++;
	for (x=1; x<nr_entries; x++) {
		prior_close          = mag->close[year_2020-1];
		close_price          = mag->close[year_2020++];
		diff                 = ((close_price/prior_close)-1)*100.0;
		stockfund->funds[x]  = stockfund->funds[x-1] * (1+(diff/100));
	}
	backtest->json_len = backtest_linechart(&backtest->json, NULL, nr_entries, &stockfund->funds[x]);
}

void HTTP_BACKTEST(char *port_id, char *backtest_id, int http_fd)
{
	struct session *session;
	struct stock    *stock;
	struct backjson *bj;
	char            *post;
	uint64_t         cookie;
	int              table_index;

	/* GET /BACK/port_id/backtest_id */
	port_id[7]               = 0;
	backtest_id[7]           = 0;
	bj = get_backtest(session, *(uint64_t *)port_id, *(uint64_t *)backtest_id);
	if (!bj) {
		send(http_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
		return;
	}
	net_send(http_fd, bj->main_json, bj->main_json_len);
}

void HTTP_SAVE_BACKTEST(char *req, int http_fd)
{
	struct session *session;
	struct stock   *stock;
	char           *post, *p, *name;
	char            port_id[8];
	uint64_t        cookie;
	int             table_index, do_export;

	/* GET /SAVE/port_id/0/Name */
	*(uint64_t *)port_id = *(uint64_t *)(req+10);
	port_id[7] = 0;
	do_export  = *(req+18) - 48;
	name = req+20;
	p = strchr(name, ' ');
	if (!p)
		return;
	*p = 0;
	if (p-name > 62)
		return;
	printf("backsave port_id: %s name: %s export: %d\n", port_id, name, do_export);
	backtest_save(session, *(uint64_t *)port_id, name, do_export);
	send(http_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
}

int backtest_args(struct backtest *backtest, char *p)
{
	/* Starting Month */
	backtest->start_month = (int)strtoul(p, NULL, 10);
	p = strchr(p, '/');
	if (!p)
		return 0;

	/* Capital */
	backtest->capital = (int)strtod(p+1, NULL);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* avgdays */
	if (*(p+1) == '/') {
		backtest->avgdays = 0;
		p += 1;
	} else {
		backtest->avgdays = (int)strtod(p+1, NULL);
		p = strchr(p+1, '/');
		if (!p)
			return 0;
	}

	/* pt */
	backtest->pt = strtod(p+1, NULL);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* maxpos */
	backtest->max_positions = (int)strtod(p+1, NULL);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* a1q1 */
	backtest->a1q1 = strtol(p+1, NULL, 10);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* a4q1 */
	backtest->a4q1 = strtol(p+1, NULL, 10);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/************
	 * Selectors
	 ***********/
	backtest->capital_dynamic = (int)strtod(p+1, NULL);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* avgdays year */
	backtest->avgdays_year = (int)strtod(p+1, NULL);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* pt dynamic */
	backtest->pt_dynamic = (int)strtod(p+1, NULL);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* maxpos select */
	backtest->maxpos_dynamic = strtol(p+1, NULL, 10);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* a1q1 select */
	backtest->a1q1_year = strtol(p+1, NULL, 10);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* a4q1 select */
	backtest->a4q1_year = (int)strtod(p+1, NULL);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* peak */
	backtest->peak = strtod(p+1, NULL);
	p = strchr(p+1, '/');
	if (!p)
		return 0;

	/* max rank */
	backtest->max_rank = (int)strtol(p+1, NULL, 10);
	p = strchr(p+1, ' ');
	if (!p)
		return 0;
	return 1;
}

void HTTP_RUN_BACKTEST(char *req, int http_fd)
{
	struct session  *session;
	struct backtest *backtest;
	struct stock    *stock;
	struct port     *port;
	char            *post, *p;
	char             port_id[8];
	uint64_t         cookie;
	int  capital, maxpos, avgdays, a1q1, a4q1, a1q1_year, a4q1_year;
	int  capital_dynamic, maxpos_dynamic, avgdays_select, a1q1_select, a4q1_select, pt_select;

	/* GET /RUN/port__id/capital/avgdays/pt/maxpos/a1q1/a4q1/capital_sel/avgdays_sel/pt_sel/maxpos_sel/a1q1_sel/a4q1_sel/peak/rank */
	if (!BACKTEST_INITIALIZED)
		return;

	backtest = malloc(sizeof(*backtest));
	*(uint64_t *)port_id = *(uint64_t *)(req+9);
	port_id[7] = 0;
	printf("RUN_BACKTEST() port id: %s\n", port_id);
	if (!backtest_args(backtest, req+18))
		return;
	backtest->http_fd = http_fd;
	backtest_ultra(session, *(uint64_t *)port_id, backtest, 0, NULL, 0);
}

void HTTP_PORTFOLIO(char *req, int http_fd)
{
	struct session *session;
	struct stock   *stock;
	char           *post;
	char            port_id[8];
	uint64_t        cookie;

	if (!BACKTEST_INITIALIZED)
		goto out;
	/* GET /PORT/port_id */
	/* GET /PORTS */
	if (*(req+9) == 'S') {
		printf("ports json: %s len: %d\n", session->ports_json, session->ports_json_len);
		net_send(http_fd, session->ports_json, session->ports_json_len);
		return;
	}
	*(uint64_t *)port_id = *(uint64_t *)(req+10);
	port_id[7] = 0;

	printf("PORT_ID: %s\n", port_id);
	if (get_port(session, *(uint64_t *)port_id, http_fd))
		return;
out:
	send(http_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
}


int set_global_port(struct port *uport, uint64_t port_id)
{
	struct port     *port;
	struct backdb   *backdb;
	struct backjson *bj;
	struct filemap   filemap;
	char *bmap;
	char path[256];
	int found = 0, filesize, fd, x;

	mutex_lock(&port_lock);
    DLIST_FOR_EACH_ENTRY(port, &port_list, list) {
		if (port->port_id == port_id) {
			memcpy(uport, port, sizeof(*port));
			found = 1;
			break;
		}
	}
	mutex_unlock(&port_lock);
	if (!found)
		return 0;
	/* Load Public Backtests */
	sprintf(path, "db/ports/%.8s.bpub", (char *)&port->port_id);
	bmap = MAP_FILE_RO(path, &filemap);
	if (!bmap)
		return 0;
	backdb = (struct backdb *)bmap;
	for (x=0; x<port->nr_backtests; x++) {
		bj                = malloc(sizeof(*bj));
		bj->main_json     = strdup(backdb->main_json);
		bj->main_json_len = backdb->main_json_len;
		bj->gain          = backdb->gain;
		bj->uid           = backdb->uid;
		bj->gid           = backdb->gid;
		strcpy(bj->name, backdb->name);
		port->backtest[port->nr_backtests++] = bj;
		backdb++;
	}
	UNMAP_FILE(bmap, &filemap);
}

void list_add_portfolio(struct port *port)
{
	LIST_ADD(&port->list, &port_list);
}

void session_load_ports(struct session *session)
{
	struct filemap   filemap, filemap2;
	struct portdb   *portdb;
	struct backdb   *backdb;
	struct backtest *backtest;
	struct port     *port;
	struct backjson *bj;
	char             path[256];
	char             buf[256];
	char            *pmap, *bmap, *tptr, *bptr;
	int              x, y, backlist_len, nr_ports, nbytes, public_offset;

	if (!BACKTEST_INITIALIZED)
		return;

	if (!session->user->logged_in) {
		sprintf(path, "db/ports/cookie/%s.port", session->cookie);
	} else {
		sprintf(path, "db/ports/%d.port", session->user->uid);
	}
	pmap = MAP_FILE_RO(path, &filemap);
	if (!pmap) {
		portdb   = (struct portdb *)&ultra_portdb;
		nr_ports = 1;
	} else {
		portdb   = (struct portdb *)pmap;
		nr_ports = (filemap.filesize/sizeof(struct portdb));
	}

	session->ports_json = malloc(16 KB);
	tptr   = session->ports_json;
	session->nr_ports = nr_ports;
	session->ports_json_len = 0;
	for (x=0; x<nr_ports; x++) {
		if (session->nr_ports >= MAX_PORTFOLIOS)
			break;
		port = malloc(sizeof(*port));
//		printf("FIRST PORTFOLIO ID: %s\n", (char *)&portdb->id);
		/* Global Backtests for THIS Portfolio */
		if (!set_global_port(port, portdb->id)) {
			port->json_mini     = strdup(portdb->json_mini);
			port->json_mini_len = portdb->json_mini_len;
			port->nr_backtests  = portdb->nr_backtests;
			public_offset       = 0;
		} else {
			public_offset       = port->nr_backtests;
		}
		/* Portfolio List HTML DIVs */
		nbytes = sprintf(tptr, "%s$", port->json_mini);
		tptr  += nbytes;
		session->ports_json_len += nbytes;

		/* User Private Backtests */
		sprintf(path, "db/ports/%s.bpriv.%d", (char *)&port->port_id, session->user->uid);
		printf("private backtest file: %s port_id: %s\n", path, (char *)&port->port_id);
		bmap = MAP_FILE_RO(path, &filemap2);
		if (!bmap)
			backdb = &ultra_backdb;
		else
			backdb = (struct backdb *)bmap;
		printf("SESSION LOAD: BACKTESTS: %d port: %p\n", portdb->nr_backtests, port);
		port->backlist_table     = malloc(32 KB);
		port->backlist_table_len = 0;
		bptr = port->backlist_table;
		strcpy(bptr, BTABLE);
		bptr += sizeof(BTABLE)-1;
		backlist_len = sizeof(BTABLE)-1;

		for (y=0; y<portdb->nr_backtests; y++) {
			bj                = malloc(sizeof(*bj));
			bj->main_json     = malloc(backdb->main_json_len+1);
			strcpy(bj->main_json, backdb->main_json);
			bj->main_json_len = backdb->main_json_len;
			bj->gain          = backdb->gain;
			bj->uid           = backdb->uid;
			bj->gid           = backdb->gid;
			bj->avgdays_pos   = backdb->avgdays_pos;
			strcpy(bj->name, backdb->name);
			printf("ADDING BACKTEST: %s gid: %s avgdays: %.2f gain: %.2f pub: %d\n", bj->name, (char *)&bj->gid, bj->avgdays_pos, bj->gain, public_offset);
			port->backtest[public_offset++] = bj;
			backdb++;
			nbytes  = sprintf(bptr, BTABLE_ENTRY, (char *)&bj->gid, bj->name, daystr(bj->gain, buf), bj->avgdays_pos);
			bptr   += nbytes;
			backlist_len += nbytes;
		}
		strcpy(bptr, "</tbody></table>");
		backlist_len += 16;
		port->backlist_table_len = backlist_len;
		printf("SESSION LOAD: BACKTEST LIST: %s port: %p strlen: %d len: %d\n", port->backlist_table, port, (int)strlen(port->backlist_table), port->backlist_table_len);
		session->portlist[x]     = port;
		if (filemap2.fd != -1)
			UNMAP_FILE(bmap, &filemap2);
	}
	if (filemap.fd != -1)
		UNMAP_FILE(bmap, &filemap);
	return;
}
