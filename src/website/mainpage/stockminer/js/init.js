/*********************************************
 * filename: mainpage/stockminer/js/init.js
 * License:  Public Domain
 ********************************************/
/*
 * qcache@user@init     // already logged in path
 * qcache@init
 *
 * [1] client calls init_quadverse()
 *   [1.1] LoadQuadverse(JSON)
 * [2] Server calls qcache() // loads QCACHE JSON
 * [3] Server calls init()
 *   [2] init_qspace()       // create User Quadspaces
 *   [3] init() creates Custom QPage's QuadVerse
 *   [4] RPC fini
 *     [4.1] Server calls rpc_session_finish() which sends the Backpage
 *     [4.2] login.js::backpage() called by the init_websocket() msg handler
 *     [4.3] init_monster_backpage()
 *     [4.4] LoadQuadVerse()
 *
 * Scenario 1: User loads / while not logged in. Result: 5 QuadVerses (0-4)
 *    User Logs in, Backtest becomes QuadVerse 5 (6th)
 * Scenario 2: User loads / while logged in. Result: 6 QuadVerses
 * Scenario 3: User loads /CustomQPage while not logged in. Result: 6 QuadVerses
 *    QPage is QuadVerse 5
 *    Backtest is QuadVerse 6
 * Scenario 4: User loads /CustomQPage while logged in. Result: 7 QuadVerses
 */
/*
 * /ws/ACTION_BOOT/addTable:lowcaps:P0Q0q0ws0/addTable:highcaps:P0Q0q0    - instruct the server to send back the JSON data for the lowcaps & highcaps tables
 * /ws/ACTION_RELOAD                                                      - websocket timed out; poll server and when server is back up the server will send a qreload RPC so the browser can reload
 */
function init_websocket(action) {
	var cmd,argv,f,rpc,backoff=1,path=document.location.pathname;

	if (action != ACTION_RELOAD) {
		action = ACTION_BOOT;
		if (path == "/") {
			// we're loading the standard mainpage from this server
			rpc    = quadspace_to_rpc(WEBSITE.QuadVerse[0].QuadSpace);
			// let the server know the QVID of the Profile QuadVerse
			rpc   += quadverse_profile(WEBSITE.QuadVerse);
		} else {
			// we're loading either a builtin QPAGE or a user's QPAGE, eg: stockminer.org/stocks (builtin) OR stockminer.org/myQuadverse (user's QPAGE)
			action = ACTION_QPAGE;
			rpc    = path+"/";
		}
	} else {
		// page reload doesn't require any extra information - this is a mess
		rpc = "";
	}
	url = 'wss://localhost:443/ws/' + action + rpc + "c=" + (localStorage.cookie?localStorage.cookie:"");
	console.log('init_websocket(): ' + url);
	WS           = new WebSocket(url);
	WS.onerror   = function() {WS.close()};
	WS.onclose   = function() {setTimeout(function() {init_websocket(ACTION_RELOAD)}, 2000*(backoff++))};
	WS.onmessage = function(e){
		var cmd  = e.data.substr(0, 4);
		CMD      = e.data; // dbg
		switch (cmd) {
			case "char":
				cmd = JSON.parse(e.data.substr(6));
				chart(cmd);
				break;
			case "back":
				backpage(e.data.substr(4)); // login.js::backpage() - load a private QuadVerse
				break;
			case "sque":
				squeak_rpc(e.data.split("`").slice(1), (cmd.indexOf("🗖") == -1) ? W.current_quadspace : ("#P"+cmd.split("🗖")[1]+"Q0"));
			case "deft":
				rpc_deftab(e.data.substr(7));
				break;
			default:
				cmd = e.data.split("@");
				for (var x = 0; x<cmd.length; x++) {
					argv = cmd[x].split(" ");
					f = op[argv[0]];
					if (typeof f === 'undefined') {
						console.log("UNDEFINED ERROR: " + argv[0]);
						break;
					} else
						f(argv);
				}
		}
	};
}

/*
 * bootstrap the first quadspace - it may come from freepage.js, indexDB or (eventually) IPFS
 *   - build an RPC command string for the server when initiating a GET /ws websocket so that
 * we can send along instructions for loading the dynamic objects of quadspace0
 *   - eg: GET /ws/action/addChart:TSLA:#P0Q0q1ws0:chartClassName/addTable:Lowcaps:P0Q0q0ws0/addTable:Highcaps:P0Q0q0ws1
 *   - eg: GET /ws/action/rpc
 * returns: rpc string chain
 */
