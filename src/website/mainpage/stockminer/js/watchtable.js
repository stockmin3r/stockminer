/*************************************************
 * filename: mainpage/stockminer/js/watchtable.js
 * License:  Public Domain
 ************************************************/
var prOFF = '<label class=cbx><input type=checkbox class=checkbox_input><span class=checkbox_ctrl><span class="check untick" onclick=pract()>&check;</span></span></label>';
var prON  = '<label class=cbx><input type=checkbox class=checkbox_input checked><span class=checkbox_ctrl><span class="check tick" onclick=pract()>&check;</span></span></label>';
var WARGS;
function Watchtable(args) {
	var tab, columns = args.dict, menu = args.menu, TID = args.TID, QGID = args.QGID, edit = args.edit, order = args.order, type = args.type,c,
	t   = 'class="MTAB dataTable"><caption></caption><thead><tr>',
	spn = '<span class="tlink link-cyan" title="Menu">⚙</span>';

	WARGS=args;

	console.log("watchtable TID: "+ TID);
	if (!TID)
		TID = "T"+randstr(6);
	DICT = columns;
	if (columns != null) {
		t += "<th>" + spn + "</th>";
		for (var x = menu?1:0; x<columns.length; x++) {
			c = columns[x].data;
			t += "<th t="+c+">"+WCOL2[c]+"</th>";
		}
	} else
		t += "<th>" + spn + "</th><th>Ticker</th><th>Price</th><th>Delta</th><th>Volume</th>";
	t += "</tr></thead></table>";

	tab = $('<table id=' + TID + " " + t)[0];
	$(QGID).append(tab);
	tab.classList.add("WBOX");
	if (args.left != "*") {
		tab.style.left = args.left;
		tab.style.top  = args.top;
	}

	newtable(TID, "desc", columns, order?order:3);
	tab.global = 0;
	tab.desc   = 1;
	tab.asc    = 0;
	tab.b      = tab.parentNode.removeChild(tab.parentNode.firstChild);
	tab.parentNode.parentNode.replaceChild(tab,tab.parentNode);

	/* ************************
	 * WATCHTABLE GLOBAL SORT
	 *************************/
	$("#"+TID).on('click', 'thead th', function(){
		var idx = T[TID].column(this).index(),x;
		if (!idx)
			return true;
		console.log("GSORT ATTR: " + $(this).attr("v")  + " " + $("#"+TID)[0].global + " idx: " + idx);
		if (tab.desc) {
			tab.desc=0;
			tab.asc=1;
			T[TID].order([idx, 'desc']).draw();
			x = 'D';
		} else {
			T[TID].order([idx, 'asc']).draw()
			tab.desc=1;
			tab.asc=0;
			x = 'U';
		}
		console.log("index: " + idx + " desc: " + tab.desc + " asc: " + tab.asc);
		if (tab.global)
			WS.send("gsort " + $(this).attr("v") + " " + TID + " " + x);
	});
	menu = "#"+TID + " .tlink";
	switch (type) {
		case 'watchlist':
			WatchTable_Menu(menu, TID, QGID);
			WatchTable_RClick_Menu(TID);
			break;
		case 'XLS':
			XLS_Menu(menu, TID, QGID);
			break;
	}

	$(tab).draggable();
	$(tab).droppable({
		accept:".wsrc li",
		drop:function(event, ui){
			TGT = event.target;
			UI = ui;
			console.log("drop: " + ui.draggable.text());
			$(event.target).val(ui.draggable.text());
		}
	});
	if (edit) {
		$("#"+TID + ' tbody').on('click', 'tr td', function(e){
			if (this.className == "dataTables_empty")
				return;
			var td = this, column = td.cellIndex+1, columnID;
			if (!$("#"+TID)[0].obj.watchlist)
				return;
			columnID = $("#"+TID + " thead th:nth-child(" + column + ")")[0].attributes['t'].nodeValue;
			if (ECELL[columnID] == undefined || td.innerHTML.indexOf("<input") != -1)
				return;
			edit_alert_update(td.innerText, watchlist, 0, td, td.parentNode.rowIndex-1);
		});
	}
	if (editor) { // XXX: WTF
		$("#"+TID + ' thead').on('click', 'th td', function(e){
			if (!editor)
				return;
			var td = this, column = td.cellIndex+1;
			debugger;
		});
	}
	return tab;
}

