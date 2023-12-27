function backtest_chart()
{
	$.ajax({url:"/BACK/ESP", async:"false", complete: function(result){
		var series_data = result.responseText.split(" ");
	    Highcharts.stockChart('backchart', {
        title: {
            text: 'Stockminer Top 10 Performance vs Indexes',
			color:"#821eda"
		},
		chart:{
		backgroundColor: {
	       linearGradient: [0, 0, 500, 500],
	       stops: [
	         [0, 'rgb(25, 28, 32)'],
	         [1, 'rgb(13, 13, 13)']
	       	]
			}
		},
		navigator:{
			enabled:false
		},
		rangeSelector:{
			enabled:false
		},
		mapNavigation:{
			enabled:false,
			enableMouseWheelZoom:true
		},
		exporting:{
			enabled:false
		},
		legend: {
			enabled:true,
			align:'right',
			verticalAlign:'top',
			itemStyle:{
				cursor:'pointer',
				color:'white',
				fontWeight:'bold',
				fontSize:'12px'
			}
		},
		xAxis:{
			ordinal:true
		},
        series: [{
            name: 'GSPC',
            data: JSON.parse(series_data[0]),
            tooltip: {
                valueDecimals: 2
            },
			color:"green"
        },{
            name: 'IXIC',
            data: JSON.parse(series_data[1]),
            tooltip: {
                valueDecimals: 2
            },
			color:"white"
		},{
            name: 'RUT',
            data: JSON.parse(series_data[2]),
			color:"red"
		},{
            name: 'DJI',
            data: JSON.parse(series_data[3]),
			color:"dodgerblue"
		},{
            name: 'SB-10',
            data: JSON.parse(series_data[4]),
			color:"#821eda"
		}]
    });
}});
}


	/* ***********************************
	 *
	 *              BACKTEST
	 *
	 ************************************/
function line_chart(data, div, color){
	Highcharts.stockChart(div, {
		chart:         {backgroundColor:color},
		navigator:     {enabled:false},
		rangeSelector: {enabled:false},
		mapNavigation: {enabled:false,},
		exporting:     {enabled:false},
		yAxis:         {visible:false},
		xAxis:         {visible:false,ordinal:true},
        series:       [{threshold:10000,data: data,color:"green"}]
    });
}
function pie_chart(data, div){
	Highcharts.chart(div, {
		title: {text:''},
		chart: {
			backgroundColor:"white",
			plotBackgroundColor:null,
			plotBorderWidth:null,
			plotShadow:false,
			type:'pie'
		},
		tooltip:{pointFormat:'<b>{point.percentage:.1f}%</b>'},
		accessibility:{point:{valueSuffix:'%'}},
		exporting:{enabled:false},
 		plotOptions: {
 			pie: {
 				allowPointSelect:true,
 				cursor:'pointer',
				dataLabels:{enabled: false},
				showInLegend:true
 			}
 		},
		series: [{innerSize: '40%',data:data}]
	});
}

function backtest() {
	WS.send("portfolios *");
}

function rpc_portfolio(av)
{
	console.log("portflios: " + av[1]);
	var portlist = av[1].split("$");
	var port     = portlist[0];
	var segment  = port.split("@");
	var port_id  = segment[0];
	var div      = segment[1];
	var line     = segment[2];
	document.getElementById("port-list").innerHTML = div;
	line_chart(JSON.parse(line), port_id+"-chart", "white");
}

function backtest_opt(){
	if (backtest_box) {
		document.getElementById("backtest-box").style.display="none";
		document.getElementById("backtest-list").style.display="none";
		backtest_box=0;
	} else {
		document.getElementById("backtest-box").style.display="block";
		document.getElementById("backtest-list").style.display="block";
		backtest_box=1;
	}
}

function backtest_load(){
	var id = window.event.target.parentNode.id;
	id = id.split("_")[0],
	self = this;
	$.ajax({url:"/BACK/"+port_id+"/"+id, async:"true", complete: function(result){
		var file    = result.responseText.split("@");
		var sum     = file[0].split(" ");
		var gain    = sum[0];
		var val     = sum[1];
		var line    = JSON.parse(file[1]);
		var itable  = file[2];
		var htable  = file[3];
		var pie     = file[4];
		var options = file[5];
		var blist   = file[6];
		self.line_chart(line, "ultra-linechart", "white");
		$("#ultra-itable").append(itable);
		$("#ultra-htable").append(htable);
		document.getElementById("ultra-gain").innerHTML = gain;
		if (pie === "none")
			return;
		self.pie_chart(JSON.parse(pie), "ultra-piechart");
		if (blist != null && blist != undefined)
			document.getElementById("backtest-list").innerHTML=blist;
	}});
}

