#include <conf.h>
#include <stocks/stocks.h>

#define NR_BUILTIN_COLUMNS sizeof(builtin_columns)/sizeof(struct column)

#define COLUMN_CLASS_STOCKS_COMMON  1
#define COLUMN_CLASS_STOCKS_TREND   2
#define COLUMN_CLASS_STOCKS_FUND    3
#define COLUMN_CLASS_STOCKS_ALGO    4
#define COLUMN_CLASS_STOCKS_INDI    5  // technical indicators
#define COLUMN_CLASS_DYNAMIC        6  // not yet implemented

struct column_hash *COLUMN_HASHTABLE;
struct column_hash *COLUMN_HASHTABLE_INT;

struct wfunction wftable[1000];

static void stock_get_common      (struct column_value *cvalue, int column_id);
static void stock_get_trend       (struct column_value *cvalue, int column_id);
static void stock_get_fundamentals(struct column_value *cvalue, int column_id);
static void stock_get_algo        (struct column_value *cvalue, int column_id);

static bool watchtable_initialized = false;

struct column_value {
	struct stock   *stock;        /* [INPUT]  stock pointer */
	struct mag     *mag;          /* [INPUT]  mag builtin stockminer price and trend data */
	struct mag2    *m2;           /* [INPUT]  m2 python builtin stockminer algorithmic data */
	struct mag3    *m3;           /* [INPUT]  m3 python TA library - broken */
	struct mag4    *m4;           /* [INPUT]  m4 builtin TA algorithms - incomplete */
	int             column_def;   /* [INPUT]  The column #define constant for this column (used for switch() - builtin columns only) */
	int             nr_entries;   /* [INPUT]  The number of days this stock has data for */
	int             entry;        /* [INPUT]  The column entry/day offset in the dataset */
	int             value_type;   /* [OUTPUT] #define value denoting the type for this column [str/double/uint64/...] */
	int             flags;        /* [INPUT]  NEEDS_MAG2|NEEDS_MAG3|NEEDS_MAG4 */
	union {                       /* [OUTPUT] the value to return for this row:column cell */
		char       *str;
		double      DOUBLE;
		uint64_t    uint64;
		int64_t     int64;
		uint32_t    uint32;
		int32_t     int32;
		uint16_t    uint16;
		int16_t     int16;
		uint8_t     uint8;
		int8_t      int8;
	} value;
	char            vstr[128];    /* [OUTPUT] stack char buffer for get_stock_volume() */
	char            timestr[128]; /* [OUTPUT] stack char buffer for unix2str() */
};

