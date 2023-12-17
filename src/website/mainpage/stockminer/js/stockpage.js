/***********************************************
 * filename: mainpage/stockminer/js/stockpage.js
 * License:  Public Domain
 **********************************************/
function init_stockpage()
{
	SPX_pr    = document.getElementById("spx-ipr");
	SPX_pc    = document.getElementById("spx-ipc");
	NDX_pr    = document.getElementById("ndx-ipr");
	NDX_pc    = document.getElementById("ndx-ipc");
	DOW_pr    = document.getElementById("dow-ipr");
	DOW_pc    = document.getElementById("dow-ipc");
	XLK_pr    = document.getElementById("XLK-ipr");
	XLK_pc    = document.getElementById("XLK-ipc");
	TXT       = document.getElementById("TXT");

	TP['New^Preset'] = JSON.stringify(STAB);
	COL["macd"] = ["#008000", "#FF0000", "#1e90ff", "#ffa500"];
	OPT["macd"] = {};
	OPT["macd"].zones      = [{value:0,color:'red'},{color:"green"}];
	OPT["macd"].macdLine   = {zones:[{value:-1,color:'#07AD73'},{value:1,color:'#07AD73'},{color:'#07AD73'}]};
	OPT["macd"].signalLine = {zones:[{value:-1.5,color:'red'},{color:'red'}]};

	OPT['aroon']      = {aroonDown:{styles:{lineColor:'#FF3406'}}, color:'dodgerblue'};
	OPT['stochastic'] = {smoothedLine:{styles:{lineColor:'#FF3406'}}, color:'dodgerblue'};
	OPT['rsi']        = {color:'#821eda'}

	T['sigmon'] = $('#SIGMON').DataTable({
        "columns": [{"data":"902"},{"data":"900"},{"data":"915"},{"data":"901"},{"data":"913"},
					{"data":"850"},{"data":"851"},{"data":"852"},{"data":"853"},{"data":"854"},{"data":"855"}],
  "columnDefs": [{"targets":11,"visible":false},{ type: 'natural', "targets": [0,2]}],
	"createdRow": function(row,data,index){tableColors(row,data,index,"#SIGMON thead tr",0)},
		"bPaginate":false,"order": [[2, "asc" ]],"paging":false,"bDestroy":false,"info":false,"searching":false});

}

