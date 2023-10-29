/**********************************************
 * filename: mainpage/stockminer/js/rpc.js
 * License:  Public Domain
 *********************************************/

function register_rpc(ops)
{
	op = Object.assign({}, op, ops);
}

function emsg(){WS.send("error")}

function rpc_menu(av)
{
	var idx = av[1].split("&");
	var spx = idx[0].split("!");
	var ndx = idx[1].split("!");
	var dow = idx[2].split("!");
	var XLK = idx[3].split("!");
	SPX_pr.innerText = spx[0];
	NDX_pr.innerText = ndx[0];
	DOW_pr.innerText = dow[0];
	XLK_pr.innerText = XLK[0];
	if (parseFloat(spx[1]) < 0) {
		SPX_pc.style.color="brown";
		SPX_pc.innerText = spx[1];
	} else {
		SPX_pc.style.color="green";
		SPX_pc.innerText = "+"+spx[1];
	}
	if (parseFloat(ndx[1]) < 0) {
		NDX_pc.style.color="brown";
		NDX_pc.innerText = ndx[1];
	} else {
		NDX_pc.style.color="green";
		NDX_pc.innerText = "+"+ndx[1];
	}
	if (parseFloat(dow[1]) < 0) {
		DOW_pc.style.color="brown";
		DOW_pc.innerText = dow[1];
	} else {
		DOW_pc.style.color="green";
		DOW_pc.innerText = "+"+dow[1];
	}
	if (parseFloat(XLK[1]) < 0) {
		XLK_pc.style.color="brown";
		XLK_pc.innerText = XLK[1];
	} else {
		XLK_pc.style.color="green";
		XLK_pc.innerText = "+"+XLK[1];
	}
}

function addPoint(av){
	var ohlc = [], v = [], c = charts[av[1]], s,
	data     = JSON.parse(av[2]);
	ohlc[0] = data[0];
	ohlc[1] = data[1];
	ohlc[2] = data[2];
	ohlc[3] = data[3];
	ohlc[4] = data[4];
	v[0]    = data[0];
	v[1]    = data[5];
	s = c.series[0];
	s.addPoint(ohlc, true, false, true);
	s = c.series[1];
	s.addPoint(v, true, false, true);
	console.log("addPoint: " + av[2]);
}

function update(av){
	var ohlc = [0], v = [0], c = charts[av[1]], s,
	data = JSON.parse(av[2]);
	ohlc[1] = data[0];
	ohlc[2] = data[1];
	ohlc[3] = data[2];
	ohlc[4] = data[3];
	v[1]    = data[4];

	/* Update Candlestick OHLC */
	s = c.series[0];
	ohlc[0] = s.options.data[s.options.data.length-1][0];
	s.options.data[s.options.data.length-1] = ohlc;
	s.setData(s.options.data,true,true);

	/* Update Volume */
	s = c.series[1];
	v[0] = s.options.data[s.options.data.length-1][0];
	s.options.data[s.options.data.length-1] = v;
	s.setData(s.options.data,true,true);
}
var MINI;
function rpc_upmini(av){
	MINI = av;
	var ticker = av[1], div = av[2], data, cpr = av[4], delta = av[5],mpc,mpr
	s = charts[div].series[0],l;

	if (av[3] == "") {
		console.log("fake mini: " + JSON.stringify(av));
		return;
	}
	data = JSON.parse(av[3])
	l    = s.data[s.data.length-1];
	if (av[6] == "add")
		s.addPoint(data);
	else
		l.update(data, true);
	mpr = document.getElementById(div + "mpr");
	if (mpr === null) {
		console.log("upmini no div: " + ticker + " " + div);
		return;
	}
	mpr.innerText = "$"+cpr;
	mpc = document.getElementById(div + "mpc");
	if (parseFloat(delta) > 0)
		delta = "+"+delta+"%";
	else {
		delta = delta + "%";
		mpc.style.color="red";
	}
	mpc.innerText = delta;
}

var TBL,TNAME;
function table(av)   {
	TNAME = av[1];
	TBL   = av;
	/*console.log("TABLE: " + av[1] + " " + av[2]);*/
	T[av[1]].clear().rows.add(JSON.parse(av[2])).draw()
}

function ctable(av)
{
	if (av[2] == "]")
		return;
	localStorage[av[1]] = av[2];
	table(av);
	if (av[1] == "FDT") {
		localStorage.age = new Date().toLocaleString('en-US', { timeZone: 'America/New_York', hour12:false });
		window['airstat']();
	}
}

function cacheload(name)
{
	table([0, name,localStorage[name]]);
}

function rpc_newtab(av)
{
	colmod(screener, av[1], "", NO_RPC);
}

function rpc_qspace(av) {Q[av[1]].qspace()}

/* RPC: Server instructs client to Create new Workspace */
function rpc_wspace(av)
{
	Q[av[1]].addBlankspace(av[1],av[2],av[3])
}
