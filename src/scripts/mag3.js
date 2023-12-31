var ISTRUCT = [
'volume_adi',
'volume_obv',
'volume_cmf',
'volume_fi',
'volume_mfi',
'volume_em',
'volume_vpt',
'volume_nvi',
'volume_vwap',
'vol_atr',  
'vol_bbm',
'vol_bbh',
'vol_bbl',
'vol_bbw',
'vol_bbp',
'vol_bbhi',
'vol_bbli',
'vol_kcc',
'vol_kch',
'vol_kcl',
'vol_kcw',
'vol_kcp',
'vol_kchi',
'vol_kcli',
'vol_dcl',
'vol_dch',
'vol_dcm',
'vol_dcw',
'vol_dcp',
'vol_ui',
'tr_macd',
'tr_macd_sig',
'tr_macd_diff',
'tr_sma_fast',
'tr_sma_slow',
'tr_ema_slow',
'tr_adx',
'tr_adx_pos',
'tr_adx_neg',
'tr_vtx_pos',
'tr_vtx_neg',
'tr_vtx_diff',
'tr_trix',
'tr_mass_idx',
'tr_cci',
'tr_dpo',
'tr_kst',
'tr_kst_sig',
'tr_kst_diff',
'tr_ICH_conv',
'tr_ICH_base',
'tr_ICH_a',
'tr_ICH_b',
'tr_vICH_a',
'tr_vICH_b',
'tr_aroon_up',
'tr_aroon_down',
'tr_aroon_ind',
'psar_up',
'psar_down',
'psar_up_ind',
'psar_down_ind',
'tr_stc',
'mom_rsi',
'stoch_rsi',
'stoch_rsi_k',
'stoch_rsi_d',
'mom_tsi',
'mom_uo',
'mom_stoch_k',
'mom_stoch_d',
'mom_wr',
'mom_ao',
'mom_kama',
'mom_roc',
'mom_ppo',
'mom_ppo_sig',
'mom_ppo_hist',
'others_dr',
'others_dlr',
'others_cr'];

var IENUM = [
'COL_VOLUME_ADI',
'COL_VOLUME_OVH',
'COL_VOLUME_CMF',
'COL_VOLUME_FI',
'COL_VOLUME_MFI',
'COL_VOLUME_EM',
'COL_VOLUME_VPT',
'COL_VOLUME_NVI',
'COL_VOLUME_VWAP',
'COL_VOL_ATR',
'COL_VOL_BBM',
'COL_VOL_BBH',
'COL_VOL_BBL',
'COL_VOL_BBW',
'COL_VOL_BBP',
'COL_VOL_BBHI',
'COL_VOL_BBLI',
'COL_VOL_KCC',
'COL_VOL_KCL',
'COL_VOL_KCH',
'COL_VOL_KCW',
'COL_VOL_KCP',
'COL_VOL_KCHI',
'COL_VOL_KCLI',
'COL_VOL_DCL',
'COL_VOL_DCH',
'COL_VOL_DCM',
'COL_VOL_DCW',
'COL_VOL_DCP',
'COL_VOL_UI',
'COL_TR_MACD',
'COL_TR_MACD_SIG',
'COL_TR_MACD_DIFF',
'COL_TR_SMA_FAST',
'COL_TR_SMA_SLOW',
'COL_TR_EMA_SLOW',
'COL_TR_ADX',
'COL_TR_ADX_POS',
'COL_TR_ADX_NEG',
'COL_TR_VTX_POS',
'COL_TR_VTX_NEG',
'COL_TR_VTX_DIFF',
'COL_TR_TRIX',
'COL_TR_MASS_IDX',
'COL_TR_CCI',
'COL_TR_DPO',
'COL_TR_KST',
'COL_TR_KST_SIG',
'COL_TR_KST_DIFF',
'COL_TR_ICH_CONV',
'COL_TR_ICH_BASE',
'COL_TR_ICH_A',
'COL_TR_ICH_B',
'COL_TR_VICH_A',
'COL_TR_VICH_B',
'COL_TR_AROON_UP',
'COL_TR_AROON_DOWN',
'COL_TR_AROON_IND',
'COL_TR_PSAR_UP',
'COL_TR_PSAR_DOWN',
'COL_TR_PSAR_UP_IND',
'COL_TR_PSAR_DOWN_IND',
'COL_TR_STC',
'COL_TR_MOM_RSI',
'COL_TR_MOM_STOCH_RSI',
'COL_TR_MOM_STOCH_RSI K',
'COL_TR_MOM_STOCH_RSI_D',
'COL_TR_MOM_TSI',
'COL_TR_MOM_UO',
'COL_TR_MOM_STOCH_K',
'COL_TR_MOM_STOCH_D',
'COL_TR_MOM_WR',
'COL_TR_MOM_AO',
'COL_TR_MOM_KAMA',
'COL_TR_MOM_ROC',
'COL_TR_MOM_PPO',
'COL_TR_MOM_PPO_SIG',
'COL_TR_MOM_PPO_HIST',
'COL_TR_MOM_OTHERS_DR',
'COL_TR_MOM_OTHERS_DLR',
'COL_TR_MOM_OTHERS_CR'];

