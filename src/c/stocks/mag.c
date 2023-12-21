#include <conf.h>
#include <stocks/stocks.h>
#include <extern.h>

struct column pythonTA_columns[] = {
	{ "MAVG200",     "\"MAVG200\":\"%.2f\"",   COL_MAVG200      },
	{ "MAVG100",     "\"MAVG100\":\"%.2f\"",   COL_MAVG100      },
	{ "MAVG50",      "\"MAVG50\":\"%.2f\"",    COL_MAVG50       },
	{ "MAVG20",      "\"MAVG20\":\"%.2f\"",    COL_MAVG20       },
	{ "EMA10",       "\"EMA10\":\"%.2f\"",     COL_EMA10        },
	{ "EMA20",       "\"EMA20\":\"%.2f\"",     COL_EMA20        },
	{ "EMA50",       "\"EMA50\":\"%.2f\"",     COL_EMA50        },
	{ "EMA100",      "\"EMA100\":\"%.2f\"",    COL_EMA100       },
	{ "EMA200",      "\"EMA200\":\"%.2f\"",    COL_EMA200       },
	{ "MFI2",        "\"MFI2\":\"%.2f\"",      COL_MFI2         },
	{ "vADI",        "\"vADI\":\"%.2f\"",      COL_VOL_ADI      },
	{ "vOBV",        "\"vOBV\":\"%.2f\"",      COL_VOL_OBV      },
	{ "vCMF",        "\"vCMF\":\"%.2f\"",      COL_VOL_CMF      },
	{ "vEM",         "\"vEM\":\"%.2f\"",       COL_VOL_EM       },
	{ "vVPT",        "\"vVPT\":\"%.2f\"",      COL_VOL_VPT      },
	{ "vNVI",        "\"vNVI\":\"%.2f\"",      COL_VOL_NVI      },
	{ "vVWAP",       "\"vVWAP\":\"%.2f\"",     COL_VOL_VWAP     },
	{ "vATR",        "\"vVATR\":\"%.2f\"",     COL_VOL_ATR      },
	{ "vBBM",        "\"vBBM\":\"%.2f\"",      COL_VOL_BBM      },
	{ "vBBH",        "\"vBBH\":\"%.2f\"",      COL_VOL_BBH      },
	{ "vBBL",        "\"vBBL\":\"%.2f\"",      COL_VOL_BBL      },
	{ "vBBW",        "\"vBBW\":\"%.2f\"",      COL_VOL_BBW      },
	{ "vBBP",        "\"vBBP\":\"%.2f\"",      COL_VOL_BBP      },
	{ "vBBHI",       "\"vBBHI\":\"%.2f\"",     COL_VOL_BBHI     },
	{ "vBBLI",       "\"vBBLI\":\"%.2f\"",     COL_VOL_BBLI     },
	{ "vKCC",        "\"vKCC\":\"%.2f\"",      COL_VOL_KCC      },
	{ "vKCL",        "\"vKCL\":\"%.2f\"",      COL_VOL_KCL      },
	{ "vKCH",        "\"vKCH\":\"%.2f\"",      COL_VOL_KCH      },
	{ "vKCW",        "\"vKCW\":\"%.2f\"",      COL_VOL_KCW      },
	{ "vKCP",        "\"vKCP\":\"%.2f\"",      COL_VOL_KCP      },
	{ "vKCHI",       "\"vKCHI\":\"%.2f\"",     COL_VOL_KCHI     },
	{ "vKCLI",       "\"vKCLI\":\"%.2f\"",     COL_VOL_KCLI     },
	{ "vDCL",        "\"vDCL\":\"%.2f\"",      COL_VOL_DCL      },
	{ "vDCH",        "\"vDCH\":\"%.2f\"",      COL_VOL_DCH      },
	{ "vDCM",        "\"vDCM\":\"%.2f\"",      COL_VOL_DCM      },
	{ "vDCW",        "\"vDCW\":\"%.2f\"",      COL_VOL_DCW      },
	{ "vDCP",        "\"vDCP\":\"%.2f\"",      COL_VOL_DCP      },
	{ "vUI",         "\"vUI\":\"%.2f\"",       COL_VOL_UI       },
	{ "MACD",       "\"tMACD\":\"%.2f\"",      COL_TR_MACD      },
	{ "MACDsig",    "\"tMACDsig\":\"%.2f\"",   COL_TR_MACD_SIG  },
	{ "MACDdiff",   "\"tMACDdiff\":\"%.2f\"",  COL_TR_MACD_DIFF },
	{ "SMAf",       "\"SMAf\":\"%.2f\"",       COL_TR_SMA_FAST  },
	{ "SMAs",       "\"SMAs\":\"%.2f\"",       COL_TR_SMA_SLOW  },
	{ "EMAs",       "\"EMAs\":\"%.2f\"",       COL_TR_EMA_SLOW  },
	{ "ADX",        "\"ADX\":\"%.2f\"",        COL_TR_ADX       },
	{ "ADXp",       "\"ADXp\":\"%.2f\"",       COL_TR_ADX_POS   },
	{ "ADXn",       "\"ADXn\":\"%.2f\"",       COL_TR_ADX_NEG   }
};


