/*********************************************
 * filename: mainpage/stockminer/js/chart.js
 * License:  Public Domain
 ********************************************/
var ctypes = {},
groups     = [['minute',[1,15]],['hour',[1]],['day',[1]],['week',[1]],['month',[1, 3, 6]],['year',null]],
SEL        = {"m1":0,"m15":1,"d1":2,"W1":3,"M1":4};

var bar_plotOptions = {
	dataLabels:{enabled:true}
};

var	bar_legend = {
	layout:'vertical',
	align:'right',
	verticalAlign:'top',
	x:-40,
	y:80,
	floating:true,
	borderWidth:1,
	backgroundColor:Highcharts.defaultOptions.legend.backgroundColor || '#FFFFFF',
	shadow:true
};

var highcharts_darkTheme  = {
	global:{timezoneOffset:240},
	lang:{rangeSelectorZoom:""},
	chart: {
		marginRight:46,
		marginBottom:1,
		borderWidth:1,
		plotBorderWidth:1,
		spacingBottom:1,
		plotBackgroundColor:'',
		reflow:true,
		panning:true,
		backgroundColor:{linearGradient:[0,0,500,500],stops:[[0,'rgb(25, 28, 32)'],[1,'rgb(13, 13, 13)']]}
	},
	credits:      {text:''},
	mapNavigation:{enabled:false,enableMouseWheelZoom:true},
	exporting:    {enabled:false},
	navigator:    {enabled:false},
	rangeSelector:{enabled:false},
	tooltip:      {split:true},
	plotOptions:  {
		macd:     {zones:[{value:0,color:'red'},{color:"green"}]},
		series:   {borderWidth:1,shadow:true,dataGrouping:{enabled:true,forced:true,units:[['minute',[1]]]},states:{inactive:{opacity:1}}},
		candlestick:{cropThreshold:1000}
	},
	xAxis:        {tickInterval:60000}
},

//{title:"DeathChart",theme:"whiteTheme", type: "bar", conf:{chart{backgroundColor:{linearGradient:[0,0,500,500],stops:[[0,'rgb(130, 30, 218)'],[1,'rgb(110, 110, 110)']]}}}}

highcharts_purpleTheme  = {
	chart: {
      backgroundColor:{linearGradient:[0, 0, 500, 500],stops:[[0, "rgb(130, 30, 218)"],[1, "rgb(110, 110, 110)"]]},
      gridLineWidth:1,
      borderWidth:2,
      plotBackgroundColor:'rgba(125, 35, 210, .9)',
      plotShadow:true,
      plotBorderWidth:1
	}
},

highcharts_whiteTheme  = {
	chart: {
      backgroundColor:{linearGradient:[0, 0, 500, 500],stops:[[0,'rgb(255, 255, 255)'],[1,'rgb(240, 240, 255)']]},
      gridLineWidth:1,
      borderWidth:2,
      plotBackgroundColor:'rgba(255, 255, 255, .9)',
      plotShadow:true,
      plotBorderWidth:1
	}
},
//ChartThemesArray = [{name:"darkTheme", theme:darkTheme}, {name:"whiteTheme", theme:whiteTheme}, {name:"purpleTheme", theme:purpleTheme}],
ChartThemesDict  = {};

function ChartTheme(theme)
{
	var div = window.event.target.parentNode.parentNode.classList[1].split("-")[1],
	ticker = $("#"+div).highcharts().title.textStr;
	console.log("chart theme ticker: " + ticker + " theme: " + theme.name + " chartware: " + theme.chartware);
	Highcharts.setOptions(theme);
	WS.send("chart " + ticker + " " + div);
}

function rpc_chart_themes(av)
{
	for (var x = 0; x<av.length; x++) {
		var theme = av[x];
		ChartThemesDict['Theme'+x] = {name:theme.name, theme:theme.theme, callback:function(i){ChartTheme(ChartThemesDict[i].theme)}};
	}
}

function init_charts()
{
	// Chart Themes
	rpc_chart_themes([{name:"darkTheme",  chartware:"highcharts", theme:highcharts_darkTheme},
					  {name:"whiteTheme", chartware:"highcharts", theme:highcharts_whiteTheme}]);
	Highcharts.setOptions(highcharts_darkTheme);
}

/* av['type']   = 'bar'
   av['data']   = 'data'
   av['dtype']  = 'array|csv|excel'
   av['conf']   = highcharts|plotly custom config
   av['div']    = existing chart div string (chartname-QGID) (to overwrite) or destination empty div
   av['QGID']   = QGID of new chart
*/
function gui_chart_onclick(type)
{
	var gcharts = $("#charts")[0], args = {};

	args['type']      = type;
	args['div']       = gcharts.CID;
	args['QGID']      = gcharts.CID.split("-")[1];
	args['title']     = gcharts.title;
	args['data']      = gcharts.data;
	args['dtype']     = gcharts.dtype;
	args['theme']     = gcharts.theme;
	args['conf']      = gcharts.conf;
	args['chartware'] = "highcharts";

	// load the chart
	chart(args);

	// close the "Chart Import" "GUI" <div>
	qclose();
}

/*
# {title:"DeathMeter",theme:"whiteTheme", chartware: "highcharts|plotly", type: "line", conf:HighChartsConfig }
Date,DeathScore
15/8,20
16/8,26
17/8,39
*/
//var conf = `# title DeathChart\n# theme whiteTheme\n--\nDate,Death\n15/8,20\n16/8,26\n17/8,39`
function chart_import(CID)
{
	var gcharts = $("#charts")[0],conf,csv,file = $("#upload-csv-file"),QGID,action,objtype,filetype;

	gcharts.CID = CID;
	QGID        = CID.split("-")[1];

	file[0].onchange = function(){
		readfile(file[0].files[0],function(data){
			gcharts.dtype = file[0].files[0].type.split("/")[1];
			gcharts.style.display='block';
			if (gcharts.dtype == 'csv') {
				// depending on the CSV formatting there may be a custom chart config
				// there may be a request for a stock chart with or without volume
				if (data.substr(0,3) == "# {") {
					nl = data.indexOf("\n");
					gcharts.conf  = JSON.parse(data.substr(2,nl-2));
					gcharts.data  = data.substr(nl+1);
					gcharts.theme = gcharts.conf.theme;
					gcharts.title = gcharts.conf.title;
				} else {
					gcharts.data  = data;
					gcharts.theme = "darkTheme";
				}
			} else if (gcharts.dtype == 'xlsx') {
				console.log("sending xlsx");
				action  = ACTION_DATA_CHART_WORKSPACE;
				objtype = OBJTYPE_DATA;
				filetye = FILETYPE_XLSX;
				sendfile({action:action,objtype:objype,filetype:filetype,data:data,filename:filename,argv:'/'+QGID});
			}
		});
	};
	file.click();
}


var AVV,NARGS,HP,CONF,CONF2,CHR;

/* av['type']      = 'stock|bar|etc'
   av['data']      = 'data'
   av['dtype']     = 'array|csv|excel'
   av['conf']      = highcharts|plotly custom config
   av['chartware'] = highcharts|plotly
   av['div']       = destination div
*/
function chart(av)
{
	CHR = av;
	var QGID = av.QGID, div = av.div, series, conf = {};

	var conf = {
		yAxis:{title:{align:'middle'}},
		plotOptions:{},
		credits:{text:''},
		chart:{
			zoomType:'x',
			type:av.type,
		}
	},

	data = av.data;
	switch (av.type) {
		case 'bar':
			conf.plotOptions.bar = bar_plotOptions;
			conf.legend          = bar_legend;
			break;
		case 'line':
			conf.plotOptions.series = {dataGrouping:{enabled:false}};
			break;
	}

	switch (av.dtype) {
		case 'array':
			av.ohlc   = [];
			av.volume = [];
			len       = data.length;
			for (var x=0; x<len; x++) {
				av.ohlc.push([data[x][0],data[x][1],data[x][2],data[x][3],data[x][4]]);
				av.volume.push([data[x][0],data[x][5]]);
			}
			break;
		case 'csv':
			conf['data'] = {'csv':data};
			break;
	}
	CONF2 = conf;
	console.log("div: " + av.div + " title: " + av.title + " QGID: " + QGID);
	if (!ID(div)) {
		div    = document.createElement('div');
		div.id = av.title+"-"+QGID;
	}

	if (QGID && typeof(div) != "string")
		WMAP[QGID].quad.appendChild(div);

	// Assign the custom highcharts|plotly config, if one is provided
	if (av.conf)
		conf = Object.assign(conf,av.conf);

	if (av.title)
		conf.title = {text:av.title};

	switch (av.theme) {
		case 'whiteTheme':
			conf = Object.assign(conf,whiteTheme);
			break;
		case 'purpleTheme':
			conf = Object.assign(conf,purpleTheme);
			break;
	}
	CONF = conf;
	if (av.chartware == "highcharts") {
		console.log("stockchart!")
		if (av.type == "stock")
			stockchart(av);
		else
			Highcharts.chart(av.div,conf);
	} else if (av.chartware == "plotly") {
		console.log("plotly");
	}
}