var IDESC = [
'ADI',
'OBV',
'CMF',
'FI',
'MFI',
'EM',
'VPT',
'NVI',
'VWAP',
'ATR',
'BBM',
'BBH',
'BBL',
'BBW',
'BBP',
'BBHI',
'BBLI',
'KCC',
'KCH',
'KCL',
'KCW',
'KCP',
'KCHI',
'KCLI',
'DCL',
'DCH',
'DCM',
'DCW',
'DCP',
'UI',
'MACD',
'MACDs Sig',
'MACDd Diff',
'SMAf - SMA Fast',
'SMAs - SMA Slow',
'EMAs - EMA Slow',
'ADX',
'ADXp - ADX Position',
'ADXn - ADX Neg',
'VTXp - Vortex Position',
'VTXn - Vortex Neg',
'VTXd - Vortex Diff',
'TRIX',
'MIDX - Mass Index',
'CCI',
'DPO',
'KST',
'KSTs - KST Sig',
'KSTd - KST Diff',
'ICHc - Ichimoku Conv',
'ICHb - Ichimoku Base',
'ICHa - Ichimoku a',
'ICHb - Ichimoku b',
'vICHa - Visual Ichimoku a',
'vICHb - Visual Ichimoku b',
'ARup - Aroon Up',
'ARdw - Aroon Down',
'ARi  - Aroon Indicator',
'PSARup - PSAR Up',
'PSARdw - PSAR Down',
'PSARui - PSAR Up Indicator',
'PSARdi - PSAR Down Indicator',
'STC',
'RSI',
'sRSI - Stochastic RSI',
'sRSIk - Stochastic RSI k',
'sRSId - Stochastic RSI d',
'TSI',
'UO',
'StoK - Stochastic K',
'StoD - Stochastic D',
'WR - Williams R%',
'AO',
'KAMA',
'ROC',
'PPO',
'PPOs - PPO Sig',
'PPOh - PPO Hist',
'DR',
'DLR',
'CR'];


	var y = 109;
	for (var x = 0; x<IENUM.length; x++)
		console.log("case " + IENUM[x] + ':nbytes      = sprintf(packet+packet_len, "\\"' + y++ + '\\":\\"%.2f\\",\", m3->' + ISTRUCT[x] + ");break;");

	y = 109;
	for (var x = 0; x<IENUM.length; x++)
		console.log('{ "' + y++ + '",    ' + IENUM[x] + " },");

	/* screener.html */
	y = 109;
	for (var x = 0; x<IDESC.length; x++)
		console.log('<li v=' + y++ + '>' + IDESC[x] + '</li>');
