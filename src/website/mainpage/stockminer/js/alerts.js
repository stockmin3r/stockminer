/*********************************************
 * filename: mainpage/stockminer/js/alerts.js
 * License:  Public Domain
 ********************************************/
var ATYPE = [ 'Price', 'Volume', 'Candle', 'SMA', 'EMA', 'MACD' ];
var CTYPE = [ 'PT', 'Target', 'AVG', 'RVOL' ];

/* Private Alerts */
function rpc_pexec(av)
{
	var a = av[1].split("!");
	for (var x=0; x<a.length; x++)
		A.push("exec " + a[x].split("^").join(" "));
	showAlerts();
}

/* Public Alerts */
function rpc_gexec(av)
{
	var a = av[1].split("!");
	for (var x=0; x<a.length; x++)
		G.push("exec " + a[x].split("^").join(" "));
}

/* icon dialog onclick */
function alerts(){
	$("#ALERTS").css("display","block");
	if (!A.length) {
		$(".pri").css("display", "block");
		$(".en").css("display", "none");
		ALERTS.indi = "pri";
	} else
		$("#ALERTS .en").css("display", "block");
}

/* build Alerts in-Dialog HTML table */
function showAlerts()
{
	var tbl = $("#ALERTS .enabled table tbody"),row,a;
	$("#ALERTS .en").css("display", "block");
	tbl.html("");
	for (var x=0; x<A.length; x++) {
		a   = A[x].split(" ");
		row = '<tr idx='+x+'><td>'+ATYPE[a[6]]+'</td><td>'+ a[4] + '</td><td>'+a[3]+'</td><td>'+CTYPE[a[7]]+'</td><td onclick=edit_alert()>' + a[8] + '</td><td class=wc title=Remove onclick=remove_alert()>×</td></tr>';
		$(tbl).append(row);
	}
	if (x != 0)
		$("#ALERTS .boxmain").addClass("boxtick");
}

/* Screener Quad 1 Alerts in-Table */
function loadAlerts(w,WID)
{
	var tbl = $("#PATAB tbody"),row,a,ALERTS,pub = $(".watchlist-select option:selected").val();
	tbl.html("");
	if (pub==1)
		ALERTS = G; /* Global/Public */
	else
		ALERTS = A; /* Private */
	for (var x=0; x<ALERTS.length; x++) {
		a   = ALERTS[x].split(" ");
		if (a[3] != w)
			continue;
		row = '<tr><td>'+a[4]+'</td><td>'+ ATYPE[a[6]] + '</td><td>'+CTYPE[a[7]]+'</td><td onclick=edit_alert()>' + a[8] + '</td></tr>';
		$(tbl).append(row);
	}
	$("#PATAB")[0].watchlist_id = WID;
}

function notifier()
{
	if ($("#notify").css("display") != "block")
		$("#notify").css("display", "block")
	else
		$("#notify").css("display", "none");
}

function rpc_notify(av)  {
	var tr = "<div class=notify-row><div class=notify-icon><i class=notify-chart></i></div><div class=notify-msg>" + av[1] + " ALERT " + av[2] + "</div><div class=notify-time>" + av[3] + "</div><div class=notify-close>×</div></div>", count = 1;
	$("#notify").append(tr);
	if (!$("#notify-count")[0].count)
		$("#notify-count")[0].count = 1;
	else
		count = ++$("#notify-count")[0].count;
	$("#notify-count")[0].textContent = "(" + count + ")";
	$("#bell").css("text-shadow", "0 0 0 #821eda");
	$("#bell").css("animation", "shake 0.5s");
}

function remove_alert()
{
	var n     = window.event.target.parentNode,alert_id,
	row       = $(n).index(),
	type      = $("td:nth-child(1)",n).text(),
	stock     = $("td:nth-child(2)",n).text(),
	watchlist = $("td:nth-child(3)",n).text(),
	cond      = $("td:nth-child(4)",n).text(),
	arg       = $("td:nth-child(5)",n).text(), watchlist_id;

	document.getElementById("ATAB").deleteRow(row+1); // Enabled alerts dialog AlertsTable
	alert_id = A.splice(row, 1)[0].split(" ")[1];
	for (var x = 0; x<WL.length; x++) {
		if (WL[x].name == watchlist) {
			watchlist_id = WL[x].id;
			WL[x].alerts--;
			for (var x = 0; y<GW.length; y++)
				if (GW[y].name == watchlist)
					GW[y].alerts--;
			break;
		}
	}
	WS.send("rma " + watchlist + " " + alert_id);
	/* Remove Alert from Watchlists Quad Alert Table if the same watchlist is currently loaded there */
	if ($("#PATAB")[0].watchlist_id == watchlist_id) {
		var rows = [];
		$('#PATAB tbody tr').each(function(){
			var row = [], td = $('td', this);
			for (var x = 0; x<td.length; x++)
				row.push(td[x].textContent);
			rows.push(row)
		});
		for (var x = 0; x<rows.length; x++) {
			if (rows[x][0] == stock && rows[x][1] == type && rows[x][2] == cond && rows[x][3] == arg) {
				document.getElementById("PATAB").deleteRow(x+1);
			}
		}
	}
	pubload(WL);
}