function rpc_stockpage(av)
{
	var QVID = av.QVID, QSID = av.QSID, ticker = av.ticker;

	console.log("rpc_stockpage: " + av[7]);
	if (av[7])
//		stockpage(av[1],av[4],0,1);
		stockpage(ticker,QVID,1);
	/* MQ */
	chart(av);
	/* Bull Quad */
	av.div = ticker + "-P" + QVID + "Q" + QSID + "q0ws0";
//	LQ = av[2] = av[1] + "-P" + av[4] + "Q" + av[5] + "q0ws0";
	ichart(av,['aroon','macd'], 'day');

	/* Bear Quad */
	av.div = ticker + "-P" + QVID + "Q" + QSID + "q1ws0";
//	RQ = av[2] = av[1] + "-P" + av[4] + "Q" + av[5] + "q1ws0";
	ichart(av,['aroon','macd','bb'], 'month');
}
var DICT;
function stockpage(ticker,QVID,rpc)
{
	var w,sp,QSID,quadspace;
	console.log("stockpage: " + ticker + " QVID: " + QVID + " rpc: " + rpc);
	if (sp=SP[ticker]) {
		console.log("stockpage already exists: " + ticker + " " + QVID + " " + sp.QSID + " loaded: " + sp.loaded);
		if (sp.loaded) {
			/* Switch to tab containing already existing stockpage */
			var QVID = sp.QVID;
			if (W.PID != QVID)
				QUADVERSE_SWITCH(QVID);
			Q[QVID].showtab(Q[QVID].workspace['ws'+sp.QSID].tab);
			return;
		}
		QVID = sp.QVID;
		QSID = sp.QSID;
		console.log("loading unloaded stockpage QVID: " + QVID + " QSID: " + QSID);
		quadspace = Q[QVID].quadspace[QSID];
	}
	if (QVID==-1) {
		w = W;
		QVID = w.PID;
	} else
		w = Q[QVID];
	if (!quadspace)
		quadspace = w.addQuadspace(ticker, 0, "opgrid stockpage", 4, 0, 0, 0, "dodgerblue",0,0,-1);
	quadspace.SP = 1;

	var BB,CT,
	LQ   = quadspace.quad[0],
	RQ   = quadspace.quad[1],
	MQ   = quadspace.quad[2],
	DQ   = quadspace.quad[3];
	QSID = quadspace.QSID;

	/* Tech */
	LQ.addWorkspace({title:"Daily",         favicon:0},{background:0}, "", QSID, 0, -1);
	MQ.addWorkspace({title:ticker,          favicon:1},{background:0}, "", QSID, 2, -1);
	RQ.addWorkspace({title:"Monthly",       favicon:0},{background:0}, "", QSID, 1, -1);
	DQ.addWorkspace({title:"Control",       favicon:0},{background:0}, "", QSID, 3, -1);
	DQ.addWorkspace({title:"TA Screen",     favicon:0},{background:1,click:'indicator_screener_click("' + ticker + '")'}, "", QSID, 3, -1);
	DQ.addWorkspace({title:"Algo Screen",   favicon:0},{background:1,click:'algo_screener_click("'      + ticker + '")'}, "", QSID, 3, -1);
	DQ.addWorkspace({title:"Candle Screen", favicon:0},{background:1,click:'candle_screener_click("'    + ticker + '")'}, "", QSID, 3, -1);

	LQ.workspace['ws0'].tab.setAttribute("active", "");
	RQ.workspace['ws0'].tab.setAttribute("active", "");
	MQ.workspace['ws0'].tab.setAttribute("active", "");
	DQ.workspace['ws3'].tab.removeAttribute("active");

	$(LQ.qdiv + " .wsnew").css("display", "none");
	$(RQ.qdiv + " .wsnew").css("display", "none");

	DQ['ws0'].style.display='block';
	DQ['ws3'].style.display='none';

	if (!rpc)
		WS.send("stockpage " + ticker + " " + QVID + " " + QSID);
//		WS.send("chart " + MQ.qdiv.substr(1) + "ws0 " + ticker);
	$(MQ.qdiv)[0].className += " MQ";
	$(quadspace.quad[0].qdiv)[0].className += " LQ";
	$(quadspace.quad[1].qdiv)[0].className += " RQ";
	$(quadspace.quad[3].qdiv)[0].className += " DQ";

	BB = $("body > .BB-screener").clone();
	CT = $("body > .DQ-control").clone();
	DQ['ws0'].append(CT[0]);
	DQ['ws1'].append(BB[0]);

	Watchtable({type:'filter',dict:BTAB,QGID:DQ.qdiv + " .BB-screener", TID:"bulls", menu:1}); /* Set to ticker + -bulls */
	BB.css("display", "block");
	CT.css("display", "block");
	$(DQ.qdiv).css("overflow", "auto");

	/* Algo */
	LQ.addWorkspace({title:"AnyDay",       favicon:0},{background:0,click:"stockpage_anyday_click()"},  "", QSID, 0, -1);
	MQ.addWorkspace({title:"Algo",         favicon:0},{background:1,click:"stockpage_scatter_click()"}, "", QSID, 2, -1);
	RQ.addWorkspace({title:"Signals",      favicon:0},{background:1,click:"stockpage_signals_click()"}, "", QSID, 1, -1);
	LQ.addWorkspace({title:"Candle Bulls", favicon:0},{background:1,click:'stockpage_candle_table_click("' + ticker + '",0,1)'},  "", QSID, 0, -1);
	RQ.addWorkspace({title:"Candle Bears", favicon:0},{background:1,click:'stockpage_candle_table_click("' + ticker + '",1,0)'},  "", QSID, 1, -1);

	/* Fundamentals */
	MQ.addWorkspace({title:"Earnings",favicon:0},{background:1,live:0,click:"stockpage_etab()"}, "", QSID, 2, -1);

	SP[ticker]   = { LQ: LQ, MQ: MQ, RQ: RQ, DQ: DQ, QSP: quadspace, QVID: QVID, QSID: QSID, QTAB:Q[QVID].workspace['ws'+QSID].tab, TScreener: BB, TControl: CT, loaded: 1 };
	QMAP[ticker] = ticker;
/*	W.quadspace[QSID].quad[0].showtab(LQ.workspace['ws0'].tab);
	W.quadspace[QSID].quad[1].showtab(RQ.workspace['ws0'].tab);
	W.quadspace[QSID].quad[2].showtab(MQ.workspace['ws0'].tab);
	W.quadspace[QSID].quad[3].showtab(DQ.workspace['ws0'].tab);*/

	LQ.showtab(LQ.workspace['ws0'].tab);
	RQ.showtab(RQ.workspace['ws0'].tab);
	MQ.showtab(MQ.workspace['ws0'].tab);
	DQ.showtab(DQ.workspace['ws0'].tab);

	$("#P"+QVID+"Q"+QSID + " .chrome-tab-close").css("display", "none");
	if (sp)
		Q[QVID].showtab(Q[QVID].workspace['ws'+sp.QSID].tab);
}