/*
 * av['data']   = data
 * av['div']    = chart div
 * av['ticker'] = ticker
 * av['cb']     = callback function after init
 * av['nr_1d']  = # 1d points (optional)
 * av['class']  = class name (only if a <div class=av['class']> already exists for the chart)
 */
function stockchart(av)
{
	NARGS = av;
	av = JSON.parse(av[1]);
	var ticker = av.ticker,div = av.div, data=av.data, DAY=av.nr_1d, cfunc = av.cb, box, quad,
	QGID=div.split("-")[1],ws, len = data.length,i=0,q=0,c,ohlc = [],volume = [],cfreq={};

	switch (av.dtype) {
		case 'array':
			for (var x=0; x<len; x++) {
				ohlc.push([data[x][0],data[x][1],data[x][2],data[x][3],data[x][4]]);
				volume.push([data[x][0],data[x][5]]);
			}
			break;
	}

	console.log("stockchart: " + ticker + " " + div);
	if (av['class'] != "")
		box = document.querySelectorAll("." + av['class'])[0];
	else
		box = document.createElement('div');
	box.id  = div;
	quad    = WMAP[QGID];
	quad.appendChild(box);
	quad    = quad.w;

	Highcharts.stockChart(div,{
	title:   {text:ticker},
	credits: {text:''},
	chart:   {
		zoomType:'x',
		events:{
			redraw:drawLabel,
			dataLoad:drawLabel,
			load:function(){
				var chart         = this, pos = chart.plotLeft + chart.plotSizeX, id="#"+div;
				charts[div]       = this;
				this.nr_1d_points = DAY;
				ws = "ws"+quad.current_workspace;
				quad.workspace[ws].obj.push({type:"chart", ref:chart});
				this.menu = null;
/*				if (len > 50 && DAY != len) {
					console.log("extremes len: fulldata: " + len + " 1m DAY ticks: " + DAY);
					chart.xAxis[0].setExtremes(data[len-50][0], data[len-1][0], false);
					chart.redraw();
				}*/

				chart.renderer.image(LINE,  28, 5, 25, 20).add().attr({title:"Line",                class:"line svg"});
				chart.renderer.image(AREA,  53, 5, 25, 20).add().attr({title:"Area",                class:"area svg"});
				chart.renderer.image(CDL,   78, 5, 25, 34).add().attr({title:"Candlestick",         class:"candle svg"});
				chart.renderer.image(OHLC, 103, 5, 25, 20).add().attr({title:"ohlc",                class:"ohlc svg"});
				chart.renderer.image(VOL,  125, 5, 25, 20).add().attr({title:"Volume Profile",      class:"vol svg"});
				chart.renderer.image(CSP,  150, 5, 25, 34).add().attr({title:"Candle Patterns",     class:"csp svg"});
				chart.renderer.image(INDI, 170, 5, 25, 34).add().attr({title:"Indicators",          class:"indi svg"});
				chart.renderer.text("🛠", 6, 20).add().attr({title:"Menu",class:"ctools svg",id:"ct-"+div,onclick:"ChartMenu()",fill:"green"});

				$(id+" .line")[0].onclick   = ctypes['line']   = function(){chart.series[0].update({type:'line',color:"#821eda",useOhlcData:true});localStorage.ctype="line"};
				$(id+" .candle")[0].onclick = ctypes['candle'] = function(){chart.series[0].update({type:'candlestick',color:"#821eda",useOhlcData:true});localStorage.ctype="candle"};
				$(id+" .ohlc")[0].onclick   = ctypes['ohlc']   = function(){chart.series[0].update({type:'ohlc',color:"#821eda",useOhlcData:true});localStorage.ctype="ohlc"};
				$(id+" .vol")[0].onclick    = function(){
					chart.menu.indi="vbp";
					if (!chart.indi)
						chart.indi = {vbp:1};
					else if (chart.indi.vbp) {
						/* if on then disable */
						chart.indi.vbp = 0;
						chart.get(ticker+'-price-vbp').remove();
						return;
					}
					chart.indi.vbp = 1;
					iadd(chart)
				};
//				$(id+" .piv")[0].onclick  = function(){chart.menu.indi="pivotpoints";iadd(chart)};
				$(id+" .indi")[0].onclick = function(){iopen(chart, ticker, div)};
				$(id+" .csp")[0].onclick  = function(){
					if (!chart.indi)
						chart.indi = {csp:1};
					else if (chart.indi.csp) {
						chart.indi.csp = 0;
						chart.get("CSP-BULL").remove();
						chart.get("CSP-BEAR").remove();
						return;
					}
					chart.indi.csp = 1;
					WS.send("csp " + ticker + " " + div)};

				$(id+" .area")[0].onclick   = ctypes['area'] = function(){
					var yx = chart.yAxis[0];
					yx.setExtremes(yx.getExtremes().dataMin, null, true, false);
					localStorage.ctype = "area";
					chart.series[0].update({type:'area',color:"#821eda",useOhlcData:true});localStorage.ctype="area"
				};
				var sel = $("<select class=freq><option class=m1>1 Minute</option><option class=m15>15 Minutes</option><option class=d1>1 Day</option><option class=W1>1 Week</option><option class=M1>1 Month</option></select>");				
				$("#"+div).prepend(sel);
				chart.sel = sel = sel[0];
				if (localStorage.cfreq)
					sel.selectedIndex = SEL[localStorage.cfreq];
				sel.onchange = function(f, c) {
					if (!Number.isInteger(f))
						f = sel.selectedIndex;
					var D = chart.series[0].options.data, freq, n, u, g = 1;
					switch (f) {
						case 0:
							freq = "m1";
							n    = D.length-DAY;
							u    = 'minute';
							if (n > 180)
								n = 180;
							if (DAY!=D.length)
								n = (D.length-n);
							break;
						case 1:freq = "m15"; n = D.length-DAY; g = 15; u = 'minute'; n = D.length - n;            break;
						case 2:freq = "d1";  n = DAY<200?DAY:200;      u = 'day';    if (D.length == DAY)  n = 0; break;
						case 3:freq = "W1";  n = 0;                    u = 'week';   break;
						case 4:freq = "M1";  n = 0;                    u = 'month';  break;
					}
					chart.xAxis[0].setExtremes(D[n][0], null,true);
					chart.update({plotOptions:{series:{dataGrouping:{units:[[u,[g]]]}}}});
					localStorage.cfreq = freq;
				}
				cfreq['m1']  = function(){sel.onchange(0,chart)}
				cfreq['m15'] = function(){sel.onchange(1,chart)}
				cfreq['d1']  = function(){sel.onchange(2,chart)}
				cfreq['W1']  = function(){sel.onchange(3,chart)}
				cfreq['M1']  = function(){sel.onchange(4,chart)}

				// tbis takes a considerable amount of time
				if (localStorage.ctype)
					ctypes[localStorage.ctype]();
				if (localStorage.cfreq)
					cfreq [localStorage.cfreq]();
				else
					sel.onchange(2,chart);

				ID(div).className += " qchart";
				$("#"+div).draggable({handle:'.highcharts-title',grid:[20,20],snap:"#"+ws,stop:function(){updatePosition(box, ticker)}});
				$("#"+div).resizable({grid:[20,20],resize:function(){chart.reflow();rsz(box, div)}});
				bshide("#"+QGID);
				chart.reflow();

				chart.cfreq = cfreq;
				if (cfunc && cfunc != "-") {
					cfunc = cfunc.split(":");
					CFUNC[cfunc[0]](cfunc, ticker, QGID);
				}
				for (var k in PSET)
					if (PSET[k].on && PSET[k].ws == 0)
						preset(PSET[k].n,this,div);
				$("#"+div + " .highcharts-title").dblclick(function(){rename(div=="graph"?"graph":div.split("-")[1])});
				iopen(chart,ticker,div);
				$(chart.menu).css("display", "none");
				document.querySelector("#"+div).onclick = function(){selobj($("#"+div)[0])};
				$("#load").remove();
			}
		},
	},
    series:[{
		type:'candlestick',
		name:ticker,
		data:ohlc,
		id:ticker+'-price',
		cropThreshold:1000,
		dataGrouping: {approximation:'ohlc'},
		marker:       {enabled:false}
	},{
		type:'column',
		name:'Volume',
		data:volume,
		id:ticker+'-volume',
		cropThreshold:1000,
		dataGrouping: {approximation:'sum'},
		yAxis:1,
	}],
	yAxis:[{
		height:'65%',
		lineWidth:1,
		resize:{enabled:true}
	},{
		labels:{align:'right',x:-3},
		top:'65%',
		height:'35%',
		offset:0,
		lineWidth:1,
	}]
});
}



