/**********************************************
 * filename: mainpage/stockminer/js/qobjects.js
 * License:  Public Domain
 *********************************************/
/*
 *  0       1         2      3
 *  qupdate grid      QGID   grid                              // set grid of Quadspace/Workspace
 *  qupdate qsp_del   QVID   QSID                              // remove Quadspace
 *  qupdate img       QGID   URL                               // load image into workspace
 *  qupdate pos       QGID   URL:height:width:top:bottom:left  // set dimensions of object
 *  qupdate indi      op:1   chart-div indicator_name          // add indicator
 *  qupdate indi      op:0   chart-div indicator_name          // remove indicator
 *  qupdate del_obj   QGID   URL   type                        // remove object
 *  qupdate del_chart QGID   ticker                            // remove chart
 */
function rpc_qupdate(av)
{
	console.log(av.join(" "));
	var QARGS = JSON.parse(av[2]), URL = QARGS.URL, QGID = QARGS.QGID, quad;
	switch (av[1]) {
		case 'grid':
			W.grid(QGID, QARGS.grid, 1);
			break;
		case 'qsp_del':
			Q[QARGS.QVID].removeTab(Q[QVID].workspace['ws'+QARGS.QSID].tab,1);
			break;
		case 'ws_del':
			quad = Q[QARGS.QVID].quadspace[QARGS.QSID].quad[QARGS.QID];
			quad.removeTab(quad.workspace['ws'+QARGS.WSID].tab,1);
			break;
		case 'wsname':
			console.log("wsname: " + QGID + " " + QARGS.title);
			$(QGID.replaceAll("^", " ") + " .chrome-tab-title").html(QARGS.title);
			break;
		case 'img':
			if (document.querySelector(".o"+URL.split(":")[0]))
				break;
			var img = new Image();
			img.src = "/blob/"+URL;
			img.className = "ws-img o" + URL;
			newobj(img,"#"+QGID,URL,'img');
			bshide("#"+QGID);
			break;
		case 'pos':
			qsetpos(QARGS.URL);
			break;
		case 'indi':
			var chart       = charts[QARGS.CGID];
			chart.menu.indi = QARGS.indi;
			if (QARGS.op == 1)
				iadd(chart,0,0,1);
			else
				iremove(chart);
			break;
		case 'del_chart':
			QARGS.type = 'chart';
			DELETE_OBJECT(QARGS, 0);
			break;
		case 'del_blob':
			console.log("QUPDATE DEL BLOB: " + QARGS.type + " " + QARGS.OBJ_ID);
			DELETE_OBJECT(QARGS, 0);
			break;
		case 'del_wstab':
			QARGS.type = 'wstab';
			DELETE_OBJECT(QARGS, 0);
			break;
		case 'add_wstab':
			console.log("qupdate: add_wstab: " + URL);
			W.addWatchtable({QGID:QGID,TID:URL,send_rpc:0});
			break;

	}
}

/* Remove objects from a Workspace's obj array */
function deref(t,QGID,type)
{
	var ws=WMAP[QGID].w,obj=ws.workspace['ws'+ws.current_workspace].obj;
	for (var x=0; x<obj.length; x++)
		if (obj[x].type == type && t == obj[x].ref.title.textStr)
			obj.splice(x,1);
	return obj.length;
}
function del_chart(ticker,QGID,rpc)
{
	var CGID = ticker+"-"+QGID, id = QGID.replace(/[^0-9]/g, '').split(''),QVID=id[0],QSID=id[1],QID=id[2],WSID=id[3];
	charts[CGID].destroy();
	if (!deref(ticker, QGID, 'chart'))
		blankspace("#"+QGID,QVID,QSID,QID,WSID);
	$("#"+CGID).remove();
	if (rpc) {
		window.event.stopPropagation();
		WS.send(QUPDATE_DEL_CHART + QSID + " " + QID + " " + WSID + " " + ticker);
	}
}

function unlink(list,obj)
{
	for (var x = 0; x<list.curobj.length; x++) {
		if (list.curobj[x].obj == obj) {
			console.log("unlinking object from screener");
			list.curobj.splice(x, 1);
			break;
		}
	}
}

