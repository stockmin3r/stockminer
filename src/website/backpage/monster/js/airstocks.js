/************************************************
 * filename: mainpage/stockminer/js/airstocks.js
 * License:  Public Domain
 ***********************************************/
var FDT = [ 'hyper', 'super', 'biz', 'eco' ];
var fdt_select = 0;
var FINIT = 0;

function fdt_tables()
{
	for (var x = 0; x < 5; x++) {
		var name = 'FDT-'+FDT[x];
		T[name] = $("#"+name).DataTable({
			"columns": [{"data":"Rank"},{"data":"Sym"},{"data":"ENDate"},{"data":"ENPrice"},{"data":"EXDate"},{"data":"EXPrice"},{"data":"RET"}],
			"columnDefs": [{"targets":7,"visible":false},{ type: 'natural', "targets": [0]}],
			"createdRow": function(row,data,index){tableColors(row,data,index,"#PW thead tr",1)},
			"bPaginate":false,"order": [[0, "asc" ]],"paging":false,"bDestroy":false,"info":false,"searching":false});
	}
}


var AIRSTAT = {1:"Landed",2:"Takeoff",3:"Climb",4:"Cruise",5:"Approach"};
function airstat()
{
	var td, tr = $("#FDT tbody tr");
	for (var x = 0; x<tr.length; x++) {
		td = tr[x].cells[7];
		td.innerText = AIRSTAT[td.innerText];
	}
}

/* fleet workspace onclick */
function fleet_click(){
	if (!FLEET) {
		WS.send("fork " + $("#PORTS tbody td:first-child")[0].innerText)
		FLEET = 1;
	}
}

function stockplane(f)
{
	fdt_select = f;
	if (!FINIT) {
		FINIT = 1;
		$.fn.dataTableExt.afnFiltering.push(function(o, rows, row) {
			if (!o.aoData || !o.aoData[row].anCells)
				return true;
			if (o.aoData[row].anCells[8].innerText != fdt_select)
				return false;
			else
				return true;
		});
	}
	$("#FDT").dataTable().fnDraw();
}

function scatter_chart(div,title,series)
{
   Highcharts.stockChart(div, {
        title:{text:title},
		credits:{text:''},
		panning:true,
		zoomType:"xy",
		chart:{
			backgroundColor:{linearGradient:[0,0,500,500],stops:[[0,'rgb(25, 28, 32)'],[1,'rgb(13, 13, 13)']]}
		},
		plotOptions:   {series:[{states:{inactive:{opacity:1}}}]},
		navigator:     {enabled:false},
		rangeSelector: {enabled:false},
		mapNavigation: {enabled:false,enableMouseWheelZoom:true},
		exporting:     {enabled:false},
    	xAxis:         {TickInterval:1,labels:{enabled:false}},
    	yAxis:         {gridLineDashStyle:'Solid',gridLineColor:'rgba(254,255,255,0.30)'},
		legend: {
			enabled:true,
			align:'right',
			verticalAlign:'top',
			itemStyle:{cursor:'pointer',color:'white',fontWeight:'bold',fontSize:'12px'}
		},
    	series:series
	});
}

function portclick_scatter(e)
{
	WS.send("fork " + e.point.options.x);
}

/* plane icon onclick - load port table + port scatter */
function plane_portclick(model)
{
	var p, tabname = 'PORTS'+model, scname = 'portscatter'+model;
	if (model==0)
		tabname = 'PORTS';
	if (p=localStorage[tabname]) {
		console.log("tabname: "+ tabname);
		table([0,"PORTS",p]);
		console.log("loading cached: tabname: " + tabname + " scname: " + scname);
		port_scatter([0,model,localStorage[scname]]);
	} else
		WS.send("ports " + model); // server returns with a call to port_table()
}


/* server sends ports table */
function port_table(av)
{
	var tabname = 'PORTS'+av[1];
	table([0, "PORTS",av[2]]);
	console.log("port table: " + tabname);
	try {
		localStorage[tabname] = av[2];
	} catch (e){return}
}

function port_scatter(av)
{
	var series = [];
	series[0] = {
		turboThreshold:0,
		cursor:'pointer',
		events:{click:function(e){portclick_scatter(e)}},
    	type: 'scatter',
    	name: 'Portfolio',
    	data: JSON.parse(av[2]),
		color:"rgb(30, 144, 255, 1)",
    	marker: {symbol:"circle",fillColor:"dodgerblue",radius:4}
	};
	scatter_chart("port-scatter", "Portfolios/Capital", series);
	try {
		localStorage['portscatter'+av[1]] = av[2];
	} catch (e){return}
}

function fork_scatter(av)
{
	var series = [];

	series[0] = {
		turboThreshold:0,
    	type: 'scatter',
    	name: 'Positions/Capital',
    	data: JSON.parse(av[1]),
		color:"rgb(30, 144, 255, 1)",
    	marker: {symbol:"circle",fillColor:"dodgerblue",radius:4}
	};
	scatter_chart("fork-scatter", "Positions/Capital", series);
}
