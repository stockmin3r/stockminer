/*************************************************
 * filename: mainpage/stockminer/js/watchlists.js
 * License:  Public Domain
 ************************************************/

function load(watchlist,watchtable,QGID,id,pub){
	var obj;
	if (!QGID)
		QGID = watchtable.parentNode.id;
	if (typeof watchtable != "string") {
		obj        = watchtable.obj;
		watchtable = watchtable.id;
	} else
		obj = $("#"+watchtable)[0].obj;
	WS.send("watchlist_load " + watchlist + " " + watchtable + " " + QGID);
	$("#"+watchtable + " caption").text(watchlist);
	obj.watchlist = watchlist;
	obj.id        = id;
	obj.pub       = pub;
}

/* Private Watchlists */
function rpc_watchlist(av)
{
	for (var x=1; x<av.length; x++) {
		var w = av[x].split(":"), name = w[0];
		wopt(name,"pwatch",x);
		wopt(name,"vwatch",x);
		wopt(name,"ufo-select",x);
		WL.push({name:name,alerts:w[1],id:w[2],nr_stocks:w[3],owner:w[4]});
		WMenu['Load'+x] = {"name":name,selector:'td',callback:function(i){
			var watchlist = WL[i.substr(4)-1];
			load(watchlist.name,this[0].parentNode.parentNode.parentNode.parentNode,0,watchlist.id,0)
		}};
	}
}

/* Public Watchlists */
function rpc_gwatch(av)
{
	for (var x=1; x<av.length; x++) {
		var w = av[x].split(":"), name = w[0];
		GW.push({name:name,alerts:w[1],id:w[2],nr_stocks:w[3],owner:w[4]});
		GMenu['GLoad'+x] = {"name":name,selector:'td',callback:function(i){
			var watchlist = GW[i.substr(5)-1];
			load(watchlist.name,this[0].parentNode.parentNode.parentNode.parentNode,0,watchlist.id,1)
		}};
	}
}

function Watchlist(name)
{
	for (var x=0; x<WL.length; x++)
		if (WL[x].name == name)
			return WL[x];
	return 0;
}

function GWatchlist(name)
{
	for (var x=0; x<GW.length; x++)
		if (GW[x].name == name)
			return GW[x];
	return 0;
}

function watchlists_onchange()
{
	var w = $(".watchlist-select option:selected").val();

	$("#PUB tbody tr").remove();
	if (w == 1)
		pubload(GW);
	else if (w == 0)
		pubload(WL);
}

function watchlist_new(watchtable)
{
	if (!watchtable)
		watchtable = window.event.target.parentNode.parentNode.querySelector("table").id;
	WS.send("watchlist_clear " + watchtable);
	T[watchtable].clear().draw();
	$("caption", "#"+watchtable).text("New Watchlist")
}

function watchlist_addstock(watchtable_id) {
	var n = window.event.target;
	if (n.tagName == "IMG")
		n = n.parentNode.querySelector("input");
	if (!watchtable_id)
		watchtable_id = n.parentNode.parentNode.querySelector("table").id;
	WS.send("watchlist_addstocks " + watchtable_id + " " + n.value);
	n.value = '';
}

/* loads dialog whose button calls tbox_addstocks() */
function watchlist_bulk(watchtable, QGID)
{
	if (!QGID)
		QGID = "#"+window.event.target.parentNode.parentNode.id;
	if (!watchtable)
		watchtable = window.event.target.parentNode.curtab;
	var ubox = $("#upload-tickers");
	if (ubox.css("display")=='block') {
		ubox.slideUp("medium");
		return;
	}
	ubox = ubox.detach();
	if (!ubox[0].D) {
		ubox[0].D = 1;
		$(ubox).draggable();
	}
	ubox[0].watchtable_id = watchtable.id;
	$(QGID).append(ubox);
	ubox.slideToggle(700,"easeOutBounce");
}

function bulk_addstocks() {
	var watchtable_id = window.event.target.parentNode.parentNode.watchtable_id, chr = '\n';
	t = $(".tickarea")[0];
	if (t.value.indexOf(" ") != -1)
		chr = ' ';
	else if (t.value.indexOf(",") != -1)
		chr = ',';
	WS.send("watchlist_addstocks " + watchtable_id + " " + t.value.split(chr).join(" "));
	t.value = ""
}

function bulk_close() {window.event.target.parentNode.style.display = "none"}

function wsave_close(){$("#share").css("display", "none")}

/* CALLED BY:
 *  - WatchList Save Dialog Box (save button onclick) -> watchlist_save(1)
 *  - WatchTable menu (to load the Dialog Box)        -> watchlist_save(0,watchtable,QGID)
 *  - WatchList Save Icon (floppy)                    -> watchlist_save()
 *  - sh_save_watchlist() (script)                    -> watchlist_save()
 */