/* Clicking on a Portfolio */
function port_load(){
	var element = window.event.target,
	self = this;
	while (1) {
		if (element.classList.contains("portfolio")) {
			port_id = element.id;
			break;
		}
		element = element.parentNode;
	}
	$.ajax({url:"/PORT/"+port_id, async:"true", complete: function(result){
		document.getElementById("port-list").style.display="none";
		document.getElementById("ultra-main").style.display="block";
		var file    = result.responseText.split("@");
		var sum     = file[0].split(" ");
		var gain    = sum[0];
		var val     = sum[1];
		var line    = JSON.parse(file[1]);
		var itable  = file[2];
		var htable  = file[3];
		var pie     = file[4];
		var options = file[5];
		var blist   = file[6];
		self.line_chart(line, "ultra-linechart", "white");
		$("#ultra-itable").append(itable);
		$("#ultra-htable").append(htable);
		document.getElementById("ultra-gain").innerHTML = gain;
		if (pie === "none")
			return;
		self.pie_chart(JSON.parse(pie), "ultra-piechart");
		if (blist != null && blist != undefined)
			document.getElementById("backtest-list").innerHTML=blist;
		options = JSON.parse(options);
	}});
}

function RUN_BACKTEST(av){
	var file    = result.responseText.split("@"),
	sum         = file[0].split(" "),
	back_gain   = sum[0],
	val         = sum[1],
	backtest_id = sum[2],
	line        = JSON.parse(file[1]),
	itable      = file[2],
	htable      = file[3],
	pie         = file[4],
	options     = file[5],
	blist       = file[6];
	$("#ultra-itable").empty();
	$("#ultra-htable").empty();
	$("#ultra-itable").append(itable);
	$("#ultra-htable").append(htable);
	document.getElementById("ultra-gain").innerHTML = back_gain;
	this.line_chart(line, "ultra-linechart", "white");
	if (pie === "none")
		return;
	$("#ultra-piechart").empty();
	this.pie_chart(JSON.parse(pie), "ultra-piechart");
	if (blist != null && blist != undefined)
		document.getElementById("backtest-list").innerHTML=blist;
}

function run_backtest(){
	var capital = document.getElementById("ultra-capital").value;
	var avgdays = document.getElementById("backbox-avgdays").value;
	var pt      = document.getElementById("backbox-pt").value;
	var maxpos  = document.getElementById("backbox-maxpos").value;
	var a1q1    = document.getElementById("backbox-a1q1").value;
	var a4q1    = document.getElementById("backbox-a4q1").value;
	var peak    = document.getElementById("backbox-peak").value;
	var rank    = document.getElementById("backbox-rank").value;
	var capital_select = document.getElementById("backbox-select-capital").options.selectedIndex;
	var avgdays_select = document.getElementById("backbox-select-avgdays").options.selectedIndex;
	var pt_select      = document.getElementById("backbox-select-pt").options.selectedIndex;
	var maxpos_select  = document.getElementById("backbox-select-maxpos").options.selectedIndex;
	var a1q1_select    = document.getElementById("backbox-select-a1q1").options.selectedIndex;
	var a4q1_select    = document.getElementById("backbox-select-a4q1").options.selectedIndex;
	WS.send("backtest_run " + port_id + " " + PORT_MONTH +" " + capital + " " +avgdays + " " + pt + " " +maxpos+ " " +a1q1+ " " +a4q1+ " "
	+ capital_select + " " + avgdays_select + " " + pt_select + " " + maxpos_select + " " + a1q1_select + " " + a4q1_select
	+ " " + peak + " " + rank);
}

function backsave(){
	var name = document.getElementById("backtest-name").value;
	var pub  = document.getElementById("backpub-check").checked;
	if (pub == true)
		pub = '1';
	else
		pub = '0';
	save_box=0;

	WS.send("SAVE "+port_id+" "+pub+" "+name);
	document.getElementById("backtest-save").style.display="none";
	/* add backtest to backtest list div */
	var table   = document.getElementById("backsave-table");
	var row     = table.insertRow(table.rows.length);
	var cell    = row.insertCell(0);
	row.onclick = backtest_load;
	row.id      = backtest_id;
	cell.innerHTML = name;
	cell        = row.insertCell(1);
	if (back_gain.indexOf('+') != -1)
		cell.classList.add("green2");
	else
		cell.classList.add("red2");
	cell.innerHTML += back_gain;
	cell            = row.insertCell(2);
	cell.innerHTML += "5.0";
}
function backtest_save(){
	var bs = document.getElementById("backtest-save");
	if (save_box) {
		bs.style.display="none";
		save_box=0;
	} else {
		bs.style.display="block";
		save_box=1;
	}
}
function port_month(month){
	PORT_MONTH=month;
}
function ultra_pie(){
	document.getElementById("ultra-pie-section").style.display="block";
	document.getElementById("ultra-backtest-section").style.display="none";
}
function ultra_backtest(){
	document.getElementById("ultra-backtest-section").style.display="block";
	document.getElementById("ultra-pie-section").style.display="none";
}