function datatable(args)
{
	return T[args.id] = $("#"+args.id).DataTable({
		columns:args.columns,
		columnDefs:[{"targets": args.ncol,"visible":false}],
    	"order":[[args.order,args.slope]],"bPaginate":false,"paging":false,"bDestroy":false,"info":false,"searching":false});
}

function newtable(id,ord,col,o){
	T[id] = $("#"+id).DataTable({
		dom: 'Bfrtip',
		buttons: [{
			extend:'excelHtml5',fieldBoundary: '',exportOptions:{columns: 'th:not(:first-child)'}
		}],
//      buttons:['copyHtml5','excelHtml5','pdfHtml5'],
		columns:col,columnDefs:[{targets:col.length,visible:false},{width:"34px",targets:[0]}],
		rowCallback:function(row,data,index){tableColors(row,data,index,"#"+id+" thead tr",0)},
		bPaginate:false,order:[[o,ord]],paging:false,info:false,searching:false
		});
	$("#"+id+" tbody").on('click','tr',function(){
        $(".highlight", this.parentNode).removeClass("highlight");
        $(this)[0].className += " highlight";
	});
	return T[id];
}

// Load the Screener's right section with the columns associated with this Column Preset dict */
function wdict_load(screener, dict)
{
	var wdst = $(".wdst ul", screener),
		wsrc = $("body > .screener .wsrc ul li");
	$("li", wdst).remove();
	for (var x = 1; x<dict.length; x++) {
		var column_id = dict[x].data, li; // v=900, v=901
		$(wsrc).each(function(){
			if ($(this).attr('v') == column_id) {
				li = $(this).clone();
				return false;
			}
		});
		$(wdst).append(li);
	}
}

/*
 * Loop through each of the <li> elements of the Screener's left box
 */
function init_stock_screener(){
	$(".watchmgr .wsrc li").each(function(){
		var c    = $(this).attr('v');
		this.setAttribute("onclick", "boxtick()");
		WCOL[c]  = $(this).text();
		WCOL2[c] = $(this).text().split(" ")[0]
	});
	$(".showtime .wsrc li").each(function(){this.setAttribute("onclick", "boxtick()")});
	$(".watchmgr .links").hover(function(){$(".links-menu").css("display","block")})
}

/*
 * Return a JSON array of dicts
 *  - constructs an array of "data":"Column Name" dicts
 */
function screener_to_json(screener)
{
	var d = '[{"data":null,"orderable":false,"defaultContent":""},';
	$(".wdst li", screener).each(function(){d += '{"data":"'+$(this).attr('v') + '"},';});
	d  = d.slice(0, -1);
	d += "]";
	return d;
}

/*
 * 1) Add columns to New^Preset, Remove columns from New^Preset (no change to selectbox)
 * 2) Save a New Preset (set selectbox to new preset) (name is passed: New^Name)
 * 3) Select Box onchange() to a different preset - Set .wmname
 * 4) Menu -> Load Preset - Set .wnewname
 * 5) Clear a Preset
 * 6) Delete a preset
 */
function wtabselect(screener)
{
	var dict, P, preset_name, watchtable_id;

	if (!screener)
		screener = window.event.target.parentNode.parentNode.parentNode.screener;

	if (screener.type == 'stocks')
		P = TP;
	else if (screener.type == 'options')
		P = OP;

	preset_name = $(".screener-select option:selected",screener).text();
	if (preset_name == "New Preset") {
		$(".wnewname", screener)[0].placeholder = preset_name;
		$(".wsave",    screener)[0].style.color="grey";
		$(".wnewname", screener).val("");
	} else
		$(".wnewname", screener)[0].value = preset_name;
	if (P[preset_name] == undefined)
		dict = STAB;
	else
		dict = JSON.parse(P[preset_name]);

	for (var x=0; x<screener.curobj.length; x++) {
		watchtable_id      = screener.curobj[x].id;
		screener.curobj[x] = realloc_watchtable(watchtable_id, dict, screener);
		wdict_load(screener, dict);
		WS.send("TPLoad " + preset_name.replaceAll(" ", "^") + " " + watchtable_id);
		console.log("checkin watchtable: " + watchtable_id);
	}
}

/* Change Category of columns */
function wcat(x,y){
	if (x == -1)
		$(".wsrc li").css("display", "block");
	else
		$(".wsrc li").css("display", "block"); // ?!?!
	$(".wsrc li").each(function(){
		var i = $(this).attr("v");
		if (i < x || i > y)
			$(this).css("display", "none")
	});
}

