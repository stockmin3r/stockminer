#ifndef __WORKSPACE_H
#define __WORKSPACE_H

#include <stdinc.h>

#define MAX_QUADVERSES          32
#define MAX_QUADSPACES          16
#define MAX_QUADS                8
#define MAX_WORKSPACES          10
#define MAX_QPAGES              10
#define MAX_CHARTS              32
#define MAX_DIVSIZE             32
#define MAX_SUBSCRIBERS         64
#define MAX_WORKSPACE_OBJECTS   32
#define MAX_BOOT_OBJECTS       256
#define MAX_WEBSOCKETS           8
#define MAX_OBJ_SIZE             32*1024*1024

#define QUPDATE_GRID            0
#define QUPDATE_QSP_DEL         1
#define QUPDATE_WS_DEL          2
#define QUPDATE_QSP_TITLE       3
#define QUPDATE_WS_NAME         4
#define QUPDATE_POSITION        5
#define QUPDATE_ADD_CHART_INDI  6
#define QUPDATE_DEL_CHART_INDI  7
#define QUPDATE_DEL_IMG         8
#define QUPDATE_DEL_CHART       9
#define QUPDATE_ADD_PDF        10
#define QUPDATE_DEL_PDF        11
#define QUPDATE_ADD_WSTAB      12
#define QUPDATE_DEL_WSTAB      13

#define CHART_TYPE_BAR                 1
#define CHART_TYPE_LINE                2
#define CHART_TYPE_CANDLE              3
#define CHART_TYPE_PIE                 4

#define CHART_TYPE_MINI                1
#define CHART_TYPE_OHLC                2

// Object type
#define OBJTYPE_IMG                    1  // Image    - (png,jpg,jpeg)
#define OBJTYPE_DOC                    2  // Document - (pdf,docx)
#define OBJTYPE_VID                    3  // Video    - (mp4,etc)
#define OBJTYPE_GIF                    6  // GIF      - (gif animation)
#define OBJTYPE_DATA                   7  // Data     - process dataset (CSV,JSON,BSON,etc) to chart/table/app/?
#define OBJTYPE_EXCEL                  8  // Excel    - XLSX file (to be converted to HTML and acted acted on: turn into chart/table/app/?
#define OBJTYPE_APP                    9  // APP      - pyScript,C/C++,R,webRTC-pdf-mod

// File Extensions
#define FILETYPE_TXT                   1
#define FILETYPE_HTML                  2
#define FILETYPE_PDF                   3
#define FILETYPE_PNG                   4
#define FILETYPE_JPG                   5
#define FILETYPE_JPEG                  6
#define FILETYPE_GIF                   7
#define FILETYPE_CSV                   8
#define FILETYPE_EXCEL                 9

// Object Actions
#define ACTION_BOOT                    0
#define ACTION_QPAGE                   1
#define ACTION_RELOAD                  2
#define ACTION_IMG_WORKSPACE           3  // Send   Image  to a Workspace
#define ACTION_IMG_QUADVERSE           4  // Upload Image  of a custom QuadVerse
#define ACTION_IMG_PROFILE             5  // Upload Image  of a User Profile
#define ACTION_IMG_BG_PROFILE          6  // Upload Image  of a User Profile Background
#define ACTION_IMG_SQUEAK              7  // Upload Image  to a Squeak
#define ACTION_DOC_WORKSPACE           8  // Send PDF/DOCX to a Workspace
#define ACTION_DOC_SQUEAK              9  // Send PDF/DOCX to a Squeak/Tweet
#define ACTION_DATA_CHART_WORKSPACE   10  // Make a Chart     out of a Dataset  (CSV,JSON,etc) OBJTYPE_DATA/DATAFILETYPE_CSV/TICKER||CHART_TITLE/
#define ACTION_DATA_CHART_SQUEAK      11  // Make a Chart     out of a Dataset  (and Squeak it)
#define ACTION_DATA_TABLE_WORKSPACE   12  // Make a DataTable out of a Dataset  (CSV,JSON,etc)
#define ACTION_DATA_TABLE_SQUEAK      13  // Make a DataTable out of a Dataset  (and squeak it)
#define ACTION_DATA_STORE_ENCRYPTED   14  // Send data to P2P connection for storage and encrypt it
#define ACTION_DATA_RANKS_UPDATE      15  // Send ranks.csv for stock ranks update
#define ACTION_APP_PYSCRIPT_WORKSPACE 16  // Load a HTML file with Python in it (into a workspace)
#define ACTION_APP_PYSCRIPT_SQUEAK    17  // Load a HTML file with Python in it (and squeak it)
#define ACTION_APP_CANVAS_WORKSPACE   18  // Load a Canvas App (into a workspace)
#define ACTION_APP_CANVAS_SQUEAK      19  // Load a Canvas App (and squeak it)