//ticker, div, data, curprice, delta, ex, c, av[8]:moveTo
function rpc_minichart(av)
{
	var points = JSON.parse(av[3]),
	ticker     = av[1],
	div        = av[2],
	delta      = av[5],
	h          = av[9],
	i          = av[10],
	box        = document.createElement('div'),
	lbx        = document.createElement('div'),
	rbx        = document.createElement('div'),
	tools      = document.createElement('span');
	console.log("mini div: " + div);
	rbx.id     = div + "rbx";  // P0Q1q0ws2AAPLrbx
	box.id     = div + "box";  // P0Q1q0ws2AAPLbox

	if (h > 250)
		box.classList.add("megabox");
	else
		box.classList.add("minibox");
	lbx.classList.add("lbx");
	rbx.classList.add("rbx");
	tools.innerHTML = "<span class=mtools>&#128736;</span>";
	box.appendChild(lbx);
	box.appendChild(rbx);
	rbx.innerHTML = "<div id=" + div + "></div>";
	if (av[8] != '0') {
		$(box).appendTo(av[8]);
		box.className += " split";
		console.log("moving " + box.id + " to " + av[8]);
	} else
		document.getElementById(div.split("-")[0]).appendChild(box);

	if (parseFloat(delta) > 0)
		delta = "+"+delta+"%";
	else
		delta = delta + "%";
	lbx.innerHTML = "<div class=mpr id=" + div + "mpr" + ">$" + av[4] + "</div><div class=mpc id=" + div + "mpc" + ">" + delta + "</div>";
	Highcharts.stockChart(div,{
    title:   {text:ticker},
	credits: {text:'',},
	chart:{
		height:h,
		reflow:true,
		events:{
			load:function(){
				var len = points.length,x,yMax=this.yAxis[0].max,chart=this;
				if (av[6] == '1' && len > 50) {
					this.xAxis[0].setExtremes(points[len-50][0], null, false);
					this.redraw();
					lbx.onclick = function(){exmini(chart,box)};
				}
				charts[div] = this;
			}
		},
		spacingBottom:0,
		marginBottom:0,
		reflow:true,
		panning:true,
		backgroundColor:{linearGradient:[0,0,500,500],stops:[[0,'rgb(25,28,32)'],[1,'rgb(13,13,13)']]}
	},
	mapNavigation: {enabled:false,enableMouseWheelZoom:true},
	exporting:     {enabled:false},
	plotOptions:   {series:{pointPadding:0.01,groupPadding:0.7,shadow:true}},
	navigator:     {enabled:false},
	rangeSelector: {enabled:false},
    tooltip:       {split:true},
    yAxis:         {labels:{align:'right',x:-3},lineWidth:1,resize:{enabled:true}},
    series:       [{type:'line',name:ticker,data:points,color:av[7],dataGrouping:{enabled:false}}]
	});
}

/* Indicator Objects */
function ichart(args,indi,unit)
{
	var ticker = args.ticker, div = args.div, data=args.data, len=data.length, i=0, ohlc=[], volume=[], box = document.createElement('div');
	for (i; i<len; i+= 1) {ohlc.push([data[i][0],data[i][1],data[i][2],data[i][3],data[i][4]]);volume.push([data[i][0],data[i][5]])}
	box.id  = div;
	WMAP[div.split("-")[1]].appendChild(box);

	console.log("ichart: " + div);
	Highcharts.stockChart(div,{
	title:   {text:ticker},
	credits: {text:''},
	chart:   {
		events:{
			load:function(){
				charts[div] = this;
				var D = this.series[0].options.data;
//				this.xAxis[0].setExtremes(D[D.length-100][0], null,true);
				WMAP[div.split("-")[1]].w.workspace['ws0'].obj.push({type:"chart", ref:this});
				for (var x = 0; x<indi.length; x++) {
					menu      = Object.values(charts)[0].menu;
					menu.indi = indi[x];
					this.menu = menu;
					iadd(this, 0, ticker+'-price');
				}
				this.reflow();
			}
		},
		marginRight:25,marginBottom:1,borderWidth:1,plotBorderWidth:1,spacingBottom:1,reflow:true,panning:true,
		backgroundColor:{linearGradient:[0,0,500,500],stops:[[0,'rgb(25, 28, 32)'],[1,'rgb(13, 13, 13)']]}
	},
	mapNavigation:{enabled:false,enableMouseWheelZoom:true},exporting:{enabled:false},navigator:{enabled:false},rangeSelector:{enabled:false},tooltip:{split:true},
	plotOptions:  {
		macd:     {zones:[{value:0,color:'red'},{color:"green"}]},
		series:   {borderWidth:1,shadow:true,dataGrouping:{forced:false,units:[[unit,[1]]]},states:{inactive:{opacity:1}}},
	},
	xAxis:        {tickInterval:60000},
	yAxis:[{
		height:'45%',
		lineWidth:1,
		resize:{enabled:true}
	}],
    series:[{
		type:'line',
		id:ticker+'-price',
		useOhlcData:true,
		name:ticker,
		color:"darkcyan",
		data:ohlc,
		marker:{enabled:false}
	}]
	});
}

function macd(i,i2)
{
	var col = window.event.target.value;
	console.log("macd color: " + col);
	if (i == "zones")
		OPT["MACD"][i][i2].color=col;
	else
		OPT["MACD"][i].zones[i2].color=col;

	for (var k in charts) {
		var c = charts[k],
		s     = c.get(c.title.textStr + "-price-macd");
		console.log("updating: " + c.title.textStr + "-price-macd");
		s.update(OPT['MACD']);
	}
}

var iAxis = [   'ad',
	            'atr',
	            'cci',
	            'cmf',	
	            'macd',
	            'mfi',
	            'roc',
	            'rsi',
	            'ao',
	            'aroon',
	            'aroonoscillator',
	            'trix',
	            'apo',
	            'dpo',
	            'ppo',
	            'natr',
	            'williamsr',
	            'stochastic',
	            'slowstochastic',
	            'linearRegression',
	            'linearRegressionSlope',
	            'linearRegressionIntercept',
	            'linearRegressionAngle'
	        ];

