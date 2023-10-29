/************************************************
 * filename: backpage/monster/js/rpc.js
 * License:  Public Domain
 ***********************************************/

/* SCP: Server -> Client Procedure Call */
var monster_rpc = { 'peakwatch':rpc_peakwatch,'fork':       rpc_fork,        'qf':   rpc_qf,   'forkscatter':rpc_fork_scatter,
                    'sigstat':  rpc_sigstat,  'portscatter':rpc_port_scatter,'ports':rpc_ports,'scatter':    rpc_scatter};	

function rpc_scatter(av)      {stock_scatter(av)}
function rpc_fork_scatter(av) {fork_scatter(av)}
function rpc_port_scatter(av) {port_scatter(av)}
function rpc_ports(av)        {port_table(av)}

function rpc_sigstat(av)
{
	var sigs = av.slice(1).join(" ").split("*"),sig,id,ws;
	for (var x = 0; x<sigs.length; x++) {
		av  = sigs[x].split(" ");
		sig = $("body > .sigflight").clone();
		$(".sf-ticker", sig).html(av[0]);
		$(".sf-date", sig).html(av[1]);   // entry_price
		$(".sf-date2", sig).html(av[2]);  // exit_price
		$(".sf-price", sig).html(av[3]);
		$(".sf-status", sig).html(av[4]);
		ws = W.quadspace[0].quad[0].current_workspace;
		switch (ws) {
			case 4:
				id = "#fork-passes";
				break;
			case 2:
				id = W.current_quadspace + "q0ws1";
				break;
		}
		if (!x)
			$(W.current_quadspace + "q0ws" + W.quadspace[0].quad[0].current_workspace + " #fork-passes").empty();
		$(id).append(sig);
		sig.css("display", "block");
	}
}

function rpc_qf(av, cache) {
	if (!cache)
		ctable(av);
}

function rpc_peakwatch(av)
{
	ctable(av);
	var tr = $("#PW tbody tr");
	for (var x = 0; x<tr.length; x++)
		PWT[x] = tr[x];
}

function rpc_fork(av)
{
	$("#forktab tbody").empty();
	$("#forktab tbody").append(av[1]);
	localStorage['forktab'] = JSON.stringify({age:new Date().toLocaleString('en-US', { timeZone: 'America/New_York' }),tab:av[1]});
}
var ARGV;
function rpc_monster_tables()
{
	$.ajax({url:"/gztab", async:"true", complete: function(r){
		var cmd = r.responseText.split("@");
		for (var x = 0; x<cmd.length; x++) {
			var argv = cmd[x].split(" ");
			op[argv[0]](argv);
		}
	}});
}