/*
max days 5% 21 action 1,max days 5% 42 action 1,max days 5% 63 action 1,success rate 5% 21 action 1,success rate 5% 42 action 1,success rate 5% 63 action 1,
max days 10% 21 action 1,max days 10% 42 action 1,max days 10% 63 action 1,success rate 10% 21 action 1,success rate 10% 42 action 1,success rate 10% 63 action 1,
# signals 21 action 4,# signals 42 action 4,# signals 63 action 4,
max days 5% 21 action 4,max days 5% 42 action 4,max days 5% 63 action 4,success rate 5% 21 action 4,success rate 5% 42 action 4,success rate 5% 63 action 4,
max days 10% 21 action 4,max days 10% 42 action 4,max days 10% 63 action 4,success rate 10% 21 action 4,success rate 10% 42 action 4,success rate 10% 63 action 4,
Plimit,Pdelta,1 yr P10%,1 yr P5%,last peak,P10orP5,peak,sig
*/

/*
0.00,0.00,0,0.00,0.00,0,0.00,0.00,0,0.00,0.00,0,
no 2,0,0,ESP,0,0,
[19] 4.14,9.68,6.41,10.17,3.01,3.25,35.97,36.16,
[27] 0.00,0.42,up,160.18,-15.52,195.29,3.00,3.01,up,162.25,
[37] -14.42,192.27,1.41,0.56,4.49,-5.19,-6.82,-9.88,0,213.76,214.12,10.00,17.00,13.00,
[51] 5.00,33.00,34.00,100.00,100.00,100.00,16.00,33.00,45.00,100.00,100.00,100.00,16.00,
[64] 9.00,9.00,23.00,23.00,23.00,93.75,100.00,100.00,25.00,25.00,25.00,87.50,100.00,100.00,
[78] 141.14,-0.26,-33.97,-38.87,1609995600.00,0,0,0
*/
void build_mag2(struct stock *stock, int nr_entries)
{
	struct mag2 *mag2;
	char        *map, *p, *p2, *value;
	char         path[256];
	double      *m;
	int64_t      filesize;
	int          msize;

	msize = sizeof(struct mag2) * nr_entries;
	mag2  = (struct mag2 *)zmalloc(msize);
	if (msize > 1760 && msize != 1764 && stock->mag)
		printf(BOLDRED "build_mag2 %s nr_entries: %d msize: %d" RESET "\n", stock->sym, stock->mag->nr_entries, (int)msize);
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/mag2/%s.csv", stock->sym);
	map   = value = fs_mallocfile_str(path, &filesize);
	if (!map)
		return;
	m     = (double *)mag2;
	while ((p2=strchr(value, '\n'))) {
		*p2++ = 0;
		while ((p=strchr(value, ','))) {
			*(double *)m++ = strtod(value, NULL);
			value = p + 1;
			if (!strchr(p+1, ','))
				break;
		}
		*(double *)m++ = strtod(p+1, NULL);
		value = p2;
		if (p2-map >= filesize)
			break;
	}
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/mag2/%s.m2", stock->sym);
	fs_newfile(path, (char *)mag2, msize);
	free(map);
}