function realloc_watchtable(watchtable_id, columns, screener)
{
	var watchtable = $("#"+watchtable_id)[0], obj = watchtable.obj, QGID = "#"+watchtable.parentNode.id;
	T[watchtable_id].destroy(1);
	watchtable.remove();
	watchtable     = Watchtable({type:'watchlist',dict:columns,QGID:QGID,TID:watchtable_id,menu:1,order:1});
	watchtable.obj = obj;
	obj.screener   = screener;
	$("#" + watchtable_id + " caption").text(obj.watchlist);
	return watchtable;
}

/* CALLERS:
 *  - TPLoad()       - WatchTable Preset Loader   - watchtable.js
 *  - loadScreener() - init of default #morphtab  - quadspace.js - XXX: must remove the static morphtab
 *      + (add column onclick button callback)
 *      + (del column onclick button callback)
 *      + .sort .sortable()
 *      + .wsave onclick when saving a Preset
 *  - wtrash()       - Trashcan icon click to reset #morphtab
 *  - sh_call_colmod - Scripting
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * screener:      Column Screener
 * watchtable_id: Watchtable linked to Column Screener
 * preset_name:   Column Preset Name
 * d:             dict of watchtable's Columns
 * save:          should the table preset be saved (RPC call)
 * --------------------------------------------------------
 */
function colmod(screener, watchtable_id, preset_name, d, save) {

	for (var x=0; x<screener.curobj.length; x++) {
		var watchtable = screener.curobj[x];
		if (watchtable_id && watchtable.id != watchtable_id)
			continue;

		if (!d)
			d = screener_to_json(screener);
		if (preset_name == "" || preset_name == undefined)
			preset_name = $(".screener-select option:selected", screener).text().replaceAll(" ", "^");
		else {
			/* Called by .wsave click handler, onchange(), TPLoad() */
			SetSelect($(".screener-select", screener), preset_name);
			$(".wnewname",screener)[0].style.color = "";
		}
		// set new preset for this watchtable
		screener.curobj[x] = realloc_watchtable(watchtable.id, JSON.parse(d), screener);
		if (save == NO_RPC)
			return;

		wsave = $(".wsave", screener)[0];
		if (save==1 || save==2) {
			WS.send("watchtable " + watchtable_id + " " + d + " " + preset_name + " " + $(".wdst li", screener).length);
			wsave.style.color = save==1 ? "limegreen" : "green";
			wsave.innerText   = save==1 ? "Save"      : "Saved";
			if (save==2) {
				var obj = {preset:preset_name};
				MODCALL("XLS", 'reload', screener.quadspace, ['any', obj], obj);
			}
		} else {
			wsave.style.color = "grey";
			wsave.innerText   = "Save";
		}
	}
}

function CUROBJ(screener, watchtable)
{
	for (x=0; x<screener.curobj.length; x++)
		if (screener.curobj[x] == watchtable)
			return 1;
	return 0;
}

function TPLoad(preset_name) {
	var watchtable_id = $(window.event.target).closest(".context-menu-root")[0].classList[1].split("-")[1], watchtable = $("#"+watchtable_id)[0], obj = watchtable.obj, screener;

	screener   = watchtable.obj.screener;
	if (obj.controller) {
		/* controller */
		obj.preset = preset_name;
		switch (obj.type) {
			case 'XLS':
				XLS_reload(obj);
				break;
		}
	} else if (!screener || !CUROBJ(screener,watchtable)) {
		realloc_watchtable(watchtable_id, JSON.parse(TP[preset_name]), 0);
		console.log("no screener");
	} else {
		console.log("got TPload");
		colmod(screener,watchtable_id,preset_name,TP[preset_name],0);
	}

	WS.send("TPLoad " + name.replaceAll(" ", "^") + " " + watchtable_id);
	if (screener) {
		$(".wnewname",screener)[0].value = preset_name;
		wdict_load(screener, JSON.parse(TP[preset_name]))
	}
}

/* Called by:
 *  - [1] .XLS-select      A Screener's Preset Name <select> (has controller) (ALTER WATCHTABLE)
 *  - [2] addXLS()         blankspace XLS image onclick      (has controller) (CREATE NEW||EMPTY WATCHTABLE)
 *  - [3] <span .XLS-load> XLS-controller's button           (no controller)  (ALTER WATCHTABLE)
 *  - [4] QuadMenu                                           (no controller)  (CREATE NEW|EMPTY)
 */
