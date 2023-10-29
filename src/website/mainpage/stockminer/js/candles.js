/*********************************************
 * filename: mainpage/stockminer/js/candles.js
 * License:  Public Domain
 ********************************************/
function cload(){
	var s = $("#cstock").val();
	if (s == "")
		s = "^GSPC";
	WS.send("candle " + s);
}

function candle_colors(r,d)
{
	for (var x=1; x<16; x++) {
		var v = parseFloat(d[x+'d']);
		if (v > 0.0 && v < 5.0)
			$('td',r).eq(x+1).addClass('W');
		if (v >= 5.0 && v < 8.0)
			$('td',r).eq(x+1).addClass('LG');
		else if (v > -2.0 && v <= 0.0)
			$('td',r).eq(x+1).addClass('Y');
		else if (v <= -2.0 && v > -5.0)
			$('td',r).eq(x+1).addClass('OR');
		else if (v <= -5.0 && v > -10.0)
			$('td',r).eq(x+1).addClass('BR');
		else if (v <= -10.0)
			$('td',r).eq(x+1).addClass('RED');
		else if (v >= 8.0 && v < 10.0)
			$('td',r).eq(x+1).addClass('GR');
		else if (v >= 10.0 && v < 15.0)
			$('td',r).eq(x+1).addClass('DG');
		else if (v >= 15.0)
			$('td',r).eq(x+1).addClass('PR');
	}
	if (d.T=='1')
		$('td',r).eq(0).addClass('GR2');
	else
		$('td',r).eq(0).addClass('RED2');
}
var csr_keys = {852:3,853:4,854:5,855:6};
function csr_colors(r,d)
{
	var values = Object.values(d), keys = Object.keys(d), c, v;
	for (var x=0; x<values.length; x++) {
		v = values[x];
		if (v.indexOf("-")!=-1)
			continue;
		if (v == 'Bull') {
			$('td',r).eq(1).addClass('GR2');
			return;
		} else if (v == 'Bear') {
			$('td',r).eq(1).addClass('RED2');
			return;	
		}

		v = parseFloat(v);
		if (isNaN(v))
			continue;
		if (v > 0.0 && v < 5.0)
			c = 'yellow2';
		else if (v >= 5.0 && v < 8.0)
			c = 'limegreen2';
		else if (v > -2.0 && v <= 0.0)
			c = 'yellow2';
		else if (v <= -2.0 && v > -5.0)
			c = 'orange2';
		else if (v <= -5.0 && v > -10.0)
			c = 'brown';
		else if (v <= -10.0)
			c = 'R2';
		else if (v >= 8.0 && v < 10.0)
			c = 'green2';
		else if (v >= 10.0 && v < 15.0)
			c = 'DG2';
		else if (v >= 15.0)
			c = 'PR2';
		$('td',r).eq(csr_keys[keys[x]]).addClass(c);
	}
}

function rpc_stockpage_candles(av)
{
	var c = av[1], svg = av[2], bull = av[3], WSID = av[4], date = av[5], avg = parseFloat(av[6]), json = JSON.parse(av[7].replaceAll("^"," ")), cdiv = $("body > .candy").clone(), CGID = WSID+'ctab', color;
	$("#"+WSID).append(cdiv);
	$(cdiv).css("display", "block");
	$('.candy-pic',  cdiv).addClass(svg);
	$('.candy-date2',cdiv)[0].innerText += date;
	$('.candy-table',cdiv)[0].id = CGID;
	if (avg >= 0) {
		avg   = "+" + avg;
		color = 'GR2';
	} else
		color = 'RED2';
	$('.candy-avg', cdiv)[0].innerText = avg;
	$('.candy-avg', cdiv).addClass(color);

	newtable(CGID, "desc", [{"data":"800"},{"data":"915"},{"data":"853"},{"data":"855"},{"data":"860"},{"data":"865"},{"data":"871"}],7, 5);
	T[CGID].clear().rows.add(json).draw();
	$('h2', cdiv).html($("#"+CGID + " td:first-child").html());
	if (bull=="1") {
		$('h2',cdiv)[0].className = 'GR2';
		$("#"+CGID+" caption").html("Last 5 Bullish Candles");
	} else {
		$('h2',cdiv)[0].className = 'RED2';
		$("#"+CGID+" caption").html("Last 5 Bearish Candles");
	}
}

function candle_table(id,tab,data,n,width,c)
{
	T[id] = $("#"+id).DataTable({
			"columns": tab,
			"columnDefs": [{
			"targets":[n],
			"visible":false
		},width,{ type: 'natural', "targets": '_all'}],
	"createdRow": function (r,d){c(r,d)},
			"bPaginate":false,
			"order":[[1, "desc" ]],
			"paging":false,
			"info":false,
			"searching":false
			});
	T[id].clear().rows.add(JSON.parse(data.replaceAll("^", " "))).draw();
}