/* 
 * 1) Remove Object from Q[QVID].quadspace[QSID].quad[QID].workspace['ws'+WSID].obj[]
 * 2) Load Blankspace Widget Selector if no objects remain in the obj array
 * 3) Remove the Object from DOM
 * 4) Send QUPDATE command to all clients who are viewing this workspace so they also remove this object
 */
function delete_object(QUPDATE_CMD, QGID, OBJ_ID, OBJ_QGID, ws_obj, obj, rpc)
{
	var id = QGID.replace(/[^0-9]/g, '').split(''),QVID=id[0],QSID=id[1],QID=id[2],WSID=id[3];
	if (!ws_obj.length)
		blankspace(QGID,QVID,QSID,QID,WSID);

	console.log("delete_object: OBJ_QGID: " + OBJ_QGID);
	if (OBJ_QGID)
		$(OBJ_QGID).remove();
	if (rpc) {
		window.event.stopPropagation();
		QUPDATE_CMD += QVID + " " + QSID + " " + QID + " " + WSID + " " + OBJ_ID;
		WS.send(QUPDATE_CMD);
	}

	// if the table is linked to a screener/controller, remove it from the list's curobj list
	if (obj.screener)
		unlink(obj.screener, obj);
	if (obj.controller)
		unlink(obj.controller, obj);
}

function DELETE_OBJECT(args,rpc)
{
	var type=args.type,ws=WMAP[args.QGID.replaceAll("#", "")].w,ws_obj=ws.workspace['ws'+ws.current_workspace].obj,obj,QUPDATE_CMD,OBJ_QGID;
	for (var x=0; x<ws_obj.length; x++) {
		console.log("DELETE_OBJECT: " + ws_obj[x].type);
		if (ws_obj[x].type != type)
			continue;
		switch (type) {
			case 'chart':
				if (args.ticker != ws_obj[x].ref.title.textStr)
					continue;
				OBJ_QGID = args.ticker + "-"+args.QGID;
				charts[OBJ_QGID].destroy();        // AAPL-P0Q0q0ws0 || (OBJ_ID + QGID)
				ws_obj.splice(x,1);
				return delete_object(QUPDATE_DEL_CHART, args.QGID, args.ticker, "#"+OBJ_QGID, ws_obj, 0, rpc);
			case 'XLS':
			case 'wstab':
				console.log("at ws_obj[x][type]: " + ws_obj[x]['TID'] + " vs " + args.TID + " ID: " + ws_obj[x].ref.id);
				if (args.TID != ws_obj[x]['TID'])
					continue;
				obj = $("#"+args.TID)[0].obj;
				T[args.TID].destroy(1);
				OBJ_QGID = "#"+args.TID+"-box";
				ws_obj.splice(x,1);
				return delete_object(QUPDATE_DEL_WSTAB, args.QGID, args.TID, OBJ_QGID, ws_obj, obj, rpc);
			default:
				console.log("default: URL: " + args.URL + " VS " + ws_obj[x][type]);
				if (args.URL != ws_obj[x]['URL'])
					continue;
				if (type == 'img')
					$(".o"+args.URL)[0].parentNode.parentNode.remove();
				console.log("calling delobj: " + args.URL);
				ws_obj.splice(x,1);
				return delete_object(QUPDATE_DEL_IMG, args.QGID, args.URL, 0, ws_obj, 0, rpc);
		}
	}
}

function init_obj_trash()
{
	var id,args={};

	$(document).keyup(function(e){
		if (e.keyCode == 46 || e.keyCode == 8) {
			var obj = $(".selected")[0];
			if (!obj)
				return;
			if (obj.classList[0] == "qchart") {
				args.type     = 'chart';
				id            = obj.id.split("-");
				args.OBJ_QGID = obj.id;
				args.ticker   = id[0];
				args.QGID     = id[1];
			} else if (obj.obj && (obj.obj.type == "wstab" || obj.obj.type == 'XLS')) {
				args.type     = obj.obj.type;
				console.log("obj id: " + obj.id);
				args.TID      = obj.id.split("-")[0];
				args.QGID     = obj.parentNode.id;
				args.OBJ_QGID = "#"+obj.id;
			} else if (obj.tagName == "IMG") {
				args.type     = "img";
				args.URL      = obj.classList[1].substr(1);
				args.QGID     = obj.parentNode.parentNode.parentNode.id;
				args.OBJ_QGID = ".o" + args.URL;
			}
			DELETE_OBJECT(args,1);
		}
	});
}