function loadXLS(controller,obj,QVID,QSID,QID,WSID) {
	var controller,ticker,preset,rows,quad,QGID,columns,XLS,t,title,fromQuadMenu=0;
	if (!obj || !obj.preset) {
		if (!obj)
			obj = {};
		/* [3] we were called by XLS-controller's Load XLS button onclick code which means a table has already been created previously */
		/* [4] QuadMenu */
		if (!obj.QGID) {
			if (QVID!=undefined) {
				/* [4] QuadMenu */
				WSID = 'ws'+WSID;
				quad = Q[QVID].quadspace[QSID].quad[QID];
				obj  = quad.workspace[WSID].obj;
				QGID = "#P"+QVID+"Q"+QSID+"q"+QID+WSID;
				controller = get_WS_OBJ('XLS-controller', obj);
				if (!controller)
					controller = quad.addXLS_controller(QGID, WSID, {});
				fromQuadMenu = 1;
			} else {
				/* [3] onclick() Load XLS button */
				controller = window.event.target.parentNode;
				QGID = "#"+controller.parentNode.parentNode.id;
			}
		} else
			QGID = obj.QGID;

		console.log("QGID: " + obj.QGID + " ticker: " + obj.ticker);
		if (!obj.ticker) {
			t = $(".XLS-ticker", controller)[0].value;
			if (t != "")
				ticker = t;
		} else
			ticker = obj.ticker;

		// GUI extract preset/ticker/row from XLS-controller
		preset = $(".XLS-presets option:selected", controller).text();
		if (!preset)
			preset = 'New^Preset';
		console.log("PresetName: " + preset + " QGID: " + obj.QGID);
		rows = $(".XLS-rows-input", controller).val();
		if (rows == '')
			rows = 16;
	} else {
		preset = obj.preset;
		rows   = obj.rows;
		ticker = obj.ticker;
		QGID   = obj.QGID;
	}

	// Create a new XLS table
	if (!controller.curobj.length || fromQuadMenu) {
		if (preset == 'New^Preset') // XXX: STATIC
			columns = STAB;
		else
			columns = JSON.parse(TP[preset]);
		if (ticker != undefined)
			title = ticker+" "+preset;
		else
			title = preset;
		obj  = {type:"XLS",ticker:ticker,preset:preset,rows:rows,QGID:QGID,title:title,controller:controller};
		XLS  = Watchtable({type:"XLS",dict:columns,QGID:QGID,TID:0,menu:1,order:1});
		controller.curobj.push(XLS);
		$("#"+XLS.id)[0].classList.replace("MTAB", "XLS");
		$("#"+XLS.id + " caption").text(obj.title.replaceAll("^", " "));
		obj.ref = XLS;
		obj.TID = XLS.id;
		XLS.obj = obj;
		quad    = $(QGID)[0].w;
		quad.workspace['ws'+quad.current_workspace].obj.push(obj);
		XLS.onclick = function(){selobj($(XLS)[0])};
		return
	}

	for (var x = 0; x<controller.curobj.length; x++) {
		var xobj = controller.curobj[x].obj;
		if (xobj.preset == preset && xobj.rows == rows && xobj.ticker == ticker && xobj.QGID == QGID)
			continue;
		if (preset != "*")
			xobj.preset = preset;
		if (rows != "*")
			xobj.rows   = rows;
		if (ticker != "*")
			xobj.ticker = ticker;
		xobj.QGID   = QGID;
		XLS_CALLBACKS['reload'](xobj)
	}
}

function XLS_reload(obj)
{
	var preset_name = obj.preset, XLS, p, left = "*", top = "*";
	if (obj.ref) {
		p = obj.ref;
		left = p.style.left;
		top  = p.style.top;
		p.remove();
	}
	XLS      = Watchtable({type:"XLS",dict:JSON.parse(TP[preset_name]),QGID:obj.QGID,TID:obj.TID,menu:1,order:1,left:left,top:top});
	obj.ref  = XLS;
	XLS.obj  = obj;
	obj.TID  = XLS.id;
	if (!obj.ticker)
		obj.title = obj.preset;
	else
		obj.title = obj.ticker + ": " + obj.preset;

	$("#"+obj.TID)[0].classList.replace("MTAB", "XLS");
	$("#"+obj.TID + " caption").text(obj.title);

	if (obj.ticker)
		WS.send("XLS " + obj.ticker + " ws " + obj.rows + " " + preset_name + " " + obj.QGID + " " + obj.TID)
}

