/**********************************************
 * filename: mainpage/stockminer/js/profile.js
 * License:  Public Domain
 *********************************************/
function squeak_chart_onclick()
{
	var upload_box = $("#upload-csv").css("display", "block").detach()[0];
	$("body").append(upload_box);
	var QGID   = window.event.target.parentNode.parentNode.parentNode.parentNode.id,
	action     = ACTION_DATA_CHART_SQUEAK,
	file       = $("#upload-csv-file")[0],
	chart_type = $(".upload-select option:selected", upload_box).val();
	upload_box.QGID = QGID;
	file.onchange = function(){
		file = file.files[0];
		obj  = file;
		sendfile({action:action,objtype:OBJTYPE_DATA,file:file,obj:obj,args:QGID+"/"+chart_type});
	}
}

function mouseconf()
{
	var c, conf = $("#MOUSECONF");
	conf.toggleClass("vshow");
	if (!conf.d) {
		$("div", conf).click(function(){
			c = $(this)[0].className;
			$("#sq-conf-pic")[0].className = c;
		});
		$(".QBTN", conf).click(function(){
			if (c) {
				$(W.current_quadspace + " .sq-pic")[0].className = "sq-pic " + c;
				var sq = $(W.current_quadspace + " .sq-msg-pic")[0];
				if (sq)
					sq.className = "sq-msg-pic " + c;
				WS.send("profile_set_img " + c);
				conf.toggleClass("vshow");
			}
		});
	}
}

/*
 * Profile RPC - called by server to instantiate
 * the user's Profile QuadVerse from the given JSON
 */
function rpc_profile(p,QGID)
{
	console.log("profile: " + QGID);
	if (!QGID)
		QGID = "#P4Q0";
	if (p.img == "minibat" || p.img == "mouse") {
		$(QGID + " .sq-pic").addClass(p.img);
		$("#sq-conf-pic").addClass(p.img);
	} else {
		$(QGID + " .sq-pic img")[0].src = p.img;
		$("#PIMG")[0].src = p.img;
	}
	if (document.location.pathname.substr(1) == USER) {
		if (p.r == "")
			return profileconf();
		$(QGID + " .SQ-HEAD").css("display", "none");
		return;
	}
	profill(prodict=p,QGID);
}

function profill(p,QGID)
{
	console.log("profil QGID: " + QGID);
	$(QGID + " .sq-name").html(p.r.replaceAll("^", " "));         // fullname
	$(QGID + " .sq-loc").html("🏰 " + p.l.replaceAll("^", " "));  // location
	$(QGID + " .sq-url").html("🕸 " + p.u);                       // URL
	$(QGID + " .sq-desc").html(p.d.replaceAll("^", " "));         // description
	if (p.j)
		$(QGID + " .sq-joined").html(p.j);  // location
	if (p.fs)
		$(QGID + " .sq-fs").html(p.fs);     // nr_followers
	if (p.fg)
		$(QGID + " .sq-fg").html(p.fg);     // nr_following
}

function profileconf()
{
	var c = $("#SQ-CONF")[0],file,obj,action;
	c.style.display="block";
	$("#sq-conf-name input").html(prodict['r']);
	$("#sq-conf-loc input").html(prodict['l']);
	$("#sq-conf-url input").html(prodict['u']);
	$("#SQ-CONF textarea").html(prodict['d']);
	if (!c.d) {
		c.d = 1;
		$(c).draggable();
		$("#PUPLOAD")[0].onchange = function(e){
			$("#sq-conf-pic").css("border","none");
			action  = ACTION_IMG_PROFILE;
			file    = $("#PUPLOAD")[0].files[0];
			obj     = $("#sq-conf-pic img")[0];
			objtype = OBJTYPE_IMG;
			sendfile({action:action,objtype:objtype,file:file,obj:obj,filename:file.name});
		}
		$("#BGUPLOAD")[0].onchange = function(e){
			action  = ACTION_IMG_BG_PROFILE;
			file    = $("#BGPLOAD")[0].files[0];
			obj     = $("#sq-bg img")[0];
			objtype = OBJTYPE_IMG
			sendfile({action:action,objtype:objtype,filename:file.name,file:file,obj:obj});
		}	
	}
}

