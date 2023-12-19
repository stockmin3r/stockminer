#ifndef __BACKTEST_H
#define __BACKTEST_H

#include <conf.h>

#define MAX_PORTFOLIOS  64
#define NEXT_DAY_ACTION 1
#define NEXT_DAY_ESP    2
#define MAX_POSITIONS   1024
#define MAX_BACKTESTS   10
#define POSITION_CLOSED 1
#define POSITION_OPEN   2

#define PORT_MAIN       0
#define PORT_ULTRA      1
#define PORT_AGRO       2
#define PORT_MODERATE   3

#define JSON_MINI_MAX 4095

struct stockfund {
        double        funds[256];
        struct stock *stock;
        int           nr_shares;
        int           start_day;
};

struct backjson {
	char              *main_json;
	char              *ctrl_json;
	int                main_json_len;
	int                ctrl_json_len;
	char               name[64];
	double             gain;
	unsigned int       uid;
	uint64_t           gid;
	double             avgdays_pos;
};

struct port {
	struct backjson  *backtest[MAX_BACKTESTS];
	struct list_head  list;
	char              name[64];
	char              desc[96];
	uint64_t          bid[MAX_BACKTESTS];
	uint64_t          port_id;
	unsigned int      uid;
	double            gain;
	char             *json_mini;
	char             *backlist_table;
	int               backlist_table_len;
	int               json_mini_len;
	int               nr_backtests;
};

struct portdb {
	uint64_t          id;
	double            gain;
	char              name[64];
	char              desc[96];
	char              json_mini[4096];
	int               json_mini_len;
	int               nr_backtests;
};

struct backdb {
	char              main_json[64 KB];
	char              ctrl_json[4096];
	char              name[64];
	double            gain;
	double            avgdays_pos;
	int               main_json_len;
	int               ctrl_json_len;
	unsigned int      uid;
	uint64_t          gid;
};

struct position {
	struct stock      *stock;
	struct esp_signal *esp;
	double             nr_shares;
	double             profit;
	double             size;
	double             target;
	double             gain;
	double             stock_price;
	double             pc_diff;
	double             invested;      /* Total amount Invested in Portfolio (Avail+Invested) at the time position was taken */
	time_t             date;
	int                month;
	int                day_index;
	int                nr_days;
	int                status;
	int                risk;
};

struct backtest {
	double              sum[256];
	struct stockfund    stockfunds[MAX_STOCKS];
	struct position     positions[MAX_POSITIONS];
	struct esp_signal **esp_signals;
	struct rank_table  *current_ranks;
	struct backjson    *backtest_json;
	char               *json_linechart;
	char               *sector_json;
	char               *ctrl_json;
	char               *json;
	int                 sectors[12];
	int                 sector_json_len;
	int                 json_linechart_len;
	int                 ctrl_json_len;
	int                 json_len;
	int                 uid;
	uint64_t            gid;
	double              gain;
	double              avgdays_pos;
	int                 http_fd;
	int                 type;
	/* ***********
	 * Saved Data*
	 ************/
	double              money_invested;
	double              money_available;
	double              money_initial;
	int                 esp_index;
	int                 nr_open_positions;
	int                 nr_positions;
	int                 current_day;
	int                 current_month;
	/* bexec */
	int                 capital;
	int                 capital_dynamic;
	int                 max_rank;
	int                 max_positions;
	int                 maxpos_dynamic;
	int                 start_month;
	int                 pt_dynamic;
	double              pt;
	/* ESP */
	int                 avgdays;
	double              peak;
	int                 a1q1;
	int                 a4q1;
	int                 a1q1_year;
	int                 a4q1_year;
	int                 avgdays_year;
};

extern struct port port_ultra;
extern struct portdb ultra_portdb;
extern struct backdb ultra_backdb;

void backtest_sectors(struct backtest *backtest);
void init_backtest();
int get_json_backtest(struct session *session, char *backname, int http_fd);
int get_port(struct session *session, uint64_t port_id, int http_fd);
void backtest_ultra(struct session *session, uint64_t port_id, struct backtest *backtest, double money_initial, char **stocklist, int nstocks);
void backtest_save(struct session *session, uint64_t port_id, char *name, int export);
int linechart_positions(char **json_out, int nr_positions, struct position *position);
struct backjson *get_backtest(struct session *session, uint64_t port_id, uint64_t backtest_id);
struct port *get_portfolio(struct session *session, uint64_t port_id);
void list_add_portfolio(struct port *port);

#define BTABLE   "<table id=backsave-table>"   \
				 "<caption>Backtests</caption>" \
				 "<thead>"              \
					"<th>Backtest</th>" \
					"<th>Gain</th>"     \
					"<th>Avgdays</th>"  \
				"</thead>"              \
				"<tbody>"               \

#define BTABLE_ENTRY "<tr id=%s_b onclick=backtest_load()>" \
						"<td>%s</td>"   \
						"<td %s</td>"   \
						"<td>%.1f</td>" \
					 "</tr>"
#endif