function XLS_loadTicker_onclick()
{
	var xload = $("#XLS-load-ticker")[0], obj = xload.obj;
	xload.style.display='none';
	obj.ticker = $("input", xload).val();
	XLS_CALLBACKS['reload'](xload.obj);
}

function XLS_loadTicker(watchtable,QGID)
{
	var xload = $("#XLS-load-ticker").detach();
	xload.css("display","block");
	xload[0].obj = $("#"+watchtable)[0].obj;
	$('body').append(xload);
	$(xload).attr("style", 'position:absolute;top:'+window.event.clientY+'px;left:'+window.event.clientX+'px;');
}

/* Short Column Name -> Long Column Name */
function colmap(c)
{
	var columns = Object.values(WCOL2);
	for (var x = 0; x<columns.length; x++)
		if (columns[x] == c)
			return Object.keys(WCOL2)[x];
}

function wtrash()
{
	var screener = window.event.target.parentNode.parentNode.screener;
	$(".wdst", screener).remove();
	$(".wsrc", screener).remove();
	var wmid = $(screener).find(".wmid").detach();
	$(screener).append($('body > .watchmgr .wsrc').clone())
	$(screener).append(wmid);
	$(screener).append($('body > .watchmgr .wdst').clone());
	colmod(screener, 0, "", null, 1); // reset watchtable
}

function wbomb(screener)
{
	var preset_name;
	if (!screener)
		screener = window.event.target.parentNode.parentNode.screener;
	preset_name = $(".screener-select option:selected",screener).text();
	if (preset_name == 'New Preset')
		return;
	WS.send("wbomb "+preset_name)
	$(".screener-select option:selected",screener).remove();
	wtabselect(screener);
}

function bulb(){
	var n = window.event.target,id,node,title,cap,tab;
	node  = n; // n is the span clicked on
	while (node.tagName != "TABLE")
		node = node.parentNode;

	id  = "#"+node.id; // tid table-id: morphtab or Trandom
	tab = $(id)[0];
	cap = $(id + " caption");
	if (tab.wname != "")
		title = tab.wname;
	else {
		title = cap.text();
		tab.wname = title;
	}
	if (n.classList.contains("grn")) {
		n.className = "bulb blue";
		n.title     = "Click to sort by only stocks in the table currently (Watchlist)";
		tab.global  = 1;
		cap.html("Screener: " + title);
	} else {
		n.className = "bulb grn";
		n.title     = "Click to Sort by ALL Stocks (Screener)";
		tab.global  = 0;
		cap.html("Watchlist: " + title);
	}
	event.stopPropagation();
}

function showlinks()
{
	QGID = "#"+window.event.target.parentNode.parentNode.parentNode.id;

	var links = $("#links").detach();
	links.css("display", "block");
	$(QGID).append(links);
}

function SetSelect(SELECT, text)
{
	$("option",SELECT).each(function(){
		if (this.text == text) {
			$(SELECT).val(this.value);
			return;
		}
	});
}

// Table Preset RPC: tpset My^New^Preset-[{data:T,data:P}]
function rpc_TPset(av)
{
	var tset  = av[1].split("-"),opt,
	name      = tset[0].replaceAll("^"," ");
	TP[name]  = tset[1];
	opt       = document.createElement("option");
	opt.value = Object.keys(TP).length;
	opt.appendChild(document.createTextNode(name));
	$(".screener-select").append(opt);
	TPMenu['Load'+(Object.keys(TPMenu).length+1)] = {"name":name, callback:function(){TPLoad(name)}};
//	TPMenu['Load'+(Object.keys(TPMenu).length+1)] = {"name":name, callback:function(i){TPLoad(TP[i.substr(4)-1])}};
}

/* wget url watchtable_id, QGID */
function wget_url_onclick()
{
	var wget = $("#wget-url")[0], URL = $("input", wget).val(), QGID = wget.QGID;
	WS.send("wget " + URL + " " + wget.watchtable_id + " " + QGID);
	wget.style.display='none';
	wget = $("#wget-import")[0];
	wget.URL = URL;
	wget.QGID = QGID;
	wget.style.display='block';
	$(wget).append("<div id=load class=loading></div>");
}

function GUI_wget_url(watchtable_id, QGID)
{
	var wget = $("#wget-url")[0];
	wget.watchtable_id = watchtable_id;
	wget.QGID = QGID;
	wget.style.display="block";
}

