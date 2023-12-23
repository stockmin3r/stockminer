/************************************************
 * filename: mainpage/stockminer/js/quadspace.js
 * License:  Public Domain
 ***********************************************/
((window, factory) => {window.ws = factory(window, window.Draggabilly)})(window, (window, Draggabilly) => {
	const TAB_CONTENT_MARGIN = 9
	const TAB_CONTENT_OVERLAP_DISTANCE = 10
	const TAB_OVERLAP_DISTANCE = (TAB_CONTENT_MARGIN * 2) + TAB_CONTENT_OVERLAP_DISTANCE
	const TAB_CONTENT_MIN_WIDTH = 140;
	const TAB_CONTENT_MAX_WIDTH = 180;
	const TAB_SIZE_SMALL = 84
	const TAB_SIZE_SMALLER = 60
	const TAB_SIZE_MINI = 48
	const noop = _ => {}
	const closest = (value, array) => {
		let closest = Infinity
		let closestIndex = -1
		array.forEach((v, i) => {
			if (Math.abs(value - v) < closest) {
				closest = Math.abs(value - v)
				closestIndex = i
			}
    	})
    	return closestIndex
	}
	const tabTemplate = `
	<div class=chrome-tab>
		<div class=chrome-tab-dividers></div>
		<div class=chrome-tab-background>
			<svg version="1.1" xmlns="http://www.w3.org/2000/svg">
				<defs>
					<symbol id="chrome-tab-geometry-left" viewBox="0 0 214 36">
						<path d="M17 0h197v36H0v-2c4.5 0 9-3.5 9-8V8c0-4.5 3.5-8 8-8z"/>
					</symbol>
					<symbol id="chrome-tab-geometry-right" viewBox="0 0 214 36">
						<use xlink:href="#chrome-tab-geometry-left"/>
					</symbol>
					<clipPath id="crop">
						<rect class="mask" width="100%" height="100%" x="0"/>
 					</clipPath>
				</defs>
				<svg width="52%" height="100%">
					<use xlink:href="#chrome-tab-geometry-left" width="214" height="36" class="chrome-tab-geometry"/>
				</svg>
				<g transform="scale(-1, 1)">
					<svg width="52%" height="100%" x="-100%" y="0">
						<use xlink:href="#chrome-tab-geometry-right" width="214" height="36" class="chrome-tab-geometry"/>
					</svg>
				</g>
			</svg>
		</div>
		<div class=chrome-tab-content>
			<div class=chrome-tab-favicon><span class="link">&#9881;</span></div>
			<div class=chrome-tab-title></div>
			<div class=chrome-tab-drag-handle></div>
			<div class=chrome-tab-close></div>
		</div>
	</div>`
  const defaultTabProperties = {title:'New tab',favicon:false}
  let instanceId = 0
  class ws {
    constructor()           {this.draggabillies = []}
	/* *******
	 *  INIT *
	 ********/
	init(el,div,PID) {
		this.workspace         = $(div);  /* Jquery ptr to the workspace (or quadverse) bound to this <div> */
		this.quadspace         = [];      /* Array of QuadSpaces */
		this.el                = el;      /* html element tag */
		this.nr_ws             = 0;
		this.PID               = PID;     /* QuadVerse ID */
		this.qdiv              = div;     /* QGID */
		this.current_workspace = 0;
		this.current_quadspace = 0;
		this.idmap             = [];
		this.instanceId = instanceId;this.el.setAttribute('data-chrome-tabs-instance-id', this.instanceId);instanceId += 1;this.setupCustomProperties();this.setupStyleEl();this.setupEvents();this.layoutTabs();this.setupDraggabilly();
	}

	addWorkspace(tabProperties, {animate=true,background=false,click=false} = {}, grid, QSID, QID, rpc) {
		/* this:Quad Context */
		var newspace           = document.createElement("div"),id = this.newid(),tab,QVID=this.PID,
		WSID                   = "ws"  + id;
		newspace.className     = "ws " + grid;
		newspace.id            = "P"   + QVID + "Q" + QSID + "q" + QID;
		newspace.qdiv          = "#"+newspace.id;
		newspace.id           += WSID;
		newspace.w             = this;
		this.current_workspace = id;        // quad->
		this[WSID]             = newspace;
		WMAP[newspace.id]      = newspace;
		this.workspace.append(newspace);
		this.nr_ws++;

		/* Tab */
		tab = this.ChromeTab(tabProperties,click,background,"",QSID,QID,id);
		tab.QVID               = QVID;
		tab.QSID_int           = QSID;
		tab.QID                = QID;
		tab.WSID               = id;
		tab.classList.add(WSID);
		this.workspace[WSID]   = {"div": this.workspace, "tab": tab, "name": tabProperties.title, "obj":[]};

		/* Workspace Tab Rename */
		this.showtab(tab,rpc);
		this.setrename("#P"+QVID+"Q"+QSID+"q"+QID+" ."+WSID,QVID,QSID,QID,id,tabProperties.title);
		return id;
	}
	addQuadspace(title,blank,grid,nr_quads,click,color,ico,skip,rpc){
		var tab,QVID,QSID,QID,QSID_STR,workspace,quadspace = $('<div class="quadspace ' + grid + '"></div>');
		/* this:QuadVerse[W.PID] Context */
		QVID = this.PID;
		quadspace.quad  = [];
		this.nr_ws++; // nr_ws means 'number of quadspaces' for a quadverse and 'number of workspaces' for a quad
		this.workspace.append(quadspace);
		QSID = quadspace.QSID = this.newid(skip);
		this.quadspace[QSID]  = quadspace;
		quadspace[0].id = QSID_STR = "P"  + QVID + "Q" + QSID;

		/* Tab */
		tab = this.ChromeTab({title:title,favicon:ico,color:color},0,1,"#P" + QVID + "Q" + QSID, QSID, "");
		this.workspace['ws'+QSID] = { tab: tab, grid: grid };

		/* Click */
		if (click) {
			tab.click   = click;
			tab.onclick = function(){window[click](QVID, QSID)}
		}
		/* Create Quads from Quad() template */
		for (var x = 0; x<nr_quads; x++) {
			QID = QSID_STR + 'q' + x;
			quadspace.append(this.Quad(QVID, QSID, QID, x));
			$("#"+QID).draggable({handle:'.wspanel'});
			/* Create a Workspace for this Quad */
    		workspace = new ws();
    		quadspace.quad.push(workspace);
    		workspace.init($("#"+QID+" .chrome-tabs")[0], "#"+QID, QVID);
    		workspace.qdiv = "#"+QID;
    		workspace.quadspace = quadspace.quad;
    		if (blank)
    			workspace.addBlankspace(QVID, QSID, x);
		}
		this.setrename("#quadverse"+QVID+" .chrome-tab:nth-child(" + QSID+")",QVID,QSID,0,-1,title);
		this.showtab(tab,rpc);
		return quadspace;
	}
	addBlankspace(QVID, QSID, QID, WSID){
		var b = $("body > .bspace").clone()[0],span,
		quad  = Q[QVID].quadspace[QSID].quad[QID];
		if (!WSID)
			WSID = 'ws'+quad.addWorkspace({title:"New Tab",favicon:true},{background:true}, "gridB", QSID, QID);
		WMAP['P'+QVID+'Q'+QSID+'q'+QID+WSID] = quad[WSID];
		b.style.display="grid";
		span = b.querySelectorAll('span');
		for (var x = 0; x<5; x++)
			span[x].setAttribute("onclick",'ModLoad('+QSID+','+QID+','+WSID.substr(2)+',"'+modules[x]+'")');
		quad[WSID].appendChild(b);
	}
	addUFO(args) {
		var QGID = this['ws'+args.WSID].id, u = $("#ufo")[0];
		if (u.style.display=='none') {
			$("#"+QGID).append(u);
			u.style.display='block';
		}
		ufoinit(MTAB,QGID)
	}
	addVFO(args) {vfoinit(VTAB,this['ws'+args.WSID].id)}

	addChart(args){
		var chart = document.createElement("div");
		chart.className = "chart" + (args.class ? (" " + args.class) : "");
		console.log("chart class: " + chart.className);
		this['ws'+args.WSID].appendChild(chart);
	}
	addTable(args){
		var tableDiv = document.createElement("div"),
		tables       = args.id.split(",");
		if (!args.params)
			return;

		var columns  = args.params.columns,
		order        = args.params.order,
		slope        = args.params.slope;

		for (var x = 0; x<tables.length; x++) {
			var column_array = [];
			for (var y = 0; y < columns[x].length; y++) {
				column_array.push({"data":columns[x][y]});
			}
			// calls DataTable() and places the table into the global T [] array of tables
			datatable({id:tables[x],columns:column_array, ncol:columns[x].length, order:order!=undefined?order[x]:1, slope:slope!=undefined?slope[x]:"asc"});
			var table = $("#"+tables[x]).detach();
			table.css("display", "");
			table.addClass(args.class);
			this['ws'+args.WSID].appendChild(table[0]);
			if (args.caption)
				table[0].caption.innerText = args.caption;
		}
	}
	addDIV(args){
		var div = $("#"+args.id).detach();
		div.css("display", "");
		if (args.className)
			div.addClass(args.className);
		this['ws'+args.WSID].appendChild(div[0]);
	}

	/* *********************************
	 *
	 *   (BLANKSPACE WORKSPACE MODULES) 
	 *
	 **********************************/
	bsChart(args) {
		var QGID, QSID = args.QSID, QID = args.QID, ticker = args.ticker;
		if (!args.QGID)
			QGID = "#"+this['ws'+this.current_workspace].id;
		if (!ticker) {
			ticker=$(QGID + " .bticker").val().toUpperCase();
			if (!ticker) {
				$(QGID + " .bticker").css("animation", "shake 0.5s");
				return;
			}
		}
		WS.send("chart " + QGID.substr(1) + " " + ticker);
		bshide(QGID);
		$("#P"+W.PID+"Q"+QSID+"q"+QID+" .ws"+this.current_workspace+" .chrome-tab-title")[0].innerText = ticker;
		this.qcache_add(W.PID,QSID,QID,this.current_workspace,ticker,['chart'],ticker);
		$(QGID).append("<div id=load class=loading></div>");
	}

	/*
	 * Changing a Preset:       ALTERS ALL XLS Watchtables in controller.curobj
	 * Reloading an XLS ticker: ALTERS XLL XLS Watchtables in controller.curobj
	 */
	addXLS_controller(QGID,WSID,args) {
		var XLS_controller = $("body > .XLS-mod").clone()[0], ticker, opt, args;
		XLS_controller.style.display='block';
		bshide(QGID);
		// Load Watchtable Presets into XLS Controller
		for (name in TP) {
			opt = document.createElement("option");
			opt.value = name;
			opt.appendChild(document.createTextNode(name));
			$(".XLS-presets", XLS_controller).append(opt);
		}

		// Append XLS Controller to the Workspace
		this[WSID].appendChild(XLS_controller);
		this.workspace[WSID].obj.push({type:"XLS-controller", ref:XLS_controller});
		$(this.qdiv).css("overflow", "auto");

		// if no args.ticker then check the blankspace ticker input field
		if (!args.ticker)
			ticker = $(".bticker", QGID).val();

		// Preset Select Box onchange
		args = {ticker:ticker,QGID:QGID};
		$(".XLS-presets", XLS_controller)[0].onchange = function() {
			loadXLS(XLS_controller,args);
		}
		$(".XLS-ticker", XLS_controller).keyup(function(event) {
			if (event.keyCode === 13)
				loadXLS(XLS_controller,args);
		});

		XLS_controller.curobj = [];
		return XLS_controller;
	}

	// called ONLY by blankspace XLS image onclick, args only contains QSID and QID
	addXLS(args) {
		var WSID = 'ws'+this.current_workspace, QGID = "#"+this[WSID].id,
		XLS_controller = get_QUAD_OBJ(['any'], "XLS-controller", this);
		if (!XLS_controller)
			XLS_controller = this.addXLS_controller(QGID, WSID, args);
		loadXLS(XLS_controller, {QGID:QGID});
	}

	addWatchtools(quad, QGID, WSID, watchtable_id) {
		var watchtools = $('body > .watchtable-tools').clone()[0];
		bshide(QGID);
		quad[WSID].appendChild(watchtools);
		$(watchtools).css("display", "block");
		$(".watchtable-input", watchtools).keyup(function(event) {
			if (event.keyCode === 13) {
				watchlist_addstock(watchtable_id);
				this.value = '';
			}
		});
		quad.workspace[WSID].obj.push({type:"watchtools", OBJ_ID:"watchtools", ref:watchtools});
		watchtools.curtab = $("#"+watchtable_id)[0];
		return watchtools;
	}

	/* CALLED BY 5 paths:
	 *   - QuadMenu({send_rpc:1}):                click to add watchtable to a Quad's currently selected Workspace (Quad Context)
	 *   - ModLoad({QSID:x,QID:y}):               Blankspace module onclicks
	 *   - quadspace_load({QGID,TID,send_rpc:0}): called when loading a QuadSpace (no context) (TID:OBJ_ID)
	 *   - qupdate({QGID,TID,send_rpc:0}):        called by RPC when another user adds a watchtable to a Quad's Workspace (no context)
	 *   - watchtable_workspace()                 called when parsing Website.json in init.js::LoadQuadverse()
	 *
	 *   + screener.curobj:                       list of tables linked with this screener
	 *   + watchtable.obj.screener:               pointer to the screener linked to this table
	 */
	addWatchtable(args){
		var QGID = args.QGID, watchtable_id = args.TID, quad, QVID,QSID,QID,WSID,watchtable,id,screener,obj;
		// QuadMenu() && ModLoad() paths (ModLoad() doesn't supply a QGID, in this function its args (QSID,QID) are unused)
		if (!QGID)
			QGID = this.qdiv+"ws"+this.current_workspace;

		/*  For workspaces & quadverses .w points to the parent "container"
		 *  for a workspace .w will point to the parent quad
		 *  for a quadspace .w will point to the parent quadverse
		 */
		quad = WMAP[QGID.substr(1)].w;

		watchtable = Watchtable({type:'watchlist',dict:STAB,QGID:QGID,TID:watchtable_id,menu:1,order:1});
		if (!watchtable_id)
			watchtable_id = watchtable.id;
//			watchtable_id = watchtable.id.split("-")[0];
		id   = QGID.replace(/[^0-9]/g, '').split('');
		QVID = id[0], QSID = id[1], QID = id[2], WSID = id[3];
		obj  = {type:"wstab", TID:watchtable_id}
		// if this is the first watchtable in this Workspace then create a watchtools controller and assign the new watchtable we created to it
		if (!get_QUAD_OBJ(['any'], "watchtools", quad)) {
			quad.addWatchtools(quad, QGID, "ws"+WSID, watchtable_id);
			console.log("setting watchtools curobj to watchtable");
		}
		// if this QuadSpace has a Screener and the Screener has no Watchtable assigned to it then assign it this one
		screener = get_QSP_OBJ([0,{filter:'stocks'}], "screener", quad.quadspace);
		if (screener.length) {
			screener = screener[0];
			screener.curobj.push(watchtable);
			obj.screener = screener;
		}

		watchtable.link = 'cyan';
		watchtable.obj  = obj;
		obj.ref         = watchtable;
		quad.workspace["ws"+WSID].obj.push(obj);
		watchtable.onclick = function(){selobj(watchtable)};
		$("#P"+QVID+"Q"+QSID+"q"+QID).css("overflow", "auto");

		// only QuadMenu
		if (args.send_rpc != 0) {
			console.log("sending rpc");
			WS.send(QUPDATE_ADD_WSTAB + QVID + " " + QSID + " " + QID + " " + WSID + " " + watchtable_id);
		}
	}

	/* ********************************
	 *
	 *   INIT DEFAULT QUADVERSE SETUP
	 *
	 *********************************/
	// [QVID, QSID, QID, WSID] - [0, 1, 0, 0] #P0Q1q0ws0
	screener_workspace(args){
		var screener           = $("body > .watchmgr").clone(), WSID = 'ws'+args.WSID;
		screener.QGID          = this[WSID].qdiv+WSID;
		screener.quadspace     = this.quadspace;
		screener.watchtable_id = args.watchtable;
		screener.type          = args.type;
		screener[0].screener   = screener;
		screener.curobj        = [];
		screener.link          = 'cyan';
		Screeners.push(screener);
		this[WSID].appendChild(screener[0]);
		this.workspace[WSID].obj.push({type:"screener", filter:args.type, ref:screener});
		this.workspace[WSID].MIN = screener_min;
		this.workspace[WSID].MAX = screener_max;
		bshide(screener.QGID);
		loadScreener(screener, args.watchtable)
	}
	// [QVID, QSID, QID, WSID] - [0, 1, 1, 0] #P0Q1q1ws0
	watchlist_workspace(args){
		var name, watchlists, pub, WSID = 'ws'+args.WSID;
		this[WSID].appendChild($(".pubwatch-quad").detach()[0]);
		newtable("PUB","desc",[{"data":"N"},{"data":"U"},{"data":"NT"},{"data":"A"}],0,1);
		$(this[WSID].qdiv+" .pubwatch-quad").css("display", "block");
		$('#PUB tbody').on('click','tr',function(){
			$("#PUB .highlight").removeClass("highlight");
			$(this)[0].className += " highlight";
			name = T['PUB'].row(this).data().N;
			if ($(".watchlist-select option:selected").val() == 1) {
				watchlists = GW;
				pub = 1;
			} else {
				watchlists = WL;
				pub = 0;
			}
			for (var x = 0; x<watchlists.length; x++) {
				var w = watchlists[x];
				if (w.name == name) {
					load(name,"morphtab","P0Q1q2ws0",w.id,pub);
					loadAlerts(name,w.id);
					break;
				}
			}
		});
		Watchlists_RClick_Menu();
	}
	// [QVID, QSID, QID, WSID] - [0, 1, 2, 0] #P0Q1q2ws0
	watchtable_workspace(args){
		var WSID = 'ws'+args.WSID, QGID = this[WSID].qdiv+WSID;
		this.addWatchtable({QGID:QGID,TID:args.watchtable,send_rpc:0});
	}

	/* *********
	 * SHOWTIME
	 **********/
	showtime_workspace(args){
		var mod = $("body > .showtime").clone(), QGID,
		WSID = 'ws'+args.WSID;
		QGID = this[WSID].qdiv+WSID;
		mod.css("display", "block");
		bshide(QGID);
		this[WSID].appendChild(mod[0]);
		$(".wdst", mod)[0].id = "stocklist-"+QGID.substr(1);
    	this.workspace[WSID].MIN = search_min;
    	this.workspace[WSID].MAX = search_max;
		$(".stocklist li", mod).draggable({helper:"clone",revert:"valid",scroll:false,stop:function(event,ui){
			STOP  = ui;
			STOPE = event;
		}});
	}

	/* *********
	 * PROFILES
	 **********/
	profile_workspace(args){
		if (W.PID ==4 || USER==document.location.pathname.substr(1))
			$('<span class=wsconf onclick=sqconf() title="Show Profile">🏰</span><span class=compose onclick=compose() title="New Squeak">🖊</span>').insertBefore($(this.qdiv+" .wsnew"));
		var sq = $('body > .SQ').clone()[0];
		sq.style.display="block";
		this['ws'+args.WSID].appendChild(sq);
		$("textarea", sq).click(function(){
			console.log("clicked on textarea");
			$(this)[0].className = "grow";
		});/*
		$("#cfile")[0].onchange = function(e) {
			var img = $("#compose-msgbox").append("<img>");
			sendfile($("#cfile")[0].files[0], img, 7);
		}*/
	}


	/* Option Quads */
	addOpTable(args){}
	addOpLists(args){}
	addOpChart(args){
		var opchart = $("<div id=OPCHART></div>");
		this['ws'+args.WSID].appendChild(opchart[0]);
	}
	addOpTools(args){
		var toolbox = $("#OPDASH").detach();
		this['ws'+args.WSID].appendChild(toolbox[0]);
		toolbox.css("display", "block");
	}
	addCandles(args) {
		this['ws'+args.WSID].appendChild($("#CANDLES").detach()[0]);
		$("#CANDLES").css("display", "block");
	}
	addIndiMod(args){
		var mod = $(".indi-mod").clone(), WSID = 'ws'+args.WSID;
		mod.css("display", "block");
		loadpresets(mod);
		this[WSID].appendChild(mod[0]);
		this.workspace[WSID].MIN = screener_min;
		this.workspace[WSID].MAX = screener_max;
		mod.on('click', 'tr', function(){ROW=this;});
	}
	addScriptMod(args){
		this.workspace[0].style.overflow='auto';
		this['ws'+args.WSID].appendChild($('<div id=scripts><table id=SCRTAB class=STAB></table></div>')[0])
	}
	addStyleMod(args){
		var mod = $(".style-mod").clone(),quad,obj, WSID = "ws"+args.WSID;
		mod.css("display", "block");
		this[WSID].appendChild(mod[0]);
		quad = $(this[WSID].qdiv).clone();
		$(".pubwatch-quad", quad).remove();
		quad[0].id = "QTMP";
		$(quad).find(".wsnew").prop("onclick", "");
		$(quad).find(".max").prop("onclick", "");
		$(quad).find(".chrome-tab-favicon").prop("onclick", "");
		$(quad).click(function(e){
			console.log("addStyleMod click: " + e.target.className);
			obj = e.target.className;
			$(mod).find(".obj").remove();
			$(mod).find(".obj-name").remove();
			$(mod).find(".obj-name").append(objName[obj]);
			$(mod).find(".temp-obj").append(styleObj[obj]);
			$(mod).find(".cblock").val(RGB2HEX($(e.target).css("color")));
			if (obj.indexOf("bgblock") != -1) {

			}
			e.preventDefault();
		});
		$(".styleSave", mod).click(SaveStyle);
		$("#quadverse0 .template").append(quad);
    	this.workspace[WSID].MIN = screener_min;
    	this.workspace[WSID].MAX = screener_max;
	}

	/*
	 * MENUS
	 */
	QuadMenu(sid,icon,QVID,QSID,QID,WSID){
    	var self = this;
		$(function(){$.contextMenu({
			selector:sid,
			trigger:'left',
			build: function() {
				return {
					items: {
						"Grid": {
							"name": "Grid",
							"items":{
								"Grid1":{"name":"2x2",   callback:function(){self.grid(sid,"grid50")}},
								"Grid2":{"name":"1x1",   callback:function(){self.grid(sid,"grid100")}},
								"Grid3":{"name":"1x2",   callback:function(){self.grid(sid,2)}},
								"Grid4":{"name":"4x4",   callback:function(){self.grid(sid,3)}},
							}},
						"Add": {
							"name": "Add",
							"items":{
								"Add1":{"name": "Chart",      callback:function(){self.QCMenu_LoadTicker()}},
								"Add2":{"name": "WatchTable", callback:function(){self.addWatchtable({send_rpc:1})}},
								"Add3":{"name": "XLS",        callback:function(){loadXLS(0,0,QVID,QSID,QID,WSID)}},
								"Add4":{"name": "Image",      callback:function(){self.QuadMenu_Image()}},
								"Add5":{"name": "PDF",        callback:function(){self.QuadMenu_PDF()}},
								"Add6":{"name": "Columns",    callback:function(){self.QuadMenu_Screener()}},
								"Add7":{"name": "PyScript",   callback:function(){self.QuadMenu_PyScript()}},
								"Add8":{"name": "Xterm",      callback:function(){self.QuadMenu_xterm()}},
								"Add9":{"name": "JTerm",      callback:function(){self.QuadMenu_jterm()}}
							}},						
						}
		            };
		        }
		    });
		});
	}
	QuadspaceMenu(sid, icon, QVID, QSID){
    	var self = this;
		$(function(){$.contextMenu({
			selector:sid,trigger:'left',
			build: function($trigger, e) {return {items: {
						"Grid": {
							"name": "Grid",
							"items":{
								"Grid1":{"name":"2x2",      icon:"2x2",   callback:function(){self.grid(sid,"grid50",   0,QVID,QSID)}},
								"Grid2":{"name":"100%",     icon:"1x1",   callback:function(){self.grid(sid,"grid100",  0,QVID,QSID)}},
								"Grid4":{"name":"Stockpage",icon:"1x2x1", callback:function(){self.grid(sid,"grid1x2x1",0,QVID,QSID)}},
							}},
						"Create":{name:"Create QuadVerse",callback:function($element,key,item){CreateQuadverse(sid)}}}
		            };
		        }
		    });
		});
	}
	/*
	 * Quad Chart Menu: Called by:
	 *   - QuadMenu: to create or reload a chart in a workspace
	 *   - ChartMenu: chart's top left tool menu to reload the current ticker in an existing chart
	 */
	QCMenu_LoadTicker(WSID) {
		var onclick, div, r = 1;
		if (!WSID) {
			WSID = this['ws'+this.current_workspace].id;
			r = 0;
		}
		onclick = 'reloadTicker("' + WSID + '",' + r + ')';
		div = document.createElement('div');
		div.innerHTML = '<input type=text placeholder=Ticker><div class=tload onclick='+onclick+'>Load</div>';
		div.className = "tpop";
		$('body').append(div);
		$(div).attr("style", 'position:absolute;top:'+window.event.clientY+'px;left:'+window.event.clientX+'px;');
		$("input", div).focus();
		$("input", div).keyup(function(event) {
			if (event.keyCode === 13)
				reloadTicker(WSID, r, 1);
		});
	}

	// this['ws'+this.current_workspace].id  (QGID: eg: P0Q1q3ws0)
	QuadMenu_Image()    {upbox(OBJTYPE_IMG,0,this)}
	QuadMenu_PDF()      {upbox(OBJTYPE_DOC,0,this)}
	QuadMenu_PyScript() {upbox(OBJTYPE_APP,ACTION_APP_PYSCRIPT_WORKSPACE,this)}
	QuadMenu_Canvas()   {upbox(OBJTYPE_APP,ACTION_APP_CANVAS_WORKSPACE,this)}
	QuadMenu_xterm()    {new_xterm(this)}
	QuadMenu_jterm()    {new_jterm(this)}
	QuadMenu_Screener() {
		var QGID = this['ws'+this.current_workspace].id;
		bshide("#"+QGID);
		this['screener_workspace'](0,{watchtable:"morphtab"});
	}
	removeTab(tab,rpc) {
		var QVID = this.PID, QSID = tab.QSID_int /* current QuadSpace # (QSID) */, /* Current Quad */QID = tab.QID, WSID = tab.WSID, QGID = "#P"+QVID+"Q"+QSID, click, msg, QSID_new, newtab;
		if (tab === this.activeTabEl) {
			if (newtab=tab.nextElementSibling)
				this.setCurrentTab(newtab);
			else if (newtab=tab.previousElementSibling)
				this.setCurrentTab(newtab);
			console.log("tabswitch current: " + this.current_workspace);
		}
		/* If this QuadSpace contains a stockpage then remove its reference from SP[] */
		for (var v in SP)
			if (SP[v].QTAB == tab)
				delete SP[v];
		/* this.current_workspace will be changed after this */
		tab.parentNode.removeChild(tab);this.emit('tabRemove',{tab});this.cleanUpPreviouslyDraggedTabs();this.layoutTabs();this.setupDraggabilly();
		this.nr_ws--;

		if (tab.QSID) {  /* QuadSpace tab was removed */
			QCACHE[QVID][QSID] = -1;
			Q[QVID].quadspace[QSID] = 0;
			this.idmap[QSID]   = 0;
			if (newtab && window[newtab.click])
				window[newtab.click](QVID,this.current_workspace);
			msg = QUPDATE_QSP_DEL + QVID + " " + QSID + " 0 0 0"; // QSP_DEL RPC
		} else {  /* WorkSpace tab was removed */
			Q[QVID].quadspace[QSID].quad[QID].workspace['ws'+WSID] = 0;
			this.idmap[WSID] = 0;
			msg = QUPDATE_WS_DEL + QVID + " " + QSID + " " + QID + " " + WSID + " 0"; // WS_DEL RPC
			QGID += "q"+QID+"ws"+WSID;
		}
		console.log("removing QGID: " + QGID);
		$(QGID).remove();
		if (!rpc) {
			WS.send(msg);
			if (window.event)
				window.event.stopPropagation();
		}
//		localStorage.QCACHE = JSON.stringify(QCACHE);
	}
	newid(skip){
		var id = 0, found = 0;
		for (var x=0; x<this.idmap.length; x++) {
			if (skip && this.idmap[x] == -1)
				continue;
			if (this.idmap[x] <= 0) {
				id = x;
				found = 1;
				break;
			}
		}
		if (!found) {
			if (this.idmap.length > 0)
				id = this.idmap.length;
			else
				id = 0;
		}
		this.idmap[id] = 1;
		return id;
	}
	/*
	 * ChromeTabs
	 */
    setupEvents()           {window.addEventListener('resize', _ => {this.cleanUpPreviouslyDraggedTabs();this.layoutTabs()});this.tabEls.forEach((tabEl) => this.setTabCloseEventListener(tabEl))}
    setupStyleEl()          {this.styleEl = document.createElement('style');this.el.appendChild(this.styleEl)}
	emit(eventName, data)   {this.el.dispatchEvent(new CustomEvent(eventName, { detail: data }))}
    setupCustomProperties() {this.el.style.setProperty('--tab-content-margin', `${ TAB_CONTENT_MARGIN }px`)}
    get tabEls()            {return Array.prototype.slice.call(this.el.querySelectorAll('.chrome-tab'))}
    get tabContentEl()      {return this.el.querySelector('.wspanel')}
    get tabContentWidths()  {
      const numberOfTabs                  = this.tabEls.length
      const tabsContentWidth              = this.tabContentEl.clientWidth
      const tabsCumulativeOverlappedWidth = (numberOfTabs - 1) * TAB_CONTENT_OVERLAP_DISTANCE
      const targetWidth                   = (tabsContentWidth - (2 * TAB_CONTENT_MARGIN) + tabsCumulativeOverlappedWidth) / numberOfTabs
      const clampedTargetWidth            = Math.max(TAB_CONTENT_MIN_WIDTH, Math.min(TAB_CONTENT_MAX_WIDTH, targetWidth))
      const flooredClampedTargetWidth     = Math.floor(clampedTargetWidth)
      const totalTabsWidthUsingTarget     = (flooredClampedTargetWidth * numberOfTabs) + (2 * TAB_CONTENT_MARGIN) - tabsCumulativeOverlappedWidth
      const totalExtraWidthDueToFlooring  = tabsContentWidth - totalTabsWidthUsingTarget
      const widths = []
      let extraWidthRemaining = totalExtraWidthDueToFlooring
      for (let i = 0; i < numberOfTabs; i += 1) {
        const extraWidth = flooredClampedTargetWidth < TAB_CONTENT_MAX_WIDTH && extraWidthRemaining > 0 ? 1 : 0
        widths.push(flooredClampedTargetWidth + extraWidth)
        if (extraWidthRemaining > 0) extraWidthRemaining -= 1
      }
      return widths
	}
    get tabContentPositions() {
      const positions = []
      const tabContentWidths = this.tabContentWidths
      let position = TAB_CONTENT_MARGIN
      tabContentWidths.forEach((width, i) => {
        const offset = i * TAB_CONTENT_OVERLAP_DISTANCE
        positions.push(position - offset)
        position += width
      })
      return positions
	}
    get tabPositions()        {const positions=[];this.tabContentPositions.forEach((contentPosition) => {positions.push(contentPosition - TAB_CONTENT_MARGIN)});return positions}
    layoutTabs() {
      const tabContentWidths = this.tabContentWidths;
      this.tabEls.forEach((tabEl, i) => {
        const contentWidth = tabContentWidths[i]
        const width = contentWidth + (2 * TAB_CONTENT_MARGIN)
        tabEl.style.width = width + 'px';
        tabEl.removeAttribute('is-small')
// variable length tabs will have to be fixed here dynamically?
//console.log("CONTENT_WIDTH: " + contentWidth);
        if (contentWidth < TAB_SIZE_SMALL) tabEl.setAttribute('is-small', '')
      })
      let styleHTML = '';
      this.tabPositions.forEach((position, i) => {
        styleHTML += `
          .chrome-tabs[data-chrome-tabs-instance-id="${ this.instanceId }"] .chrome-tab:nth-child(${ i + 1 }) {
            transform: translate3d(${ position }px, 0, 0)
          }`
      })
      this.styleEl.innerHTML = styleHTML
    }
	setTabCloseEventListener(tabEl) {tabEl.querySelector('.chrome-tab-close').addEventListener('click', _ => this.removeTab(tabEl))}
    cleanUpPreviouslyDraggedTabs()  {this.tabEls.forEach((tabEl) => tabEl.classList.remove('chrome-tab-was-just-dragged'))}
	get activeTabEl()               {return this.el.querySelector('.chrome-tab[active]')}
    hasActiveTab()                  {return !!this.activeTabEl}
	setupDraggabilly() {
		const tabEls = this.tabEls
		const tabPositions = this.tabPositions
		if(this.isDragging){this.isDragging = false;this.el.classList.remove('chrome-tabs-is-sorting');this.draggabillyDragging.element.classList.remove('chrome-tab-is-dragging');this.draggabillyDragging.element.style.transform='';this.draggabillyDragging.dragEnd();this.draggabillyDragging.isDragging=false;this.draggabillyDragging.positionDrag=noop;this.draggabillyDragging.destroy();this.draggabillyDragging = null;}
		this.draggabillies.forEach(d => d.destroy())
		tabEls.forEach((tabEl, originalIndex) => {
			const originalTabPositionX=tabPositions[originalIndex]
			const draggabilly=new Draggabilly(tabEl,{axis:'x',handle:'.chrome-tab-drag-handle',containment:this.tabContentEl})
			this.draggabillies.push(draggabilly)
			draggabilly.on('pointerDown', _ => {this.setCurrentTab(tabEl)})
			draggabilly.on('dragStart', _ => {
				this.isDragging = true
				this.draggabillyDragging = draggabilly
				tabEl.classList.add('chrome-tab-is-dragging')
				this.el.classList.add('chrome-tabs-is-sorting')
			})
			draggabilly.on('dragEnd', _ => {
				this.isDragging = false;
				const finalTranslateX = parseFloat(tabEl.style.left, 10);
				tabEl.style.transform = `translate3d(0, 0, 0)`
				requestAnimationFrame(_ => {
					tabEl.style.left = '0'
					tabEl.style.transform = `translate3d(${ finalTranslateX }px, 0, 0)`
					requestAnimationFrame(_ => {tabEl.classList.remove('chrome-tab-is-dragging');this.el.classList.remove('chrome-tabs-is-sorting');tabEl.classList.add('chrome-tab-was-just-dragged')
					requestAnimationFrame(_ => {tabEl.style.transform = '';this.layoutTabs();this.setupDraggabilly()})})
				})
			})
			draggabilly.on('dragMove', (event, pointer, moveVector) => {
				const tabEls = this.tabEls
				const currentIndex = tabEls.indexOf(tabEl)
				const currentTabPositionX = originalTabPositionX + moveVector.x
				const destinationIndexTarget = closest(currentTabPositionX, tabPositions)
				const destinationIndex = Math.max(0, Math.min(tabEls.length, destinationIndexTarget))
				if (currentIndex !== destinationIndex)this.animateTabMove(tabEl, currentIndex, destinationIndex)
        	})
		})
	}
    animateTabMove(tabEl, originIndex, destinationIndex) {
      if (destinationIndex < originIndex)
        tabEl.parentNode.insertBefore(tabEl, this.tabEls[destinationIndex])
      else
        tabEl.parentNode.insertBefore(tabEl, this.tabEls[destinationIndex + 1])
      this.emit('tabReorder', { tabEl, originIndex, destinationIndex })
      this.layoutTabs()
	}
	Quad(QVID, QSID, QID, x){
		return `<div id=${QID} class=quad style=display:block>
			<div class="chrome-tabs chrome-tabs-dark-theme" style="--tab-content-margin: 5px" data-chrome-tabs-instance-id="0">
				<div class=wspanel></div>
				<span onclick="W.addBlankspace(${QVID},${QSID},${x})" class=wsnew title="New Workspace">❖</span>
				<span onclick="MaxQuad(${QSID},${x})" class=max title=Maximize>❐</span>
			</div>
		</div>`;
	}
	createNewTabEl(click){
		const div = document.createElement('div');
		div.innerHTML = tabTemplate;
		if(click)
			div.firstElementChild.setAttribute("onclick",click)
		return div.firstElementChild
	}
	ChromeTab(tabProperties,click,bg,QSID_str,QSID_int,QID_int,WSID_int){
		const tab = this.createNewTabEl(click);
		tab.classList.add('chrome-tab-was-just-added');
		setTimeout(() => tab.classList.remove('chrome-tab-was-just-added'),500);
		tabProperties=Object.assign({},defaultTabProperties,tabProperties);
		this.tabContentEl.append(tab);
		this.setTabCloseEventListener(tab);
		tab.QSID     = QSID_str;
		tab.QSID_int = QSID_int;
		tab.QID      = QID_int;
		tab.WSID     = WSID_int;
		this.updateTab(tab,tabProperties);
		this.emit('tabAdd',{tab});
		if (WINIT && !bg)
			this.setCurrentTab(tab,1);
		return tab;
	}
	updateTab(tab, tabProperties) {
		var t           = tab.querySelector('.chrome-tab-title');
		t.textContent   = tabProperties.title;
		if (tabProperties.color)
			$(t).attr("style", "color:" + tabProperties.color + "!important");
		const icon = tab.querySelector('.chrome-tab-favicon');
		if (tabProperties.favicon) {
			icon.removeAttribute('hidden', '');
			if (tab.QSID != "")
				this.QuadspaceMenu(this.qdiv + " .QSP .chrome-tab-favicon",icon, W.PID, tab.QSID_int);
			else
				this.QuadMenu     (this.qdiv + " .ws" + tab.WSID + " .chrome-tab-favicon", icon, W.PID, tab.QSID_int, tab.QID, tab.WSID);
		} else {
			icon.setAttribute('hidden', '');
			icon.removeAttribute('style');
		}
		if (tabProperties.id)
			tab.setAttribute('data-tab-id', tabProperties.id)
	}

	showtab(t,rpc){this.setCurrentTab(t,0,rpc);this.cleanUpPreviouslyDraggedTabs();this.layoutTabs();this.setupDraggabilly()}

	setCurrentTab(tab,bg,rpc) {
		const activeTabEl = this.activeTabEl;
		if (activeTabEl === tab)
			return;
		if(activeTabEl)
			activeTabEl.removeAttribute('active');
		tab.setAttribute('active','');
		this.emit('activeTabChange',{tab});
		if (bg)
			return;

		if (tab.QSID) {
			// rpc qswitch 2 -> workspace::quadspace_switch()
			this.current_quadspace = tab.QSID;
			this.current_workspace = tab.QSID.substr(4);
			$(".quadspace").css("display", "none");
			$(tab.QSID).css("display", "grid");
			if (WINIT && rpc != -1)
				WS.send("qswitch q " + tab.QSID.substr(1));
		} else {
			// rpc qswitch 3 -> workspace::workspace_switch()
			this.current_workspace = tab.WSID;
			this.workspace.find(".ws").css("display", "none");
			this['ws'+tab.WSID].style.display = "grid";
			if (WINIT && rpc != -1)
				WS.send("qswitch w " + tab.QID + " " + tab.WSID);
		}
	}

	qspace(){
		var title = document.getElementById("wstock").value;
		if (title == "")
			title = "QuadSpace " + (this.nr_ws+1);
		var QSID  = this.nr_ws,
		quadspace = this.addQuadspace(title,1,"grid50",4,0,0,1);
		if (!QCACHE[this.PID])
			QCACHE[this.PID] = [];
		quadspace.custom = 1;
		QCACHE[this.PID][QSID] = {title:title, grid:"grid50", quads:[]};
	}
	reflow(quad,WSID){
		var obj = quad.workspace[WSID].obj;
		for (var x = 0; x<obj.length; x++)
			if (obj[x].type == "chart")
				obj[x].ref.reflow();
	}
	grid_QSP(qdiv,g,rpc,QVID,QSID){
		var id = "#P"+QVID+"Q"+QSID, h;
		$(id).attr('class',function(i,c){return c.replace(/(^|\s)grid\S+/g, '');});
		$(id).addClass(g);
		switch (g) {
			case 'grid100': h = '90vh';break;
			case 'grid50':  h = '47vh';break;
		}
		$(id + " .quad")[0].style.minHeight=h;
	}
	grid(qdiv,g,rpc,QVID,QSID){
		qdiv   = qdiv.split(" ")[0];
		if (qdiv.indexOf("qu") != -1)
			return this.grid_QSP(qdiv,g,rpc,QVID,QSID);
		var id = qdiv.replace(/[^0-9]/g, '').split(''), QSID = id[1], QID = id[2], quad = W.quadspace[QSID].quad[QID],ws;
		if (rpc)
			ws = qdiv.substr(qdiv.indexOf('w'));
		else {
			ws = 'ws' + quad.current_workspace;
			qdiv += ws;
		}
		$(qdiv)[0].className = g;
		this.reflow(quad,ws);
		if (!rpc)
			WS.send(QUPDATE_SETGRID+W.PID+" "+QSID+" "+QID+" "+quad.current_workspace+" "+g);
		if (W.quadspace[QSID].custom) {
			QCACHE[W.PID][QSID].quads[QID].workspace[ws.substr(2)].grid=g;
			localStorage.QCACHE = JSON.stringify(QCACHE);
		}
	}
	setrename(GID, QVID, QSID, QID, WSID, title){
		var d = $(GID + " .chrome-tab-content")[0];
		$(GID + " .chrome-tab-content").dblclick(function() {
			if (d.R)
				return;
			d.R = 1;
			var input   = document.createElement("input");
			input.value = title;
			input.focus();
			input.select();
			$(this).append(input);
			input.className = "rename";
			$(input).keyup(function(event) {
		    	if (event.keyCode === 13) {
					var name = input.value;
					$(GID + " .chrome-tab-title").html(name);
					input.remove();
					d.R = 0;
					if (W.quadspace[QSID].custom) {
						WS.send(QUPDATE_WSNAME+QVID+" "+QSID+" "+QID+" "+WSID+" "+name.replaceAll(" ", "^"));
/*						if (WSID == -1)
							QCACHE[QVID][QSID].title=name;
						else
							QCACHE[QVID][QSID].quads[QID].workspace[WSID].title=name;
						localStorage.QCACHE = JSON.stringify(QCACHE);*/
					}
				}
			});
		});
	}
	// currently not in use yet
	qcache_add(QVID,QSID,QID,WSID,title,type,object){
		var QSPACE,QSID_grid,QSID_title,ws,obj=[];
		if (!QCACHE[QVID]) {
			QCACHE[QVID] = [];
			QSID_grid  = $("#P"+QVID+"Q"+QSID)[0].classList[1];
			QSID_title = $("#quadverse"+QVID+" .chrome-tab:nth-child(1) .chrome-tab-title")[0].innerText;
			QCACHE[QVID][QSID] = {title:QSID_title, grid:QSID_grid, quads:[]};
		}
		QSPACE = QCACHE[QVID][QSID];
		if (!QSPACE)
			return;
		obj[0] = {};
		obj[0][type] = object;
		console.log("qcache_add QVID: " + QVID + " QSID: " + QSID + " obj: " + object);
		if (!QSPACE.quads[QID] || QSPACE.quads[QID]==-1) // first workspace
			QSPACE.quads[QID] = {workspace:[{title:title,grid:"grid100",obj:obj}]};
		else {
			ws = QSPACE.quads[QID].workspace;
			if (!ws[WSID]) // new workspace
				ws[WSID] = {title:title,grid:"grid100",obj:obj};
			else // add second/third/etc chart to existing workspace
				ws[WSID].obj.push(obj[0]);
		}
		localStorage.QCACHE = JSON.stringify(QCACHE);
	}
	qcache_del(QSID,QID,WSID,type,object){
		if (!QCACHE[W.PID] || !QCACHE[W.PID][QSID])
			return;
		var workspace = QCACHE[W.PID][QSID].quads[QID].workspace[WSID],obj = workspace.obj;
		for (var x = 0; x<obj.length; x++) {
			if (obj[x][type] == object) {
				obj.splice(x,1);
				localStorage.QCACHE = JSON.stringify(QCACHE);
				return;
			}
		}
	}
	// unfinished
	QBuilder_addConf(id){}empty(){}addMySpace(id){}addGroup(id, args){for (var x = 0; x<args.length; x++)this.workspace['ws'+id].obj.push(args[x])}addFame(args){}addBacktest(args){}addSQL(args){}addSQR(args){}
	}
	return ws
});


