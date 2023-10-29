/**********************************************
 * filename: mainpage/stockminer/js/QBuilder.js
 * License:  Public Domain
 * -incomplete, out of date, ancient, died
 *********************************************/
var ChartSpace = {
	QuadVerse: [{
		"title": "ChartSpace",
		"paid":  "",
		"class": " cspace",
		"QuadSpace": [{
			"title": "Config",
			"grid":  "grid100",
			"quads": [{
				"workspace":  [{"mod": "QBuilder_addConf"}]
			}]
		},{
			"title": "Tech",
			"grid":  "grid100",
			"click": "quadspace_load",
			"quads": [{
				"workspace":  [{"mod": "empty",  "title": "Growth", "grid":"grid50", "obj":[{"chart":"AAPL"},{"chart":"AMZN"},{"chart":"META"},{"chart":""}]}]
			}]
		}]
	}]
};

function CreateQuadverse()
{
	var quadverse, quadspace, qsp_div, ws, workspace, QSID_old, QSID_new, QVID = Q.length;

	QVID            = Q.length;
	QSID_old        = W.current_quadspace;
	quadspace       = W.quadspace[W.current_workspace];
	QSID_new        = "P"+QVID+"Q0";
	quadspace[0].id = QSID_new;
	console.log("QSID_old: " + QSID_old + " QSID_new: " + QSID_new);
	for (var x = 0; x<quadspace.quad.length; x++) {
		quad      = quadspace.quad[x];
		quad.qdiv = "#"+QSID_new + "q"+x;            /* New QID_str qdiv ptr (same as below) */
		$(QSID_old+"q"+x)[0].id = QSID_new + "q"+x;  /* New QID_str div id */
		quad.PID  = QVID;                            /* New QuadVerse ID */
		console.log("quad[" + x + "] qdiv: " + quad.qdiv);
		for (var y=0; y<quad.nr_ws; y++) {
			ws           = 'ws'+y;
			console.log("ws" + y + " nr_ws: " + quad.nr_ws);
			workspace    = quad[ws];
			workspace.id = QSID_new + "q"+x+ws;
			WMAP[workspace.id] = workspace;
		}
	}
	quadverse = new_quadverse("myQuads", "");
	quadverse.quadspace[0] = quadspace;
	W = quadverse;
	QVID = "#quadverse"+QVID;
	$(QVID).append($("#"+QSID_new).detach());
//	$(QVID).append($(QVID + " .chrome-tab:nth-child(" + Q.length + ")")
	$(".quadverse").css("display", "none");
	$(QVID).css("display", "block");
}


function QReload()
{
	var QB = document.querySelectorAll(".QB-QSP");
	for (var x=0; x<QB.length; x++) {
		var qname = QB.querySelectorAll(".qname");
	}
}

function QBuilder()
{
	var csp = WEBSITE.QuadVerse[3], QSEL = "<select id=QB-select onchange=QBuilder_onchange>";

	$("#P3Q0q0ws0").append("<div id=QB><div id=reload onclick=QReload()></div></div>");
	for (var x = 0; x<csp.QuadSpace.length; x++) {
		var qsp = csp.QuadSpace[x], quads = qsp.quads, QSP;
		if (!qsp.build)
			continue;
		QSP  = $('<div class="QB-QSP ' + qsp.grid +  '" id=Q'+x+"></div>");
		$("#QB").append(QSP);
		QSEL += "<option value=Q"+x+">"+qsp.title+"</option>";
		for (var y = 0; y < quads.length; y++) {
			var ws_obj = quads[y].workspace, quad;

			$(QSP).append('<div class="QB-Quad q' + y + '"><span class=qname>Quad'  + y + '</span></div>');
			quad = $(".q"+y, QSP);
			for (var z = 0; z<ws_obj.length; z++) {
				var obj   = ws_obj[z].obj,
				wsid      = "P3Q"+x+"q"+y+"ws"+z;
				workspace = $('<div class="'+wsid+ " " + ws_obj[z].grid + '"></div>');
				quad.append(workspace);

				for (var o = 0; o<obj.length; o++) {
					if (obj[o].chart) {
						workspace.append('<div class="QB-obj o' + o + '"><input type=text value='+obj[o].chart+"></div>");
						$(".o"+o,workspace).append($(".bchart img")[0].cloneNode());
					}
				}
			}
		}
		$("#QB").prepend(QSEL);
	}
}

function QBuilder_onchange()
{
	var name = $("#QB-select option:selected").val();
	console.log("onchnage name: " + name);
	$(".QB-QSP").css("display", "none");
	$("#"+name).css("display", "block");
}

