/*********************************************
 * filename: mainpage/stockminer/js/help.js
 * License:  Public Domain
 ********************************************/
var vplay = 0;	
function PlayVideo(id){
	if (vplay) {
		document.getElementById('HV'+id).pause();
		vplay=0;
	} else {
		document.getElementById('HV'+id).play();
		vplay=1;
	}
}

function toghelp(page, tips)
{
	if (HELP[page]['on']) {
		$(tips).css("display", "none");
		$("#helpvid").css("display", "none");
		for (arrow in HELP[page]) {
			if (arrow == 'on')
				continue;
			HELP[page][arrow].css("display", "none");
			HELP['main']['on'] = 0;
		}
	} else {
		$(tips).css("display", "block");
		$("#helpvid").css("display", "grid");
		for (arrow in HELP[page]) {
			if (arrow=='on')
				continue;
			HELP[page][arrow].css("display", "block");
			HELP['main']['on'] = 1;
		}
	}
}
function help_mainpage()
{
	if (!HELP['main']) {
		var main = HELP['main'] = {},arrows = 0,

		/* QuadVerse Tip */
		arrow = main['qverse'] = $("#help .qhelp").clone();
		arrow.addClass("arrow-qverse");
		arrow.css("display", "block");
		$(".arrow", arrow).css("marker-end", "url(#markerArrow" + arrows + ")");
		$("marker", arrow)[0].id = "markerArrow" + arrows++;
		$("body").append(arrow);

		/* ChartSpace Tip */
		arrow = main['cspace'] = $("#help .qhelp").clone();
		arrow.addClass("arrow-cspace");
		arrow.css("display", "block");
		$(".arrow", arrow).css("marker-end", "url(#markerArrow" + arrows + ")");
		$(".arrow", arrow).attr({"x1":"-150","y1":"200","x2":"80","y2":"20"});
		$("marker", arrow)[0].id = "markerArrow" + arrows++;
		$("body").append(arrow);

		/* Indicator Tip */
		arrow = main['indi'] = $("#help .qhelp").clone();
		arrow.addClass("arrow-indi");
		arrow.css("display", "block");
		$(".arrow", arrow).css("marker-end", "url(#markerArrow" + arrows + ")");
		$(".arrow", arrow).attr({"x1":"200","y1":"200","x2":"50","y2":"15"});
		$("marker", arrow)[0].id = "markerArrow" + arrows++;
		$("body").append(arrow);

		/* Help Tip */
		arrow = main['help'] = $("#help .qhelp").clone();
		arrow.addClass("arrow-help");
		arrow.css("display", "block");
		$(".arrow", arrow).css("marker-end", "url(#markerArrow" + arrows + ")");
		$(".arrow", arrow).attr({"x1":"-150","y1":"200","x2":"80","y2":"20"});
		$("marker", arrow)[0].id = "markerArrow" + arrows++;
		$("body").append(arrow);

		$(".maintips").css("display", "block");
		HELP['main']['on'] = 1;
		$("#helpvid").css("display", "grid");
		return;
	}
	toghelp('main', '.maintips');
}

function help_screener(){}
function help_ufo(){}
function help_vfo(){}
function help_candles(){}
function help_options(){}
function help_airports(){}
function help_overview(){}
function help_overview(){}
function help_boarding(){}
function help_flights(){}
function help_fleet(){}
function help_saftey(){}

var help_mainpages = [ help_mainpage, help_screener, help_ufo, help_vfo, help_candles ];
var help_opages    = [ help_options ];
var help_airpages  = [ help_airports, help_overview, help_boarding, help_flights, help_fleet, help_saftey ];
function help()
{
	var page = W.current_quadspace.replace(/[^0-9]/g, '').split(''), p = page[1];
	switch (page[0]) {
		case '0': help_mainpages[p](); break;
		case '1': help_opages[p]();    break;
		case '5': help_airpages[p]();  break;
	}
}
