/**********************************************
 * filename: mainpage/stockminer/js/modules.js
 * License:  Public Domain
 *********************************************/
var modules   = ['bsChart', 'addWatchtable', 'addXLS', 'screener_workspace', 'showtime_workspace'],
styleTags     = ['wsnew', 'max', 'link', 'wspanel'],
/* Display Object HTML */
styleObj      = {'wsnew':    '<div class="wsnew-edit obj">❖</div>',
                 'max':      '<div class="max-edit obj">❐</div>',
                 'link':     '<span class="link-edit obj">⚙</span>',
		 'wspanel':  '<div class="wspanel-edit obj"></div>'},
/* Object Names */
objName       = {'wsnew':   'New Workspace', 'max': 'Maximize Quad', 'link': 'Link',
		 'wspanel': 'Tab Panel Background'},
/* Objects */
wsnew         = {'color':"#5f679f", 'font-size':'24px', 'icon':'❖'},
wsmax         = {'color':"#5f679F", 'font-size':'24px', 'icon':'❐'},
wspanel       = {'background-color':"#292929"},
wslink        = {'icon':'⚙'},
defaultStyle  = {'.wsnew':wsnew, '.max':wsmax,'.wspanel':wspanel},
STYLES        = {'default':defaultStyle},
CURSTYLE      = 'default';

function ModLoad(QSID, QID, WSID, mod){W.quadspace[QSID].quad[QID][mod]({QSID:QSID, QID:QID, WSID:WSID})}

/****************
 *
 * STYLER MODULE
 *
 ***************/
function RGB2HEX(rgb) {var s=rgb.match(/^rgb\((\d+),\s*(\d+),\s*(\d+)\)$/);if(!s)return"";else return "#"+s.slice(1).map(n => parseInt(n, 10).toString(16).padStart(2, '0')).join('')}
function randstr      (length = 15) {return [...Array(length + 10)].map((value) => (Math.random() * 1000000).toString(36).replace('.', '')).join('').substring(0, length)}
function IDS(s)       {return s.replace(/[^0-9]/g, '').split('')}
function ID(id)       {return document.getElementById(id)}

function highcaps()   {if(!HCINIT)WS.send("highcaps")}

/* Remove the BlankSpace module HTML elements */
function bshide(QGID) {$(QGID + " .bspace").css("display","none");$(QGID).removeClass("gridB")}

var XLS_CALLBACKS = { 'reload': XLS_reload };


/*
var modules = [ { name: "stocks" stocks_load_blankspace} ];

function module_hook(hook,args)
{
	switch (hook) {
		case MODULE_LOAD_BLANKSPACE:
			for (module in modules)
				module->hook(args);
	}

}*/

function html_new_style(style)
{
	var new_style = document.createElement("style");
	new_style.innerHTML = style;
	$("body").append(new_style);
}

/*
 * XLS:        OBJ_ID is a Preset Name               (ref points to upper watchtable-box/WBOX container) 
 * Watchtable: OBJ_ID is the ID of the inner <table> (ref points to upper watchtable-box/WBOX container)
 * watchtools: OBJ_ID is "watchtools"
 */
function OBJ_MATCH(obj, match)
{
	var keys = Object.keys(match), val = Object.values(match);
	for (var x = 0; x<keys.length; x++) {
		console.log("keys[x]: " + keys[x] + " " + obj[keys[x]] + " vs: " + val[x]);
		if (obj[keys[x]] != val[x])
			return 0;
	}
	return 1;
}

function MODCALL(type, func, quads, obj_match, obj_set){
	var any = 0;
	if (obj_match[0] == 'any')
		any = 1;
	for (var x = 0; x<quads.length; x++) {
		var obj = quads[x].workspace['ws'+quads[x].current_workspace].obj;
		for (var y = 0; y < obj.length; y++) {
			if (obj[y].type == type && (any || OBJ_MATCH(obj[y], obj_match[1]))) {
				switch (type) {
					case "XLS":
						XLS_CALLBACKS[func](obj[y]);
						break;
				}
				if (obj_set)
					Object.assign(obj[y], obj_set);
				if (any)
					continue;
				else
					break;
			}
		}
	}
}

function get_QSP_OBJ(obj_match, type, quadspace)
{
	var found_objects = [], obj, any = 0;
	if (obj_match[0] == 'any')
		any = 1;
	for (var x = 0; x<quadspace.length; x++) {
		var workspace = quadspace[x].workspace['ws'+quadspace[x].current_workspace];
		if (!workspace)
			continue;
		obj = workspace.obj;
		if (!obj || obj.length == 0)
			continue;
		for (var y = 0; y < obj.length; y++)
			if (obj[y].type == type && (any || OBJ_MATCH(obj[y], obj_match[1])))
				found_objects.push(obj[y].ref);
	}
	return found_objects;
}