void build_mag3(struct stock *stock, int nr_entries)
{
	struct filemap filemap;
	struct mag3   *m;
	char          *map, *line, *p;
	char           path[256];
	int            filesize, fd, n = 0, msize;

	msize = sizeof(struct mag3) * nr_entries;
	m     = (struct mag3 *)zmalloc(msize);
	if (!m)
		return;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/mag3/%s.csv", stock->sym);
	map = MAP_FILE_RO(path, &filemap);
	if (!map)
		return;
	line = map;
	while ((p=strchr(line, '\n'))) {
		sscanf(line,"%lf,%llu,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,"
	                "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
				&m[n].volume_adi, &m[n].volume_obv, &m[n].volume_cmf,  &m[n].volume_fi,    &m[n].volume_mfi, &m[n].volume_em,   &m[n].volume_sma_em,&m[n].volume_vpt,  &m[n].volume_nvi,   &m[n].volume_vwap,
				&m[n].vol_atr,    &m[n].vol_bbm,    &m[n].vol_bbh,     &m[n].vol_bbl,      &m[n].vol_bbw,    &m[n].vol_bbp,     &m[n].vol_bbhi,     &m[n].vol_bbli,
				&m[n].vol_kcc,    &m[n].vol_kch,    &m[n].vol_kcl,     &m[n].vol_kcw,      &m[n].vol_kcp,    &m[n].vol_kchi,    &m[n].vol_kcli,
				&m[n].vol_dcl,    &m[n].vol_dch,    &m[n].vol_dcm,     &m[n].vol_dcw,      &m[n].vol_dcp,    &m[n].vol_ui,
				&m[n].tr_macd,    &m[n].tr_macd_sig,&m[n].tr_macd_diff,&m[n].tr_sma_fast,  &m[n].tr_sma_slow,&m[n].tr_ema_fast, &m[n].tr_ema_slow,  &m[n].tr_adx,       &m[n].tr_adx_pos,  &m[n].tr_adx_neg,
				&m[n].tr_vtx_pos, &m[n].tr_vtx_neg, &m[n].tr_vtx_diff, &m[n].tr_trix,      &m[n].tr_mass_idx,&m[n].tr_cci,      &m[n].tr_dpo,       &m[n].tr_kst,       &m[n].tr_kst_sig,  &m[n].tr_kst_diff,
				&m[n].tr_ICH_conv,&m[n].tr_ICH_base,&m[n].tr_ICH_a,    &m[n].tr_ICH_b,     &m[n].tr_vICH_a,  &m[n].tr_vICH_b,   &m[n].tr_aroon_up,  &m[n].tr_aroon_down,&m[n].tr_aroon_ind,
				&m[n].psar_up,    &m[n].psar_down,  &m[n].psar_up_ind, &m[n].psar_down_ind,&m[n].tr_stc,
				&m[n].mom_rsi,    &m[n].stoch_rsi,  &m[n].stoch_rsi_k, &m[n].stoch_rsi_d,  &m[n].mom_tsi,    &m[n].mom_uo,      &m[n].mom_stoch_k,  &m[n].mom_stoch_d,  &m[n].mom_wr,
				&m[n].mom_ao,     &m[n].mom_kama,   &m[n].mom_roc,     &m[n].mom_ppo,      &m[n].mom_ppo_sig,&m[n].mom_ppo_hist,&m[n].others_dr,    &m[n].others_dlr,   &m[n].others_cr);
		line = p + 1;
		n   += 1;
	}
	UNMAP_FILE(map, &filemap);
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/mag3/%s.m3", stock->sym);
	fs_newfile(path, (char *)m, msize);
}