function iadd(chart,sh,t,rpc)
{
	var menu = chart.menu, type = menu.indi, /* Currently Selected Indicator */
	QGID,sid, id, s, config = {}, input, param, params = {}, period = [];

	console.log("iadd indicator: " + menu.indi);
	if (type === "add")
		return Chart_addTicker(chart, $('.add-ticker',menu)[0].value);
	
	if (!t)
		sid = $("select option:selected", menu)[0].text;
	else
		sid = t;

	id = sid+"-"+type;
	if (OPT[type])
		config = OPT[type];

	config.params = {};
	input = menu.querySelectorAll("." + type + " input");
	for (var x = 0; x<input.length; x++) {
		param = input[x].className;
		if (param == "period")
			period.push(input[x].value);
		else 
			config.params[param] = input[x].value;
	}
	if (period.length == 1)
		period = period[0];

	config.params['period'] = period;
	config.id       = id;
	config.linkedTo = sid;
	config.type     = type;

	if (iAxis.indexOf(type) != -1) {
		var yAxis = chart.addAxis({id:id+"-y",offset:0,opposite:true,title:{text:type,rotation:0,y:-10},tickPixelInterval:10,showLastLabel:false,labels:{enabled:false}},false,false);
		config.yAxis = id+"-y";
		ryx(chart);
	} else {
		if (type == 'vbp') {
			config['params']['volumeSeriesID'] = chart.title.textStr+'-volume';
            config['dataLabels']               = {enabled:false};
			config['zoneLines']                = {enabled:false};
			config['volumeDivision']           = {styles:{positiveColor:'rgb(0, 246, 246, 0.6)',negativeColor:'rgb(254, 55, 57, 0.6)'}};
		} else if (type == 'pivotpoints') {
            config['zIndex']     = 0;
			config['period']     = 28;
            config['lineWidth']  = 1;
            config['dataLabels'] = {overflow:'none',crop:false,y:4,style:{fontSize:9}}
		}
		config.yAxis = chart.get(sid).options.yAxis;
	}
	chart.addSeries(config);
	s = chart.get(sid);
	if (!s.ilist)
		s.ilist = [type];
	else
		s.ilist.push(type);
	if (sh)
		showEnabled(chart);

	QGID = chart.container.parentNode.id;
	if (QGID == "graph")
		QGID = "P0Q0q1ws0";
	id   = QGID.replace(/[^0-9]/g, '').split('');

	/* TSLA-P0Q5q0ws0&macd&shortPeriod:10&longPeriod:20 */
	if (!rpc && !Q[id[0]].quadspace[id[1]].SP)
		WS.send(QUPDATE_ADD_CHART_INDI + id[0] + " " + id[1] + " " + id[2] + " " + id[3] + " " + QGID+"&"+type);

	if (!menu.enabled)
		menu.enabled = {};
	menu.enabled[type] = 1;
	$('button', menu)[0].className = 'del';
	$('button', menu)[0].innerText = 'Remove';
}

function iremove(chart)
{
	var menu = chart.menu,
	mm       = $(".mm", menu),
	sid      = $("select option:selected", mm)[0].text,
	s        = chart.get(sid),
	QGID     = chart.container.parentNode.id,
	id       = replace(/[^0-9]/g, '').split(''),
	indi     = menu.indi;

	for (var x=0; x<s.ilist.length; x++) {
		if (s.ilist[x] == indi) {
			s.ilist.slice(i, 1);
			$(this).remove();
			s.remove(1);
			ryx(chart);
			WS.send(QUPDATE_ADD_CHART_INDI + id[0] + " " + id[1] + " " + id[2] + " " + id[3] + " " + QGID + "&" + indi);
			$('button', menu)[0].className = '';
			menu.enabled[indi] = 0;
			$('button', menu)[0].innerText = 'Add';
		}
	}
}

/* Indicator Dialog Open */
function iopen(chart,ticker,div)
{
	if (!chart.menu) {
		var menu = $("#INDI").clone()[0];
		$(menu).attr("id", "INDI" + div);
		$("#"+div).prepend(menu);
		$(menu).css("display", "block");
		chart.menu   = menu;
		chart.iapply = $('button', menu);
		$(menu).draggable();
		$('select', menu).each(function(){
			var opt = document.createElement("option");
			opt.appendChild(document.createTextNode(ticker+'-price'));
			opt.value = '0';
			$(this).append(opt);
		});
		$(chart.iapply).click(function(){
			if (chart.menu.enabled[chart.menu.indi])
				iremove(chart);
			else
				iadd(chart);
		});
		loadpresets(menu);
	} else
		showEnabled(chart);
}

function closeindi(){
	var box = window.event.target.parentNode;
	box.style.display="none";
	$(box).find(".imenu").css("display", "none");
	$(box).find(".mm").css("display", "block");
}

function indicator(indicator) {
	var box = window.event.target.parentNode, menu, button;
	$(box).find("*").removeClass("boxtick");
	$(window.event.target).addClass("boxtick");
	box = box.parentNode.parentNode;
	$(box).find(".imenu").css("display", "none");
	$(box).find("."+indicator).css("display", "block");
	menu = box.parentNode;
	menu.indi = indicator;
	if (!menu.enabled) {
		menu.enabled = {};
		return;
	}
	button = $('button', menu)[0];
	if (menu.enabled[indicator]) {
		button.className = 'del';
		button.innerText = 'Remove';
	} else {
		button.className = '';
		button.innerText = 'Add';
	}
}

function indi_save_preset()
{
	var node = window.event.target.parentNode,
	sid      = $(node.parentNode).find("select option:selected")[0].text,
	name     = $(node.parentNode).find(".prename").val(),
	chart    = charts[sid.split("-")[0]],
	s        = chart.get(sid),
	preset   = name + " ",indi="";

	for (var x = 0; x<s.nr_indi; x++) {
		console.log("indicator: " + s.ilist[x]);
		indi += s.ilist[x] + "^";
	}
	console.log("indi: " + indi);
	preset += indi + "0^0";
	WS.send("preset " + preset);
	rpc_indicator(name,indi+"0^0");
	loadpresets(chart.menu);
	$(chart.menu).find(".imenu").css("display", "none");
	$(chart.menu).find(".PRE").css("display", "block");
}

function pract()
{
	var on = check(),
		tr = window.event.target.parentNode;
	while (tr.tagName != "TR")
		tr = tr.parentNode;
	row    = tr.rowIndex-1;
	var ws = $(tr).find(".prselect")[0].value;
	console.log("row: " + row + " ws: " + ws);
	PSET[row].ws = ws;
	PSET[row].on = on;
	WS.send("pset " + row + " " + ws + " " + on);
	event.stopPropagation();
}

function loadpresets(menu)
{
	var l = PR.length;
	if (!l)
		return;

	var tbl = $(menu).find(".prelist table tbody");
	for (var x = 0; x<l; x++) {
		var preset = PR[x].split(" "),
		ac         = preset[1].split("^"),pr,
		active     = ac[ac.length-1];
		console.log("preset: " + preset + " " + "active: " + active);
		if (active=='1')
			pr = prON;
		else
			pr = prOFF;
		row        = '<tr><td>' + pr + '</td><td>'+preset[0]+'</td><td>'+preset[1].replaceAll("^", " ").split(" ").slice(0, -2).join(" ") + '</td><td>'+PMS[x]+'</td><td class=wc title=Remove>&times;</td></tr>';
		$(tbl).append(row);
	}
}

function check(p){
	var on, n = window.event.target;
	if (p==1)
		n = window.event.target.firstChild;
	if ($(n).hasClass("tick")) {
		$(n).removeClass("tick");
		$(n).addClass("untick");
		on = 0;
	} else {
		$(n).removeClass("untick");
		$(n).addClass("tick");
		on = 1;
	}
	event.stopPropagation();
	return on;
}

function preset(name, chart, id)
{
	if (chart == null)
		chart = $("#"+id).highcharts();
	console.log("PRESET CALLED: " + name);
	var menu  = chart.menu,
	indi      = PMD[name].split("^");
	if (!menu) {
		iopen(chart,chart.title.textStr, id);
		menu = chart.menu;
	}
	for (var x=0; x<indi.length-2; x++) {
		menu.indi = indi[x];
		console.log("ADDING INDICATOR: " + indi[x]);
		iadd(chart);
	}
	$(menu).css("display", "none");
}