function quadspace_to_rpc(quadspace)
{
	var quads = quadspace[0].quads, rpc = "/";
	for (var QID = 0; QID<quads.length; QID++) {
		quad = quads[QID];
		for (var WSID = 0; WSID<quad.workspace.length; WSID++) {
			var workspace = quad.workspace[WSID];
			var QGID      = ":P0Q0q" + QID + "ws" + WSID;
			switch (workspace.mod) {
				case "addTable":
					rpc += "addTable:" + workspace.args.id + QGID + "/";
					break;
				case "addChart":
					rpc += "addChart:" + "?" + QGID + ":" + workspace.args.class + "/";
					break;
			}
		}
	}
	return rpc;
}

function quadverse_profile(quadverse_array)
{
	var quadverse_index = 0;
	for (quadverse in quadverse_array) {
		if (quadverse.profile)
			return "addProfile:" + quadverse_index + "/";
		quadverse_index++;
	}
	return "";
}

// used by init() and LoadQuadverse(), ancient, a mess
function new_quadverse(name,className,QVID)
{
	var quadverse, id;

	W = new ws();
	quadverse = $("#QUADVERSE > .quadverse").clone();
	QUADVERSE.append(quadverse[0]);
	if (!QVID)
		QVID = Q.length;
	id                      = "quadverse" + QVID;
	quadverse[0].id         = id;
	quadverse[0].className += className;
	W.init(document.querySelector('#quadverse' + QVID + ' .chrome-tabs'), "#"+id, QVID);
	QuadVerses[name]        = W;
	Q[QVID]                 = W;
//	Q.push(W);
	return W;
}

/*
 * This will one day be callable not just during init, it will need to be able to support multiple "module/website" types
 * eg: "stocks", "biomedical", "aero", "maths"
*/
function LoadBlankspace(JSON)
{
	var bspace = '<div class=bspace style=display:none>';

	if (!JSON)
		return;
//	module_hook(MODULE_LOAD_BLANKSPACE, bspace); not done yet
	bspace += '<input class=bticker type=text placeholder=Ticker title="Specify a ticker when loading a chart"></div>';
	$("body").append(bspace);
	JSON.forEach((entry) => {
		var name = entry.name, type = entry.type, click = entry.click ? entry.click : "", div, img;
		switch (type) {
			case "img":
				img = '<img src=/img/bs-' + name.toLowerCase() + '.png />';
				div = '<div class=bobj><span class="bname bimg" '  + click + '>' + img + name + '</span></div>';
				break;
			case "icon":
				div = '<div class=bobj><span class="bname bicon" onclick="' + click + '">' + entry.icon + '</span><span class=bname>' + name + '</span></div>';
				break;
		}
		$("body > .bspace").append(div);
	});
}

/*
'[{"workspace":[{"mod":"addTable","title":"Lowcaps", "args":{"id":"LG,LL"}},
                {"mod":"addTable","title":"Highcaps","args":{"id":"HG,HL"},"click":"highcaps()"}]},
  {"workspace":[{"mod":"addChart","title":"Chart"}]}]'*/

function LoadQuadverse(Website)
{
	var JQuadVerse, JQuadSpace, JQuad, JWorkspace, quadverse, quadspace, quadverse_title,quad, QVID, style, nr_presets = $(".desktop").length;

	LoadBlankspace(Website.BlankSpace);

	for (var x = 0; x<Website.QuadVerse.length; x++) {
		/* new_quadverse() */
		JQuadVerse      = Website.QuadVerse[x];
		quadverse_title = JQuadVerse.title;
		quadverse       = new_quadverse(quadverse_title, JQuadVerse.class);
		QVID            = quadverse.PID;
		if (JQuadVerse.init)
			quadverse.qinit = window[JQuadVerse.init];
		QuadVerses[quadverse_title] = quadverse;

		if (JQuadVerse.backpage)
			WEBSITE.QuadVerse.push(JQuadVerse);

		quadverse_menu_add(quadverse_title, JQuadVerse.backpage?"backpage":"", JQuadVerse.image, JQuadVerse.squeakpage);
		/* addQuadspace() */
		for (var QSID  = 0; QSID<JQuadVerse.QuadSpace.length; QSID++) {
			JQuadSpace = JQuadVerse.QuadSpace[QSID];
			quadspace  = quadverse.addQuadspace(JQuadSpace.title,0,JQuadSpace.grid,JQuadSpace.quads.length,JQuadSpace.click);
			if (JQuadSpace.noload)
				continue;
			/* for each quad */
			for (var QID = 0; QID<JQuadSpace.quads.length; QID++) {
				JQuad  = JQuadSpace.quads[QID];
				quad   = quadspace.quad[QID];
				if (JQuad.class)
					$(quad.qdiv)[0].className += " " + JQuad.class;
				/* addWorkspace() */
				for (var WSID = 0; WSID<JQuad.workspace.length; WSID++) {
					JWorkspace = JQuad.workspace[WSID];
					if (!JWorkspace.args)
						JWorkspace.args = {};
					grid = JWorkspace.grid;
					if (JWorkspace.mod == "addBlankspace") {
						quad['addBlankspace'](QVID, QSID, QID);
					} else {
						JWorkspace.args['WSID'] = quad.addWorkspace({title:JWorkspace.title,favicon:true},{background:false,click:JWorkspace.click},grid?grid:"",QSID,QID);
//						console.log("QSID: " + QSID + " WSID: " + WSID + " args: " + JSON.stringify(JWorkspace.args));
						quad[JWorkspace.mod](JWorkspace.args); // call workspace module init function
					}
					/********************
					* Add Custom Styles *
					*********************/
					if ((style=JWorkspace.style)) {
						if (style[0] == '@') {
							var styles = style.split(",");
							for (var style_idx = 0; style_idx < styles.length; style_idx++)
								html_new_style(Website[styles[style_idx]]);
						}
					}
				} // for each Workspace
				/* show the deafult tab (0) for this workspace */
				workspace_showtab(QSID, QID, 'ws0');
			} // for each Quad
			if (JQuadSpace.init)
				window[JQuadSpace.init](QVID,QSID);
		} // for each QuadSpace
		if (!JQuadSpace.noload)
			quadverse.showtab(quadverse.workspace['ws0'].tab);
	} // for each QuadVerse
}