#define ACTION_UPLOAD_MIN  3
#define ACTION_UPLOAD_MAX 19

// Application Types
#define APP_TYPE_PYTHON               1
#define APP_TYPE_WASM                 2

#define QPAGE_BACKGROUND_UPLOAD  '0' /* Upload a png/jpeg */
#define QPAGE_BACKGROUND_MOUSE   '1' /* mouse   SVG */
#define QPAGE_BACKGROUND_BAT     '2' /* bat     SVG */
#define QPAGE_BACKGROUND_BIRD    '3' /* bird    SVG */
#define QPAGE_BACKGROUND_DOLPHIN '4' /* dolphin SVG */
#define QPAGE_BACKGROUND_SHARK   '5' /* shark   SVG */
#define QPAGE_BACKGROUND_UNICORN '6' /* unicorn SVG */

#define QUADVERSE_STOCKS          0
#define QUADVERSE_PROFILE         4
#define QUADVERSE_ALGO            5

struct webclient;
struct quadverse;

extern int nr_quadverse_pages;
extern struct qpage *quadverse_pages[4096];

#define QPAGE_PATH       "db/uid/%d/qpage"
#define QCACHE_PATH      "db/uid/%d/qcache"
#define MAX_QCACHE_SIZE  24575
#define QPAGE_DIRTY      1

struct peakwatch_workarea {
	double              *last_price;
};

struct stockpage {
	struct stock        *stock;
	int                  QVID;
	int                  QSID;
};

struct qlive {
	int                  key;
	unsigned short       value;
	UT_hash_handle       hh;
};

struct owner {
	int                  uid;
	unsigned short       acl;
}__attribute__((packed));

struct subscriber {
	struct session      *session;
	int                  client_id; // to identify the http_fd/websocket[x]
	int                  QVID;
};

struct qpage {
	yyjson_mut_doc      *qcache;
	struct quadverse    *quadverse;
	char                 qcache_json[MAX_QCACHE_SIZE+1];
	char                 url[128];
	struct subscriber  **subscribers;
	struct owner         owners[64];
	mutex_t              qpage_lock;
	uint64_t             flags;
	unsigned int         max_subscribers;
	unsigned short       nr_owners;
	unsigned short       nr_subscribers;
	unsigned short       qsize;
	unsigned short       usize;
	unsigned short       background_image;
}__attribute__((packed));

struct wsid {
	int                  QVID;
	int                  QSID;
	int                  QID;
	int                  WSID;
};

struct chart {
	struct stock        *stock;
	char                 div[MAX_DIVSIZE];
	char               **indicators;
	unsigned short       nr_indicators;
	unsigned short       nr_ohlc;
	unsigned short       nr_updates;
	unsigned short       type;
	unsigned short       dead;
};

struct workspace {
	struct chart        *charts[MAX_CHARTS];
	struct watchlist    *watchlists[64/2];
	struct table       **tables;
	int                (*watchtables)(struct session *session, char *packet, struct workspace *workspace);
	void                *workarea;
	int                  ufo_tables;
	unsigned char        nr_charts;
	unsigned char        nr_watchtables;
	unsigned char        nr_indicators;
	unsigned char        nr_tables;
};

struct quad {
	struct workspace    *workspace[MAX_WORKSPACES];
	struct qlive        *current_workspace_live;
	unsigned short       nr_workspaces;
	unsigned short       current_workspace[MAX_WEBSOCKETS];
};

struct quadspace {
	struct quad         *quad[MAX_QUADS];
	char                *stockpage;
	unsigned short       nr_quads;
};

struct quadverse {
	struct quadspace    *quadspace[MAX_QUADSPACES];
	struct qpage        *qpage;
	struct qlive        *current_quadspace_live;
	unsigned short       current_quadspace[MAX_WEBSOCKETS];
	unsigned short       nr_quadspaces;
	unsigned short       flags;
};

#endif