function watchlist_save(save, watchtable, QGID){
	var sh = $("#share");
	if (!QGID) {
		var node   = window.event.target.parentNode.parentNode;
		QGID       = "#"+node.parentNode.id;
		if (save)
			watchtable = sh[0].watchtable;
		if (!watchtable)
			sh[0].watchtable = watchtable = node.querySelector("table").id;
	}
	if (!save && sh.css("display")=='block') {
		sh.slideUp("medium");
		return;
	}
	if (!save) {
		$(QGID).append(sh.detach());
		if (!sh[0].D) {
			sh[0].D = 1;
			sh.draggable();
		}
		sh.slideToggle(700,"easeOutBounce");
		return;
	}

	var watchlist = document.getElementById("watchname").value.replaceAll(" ", "-"),
	pub           = document.getElementById("cshare").checked,
	id            = randstr(7),
	cmd           = "watchlist_save " + watchlist + " " + watchtable + " " + (pub ? "1 " : "0 ") + id, l, obj,
	nr_stocks     = T[watchtable].rows().count(),
	owner         = USER?USER:"Anonymous";

	$("#"+watchtable+" caption").text("Watchlist: "+watchlist);
	WS.send(cmd);
	obj = $("#"+watchtable)[0].obj;
	obj.watchlist = watchlist;
	obj.id = id;
	obj.pub = pub;
	WL.push({name:watchlist,alerts:0,id:id,nr_stocks:nr_stocks,owner:owner});
	if (pub) {
		GW.push({name:watchlist,alerts:0,id:id,nr_stocks:nr_stocks,owner:owner});
		pubload(GW);
	} else
		pubload(WL);
	l = WL.length;
	wopt(watchlist,"pwatch",l);
	wopt(watchlist,"vwatch",l);
	wopt(watchlist,"ufo-select",l);
	WMenu['Load'+(Object.keys(WMenu).length+1)] = {"name":watchlist, callback:function(i){load(WL[i.substr(4)-1,id])}};
	$("#watchname").val("");
	$("#nickname").val("");
	$("#cshare")[0].checked=0;
	$("#share .check")[0].classList.replace("tick", "untick");
	sh.slideUp("medium");
}

function watchlist_remove(watchlist,p)
{
	for (var x = 0; x<WL.length; x++) {
		var id = WL[x].id;
		if (WL[x].name == watchlist) {
			if (p)
				p[0].parentNode.remove(); // remove <tr> from Watchlists table
			WL.splice(x, 1);
			WS.send("watchlist_remove " + watchlist);
			for (var y = 0; y<GW.length; y++) {
				if (GW[y].id == id) {
					GW.splice(y, 1);
					break;
				}
			}
			watchlist_new('morphtab');
			break;
		}
	}
}

function watchtable_clear(watchtable, watchlist_id)
{
	var tab = $(watchtable + " tbody tr"),stocks="";

	if ($(watchtable)[0].obj.id != watchlist_id)
		return;
	if (tab.length <= 0 || tab[0].cells[0].innerText.indexOf("No") != -1)
		return;
	for (var x = 0; x<tab.length; x++)
		stocks += tab[x].cells[1].innerText + ",";
	stocks = stocks.split(",");
	stocks.pop();
	for (var x = 0; x < stocks.length; x++) {
		console.log("removing stock: " + stocks[x]);
		del_tr(watchtable.substr(1), 	900, stocks, x);
	}

}

function del_tr(tab, key, val, i) {
	T[tab].rows(function(i,data) {return data[key] === val[i]}).remove().draw();
}

function TableTheme(theme)
{
	var tab = $("#"+window.event.target.parentNode.parentNode.parentNode.parentNode.className.split(" ")[1].split("-")[1])[0],
	c = tab.className.split(" ");
	c[0] = theme;
	tab.className = c.join(" ");
	if (!document.querySelector(".ut-"+theme) && (c=localStorage[theme])) {
		console.log("loading table theme: " + theme);
		var style = document.createElement("style");
		style.innerHTML = c;
		style.className = 'ut-'+theme;
		document.body.appendChild(style);
	}
}

function LoadThemes()
{
	var x = 0, nr_css;

	// Watchtable Themes
	for (; x<TCLASS.length; x++)
		Themes['Theme'+x] = {"name":TCLASS[x], callback:function(i){TableTheme(TCLASS[i.substr(5)])}};
	nr_css = localStorage.nr_table_css;
	if (!nr_css)
		return;
	console.log("nr_css: " + nr_css);
	for (var y = 0; y<nr_css; y++,x++) {
		TCLASS[x+y] = localStorage['TCLASS'+(x+y)];
		console.log("loading tclass: " + (x+y));
		Themes['Theme'+(x+y)] = {"name":TCLASS[x+y], callback:function(i){TableTheme(TCLASS[i.substr(5)])}};
	}
}

