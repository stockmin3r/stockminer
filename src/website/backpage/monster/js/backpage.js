/**********************************************
 * filename: backpage/monster/backpage.js
 * License:  Public Domain
 *********************************************/
var BACKPAGE_monster = @BACKPAGE_monster;

var SB150_table,SB1K_table,ESP_table,MSR_table,MSR2_table; /* Tables */ 
var ESP,MSR,MSR2,MSR_MENU,SB,SB1K;                         /* pages  */ 
var SB1K_INIT, FINIT = 0; /* init */
var PWT = [];             /* PeakWatch/Radar2 rows */

function monster()     {
//	quadspace_load(W.PID,0,WEBSITE.QuadVerse[W.PID].QuadSpace[0].quads);
//	quadspace_load(W.PID,1,WEBSITE.QuadVerse[W.PID].QuadSpace[1].quads);
}

function init_monster()
{
	register_rpc(monster_rpc);
	init_monster_tables();
	WINIT=0;

	// horror built on bones of horror
	LoadQuadverse(BACKPAGE_monster, document.location.pathname.length>1?(Q.length-2):0);
	if (localStorage.age) {
		var d1   = localStorage.age.split("/"),
		d2       = new Date(), d1_hours, d2_hours, day = d2.getDay();
		d2       = d2.toLocaleString('en-US', { timeZone: 'America/New_York',hour12:false }).split("/");
		d2_hours = d2[2].split(" ")[1].split(":");
		d1_hours = d1[2].split(" ")[1].split(":");
		day      = new Date(d2).getDay();
		console.log(d2 + " " + " d1_hours: " + d1_hours + " d2_hours: " + d2_hours + " day: " + day);
		if ((day == 6 || day == 0) || (d1[1] == d2[1] && d1_hours[0] < 16 && d2_hours[0] < 16)) {
			console.log("getting cached table: " + (new Date().getHours()) + " " + d1);
			cacheload('PW');
			cacheload('QL');
			cacheload('FDT');
			airstat();
			cacheload('QF');
			qf(0,1);
			plane_portclick(0);
			$("#forktab tbody").append(JSON.parse(localStorage['forktab']).tab);
		} else {
			console.log("cache expired: getting new table " + " day: " + day + " d1: " + d1 + " d2: " + d2 + " d1_hours: " + d1_hours + " d2_hours: " + d2_hours );
			rpc_monster_tables();
		}
	} else {
		console.log("no cache");
		rpc_monster_tables();
	}

	$('#PORTS tbody').on('click', 'tr', function() {WS.send("fork " + T['PORTS'].row(this).data().PID)});
	$('#PW tbody').on('click', 'tr', function() {
		WS.send("sigstat " + T['PW'].row(this).data()['900']);
	});
	boarding();
	$(".port-month > ul > li").click(function(){$(".port-month > ul > li").removeClass('mtick');$(this).addClass('mtick')});
}
