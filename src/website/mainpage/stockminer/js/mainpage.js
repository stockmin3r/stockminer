/**********************************************
 * filename: mainpage/stockminer/js/mainpage.js
 * License:  Public Domain
 *********************************************/

// eg: src/website/mainpage/stockminer/website.json
var WEBSITE = @MAINPAGE_stockminer;

// Globals
var SP     = {},   /* StockPage Dictionary */
QMAP       = {},   /* Currently Loaded Stockpages */
WMAP       = {},   /* QuadVerse Workspace QGID Map to the workspace div referenced by the QGID */
WS,                /* WebSocket */
QUADVERSE,         /* QuadVerse  <div> */
LBX,               /* Login box  <div> */
NONCE,             /* NONCE for Password-Based Authentication (sent by server after websocket conn */
QuadVerses = {},   /* QuadVerse Dictionary */
charts     = {},   /* Charts */
QCACHE     = [],   /* Custom QuadVerse Layout */
QPAGE      = [],   /* Array of User's Custom Quadverse Names */
T          = {},   /* Tables */
Q          = [],   /* QuadVerse Array */
A          = [],   /* Private Alerts */
G          = [],   /* Public  Alerts */
WL         = [],   /* Private Watchlists */
GW         = [],   /* Public  Watchlists */
HELP       = {},   /* Help Dictionary */
WMenu      = {},   /* (Private) Watchlist JQuery right-click context menu */
GMenu      = {},   /* (Global)  Watchlist JQuery right-click context menu */
PMenu      = {},   /* Screener Column Preset dictionary */
TPMenu     = {},   /* Screener Column Preset JQuery right-click context-menu */ 
TP         = {},   /* Stock Preset Dicts  */
OP,                /* Option Preset Dicts */
PMD        = {},   /* Chart Indicator Preset Dictionary options */
PMS        = [],   /* <select> innerHTML cache of currently selected Global Indicator */
PSET       = [],   /* Array of Indicator Preset Dictionaries? */ 
PR         = [],   /* Array of Chart Indicator Preset Names */
COL        = {},   /* datatable colors */
OPT        = {},   /* MACD Indicator Options */
QP,                /* QuadVerse JSON */
CTAB       = {},   /* Cached Tables */
ECELL      = {},   /* Editable Cells */
prodict    = {},   /* Profile Settings Dictionary */
USER,              /* Currently Logged in User */
CSR,               /* Candle Screener Table */
CSP,               /* Current Stock Page (ticker) */
C          = {},   /* Stock Candlestick Flags JSON */
Screeners  = [],   /* Column Screener Modules */
WCOL       = {},   /* Stock Column Screener Dictionary */
WCOL2      = {},   /* Same as above but no spaces in the columns */
WLI        = null, /* Screener highlight row onclick global variable */ 
QSH_SP     = {},   /* Scrippies */
css_editor,        /* CSS Ace Editor reference */
js_editor,         /* JS Ace Editor reference */
CSS        = [],   /* Array of datatable CSS objects */
editor     = 0,    /* editor initialized? */
WINIT      = 0,    /* Website Completed Loading - fini() returned */
HCINIT     = 0,    /* Highcaps Freepage Data is only fetched when the user clicks on the Highcaps workspace first time */
FLEET      = 0,    /* Fleet AirPort App Init */
UINIT      = 0,    /* Price UFO Init */
VINIT      = 0,    /* Volume UFO Init */
SIGMON     = 0,    /* Sigmon Init (bottom quad screener in the stockpage */
OPSEL      = 1,
OPCHART    = 1,    /* Options - option chart */
OPCHAIN    = 2,    /* Options - OpChain */
OPCC       = 3,    /* Options - Covered Calls */
NO_RPC     = 0,
CUROP,
CPLAY,
UFO_PRICE  = 1,
UFO_VOL    = 2,
PORT_MONTH = 0,backtest_box=0,save_box=0,backtest_id=0,back_gain,port_id,ports,
SPX_pr,SPX_pc,NDX_pr,NDX_pc,DOW_pr,DOW_pc,XLK_pr,XLK_pc,

/* Live QuadVerse Update RPCs */
QUPDATE_SETGRID        = "qupdate 0 ",
QUPDATE_QSP_DEL        = "qupdate 1 ",
QUPDATE_WS_DEL         = "qupdate 2 ",
QUPDATE_QNAME          = "qupdate 3 ",
QUPDATE_WSNAME         = "qupdate 4 ",
QUPDATE_POS            = "qupdate 5 ",
QUPDATE_ADD_CHART_INDI = "qupdate 6 ",
QUPDATE_DEL_CHART_INDI = "qupdate 7 ",
QUPDATE_DEL_IMG        = "qupdate 8 ",
QUPDATE_DEL_CHART      = "qupdate 9 ",
QUPDATE_ADD_PDF        = "qupdate 10 ",
QUPDATE_DEL_PDF        = "qupdate 11 ",
QUPDATE_ADD_WSTAB      = "qupdate 12 ",
QUPDATE_DEL_WSTAB      = "qupdate 13 ",
QSUBCMD_CHART_CSV      = '1',