/*
volume_adi,        volume_obv,        volume_cmf,            volume_fi,           volume_mfi,       volume_em,          volume_sma_em,      volume_vpt, volume_nvi,    volume_vwap,
volatility_atr,    volatility_bbm,    volatility_bbh,        volatility_bbl,      volatility_bbw,   volatility_bbp,     volatility_bbhi,    volatility_bbli,
volatility_kcc,    volatility_kch,    volatility_kcl,        volatility_kcw,      volatility_kcp,   volatility_kchi,    volatility_kcli,   
volatility_dcl,    volatility_dch,    volatility_dcm,        volatility_dcw,      volatility_dcp,   volatility_ui,
trend_macd,        trend_macd_signal, trend_macd_diff,       trend_sma_fast,      trend_sma_slow,   trend_ema_fast,     trend_ema_slow,     trend_adx,  trend_adx_pos, trend_adx_neg,
trend_vtx_ind_pos, trend_vtx_ind_neg, trend_vtx_ind_diff,    trend_trix,          trend_mass_index, trend_cci,          trend_dpo,          trend_kst,  trend_kst_sig, trend_kst_diff,
trend_ICH_conv,    trend_ICH_base,    trend_ICH_a,           trend_ICH_b,         trend_vICH_a,     trend_vICH_b,
trend_aroon_up,    trend_aroon_down,  trend_aroon_ind,       trend_psar_up,       trend_psar_down,  trend_psar_up_ind,  trend_psar_down_ind,trend_stc,
momentum_rsi,      momentum_stoch_rsi,momentum_stoch_rsi_k,  momentum_stoch_rsi_d,momentum_tsi,     momentum_uo,        momentum_stoch,     momentum_stoch_signal,
momentum_wr,       momentum_ao,       momentum_kama,         momentum_roc,        momentum_ppo,     momentum_ppo_signal,momentum_ppo_hist,  others_dr,  others_dlr,    others_cr
*/

/*
	Stochastic: [Bull:  %K line crosses over  & above the %D Line]
				[Bear:  %K line crosses under & below the %D Line]
				[Works better in Ranging Markets]

	MACD:       [Bull:  MACD Line crosses over & above the Signal Line]
				[Bear:  MACD Line crosses over & below the Signal Line]

	RSI:        [Bull: 30 & below]
				[Bear: 70 & above]

	Aroon:      [Bull: Aroon UP   is ABOVE the 70 Line & Aroon DOWN is below 30 Line]
				[Bear: Aroon DOWN is ABOVE the 70 Line & Aroon UP   is below 30 Line]
				[Neutral: When both UP & DOWN are BELOW 50: consolidating]

	BB:         [Bull: Upside   band break]
				[Bear: Downsize band break]

	KCH:        [Bull: Price moves ABOVE the upper band]
				[Bear: Price moves BELOW the lower band]

	PSAR:
				[Trend Following, Can be used with ADX]



	Sto&MACD COMBO: [First Stochastic bullish signal followed by MACD bullish signal]
*/