function rpc_csr(av)
{
	var t = [{"data":"900"},{"data":"800"},{"data":"915"},{"data":"852"},{"data":"853"},{"data":"854"},{"data":"855"},{"data":"899"}];
	candle_table("CSR",t,av[1],7,{},csr_colors);
	SP[CSP].DQ['ws3'].append($("#CSR_DIV").detach()[0]);
	$("#CSR_DIV").css("display", "block");

}

/* RPC: Candlestick Patterns */
function rpc_csp(av)
{
	var c = charts[av[1]], flags = av[2].split("!");
	c.cfreq['d1']();
	for (var x=0; x<flags.length; x++)
		c.addSeries(JSON.parse(flags[x]));
	return c;
}

var CFUNC = { czoom:czoom_cb };

/* candle zoom callback: cfunc[1] = candle_date
 *   - called by newchart() when a chart is created
 *   - newchart::CFUNC[cfunc[0]](cfunc, ticker, QGID);
 *   - adds a candle flag to the chart and 'zooms' to it
 */
var CC;
function czoom_cb(av,ticker,QGID)
{
	var c = C[ticker][av[1]], point = c[1];
	console.log("czoom callback: " + ticker + " " + QGID + " " + point);
	c = rpc_csp([0,ticker+'-'+QGID,c[0]]);
	CC = c;
	c.xAxis[0].setExtremes(c.series[0].xData[point-60], c.series[0].xData[Math.min(c.nr_points_1d-point, 50)], false);
	c.redraw();
	
}

/*
 * av[1] = ticker
 * av[2] = start_date
 * av[3] = QGID
 */
function rpc_candle_zoom(av)
{
	var ticker = av[1], start_date = av[2], QGID = av[3], point = av[4], candle = av[5],
	id   = QGID.replace(/[^0-9]/g, '').split(''),
	QVID = id[0], QSID = id[1], QID = id[2], WSID = id[3],
	quad = Q[QVID].quadspace[QSID].quad[QID];

//	RQ.addWorkspace({title:"Candle Bears", favicon:0},{background:1,click:'candle_qclick("' + ticker + '",1,0)'},  "", QSID, 1, -1);
//	W.quadspace[QSID].quad[1].showtab(RQ.workspace['ws0'].tab);

	if (!C[ticker])
		C[ticker] = {};
	C[ticker][start_date] = [candle,point];
	console.log("rpc_candle zooming: ticker: " + ticker + " start_date: " + start_date + " QGID: " + QGID);
	if (!charts[ticker+'-'+QGID]) {
		/* create new workspace in the Quad that the Stock's Candle Table is situated in */
		WSID = quad.addWorkspace({title:ticker, favicon:0},{background:1}, "", QSID, QID, -1);
		WS.send("qchart " + QGID + " " + ticker + " - czoom:"+start_date);
	} else {
		/* showtab() of workspace where the Stock's Chart is already loaded, display the candle flag and setExtremes to it */
		quad.showtab(quad.workspace['ws'+WSID].tab);
		czoom_cb([0,start_date,point], ticker, QGID);
	}
}

/* Candles QuadSpace - #QuadVerse 0 */
function candle(av)
{
	var t = [{"data":"C"},{"data":"D"},{"data":"1d"},{"data":"2d"},{"data":"3d"},{"data":"4d"},{"data":"5d"},{"data":"6d"},{"data":"7d"},
			 {"data":"8d"},{"data":"9d"},{"data":"10d"},{"data":"11d"},{"data":"12d"},{"data":"13d"},{"data":"14d"},{"data":"15d"},{"data":"T"}], cinit, id, QGID, WSID;

	if (!T['CRT'])
		cinit = 1;

	candle_table("CRT",t,av[1],17,{ "width": "34px", "targets":[0]},candle_colors);

	/*
	 * Candle Stock Table onclick td(1,2)
	 */
	if (cinit) {
        $('#CRT tbody tr').on('click','td:eq(0), td:eq(1)',function(){
			var date = $("td:eq(1)", this.parentNode)[0].innerHTML;
			ticker   = $("#CRT caption").text().split(" ")[0],
			c = C[ticker];
			if (c && (c=c[date]))
				rpc_candle_zoom([0,ticker,date,QGID,c]);
			else {
				QGID = $(this).parents().eq(5)[0].id;
				id   = QGID.replace(/[^0-9]/g, '').split('');
				WSID = parseInt(id[2]) + 1;
				QGID = "P"+id[0]+"Q"+id[1]+"q"+id[2]+"ws"+WSID;
				WS.send("czoom " + ticker + " " + date + " " + QGID);
			}
		});
	}
}