/* ***********************************
 *        Quadverse Switcher
 ************************************/
function QUADVERSE_SWITCH(name,c,QVID)
{
	if (name == -1) {
		/* User Clicked on the QuadVerse Menu */
		var quadverse = window.event.target;
		if (quadverse.previousSibling.className == "qtitle")
			name = quadverse.previousSibling.innerText;
		else
			name = quadverse.innerText;
		W = QuadVerses[name];
	} else if (name) {
		W = QuadVerses[name];
	} else {
		W = Q[QVID];
	}
	console.log("switching to: " + name);
	if (c) {
		$("#desktops").slideUp();
		$('.desktop').addClass("animate__backOutLeft");
		$('.qpop').removeClass("clicked-once");
	}
	QUADVERSE.style.display="block";
	$(".quadverse").css("display", "none");
	$("#quadverse"+W.PID).css("display", "block");
	// If the website is fully loaded and we switch to a new quadverse: if it has an init() registered then we call it
	if (WINIT) {
		// rpc qswitch 1 -> workspace::quadverse_switch()
		WS.send("qswitch Q " + W.PID);
		if (W.qinit)
			W.qinit(0);
	}
	W.quadspace[W.current_workspace][0].style.display="grid";

	// relic from static layouts, reloads the table of watchlists in the top-right quad of the Screener QuadSpace
	if (W.PID==0)
		pubload(GW)
}