/* receives list of table names served by a URL */
function rpc_wget(av)
{
	var a,x,rows,name,c=' checked',
	wget = $("#wget-import"),
	e    = $(".uentries", wget),
	URL  = wget[0].URL;
	$("#load").remove();
	for (x=0; x<av.length-1; x++) {
		var a = av[x+1].split("`"), name = a[0].replaceAll("^", " "), rows = a[1];
		console.log("name: " + name);
		e.append("<div class=uentry><input type=checkbox" + c + "><span class=uname>" + name + "</span><span class=uinfo>" + rows + " Rows</span></div>");
		c='';
	}
}

var WGET;
function wget_import_onclickRPC()
{
	var wget = $("#wget-import")[0], URL = wget.URL, msg = 'deftab ' + URL + " " + wget.QGID + " ";
	wget = $(".uentry", wget);WGET=wget;
	for (var x = 0; x<wget.length; x++)
		if (wget[x].firstChild.checked)
			msg += wget[x].children[1].innerText.replaceAll(" ", "^") + "`";
	WS.send(msg);
}

function copytable(dt)
{
	var rows = dt.cells().rows.data(), val = Object.values(rows), key = Object.keys(key), t = '', kl = key.length;

	for (var x = 0; x<kl; x++)
		t += WCOL[key[x]] + " ";
	t += "\n";
	for (var x = 0; x<rows.length; x++) {
		row = rows[x];
		for (var y = 0; y<kl; y++)
			t += row[x] + " ";
	}
	return t;
}

var AV,CC,DD;
/* QGID`tableFormat`columns(def)~rows(data)`columns(def)~rows(data) */
/* QGID`dict`1000|Company|1001|Sector~[{1000:RR},{1001:Telco}]`2000|Gene|2001|Organs~[{2000:BDNF},{2001:Brain}] */
// dt = { QGID: "P0Q0q0ws0", tables: [{tableFormat:"dict", columns: [{2000:"Gene"},{2001:"Organs"}] data: [{2000:"BDNF",2001:"Brain"}]}, {tableFormat:"csv", csv: "Telco,Gene\n"}] } 


function rpc_deftab2(av)
{


}

function rpc_deftab(av)
{
	var tables = av.split("`"), TID = $("#wget-url")[0].watchtable_id, QGID = tables[0], columns = [{"orderable":false,"data":null,"defaultContent":''}], w;

	if (T[TID]) {
		w = $("#"+TID)[0];
		T[TID].destroy(1);
		w.remove();
	}
	for (var x=1; x<tables.length; x++) {
		var t = tables[x].split("~"), col = t[0].split("|"), data = t[1];
		// define the columns for this table
		for (var y=0; y<col.length; y+=2) {
			WCOL[col[y]]  = col[y] + " - " + col[y+1];
			WCOL2[col[y]] = col[y+1];
			columns.push({"data":col[y]});
		}
		CC  = columns;
		DD  = data;
		TID = Watchtable({type:'www',TID:TID,QGID:QGID,dict:columns,menu:1,order:1}).id;
		console.log("created: " + TID);
		T[TID].clear().rows.add(JSON.parse(data)).draw();
		TID = 0;
		columns.length = 1;
	}
}

/*
 * Save a HTML table in CSV format as a file
 */
function table_export_csv(watchtable_id)
{
	var link     = document.createElement("a");
	var textarea = document.getElementById("copypaste-textarea");
	var table    = $("#"+watchtable_id)[0];
	var csv      = "";
	var caption  = table.caption.innerText;

	for (var x = 0; x<table.rows.length; x++) {
		var cells = table.rows[x].cells;
		for (var y = 0; y<cells.length; y++) {
			var cell = cells[y].innerText;
			if (cell == "⚙" || cell == "")
				continue;
			csv += cell + ",";
		}
		csv += "\r\n";
	}
	textarea.innerHTML = csv;

	var content        = textarea.value+"\r\n";
	var file           = new Blob([content], {type:'text/plain'});
	link.href          = URL.createObjectURL(file);
	link.download      = caption?caption:"watchtable.csv";
	link.click();
	URL.revokeObjectURL(link.href);
}

function table_import(TID,input)
{
	switch (input) {
		case 1:
			// Excel
			break;
		case 2:
			// CSV
			break;
		case 3:
			// TXT
			break;
		case 4:
			// PASTE
			break;
	}
}

function table_export(watchtable_id,output)
{
	switch (output) {
		case 1:
			T[TID].buttons(".buttons-excel").trigger();
			break;
		case 2:
			table_export_csv(watchtable_id);
			break;
		case 3:
			break;
	}
}