// caters to static table in the screener quadspace, top right quad, will be replaced
function pubload(watchlist) {
	T['PUB'].clear();
	for(var x=0;x<watchlist.length;x++)
		T['PUB'].rows.add([{"N":watchlist[x].name,"U":watchlist[x].owner,"NT":watchlist[x].nr_stocks,"A":watchlist[x].alerts}]).draw()
}

/* ***********************************************************************************************
 *                                    TABLE COLORS
 ************************************************************************************************/
function tableColors_onclick()
{
	$("#color-diag").css("display", "block");
	var cs = COLORS[$(".cs-select option:selected").val()], rules, column, from, to, c, slots=6;
	var screener = window.event.target.parentNode.parentNode;

	column = $(".wdst li.boxtick", screener)[0];
	if (!column) {
		column = $(".wsrc li.boxtick", screener)[0];
		if (!column)
			return;
	}
	column = column.getAttribute("v");
	$(".column-name").text(WCOL[column].split(" ")[0]);
	$(".column-name")[0].column = column;
	rules = cs[column];
	if (rules) {
		for (var x = 0; x<rules.length; x++) {
			from = rules[x].f;
			to   = rules[x].t;
			c    = rules[x].c;
			bg   = rules[x].b;
			if (bg)
				bg = '<option value=1>Background</option><option value=0>Text</option>';
			else
				bg = '<option value=0>Text</option><option value=1>Background</option>';
			$("#color-diag").append('<div class=color-rule>Cell Value between <input class=color-from type=text value=' + 
			from + '> and <input class=color-to type=text value=' + to + '> <input class=color-box type=color value=' + c + '>' +
			'<select class=color-bg>' + bg + '</select>');
		}
		slots = 6-rules.length;
	}
	for (var x=0; x<slots; x++)
		$("#color-diag").append(CRULE.clone());
}

function tableColors(row,data,index,tid,hdr,nobg)
{
	var cs = COLORS[0], keys = Object.keys(data), rules,
	th = $(tid)[hdr].children;

	for (var x = 0; x<keys.length; x++) {
		if (rules=cs[keys[x]]) {
			var v = parseFloat(data[keys[x]]);
			for (var y = 0; y < rules.length; y++) {
//if (keys[x] == 913)
//	console.log("key: " + keys[x] + " rules f: " + rules[y].f + " rules t: " + rules[y].t + " bg: " + rules[y].b);
				if (rules[y].f <= v && rules[y].t > v) {
					if (!nobg && rules[y].b=="1")
						attr = "background-color:"+rules[y].c+"!important;color:black!important";
					else
						attr = "color:"+rules[y].c+"!important";
					column = WCOL2[keys[x]];
					for (var z = 0; z<th.length; z++) {
						if (th[z].innerText === column) {
//							console.log(tid + " " + keys[x] + " " + index + " setting attr: " + attr);
							$('td',row).eq(z).attr("style",attr);
							break;
						}
					}
				}
			}
		}
	}
}

function tableColors_set(t)
{


}

function tableColors_save()
{
	var cs = COLORS[$(".cs-select option:selected").val()],
	rules  = document.querySelectorAll(".color-rule"),r,f,t,c,new_rules = [],
	column = $(".column-name")[0].column;
	for (var x = 0; x<rules.length; x++) {
		r = rules[x];
		f = $(".color-from", r).val();
		t = $(".color-to", r).val();
		if (!f || !t)
			continue;

		new_rules.push({f:f, t:t, c:$(".color-box", r).val(), b:$(".color-bg", r).val()});
	}
	cs[column] = new_rules;
	t = Object.values(T);
	for (var x = 0; x<t.length; x++)
		t[x].draw();
	// [{"Open":[{f:17,to:21,c:purple},{f:1,t:5,c:green}]},{"#Days":[{f:22,to:88,c:red}]}]
}

function tableColors_close()
{
	$("#color-diag").css("display","none");
	$("#color-diag .color-rule").remove();
}


function tableColors_onchange()
{
	var color = window.event.target.value;

}


function boxtick()   {window.event.target.className="boxtick";if(WLI)WLI.className="";WLI=window.event.target}
function expand(id)  {if ($(id).hasClass("expand"))$(id).css("background-color", "#821eda");else $(id).css("background-color", "#2f3136");$(id).toggleClass('expand')}
function wopt(n,id,x){
	var opt = document.createElement("option");
	opt.appendChild(document.createTextNode(n));
	opt.value = x;
	document.getElementById(id).appendChild(opt)
}