/* ***********************************
 *        LOAD SCREENER UI
 ************************************/
var SC;
function loadScreener(screener,watchtable_id){
	var self = this;
	$(screener).css("display", "block");
	$(".wmadd", screener).click(function(){var li=$(WLI).clone();li[0].className="";$(".wdst ul", screener).append(li);colmod(screener, 0, "",null,1)});
	$(".wmrem", screener).click(function(){$(WLI).remove();colmod(screener, 0, "", null, 1)});
	$(".wbtn",  screener).click(function(){$(".wbtn", screener).removeClass("on");$(this).addClass("on")});
	/* Sortable Lists */
	$('.sort', screener).each(function(){
		$(this).sortable({
			connectWith:$('.sort').not(this),helper:"clone",revert:"invalid",
			stop:function(event,ui){colmod(screener, watchtable_id, "", null, 1)}
		});
	});
	/* WSAVE RPC */
	$(".wsave", screener).click(function(){
		var preset_name = $(".wnewname",screener).val().replaceAll(" ", "^"), dict;
		if (preset_name == "") {
			$('.wnewname',screener)[0].style.color="red";
			return;
		}
		dict = screener_to_json(screener);
		TP[preset_name] = dict;
		SC = screener;
		var set  = $(".screener-select option:selected", screener).text();
		if (set !== preset_name) {
			TPMenu['Load'+(Object.keys(TPMenu).length+1)]={"name":preset_name,callback:function(itemKey,opt,e){TPLoad(preset_name)}};
			var opt = document.createElement("option");
			opt.appendChild(document.createTextNode(preset_name));
			opt.value = Object.keys(TP).length;
			$(".screener-select", screener).append(opt);
			$(".screener-select", screener).val(Object.keys(TP).length);
		}
		colmod(screener, 0, preset_name, null, 2); // XLS_reload() because save==2
	});
}