function init_quadverse(){
//	if (localStorage.QCACHE)
//		QCACHE = JSON.parse(localStorage.QCACHE);
	init_stock_screener();
	QUADVERSE  = ID("QUADVERSE");
	LoadQuadverse(WEBSITE);
	W          = QuadVerses['Screener'];
	ECELL[916] = 1; // editable cells in watchtables
	init_obj_trash();
	LoadThemes();
}

// post stock site initialization
function init_menu()
{
	$("#desktops").hide();
	$("#bell").click (function() {notifier()});
	$("#query").keyup(function(event) {search(event)});
	$("#logo").removeClass("logoi")
	$(".login").val("Login");
	CRULE = $(".color-rule").detach();
	CRULE.css("display", "block");
	$("#color-diag").draggable();

	/* QuadVerse Menu Switch onclick */
	$(".qpop").click(function(){
		if ($(this).hasClass("clicked-once")) {
			$("#desktops").slideUp();
			$('.desktop').addClass("animate__backOutLeft");
			$(this).removeClass("clicked-once");
	   	} else {
			$('.desktop').removeClass("animate__backOutLeft");
			$(this).addClass("clicked-once");
			$('#desktops').slideDown();
			$('.desktop').addClass("animate__backInLeft");
		}
	});

	// need to move these elsewhere	
	init_options();
	if (localStorage.sigmon) {
		table([0, 'sigmon', localStorage.sigmon]);
		SIGMON = 1;
	}
}

function loadprofile(p,title)
{
	var owner = (USER==document.location.pathname.substr(1))?1:0,qsp,LQ,MQ,RQ;

	if (!owner) {
		qsp   = W.addQuadspace(title, 0, "gridSQ", 3, 0, "dodgerblue",0,0,-1);
		LQ    = qsp.quad[0];
		MQ    = qsp.quad[1];
		RQ    = qsp.quad[2];

		LQ.addWorkspace({title:"Channels", favicon:0},{background:0}, "", 0, 0, -1);
		MQ.addWorkspace({title:title,      favicon:1},{background:0}, "", 0, 1, -1);
		RQ.addWorkspace({title:"Friends",  favicon:0},{background:0}, "", 0, 2, -1);
		MQ.addProfile(0);
	}
	squeak_rpc(p,W.current_quadspace);
}

// called by the server after the websocket connection is established
function rpc_init(av)
{
	init_qspace();
	if (!av[1]) {
		W.showtab(W.workspace['ws0'].tab);
		QUADVERSE_SWITCH('Screener');
	} else {
		if (av[1] == 'stocks') {
			QUADVERSE_SWITCH('Screener');
			stockpage(av[2],-1);
		/*  0   1       2           3    4    5            */
		/* init qpage   qpage_json  name QVID              */
		/* init profile qpage_json, name QVID profile_json */
		} else if (av[1] == 'profile' || av[1] == 'qpage') {
			var QSPACE = JSON.parse(av[2])[0], name, QVID = av[4], start = 0;

			console.log("INIT: " + av.join(" "));
			QCACHE[QVID] = QSPACE;
			if (QVID > 5) {
				name = av[3];
				W = new_quadverse(name, "", QVID);
			} else {
				name = 'Profile';
				W = Q[4];
			}

			if (av[1] == 'profile') {
				loadprofile(av.slice(5).join(" ").split("`").slice(1), name);
				start = 1;
			}

			for (var QSID=start; QSID<QSPACE.length; QSID++) {
				if (!QSPACE[QSID] || QSPACE[QSID] == -1)
					continue;
				for (var x = 0; x<QSPACE.length; x++)
					if (QSPACE[x] == -1)
						W.idmap[x] = -1;
				W.addQuadspace(QSPACE[QSID].title, 0, QSPACE[QSID].grid, 4, "quadspace_load",0,0,1).custom = 1;
			}
			Q[0].showtab(Q[0].workspace['ws0'].tab);
			QUADVERSE_SWITCH(name);
			if (!start)
				quadspace_load(W.PID,0);
			W.showtab(W.workspace['ws0'].tab);
			W.URL = document.location.pathname.substr(1);
		}
	}

	/*
	 * Notify the server that initialization is complete
	 * server will respond with its own fini RPC msg
	 * resulting in a call to rpc_fini() below
	 */
	WS.send("fini");
	WINIT=1; // workspaces are initialized
}

