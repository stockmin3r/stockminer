/*********************************************
 * filename: mainpage/stockminer/js/menu.js
 * License:  Public Domain
 ********************************************/
function spage(t)     {$('.query-div').remove();$('#autoq').css('height',"0px");stockpage(t,-1);$("#query").val('')}
function sbox()       {spage(document.getElementById("query").value.toUpperCase())}
function qload()      {spage($('.query-stock', window.event.target.parentNode).html())}
function gridspace()  {
	var w = QuadVerses['ChartSpace'];
	QUADVERSE_SWITCH("ChartSpace");
	w.showtab(w.workspace['ws1'].tab);
	quadspace_load(1);
}
function rpc_query(av)    {
	var q, qv = av[1].replaceAll("^", " ").split('#');
	$('.query-div').remove();
	for (var x = 0; x<qv.length; x++) {
		q = qv[x].split(" ");
		$('#autoq').append('<div class=query-div onclick=qload()><span class=query-stock>' + q[0] + '</span><span class=query-name>' + q[1].replaceAll('%',' ') + '</span></div>');
	}
	$('#autoq').css('height',qv.length * 32 + "px").slideDown();
}

function search(e)
{
	if (e.keyCode === 13)
		return sbox();
	var query = e.target.value.toUpperCase();
	if (query.length > 2)
		WS.send("search " + query);
	else {
		$('.query-div').remove();
		$('#autoq').css('height',"0px");
	}
}

/* Table & Quad Menus */
function WatchTable_Menu(menu, watchtable, QGID){
	var dt = T[watchtable];
	$.contextMenu({
		selector:menu,
		trigger:'left',
		className:'c-'+watchtable,
		build: function(){return{items:{
			"New":    {name:"New Watchlist",    callback:function(){
				watchlist_new(watchtable);
			}},
			"Save":   {name: "Save Watchlist",  callback:function(){
				watchlist_save(0,watchtable,QGID);
			}},
			"Copy":   {name:"Copy",callback:function(){
//					dt.buttons(".buttons-copy").trigger('click');
//					$("#morphtab .buttons-copy").click();
				navigator.clipboard.writeText(copytable(dt));
//					$(".buttons-copy", $("#morphtab")[0].b).click()
			}},
			"Link": {
				"name": "Link",
				"items":{
					"Link1":{"name":"Cyan",   icon: "Cyan",  callback:function(){link("cyan")}},
					"Link2":{"name":"White",  icon: "White", callback:function(){link("white")}},
					"Link3":{"name":"Orange", icon: "Orange",callback:function(){link("orange")}},
					"Link4":{"name":"Green",  icon: "Green", callback:function(){link("green")}},
					"Link5":{"name":"Red",    icon: "Red",   callback:function(){link("red")}},
				}},
			"GLS":    {name:"Public Watchlist", items:GMenu},
			"WLS":    {name:"Private Watchlist",items:WMenu},
			"Preset": {name:"Columns",          items:TPMenu},
			"BLK":    {name:"Load Tickers",     callback:function(){
				watchlist_bulk(watchtable,QGID);
			}},
			"Destroy":{name:"Destroy Table",    callback:function(){
				DELETE_OBJECT({type:'wstab',TID:watchtable,QGID:QGID.substr(1)}, 1);
			}},
			"Theme":  {name:"Theme",        items:Themes},
			"Create": {name:"Create Theme", callback:function(){
				Editor("watchtable", "css");	
			}},
			"Script": {name:"Scripting",    callback:function(){
				Editor("watchtable", "js");	
			}},
			"Import": {name:"Import",    callback:function(){
				GUI_wget_url(watchtable, QGID);
			}},
			"Import": {
				name: "Import",
				items:{
					"I0":{name:"Website", callback:function(){GUI_wget_url(watchtable,QGID)}},
					"I1":{name:"Excel",   callback:function(){table_import(watchtable,1)}},
					"I2":{name:"CSV",     callback:function(){table_import(watchtable,2)}},
					"I2":{name:"TXT",     callback:function(){table_import(watchtable,3)}},
					"I3":{name:"Paste",   callback:function(){table_import(watchtable,4)}},
				}
			},
			"Export": {
				name: "Export",
				items:{
					"E1":{name:"Excel",  callback:function(){table_export(watchtable,1)}},
					"E2":{name:"CSV",    callback:function(){table_export(watchtable,2)}},
					"E3":{name:"TXT",    callback:function(){table_export(watchtable,3)}},
					"E4":{name:"Column", callback:function(){table_export(watchtable,4)}},
				}
			},
		}};
	}});
}
function Watchlists_RClick_Menu(){
	$.contextMenu({
		selector:"#PUB tbody td",
		trigger:'right',
		build: function(){return {items:{
			"Del": {name: "Remove Watchlist", selector:'.context-menu-one td', callback:function(){
				watchlist_remove(this[0].parentNode.cells[0].textContent,this)
			}},
		}};
	}});
}

function WatchTable_RClick_Menu(TID){
    $.contextMenu({
		selector:"#" + TID + " tbody td",
		trigger:'right',
        build: function(){return{items:{
			"Del": {name: "Remove Stock", selector:'.context-menu-one td', callback:function(){
				var row = this[0].parentNode, ticker = row.cells[1].textContent, watchlist = $("#"+TID)[0].obj.watchlist;
				console.log("watchlist: " + watchlist + " rowIndex: " + row.rowIndex + " sIndex: " + row.sectionRowIndex);
				T[TID].rows(row.rowIndex-1).remove().draw();
				if (watchlist == undefined)
					watchlist = 'morphtab';
				WS.send("watchlist_delstock " + watchlist + " " + ticker);
			}},
        }};
	}});
}

function XLS_Menu(id, watchtable, QGID){
	$.contextMenu({
		selector:id,
		trigger:'left',
		className:'c-'+watchtable,
		build: function(){return{items:{
			"Link": {
				"name": "Link",
				"items":{
					"Link1":{"name":"Cyan",   icon: "Cyan",  callback:function(){link("cyan")}},
					"Link2":{"name":"White",  icon: "White", callback:function(){link("white")}},
					"Link3":{"name":"Orange", icon: "Orange",callback:function(){link("orange")}},
					"Link4":{"name":"Green",  icon: "Green", callback:function(){link("green")}},
					"Link5":{"name":"Red",    icon: "Red",   callback:function(){link("red")}},
				}},
			"Preset": {name:"Columns",          items:TPMenu},
			"Load":   {name:"Load Ticker",      callback:function(){
				XLS_loadTicker(watchtable,QGID);
			}},
			"Destroy":{name:"Destroy Table",    callback:function(){
				DELETE_OBJECT({type:'XLS',TID:watchtable,QGID:QGID.substr(1)}, 1);
			}},
			"Theme":  {name:"Theme",        items:Themes},
			"Create": {name:"Create Theme", callback:CSSEditor},
			"Script": {name:"Scripting",    callback:JSEditor},
		}};
	}});
}
