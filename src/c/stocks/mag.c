#include <conf.h>
#include <stocks/stocks.h>

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
	{ "vOVH",        "\"vOVH\":\"%.2f\"",      COL_VOL_OVH      },
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

/*
		switch (wtab->colmap[x]) {
			case COL_MAVG200:nbytes            = sprintf(packet+packet_len, "\"100\":\"%.2f\",", m4->sma200);break;
			case COL_MAVG100:nbytes            = sprintf(packet+packet_len, "\"101\":\"%.2f\",", m4->sma100);break;
			case COL_MAVG50:nbytes             = sprintf(packet+packet_len, "\"102\":\"%.2f\",", m4->sma50);break;
			case COL_MAVG20:nbytes             = sprintf(packet+packet_len, "\"103\":\"%.2f\",", m4->sma20);break;
			case COL_EMA10:nbytes              = sprintf(packet+packet_len, "\"190\":\"%.2f\",", m4->ema10);break;
			case COL_EMA20:nbytes              = sprintf(packet+packet_len, "\"191\":\"%.2f\",", m4->ema20);break;
			case COL_EMA50:nbytes              = sprintf(packet+packet_len, "\"192\":\"%.2f\",", m4->ema50);break;
			case COL_EMA100:nbytes             = sprintf(packet+packet_len, "\"193\":\"%.2f\",", m4->ema100);break;
			case COL_EMA200:nbytes             = sprintf(packet+packet_len, "\"194\":\"%.2f\",", m4->ema200);break;
			case COL_MFI2:nbytes               = sprintf(packet+packet_len, "\"195\":\"%.2f\",", m4->mfi);break;
			case COL_VOL5:nbytes               = sprintf(packet+packet_len, "\"104\":\"%.2f\",", 0.0);break;
			case COL_VOL10:nbytes              = sprintf(packet+packet_len, "\"105\":\"%.2f\",", 0.0);break;
			case COL_VOL21:nbytes              = sprintf(packet+packet_len, "\"106\":\"%.2f\",", 0.0);break;
			case COL_VOL3M:nbytes              = sprintf(packet+packet_len, "\"107\":\"%.2f\",", 0.0);break;
			case COL_VOLUME_ADI:nbytes         = sprintf(packet+packet_len, "\"109\":\"%.2f\",", m3->volume_adi);break;
			case COL_VOLUME_OVH:nbytes         = sprintf(packet+packet_len, "\"110\":\"%lu\",",  m3->volume_obv);break;
			case COL_VOLUME_CMF:nbytes         = sprintf(packet+packet_len, "\"111\":\"%.2f\",", m3->volume_cmf);break;
			case COL_VOLUME_FI:nbytes          = sprintf(packet+packet_len, "\"112\":\"%.2f\",", m3->volume_fi);break;
			case COL_VOLUME_MFI:nbytes         = sprintf(packet+packet_len, "\"113\":\"%.2f\",", m3->volume_mfi);break;
			case COL_VOLUME_EM:nbytes          = sprintf(packet+packet_len, "\"114\":\"%.2f\",", m3->volume_em);break;
			case COL_VOLUME_VPT:nbytes         = sprintf(packet+packet_len, "\"115\":\"%.2f\",", m3->volume_vpt);break;
			case COL_VOLUME_NVI:nbytes         = sprintf(packet+packet_len, "\"116\":\"%.2f\",", m3->volume_nvi);break;
			case COL_VOLUME_VWAP:nbytes        = sprintf(packet+packet_len, "\"117\":\"%.2f\",", m3->volume_vwap);break;
			case COL_VOL_ATR:nbytes            = sprintf(packet+packet_len, "\"118\":\"%.2f\",", m3->vol_atr);break;
			case COL_VOL_BBM:nbytes            = sprintf(packet+packet_len, "\"119\":\"%.2f\",", m3->vol_bbm);break;
			case COL_VOL_BBH:nbytes            = sprintf(packet+packet_len, "\"120\":\"%.2f\",", m3->vol_bbh);break;
			case COL_VOL_BBL:nbytes            = sprintf(packet+packet_len, "\"121\":\"%.2f\",", m3->vol_bbl);break;
			case COL_VOL_BBW:nbytes            = sprintf(packet+packet_len, "\"122\":\"%.2f\",", m3->vol_bbw);break;
			case COL_VOL_BBP:nbytes            = sprintf(packet+packet_len, "\"123\":\"%.2f\",", m3->vol_bbp);break;
			case COL_VOL_BBHI:nbytes           = sprintf(packet+packet_len, "\"124\":\"%.2f\",", m3->vol_bbhi);break;
			case COL_VOL_BBLI:nbytes           = sprintf(packet+packet_len, "\"125\":\"%.2f\",", m3->vol_bbli);break;
			case COL_VOL_KCC:nbytes            = sprintf(packet+packet_len, "\"126\":\"%.2f\",", m3->vol_kcc);break;
			case COL_VOL_KCL:nbytes            = sprintf(packet+packet_len, "\"127\":\"%.2f\",", m3->vol_kch);break;
			case COL_VOL_KCH:nbytes            = sprintf(packet+packet_len, "\"128\":\"%.2f\",", m3->vol_kcl);break;
			case COL_VOL_KCW:nbytes            = sprintf(packet+packet_len, "\"129\":\"%.2f\",", m3->vol_kcw);break;
			case COL_VOL_KCP:nbytes            = sprintf(packet+packet_len, "\"130\":\"%.2f\",", m3->vol_kcp);break;
			case COL_VOL_KCHI:nbytes           = sprintf(packet+packet_len, "\"131\":\"%.2f\",", m3->vol_kchi);break;
			case COL_VOL_KCLI:nbytes           = sprintf(packet+packet_len, "\"132\":\"%.2f\",", m3->vol_kcli);break;
			case COL_VOL_DCL:nbytes            = sprintf(packet+packet_len, "\"133\":\"%.2f\",", m3->vol_dcl);break;
			case COL_VOL_DCH:nbytes            = sprintf(packet+packet_len, "\"134\":\"%.2f\",", m3->vol_dch);break;
			case COL_VOL_DCM:nbytes            = sprintf(packet+packet_len, "\"135\":\"%.2f\",", m3->vol_dcm);break;
			case COL_VOL_DCW:nbytes            = sprintf(packet+packet_len, "\"136\":\"%.2f\",", m3->vol_dcw);break;
			case COL_VOL_DCP:nbytes            = sprintf(packet+packet_len, "\"137\":\"%.2f\",", m3->vol_dcp);break;
			case COL_VOL_UI:nbytes             = sprintf(packet+packet_len, "\"138\":\"%.2f\",", m3->vol_ui);break;
			case COL_TR_MACD:nbytes            = sprintf(packet+packet_len, "\"139\":\"%.2f\",", m3->tr_macd);break;
			case COL_TR_MACD_SIG:nbytes        = sprintf(packet+packet_len, "\"140\":\"%.2f\",", m3->tr_macd_sig);break;
			case COL_TR_MACD_DIFF:nbytes       = sprintf(packet+packet_len, "\"141\":\"%.2f\",", m3->tr_macd_diff);break;
			case COL_TR_SMA_FAST:nbytes        = sprintf(packet+packet_len, "\"142\":\"%.2f\",", m3->tr_sma_fast);break;
			case COL_TR_SMA_SLOW:nbytes        = sprintf(packet+packet_len, "\"143\":\"%.2f\",", m3->tr_sma_slow);break;
			case COL_TR_EMA_SLOW:nbytes        = sprintf(packet+packet_len, "\"144\":\"%.2f\",", m3->tr_ema_slow);break;
			case COL_TR_ADX:nbytes             = sprintf(packet+packet_len, "\"145\":\"%.2f\",", m3->tr_adx);break;
			case COL_TR_ADX_POS:nbytes         = sprintf(packet+packet_len, "\"146\":\"%.2f\",", m3->tr_adx_pos);break;
			case COL_TR_ADX_NEG:nbytes         = sprintf(packet+packet_len, "\"147\":\"%.2f\",", m3->tr_adx_neg);break;
			case COL_TR_VTX_POS:nbytes         = sprintf(packet+packet_len, "\"148\":\"%.2f\",", m3->tr_vtx_pos);break;
			case COL_TR_VTX_NEG:nbytes         = sprintf(packet+packet_len, "\"149\":\"%.2f\",", m3->tr_vtx_neg);break;
			case COL_TR_VTX_DIFF:nbytes        = sprintf(packet+packet_len, "\"150\":\"%.2f\",", m3->tr_vtx_diff);break;
			case COL_TR_TRIX:nbytes            = sprintf(packet+packet_len, "\"151\":\"%.2f\",", m3->tr_trix);break;
			case COL_TR_MASS_IDX:nbytes        = sprintf(packet+packet_len, "\"152\":\"%.2f\",", m3->tr_mass_idx);break;
			case COL_TR_CCI:nbytes             = sprintf(packet+packet_len, "\"153\":\"%.2f\",", m3->tr_cci);break;
			case COL_TR_DPO:nbytes             = sprintf(packet+packet_len, "\"154\":\"%.2f\",", m3->tr_dpo);break;
			case COL_TR_KST:nbytes             = sprintf(packet+packet_len, "\"155\":\"%.2f\",", m3->tr_kst);break;
			case COL_TR_KST_SIG:nbytes         = sprintf(packet+packet_len, "\"156\":\"%.2f\",", m3->tr_kst_sig);break;
			case COL_TR_KST_DIFF:nbytes        = sprintf(packet+packet_len, "\"157\":\"%.2f\",", m3->tr_kst_diff);break;
			case COL_TR_ICH_CONV:nbytes        = sprintf(packet+packet_len, "\"158\":\"%.2f\",", m3->tr_ICH_conv);break;
			case COL_TR_ICH_BASE:nbytes        = sprintf(packet+packet_len, "\"159\":\"%.2f\",", m3->tr_ICH_base);break;
			case COL_TR_ICH_A:nbytes           = sprintf(packet+packet_len, "\"160\":\"%.2f\",", m3->tr_ICH_a);break;
			case COL_TR_ICH_B:nbytes           = sprintf(packet+packet_len, "\"161\":\"%.2f\",", m3->tr_ICH_b);break;
			case COL_TR_VICH_A:nbytes          = sprintf(packet+packet_len, "\"162\":\"%.2f\",", m3->tr_vICH_a);break;
			case COL_TR_VICH_B:nbytes          = sprintf(packet+packet_len, "\"163\":\"%.2f\",", m3->tr_vICH_b);break;
			case COL_TR_AROON_UP:nbytes        = sprintf(packet+packet_len, "\"164\":\"%.2f\",", m3->tr_aroon_up);break;
			case COL_TR_AROON_DOWN:nbytes      = sprintf(packet+packet_len, "\"165\":\"%.2f\",", m3->tr_aroon_down);break;
			case COL_TR_AROON_IND:nbytes       = sprintf(packet+packet_len, "\"166\":\"%.2f\",", m3->tr_aroon_ind);break;
			case COL_TR_PSAR_UP:nbytes         = sprintf(packet+packet_len, "\"167\":\"%.2f\",", m3->psar_up);break;
			case COL_TR_PSAR_DOWN:nbytes       = sprintf(packet+packet_len, "\"168\":\"%.2f\",", m3->psar_down);break;
			case COL_TR_PSAR_UP_IND:nbytes     = sprintf(packet+packet_len, "\"169\":\"%.2f\",", m3->psar_up_ind);break;
			case COL_TR_PSAR_DOWN_IND:nbytes   = sprintf(packet+packet_len, "\"170\":\"%.2f\",", m3->psar_down_ind);break;
			case COL_TR_STC:nbytes             = sprintf(packet+packet_len, "\"171\":\"%.2f\",", m3->tr_stc);break;
			case COL_TR_MOM_RSI:nbytes         = sprintf(packet+packet_len, "\"172\":\"%.2f\",", m3->mom_rsi);break;
			case COL_TR_MOM_STOCH_RSI:nbytes   = sprintf(packet+packet_len, "\"173\":\"%.2f\",", m3->stoch_rsi);break;
			case COL_TR_MOM_STOCH_RSI_K:nbytes = sprintf(packet+packet_len, "\"174\":\"%.2f\",", m3->stoch_rsi_k);break;
			case COL_TR_MOM_STOCH_RSI_D:nbytes = sprintf(packet+packet_len, "\"175\":\"%.2f\",", m3->stoch_rsi_d);break;
			case COL_TR_MOM_TSI:nbytes         = sprintf(packet+packet_len, "\"176\":\"%.2f\",", m3->mom_tsi);break;
			case COL_TR_MOM_UO:nbytes          = sprintf(packet+packet_len, "\"177\":\"%.2f\",", m3->mom_uo);break;
			case COL_TR_MOM_STOCH_K:nbytes     = sprintf(packet+packet_len, "\"178\":\"%.2f\",", m3->mom_stoch_k);break;
			case COL_TR_MOM_STOCH_D:nbytes     = sprintf(packet+packet_len, "\"179\":\"%.2f\",", m3->mom_stoch_d);break;
			case COL_TR_MOM_WR:nbytes          = sprintf(packet+packet_len, "\"180\":\"%.2f\",", m3->mom_wr);break;
			case COL_TR_MOM_AO:nbytes          = sprintf(packet+packet_len, "\"181\":\"%.2f\",", m3->mom_ao);break;
			case COL_TR_MOM_KAMA:nbytes        = sprintf(packet+packet_len, "\"182\":\"%.2f\",", m3->mom_kama);break;
			case COL_TR_MOM_ROC:nbytes         = sprintf(packet+packet_len, "\"183\":\"%.2f\",", m3->mom_roc);break;
			case COL_TR_MOM_PPO:nbytes         = sprintf(packet+packet_len, "\"184\":\"%.2f\",", m3->mom_ppo);break;
			case COL_TR_MOM_PPO_SIG:nbytes     = sprintf(packet+packet_len, "\"185\":\"%.2f\",", m3->mom_ppo_sig);break;
			case COL_TR_MOM_PPO_HIST:nbytes    = sprintf(packet+packet_len, "\"186\":\"%.2f\",", m3->mom_ppo_hist);break;
			case COL_TR_MOM_OTHERS_DR:nbytes   = sprintf(packet+packet_len, "\"187\":\"%.2f\",", m3->others_dr);break;
			case COL_TR_MOM_OTHERS_DLR:nbytes  = sprintf(packet+packet_len, "\"188\":\"%.2f\",", m3->others_dlr);break;
			case COL_TR_MOM_OTHERS_CR:nbytes   = sprintf(packet+packet_len, "\"189\":\"%.2f\",", m3->others_cr);break;

			case COL_DAYS_5PC:nbytes      = sprintf(packet+packet_len, "\"200\":\"%.2f\",", m2->days_5pc);break;
			case COL_DAYS_10PC:nbytes      = sprintf(packet+packet_len, "\"201\":\"%.2f\",", m2->days_10pc);break;
			case COL_DAYS_15PC:nbytes      = sprintf(packet+packet_len, "\"202\":\"%.2f\",", m2->days_15pc);break;
			case COL_DAYS_20PC:nbytes      = sprintf(packet+packet_len, "\"203\":\"%.2f\",", m2->days_20pc);break;
			case COL_RET_5PC:nbytes      = sprintf(packet+packet_len, "\"204\":\"%.2f\",", m2->days_ret_5pc);break;
			case COL_RET_10PC:nbytes      = sprintf(packet+packet_len, "\"205\":\"%.2f\",", m2->days_ret_10pc);break;
			case COL_RET_15PC:nbytes      = sprintf(packet+packet_len, "\"206\":\"%.2f\",", m2->days_ret_15pc);break;
			case COL_RET_20PC:nbytes      = sprintf(packet+packet_len, "\"207\":\"%.2f\",", m2->days_ret_20pc);break;
			case COL_MAX_5PC:nbytes      = sprintf(packet+packet_len, "\"208\":\"%.2f\",", m2->days_max_5pc);break;
			case COL_MAX_10PC:nbytes      = sprintf(packet+packet_len, "\"209\":\"%.2f\",", m2->days_max_10pc);break;
			case COL_MAX_15PC:nbytes      = sprintf(packet+packet_len, "\"210\":\"%.2f\",", m2->days_max_15pc);break;
			case COL_MAX_20PC:nbytes      = sprintf(packet+packet_len, "\"211\":\"%.2f\",", m2->days_max_20pc);break;
			case COL_A1_ESP:nbytes      = sprintf(packet+packet_len, "\"212\":\"%.2f\",", m2->a1esp);break;
			case COL_A1:nbytes      = sprintf(packet+packet_len, "\"213\":\"%.2f\",", m2->action1);break;
			case COL_A1SAME:nbytes      = sprintf(packet+packet_len, "\"214\":\"%.2f\",", m2->a1same);break;
			case COL_A1RESULT:nbytes      = sprintf(packet+packet_len, "\"215\":\"%.2f\",", m2->a1result);break;
			case COL_A4ESP:nbytes      = sprintf(packet+packet_len, "\"216\":\"%.2f\",", m2->a4esp);break;
			case COL_A4:nbytes      = sprintf(packet+packet_len, "\"217\":\"%.2f\",", m2->action4);break;
			case COL_1D:nbytes      = sprintf(packet+packet_len, "\"218\":\"%.2f\",", m2->day1);break;
			case COL_3D:nbytes      = sprintf(packet+packet_len, "\"219\":\"%.2f\",", m2->day3);break;
			case COL_5D:nbytes      = sprintf(packet+packet_len, "\"220\":\"%.2f\",", m2->day5);break;
			case COL_8D:nbytes      = sprintf(packet+packet_len, "\"221\":\"%.2f\",", m2->day8);break;
			case COL_13D:nbytes      = sprintf(packet+packet_len, "\"222\":\"%.2f\",", m2->day13);break;
			case COL_21D:nbytes      = sprintf(packet+packet_len, "\"223\":\"%.2f\",", m2->day21);break;
			case COL_42D:nbytes      = sprintf(packet+packet_len, "\"224\":\"%.2f\",", m2->day42);break;
			case COL_63D:nbytes      = sprintf(packet+packet_len, "\"225\":\"%.2f\",", m2->day63);break;
			case COL_RTD:nbytes      = sprintf(packet+packet_len, "\"226\":\"%.2f\",", m2->RTD);break;
			case COL_STREAK:nbytes      = sprintf(packet+packet_len, "\"227\":\"%.2f\",", m2->streak);break;
			case COL_DIR:nbytes      = sprintf(packet+packet_len, "\"228\":\"%.2f\",", m2->dir);break;
			case COL_BUY:nbytes      = sprintf(packet+packet_len, "\"229\":\"%.2f\",", m2->buy);break;
			case COL_BUY_DELTA:nbytes      = sprintf(packet+packet_len, "\"230\":\"%.2f\",", m2->buy_delta);break;
			case COL_SELL:nbytes      = sprintf(packet+packet_len, "\"231\":\"%.2f\",", m2->sell);break;
			case COL_SELL_DELTA:nbytes      = sprintf(packet+packet_len, "\"232\":\"%.2f\",", m2->sell_delta);break;
			case COL_FIB:nbytes      = sprintf(packet+packet_len, "\"233\":\"%.2f\",", m2->fib);break;
			case COL_FIB_DIR:nbytes      = sprintf(packet+packet_len, "\"234\":\"%.2f\",", m2->fib_dir);break;
			case COL_BUY_FIB:nbytes      = sprintf(packet+packet_len, "\"235\":\"%.2f\",", m2->buy_fib);break;
			case COL_DELTA_FIB:nbytes      = sprintf(packet+packet_len, "\"236\":\"%.2f\",", m2->buy_delta_fib);break;
			case COL_SELL_FIB:nbytes      = sprintf(packet+packet_len, "\"237\":\"%.2f\",", m2->sell_fib);break;
			case COL_SELL_DELTA_FIB:nbytes      = sprintf(packet+packet_len, "\"238\":\"%.2f\",", m2->sell_delta_fib);break;
			case COL_MEAN:nbytes      = sprintf(packet+packet_len, "\"239\":\"%.2f\",", m2->mean);break;
			case COL_STD:nbytes      = sprintf(packet+packet_len, "\"240\":\"%.2f\",", m2->std);break;
			case COL_VAR90:nbytes      = sprintf(packet+packet_len, "\"241\":\"%.2f\",", m2->var_90);break;
			case COL_VAR95:nbytes      = sprintf(packet+packet_len, "\"242\":\"%.2f\",", m2->var_95);break;
			case COL_VAR99:nbytes      = sprintf(packet+packet_len, "\"243\":\"%.2f\",", m2->var_99);break;
			case COL_VAR99P:nbytes                  = sprintf(packet+packet_len, "\"244\":\"%.2f\",", m2->var_99pot);break;
			case COL_1_YEAR_AGO:nbytes              = sprintf(packet+packet_len, "\"245\":\"%.2f\",", m2->one_year_ago);break;
			case COL_1_YEAR_PKPC:nbytes             = sprintf(packet+packet_len, "\"246\":\"%.2f\",", m2->one_year_pkpc);break;
			case COL_YTD:nbytes                     = sprintf(packet+packet_len, "\"247\":\"%.2f\",", m2->YTD);break;
			case COL_SIG_21_A1:nbytes               = sprintf(packet+packet_len, "\"248\":\"%.2f\",", m2->sig_21_a1);break;
			case COL_SIG_42_A1:nbytes               = sprintf(packet+packet_len, "\"249\":\"%.2f\",", m2->sig_42_a1);break;
			case COL_SIG_63_A1:nbytes               = sprintf(packet+packet_len, "\"250\":\"%.2f\",", m2->sig_63_a1);break;
			case COL_MAXDAYS_5PC_21_A1:nbytes       = sprintf(packet+packet_len, "\"251\":\"%.2f\",", m2->max_days_5pc_21_a1);break;
			case COL_MAXDAYS_5PC_42_A1:nbytes       = sprintf(packet+packet_len, "\"252\":\"%.2f\",", m2->max_days_5pc_42_a1);break;
			case COL_MAXDAYS_5PC_63_A1:nbytes       = sprintf(packet+packet_len, "\"253\":\"%.2f\",", m2->max_days_5pc_63_a1);break;
			case COL_SUCCESS_5PC_21_A1:nbytes       = sprintf(packet+packet_len, "\"254\":\"%.2f\",", m2->success__5pc_21_a1);break;
			case COL_SUCCESS_5PC_42_A1:nbytes       = sprintf(packet+packet_len, "\"255\":\"%.2f\",", m2->success__5pc_42_a1);break;
			case COL_SUCCESS_5PC_63_A1:nbytes       = sprintf(packet+packet_len, "\"256\":\"%.2f\",", m2->success__5pc_63_a1);break;
			case COL_MAXDAYS_10PC_21_A1:nbytes      = sprintf(packet+packet_len, "\"257\":\"%.2f\",", m2->max_days_10pc_21_a1);break;
			case COL_MAXDAYS_10PC_42_A1:nbytes      = sprintf(packet+packet_len, "\"258\":\"%.2f\",", m2->max_days_10pc_42_a1);break;
			case COL_MAXDAYS_10PC_63_A1:nbytes      = sprintf(packet+packet_len, "\"259\":\"%.2f\",", m2->max_days_10pc_63_a1);break;
			case COL_SUCCESS_10PC_21_A1:nbytes      = sprintf(packet+packet_len, "\"260\":\"%.2f\",", m2->success__10pc_21_a1);break;
			case COL_SUCCESS_10PC_42_A1:nbytes      = sprintf(packet+packet_len, "\"261\":\"%.2f\",", m2->success__10pc_42_a1);break;
			case COL_SUCCESS_10PC_63_A1:nbytes      = sprintf(packet+packet_len, "\"262\":\"%.2f\",", m2->success__10pc_63_a1);break;
			case COL_SIG_21_A4:nbytes               = sprintf(packet+packet_len, "\"263\":\"%.2f\",", m2->sig_21_a4);break;
			case COL_SIG_42_A4:nbytes               = sprintf(packet+packet_len, "\"264\":\"%.2f\",", m2->sig_42_a4);break;
			case COL_SIG_63_A4:nbytes               = sprintf(packet+packet_len, "\"265\":\"%.2f\",", m2->sig_63_a4);break;
			case COL_MAXDAYS_5PC_21_A4:nbytes       = sprintf(packet+packet_len, "\"266\":\"%.2f\",", m2->max_days_5pc_21_a4);break;
			case COL_MAXDAYS_5PC_42_A4:nbytes       = sprintf(packet+packet_len, "\"267\":\"%.2f\",", m2->max_days_5pc_42_a4);break;
			case COL_MAXDAYS_5PC_63_A4:nbytes       = sprintf(packet+packet_len, "\"268\":\"%.2f\",", m2->max_days_5pc_63_a4);break;
			case COL_SUCCESS_5PC_21_A4:nbytes       = sprintf(packet+packet_len, "\"269\":\"%.2f\",", m2->success__5pc_21_a4);break;
			case COL_SUCCESS_5PC_42_A4:nbytes       = sprintf(packet+packet_len, "\"270\":\"%.2f\",", m2->success__5pc_42_a4);break;
			case COL_SUCCESS_5PC_63_A4:nbytes       = sprintf(packet+packet_len, "\"271\":\"%.2f\",", m2->success__5pc_63_a4);break;
			case COL_MAXDAYS_10PC_21_A4:nbytes      = sprintf(packet+packet_len, "\"272\":\"%.2f\",", m2->max_days_10pc_21_a4);break;
			case COL_MAXDAYS_10PC_42_A4:nbytes      = sprintf(packet+packet_len, "\"273\":\"%.2f\",", m2->max_days_10pc_42_a4);break;
			case COL_MAXDAYS_10PC_63_A4:nbytes      = sprintf(packet+packet_len, "\"274\":\"%.2f\",", m2->max_days_10pc_63_a4);break;
			case COL_SUCCESS_10PC_21_A4:nbytes      = sprintf(packet+packet_len, "\"275\":\"%.2f\",", m2->success__10pc_21_a4);break;
			case COL_SUCCESS_10PC_42_A4:nbytes      = sprintf(packet+packet_len, "\"276\":\"%.2f\",", m2->success__10pc_42_a4);break;
			case COL_SUCCESS_10PC_63_A4:nbytes      = sprintf(packet+packet_len, "\"277\":\"%.2f\",", m2->success__10pc_63_a4);break;
			case COL_1_YEAR_PKPR:nbytes             = sprintf(packet+packet_len, "\"278\":\"%.2f\",", m2->one_year_pk_price);break;
			case COL_PLIMIT:nbytes                  = sprintf(packet+packet_len, "\"279\":\"%.2f\",", m2->plimit);break;
			case COL_PDELTA:nbytes                  = sprintf(packet+packet_len, "\"280\":\"%.2f\",", m2->pdelta);break;
			case COL_ONE_YR_P10:nbytes              = sprintf(packet+packet_len, "\"281\":\"%.2f\",", m2->one_yr_p10);break;
			case COL_ONE_YR_P5:nbytes               = sprintf(packet+packet_len, "\"282\":\"%.2f\",", m2->one_yr_p5);break;
			case COL_LAST_PEAK:nbytes               = sprintf(packet+packet_len, "\"283\":\"%s\",",   unix2str(m2->last_peak, timestr));break;
			case COL_P10ORP5:nbytes                 = sprintf(packet+packet_len, "\"284\":\"%.2f\",", m2->p10orp5);break;
			case COL_PEAK2:nbytes                   = sprintf(packet+packet_len, "\"285\":\"%s\",",   PEAK(m2->peak));break;
			case COL_SIG:nbytes                     = sprintf(packet+packet_len, "\"286\":\"%s\",",   SIGNAL(m2->sig));break;


		}
*/