struct column builtin_columns[] = {
	{ "d",             "\"915\":\"%s\",",            COLUMN_CLASS_STOCKS_COMMON, COL_DATE        },
	{ "T",             "\"900\":\"%s\",",            COLUMN_CLASS_STOCKS_COMMON, COL_SYMBOL      },
	{ "P",             "\"901\":\"%.2f\",",          COLUMN_CLASS_STOCKS_COMMON, COL_PRICE       },
	{ "R",             "\"902\":\"%d\",",            COLUMN_CLASS_STOCKS_COMMON, COL_RANK        },
	{ "D",             "\"903\":\"%.2f\",",          COLUMN_CLASS_STOCKS_COMMON, COL_DELTA       },
	{ "V",             "\"904\":\"%s\",",            COLUMN_CLASS_STOCKS_COMMON, COL_VOLUME      },
	{ "O",             "\"905\":\"%.2f\",",          COLUMN_CLASS_STOCKS_COMMON, COL_OPEN        },
	{ "H",             "\"906\":\"%.2f\",",          COLUMN_CLASS_STOCKS_COMMON, COL_HIGH        },
	{ "L",             "\"907\":\"%.2f\",",          COLUMN_CLASS_STOCKS_COMMON, COL_LOW         },
	{ "PC",            "\"908\":\"%.2f\",",          COLUMN_CLASS_STOCKS_COMMON, COL_PC          },
	{ "OP",            "\"909\":\"%.2f\",",          COLUMN_CLASS_STOCKS_COMMON, COL_OP          },
	{ "HP",            "\"910\":\"%.2f\",",          COLUMN_CLASS_STOCKS_COMMON, COL_HP          },
	{ "LP",            "\"911\":\"%.2f\",",          COLUMN_CLASS_STOCKS_COMMON, COL_LP          },
	/* Trend */
	{ "PK",            "\"912\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_PK          },
	{ "PKP",           "\"913\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_PKP         },
	{ "NDUP",          "\"400\":\"%d\",",            COLUMN_CLASS_STOCKS_TREND,  COL_NDUP        },
	{ "NDDW",          "\"401\":\"%d\",",            COLUMN_CLASS_STOCKS_TREND,  COL_NDDW        },
	{ "NWUP",          "\"402\":\"%d\",",            COLUMN_CLASS_STOCKS_TREND,  COL_NWUP        },
	{ "NWDW",          "\"403\":\"%d\",",            COLUMN_CLASS_STOCKS_TREND,  COL_NWDW        },
	{ "BIX",           "\"406\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_BIX         },
	{ "5AD21Q1",       "\"418\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_5AD21Q1     },
	{ "5AD21Q2",       "\"419\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_5AD21Q2     },
	{ "5AD20Q1",       "\"420\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_5AD20Q1     },
	{ "5AD20Q2",       "\"421\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_5AD20Q2     },
	{ "5AD19Q1",       "\"422\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_5AD19Q1     },
	{ "5AD19Q2",       "\"423\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_5AD19Q2     },
	{ "10AD21Q1",      "\"424\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_10AD21Q1    },
	{ "10AD21Q2",      "\"425\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_10AD21Q2    },
	{ "10AD20Q1",      "\"426\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_10AD20Q1    },
	{ "10AD20Q2",      "\"427\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_10AD20Q2    },
	{ "10AD19Q1",      "\"428\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_10AD19Q1    },
	{ "10AD19Q2",      "\"429\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_10AD19Q2    },
	{ "5AD21",         "\"430\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_5AD21       },
	{ "5AD20",         "\"431\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_5AD20       },
	{ "5AD19",         "\"432\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_5AD19       },
	{ "10AD21",        "\"433\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_10AD21      },
	{ "10AD20",        "\"434\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_10AD20      },
	{ "10AD19",        "\"435\":\"%.2f\",",          COLUMN_CLASS_STOCKS_TREND,  COL_10AD19      },
	/* Fundamentals */
	{ "fED",           "\"300\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_ED          },
	{ "fEDOFF",        "\"313\":\"%d\"",             COLUMN_CLASS_STOCKS_FUND,   COL_EDOFF       },
	{ "fSEC",          "\"301\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_SEC         },
	{ "fDY",           "\"302\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_DY          },
	{ "fDIV",          "\"303\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_DIV         },
	{ "fXD",           "\"304\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_XD          },
	{ "fAR",           "\"305\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_AR          },
	{ "fEPS",          "\"306\":\"%.2f\"",           COLUMN_CLASS_STOCKS_FUND,   COL_EPS         },
	{ "fVAR",          "\"307\":\"%.2f\"",           COLUMN_CLASS_STOCKS_FUND,   COL_EPS         },
	{ "fMCP",          "\"308\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_MCP         },
	{ "fPEG",          "\"309\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_PEG         },
	{ "fPBR",          "\"310\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_PBR         },
	{ "fPM",           "\"311\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_PM          },
	{ "fROA",          "\"312\":\"%s\"",             COLUMN_CLASS_STOCKS_FUND,   COL_ROA         },
	/* Algorithmic */
	{ "days5pc",       "\"200\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_DAYS_5PC    },
	{ "days10pc",      "\"201\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_DAYS_10PC   },
	{ "days15pc",      "\"202\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_DAYS_15PC   },
	{ "days20pc",      "\"203\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_DAYS_20PC   },
	{ "ret5pc",        "\"204\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_RET_5PC     },
	{ "ret10pc",       "\"205\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_RET_10PC    },
	{ "ret15pc",       "\"206\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_RET_15PC    },
	{ "ret20pc",       "\"207\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_RET_20PC    },
	{ "max5pc",        "\"208\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_RET_5PC     },
	{ "max10pc",       "\"209\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_RET_10PC    },
	{ "max15pc",       "\"210\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_RET_15PC    },
	{ "max20pc",       "\"211\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_RET_20PC    },
	{ "a1_esp",        "\"212\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_A1_ESP      },
	{ "a1",            "\"213\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_A1          },
	{ "a1_same",       "\"214\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_A1SAME      },
	{ "a1_result",     "\"215\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_A1RESULT    },
	{ "a4_esp",        "\"216\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_A4ESP       },
	{ "a4",            "\"217\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_A4          },
	{ "1d",            "\"218\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_1D          },
	{ "3d",            "\"219\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_3D          },
	{ "5d",            "\"220\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_5D          },
	{ "8d",            "\"221\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_8D          },
	{ "13d",           "\"222\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_13D         },
	{ "21d",           "\"223\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_21D         },
	{ "42d",           "\"224\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_42D         },
	{ "63d",           "\"225\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_63D         },
	{ "RTD",           "\"226\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_RTD         },
	{ "STREAK",        "\"227\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_STREAK      },
	{ "DIR",           "\"228\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_DIR         },
	{ "BUY",           "\"229\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_BUY         },
	{ "BUY_DELTA",     "\"230\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_BUY_DELTA   },
	{ "SELL",          "\"231\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SELL        },
	{ "SELL_DELTA",    "\"232\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SELL_DELTA  },
	{ "FIB",           "\"233\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_FIB         },
	{ "FIB_DIR",       "\"234\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_FIB_DIR     },
	{ "BUY_FIB",       "\"235\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_BUY_FIB     },
	{ "DELTA_FIB",     "\"236\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_DELTA_FIB   },
	{ "SELL_FIB",      "\"237\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SELL_FIB    },
	{ "SDT_FIB",       "\"238\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SDT_FIB     },
	{ "MEAN",          "\"239\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MEAN        },
	{ "STD",           "\"240\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_STD         },
	{ "VAR90",         "\"241\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_VAR90       },
	{ "VAR95",         "\"242\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_VAR95       },
	{ "VAR99",         "\"243\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_VAR99       },
	{ "VAR99P",        "\"244\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_VAR99P      },
	{ "1_YEAR_AGO",    "\"245\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_1_YEAR_AGO  },
	{ "1_YEAR_PKPC",   "\"246\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_1_YEAR_PKPC },
	{ "YTD",           "\"247\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_YTD         },
	{ "SIG_21_A1",     "\"248\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SIG_21_A1   },
	{ "SIG_42_A1",     "\"249\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SIG_42_A1   },
	{ "SIG_63_A1",     "\"250\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SIG_63_A1   },
	{ "MXD_5PC_21_A1", "\"251\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_5PC_21_A1  },
	{ "MXD_5PC_42_A1", "\"252\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_5PC_42_A1  },
	{ "MXD_5PC_63_A1", "\"253\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_5PC_63_A1  },
	{ "SCS_5PC_21_A1", "\"254\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_5PC_21_A1  },
	{ "SCS_5PC_42_A1", "\"255\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_5PC_42_A1  },
	{ "SCS_5PC_63_A1", "\"256\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_5PC_63_A1  },
	{ "MXD_10PC_21_A1","\"257\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_10PC_21_A1 },
	{ "MXD_10PC_42_A1","\"258\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_10PC_42_A1 },
	{ "MXD_10PC_63_A1","\"259\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_10PC_63_A1 },
	{ "SCS_10PC_21_A1","\"260\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_10PC_21_A1 },
	{ "SCS_10PC_42_A1","\"261\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_10PC_42_A1 },
	{ "SCS_10PC_63_A1","\"262\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_10PC_63_A1 },
	{ "SIG_21_A4",     "\"263\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SIG_21_A4          },
	{ "SIG_42_A4",     "\"264\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SIG_42_A4          },
	{ "SIG_63_A4",     "\"265\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SIG_63_A4          },
	{ "MXD_5PC_21_A4", "\"266\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_5PC_21_A4  },
	{ "MXD_5PC_42_A4", "\"267\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_5PC_42_A4  },
	{ "MXD_5PC_63_A4", "\"268\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_5PC_63_A4  },
	{ "SCS_5PC_21_A4", "\"269\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_5PC_21_A4  },
	{ "SCS_5PC_42_A4", "\"270\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_5PC_42_A4  },
	{ "SCS_5PC_63_A4", "\"271\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_5PC_63_A4  },
	{ "MXD_10PC_21_A4","\"272\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_10PC_21_A4 },
	{ "MXD_10PC_42_A4","\"273\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_10PC_42_A4 },
	{ "MXD_10PC_63_A4","\"274\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_MAXDAYS_10PC_63_A4 },
	{ "SCS_10PC_21_A4","\"275\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_10PC_21_A4 },
	{ "SCS_10PC_42_A4","\"276\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_10PC_42_A4 },
	{ "SCS_10PC_63_A4","\"277\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SUCCESS_10PC_63_A4 },
	{ "1_YEAR_PKPR",   "\"278\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_1_YEAR_PKPR        },
	{ "PLIMIT",        "\"279\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_PLIMIT             },
	{ "PDELTA",        "\"280\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_PDELTA             },
	{ "ONE_YR_P10",    "\"281\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_ONE_YR_P10         },
	{ "ONE_YR_P5",     "\"282\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_ONE_YR_P5          },
	{ "LAST_PEAK",     "\"283\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_LAST_PEAK          },
	{ "P10ORP",        "\"284\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_P10ORP5            },
	{ "PEAK2",         "\"285\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_PEAK2              },
	{ "SIG",           "\"286\":\"%.2f\"",             COLUMN_CLASS_STOCKS_ALGO,   COL_SIG                },
	{ "VOL_ADI",       "\"109\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_ADI            },
	{ "VOL_OBV",       "\"110\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_OBV            },
	{ "VOL_CMF",       "\"111\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_CMF            },
	{ "VOL_FI",        "\"112\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_FI             },
	{ "VOL_MFI",       "\"113\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_MFI            },
	{ "VOL_EM",        "\"114\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_EM             },
	{ "VOL_VPT",       "\"115\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_VPT            },
	{ "VOL_NVI",       "\"116\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_NVI            },
	{ "VOL_VWAP",      "\"117\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_VWAP           },
	{ "VOL_ATR",       "\"118\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_ATR            },
	{ "VOL_BBM",       "\"119\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_BBM            },
	{ "VOL_BBH",       "\"120\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_BBH            },
	{ "VOL_BBL",       "\"121\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_BBL            },
	{ "VOL_BBW",       "\"122\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_BBW            },
	{ "VOL_BBP",       "\"123\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_BBP            },
	{ "VOL_BBHI",      "\"124\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_BBHI           },
	{ "VOL_BBLI",      "\"125\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_BBLI           },
	{ "VOL_KCC",       "\"126\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_KCC            },
	{ "VOL_KCL",       "\"127\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_KCL            },
	{ "VOL_KCH",       "\"128\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_KCH            },
	{ "VOL_KCW",       "\"129\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_KCW            },
	{ "VOL_KCP",       "\"130\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_KCP            },
	{ "VOL_KCHI",      "\"131\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_KCHI           },
	{ "VOL_KCLI",      "\"132\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_KCLI           },
	{ "VOL_DCL",       "\"133\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_DCL            },
	{ "VOL_DCH",       "\"134\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_DCM            },
	{ "VOL_DCM",       "\"135\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_DCM            },
	{ "VOL_DCW",       "\"136\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_DCW            },
	{ "VOL_DCP",       "\"137\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_DCP            },
	{ "VOL_UI",        "\"138\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_VOL_UI             },
	{ "TR_MACD",       "\"139\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_MACD            },
	{ "TR_MACD_SIG",   "\"140\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_MACD_SIG        },
	{ "TR_MACD_DIFF",  "\"141\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_MACD_DIFF       },
	{ "TR_SMA_FAST",   "\"142\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_SMA_FAST        },
	{ "TR_SMA_SLOW",   "\"143\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_SMA_SLOW        },
	{ "TR_EMA_SLOW",   "\"144\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_EMA_SLOW        },
	{ "TR_ADX",        "\"145\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_ADX             },
	{ "TR_ADX_POS",    "\"146\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_ADX_POS         },
	{ "TR_ADX_NEG",    "\"147\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_ADX_NEG         },
	{ "TR_VTX_POS",    "\"148\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_VTX_POS         },
	{ "TR_VTX_NEG",    "\"149\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_VTX_NEG         },
	{ "TR_VTX_DIFF",   "\"150\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_VTX_DIFF        },
	{ "TR_TRIX",       "\"151\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_TRIX            },
	{ "TR_MASS_IDX",   "\"152\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_MASS_IDX        },
	{ "TR_CCI",        "\"153\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_CCI             },
	{ "TR_DPO",        "\"154\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_DPO             },
	{ "TR_KST",        "\"155\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_KST             },
	{ "TR_KST_SIG",    "\"156\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_KST_SIG         },
	{ "TR_KST_DIFF",   "\"157\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_KST_DIFF        },
	{ "TR_ICH_CONV",   "\"158\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_ICH_CONV        },
	{ "TR_ICH_BASE",   "\"159\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_ICH_BASE        },
	{ "TR_ICH_A",      "\"160\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_ICH_A           },
	{ "TR_ICH_B",      "\"161\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_ICH_B           },
	{ "TR_VICH_A",     "\"162\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_VICH_A          },
	{ "TR_VICH_B",     "\"163\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_VICH_B          },
	{ "TR_AROON_UP",   "\"164\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_AROON_UP        },
	{ "TR_AROON_DOWN", "\"165\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_AROON_DOWN      },
	{ "TR_AROON_IND",  "\"166\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_AROON_IND       },
	{ "TR_PSAR_UP",    "\"167\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_PSAR_UP         },
	{ "TR_PSAR_DOWN",  "\"168\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_PSAR_UP         },
	{ "TR_PSAR_UP_IND","\"169\":\"%.2f\"",             COLUMN_CLASS_STOCKS_INDI,   COL_TR_PSAR_UP         },
	{ "TR_PSAR_DOWN_IND",  "\"170\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_PSAR_DOWN_IND   },
	{ "TR_STC",            "\"171\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_STC             },
	{ "TR_MOM_RSI",        "\"172\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_RSI         },
	{ "TR_MOM_STOCH_RSI",  "\"173\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_STOCH_RSI   },
	{ "TR_MOM_STOCH_RSI_K","\"174\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_STOCH_RSI_K },
	{ "TR_MOM_STOCH_RSI_D","\"175\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_STOCH_RSI_D },
	{ "TR_MOM_TSI",        "\"176\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_TSI         },
	{ "TR_MOM_UO",         "\"177\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_UO          },
	{ "TR_MOM_STOCH_K",    "\"178\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_STOCH_K     },
	{ "TR_MOM_STOCH_D",    "\"179\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_STOCH_D     },
	{ "TR_MOM_WR",         "\"180\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_WR          },
	{ "TR_MOM_AO",         "\"181\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_AO          },
	{ "TR_MOM_KAMA",       "\"182\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_KAMA        },
	{ "TR_MOM_ROC",        "\"183\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_ROC         },
	{ "TR_MOM_PPO",        "\"184\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_PPO         },
	{ "TR_MOM_PPO_SIG",    "\"185\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_PPO_SIG     },
	{ "TR_MOM_PPO_HIST",   "\"186\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_PPO_HIST    },
	{ "TR_MOM_OTHERS_DR",  "\"187\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_OTHERS_DR   },
	{ "TR_MOM_OTHERS_DLR", "\"188\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_OTHERS_DLR  },
	{ "TR_MOM_OTHERS_CR",  "\"189\":\"%.2f\"",         COLUMN_CLASS_STOCKS_INDI,   COL_TR_MOM_OTHERS_CR   }
};

void init_watchtable()
{
	int   column_name_int, column_id;

	if (watchtable_initialized)
		return;

	for (int x=0; x<NR_BUILTIN_COLUMNS; x++) {
		struct column      *column = (struct column *)zmalloc(sizeof(*column));
		struct column_hash *chash  = (struct column_hash *)zmalloc(sizeof(*chash));

		chash->column         = column;
		chash->column_id      = builtin_columns[x].column_id;
		memcpy(column, &builtin_columns[x], sizeof(*column));
		HASH_ADD_INT(COLUMN_HASHTABLE_INT, column_id, chash);
		switch(column->column_class) {
			case COLUMN_CLASS_STOCKS_COMMON:
				column->cb = stock_get_common;
				break;
			case COLUMN_CLASS_STOCKS_TREND:
				column->cb = stock_get_trend;
				break;
			case COLUMN_CLASS_STOCKS_FUND:
				column->cb = stock_get_fundamentals;
				break;
			case COLUMN_CLASS_STOCKS_ALGO:
				column->cb = stock_get_algo;
				break;
		}
	}
	watchtable_initialized = true;
}

static __inline__ void watchtable_path(struct session *session, char *path)
{
	if (!session->user->logged_in)
		snprintf(path, 64, "db/uid/cookie/%s.wtab", session->filecookie);
	else
		snprintf(path, 32, "db/uid/%d.wtab", session->user->uid);
}

int column_sprintf(struct session *session, struct watchlist *watchlist, struct stock *stock, struct wtab *wtab, char *packet, int packet_len, int entry)
{
	struct mag         *mag   = stock->mag;
	struct column      *column;
	char               *column_str, *curdict, *p, *p2;
	struct column_value cvalue;
	int                 x, nbytes, nr_columns, nr_entries, column_id;

	if (!mag)
		return 0;

	nr_entries = (mag->nr_entries-1);
	if (entry < 0 || entry > nr_entries)
		return 0;

	nr_columns = wtab->nr_columns;

	cvalue.entry      = entry;
	cvalue.nr_entries = nr_entries;
	cvalue.mag        = mag;
	cvalue.m2         = (stock->mag2 ? &stock->mag2[nr_entries-entry] : NULL);
	cvalue.m3         = (stock->mag3 ? &stock->mag3[nr_entries-entry] : NULL);
	cvalue.m4         = (stock->mag4 ? &stock->mag4[nr_entries-entry] : NULL);

	*(packet+packet_len) = '{';
	packet_len  += 1;
	cvalue.stock = stock;
	for (int x = 0; x<wtab->nr_columns; x++) {
		struct column_hash *chash = NULL;

		column_id = wtab->colmap[x];
		HASH_FIND_INT(COLUMN_HASHTABLE_INT, &column_id, chash);
		if (!chash)
			return 0;
		column      = chash->column;
		column->cb(&cvalue, column_id);
		switch (cvalue.value_type) {
			case VALUE_TYPE_STR:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (char *)cvalue.value.str);
				break;
			case VALUE_TYPE_DOUBLE:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (double)cvalue.value.DOUBLE);
				break;
			case VALUE_TYPE_INT64:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (int64_t)cvalue.value.int64);
				break;
			case VALUE_TYPE_UINT64:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (uint64_t)cvalue.value.uint64);
				break;
			case VALUE_TYPE_INT32:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (int)cvalue.value.int32);
				break;
			case VALUE_TYPE_UINT32:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (unsigned int)cvalue.value.uint32);
				break;
			case VALUE_TYPE_INT16:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (short)cvalue.value.int16);
				break;
			case VALUE_TYPE_UINT16:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (unsigned short)cvalue.value.uint16);
				break;
			case VALUE_TYPE_INT8:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (char)cvalue.value.int8);
				break;
			case VALUE_TYPE_UINT8:
				nbytes = snprintf(packet+packet_len, 256, column->column_fmt_str, (unsigned char)cvalue.value.uint8);
				break;
		}
		packet_len += nbytes;
	}
	*(packet+packet_len-1) = '}';
	packet[packet_len] = 0;
	printf("packet: %s\n", packet);
	return (packet_len);
}

int watchtable_packet(struct session *session, struct watchlist *watchlist, char *packet)
{
	struct watchstock *watchstock;
	struct wtab       *wtab;
	struct stock      *stock;
	int                nr_stocks, packet_len;

	if (!watchlist->nr_stocks) 
		return 0;

	packet_len = snprintf(packet, 32, "table %s [", watchlist->basetable);  // a watchlist is inside a basetable, use the ID of the table which is in T[table_name]
	wtab       = watchlist->wtab;
	/* Assign default watchtable preset */
	if (!wtab) {
		printf(BOLDRED "watchtable packet NULL for watchlist: %s" RESET "\n\n\n\n\n\n", watchlist->name);
		watchlist->wtab = wtab = default_watchtable_preset();
	}
	nr_stocks = watchlist->nr_stocks;
	printf("watchtable_packet(): wtab->name: %s watchlist->name %s nstocks: %d wtab->nr_columns: %d packet: %s (%p) plen: %d\n", wtab->name, watchlist->name, nr_stocks, wtab->nr_columns, packet, packet, packet_len);
	for (int x=0; x<nr_stocks; x++) {
		watchstock = &watchlist->stocks[x];
		stock      = watchstock->stock;
		if (!stock || !stock->mag)
			continue;
		packet_len = column_sprintf(session, watchlist, stock, wtab, packet, packet_len, 0);
		*(packet+packet_len++) = ',';
	}
	packet[packet_len-1] = ']';
	packet[packet_len] = 0;
	if (verbose)
		printf(BOLDWHITE "%s" RESET "\n", packet);
	return (packet_len);
}

int map_dict(struct wtab *wtab, char *dict, int nr_columns)
{
	char *key;
	int x, count = 0;

	// [{"data":null,"orderable":false,defaultContent:""},{"data":"100"},{"data":"101"}]
	key = dict + 62;
	for (x=0; x<nr_columns; x++) {
		wtab->colmap[x] = atoi(key);
		key += 15;
		count++;
	}
	if (count != nr_columns) {
		printf(BOLDRED "nr_columns: %d count: %d failure" RESET "\n", nr_columns, count);
		goto out_error;
	}
	wtab->nr_columns = nr_columns;
	return 1;
out_error:
	free(wtab);
	return 0;
}
/* watchtable column creator: (dict, name, nr_columns) */
void rpc_watchtable_columns(struct rpc *rpc)
{
	struct session   *session       = rpc->session;
	char             *packet        = rpc->packet;
	char             *watchtable_id = rpc->argv[1];
	char             *dict          = rpc->argv[2];
	char             *preset_name   = rpc->argv[3];
	int               nr_columns    = atoi(rpc->argv[4]);
	int               dictsize      = strlen(dict);
	int               namesize      = strlen(preset_name);
	struct watchlist *watchlist, *origin, *watchtable;
	struct wtab      *wtab;
	char              path[256];
	int               x, fd, packet_len, newtab_len, nr_presets, new_preset = 0, tmp = 0, position = 0;

	if (dictsize >= MAX_WTAB_SIZE || namesize >= MAX_WTAB_NAME || nr_columns <= 0 || nr_columns >= MAX_WTAB_COLUMNS)
		return;

	printf("watchtable_id: %s\n", watchtable_id);
	watchtable = search_watchtable(session, watchtable_id);
	if (!watchtable) {
		printf("creating new watchtable\n");
		watchtable = watchtable_create(session, watchtable_id, watchtable_id);
		if (!watchtable)
			return;
	}

	origin     = session->morphtab->origin;
	nr_presets = session->nr_watchtable_presets;
	newtab_len = snprintf(packet, MAX_WTAB_SIZE+32, "newtab %s %s@", session->morphtab->basetable, dict);

	printf("watchtable_preset(): %s preset_name: %s nr_columns: %d" BOLDGREEN " nr_presets: %d" RESET "\n", dict, preset_name, nr_columns,nr_presets);
	wtab = search_watchtable_preset(session, preset_name, &position);
	if (!wtab) {
		new_preset = 1;
		wtab       = (struct wtab *)zmalloc(sizeof(*wtab)); // can't be New^Preset
		printf("name: %s not found, allocating new\n", preset_name);
	} else {
		if (origin)
			origin->wtab = wtab;
		else
			session->morphtab->wtab = wtab; // if not NULL then wtab RPC is saving a real named preset
		printf("search_watchtable_preset: %s success %s\n", preset_name, wtab->name);
	}
	strncpy(wtab->name, preset_name, MAX_WTAB_NAME);
	strncpy(wtab->dict, dict,        MAX_WTAB_SIZE);
	if (!map_dict(wtab, dict, nr_columns))
		return;
	if (!strcmp(wtab->name, "New^Preset")) {
		printf("!strcmp: %s\n", wtab->name);
		goto out;
	}
	watchtable_path(session, path);
	printf("writing to path: %s\n", path);
	fd = open(path, O_RDWR|O_CREAT, 0644);
	if (!new_preset)
		lseek(fd, position*sizeof(struct wtab), SEEK_SET);
	else
		lseek(fd, 0, SEEK_END);
	printf(BOLDRED "WRITING PRESET new: %d" RESET "\n", new_preset);
	write(fd, wtab, sizeof(*wtab));
	close(fd);

	/* Add New struct preset when saving column presets */
	if (new_preset) {
//		mutex_lock(&session->watchlist_lock);
		wtab->id = session->nr_watchtable_presets;
		session->watchtable_presets[session->nr_watchtable_presets++] = wtab;
		if (origin)
			origin->wtab = wtab;
		else
			session->morphtab->wtab = wtab;
//		mutex_unlock(&session->watchlist_lock);
		printf(BOLDYELLOW "ADDED NEW PRESET: %s: %d" RESET "\n",preset_name,session->nr_watchtable_presets);
	}
out:
	if ((origin && origin->nr_stocks) || session->morphtab->nr_stocks) {
		if (session->morphtab->origin)
			watchlist = session->morphtab->origin;
		else
			watchlist = session->morphtab;
		if ((packet_len=watchtable_packet(session, watchlist, packet+newtab_len)) > 0) {
			websocket_send(rpc->connection, packet+newtab_len, packet_len);
			*(packet+newtab_len+packet_len) = 0;
			printf("watchtTABLE PACKET: %s nstocks: %d packet_len: %d strlen: %d\n", packet, session->morphtab->nr_stocks, (int)packet_len, (int)strlen(packet+newtab_len));
			workspace_broadcast(session, rpc->connection, packet, packet_len+newtab_len);
		}
	}
}

static void
stock_get_common(struct column_value *cvalue, int column_id)
{
	struct stock *stock      = cvalue->stock;
	struct mag   *mag        = stock->mag;
	int           nr_entries = cvalue->nr_entries;
	int           entry      = cvalue->entry;

	switch (column_id) {
		case COL_SYMBOL:
			cvalue->value.str    = stock->sym;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_DATE:
			cvalue->value.str    = mag->date[nr_entries-entry];
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_PRICE:
			cvalue->value.DOUBLE = get_stock_price(stock, entry, nr_entries);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOLUME:
			cvalue->value.str    = get_stock_volume(stock, entry, nr_entries, cvalue->vstr);
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_DELTA:
			cvalue->value.DOUBLE = get_stock_delta(stock, entry, nr_entries);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_OPEN:
			cvalue->value.DOUBLE = get_stock_open(stock, entry, nr_entries);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_HIGH:
			cvalue->value.DOUBLE = get_stock_high(stock, entry, nr_entries);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_LOW:
			cvalue->value.DOUBLE = get_stock_low(stock, entry, nr_entries);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_PC:
			cvalue->value.DOUBLE = get_stock_prior_close(stock, entry, nr_entries);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_OP:
			cvalue->value.DOUBLE = get_stock_openpc(stock, entry, nr_entries);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_HP:
			cvalue->value.DOUBLE = get_stock_highpc(stock, entry, nr_entries);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_LP:
			cvalue->value.DOUBLE = get_stock_lowpc(stock, entry, nr_entries);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
	}
}

static void
stock_get_trend(struct column_value *cvalue, int column_id)
{
	struct stock *stock      = cvalue->stock;
	struct mag   *mag        = cvalue->mag;
	int           nr_entries = cvalue->nr_entries;
	int           entry      = cvalue->entry;

	switch (column_id) {
		case COL_RANK:
			cvalue->value.int32 = date_to_rank(stock, mag->date[nr_entries-entry]);
			cvalue->value_type   = VALUE_TYPE_INT32;
			break;
		case COL_PK:
			cvalue->value.DOUBLE = mag->peak_yr[nr_entries-entry];
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_PKP:
			cvalue->value.DOUBLE = mag->peak_pc[nr_entries-entry];
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_NDDW:
			cvalue->value.int32 = stock->nr_days_down;
			cvalue->value_type   = VALUE_TYPE_INT32;
			break;
		case COL_NDUP:
			cvalue->value.int32  = stock->nr_days_up;
			cvalue->value_type   = VALUE_TYPE_INT32;
			break;
		case COL_NWDW:
			cvalue->value.int32  = mag->nr_weeks_down;
			cvalue->value_type   = VALUE_TYPE_INT32;
		case COL_NWUP:
			cvalue->value.int32  = mag->nr_weeks_up;
			cvalue->value_type   = VALUE_TYPE_INT32;
			break;
		case COL_PNDDW:
			cvalue->value.DOUBLE = stock->pc_days_down;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_PNDUP:
			cvalue->value.DOUBLE = stock->pc_days_up;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_BIX:
			cvalue->value.DOUBLE = mag->BIX[entry];
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5AD21Q1:
			cvalue->value.DOUBLE = anyday_column(mag, COL_5AD21Q1);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5AD21Q2:
			cvalue->value.DOUBLE = anyday_column(mag, COL_5AD21Q2);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5AD20Q1:
			cvalue->value.DOUBLE = anyday_column(mag, COL_5AD20Q1);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5AD20Q2:
			cvalue->value.DOUBLE = anyday_column(mag, COL_5AD20Q2);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5AD19Q1:
			cvalue->value.DOUBLE = anyday_column(mag, COL_5AD19Q1);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5AD19Q2:
			cvalue->value.DOUBLE = anyday_column(mag, COL_5AD19Q2);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_10AD21Q1:
			cvalue->value.DOUBLE = anyday_column(mag, COL_10AD21Q1);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_10AD21Q2:
			cvalue->value.DOUBLE = anyday_column(mag, COL_10AD21Q2);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_10AD20Q1:
			cvalue->value.DOUBLE = anyday_column(mag, COL_10AD20Q1);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_10AD20Q2:
			cvalue->value.DOUBLE = anyday_column(mag, COL_10AD20Q2);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_10AD19Q1:
			cvalue->value.DOUBLE = anyday_column(mag, COL_10AD19Q1);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_10AD19Q2:
			cvalue->value.DOUBLE = anyday_column(mag, COL_10AD19Q2);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5AD21:
			cvalue->value.DOUBLE = anyday_column(mag, COL_5AD21);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5AD20:
			cvalue->value.DOUBLE = anyday_column(mag, COL_5AD20);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5AD19:
			cvalue->value.DOUBLE = anyday_column(mag, COL_5AD19);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_10AD21:
			cvalue->value.DOUBLE = anyday_column(mag, COL_10AD21);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_10AD20:
			cvalue->value.DOUBLE = anyday_column(mag, COL_10AD20);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_10AD19:
			cvalue->value.DOUBLE = anyday_column(mag, COL_10AD19);
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
	}
}

static void
stock_get_algo(struct column_value *cvalue, int column_id)
{
	struct stock *stock      = cvalue->stock;
	struct mag2  *m2         = cvalue->m2;
	int           nr_entries = cvalue->nr_entries;
	int           entry      = cvalue->entry;
	char          timestr[64];

	if (!m2)
		return;

	switch (column_id) {
		case COL_DAYS_5PC:
			cvalue->value.DOUBLE = m2->days_5pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_DAYS_10PC:
			cvalue->value.DOUBLE = m2->days_10pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_DAYS_15PC:
			cvalue->value.DOUBLE = m2->days_15pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_DAYS_20PC:
			cvalue->value.DOUBLE = m2->days_20pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_RET_5PC:
			cvalue->value.DOUBLE = m2->days_ret_5pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_RET_10PC:
			cvalue->value.DOUBLE = m2->days_ret_10pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_RET_15PC:
			cvalue->value.DOUBLE = m2->days_ret_15pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_RET_20PC:
			cvalue->value.DOUBLE = m2->days_ret_20pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAX_5PC:
			cvalue->value.DOUBLE = m2->days_max_5pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAX_10PC:
			cvalue->value.DOUBLE = m2->days_max_10pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAX_15PC:
			cvalue->value.DOUBLE = m2->days_max_15pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAX_20PC:
			cvalue->value.DOUBLE = m2->days_max_20pc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_A1_ESP:
			cvalue->value.DOUBLE = m2->a1esp;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_A1:
			cvalue->value.DOUBLE = m2->action1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_A1SAME:
			cvalue->value.DOUBLE = m2->a1same;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_A1RESULT:
			cvalue->value.DOUBLE = m2->a1result;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_A4ESP:
			cvalue->value.DOUBLE = m2->a4esp;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_A4:
			cvalue->value.DOUBLE = m2->action4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_1D:
			cvalue->value.DOUBLE = m2->day1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_3D:
			cvalue->value.DOUBLE = m2->day3;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_5D:
			cvalue->value.DOUBLE = m2->day5;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_8D:
			cvalue->value.DOUBLE = m2->day8;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_13D:
			cvalue->value.DOUBLE = m2->day13;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_21D:
			cvalue->value.DOUBLE = m2->day21;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_42D:
			cvalue->value.DOUBLE = m2->day42;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_63D:
			cvalue->value.DOUBLE = m2->day63;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_RTD:
			cvalue->value.DOUBLE = m2->RTD;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_STREAK:
			cvalue->value.DOUBLE = m2->streak;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_DIR:
			cvalue->value.DOUBLE = m2->dir;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_BUY:
			cvalue->value.DOUBLE = m2->buy;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_BUY_DELTA:
			cvalue->value.DOUBLE = m2->buy_delta;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SELL:
			cvalue->value.DOUBLE = m2->sell;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SELL_DELTA:
			cvalue->value.DOUBLE = m2->sell_delta;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_FIB:
			cvalue->value.DOUBLE = m2->fib;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_FIB_DIR:
			cvalue->value.DOUBLE = m2->fib_dir;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_BUY_FIB:
			cvalue->value.DOUBLE = m2->buy_fib;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_DELTA_FIB:
			cvalue->value.DOUBLE = m2->buy_delta_fib;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SELL_FIB:
			cvalue->value.DOUBLE = m2->sell_fib;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SDT_FIB:
			cvalue->value.DOUBLE = m2->sell_delta_fib;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MEAN:
			cvalue->value.DOUBLE = m2->mean;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_STD:
			cvalue->value.DOUBLE = m2->std;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VAR90:
			cvalue->value.DOUBLE = m2->var_90;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VAR95:
			cvalue->value.DOUBLE = m2->var_95;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VAR99:
			cvalue->value.DOUBLE = m2->var_99;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VAR99P:
			cvalue->value.DOUBLE = m2->var_99pot;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_1_YEAR_AGO:
			cvalue->value.DOUBLE = m2->one_year_ago;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_1_YEAR_PKPC:
			cvalue->value.DOUBLE = m2->one_year_pkpc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_YTD:
			cvalue->value.DOUBLE = m2->YTD;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SIG_21_A1:
			cvalue->value.DOUBLE = m2->sig_21_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SIG_42_A1:
			cvalue->value.DOUBLE = m2->sig_42_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SIG_63_A1:
			cvalue->value.DOUBLE = m2->sig_63_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_5PC_21_A1:
			cvalue->value.DOUBLE = m2->max_days_5pc_21_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_5PC_42_A1:
			cvalue->value.DOUBLE = m2->max_days_5pc_42_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_5PC_63_A1:
			cvalue->value.DOUBLE = m2->max_days_5pc_63_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_5PC_21_A1:
			cvalue->value.DOUBLE = m2->success__5pc_21_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_5PC_42_A1:
			cvalue->value.DOUBLE = m2->success__5pc_42_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_5PC_63_A1:
			cvalue->value.DOUBLE = m2->success__5pc_63_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;

		case COL_MAXDAYS_10PC_21_A1:
			cvalue->value.DOUBLE = m2->max_days_10pc_21_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_10PC_42_A1:
			cvalue->value.DOUBLE = m2->max_days_10pc_42_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_10PC_63_A1:
			cvalue->value.DOUBLE = m2->max_days_10pc_63_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_10PC_21_A1:
			cvalue->value.DOUBLE = m2->success__10pc_21_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_10PC_42_A1:
			cvalue->value.DOUBLE = m2->success__10pc_42_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_10PC_63_A1:
			cvalue->value.DOUBLE = m2->success__10pc_63_a1;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SIG_21_A4:
			cvalue->value.DOUBLE = m2->sig_21_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SIG_42_A4:
			cvalue->value.DOUBLE = m2->sig_42_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SIG_63_A4:
			cvalue->value.DOUBLE = m2->sig_63_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_5PC_21_A4:
			cvalue->value.DOUBLE = m2->max_days_5pc_21_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_5PC_42_A4:
			cvalue->value.DOUBLE = m2->max_days_5pc_42_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_5PC_63_A4:
			cvalue->value.DOUBLE = m2->max_days_5pc_63_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_5PC_21_A4:
			cvalue->value.DOUBLE = m2->success__5pc_21_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_5PC_42_A4:
			cvalue->value.DOUBLE = m2->success__5pc_42_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_5PC_63_A4:
			cvalue->value.DOUBLE = m2->success__5pc_63_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_10PC_21_A4:
			cvalue->value.DOUBLE = m2->max_days_10pc_21_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_10PC_42_A4:
			cvalue->value.DOUBLE = m2->max_days_10pc_42_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MAXDAYS_10PC_63_A4:
			cvalue->value.DOUBLE = m2->max_days_10pc_63_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_10PC_21_A4:
			cvalue->value.DOUBLE = m2->success__10pc_21_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_10PC_42_A4:
			cvalue->value.DOUBLE = m2->success__10pc_42_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_SUCCESS_10PC_63_A4:
			cvalue->value.DOUBLE = m2->success__10pc_63_a4;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_1_YEAR_PKPR:
			cvalue->value.DOUBLE = m2->one_year_pk_price;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_PLIMIT:
			cvalue->value.DOUBLE = m2->plimit;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_PDELTA:
			cvalue->value.DOUBLE = m2->pdelta;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_ONE_YR_P10:
			cvalue->value.DOUBLE = m2->one_yr_p10;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_ONE_YR_P5:
			cvalue->value.DOUBLE = m2->one_yr_p5;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_LAST_PEAK:
			cvalue->value.str    = unix2str(m2->last_peak, timestr);
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_P10ORP5:
			cvalue->value.DOUBLE = m2->p10orp5;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_PEAK2:
			cvalue->value.str    = (char *)PEAK(m2->peak);
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_SIG:
			cvalue->value.str    = (char *)SIGNAL(m2->sig);
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
	}
}

static void
stock_get_fundamentals(struct column_value *cvalue, int column_id)
{
	struct stock *stock      = cvalue->stock;
	int           nr_entries = cvalue->nr_entries;
	int           entry      = cvalue->entry;

	switch (column_id) {
		case COL_ED:
			cvalue->value.str    = "XX";
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_EDOFF:
			cvalue->value.int32  = stock->earnings.earning_days;
			cvalue->value_type   = VALUE_TYPE_INT32;
			break;
		case COL_SEC:
			cvalue->value.str    = stock->sector;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_DY:
			cvalue->value.str    = stock->fund_str.forward_annual_div_yield;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_DIV:
			cvalue->value.str    = stock->fund_str.div_date;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_XD:
			cvalue->value.str    = stock->fund_str.xdiv_date;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_AR:
			cvalue->value.str    = stock->fund_str.revenue;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_EPS:
			cvalue->value.str    = stock->fund_str.revenue;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_MCP:
			cvalue->value.str    = stock->fund_str.market_cap;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_PEG:
			cvalue->value.str    = stock->fund_str.peg_ratio;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_PBR:
			cvalue->value.str    = stock->fund_str.PBR;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_PM:
			cvalue->value.str    = stock->fund_str.profit_margin;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
		case COL_ROA:
			cvalue->value.str    = stock->fund_str.return_on_assets;
			cvalue->value_type   = VALUE_TYPE_STR;
			break;
	}
}

static void
stock_get_indi(struct column_value *cvalue, int column_id)
{
	struct stock *stock      = cvalue->stock;
	struct mag3  *m3         = cvalue->m3;
	int           nr_entries = cvalue->nr_entries;
	int           entry      = cvalue->entry;
	char          timestr[64];

	if (!m3)
		return;

	switch (column_id) {
		case COL_VOL_ADI:
			cvalue->value.DOUBLE = m3->volume_adi;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_OBV:
			cvalue->value.DOUBLE = m3->volume_obv;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_CMF:
			cvalue->value.DOUBLE = m3->volume_cmf;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_FI:
			cvalue->value.DOUBLE = m3->volume_fi;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_MFI:
			cvalue->value.DOUBLE = m3->volume_mfi;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_EM:
			cvalue->value.DOUBLE = m3->volume_em;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_VPT:
			cvalue->value.DOUBLE = m3->volume_vpt;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_NVI:
			cvalue->value.DOUBLE = m3->volume_nvi;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_VWAP:
			cvalue->value.DOUBLE = m3->volume_vwap;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_ATR:
			cvalue->value.DOUBLE = m3->vol_atr;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_BBM:
			cvalue->value.DOUBLE = m3->vol_bbm;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_BBH:
			cvalue->value.DOUBLE = m3->vol_bbh;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_BBL:
			cvalue->value.DOUBLE = m3->vol_bbl;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_BBW:
			cvalue->value.DOUBLE = m3->vol_bbw;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_BBP:
			cvalue->value.DOUBLE = m3->vol_bbp;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_BBHI:
			cvalue->value.DOUBLE = m3->vol_bbhi;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_BBLI:
			cvalue->value.DOUBLE = m3->vol_bbli;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_KCC:
			cvalue->value.DOUBLE = m3->vol_kcc;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_KCL:
			cvalue->value.DOUBLE = m3->vol_kcl;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_KCH:
			cvalue->value.DOUBLE = m3->vol_kch;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_KCW:
			cvalue->value.DOUBLE = m3->vol_kcw;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
		case COL_VOL_KCP:
			cvalue->value.DOUBLE = m3->vol_kcp;
			cvalue->value_type   = VALUE_TYPE_DOUBLE;
			break;
	}
}

void rpc_define_table(struct rpc *rpc)
{
	char *URL         = rpc->argv[1];
	char *QGID        = rpc->argv[2];
	char *table_names = rpc->argv[3];
	int   packet_len;

	packet_len = LIBXLS_deftab(rpc->packet, URL, QGID, table_names);
	if (packet_len <= 0)
		return;
	websocket_send(rpc->connection, rpc->packet, packet_len);
}

void rpc_watchtable_bomb(struct rpc *rpc)
{
	struct session *session     = rpc->session;
	char           *preset_name = rpc->argv[1];
	struct wtab    *wtab, *preset;
	struct filemap  filemap;
	char            path[256];
	char           *map;
	int             position = 0, nr_presets;

	watchtable_path(session, path);
//	mutex_lock(&session->watchlist_lock);
	wtab = search_watchtable_preset(session, preset_name, &position);
	if (!wtab || position < 0)
		goto out;
	nr_presets = session->nr_watchtable_presets;
	session->nr_watchtable_presets -= 1;
	/* Only 1 Saved Preset */
	if (nr_presets == 2) {
		session->watchtable_presets[1] = 0;
		fs_truncate(path, 0);
		goto out;
	}
	map = MAP_FILE_RW(path, &filemap);
	if (!map) {
		printf(BOLDRED "watchtable_bomb_preset: fatal mmap: %s" RESET "\n", path);
		goto out;
	}
	preset = (struct wtab *)(map+((position+1)*sizeof(*preset)));

	/* Preset is in the Middle */
	if (position + 1 < (nr_presets-1)) {
		memmove(&session->watchtable_presets[position+1], &session->watchtable_presets[position+2], (nr_presets-2)*8);
		memmove(preset, preset+1, (nr_presets-2)*sizeof(*preset));
	}
	/* If Preset is at the end we just have to decrement filesize, also decrement if Preset is in middle */
	ftruncate(filemap.fd, filemap.filesize-sizeof(preset));
	UNMAP_FILE(map, &filemap);
out:
	return;
//	mutex_unlock(&session->watchlist_lock);
}

void rpc_watchtable_load(struct rpc *rpc)
{
	struct session   *session         = rpc->session;
	char             *preset_name     = rpc->argv[1]; // argv[1]: watchtable column preset name
	char             *watchtable_name = rpc->argv[2]; // argv[2]: watchtable name (HTML ID of the <table>)
	struct watchlist *watchtable      = search_watchtable(session, watchtable_name);
	struct wtab      *preset;
	int               nr_presets, packet_len, x;

	if (!watchtable)
		return;

	nr_presets = session->nr_watchtable_presets;
	for (x=0; x<nr_presets; x++) {
		preset = session->watchtable_presets[x];
		if (!strcmp(preset->name, preset_name)) {
			watchtable->wtab = preset;
			packet_len = watchtable_packet(session, watchtable, rpc->packet);
			if (packet_len)
				websocket_send(rpc->connection, rpc->packet, packet_len);
			return;
		}
	}
}

struct wtab *default_watchtable_preset()
{
	struct wtab *wtab = (struct wtab *)malloc(sizeof(*wtab));
	memset(wtab, 0, sizeof(*wtab));
	wtab->colmap[0]  = 900; wtab->colmap[1]  = 901;
	wtab->colmap[2]  = 903; wtab->colmap[3]  = 904;
	wtab->nr_columns = 4;
	wtab->table_type = WATCHTABLE_ROOT;
	strcpy(wtab->name, "New^Preset");
	return (wtab);
}

int websocket_watchtable_presets(struct session *session, char *packet)
{
	struct wtab *wtab;
	int x, nbytes, packet_len = 0, nr_wtab;

	nr_wtab = session->nr_watchtable_presets;
	if (!nr_wtab)
		return 0;
	for (x=0; x<nr_wtab; x++) {
		wtab = session->watchtable_presets[x];
		if (!wtab || wtab->table_type == WATCHTABLE_ROOT)
			continue;
		nbytes = snprintf(packet+packet_len, 4096, "TPset %s-%s@", wtab->name, wtab->dict); // XXX: REALLOC
		packet_len += nbytes;
	}
out:
	return (packet_len);
}

void session_load_watchtable_presets(struct session *session)
{
	struct wtab *wtab = (struct wtab *)zmalloc(sizeof(*wtab));
	struct filemap filemap;
	char *map;
	char path[256];
	int nr_wtab, x;

	if (!wtab)
		return;
	wtab->colmap[0]  = 900; wtab->colmap[1]  = 901;
	wtab->colmap[2]  = 903; wtab->colmap[3]  = 904;
	wtab->nr_columns = 4;
	wtab->table_type = WATCHTABLE_ROOT;
	session->watchtable_presets[0] = wtab;
	session->nr_watchtable_presets = 1;
	strcpy(wtab->name, "New^Preset");
	session->morphtab->wtab = wtab;

	watchtable_path(session, path);
	map = MAP_FILE_RO(path, &filemap);
	if (!map)
		return;
	nr_wtab = MIN(filemap.filesize/sizeof(struct wtab), 16);
	wtab    = (struct wtab *)map;
	for (x=0; x<nr_wtab; x++) {
		struct wtab *watchtable = (struct wtab *)malloc(sizeof(*wtab));
		if (!watchtable)
			continue;
		memcpy(watchtable, wtab, sizeof(*wtab));
		session->watchtable_presets[session->nr_watchtable_presets++] = watchtable;
		wtab++;
	}
	UNMAP_FILE(map, &filemap);
}

struct wtab *search_watchtable_preset(struct session *session, char *preset_name, int *position)
{
	struct wtab *wtab;
	int x, nr_presets;

	nr_presets = session->nr_watchtable_presets;
	for (x=0; x<nr_presets; x++) {
		wtab = session->watchtable_presets[x];
		if (wtab && !strcmp(wtab->name, preset_name)) {
			if (position)
				*position = (x-1);
			return (wtab);
		}
	}
	return (NULL);
}

char *XLS_packet(struct stock *stock, char *TID, struct wtab *preset, int nr_rows, int *outlen)
{
	char *packet;
	int x, packet_len, max_size = 256 KB;

	packet     = (char *)malloc(256 KB);
	packet_len = sprintf(packet, "table %s [", TID);
	for (x=0; x<nr_rows; x++) {
		packet_len = column_sprintf(NULL, NULL, stock, preset, packet, packet_len, x);
		if (packet_len+1024 >= max_size) {
			max_size *= 2;
			packet = (char *)realloc(packet, max_size);
			if (!packet)
				return 0;
		}
		*(packet+packet_len++) = ',';
	}
	*(packet+packet_len-1) = ']';
	*outlen = packet_len;
	return (packet);
}


/**
 * rpc_css
 * - save CSS <style> blocks
 * argv[1] = css_name   - name of the CSS style
 * argv[2] = css_style  - css <style>...</style> block (without the opening & closing tags)
 */
void rpc_css(struct rpc *rpc)
{
	struct session *session   = rpc->session;
	char           *css_name  = rpc->argv[1];
	char           *css_style = rpc->argv[2];
	struct stat     sb;
	char            path[256];
	char           *p;
	int             css_style_len, css_name_len, x;

	css_style_len = strlen(css_style);
	css_name_len  = strlen(css_name);
	if (!session->user || !session->user->logged_in || css_name_len > 64 || css_style_len >= MAX_CSS_SIZE)
		return;

	p = css_name;
	for (x=0; x<css_name_len; x++) {
		if (!isalnum(*p) && *p != '-' && *p != '_')
			continue;
		p++;
	}
	snprintf(path, sizeof(path)-1, "db/uid/%d/css/", session->user->uid);
	if (fs_stat(path, &sb) == -1)
		fs_mkdir(path, 0644);
	snprintf(path, sizeof(path)-1, "db/uid/%d/css/%s", session->user->uid, css_name);
	fs_newfile(path, css_style, css_style_len);
}

void session_load_css(struct session *session)
{
	struct dirmap   dirmap;
	struct filemap  filemap;
	struct style   *styles;
	char            path[256];
	char           *css_file, *css_mem, *filename;
	int             pathlen;

	if (!session->user || !session->user->logged_in)
		return;
	pathlen = snprintf(path, sizeof(path)-1, "db/uid/%d/css/", session->user->uid);
	if (!fs_opendir(path, &dirmap))
		return;

	session->styles      = styles = (struct style *)zmalloc(sizeof(struct style));
	styles->table_css    = (char **)zmalloc(MAX_TABLE_CSS_STYLES);
	styles->css_name     = (char **)zmalloc(MAX_TABLE_CSS_STYLES);
	styles->nr_table_css = 0;
	printf("checking css dir: %s\n", path);
	while ((filename=fs_readdir(&dirmap)) != NULL) {
		if (*filename == '.')
			continue;
		strncpy(path+pathlen, filename, 64);
		printf("css file: %s path: %s\n", filename, path);
		css_file = MAP_FILE_RO(path, &filemap);
		if (!css_file)
			continue;
		css_mem = (char *)malloc(filemap.filesize+1);
		if (!css_mem)
			continue;
		styles->css_name[styles->nr_table_css] = strdup(filename);
		printf("session loaded css style: %s\n", path);
		styles->table_css[styles->nr_table_css++] = css_mem;
		strcpy(css_mem, css_file);
		UNMAP_FILE(css_mem, &filemap);
		if (styles->nr_table_css >= MAX_TABLE_CSS_STYLES)
			break;
	}
	fs_closedir(&dirmap);
}

/* *****************************
 * global ticker wachtable sort
 *  - global sort mode exits 'watchlist mode' and enters 'watchtable mode'
 *********************/
void rpc_watchtable_sort(struct rpc *rpc)
{
	struct session    *session    = rpc->session;
	struct connection *connection = rpc->connection;
	char              *packet     = rpc->packet;
	int                idx        = atoi(rpc->argv[1]); // argv[1]: index into wftable
	char               direction  = *rpc->argv[2];      // argv[2]: sort direction (high/low)
	int                packet_len;

	if (!session->morphtab || idx >= 100 || idx <= 0)
		return;

	if (wftable[idx].f)
		packet_len = wftable[idx].f(session, session->morphtab, packet, direction, 20);
	if (packet_len) {
		*(packet+packet_len-1) = 0;
		printf(BOLDRED "rpc_watchtable_sort: %s" RESET "\n", packet);
		websocket_send(connection, packet, packet_len-1);
	}
}

#define SORT_STOCK_UP(type)                                           \
void                                                                  \
SORTUP##_##type(struct stock *stock, struct stock **board)            \
{                                                                     \
    struct stock *sp;                                                 \
    for (int x=0; x<20; x++) {                                        \
        sp = board[x];                                                \
        if (stock->type > sp->type) {                                 \
            memmove(&board[x+1], &board[x], (20-x)*8);                \
            board[x]  = stock;                                        \
            break;                                                    \
        }                                                             \
    }                                                                 \
}                                                                     \

#define SORT_STOCK_DOWN(type)                                         \
void                                                                  \
SORTDOWN##_##type(struct stock *stock, struct stock **board)          \
{                                                                     \
    struct stock *sp;                                                 \
    for (int x=0; x<20; x++) {                                        \
        sp = board[x];                                                \
        if (stock->type < sp->type) {                                 \
            memmove(&board[x+1], &board[x], (20-x)*8);                \
            board[x]  = stock;                                        \
            break;                                                    \
        }                                                             \
    }                                                                 \
}                                                                     \

#define SORT_MAG_UP(type)                                             \
void                                                                  \
MAGUP##_##type(struct stock *stock, struct stock **board)             \
{                                                                     \
    struct stock *sp;                                                 \
    for (int x=0; x<20; x++) {                                        \
        sp = board[x];                                                \
        if (stock->mag->type > sp->mag->type) {                       \
            memmove(&board[x+1], &board[x], (20-x)*8);                \
            board[x]  = stock;                                        \
            break;                                                    \
        }                                                             \
    }                                                                 \
}                                                                     \

#define SORT_MAG_DOWN(type)                                           \
void                                                                  \
MAGDOWN##_##type(struct stock *stock, struct stock **board)           \
{                                                                     \
    struct stock *sp;                                                 \
    for (int x=0; x<20; x++) {                                        \
        sp = board[x];                                                \
        if (stock->mag->type < sp->mag->type) {                       \
            memmove(&board[x+1], &board[x], (20-x)*8);                \
            board[x]  = stock;                                        \
            break;                                                    \
         }                                                            \
     }                                                                \
}                                                                     \


SORT_STOCK_UP(current_price);
SORT_STOCK_UP(rank);
SORT_STOCK_UP(day_volume);
SORT_STOCK_UP(pr_percent);
SORT_STOCK_UP(price_open);
SORT_STOCK_UP(intraday_high);
SORT_STOCK_UP(intraday_low);
SORT_STOCK_UP(open_pc);
SORT_STOCK_UP(high_pc);
SORT_STOCK_UP(low_pc);
SORT_STOCK_UP(prior_close);
SORT_STOCK_UP(day1);
SORT_STOCK_UP(day3);
SORT_STOCK_UP(day5);
SORT_STOCK_UP(day8);
SORT_STOCK_UP(day13);
SORT_STOCK_UP(day21);
SORT_STOCK_UP(day42);
SORT_STOCK_UP(day63);
SORT_STOCK_UP(nr_days_up);
SORT_STOCK_UP(nr_days_down);
SORT_STOCK_UP(nr_weeks_up);
SORT_STOCK_UP(nr_weeks_down);
SORT_STOCK_UP(green_days);

SORT_STOCK_DOWN(current_price);
SORT_STOCK_DOWN(rank);
SORT_STOCK_DOWN(day_volume);
SORT_STOCK_DOWN(pr_percent);
SORT_STOCK_DOWN(price_open);
SORT_STOCK_DOWN(intraday_high);
SORT_STOCK_DOWN(intraday_low);
SORT_STOCK_DOWN(open_pc);
SORT_STOCK_DOWN(high_pc);
SORT_STOCK_DOWN(low_pc);
SORT_STOCK_DOWN(prior_close);
SORT_STOCK_DOWN(day1);
SORT_STOCK_DOWN(day3);
SORT_STOCK_DOWN(day5);
SORT_STOCK_DOWN(day8);
SORT_STOCK_DOWN(day13);
SORT_STOCK_DOWN(day21);
SORT_STOCK_DOWN(day42);
SORT_STOCK_DOWN(day63);
SORT_STOCK_DOWN(nr_days_up);
SORT_STOCK_DOWN(nr_days_down);
SORT_STOCK_DOWN(nr_weeks_up);
SORT_STOCK_DOWN(nr_weeks_down);
SORT_STOCK_DOWN(green_days);

SORT_MAG_UP(peak_yr);
SORT_MAG_UP(peak_pc);
SORT_MAG_UP(YTD);

SORT_MAG_DOWN(peak_yr);
SORT_MAG_DOWN(peak_pc);
SORT_MAG_DOWN(YTD);


#define WTAB(type)                                                                          \
int                                                                                         \
wtab##_##type(struct session *session, struct watchlist *watchlist, char *packet, char dir, int size) \
{                                                                                           \
    struct watchstock     *watchstock;                                                      \
    struct stock *stock,  *board[256];                                                      \
    struct XLS   *XLS     = stock->XLS;                                                     \
    struct stock **STOCKS = XLS->STOCKS_PTR;                                                \
    int    nr_stocks      = XLS->nr_stocks, x;                                              \
    memcpy(board, STOCKS, size*8);                                                          \
    for (x=20; x<nr_stocks; x++) {                                                          \
        stock = STOCKS[x];                                                                  \
        if (dir == 'D')                                                                     \
           SORTUP##_##type(stock, board);                                                   \
        else                                                                                \
          SORTDOWN##_##type(stock, board);                                                  \
    }                                                                                       \
    watchlist->nr_stocks = 0;                                                               \
    for (x=0; x<size; x++) {                                                                \
        stock = board[x];                                                                   \
        watchstock = &watchlist->stocks[watchlist->nr_stocks++];                            \
       *(uint64_t *)(watchstock->ticker) = *(uint64_t *)stock->sym;                         \
        watchstock->ticker[7] = 0;                                                          \
        watchstock->stock     = stock;                                                      \
    }                                                                                       \
    return watchtable_packet(session, watchlist, packet);                                   \
}                                                                                           \

#define WTAB_MAG(type)                                                                      \
int                                                                                         \
wtab##_##type(struct session *session, struct watchlist *watchlist, char *packet, char dir, int size) \
{                                                                                           \
    struct watchstock      *watchstock;                                                     \
    struct stock *stock,   *board[size];                                                    \
    struct XLS   *XLS     = stock->XLS;                                                     \
    struct stock **STOCKS = XLS->STOCKS_PTR;                                                \
    int    nr_stocks      = XLS->nr_stocks, x, y;                                           \
    memcpy(board, STOCKS, size*8);                                                          \
    for (x=20; x<nr_stocks; x++) {                                                          \
        stock = STOCKS[x];                                                                  \
            if (!stock->mag)                                                                \
                continue;                                                                   \
            if (dir == 'D')                                                                 \
                MAGUP##_##type(stock, board);                                               \
            else                                                                            \
                MAGDOWN##_##type(stock, board);                                             \
            for (y=0; y<20; y++)                                                            \
                stock = board[y];                                                           \
        }                                                                                   \
        watchlist->nr_stocks = 0;                                                           \
        for (x=0; x<20; x++) {                                                              \
            stock = board[x];                                                               \
            watchstock = &watchlist->stocks[watchlist->nr_stocks++];                        \
            *(uint64_t *)(watchstock->ticker) = *(uint64_t *)stock->sym;                    \
            watchstock->ticker[7] = 0;                                                      \
            watchstock->stock     = stock;                                                  \
        }                                                                                   \
        return watchtable_packet(session, watchlist, packet);                               \
}

WTAB(current_price);
WTAB(rank);
WTAB(pr_percent);
WTAB(day_volume);
WTAB(price_open);
WTAB(prior_close);
WTAB(intraday_high);
WTAB(intraday_low);
WTAB(open_pc);
WTAB(high_pc);
WTAB(low_pc);
WTAB(day1);
WTAB(day3);
WTAB(day5);
WTAB(day8);
WTAB(day13);
WTAB(day21);
WTAB(day42);
WTAB(day63);
WTAB(nr_days_up);
WTAB(nr_days_down);
WTAB(nr_weeks_up);
WTAB(nr_weeks_down);
WTAB(green_days);

WTAB_MAG(YTD);
WTAB_MAG(peak_yr);
WTAB_MAG(peak_pc);

void init_wtable()
{
	wftable[1].f  = wtab_current_price;
	wftable[2].f  = wtab_rank;
	wftable[3].f  = wtab_pr_percent;
	wftable[4].f  = wtab_day_volume;
	wftable[5].f  = wtab_price_open;
	wftable[6].f  = wtab_intraday_high;
	wftable[7].f  = wtab_intraday_low;
	wftable[9].f  = wtab_prior_close;
	wftable[10].f = wtab_open_pc;
	wftable[11].f = wtab_high_pc;
	wftable[12].f = wtab_low_pc;
	wftable[13].f = wtab_peak_yr;
	wftable[14].f = wtab_peak_pc;
/*
	wftable[100].f = wtab_ma200;
	wftable[101].f = wtab_ma150;
	wftable[102].f = wtab_ma50;
	wftable[103].f = wtab_ma20;
	wftable[104].f = wtab_vwap;
	wftable[105].f = wtab_vol5;
	wftable[106].f = wtab_vol10;
	wftable[107].f = wtab_vol21;
	wftable[108].f = wtab_vol3m;

	wftable[200].f = wtab_21q21;
	wftable[201].f = wtab_21q20;
	wftable[202].f = wtab_21avg21;
	wftable[203].f = wtab_21avg20;
	wftable[204].f = wtab_63q;
	wftable[205].f = wtab_21q;
	wftable[206].f = wtab_one;
	wftable[207].f = wtab_two;

	wftable[300].f = wtab_edate;
	wftable[302].f = wtab_div_yield;
	wftable[303].f = wtab_div;
	wftable[304].f = wtab_xdate;
	wftable[305].f = wtab_annual_revenue;
	wftable[306].f = wtab_eps;
	wftable[307].f = wtab_var;
	wftable[308].f = wtab_mcap;
	wftable[309].f = wtab_peg;
	wftable[310].f = wtab_pbr;
	wftable[311].f = wtab_profit_margin;
	wftable[312].f = wtab_roa;
*/
	wftable[400].f = wtab_day1;
	wftable[401].f = wtab_day3;
	wftable[402].f = wtab_day5;
	wftable[420].f = wtab_day8;
	wftable[421].f = wtab_day13;
	wftable[403].f = wtab_day21;
	wftable[404].f = wtab_day42;
	wftable[405].f = wtab_day63;

	wftable[406].f = wtab_YTD;
//	wftable[407].f = wtab_rtd;
	wftable[408].f = wtab_nr_days_up;
	wftable[409].f = wtab_nr_days_down;
	wftable[410].f = wtab_nr_weeks_up;
	wftable[411].f = wtab_nr_weeks_down;
	wftable[412].f = wtab_green_days;
/*
	wftable[413].f = wtab_buy;
	wftable[414].f = wtab_buy_pc;
	wftable[415].f = wtab_sell;
	wftable[416].f = wtab_sell_delta;
	wftable[417].f = wtab_bix;
	wftable[418].f = wtab_avg5;
	wftable[419].f = wtab_avg10;*/
}