function prselect()
{
	var op = window.event.target,
	sel    = opt.parentNode, tr = sel;
	while (tr.tagName != "TR")
		tr = tr.parentNode;

	console.log("prselect: " + (tr.rowIndex-1) + " " + opt.value + " 2");
	PMS[row] = window.event.target.parentNode.innerHTML;
	WS.send("pset " + tr.rowIndex-1 + " " + opt.value + " 2");
}

function rpc_indicator(av)
{
	var name = av[1], indicators = av[2];
	console.log("SETPRE: " + name + " INDI: " + indicators);
	PR.push(name + " " + indicators);
	PMD[name] = indicators;
	var ar    = indicators.split("^"),
	ws        = ar[ar.length-2],
	on        = ar[ar.length-1];
	PSET.push({ws:ws,on:on,n:name});
	PMenu['Preset'+(PMenu.length+1)] = {"name":name, callback:function(itemKey,opt,e){preset(name,null,$(this).closest(".chart").attr("id"))}};
	var s = "<select class=prselect onchange=prselect()>";
	if (ws == '0')
		s += "<option value=0>All Charts</option>";
	else {
		s += "<option value=" + ws + ">" + WName[ws-1] + "</option>";
		s += "<option value=0>All Charts</option>";
	}
	for (var x = 0; x<WName.length; x++) {
		if (x-1 == ws)
			continue;
		s += "<option value="+(x+1)+">" + WName[x] + "</option>";
	}
	s += "</select>";
	PMS.push(s);
}

function showEnabled(chart)
{
	var mm  = $(".mm", chart.menu),
	id      = $(mm).find("select option:selected")[0].text,
	s       = chart.get(id),
	tab     = $("tbody", mm);

	tab.html("");
	if (!s.ilist)
		s.ilist = [];
	for (var x=0; x<s.ilist.length; x++)
		$(tab).append('<tr><td>' + s.ilist[x] + '</td><td i='+x+' class=ic2>✖</td></tr>');
	$(".ic2", tab).click(function(){
		var i = $(this).attr("i"), indi = s.ilist[i];
		console.log("idx: " + i + " " + indi);
		s.ilist.slice(i, 1);
		$(this.parentNode).remove();
		chart.get(id+'-'+indi).remove(1);
		ryx(chart);
	});

	$(".imenu", chart.menu).css("display", "none");
	$(mm).css("display", "block");
	$(chart.menu).css("display", "block");
}

function rsz(box,div)
{
	var h = -box.offsetHeight+10;
	$("#"+div+" .WS").attr("style", "margin-top:"+h+"px !important");
//	console.log("resizing: " + div);
}
function hideY(chart,name) {
    $(chart.yAxis).each(function(i, item) {
        if (item.userOptions.id == name)
            chart.yAxis[i].update({visible: false});
  });
}