/* StockPage onClick: workspace context */
function stockpage_indicator(){
	WS.send("sp-indicator bulls")
}

function stockpage_algo_screener_click(t){
	if (!SIGMON) {
		WS.send("sigmon " + t);
	} else {
		var DQ = SP[t].DQ;
		DQ['ws2'].append($("#SIGMON").detach()[0]);
		$("#SIGMON").css("display", "block");
	}
}

/* Bottom quad of the stockpage which hold various tables of which a candle table is one */
function stockpage_candle_screener_click(t){
	if (!CSR) {
		CSP = t;
		WS.send("csr");
	} else {
		var DQ = SP[t].DQ;
		DQ['ws3'].append($("#CSR_DIV").detach()[0]);
		$("#CSR_DIV").css("display", "block");
	}
}

/*
 * Send an RPC to the server to retrieve data for predefined parts of the built-in "stockpage"
 */
function stockpage_rpc(rpc, QID) {
	var QSID = W.current_quadspace;
	if (!$(QSID)[0][rpc]) {
		WS.send(rpc + W.current_quadspace + "q" + QID + "ws1 " + $(W.current_quadspace + "q2 .ws0 .chrome-tab-title").text())
		$(QSID)[0][rpc] = 1;
	}
}
function stockpage_anyday_click() {stockpage_rpc("sp-anyday ",  0)}
function stockpage_signals_click(){stockpage_rpc("sp-signals ", 1)}     // abducted?!?!? never written!?!?! too many cookies to remember
function stockpage_scatter_click(){stockpage_rpc("sp-scatter ", 2)}
function stockpage_candle_table_click(t,QID,b){WS.send("sp-candle " + t + " " + W.current_quadspace + "q"+QID+'ws2 ' + b)}

/* Stockpage Earnings Tab (Middle Quad) */
function stockpage_etab(){
	var QSID = W.current_quadspace;
	if (!$(QSID)[0].ETAB) {
		WS.send("etab " + W.current_quadspace + "q2ws2 " + $(W.current_quadspace + "q2 .ws0 .chrome-tab-title").text());
		$(QSID)[0].ETAB = 1;
	}
	$(QSID + "q2").css("overflow", "auto");
}