function workspace_showtab(QSID, QID, ws) {
	var q = W.quadspace[QSID].quad[QID];
	q.showtab(q.workspace[ws].tab)
}

function quadspace_load(QVID,QSID,quads) {
	var JQuadverse,JQuadspace,JWorkspace,QID,ws,WSID,QGID,quadverse,quadspace,workspace,quad,quads,obj,objsize,grid,URL,url,img;
	quadspace = Q[QVID].quadspace[QSID];
	if (QCACHE[QVID] && QCACHE[QVID][QSID].sp) { // stockpage
		quadspace[0].className = "quadspace opgrid stockpage";
		return stockpage(QCACHE[QVID][QSID].title,QVID);
	}
	if (quadspace.loaded == 1)
		return;
	if (!quads && !QCACHE[QVID])
		return;
	if (!quads)
		quads = QCACHE[QVID][QSID].quads;
	quadverse = Q[QVID];
	for (var QID = 0; QID<quads.length; QID++) {
		quad = quads[QID];
		if (!quad) {
			quadspace.quad[QID].addBlankspace(QVID, QSID, QID);
			continue;
		}
		workspace = quad.workspace;
		if (!workspace.length) {
			quadspace.quad[QID].addBlankspace(QVID, QSID, QID);
			continue;
		}
		console.log("quadspace_load QSID: " + QSID + " QID: " + QID + " workspace len: " + workspace.length + " workspace: " + workspace);
		for (var ws=0; ws<workspace.length; ws++) {
			if (workspace[ws] == "-1")
				continue;
			grid = workspace[ws].grid;
			obj  = workspace[ws].obj;
			if (obj) {
				objsize = obj.length;
				QGID    = "P"+QVID+"Q"+QSID+"q"+QID+"ws"+ws;
//				console.log("quadspace_load: QGID: " + QGID + " objLEN: " + obj.length + " workspaceTitle: " + workspace[ws].title);
				if (!objsize) {
					quadspace.quad[QID].addBlankspace(QVID, QSID, QID);
					continue;
				}
			} else
				objsize = 0;
			WSID = quadverse.quadspace[QSID].quad[QID].addWorkspace({title:workspace[ws].title,favicon:true},{background:true,click:workspace[ws].click},grid?grid:"",QSID,QID,-1);
			for (var x=0; x<objsize; x++) {
				if (obj[x].chart) {
					/*********
					* Charts *
					*********/
					WS.send("chart " + QGID + " " + obj[x].chart.split("&")[0]);
					$("#"+QGID).append("<div id=load class=loading></div>");
				} else if (obj[x].img) {
					/*********
					* Images *
					*********/
					img           = new Image();
					URL           = obj[x].img;
					url           = URL.split(":")[0];
					img.src       = "/blob/"+url;
					img.className = "ws-img o" + url;
					newobj(img,"#"+QGID,URL,'img',1);
				} else if (obj[x].wstab) {
					/*********
					* Tables *
					*********/
					var wstab = obj[x].wstab.split(":"), watchlist;
					W.addWatchtable({QGID:"#"+QGID, TID:wstab[0],send_rpc:0});
					console.log("adding Watchtable: " + wstab[1]);
					watchlist = Watchlist(wstab[1]);
					if (!watchlist)
						watchlist = GWatchlist(wstab[1]);
					if (watchlist)
						load(wstab[1],wstab[0],QGID,watchlist.id,watchlist.pub);
				} else if (obj[x].pdf) {
					/*********
					*  PDFs  *
					*********/
					URL = obj[x].pdf;
					url = URL.split(":")[0];
					obj = $('<object class="ws-pdf o' + url + '" data=/blob/'+URL+' type="application/pdf" width="100%" height="40vh">')[0];
					newobj(obj,"#"+QGID,URL,'pdf',1);
				}
			}
			JQuadverse = WEBSITE.QuadVerse[QVID];
			if (!JQuadverse)
				continue;
			JQuadspace = JQuadverse.QuadSpace[QSID]
			if (!JQuadspace)
				continue;
			JWorkspace = JQuadspace.quads[QID].workspace[ws];
			if (JWorkspace && JWorkspace.mod)
				quadspace.quad[QID][JWorkspace.mod](JWorkspace.args);

		}
	}
	quadspace.loaded = 1;
	if ((QVID==3 && QSID == 0) || JWorkspace)
		return;
	for (var x = quads.length; x<4; x++) {
		quad = quadspace.quad[x];
		quad.addBlankspace(QVID, QSID, x);
	}
}

/********************************************************************
 * ONCLICK handlers for specific (built-in) QuadSpaces & Workspaces
 *******************************************************************/
function ufo() {
	if (UINIT)
		return;
	UINIT=1;
	WS.send("ufoinit PL1 15 "+W.current_quadspace+"q0ws0")
}
function vfo() {
	if (VINIT)
		return;
	VINIT=1;
	WS.send("ufoinit VL1 15 "+W.current_quadspace+"q0ws0")
}

/* QuadVerse INIT */
function screener()    {pubload(GW)}
function options()     {WS.send("options")}
function profile()     {WS.send("profile")}
function candles()     {WS.send("candle AAPL")}
function chartspace()  {
	quadspace_load(W.PID,0,WEBSITE.QuadVerse[W.PID].QuadSpace[0].quads)
}
function cryptoverse() {
	quadspace_load(W.PID,0,WEBSITE.QuadVerse[W.PID].QuadSpace[0].quads)
}