var LINE = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='25' height='16' stroke='%23000' stroke-width='1.5'  stroke-linecap='round' stroke-linejoin='round' fill='black' fill-rule='evenodd'%3E%3Cdefs%3E%3ClinearGradient id='A' x1='0%25' y1='0%25' x2='100%25' y2='100%25'%3E%3Cstop offset='0%25' stop-color='purple'/%3E%3Cstop offset='100%25' stop-color='%23672965'/%3E%3C/linearGradient%3E%3C/defs%3E%3Cpath d='M.5 14.5L10 5l5 5.5L23.5 1' stroke='url(%23A)' fill='none'/%3E%3C/svg%3E";
var AREA = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='25' height='20' stroke='%23000' stroke-linecap='round' stroke-linejoin='round' fill='%23fff' fill-rule='evenodd'%3E%3Cdefs%3E%3ClinearGradient id='A' x1='0%25' y1='0%25' x2='100%25' y2='100%25'%3E%3Cstop offset='0%25' stop-color='%23821eda'/%3E%3Cstop offset='100%25' stop-color='%23672965'/%3E%3C/linearGradient%3E%3C/defs%3E%3Cpath d='M23.5 19H.5v-4.5L10 5l5 5.5L23.5 1z' stroke='url(%23A)' fill='url(%23A)'/%3E%3C/svg%3E";
var CDL  = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg'%3E%3Cg%3E%3Crect fill='none' id='canvas_background' height='34' width='34' y='-1' x='-1'/%3E%3Cg display='none' overflow='visible' y='0' x='0' height='100%25' width='100%25' id='canvasGrid'%3E%3Crect fill='url(%23gridpattern)' stroke-width='0' y='0' x='0' height='100%25' width='100%25'/%3E%3C/g%3E%3C/g%3E%3Cg%3E%3Crect id='svg_1' height='17.43785' width='9.8752' y='5.06228' x='10.46864' stroke-width='0.5' stroke='%23000' fill='%23821eda'/%3E%3Cline stroke-linecap='null' stroke-linejoin='null' id='svg_3' y2='27.56273' x2='15.59374' y1='22.75014' x1='15.59374' stroke='%23821eda' fill='none'/%3E%3Cline stroke-linecap='null' stroke-linejoin='null' id='svg_4' y2='4.81228' x2='15.59374' y1='-0.00032' x1='15.59374' stroke='%23821eda' fill='none'/%3E%3C/g%3E%3C/svg%3E";
var OHLC = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg'%3E%3Cg%3E%3Crect fill='none' id='canvas_background' height='26' width='26' y='-1' x='-1'/%3E%3Cg display='none' overflow='visible' y='0' x='0' height='100%25' width='100%25' id='canvasGrid'%3E%3Crect fill='url(%23gridpattern)' stroke-width='0' y='0' x='0' height='100%25' width='100%25'/%3E%3C/g%3E%3C/g%3E%3Cg%3E%3Ctitle%3ELayer 1%3C/title%3E%3Cline stroke-linecap='undefined' stroke-linejoin='undefined' id='svg_1' y2='21.24977' x2='10.84378' y1='3.62521' x1='10.90628' stroke-width='1.5' stroke='%23821eda' fill='none'/%3E%3Cline stroke-linecap='undefined' stroke-linejoin='undefined' id='svg_2' y2='17.62486' x2='18.40609' y1='17.56236' x1='11.53126' stroke-width='1.5' stroke='%23821eda' fill='none'/%3E%3Cline stroke-linecap='undefined' stroke-linejoin='undefined' id='svg_3' y2='6.18765' x2='10.1563' y1='6.12515' x1='3.28147' stroke-width='1.5' stroke='%23821eda' fill='none'/%3E%3C/g%3E%3C/svg%3E";
var VOL  = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg'%3E%3Cg%3E%3Ctitle%3ELayer 1%3C/title%3E%3C/g%3E%3Cg%3E%3Ctitle%3Ebackground%3C/title%3E%3C/g%3E%3Cg%3E%3Ctitle%3Ebackground%3C/title%3E%3Crect fill-opacity='0' stroke='%23000000' stroke-width='0.5' x='-1' y='-1' width='26' height='26' id='canvas_background' fill='%23000000'/%3E%3Cpath id='svg_1' d='m13.27182,5.20054c0.32035,-0.58575 0.90646,-1.34879 -0.10055,-1.53287c-0.28051,-0.1454 -0.56112,-0.40268 -0.84127,-0.11607c-0.62333,0.24862 -1.16368,0.59631 -0.50806,1.24702c0.29074,0.42023 0.55517,1.33712 0.8845,1.4381c0.19656,-0.34079 0.38069,-0.68875 0.56537,-1.03619l0,0zm-1.37703,1.61733c-0.57222,-1.0312 -1.0327,-2.13749 -1.73308,-3.08879c-0.85183,-0.9183 -2.53024,-0.50895 -2.91166,0.67553c-0.50668,1.17107 0.58656,2.56928 1.82417,2.4227c0.93915,0.01588 1.88364,0.05858 2.82058,-0.00944zm4.75502,0.03481c1.21705,-0.16472 1.91694,-1.71174 1.224,-2.73892c-0.61406,-1.09618 -2.3807,-1.1344 -2.99744,-0.02043c-0.53403,0.90059 -0.99755,1.84166 -1.4876,2.76694c1.08341,0.06255 2.17701,0.04796 3.26104,-0.00759l0,0zm2.01556,1.31898c0.4149,-0.36861 2.02157,-0.80317 0.80476,-1.15325c-0.30143,-0.08637 -0.65286,-0.48257 -0.92377,-0.37415c-0.58827,0.7385 -1.49279,1.17173 -2.43719,1.12635c-0.18488,0.06136 -1.12784,-0.09332 -0.87306,0.08237c0.66624,0.38357 1.3157,0.80013 2.00483,1.14059c0.48268,-0.25956 0.95113,-0.54548 1.42443,-0.82192l0,0zm-9.47907,0.20869c0.35481,-0.20869 0.70962,-0.41739 1.06442,-0.62608c-1.08453,-0.05441 -2.32568,0.10343 -3.14531,-0.77733c-0.23109,-0.26126 -0.45268,-0.56038 -0.77296,-0.23146c-0.38362,0.26685 -1.5636,0.54032 -0.65508,0.86607c0.79112,0.47186 1.57667,0.95524 2.38734,1.39219c0.38629,-0.18291 0.75043,-0.41151 1.12159,-0.62339l0,0zm5.1998,2.32858c0.45979,-0.37301 1.57348,-0.72839 1.61068,-1.12867c-1.03954,-0.604 -2.04881,-1.27649 -3.13637,-1.78702c-0.37427,-0.14337 -0.67772,0.07723 -0.97898,0.26884c-0.89084,0.53503 -1.80312,1.03685 -2.66498,1.61857c1.12715,0.70375 2.26785,1.38669 3.41965,2.04851c0.58748,-0.3327 1.16736,-0.679 1.75001,-1.02023l0,0zm4.61281,1.16429c0.47701,-0.30389 0.95401,-0.60777 1.43102,-0.91166c-0.00391,-1.01782 0.01219,-2.03606 -0.01538,-3.05352c-0.96274,0.5424 -1.91178,1.11188 -2.84481,1.70487c-0.10205,1.04977 -0.05029,2.11691 -0.02811,3.17197c0.49233,-0.29287 0.9727,-0.60611 1.45728,-0.91166zm-11.18718,-0.64124c0.04845,-0.8676 0.15288,-1.85662 -0.88338,-2.11363c-0.6544,-0.30817 -1.56146,-1.09776 -2.06561,-1.08984c-0.01571,0.97759 -0.00739,1.95537 -0.00909,2.93305c0.96666,0.60909 1.91616,1.24751 2.90368,1.82168c0.09575,-0.50394 0.03316,-1.03814 0.05439,-1.55126zm6.92223,3.35925c0.57176,-0.36446 1.14352,-0.72893 1.71528,-1.09339c-0.00265,-1.05078 0.04038,-2.10306 -0.00665,-3.15271c-0.62837,0.18194 -1.34707,0.74432 -2.00909,1.09364c-0.49571,0.29318 -0.99142,0.58637 -1.48713,0.87955c0.00978,1.12197 -0.02672,2.24599 0.03615,3.3663c0.5926,-0.34961 1.16946,-0.72614 1.75144,-1.09339l0,0zm-2.39678,-0.58976c0,-0.56105 0,-1.1221 0,-1.68315c-1.1478,-0.67727 -2.29192,-1.36096 -3.4442,-2.03044c-0.06372,1.07289 -0.02731,2.14981 -0.03692,3.22455c1.14754,0.72543 2.28319,1.47067 3.44525,2.1722c0.04522,-0.55906 0.02871,-1.12259 0.03586,-1.68315zm6.69026,2.25541c0.46777,-0.31451 0.93555,-0.62902 1.40332,-0.94352c-0.01952,-0.99714 0.04819,-2.00777 -0.05028,-2.9939c-0.97279,0.56039 -1.91164,1.19839 -2.86709,1.79663c0.02295,1.03171 -0.04713,2.07522 0.05842,3.09811c0.49351,-0.30585 0.97198,-0.63596 1.45563,-0.95732zm-11.22604,-0.55901c-0.00381,-0.52281 -0.00762,-1.04562 -0.01143,-1.56844c-0.96641,-0.61047 -1.92064,-1.24137 -2.90156,-1.82784c-0.06139,1.00163 -0.0264,2.00716 -0.03563,3.01058c0.97492,0.65159 1.93887,1.32045 2.92686,1.95171c0.03785,-0.52045 0.01907,-1.04441 0.02176,-1.56601zm6.95727,3.4357c0.56784,-0.38078 1.13567,-0.76155 1.70351,-1.14232c-0.00132,-1.06174 0.02005,-2.12395 -0.00821,-3.18533c-1.15589,0.71962 -2.30405,1.45248 -3.44374,2.19796c-0.1288,1.02522 -0.04529,2.08225 -0.04548,3.11867c0.35373,0.21754 1.25814,-0.74658 1.79391,-0.98899l0,0zm-2.42149,-0.45451c-0.06866,-0.73211 0.29532,-1.76259 -0.62624,-2.04641c-0.92935,-0.59433 -1.85496,-1.19515 -2.79654,-1.76959c-0.13724,1.00511 0.00629,2.0522 -0.09913,3.06961c1.12913,0.82416 2.28777,1.61203 3.46752,2.36028c0.09462,-0.52435 0.03577,-1.07978 0.05439,-1.61388l0,0zm-3.94891,-0.19354c-1.37431,-0.92631 -2.74862,-1.85261 -4.12292,-2.77892c0.00156,-2.85871 -0.03756,-5.71811 0.01269,-8.57629c0.68819,-0.42874 1.42053,-0.7802 2.12877,-1.17356c-0.45745,-1.32221 0.20256,-2.92898 1.52901,-3.4239c1.11194,-0.46066 2.45229,-0.04495 3.16121,0.92483c0.51491,-0.26664 1.02983,-0.53328 1.54474,-0.79992c0.51768,0.26905 1.03536,0.5381 1.55304,0.80715c0.74179,-0.88843 2.01267,-1.39544 3.11399,-0.89754c1.32197,0.46916 2.04254,2.07234 1.56845,3.39248c0.71346,0.40671 1.48915,0.72452 2.14331,1.22081c0.029,2.84178 0.00026,5.68398 -0.00237,8.52594c-2.73454,1.82722 -5.42477,3.72418 -8.2037,5.48192c-0.80807,-0.00022 -1.50555,-0.88611 -2.25538,-1.24529c-0.72493,-0.48392 -1.44802,-0.97062 -2.17085,-1.45772l0,0z' opacity='undefined' stroke-dasharray='null' stroke='%23821eda' fill='none'/%3E%3C/g%3E%3C/svg%3E";
var PIV  = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg'%3E%3Cg%3E%3Ctitle%3ELayer 1%3C/title%3E%3Cline stroke-width='2' stroke-linecap='undefined' stroke-linejoin='undefined' id='svg_1' y2='2.81736' x2='21.79372' y1='2.78432' x1='2.20628' opacity='undefined' stroke='%23821eda' fill='none'/%3E%3Cline stroke-width='2' stroke-linecap='undefined' stroke-linejoin='undefined' id='svg_7' y2='9.73737' x2='21.79372' y1='9.70434' x1='2.20628' opacity='undefined' stroke='%23821eda' fill='none'/%3E%3Cline stroke-width='2' stroke-linecap='undefined' stroke-linejoin='undefined' id='svg_15' y2='17.43362' x2='21.79372' y1='17.40058' x1='2.20628' opacity='undefined' stroke='%23821eda' fill='none'/%3E%3C/g%3E%3C/svg%3E";
var CSP  = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg'%3E%3Cg id='Layer_1'%3E%3Cpath id='svg_1' d='m7.81445,19.59043c0.53309,-1.51191 1.9849,-2.40325 3.3949,-2.5175c0,-0.32185 0,-0.64371 0,-0.96556c-2.2632,-0.18009 -4.5025,-1.23506 -6.00665,-3.16195c-1.37411,-1.6348 -2.10235,-3.83367 -2.20245,-6.05057c-0.79497,-0.62748 0.74242,-1.08404 0.92971,-0.38993c-0.4283,0.57586 -0.01335,1.47911 0.04752,2.18442c0.67826,3.33322 3.33172,5.99716 6.36842,6.46981c0.77652,0.38131 0.98312,-0.1311 0.8504,-0.91623c0.16618,-0.62075 -0.43963,-0.4323 -0.80201,-0.5482c-2.39416,-0.46509 -4.47419,-2.53035 -5.08805,-5.15584c-0.17587,-0.75118 -0.23306,-1.57866 -0.44723,-2.19415c0.4551,-0.39438 1.54778,-0.10504 0.94361,0.59947c0.06474,2.98287 2.29924,5.61469 4.94252,5.98961c0.54652,0.29608 0.48385,-0.21045 0.46422,-0.61601c0.2896,-0.98597 -0.57406,-0.83776 -1.15447,-1.08026c-1.63228,-0.61488 -2.82868,-2.37982 -2.9418,-4.289c-0.78081,-0.75399 0.93053,-1.1483 0.94261,-0.36369c-0.4485,0.57932 0.0572,1.51996 0.32742,2.13794c0.61054,1.10295 1.66902,1.84756 2.82624,1.99534c0,-0.49248 0,-0.98495 0,-1.47743c-1.07586,-0.18431 -1.93519,-1.22565 -2.05689,-2.42055c-0.73301,-0.66133 0.86202,-0.98024 0.97672,-0.34673c-0.39799,0.34568 -0.09852,0.90032 0.13514,1.27497c0.16696,0.22155 0.99327,0.99267 0.94502,0.41173c0,-0.65679 0,-1.31358 0,-1.97038c-0.50396,-0.3546 0.19008,-0.59075 -0.113,-1.00938c0.17432,-0.43442 0.25329,-1.55811 0.38383,-1.57148c0.43727,0.69028 0.27403,1.64844 0.68665,2.24877c-0.22769,0.27149 -0.26725,0.57268 -0.2284,0.97505c0,0.52754 0,1.05507 0,1.58261c0.65213,-0.21236 1.24242,-0.81337 1.28368,-1.60277c-0.74517,-0.96947 1.81342,-0.7548 0.75154,0.10471c-0.1106,1.16888 -1.01029,2.13232 -2.03523,2.32927c0,0.49054 0,0.98107 0,1.47161c1.7542,-0.19524 3.30118,-1.90892 3.33228,-3.89365c-0.73042,-0.89407 1.64926,-0.8348 0.83474,-0.03463c-0.18695,0.96931 -0.3681,1.99952 -0.96299,2.79126c-0.75043,1.12451 -1.95255,1.79215 -3.18987,1.94971c0.01004,0.5987 -0.27508,1.86326 0.65789,1.35879c2.32277,-0.43805 4.29787,-2.59468 4.65273,-5.20033c0.33913,-0.57921 -0.51452,-1.63694 0.48945,-1.49112c0.62041,-0.16039 0.70794,0.41533 0.34677,0.7678c-0.12097,2.34984 -1.2947,4.61239 -3.1372,5.80053c-0.91754,0.59777 -1.95945,0.97451 -3.0238,1.03802c0.01054,0.49308 -0.02813,0.99257 0.03641,1.4805c2.08672,-0.18456 4.14079,-1.18517 5.50184,-2.99025c1.21233,-1.50855 1.83663,-3.49847 1.94076,-5.49953c-0.61653,-0.65764 0.67324,-0.76037 0.98046,-0.46109c-0.34759,0.76348 -0.30822,1.77068 -0.53883,2.65294c-0.74973,3.47517 -3.45133,6.27503 -6.60887,6.90807c-0.43282,0.10271 -0.87313,0.15473 -1.31176,0.21554c-0.04197,0.59574 -0.03003,1.20163 0.68527,1.08777c1.21668,0.28323 2.38475,1.21469 2.77344,2.56598c-2.54597,0 -5.09194,0 -7.63792,0c0.01841,-0.05801 0.03682,-0.11602 0.05523,-0.17403l0,0.00001zm-4.89743,-13.65621c-0.03243,-0.53123 0.2562,-1.66432 0.33448,-1.79586c0.1342,0.48627 0.7436,1.5571 0.27939,1.87263c-0.19446,-0.02511 -0.47552,0.10315 -0.61388,-0.07676zm2.03986,0.0361c-0.10131,-0.65583 0.3224,-1.27093 0.26672,-1.94787c0.19023,0.55191 0.96684,1.92999 0.11619,1.99158c-0.12805,-0.00506 -0.2592,0.00233 -0.38291,-0.04371l0,0zm2.10487,-0.01128c-0.09006,-0.64337 0.30745,-1.25221 0.27282,-1.91585c0.23561,0.55708 1.0278,2.14147 -0.049,1.96923l-0.11298,-0.00827l-0.11084,-0.04512l0,0zm2.04478,0.0173c-0.11534,-0.65591 0.3193,-1.27551 0.26213,-1.95388c0.19324,0.55548 0.96273,1.9224 0.11757,2.00539c-0.12723,-0.00537 -0.25837,0.00141 -0.3797,-0.05151zm4.08911,-0.02506c-0.09184,-0.65974 0.31412,-1.28379 0.257,-1.96756c0.15984,0.33288 0.37352,0.89915 0.46788,1.34271c0.28204,0.72224 -0.23345,0.81958 -0.72488,0.62485l0,0zm2.10998,0.02913c-0.17099,-0.63871 0.31339,-1.28043 0.24089,-1.95795c0.20221,0.54496 0.73309,1.51617 0.38613,1.98609c-0.20893,0.00128 -0.4216,0.02732 -0.62702,-0.02814zm2.06027,-0.01351c-0.13194,-0.53244 0.25692,-1.62555 0.29978,-1.84871c0.2067,0.54932 0.9766,2.19874 -0.16506,1.89435l-0.13472,-0.04564l0,0zm2.07385,-0.01004c-0.11272,-0.58473 0.30748,-1.42628 0.28553,-1.87973c0.20505,0.52524 1.00323,2.11477 -0.05593,1.93807l-0.11173,-0.00979l-0.11786,-0.04854l0,0z' opacity='undefined' fill-opacity='null' stroke-opacity='null' stroke-dasharray='null' stroke-width='null' stroke='%23821eda' fill='none'/%3E%3C/g%3E%3C/svg%3E";
var INDI = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg'%3E%3C!----%3E%3Cg%3E%3Ctitle%3Ebackground%3C/title%3E%3Crect fill='none' id='canvas_background' height='34' width='34' y='-1' x='-1'/%3E%3C/g%3E%3Cg%3E%3Ctitle%3ELayer 1%3C/title%3E%3Cpath id='svg_1' d='m19.46141,6.85511l-8.85177,6.17499l8.74234,2.1214l-8.08047,6.62938l-3.26241,-1.11947l2.31391,6.39l9.19572,-2.78792l-3.88258,-1.16534l10.54291,-9.52194l-9.63741,-1.72579l9.30648,-6.43444l-5.30859,-0.9661l5.78185,-4.08942l-2.12401,-0.05877l-8.97229,5.61025l4.23631,0.94316l0,0z' stroke-width='0.5' stroke='%23000' fill='%23821eda'/%3E%3C/g%3E%3C/svg%3E";

