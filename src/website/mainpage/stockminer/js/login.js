/*********************************************
 * filename: mainpage/stockminer/js/login.js
 * License:  Public Domain
 ********************************************/

/*
 * Called by the init_websocket() msg loop after a successful login
 * the "Backpage" contains the HTML/JS/CSS and the BACKPAGE JSON
 * which consists of all the QuadSpaces,Quads and their Workspace
 * definitions which will be loaded by LoadQuadverse()
 */
function backpage(p){
	var pageidx  = p.indexOf('<');
	var pagename = p.substr(0, pageidx).split("-")[1];

	/* append the backpage HTML/JS/CSS to the body */
	$("body").append(p.substr(pageidx));

	console.log("backpage index: " + pageidx + " pagename: " + pagename);
	var BACKPAGE = window["BACKPAGE_"+pagename]['init_backpage'];

	/* call src/website/backpage/monster/backpage.js::init_monster() */
	window[BACKPAGE]();

	/* Upon login, the default/first quadverse of the mainpage will be shown
	 * this will need to be more generic - currently it calls mainpage::stockminer::Screener
	 */
	if (document.location.pathname.length == 1)
		QUADVERSE_SWITCH('Screener');
	WINIT=1;
	$("#LBOX").css("display", "none");
	$("#MENU").css("display", "block");
	$("#MAIN").css("display", "block");
	$("#LOGIN").text(USER);
	if (USER=="admin")
		admin([]);
}

/* RPC: Server instructs client that the user registration process was a success */
function rpc_regok(av) {window.location.reload()}

function rpc_login_error(av) {
	errors[av[1]][av[2]](av[3])
}

function rpc_set_user(av)
{
	localStorage.USER = USER = av[1];
	$("#LOGIN").text(USER);
	UID=av[2]
}

function rpc_set_cookie(av)
{
	localStorage.cookie = av[1]
	console.log("setting local cookie: " + av[1]);
}

function login() {
	if (USER == localStorage.user) {
		QUADVERSE_SWITCH("Profile");
		return;
	}
	QUADVERSE.style.display="none";
	LBOX=$("#LBOX").clone();
	$("#MENU").css("display", "none");
	$("#LBOX").css("display", "block")
}

function register()
{
	$(".mailbox").slideDown();
	$(".login").val("Register");
	$(".login").attr("onclick", "wsregister()");
	$(".reg").html("Back to Login");
	$(".reg").attr("onclick", "logback()");
}

function wslogin()    {
	$(".error").remove();
	USER=$("#LBOX .usr").val();
	WS.send("login "    + USER + " " + $("#LBOX .pwd").val());
	localStorage.user = USER;
}
function wsregister() {
	$(".error").remove();
	USER=$("#LBOX .usr").val();
	WS.send("register " + USER + " " + $("#LBOX .pwd").val() + " " + $("#LBOX .mail").val())
}

function elogin(e) {
	if (!e)
		e = '<div class=error>Incorrect Username/Password</div>';
	else if (e == 1)
		e = '<div class=error>That Username already exists</div>';
	else if (e == 2)
		e = '<div class=error>Illegal Username format</div>';
	$(e).insertBefore(".reg")
}

function newuser(){
	WS.send("reg " + $("#new-uid").val() + " " + $("#new-pwd").val());
}

function backhome() {
	$("#LBOX").css("display", "none");
	$("#MENU").css("display", "block");
	$("#MAIN").css("display", "block");
	QUADVERSE.style.display='block';
}

function logback()
{
	$(".mailbox").slideUp();
	$(".login").val("Login");
	$(".login").attr("onclick", "wslogin()");
	$(".reg").html("Register as a new user");
	$(".reg").attr("onclick", "register()");
}

var OBJ;
function init_webassembly_auth()
{
	var importObject={ "env": {
		STACKTOP:0,
		STACK_MAX:65536,
		abortStackOverflow: function(val) { throw new Error("stackoverfow"); },
		abort:function(){},
		memory: new WebAssembly.Memory( { initial: 256, maximum:256 } ),
		table:  new WebAssembly.Table ( { initial:0, maximum:0, element: "anyfunc" }),
		memoryBase:0,
		tableBase:0
	}, imports: {
		imported_func(arg) {
		console.log(arg);
	    },
	  }
	};

	WebAssembly.instantiateStreaming(fetch("wasm"), importObject).then(
	  (obj) => {OBJ=obj;}
	);
}

function newranks(){ $("#admin-ranks-file")[0].upload() }
function maxranks(){ WS.send("ranks " + $("#admin-max-ranks").val()) }

// obsolete
function admin(av)
{
	if (av[1]) {
		$("#admin-status").html(av[1].replaceAll("-", " "));
		return;
	}
	var quad = Q[0].quadspace[1].quad[1],action,objtype,
	ws = quad.addWorkspace({title:"Admin", favicon:0},{background:0}, "", 1, 1, -1);
	quad.workspace['ws'+ws].tab.removeAttribute("active");
	quad.workspace['ws0'].tab.setAttribute("active", "");

	$("#P0Q1q1ws" + ws).append($("#admin").detach());
	$("#P0Q1q1ws" + ws).css("display", "none");
	$("#admin").css("display", "block");
	$("#admin-ranks-file")[0].onchange = function(){
		file    = $("#admin-ranks-file")[0].files[0];
		obj     = $("#admin-ranks-file")[0];
		objtype = OBJTYPE_DATA;
		action  = ACTION_DATA_RANKS_UPDATE;
		sendfile({action:action,objtype:objtype,file:file,obj:obj});
	};
}