function get_QUAD_OBJ(obj_match, type, quad)
{
	var any = 0;
	if (obj_match[0] == 'any')
		any = 1;
	for (var x = 0; x<quad.workspace.length; x++) {
		var obj = quad.workspace['ws'+quad.current_workspace].obj;
		if (!obj || obj.length == 0)
			continue;
		for (var y = 0; y < obj.length; y++)
			if (obj[y].type == type && (any || OBJ_MATCH(obj[y], obj_match[1])))
				return obj[y].ref;
	}
	return 0;
}


function get_WS_OBJ(type, obj)
{
	for (var x = 0; x<obj.length; x++)
		if (obj[x].type == type)
			return obj[x].ref;
	return 0;
}

/* ****************
 *
 * SHOWTIME MODULE
 *
 *****************/
var WebStage = {'stocks':stock_stage};

function stock_stage(ticker, QGID)
{
	WS.send("chart " + ticker + " stocklist-" + QGID);
}
function rpc_stageload(av)
{
	var stocks = "", stage = av[1], watchlist = av[2], QGID = av[3], stocklist = av[4].split("-");
	for (var x = 0; x<stocklist.length; x++)
		stocks += "<li>"+stocklist[x]+"</li>";
	console.log("RPC_stageload: " + stocks);
	watchlist = Watchlist(watchlist);
	if (watchlist)
		watchlist.stocks = stocklist;

	$(QGID + " .stocklist").append(stocks);
	$(QGID + " .stocklist li").click(function(){
		WebStage[stage](this.textContent, QGID.substr(1));
		console.log("loading chart: " + this.textContent);
		$("li", this.parentNode.parentNode).removeClass("boxtick");
		this.className = 'boxtick';
	});
}

// [ChromeTAB] reload watchlists and stocks but do not initiate showtime play
function showtime_stocks_onclick()
{
	var n = window.event.target.parentNode.parentNode, WSID = n.classList[1], watchlist,stocks, select,
	QGID  = "#" + n.parentNode.parentNode.parentNode.id + WSID;
	console.log("stage_screener " + QGID + " " + WSID);
	select = $(".showtime-watchlists-select", QGID);
	html_select_append(select, WL, 'name');
	html_select_append(select, GW, 'name');
	watchlist = WL[0];
	stocks = watchlist.stocks;
	if (stocks) {
		console.log("HAVE STOCKS LOADING");
		RPC_stageload([QGID, watchlist.name, 'stocks', stocks]);
	} else {
		console.log("NO STOCKS RPC");
		WS.send("stage stocks " + watchlist.name + " " + QGID)
	}
}

// [SVG PLAY BUTTON]
function Showtime_Play()
{
	var screener = window.event.target.parentNode.parentNode.parentNode,
	QGID         = screener.parentNode.id, x = 1, stocks;

	if (screener.playing) {
		screener.stop = 1;
		$(".pause", screener)[0].className = "start";
		return;
	}

	stocks = document.querySelectorAll("#"+QGID + " .showtime .wsrc ul li");
	WS.send("chart " + stocks[0].textContent + " stocklist-" + QGID);
	screener.delay = $("#"+QGID + " .showtime-delay-input").val() * 1000;
	screener.playing = 1;
	$(".start", screener)[0].className = "pause";
	console.log("delay: " + screener.delay);
	screener.showtime  = setInterval(function() {
		if (screener.stop) {
			$(".pause", screener)[0].className = "start";
			screener.playing = 0;
			return;
		}
		var ticker = stocks[x++].textContent, obj = {ticker:ticker};
		WS.send("chart " + ticker + " stocklist-" + QGID);
		MODCALL("XLS", 'reload', $("#"+QGID)[0].w.quadspace, ['any', obj], obj);
	}, screener.delay);
}

// [SVG STOP BUTTON]
function Showtime_Stop()
{
	var screener = window.event.target.parentNode.parentNode.parentNode;
	screener.stop = 1;
	screener.playing = 0;
	$(".pause", screener)[0].className = "start";
	clearInterval(screener.showtime);
}

/* Watchtable: Link with Screener
 *  - if previously the icons had the same color and changes from one screener to another or to none, it must be removed from those screener's curobj lists
 */