function drawLabel(e, c) {
	var chart;
	if (c != undefined)
		chart = c;
	else
		chart = e.target;
	const renderer  = chart.renderer;
	const pathStr   = 'M0 7.2 L7.2 0 L38.4 0 L38.4 14.4 L7.2 14.4 Z';
//	const pathStr   = 'M0 12 L12 0 L64 0 L64 24 L12 24 Z';
	const path      = pathStr.split(' ');
	const colors    = Highcharts.getOptions().colors;
	const points    = chart.series[0].groupedData;
	if (points == undefined)
		return;
	const lastPoint = points[points.length - 1];
	!chart.labelBg  && (chart.labelBg  = renderer.path(path))
	!chart.labelTxt && (chart.labelTxt = renderer.text(lastPoint.close))
    chart.labelBg.attr ({zIndex:1,fill:colors[2]}).translate(chart.plotSizeX+12,lastPoint.plotClose+35).add()
    chart.labelTxt.attr({zIndex:2,fill:"#000000"}).translate(chart.plotSizeX+18,lastPoint.plotClose+47).css({fontWeight:'bold',fontSize:'13px'}).add()
	chart.labelTxt.textStr = lastPoint.close;
}

function pick() {var args = arguments;var length = args.length;for (var i = 0; i < length; i++) {var arg = args[i];if (typeof arg !== 'undefined' && arg !== null)return arg;}}
function getYAxisResizers(chart, yAxes) {var resizers = [];yAxes.forEach(function (_yAxis, index) {var nextYAxis = yAxes[index + 1];if (nextYAxis)resizers[index] = {enabled:true,controlledAxis:{next: [pick(nextYAxis.options.id, nextYAxis.options.index)]}};else resizers[index] = {enabled:false};});return resizers;}
function recalculateYAxisPositions(positions, changedSpace, modifyHeight, adder) {positions.forEach(function (position, index) {var prevPosition = positions[index - 1];position.top = !prevPosition ? 0 : parseFloat(prevPosition.height + prevPosition.top);if (modifyHeight)position.height = parseFloat(position.height + adder * changedSpace);});return positions;}
function isNumber(n) {return typeof n === 'number' && !isNaN(n) && n < Infinity && n > -Infinity;}
function defined(obj){return typeof obj !== 'undefined' && obj != null;}
function getYAxisPositions(yAxes, plotHeight, defaultHeight) {
	var positions,
	allAxesHeight = 0;
	function ipc(prop) {return defined(prop) && !isNumber(prop) && prop.match('%');}
	positions = yAxes.map(function (yAxis) {
		var height = ipc(yAxis.options.height) ?
		parseFloat(yAxis.options.height) / 100 :
		yAxis.height / plotHeight,
		top = ipc(yAxis.options.top) ?
		parseFloat(yAxis.options.top) / 100 :
		parseFloat(yAxis.top - yAxis.chart.plotTop) / plotHeight;
		if (!isNumber(height))
			height = defaultHeight / 100;
		allAxesHeight = parseFloat(allAxesHeight + height);
		return {height: height * 100,top:top * 100};
	});
	positions.allAxesHeight = allAxesHeight;
	return positions;
}
function ryx(chart) {
	defaultHeight = 20;
	yAxes         = chart.yAxis,
	plotHeight    = chart.plotHeight,
	allAxesLength = yAxes.length,
	positions     = getYAxisPositions(yAxes,plotHeight,defaultHeight),
	resizers      = getYAxisResizers(chart, yAxes),
	allAxesHeight = positions.allAxesHeight,
	changedSpace = defaultHeight;
	if (allAxesHeight > 1) {
		if (allAxesLength < 6) {
			positions[0].height = parseFloat(positions[0].height - changedSpace);
			positions = recalculateYAxisPositions(positions, changedSpace);
		} else {
			defaultHeight = 100 / allAxesLength;
			changedSpace = defaultHeight / (allAxesLength - 1);
			positions = recalculateYAxisPositions(positions, changedSpace, true, -1);
		}
		positions[allAxesLength - 1] = {top:parseFloat(100 - defaultHeight),height:defaultHeight};
	} else {
		changedSpace = parseFloat(1 - allAxesHeight) * 100;
		if (allAxesLength < 5) {
			positions[0].height = parseFloat(positions[0].height + changedSpace);
			positions = recalculateYAxisPositions(positions, changedSpace);
		} else {
			changedSpace /= allAxesLength;
			positions = recalculateYAxisPositions(positions, changedSpace, true, 1);
		}
	}
	positions.forEach(function (position, index) {yAxes[index].update({height:position.height + '%',top:position.top + '%',resize:resizers[index]}, false);});
}

