#ifndef __WATCHTABLE_H
#define __WATCHTABLE_H

#include <conf.h>
#include <stocks/stocks.h>

#define VALUE_TYPE_INT8    1
#define VALUE_TYPE_UINT8   2
#define VALUE_TYPE_INT16   3
#define VALUE_TYPE_UINT16  4
#define VALUE_TYPE_INT32   5
#define VALUE_TYPE_UINT32  6
#define VALUE_TYPE_INT64   7
#define VALUE_TYPE_UINT64  8
#define VALUE_TYPE_DOUBLE  9
#define VALUE_TYPE_STR    10

struct stock;
struct column;
struct column_value;

typedef void (*column_cb) (struct column_value *cvalue);

struct column {
	char           *column_name_str;
	char           *column_fmt_str;
	int             column_class;
	column_cb       cb;
	UT_hash_handle  hh;
};

struct preset {
	char              name[64];
	char              indicators[64];
}__attribute__((packed));

struct wfunction {
	int (*f)(struct session *session, struct watchlist *watchlist, char *packet, char dir, int size);
};
extern struct wfunction wftable[1000];

static __inline__ double anyday_column(struct mag *mag, int column)
{
	switch (column) {
		case COL_5AD21Q1: {
			struct quarter *quarter = &mag->quarters[0];
			return quarter->anyday_1r;
		};
		case COL_5AD21Q2: {
			struct quarter *quarter = &mag->quarters[0];
			return quarter->anyday_2r;
		};
		case COL_5AD20Q1: {
			struct quarter *quarter = &mag->quarters[1];
			return quarter->anyday_1r;
		};
		case COL_5AD20Q2: {
			struct quarter *quarter = &mag->quarters[1];
			return quarter->anyday_2r;
		};
		case COL_5AD19Q1: {
			struct quarter *quarter = &mag->quarters[2];
			return quarter->anyday_1r;
		};
		case COL_5AD19Q2: {
			struct quarter *quarter = &mag->quarters[2];
			return quarter->anyday_2r;
		};
		case COL_10AD21Q1: {
			struct quarter *quarter = &mag->quarters10[0];
			return quarter->anyday_1r;
		};
		case COL_10AD21Q2: {
			struct quarter *quarter = &mag->quarters10[0];
			return quarter->anyday_2r;
		};
		case COL_10AD20Q1: {
			struct quarter *quarter = &mag->quarters10[1];
			return quarter->anyday_1r;
		};
		case COL_10AD20Q2: {
			struct quarter *quarter = &mag->quarters10[1];
			return quarter->anyday_2r;
		};
		case COL_10AD19Q1: {
			struct quarter *quarter = &mag->quarters10[2];
			return quarter->anyday_1r;
		};
		case COL_10AD19Q2: {
			struct quarter *quarter = &mag->quarters10[2];
			return quarter->anyday_2r;
		};
		case COL_5AD21: {
			struct quarter *quarter = &mag->quarters[0];
			return quarter->avgdays_anyday;
		};
		case COL_5AD20: {
			struct quarter *quarter = &mag->quarters[1];
			return quarter->avgdays_anyday;
		};
		case COL_5AD19: {
			struct quarter *quarter = &mag->quarters[2];
			return quarter->avgdays_anyday;
		};
		case COL_10AD21: {
			struct quarter *quarter = &mag->quarters10[0];
			return quarter->avgdays_anyday;
		};
		case COL_10AD20: {
			struct quarter *quarter = &mag->quarters10[1];
			return quarter->avgdays_anyday;
		};
		case COL_10AD19: {
			struct quarter *quarter = &mag->quarters10[2];
			return quarter->avgdays_anyday;
		};
	}
	return 0.0;
}

char *XLS_packet(struct stock *stock, char *TID, struct wtab *preset, int nr_rows, int *outlen);

/* indi.c */
void session_load_presets            (struct session *session);
int  websocket_presets               (struct session *session, char *packet);
void savepreset                      (struct session *session, char *name, char *indicators);
void preset_modify                   (struct session *session, int   pid,  int   workspace, int activate);

void session_load_css                (struct session *session);
void session_load_watchtable_presets (struct session *session);
void save_table_css                  (struct session *session, char *css_name);
int  websocket_watchtable_presets    (struct session *session, char *packet);
struct watchlist *search_watchtable  (struct session *session, char *watchtable_name);
struct wtab *search_watchtable_preset(struct session *session, char *name, int *position);

int             watchtable_packet    (struct session   *session,   struct watchlist *watchlist, char *packet);
int             map_dict             (struct wtab      *wtab,      char *dict, int nr_columns);
void            init_wtable          (void);
unsigned short  colmap               (char *key, int len);
int             column_sprintf       (struct session *session, struct watchlist *watchlist, struct stock *stock, struct wtab *wtab, char *packet, int packet_len, int column);
struct wtab    *default_watchtable_preset(void);
#endif