function stockview(ws)
{
	var id = window.event.target.parentNode.parentNode.parentNode.id.replace(/[^0-9]/g, '').split(''),qdiv, QSID, QID, LQ,DQ,MQ,RQ,quadspace;
	QSID = id[1]; QID = id[2];
	qdiv = "#P" + id[0] + "Q" + QSID + "q";
	quadspace = W.quadspace[W.current_workspace];
	LQ = quadspace.quad[0];
	MQ = quadspace.quad[1];
	RQ = quadspace.quad[2];
	DQ = quadspace.quad[3];
	DQ.showtab(DQ.workspace['ws'+ws].tab);
	LQ.showtab(LQ.workspace['ws'+ws].tab);
	RQ.showtab(RQ.workspace['ws'+ws].tab);
	MQ.showtab(MQ.workspace['ws'+ws].tab);
	for (var x=0; x<3; x++) {
		var click = $(qdiv + x + " .ws" + ws)[0].onclick;
		if (click)
			click();
	}
}

function rpc_etab(av){
	var th = av[2].split(":"), tid = av[1], t;

	t = "<table id=" + tid.substr(1) + 'eTAB class="ETAB XTAB"><thead><tr><th>#Days</th>';
	for (var x = 0; x<th.length; x++)
		t += "<th>" + th[x] + "</th>";
	t += "</tr></thead><tbody>";
	for (var x = 0; x<30; x++) {
		t += "<tr>";
		var tr = av[x+3].split(":");
		for (var z=0; z<tr.length; z++)
			t += "<td>" + tr[z] + "</td>";
		t += "</tr>";
	}
	t += "</tbody></table>";
	$(tid).append(t);
}

function rpc_anyday(av){
	var th = av[2].split(":"), tid = av[1], t, tab;

	for (var x = 0; x<av.length-2; x++) {
		var v = parseInt(av[x+2]);
		if (v >= 90)
			c = "green";
		else if (v >= 80)
			c = "blue";
		else if (v >= 70)
			c = "yellow";
		else
			c = "white";
		if (av[x+2] == '-')
			v = "-";
		t += "<td class="+c+">" + v + "</td>";
	}
	tab = $("body > .ANYTAB").clone();
	tab.css("display","block");
	$("tbody", tab).append(t);
	$(tid).append(tab);
}

function rpc_peak(av)
{
	var v,tr,chgpc,pdelta,buyFibDelta,h=0;
	for (var x = 0; x<PWT.length; x++) {
		v  = av[x+1].split(":");
		tr = PWT[x];
		tr.cells[4].innerText = v[0];
		tr.cells[5].innerText = v[1];
		pdelta = parseFloat(tr.cells[21].innerText);
		chgpc  = parseFloat(v[1]);
		TR = tr;
		if (chgpc <= pdelta) {
			h = 1;
//			console.log("setting sig, chgpc " + chgpc + " <= pdelta " + pdelta);
		} else {
			buyFibDelta = parseFloat(tr.cells[18].innerText);
			if (chgpc <= buyFibDelta) {
				h=1;
//				console.log("setting sig, chgpc " + chgpc + " <= buyFibDelta " + buyFibDelta);
			}
		}
		if (h && !tr.classList.contains("HIGHLIGHT")) {
			tr.className += " HIGHLIGHT";
			tr.cells[22].className = "sig";
			tr.cells[22].innerHTML = "";
		} else if (!h) {
			tr.classList.remove("HIGHLIGHT");
//			console.log("removing sig");
			tr.cells[22].className = "";
			tr.cells[22].innerHTML = "";
		}
		h = 0;
	}
}

function rpc_sigmon(av)	{
	if (av[2] == "]")
		return;
	localStorage['sigmon'] = av[2];
	table(av);
	SIGMON=1;
	if (av[3]) {
		var DQ = SP[t].DQ;
		DQ['ws2'].append($("#SIGMON").detach()[0]);
		$("#SIGMON").css("display", "block");
	}
}