// ancient
function Chart_addTicker(chart, ticker)
{
	$.ajax({url:"/1mOHLC/"+ticker, async:"true", complete:function(result){
		var points = JSON.parse(result.responseText),
		len        = points.length,
		ohlc       = [],
	    i = 0;
		for (i;i<len;i++)
			ohlc.push([points[i][0],points[i][1],points[i][2],points[i][3],points[i][4]]);
		config    = {
			id:ticker+'-price',
			type:'line',
			name:ticker,
			data:ohlc,
			dataGrouping:{enabled:false}
		};
		chart.addSeries(config, true, true);
	}});
}

function rename(QGID)
{
	var d, input = document.createElement("input"), ticker = window.event.target.textContent, div;
	if (QGID == "graph") {
		d = $("#graph")[0];
		div = QGID;
	} else {
		div = ticker+"-"+QGID;
		d = $("#"+div)[0];
	}
	console.log("div: " + div);
	if (d.R == 1)
		return;
	d.R = 1;
	input.value = ticker;
	input.className = "rename";
	input.select();
	$("#"+div).prepend(input);
	$("#"+div).keyup(function(event) {
    	if (event.keyCode === 13) {
			reload(input.value.toUpperCase(),div);
			input.remove();
			d.R = 0;
		}
	});
}

function reload(ticker,div)
{
	var cmd, prev = $("#"+div).highcharts().title.textStr, QGID = div.split("-")[1];
	$("#"+div + " .highcharts-title").html(ticker);
	if (QGID) {
		cmd = "qchart "+QGID+ " " + ticker + " " + prev;
		$("#"+div).attr("id", ticker + "-" + QGID);
	} else
		cmd = "chart " + ticker + " graph";
	WS.send(cmd);
	if (div == "graph")
		QGID = "P0Q0q1ws0";
	deref(prev, QGID);
}

function reloadTicker(div,r,enter)
{
	var ticker=window.event.target;
	if (!enter)
		ticker = ticker.previousSibling;
	ticker = ticker.value.toUpperCase();
	if (r)
		reload(ticker,div);
	else
		WS.send("qchart " + div + " " + ticker);
	console.log("reload ticker: " + "qchart " + div + " " + ticker);
	window.event.target.parentNode.remove();
	div = div.replace(/[^0-9]/g, '').split('');
	W.qcache_add(div[0],div[1],div[2],div[3],ticker,['chart'],ticker);
}

function chart_table_onclick(id)
{
	$('#'+id+' tbody').on('click', 'tr', function() {
		WS.send("chart " + T[id].row(this).data().T + " graph");
		$("body").append("<div id=load class=loading></div>");
	});
}

function ChartMenu()
{
	// CID: "Chart's Global ID" eg: TSLA-P0Q0q1ws0
	var CID = window.event.target.parentNode.parentNode.parentNode.id, av = CID.split("-"), ticker = av[0], QGID = av[1];
    $.contextMenu({selector:"#ct-"+CID,trigger:'left',build:function($trigger, e) {
			return{items:{
                    "Load":  {name:"Load Ticker", callback:function(){W.QCMenu_LoadTicker(CID)}},
                    "Full":  {name:"Full Screen", callback:function(){$("#"+CID).highcharts().fullscreen.toggle()}},
		            "Preset":{name:"Preset","className":"ct-"+CID,"items":PMenu},
					"Import": {
						name: "Import",
						items:{
							"I0":{name:"Website", callback:function(){GUI_wget_url(CID,QGID,1)}},
							"I1":{name:"Excel",   callback:function(){chart_import(CID)}},
							"I2":{name:"CSV",     callback:function(){chart_import(CID)}},
							"I3":{name:"TXT",     callback:function(){chart_import(CID)}},
							"I4":{name:"HTML",    callback:function(){chart_import(CID)}},
						}
					},
					"Theme": {name:"Theme", className:"ct-"+CID, items:ChartThemesDict},
                    "Volume":{name:"Toggle Volume", callback:function(){
						var c = $("#"+CID).highcharts();
						if (c.vol) {
							c.yAxis[0].update({height:'65%'});
							c.yAxis[1].update({height:'35%',top:'65%'});
							c.vol = 0;
						} else {
							c.yAxis[0].update({height:'100%'});
							c.yAxis[1].update({height:'0%',top:'100%'});
							c.vol = 1;
						}
					}},
                    "Close":{name:"Close", callback:function(){DELETE_OBJECT({type:'chart', ticker:ticker, QGID:QGID}, 1)}
			}}};}});
}

function csv(data){return $("<pre>"+data+"</pre>")[0].innerHTML}
