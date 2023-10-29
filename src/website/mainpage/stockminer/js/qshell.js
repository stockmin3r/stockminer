/************************************************
 * filename: mainpage/stockminer/js/qshell.js
 * License:  Public Domain
 ***********************************************/
var QSH = {};

function qsh_exec()
{
	var span = window.event.target, name = span.parentNode.parentNode.firstChild.innerText;
	if (span.innerText == "Run") {
		span.innerText = "Undo";
		span.className += " yellow2";
		qscript = QSH[name];
	} else {
		span.innerText = "Run";
		span.classList.remove("yellow2");
		qscript = QSH[name + ".undo"];
	}

	for (var x = 0; x<qscript.length; x++) {
		var code = qscript[x].split(" ");
		if (code == "")
			break;
		var f = QSHELL['sh_' + code[0] + "_" + code[1]];
		if (!f)
			console.log("qsh_exec func undefined: " + f);
		else
			f(code);
	}
}

/*
 * "QSHELL" Webtest Scripts
 */
function rpc_qsh(av) {
	av = av.slice(1);   // clear qsh
	av = av.join(" ");
	av = av.split("!");
	for (var x = 0; x<av.length; x++) {
		var v = av[x].split("*"), name = v[0], desc = v[1];
		if (name.indexOf("undo") == -1)
			$("#SCRTAB").append($('<tr><td>' + name + "</td><td>" + desc + "</td><td><span class=qact onclick=qsh_exec()>Run</span></td></tr>"))
		QSH[name] = av[x].split("\n").slice(1);
	}
}

function rpc_netsh(av)  {
	console.log('sh_' + av[1] + "_" + av[2]);
	window['sh_' + av[1] + "_" + av[2]](av.slice(1))
}

/* ****************
 * TEST MECHANIZER
 ******************/
function sh_set_watchlist(av){
	var sh = $("#share").css("display", "block");
	$("#P0Q1q2 .watchtable").append(sh.detach());
	sh[0].watchtable = "morphtab";
	$("#share .checkbox_ctrl")[0].click();
	$("#watchname").val(av[2]);
}
function sh_save_watchlist()     {watchlist_save();$("#share .checkbox_ctrl")[0].click()}
function sh_set_quadverse(av)    {this.quadverse(av[2], 0)}
function sh_set_display(av)      {$(av[2]).css("display", av[3])}
function sh_set_text(av)         {$(av[2]).val(av[3])}
function sh_add_stocks(av)       {WS.send("watchlist_addstock " + av[2] + " " + av[3].split(",").join(" "))}
function sh_set_grid(av)         {W.grid("#P"+av[3] + "Q" + av[4] + "q" + av[5], av[2], 0)}
function sh_add_quadspace(av)    {qspace([0,av[2]])}
function sh_new_workspace(av)    {wspace([0,av[2],av[3],av[4]])}
function sh_del_watchlist(av){
	var watchlist = av[2];
	watchlist_remove(watchlist);
}
function sh_del_stocks(av){
	var stocks = av[2].split(",");
	for (var x = 0; x<stocks.length; x++)
		WS.send("watchlist_delstock " + av[3] + " " + stocks[x]);
}
function sh_reset_watchlists(av) {
	for (var x = 0; x<WL.length; x++) {
		console.log("watchlist: " + WL[0].name);
		watchlist_remove(WL[0].name)
	}
}

function sh_move_tag(av){
	var src = $(av[2].replaceAll("_"," ")),
	dst = $(av[3].replaceAll("_", " "));
	$(dst).append($(src).detach())
}

function sh_add_stockpage(av){
	if (W.PID != av[3])
		QUADVERSE_SWITCH(0,0,av[3]);
	av = av[2].split(",");
	for (var x = 0; x<av.length; x++) {
		stockpage(av[x],-1);
		QSH_SP[av[x]] = SP[av[x]];
	}
}

function sh_del_stockpage(av){
	Q[av[3]].removeTab(QSH_SP[av[2]].QTAB, 0);
}

function sh_del_quadspace(av){
	var QVID = av[2], QSID = av[3];
	Q[QVID].removeTab(Q[QVID].workspace['ws'+QSID].tab);
}

function sh_set_shake(av){
	$(av[2]).effect("shake",{distance:2})
}

function sh_call_colmod(av){
	var name = "", screener = Screeners[0];
	if (av[2].indexOf('-') != -1)
		name = av[2];
	colmod(screener, screener.watchtable_id, name, 0, av[3]);
}

function sh_del_tr(av){
	var thead = document.querySelectorAll(av[2] + " thead th"), args = av[3].split("="), 
	key = colmap(args[0]),
	val = args[1].split(",")
	for (var x = 0; x<val.length; x++)
		T[av[2].substr(1)].rows(function(i,data) {return data[key] === val[x]}).remove().draw();
}

function sh_set_debugger(av){
	debugger;
}

function sh_show_tab(av){
	var QVID = av[2], QSID = av[3];
	Q[QVID].showtab(Q[QVID].workspace['ws'+QSID].tab);
}

function sh_add_chart(av){
	Q[av[3]].quadspace[av[4]].quad[av[5]].bsChart(av[4], av[5], av[2]);
}