function rpc_fini(av,rpc){
//	load_qpages();
	$(".matrix-loader").css("display","none");
}

function quadverse_menu_add(name, pagetype, quadverse_image, is_squeakpage)
{
	var onclick = "QUADVERSE_SWITCH(-1,1)";

	$("#desktops").append("<div id=P" + name + ' class="desktop animate__animated ' + pagetype + '"><div class=qtitle>' + name + '</div></div>');

	if (quadverse_image)
		$("#P"+name).append('<img onclick="' + onclick + '" src=/img/' + quadverse_image + ' />');

	if (is_squeakpage) {
		$("#P"+name).addClass("minibat");
		$("#P"+name).attr("onclick", onclick);
	}
}

function quadverse_img_set(user,URL,img)   {
	img.onclick = function() {window.location="/"+user+"/"+URL}
//	img.setAttribute("onclick", encodeURIComponent(loc));
//	img.setAttribute("onclick", "QUADVERSE_SWITCH(window.event.target.previousSibling.innerText,1)");
	$("#desktops").append("<div id=P" + URL + ' class="desktop animate__animated"><div class=qtitle>' + URL + '</div></div>');
	$("#P"+URL).append(img);
}

var QIMG = [ 0, 'mouse', 'bat', 'bird', 'dolph', 'shark', 'uni' ];

function load_qpages()
{
	for (var x = 0; x<QPAGE.length; x++) {
		var URL = QPAGE[x].split(":"), user = URL[0], img = URL[2];
		if (img == "0") {
			img     = new Image();
			img.src = "/blob/" + QPAGE[x];
		} else
			img = $('<span class=' + QIMG[img] + "></span>")[0];
		quadverse_img_set(user, URL[1], img);
	}
}

/* qpage QPageTitle:[0-6] */
var KPG;
function rpc_qpage(av) {
	QPAGE = av.splice(1);
	KPG=av;
	console.log("RPC QPAGE");
}

function rpc_qcache(av)
{
	console.log("qcache: " + av[1]);
	var r = 0;
	if (QCACHE.length > 0)
		r = 1;
	QCACHE = JSON.parse(av[1].replaceAll("^", " "));
	if (av[2]) { // getting qcache after login
		if (r)
			window.location.reload();
		init_qspace();
		W.showtab(W.workspace['ws0'].tab);
	}
}

/* Custom User QuadSpaces */
function init_qspace()
{
	var QVID, QSID, QID, QSPACE, grid, rpc;

	for (var QVID=0; QVID<5; QVID++) {
		if (!QCACHE[QVID])
			continue;
		W = Q[QVID];
		for (var QSID=0; QSID<QCACHE[QVID].length; QSID++) {
			if (QVID == 0 && QSID < 4) // HARDCODED
				continue;
			if (QVID == 1 && QSID < 2)
				continue;
			QSPACE = QCACHE[QVID][QSID];
			if (QSPACE == -1) {
				W.idmap[QSID] = -1;
				continue;
			}
			if (!QSPACE)
				continue;
			grid = QSPACE.grid;
			if (QSPACE.sp) {
				SP[QSPACE.title] = { QVID: QVID, QSID: QSID, loaded: 0 };
				grid += " opgrid stockpage";
				rpc = -1;
			}
			W.addQuadspace(QSPACE.title, 0, grid, 4, "quadspace_load", 0, 1, 1, rpc).custom = 1;
		}
	}
	W = Q[0];
}

function compat()
{
	if(typeof String.prototype.replaceAll === "undefined")
		String.prototype.replaceAll = function replaceAll(search, replace) { return this.split(search).join(replace); }
	if (!localStorage.version || localStorage.version != VERSION) {
		localStorage.clear();
		localStorage.version = VERSION;
	}
}

// will be used to script different events/object creations/deletions/mods in a 'scriptable' QuadSpace
var WebScript = `{
	"Scene1":[{
		// Actions to take in Quad 1
		"Quad1":[{
			"Action1":{"Load":"Screener", "World":"Stocks", "Type":"Showtime"}
		}]
	}]
}`;