/* profile [NAME] [LOC] [URL] [DESC] */
function setprofile()
{
	var rpc = "profile_set ", img ,v;

	prodict['r'] = v = $('#sq-conf-name input').val().replaceAll(" ", "^");
	rpc += v + "`";
	prodict['l'] = v = $('#sq-conf-loc input').val().replaceAll(" ", "^");
	rpc += v + "`";
	prodict['u'] = v = $('#sq-conf-url input').val();
	rpc += v + "`";
	prodict['d'] = v = $('#SQ-CONF textarea').val().replaceAll(" ", "^");
	rpc += v;
	img = $("#sq-conf-pic img")[0];
	if (img.upload) {
		img.upload();
		prodict['img'] = UID+'/pic.jpeg';
	} else
		prodict['img'] = 'minibat';
	WS.send(rpc);
}

function propic(){$("#QUPLOAD").click()}
function setprofile_ok(){$("#SQ-CONF").css("display", "none");profill(prodict,"#P4Q0")}
function setprofile_fail(e){}

/* Compose Squeak Message */
function compose(){$("#compose").toggleClass("vhide")}
var SQ;
//function newsqueak(n, msg, QGID, URL, img, nickname, uname)
function newsqueak(args)
{
	var QGID=args.QGID,img = args.img,URL=args.URL,nickname=args.nickname,msg=args.msg;
	if (img == 'minibat')
		img = '<span class="sq-msg-pic minibat"></span>';
	else
		img = ('<img src=' + img + '>');
	nickname = '<div class=sq-msg-name>' + nickname + '</div>';
	msg = $('<div id=' + URL + ' class=squeak>' + img + '<div class=sq-msg>' + nickname + msg.replace(/\\n/g, "<br>").replaceAll("^AT", "@") + "</div><div class=sq-obj></div></div>");
	MSG = msg;
	console.log("newsqueak new: " + n + " QGID: " + QGID + " URL: " + URL);
	if (args.new) // if new
		$(QGID + " .SQ-SQUEAKS").prepend(msg);
	else
		$(QGID + " .SQ-SQUEAKS").append(msg);
}

function sqconf()
{
	var sq = $(W.current_quadspace + " .SQ-HEAD");
	if (sq.css("display") == "block")
		$(sq).slideUp();
	else
		$(sq).slideDown();
}

/* callback html quadverse.html compose squeak send button */
function squeak_send_onclick()
{
	var msg = $("#compose-textarea").val().replace(/(\r\n|\n|\r)/gm, "\\n").replaceAll("@", "^AT"), URL = randstr();
	WS.send("squeak " + URL + " " + msg);
	newsqueak({new:1,URL:URL,msg:msg,QGID:"#P4Q0",img:prodict.img,nickname:prodict.r});
//	newsqueak(1,msg,"#P4Q0",URL,prodict.img,prodict.r);
}

// m`{n:nickname,t:timestamp,m:message,url:URL,uname:username,l:nr_likes,r:nr_replies,R:nr_resqueaks}
function squeak_rpc(cmd,QGID)
{
	console.log("squeak RPC QGID: " + QGID);
	for (var x=0; x<cmd.length; x+=2) {
		if (cmd[x] == "m") {
			var m = JSON.parse(cmd[x+1]), img;
			if (m.i == 0)
				img = "minibat";
			else
				img = "/blob/"+m.uid+"/pic.jpeg";
			newsqueak(0,m.m,QGID,m.url,img,m.n,m.uname);
		} else if (cmd[x] == "profile")
			profile(JSON.parse(cmd[x+1]),QGID);
	}
}

function follow()
{
	var name = $(W.current_quadspace + " .sq-name").html();
	WS.send("follow " + name);
}