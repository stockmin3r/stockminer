var   checkpoint_timer;
const CHECKPOINT_EMPTY    = 0;
const CHECKPOINT_PARTIAL  = 1;
const CHECKPOINT_COMPLETE = 2;
const CHECKPOINT_REBOOT   = 3;
var   checkpoint_init     = 0;

function checkpoint_poll()
{
	WS.send("checkpoint");
}

function checkpoint_progress(progress)
{
	console.log("setting checkpoint progress: " + progress + "%");
	$("#animated-progress").animate({width:progress+"%"}, 1000);
	$("#installer-percent").text(progress+"%");
}

function rpc_checkpoint(av)
{
	if (!checkpoint_init) {
		var id   = $("#installer-div")[0].parentNode.id, q_idx = id.indexOf("q")+1, ws_idx = id.indexOf("ws");	
		var QID  = id.substr(q_idx, ws_idx-q_idx);
		var WSID = id.substr(id.indexOf("ws"), ws_idx);

		Q[0].quadspace[0].quad[QID].showtab(Q[0].quadspace[0].quad[QID].workspace[WSID].tab);
		checkpoint_init = 1;
	}
	if (av[1] == CHECKPOINT_EMPTY || av[1] == CHECKPOINT_PARTIAL)
		$("#installer").css("display", "block");

	if (av[1] == CHECKPOINT_REBOOT)
		WS.send("boot " + RPC_BOOT);

	$("#checkpoint").val(av[2]);
	checkpoint_progress(av[2]);
	if (av[1] == 100.0) {
		clearInterval(checkpoint_timer);
		return;
	}
	if (!checkpoint_timer)
		checkpoint_timer = setInterval(checkpoint_poll, 3000);

}