LINK      = ['green','red','dodgerblue','#821eda', 'yellow', 'white'],
TCLASS    = ['MTAB', 'XLS', 'SBTAB'], 
Themes    = {},  /* Table Themes */

STAB =[{"orderable":false,"data":null,"defaultContent":''},{"data":"900"},{"data":"901"},{"data":"903"},{"data":"904"}],
BTAB =[{"orderable":false,"data":null,"defaultContent":''},{"data":"900"},{"data":"901"},{"data":"903"},{"data":"904"},{"data":"700"},{"data":"701"},{"data":"702"},{"data":"704"},{"data":"704"},{"data":"705"},{"data":"706"}],
MTAB =[{"data":"900"},{"data":"901"},{"data":"903"},{"data":"904"}],VTAB=[{"data":"900"},{"data":"901"},{"data":"903"},{"data":"905"},{"data":"904"}],

/* RPC */
op   = {'addPoint':addPoint,       'update':update,       'stockchart':stockchart,     'mini':   rpc_minichart,   'upmini':  rpc_upmini,    'ctheme':rpc_chart_themes,
        'table':   table,          'ctable':ctable,       'optab':     optab,          'notify': rpc_notify,      'deftab':  rpc_deftab,    'wget':  rpc_wget,
        'watch':   rpc_watchlist,  'gwatch':rpc_gwatch,   'gexec':     rpc_gexec,      'exec':   rpc_pexec,       'preset':  rpc_indicator, 'TPset': rpc_TPset,
        'init':    rpc_init,       'fini':  rpc_fini,     'stage':     rpc_stageload,  'profile':rpc_profile,     'qspace':  rpc_qspace,    'index': rpc_menu,
        'anyday':  rpc_anyday,     'peak':  rpc_peak,     'sc':        rpc_stockpage,  'etab':   rpc_etab,        'sigmon':  rpc_sigmon,    'err':   rpc_login_error,
        'cookie':  rpc_set_cookie, 'user':  rpc_set_user, 'regok':     rpc_regok,      'nonce':  rpc_nonce,       'query':   rpc_query,
        'wspace':  rpc_wspace,     'qpage': rpc_qpage,    'qcache':    rpc_qcache,     'qreload':rpc_qreload,     'qupdate': rpc_qupdate,
        'candle':  candle,         'csr':   rpc_csr,      'czoom':     rpc_candle_zoom,'csp':    rpc_csp,         'candy':   rpc_stockpage_candles,
        'netsh':   rpc_netsh,      'qsh':   rpc_qsh,      'admin':     admin,          'styles': rpc_styles,      'newtab':  rpc_newtab},

SBC    = {913:[{f:-24.9,t:-20.0,c:"#ffa500",b:1},{f:-99,t:-25.0,c:"#FF3406",b:1}],
		  400:[{f:1.0,t:4.0,c:"#ffff00",b:0}],
/*delta*/ 903:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-19.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  909:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-19.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  910:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-19.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  911:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-19.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  218:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-19.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  219:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-19.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  220:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  221:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  222:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.0,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  223:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.0,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  230:[{f:-5.0,t:-2.99,c:"#ffff00",b:1},{f:-3.0,t:100.0,c:"#ffa500",b:1}],
		  850:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-19.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  851:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-19.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  852:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  853:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.9,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  854:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.0,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  855:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.0,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  860:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.0,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  865:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.0,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  871:[{f:5.0,t:9.9,c:"#32cd32",b:1},{f:10.0,t:200.0,c:"#008000",b:1},{f:-9.9,t:-4,c:"#ffff00",b:1},{f:-10.0,t:-10,c:"#ffa500",b:1},{f:-99.9,t:-20.0,c:"#ff0000",b:1}],
		  236:[{f:-5.0,t:-2.99,c:"#ffff00",b:1},{f:-3.0,t:100.0,c:"#ffa500",b:1}]},