function link(new_color)
{
	var obj_id = $(window.event.target).closest(".context-menu-root")[0].classList[1].split("-")[1],
	obj        = $("#"+obj_id)[0],
	QGID       = obj.parentNode.parentNode.parentNode.id,
	screeners  = get_QSP_OBJ(["any"], "screener", $("#"+QGID)[0].w.quadspace),
	prev_color = $(".tlink", obj)[0].classList[1].split("-")[1], found = 0;

	if (new_color == prev_color)
		return;

	obj.link = new_color;
	$(".tlink", obj)[0].classList.replace("link-"+prev_color, "link-"+new_color);
	obj = obj.parentNode.parentNode;
	if (!screeners.length)
		return;
	for (var x = 0; x<screeners.length; x++) {
		var screener = screeners[x];
		console.log("screener color: " +  screener.link + " prev: " + prev_color);
		if (screener.link == new_color) {
			console.log("found screener with color: " + new_color);
			for (var y = 0; y<screener.curobj.length; y++) {
				if (screener.curobj[y] == obj) {
					found = 1;
					console.log("found screener to link with");
					$(".tlink", obj)[0].classList.replace("link-"+prev_color, "link-"+new_color);
					break;
				}
			}
			if (!found) {
				screener.curobj.push(obj);
				obj.obj.screener = screener;
			}
		// remove watchtable from screener
		} else if (screener.link == prev_color) {
			for (var y = 0; y<screener.curobj.length; y++) {
				if (screener.curobj[y] == obj) {
					console.log("removing watchtable from screener");
					screener.curobj.splice(y, 1);
					break;
				}
			}
		}
	}
}

/* Screener wheel onclick: set color to signify a link between the screener and Watchtable(s)
 *  - unlink all Watchtables previously linked with this screener which are not of the same color
 *  - link all Watchtables previously unlinked with this screener which now share the same color with the screener
 */
function screener_setlink(new_color)
{
	var screener = window.event.target.parentNode.parentNode.parentNode.screener, QGID = screener.QGID, prev_color = screener.link,

	// Get ALL watchtables in this QuadSpace since we are changing colors
	tables  = get_QSP_OBJ(['any'], 'wstab', $(QGID)[0].w.quadspace);

	$(".links-menu", screener[0]).css("display", "none");
	screener.link = new_color;
	$(".links", screener)[0].classList.replace("link-"+prev_color, "link-"+new_color);
	if (!tables.length)
		return;

	for (var x = 0; x<tables.length; x++) {
		var tab = tables[x];
		/* if this watchtable has the same color as our screener's previous color, unlink it from the screener.curobj list [] */
		if (tab.link == prev_color) {
			/* unlink */
			screener.curobj.splice(x,1)[0].obj.screener = 0;
		} else if (tab.link == new_color) {
			/* link */
			screener.curobj.push(tab.parentNode.parentNode);
			tab.obj.screener = screener;
		}
	}
}

function html_select_append(select_div, list, key)
{
	for (var x = 0; x<list.length; x++) {
		var opt = document.createElement("option"),
		name = list[x][key];
		opt.value = name;
		console.log("html_select_append: name: " + name);
		opt.appendChild(document.createTextNode(name));
		$(select_div).append(opt);
	}
}

/* ****************
 *
 *   STYLE MODULE
 *
 *****************/
function SaveStyle()
{
	var name = window.event.target.previousSibling.value,
	style    = {},
	id       = "#QTMP ";

	for (obj in defaultStyle) {
		style[obj]   = {'color':            RGB2HEX($(id+obj).css('color')),
				'font-size':        $(id+obj).css('font-size'),
				'background-color': RGB2HEX($(id+obj).css('background-color'))};
		console.log("setting dict for obj: " + obj)
	}
	STYLES[name] = style;
	for (quadspace in W.quadspace) {
		for (quad in quadspace.quad)
			SetQuadStyle("#"+quad.id, name);
	}
}

/* Style Color Change */
function tmpcol()
{
	console.log("color: " + window.event.target.value);
	var color = window.event.target.value,
	obj       = window.event.target.parentNode.parentNode.querySelector(".obj");
	obj.style.color         = color;

	/* Set the object's color in the actual Template */
	$(".template ." + obj.classList[0].split("-")[0]).css("color", color);
}

function SetQuadStyle(quad, styleName)
{
	var styles = STYLES[styleName];
	for (style in styles) {
		console.log("setting " + quad + " color: " + styles[style].color);
		$(quad).find(style).css("color", styles[style].color)
	}
}

/* ****************
 *
 *  MAX/MIN QUADS
 *
 *****************/