function tabview(tab, i)
{
	var div = document.createElement("div"), t;
	if (i == 0)
		t = '0';
	else
		t = '1';
	div.className = "tabview";
	div.innerHTML = "<div class=itab onclick=rload(1," + t + ")>15m Interval</div><div class=itab onclick=rload(2," + t + ")>5m Interval</div><div class=itab onclick=rload(3," + t + ")>1m Interval</div>";
	console.log("html: " + div.innerHTML);
	$(window.event.target.parentNode).prepend(div);
	$(div).animate({height:"90"});
	$(div).mouseleave(function(){div.remove()});
}

function col(i,c)
{
	COL[i][c] = window.event.target.value;
	for (var k in charts) {
		var c = charts[k],
		s     = c.get(c.title.textStr + "-price-" + i);
	}
}

/*
 * UFO Screener
 */
function ufoload(){
	var wlist = $("#ufo-select option:selected").text(),
	wsx  = document.getElementById(window.event.target.parentNode.parentNode.id),
	QGID = W.current_quadspace+"q0ws0",
	ufo  = $("#ufo").detach();
	console.log("ufoload!@");
	while (wsx.firstChild)
		wsx.removeChild(wsx.firstChild);
	if (wlist === "Highcaps") {
		if ($(QGID)[0].UFO_TYPE == UFO_PRICE) {
			T['R15'].destroy(1);
			T['R5'].destroy(1);
			T['R1'].destroy(1);
			T['R15mL'].destroy(1);
			T['R5mL'].destroy(1);
			T['R1mL'].destroy(1);
			console.log("doing ufo init");
			ufoinit(MTAB, QGID.substr(1));
			$(QGID).append(ufo);
			WS.send("ufoinit PH1 15 "+W.current_quadspace+"q0ws0")
		}
	}
}

function ufoinit(tab, id){ // id comes from addUFO() this['wsX'].id (QGID) via LoadQuadverse()
	var t  = 'class="GTAB MTAB cell-border dataTable"><caption onclick=ufocharts("'+id+'",', t2 = '</caption><thead><tr>', R;

	for (var x = 0; x<tab.length; x++)
		t2 += "<th>" + WCOL[Object.values(tab[x])[0]] + "</th>";
	t2 += "</tr></thead></table>";

	R    = document.createElement("div");
	R.id = "R15box";
	R.innerHTML = '<table id=R15 ' + t + '15,1)>' + '15m Gainers' + t2 + '<table id=R15mL ' + t + '15,2)>' + '15m Losers' + t2;
	$("#"+id).append(R);

	R    = document.createElement("div");
	R.id = "R5box";
	R.innerHTML  = '<table id=R5 ' + t + '5,1)>'  + '5m Gainers'  + t2 + '<table id=R5mL '  + t + '5,2)>'  + '5m Losers'  + t2;
	$("#"+id).append(R);

	R    = document.createElement("div");
	R.id = "R1box";
	R.innerHTML  = '<table id=R1 ' + t + '1,1)>'  + '1m Gainers'  + t2 + '<table id=R1mL '  + t + '1,2)>'  + '1m Losers'  + t2;	
	$("#"+id).append(R);

	newtable("R15",   "desc", tab, 2,3);
	newtable("R5",    "desc", tab, 2,3);
	newtable("R1",    "desc", tab, 2,3);
	newtable("R15mL", "asc",  tab, 2,3);
	newtable("R5mL",  "asc",  tab, 2,3);
	newtable("R1mL",  "asc",  tab, 2,3);

	$('#R15 tbody').on  ('click', 'tr', function(){ufochart(T['R15'].row  (this).data()['900'],id)});
	$('#R5 tbody').on   ('click', 'tr', function(){ufochart(T['R5'].row   (this).data()['900'],id)});
	$('#R1 tbody').on   ('click', 'tr', function(){ufochart(T['R1'].row   (this).data()['900'],id)});
	$('#R15mL tbody').on('click', 'tr', function(){ufochart(T['R15mL'].row(this).data()['900'],id)});
	$('#R5mL tbody').on ('click', 'tr', function(){ufochart(T['R5mL'].row (this).data()['900'],id)});
	$('#R1mL tbody').on ('click', 'tr', function(){ufochart(T['R1mL'].row (this).data()['900'],id)});
	$("#"+id)[0].UFO_TYPE = UFO_PRICE;
}