function selobj(obj)
{
	$(".selected").removeClass("selected");
	if (obj.classList.contains("selected"))
		obj.classList.remove("selected");
	else
		obj.classList.add("selected");
}

function newobj(obj, QGID, URL, type, cached)
{
	var id = QGID.replace(/[^0-9]/g, '').split(''),div=document.createElement('div'),QVID=id[0],QSID=id[1],QID=id[2],WSID=id[3],url=URL.split(":")[0],selector=".o"+url;
	div.className = type;
	$(QGID).append(div);
	$(div).append(obj);
	$(obj).resizable({grid:[20,20],resize:function(e,ui){updateSize(ui, div, id,url,type)}});
	$(div).draggable({grid:[20,20],snap:QGID,stop:function(e,ui){updatePosition(ui, div, id, url, type)}});
	div.style.position = "absolute";
	div.style.zIndex = 10000;
	console.log("newobj: " + type + " " + URL + " QGID: " + QGID);
	/* Don't add object to QCACHE JSON on quadspace_load() init as is already there */
	if (!cached)
		W.qcache_add(QVID,QSID,QID,WSID,'NewTab',type,"/blob/"+URL);
	else
		qsetpos(URL);

	$.contextMenu({selector:selector,trigger:'right',build:function(){return{items:{"R":{name:"Remove",callback:function(){DELETE_OBJECT(type,URL,QGID,1)}}}};}});
	document.querySelector(selector).onclick = function(e){selobj($(selector)[0])};
	WMAP[QGID.substr(1)].w.workspace['ws'+WSID].obj.push({type:type,URL:url,ref:obj});
}
var NR_PDF = 0;
var CANVAS = [];
var CC,CVX;

function updatePosition(ui, div, ID, URL, obj_type)
{
	var n = ui.helper[0].children[0], width = $(n).width(), height = $(n).height(), pos = ui.position;
	WS.send(QUPDATE_POS + ID[0] + " " + ID[1] + " " + ID[2] + " " + ID[3] + " " + obj_type + ":" + URL+":"+width+":"+height+":"+pos.top+":"+pos.left);
}

function updateSize(ui, div, ID, URL, obj_type)
{
	var n = $(ui.helper[0].parentNode)[0].style, size = ui.size,
	top = n.top.substr(0, n.top.indexOf('p')), left = n.top.substr(0, n.left.indexOf('p'));
	WS.send(QUPDATE_POS + ID[0] + " " + ID[1] + " " + ID[2] + " " + ID[3] + " " + obj_type + ":" + URL + ":"+size.height+":"+size.width+":"+top+":"+left);
}

function blankspace(QGID,QVID,QSID,QID,WSID)
{
	if (QGID.indexOf("#") == -1)
		QGID = "#" + QGID;
	if ($(QGID+" .bspace")[0] == undefined)
		W.addBlankspace(QVID,QSID,QID,'ws'+WSID);
	$(QGID+" .bspace").css("display", "grid");
	$(QGID)[0].className = "gridB";
}

/* *******************
 *
 * QUADVERSE EXPORT
 *
 ********************/
function qclose()    {window.event.target.parentNode.style.display="none"}
function qpublish()  {$("#QPUB").css("display", "block")}
function qdeploy()   {qexport($("#QNAME").val())}
function qpic_onchange()
{
	var q = $("#QUPLOAD");
	if ($("#QSEL option:selected").val() != "0") {
		q.removeAttr("disabled");
		q.css("color","white");
	} else {
		q.attr("disabled", 1);
		q.css("color","grey");
	}
}

function qsetpos(url)
{
	var pos = url.split(':');
	if (pos.length == 1)
		return;
	var obj = document.querySelector(".o"+pos[0]), height = pos[1]+"px", width = pos[2]+"px";
	obj.style.height = height;
	obj.style.width  = width;
	obj.parentNode.style.width  = width;
	obj.parentNode.style.height = height;
	obj.parentNode.parentNode.style.top  = pos[3]+"px";
	obj.parentNode.parentNode.style.left = pos[4]+"px";
}

function ctool(tool)
{
	var c = window.event.target.parentNode.parentNode.C;
	c.setSelection(c,0,tools[tool])
}