function MaxQuads()
{
	var qsp = window.event.target;
	$("#MENU").addClass("fade");
	$("#MENU").slideUp();
	qsp.innerText = "🗗";
	qsp.setAttribute("onclick", "MinQuads()");
	qsp.title = "Minimize";
}
function MinQuads()
{
	var qsp = window.event.target;
	$("#MENU").removeClass("fade");
	$("#MENU").slideDown();
	qsp.innerText = "❐";
	qsp.setAttribute("onclick", "MaxQuads()");
	qsp.title = "Maximize";
}
function MaxQuad(QSID, QID)
{
	var quadspace = W.quadspace[QSID].quad,handler,obj,bsp,
	n             = window.event.target,
	qdiv          = "#P"+W.PID+"Q"+QSID+"q"+QID,
	quad          = quadspace[QID],
	ws            = 'ws'+quad.current_workspace;

	$(qdiv).css("min-height", "85vh");
	W.quadspace[QSID][0].className = "quadspace qmax";
	for (var x = 0; x<quadspace.length; x++) {
		if (QID != x)
			$(quadspace[x].qdiv).css("display", "none");
	}
	n.setAttribute("onclick", 'MinQuad('+QSID+','+QID+')');
//	$("#Q" + QSID + "q" + QID).addClass("animate__rubberBand");
	n.title = "Minimize";
	n.innerText = "🗗";
	obj = quad.workspace[ws].obj;
	for (var x = 0; x<obj.length; x++)
		if (obj[x].type == "chart")
			obj[x].ref.reflow();
	handler = quad.workspace[ws].MAX;
	if (typeof handler !== 'undefined')
		handler();

	bsp = n.parentNode.parentNode.firstElementChild.nextElementSibling;
	if (!bsp || bsp.classList[1] != "gridB")
		return;	
	$(".bobj", bsp).css("marginBottom", "50px")
}
function MinQuad(QSID,QID)
{
	var quadspace = W.quadspace[QSID].quad,handler,obj,
	n             = window.event.target,
	qdiv          = "#P"+W.PID+"Q"+QSID+"q"+QID,
	quad          = quadspace[QID],
	ws            = 'ws'+quad.current_workspace;

	$(qdiv).css("min-height", "47vh");
	W.quadspace[QSID][0].className = "quadspace " + W.workspace['ws'+QSID].grid;
	for (var x = 0; x<quadspace.length; x++) {
		if (QID != x)
			$(quadspace[x].qdiv).css("display", "block");
	}
	obj = quad.workspace[ws].obj;
	for (var x = 0; x<obj.length; x++)
		if (obj[x].type == "chart")
			obj[x].ref.reflow();
	n.setAttribute("onclick", 'MaxQuad('+QSID+','+QID+')');
//	$("#Q" + QSID + "q" + QID).removeClass("animate__rubberBand");
	n.title = "Maximize";
	n.innerText = "🗖";
	handler = quad.workspace[ws].MIN;
	if (typeof handler !== 'undefined')
		handler();
	bsp = n.parentNode.parentNode.firstElementChild.nextElementSibling;
	if (!bsp || bsp.classList[1] != "gridB")
		return;
	$(".bobj", bsp).css("marginBottom", "")

}
function screener_max(){$(".screener").css("height", "80vh")}
function screener_min(){$(".screener").css("height", "42vh")}
function search_min(){screener_min();$("#stocklist").highcharts().reflow()}
function search_max(){screener_max();$("#stocklist").highcharts().reflow()}
function template_min(){}
function template_max(){}


var StopTimer = 0;
function boarding(){
	setInterval(function() {
		var time = new Date().toLocaleString('en-US', { timeZone: 'America/New_York',hour12:false }).split("/")[2].split(" ")[1];
		if (time == "16:00")
			StopTimer = 1;
		else if (time == "09:30")
			StopTimer = 0;
		if (StopTimer)
			return;
		$("#boarding-time").html(time);
	}, 1000);
}


/* xterm.js */
function xterm_prompt(term)
{
	term.write('\r\n$ ');
}

function new_xterm(workspace)
{
	var xterm = new Terminal({cursorBlink:true,cols:80,rows:24}), xdiv = document.createElement("div"),QGID = "#"+workspace['ws'+workspace.current_workspace].id;
	bshide(QGID);
	xdiv.id = QGID + "-xterm";
	xterm.open(xdiv);
	$(QGID).append(xdiv);
	xterm._initialized = true;

	xterm.prompt = () => {
		term.write('\r\n$ ');
	};
	xterm_prompt(xterm);
}

/* JQueryTerm.js */
function new_jterm(workspace)
{
	var jterm = document.createElement("div"), QGID = "#"+workspace['ws'+workspace.current_workspace].id;
	bshide(QGID);
 	$(QGID).terminal(function(command) {
		if (command !== '') {
			try {
				var result = __EVAL(command);
				if (result !== undefined)
					this.echo(new String(result));
			} catch(e) {
				this.error(new String(e));
			}
		} else {
			this.echo('');
		}
	}, {
		greetings: 'JavaScript Interpreter',
		name: 'js_workspace',
		height: 400,
		prompt: 'js> '
	});
}

function new_select_option(option_text,select_box,value)
{
	var opt = document.createElement("option");
	opt.appendChild(document.createTextNode(option_text));
	opt.value = value;
	$(select_box).append(opt)
}

function sort_options(select_id)
{
	var options = $(select_id + " option");
	options.detach().sort(function(a,b) {
		var atext = $(a).text();
		var btext = $(b).text();
		return (atext > btext)?1:((atext < btext)?-1:0);
	});
	options.appendTo(select_id);
}