function ufocharts(ws,i,t){
	$("#" + ws + " .megabox").remove();
	$("#" + ws + " .minibox").remove();
	$("#" + ws + " .minibox").css("height", 250);
	WS.send("ufoinit PL" + t + " " + i + " " + ws);
	document.getElementById(ws).className = "grid13";
}
function ufochart(t,id){
	$("#" + id + " .minibox").remove();
	$("#" + id + " .megabox").remove();
	document.getElementById(id).className = "gridUFO";
	WS.send("ufomega " + t + " " + W.current_quadspace+"q0ws0");
}

// ancient
function exmini(chart, div){
	var ws = W.current_quadspace+"q0ws0",
	d = $(div).detach();
	$(ws + " .minibox").remove();
	d.appendTo(ws);
	if (!div.classList.contains("split"))
		$(ws + " .split").remove();
	$(ws)[0].className = "gridUFO";
	chart.setSize(d.width(), 805);
	$(div).css("height", 865)
	WS.send("MDEL " + chart.title.textStr);
}

function vfoinit(tab,id){ // id comes from addVFO() this['wsX'].id via LoadQuadverse()
	var V, t = 'class="GTAB MTAB cell-border dataTable"><caption onclick=ufocharts("'+id+'",', t2 = '</caption><thead><tr><th>Ticker</th><th>Price</th><th class=ct>%CHG</th><th class=vsp>VSpike</th><th>Volume</th></tr></thead></table>',

	V    = document.createElement("div");
	V.id = "V15box";
	V.innerHTML = '<table id=V15 ' + t + '15,1)>' + '15m Volume Spike (Lowcaps)' + t2 + '<table id=V15H ' + t + '15,2)>' + '15m Volume Spike (Highcaps)' + t2;
	$("#"+id).append(V);

	V    = document.createElement("div");
	V.id = "V5box";
	V.innerHTML = '<table id=V5 ' + t + '5,1)>'   + '5m Volume Spike (Lowcaps)' + t2 + '<table id=V5H ' + t + '5,2)>' + '5m Volume Spike (Highcaps)' + t2;
	$("#"+id).append(V);

	V    = document.createElement("div");
	V.id = "V1box";
	V.innerHTML = '<table id=V1 ' + t + '1,1)>'   + '15m Volume Spike (Lowcaps)' + t2 + '<table id=V1H ' + t + '1,2)>' + '1m Volume Spike (Highcaps)' + t2;
	$("#"+id).append(V);

	newtable("V15", "desc",tab,2,3);
	newtable("V5",  "desc",tab,2,3);
	newtable("V1",  "desc",tab,2,3);
	newtable("V15H","asc", tab,2,3);
	newtable("V5H", "asc", tab,2,3);
	newtable("V1H", "asc", tab,2,3);
	
	$('#V15 tbody').on ('click', 'tr', function(){ufochart(T['V15'].row (this).data()['900'],id)});
	$('#V5 tbody').on  ('click', 'tr', function(){ufochart(T['V5'].row  (this).data()['900'],id)});
	$('#V1 tbody').on  ('click', 'tr', function(){ufochart(T['V1'].row  (this).data()['900'],id)});
	$('#V15H tbody').on('click', 'tr', function(){ufochart(T['V15H'].row(this).data()['900'],id)});
	$('#V5H tbody').on ('click', 'tr', function(){ufochart(T['V5H'].row (this).data()['900'],id)});
	$('#V1H tbody').on ('click', 'tr', function(){ufochart(T['V1H'].row (this).data()['900'],id)});
	$("#"+id)[0].UFO_TYPE = UFO_PRICE;
}

function qhide()
{
	var id = window.event.target.parentNode.parentNode.parentNode.id.replace(/[^0-9]/g, '').split(''),qdiv, QID;
	qdiv = "#P" + id[0] + "Q" + id[1];
	QID  = id[2] - 3;
	for (var x = 0; x<4; x++)
		$(qdiv+"q"+(QID+x)).css("display", "none");
}

function gridlines(){$('.highcharts-yaxis-grid')}

/* (ancient:unused) Only logical on the Master Workspace */
function grid(sz, gid){
	$("#grids").find("*").css("border","2px inset grey");
	$("#grid"+gid).css("border","2px solid #821eda");
	$("#ws" + W.current_workspace).addClass("grid"+sz);
	$("#ws" + W.current_workspace).removeClass("grid"+W.grid);
	W.grid = sz;
	var n  = W['nr_ws' + W.current_workspace+'_charts'],
	c      = W['ws'    + W.current_workspace+'_charts'];
	for (var x=0; x<n; x++)
		c[x].reflow();
}