COLORS        = [SBC], /* Color Schemes */
CMAP          = {},    /* COLORS pointer/index/map */
CRULE,                 /* Color-Rule <div> */
errors        = [{fail:elogin}, {ok:setprofile_ok, fail:setprofile_fail}],
VERSION       = '0.94',
OBJTYPES      = { img:1, doc:2,  vid:3, app:4, data:5},
FILETYPES     = { txt:1, html:2, pdf:3, png:4, jpg:5, jpeg:6, gif:7, csv:8, xlsx:9 },
ENCODINGS     = { png:'bin',jpg:'bin',jpeg:'bin',gif:'bin',xlsx:'bin', pdf:'bin',csv:'txt',html:'txt',py:'txt'},
ENC2OBJ       = { png:'img',jpg:'img',jpeg:'img',gif:'img',xlsx:'data',pdf:'doc',csv:'txt',html:'txt'},
// object types
OBJTYPE_IMG   = 1,
OBJTYPE_DOC   = 2,
OBJTYPE_VID   = 3,
OBJTYPE_APP   = 4,
OBJTYPE_DATA  = 5,
// filetypes
FILETYPE_TXT  = 1,
FILETYPE_HTML = 2,
FILETYPE_PDF  = 3,
FILETYPE_PNG  = 4,
FILETYPE_JPG  = 5,
FILETYPE_JPEG = 6,
FILETYPE_GIF  = 7,
FILETYPE_CSV  = 8,
FILETYPE_XLSX = 9,

// object actions
ACTION_BOOT                   = 0,   // "boot" the first quadverse's quadspace - see init.js::boot_quadspace0()
ACTION_QPAGE                  = 1,   // Load a User's Profile or a User's Quadverse
ACTION_RELOAD                 = 2,   // Reload the websocket connection to the website after detecting it coming back online
ACTION_IMG_WORKSPACE          = 3,   // Send   Image  to a Workspace
ACTION_IMG_QUADVERSE          = 4,   // Upload Image  of a User QuadVerse
ACTION_PROFILE                = 5,   // Upload Image  of a User Profile
ACTION_IMG_BG_PROFILE         = 6,   // Upload Image  of a User Profile Background
ACTION_IMG_SQUEAK             = 7,   // Upload Image  to a Squeak
ACTION_DOC_WORKSPACE          = 8,   // Send PDF/DOCX to a Workspace
ACTION_DOC_SQUEAK             = 9,   // Send PDF/DOCX to a Squeak/Tweet
ACTION_DATA_CHART_WORKSPACE   = 10,  // Make a Chart     out of a Dataset  (CSV,JSON,etc) OBJTYPE_DATA/DATAFILETYPE_CSV/TICKER||CHART_TITLE/
ACTION_DATA_CHART_SQUEAK      = 11,  // Make a Chart     out of a Dataset  (and Squeak it)
ACTION_DATA_TABLE_WORKSPACE   = 12,  // Make a DataTable out of a Dataset  (CSV,JSON,etc)
ACTION_DATA_TABLE_SQUEAK      = 13,  // Make a DataTable out of a Dataset  (and squeak it)
ACTION_DATA_STORE_ENCRYPTED   = 14,  // Send data to P2P network for storage and encrypt it
ACTION_DATA_RANKS_UPDATE      = 15,  // Send a ranks.csv data file to update the stock ranks
ACTION_APP_PYSCRIPT_WORKSPACE = 16,  // Load a HTML file with Python in it (into a workspace)
ACTION_APP_PYSCRIPT_SQUEAK    = 17,  // Load a HTML file with Python in it (and squeak it)
ACTION_APP_CANVAS_WORKSPACE   = 18,  // Load a Canvas App (into a workspace)
ACTION_APP_CANVAS_SQUEAK      = 19,  // Load a Canvas App (and squeak it)

// app types
APP_TYPE_PYSCRIPT = 1,
APP_TYPE_C        = 2,
APP_TYPE_CPP      = 4,
APP_TYPE_GO       = 5,
APP_TYPE_WAVE     = 6,
DEL_RPC           = { 'img': QUPDATE_DEL_IMG, 'pdf': QUPDATE_DEL_PDF, 'chart': QUPDATE_DEL_CHART, 'wstab': QUPDATE_DEL_WSTAB },
tools             = [ 'pencil', 'marker', 'eraser', 'arrow', 'text', 'image', 'pdf', 'line', 'rect', 'arc', 'quad', 'bezier' ],
WS_URL            = "wss://localhost:port/ws/";
$.fn.dataTable.ext.errMode='none';

function init_stockminer()
{
	compat();
	init_charts();
	init_stockpage();

	chart_table_onclick("LG");
	chart_table_onclick("LL");
	chart_table_onclick("HG");
	chart_table_onclick("HL");
	datatable({id:"HG",columns:[{"data":"T"},{"data":"R"},{"data":"P"},{"data":"D"},{"data":"V"}], ncol:5, order:3, slope:"desc"});
	datatable({id:"HL",columns:[{"data":"T"},{"data":"R"},{"data":"P"},{"data":"D"},{"data":"V"}], ncol:5, order:3, slope:"asc"});

	// init_quadverse() must be called before init_websocket()
	init_quadverse();
	init_websocket("");
	init_menu();
};


$(document).ready(function(){
	init_stockminer()
});