function getAlert(watchlist,ticker)
{
	if (!A.length)
		return 0;
	for (var x = 0; x<A.length; x++) {
		var a = A[x].split(" ");
		if (watchlist == a[3] && ticker == a[4])
			return x;
	}
	return -1;
}

function edit_alert(){
	var td = window.event.target;
	window.event.stopPropagation();
	if (td.tagName === 'INPUT')
		return;
	var oldPT = td.innerText,
	watchlist = td.previousSibling.previousSibling.innerText,
	ticker    = td.previousSibling.previousSibling.previousSibling.innerText,
	row       = td.parentNode.rowIndex-1;
	edit_alert_update(oldPT,watchlist,ticker,td,row);
}

function edit_alert_update(oldPT,watchlist,ticker,td,row)
{
	var input,newPT,aIdx,a;

	td.innerHTML = '<input class=etd type=text value='+oldPT+'>';
	input = $("input",td)[0];
	input.focus();
	$(input).keyup(function(e) {
		if (e.keyCode === 13) {
			if (!ticker)
				ticker = getTableCell("#"+td.parentNode.parentNode.parentNode.id, row, 'Ticker');
			aIdx = getAlert(watchlist,ticker);
			if (aIdx < 0) {
				console.log("getAlert(" + watchlist + ") returned -1");
				return;
			}
			a       = A[aIdx].split(" ");
			newPT   = td.innerHTML = a[8] = input.value;
			A[aIdx] = a.join(" ");
			WS.send(A[aIdx] + " U");

			/* Update Alerts Table (Top Right Quad in Screener QuadVerse) */
			for (var x = 0; x<WL.length; x++) {
				if (WL[x].name == watchlist) {
					if ($("#PATAB")[0].watchlist_id == WL[x].id) {
						$('#PATAB tbody tr').each(function(){
							td = $('td', this);
							if (td[0].textContent == ticker && parseFloat(td[3].textContent) == oldPT) {
								td[3].textContent = newPT;
								return;
							}
						});
					}
					return;
				}
			}
		}
	});
}

function exec_alert(){
	var cmd = "exec "+randstr(7),period=" [",cond,condarg,id1,id2,id3,p,wname,WID;

	if (ALERTS.indi == "pri") {
		var arg1=$("#parg1").val(),ticker=$("#parg2").val();
		if (arg1 == null || ticker == null)
			return;

		p       = 0;
		cond    = "0 ";         /* CONDITION_PRICE */
		condarg = "0 " + arg1;  /* #define CONDARG_PT 0 */
		id1     = "pricewatch"; /* <input checkbox id=pricewatch checked> */
		id2     = "#pwatch";    /* <select id=pwatch> */
		id3     = ".pri";       /* <div class="imenu .pri"> */
	} else if (ALERTS.indi === 'vol') {
		/* exec id 0|1|2(ADD|DEL|ALERT) WatchList Ticker [Periods] Condition condargclass condarg */
		var arg1=$("#varg1").val(),arg2=$("#varg2").val(),arg3=$("#varg3").val(),ticker=$("#varg4").val();
		if (ticker == null)
			return;

		p       = 1;
		cond    = "1 ";         /* CONDITION_VOLUME */
		id1     = "volwatch";   /* <input checkbox id=volwatch checked> */
		id2     = "#vwatch";    /* <select id=vwatch> */
		id3     = ".vol";       /* <div class="imenu .vol"> */

		/* condargclass + condarg */
		if (arg1 != null)
			condarg = "1 " + arg1;   /* #define CONDARG_VOL_TARGET 1 */
		else if (arg2 != null)
			condarg = "2 " + arg2;   /* #define CONDARG_VOL_AVG    2 */
		else if (arg3 != null)
			condarg = "3 " + arg3;   /* #define CONDARG_VOL_RVOL   3 */
		else return;
	}

	/* ADDSTOCK + WatchlistName + Ticker */
	wname = $(id2 + " option:selected")[0].text;
	cmd += " 0 " + wname + " " + ticker;

	/* Period */
	if (p) {
		$("#ALERTS " + id3 + " .checkbox input:checked").each(function(){
			period += $(this).attr("period") + " ";
			console.log(period);
		});
	}
	period += "] ";

	/* Condargs */
	cmd += period + cond + condarg;

	console.log(cmd);
	A[A.length] = cmd;
	$("#ALERTS .box .boxmenu ul li").removeClass("boxtick");
	$("#ALERTS .imenu").css("display", "none");
	for (var x = 0; x<WL.length; x++) {
		if (WL[x].name == wname) {
			WL[x].alerts++;
			WID = WL[x].id;
			break;
		}
	}
	for (var x = 0; x<GW.length; x++) {
		if (GW[x].id == WID) {
			GW[x].alerts++;
			G[G.length] = cmd;
			break;
		}
	}
	showAlerts();
	loadAlerts(wname,WID);
	WS.send(cmd);
	if ($(".watchlist-select option:selected").val())
		pubload(GW);
	else
		pubload(WL);
}