void process_mag3(struct XLS *XLS)
{
	struct stock *stock, **stocks;
	struct mag3  *mag3;
	int nr_rows, nr_stocks, x;

	stocks    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = stocks[x];
		if (!stock->mag3)
			continue;
		nr_rows = stock->mag->nr_entries;
		mag3    = &stock->mag3[nr_rows-1];

		/* *********
		 * Momentum
		 **********/

		/* Stochastic */
		if (mag3->mom_stoch_k > mag3->mom_stoch_d) {
			stock->indicators |= BULLISH_STOCH;
			stock->nr_bullish_indicators++;
		} else if (mag3->mom_stoch_k < mag3->mom_stoch_d) {
			stock->indicators |= BEARISH_STOCH;
			stock->nr_bearish_indicators++;
		}

		/* RSI */
		if (mag3->mom_rsi <= 30) {
			stock->indicators |= BULLISH_RSI;
			stock->nr_bullish_indicators++;
		} else if (mag3->mom_rsi >= 70) {
			stock->indicators |= BEARISH_RSI;
			stock->nr_bearish_indicators++;
		}

		/* *********
		 *  Trend
		 **********/

		/* Aroon */
		if ((mag3->tr_aroon_up-mag3->tr_aroon_down > 10) && (mag3->tr_aroon_up > 70)) {
			stock->indicators |= BULLISH_AROON;
			stock->nr_bullish_indicators++;
		} else if ((mag3->tr_aroon_up-mag3->tr_aroon_down < -10) && (mag3->tr_aroon_down > 70)) {
			stock->indicators |= BEARISH_AROON;
			stock->nr_bearish_indicators++;
		}

		/* Keltner Channels */
		if (mag3->vol_kchi) {
			stock->indicators |= BULLISH_KCH;
			stock->nr_bullish_indicators++;
		} else if (mag3->vol_kcli) {
			stock->indicators |= BEARISH_KCH;
			stock->nr_bearish_indicators++;
		}
		add_double_gainer(stock, XLS->boards[BULL_BOARD]);
		add_double_gainer(stock, XLS->boards[BEAR_BOARD]);
	}
}

void load_mag2(struct stock *stock)
{
	struct filemap  filemap;
	char            *map;
	char             path[256];
	int              filesize;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/mag2/%s.m2", stock->sym);
	map = MAP_FILE_RO(path, &filemap);
	if (!map) {
		stock->mag2 = NULL;
		return;
	}
	filesize    = filemap.filesize;
	stock->mag2 = (struct mag2 *)malloc(filesize);
	stock->nr_mag2_entries = filesize/sizeof(struct mag2);
	memcpy(stock->mag2, map, filesize);
	UNMAP_FILE(map, &filemap);
}

void load_mag3(struct stock *stock)
{
	struct filemap filemap;
	char           path[256];
	char          *map;

	if (!stock->mag)
		return;
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/mag3/%s.m3", stock->sym);
	map = MAP_FILE_RO(path, &filemap);
	if (!map) {
		stock->mag3 = NULL;
		return;
	}
	stock->mag3 = (struct mag3 *)malloc(filemap.filesize);
	memcpy(stock->mag3, map, filemap.filesize);
	UNMAP_FILE(map, &filemap);
}

void load_mag4(struct stock *stock)
{
	struct filemap filemap;
	struct mag4   *m;
	char          *map;
	char           path[256];
	int            msize;

	return;
	if (!stock->mag)
		return;
	msize = sizeof(struct mag4) * stock->mag->nr_entries;
	m     = (struct mag4 *)zmalloc(msize);

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/mag4/%s.m4", stock->sym);
	map = MAP_FILE_RO(path, &filemap);
	if (!map)
		return;
	stock->mag4 = (struct mag4 *)malloc(msize);
	memcpy(stock->mag4, map, msize);
	UNMAP_FILE(map, &filemap);
}

void build_mag(char *ticker, struct XLS *XLS)
{
	struct stock *stock, **stocks;
	char path[256];
	int nr_entries, nr_stocks, x;

	/* build mag2/mag3 */
	stocks    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = stocks[x];
		if (ticker && strcmp(ticker, stock->sym))
			continue;
		snprintf(path, sizeof(path)-1, "data/stocks/stockdb/csv/%s.csv", stock->sym);
		nr_entries = fs_line_count(path);
		if (nr_entries < 252)
			continue;
		build_mag2(stock, nr_entries);
		build_mag3(stock, nr_entries);
	}
}

void stockdata_poll_thread(struct thread *thread)
{
	struct dirmap dirmap;
	char          directory[256];
	struct XLS   *XLS = thread->XLS;
	char         *filename;
	int           nr_mag2 = 0;

	if (!fs_opendir(STOCKDB_MAG2_PATH, &dirmap))
		return;

	while ((filename=fs_readdir(&dirmap)) != NULL) {
		if (strstr(filename, ".m2"))
			nr_mag2++;
	}

}
