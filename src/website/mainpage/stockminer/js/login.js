/*********************************************
 * filename: mainpage/stockminer/js/login.js
 * License:  Public Domain
 ********************************************/

/*
 * libhydrogen constants for Password-based Authentication
 */
var hydro_sign_BYTES          = 64;
var hydro_sign_SEEDBYTES      = 32;
var hydro_sign_PUBLICKEYBYTES = 32;
var hydro_sign_SECRETKEYBYTES = 64;

/* Server sends a random nonce upon websocket connecton */
function rpc_nonce(av)
{
	NONCE = av[1];
}

/*
 * User clicks on html/menu.html::<div id=LOGIN onclick=login()>Login</div>
 *   - Password-Based Authentication by constructing the challenge
 *   - NONCE was already sent by the server after the websocket is established
 *   - Reconstruct the Pub/Priv keypair from a deterministic seed of "username|password"
 *   - Sign the "username|nonce" with the public key and send the "username|signature"
 *   - The Server will lookup the username and acquire its public key (stored after registering)
 *   - The Server will verify the signature of "username|NONCE"
 *     - The NONCE will be kept server-side in struct connection and in mainpage.js::var NONCE; client-side
 */
var KP,SEED,SIG,CH;
function login_onclick()
{
	var rc; // return value for libhydrogen function calls

	$(".error").remove();
	localStorage.user = USER=$("#LBOX .usr").val();

	var challenge = CH = new Uint8Array([
		...new TextEncoder().encode(USER + "|"),
		...Uint8Array.from(atob(NONCE), c => c.charCodeAt(0))
	]);

	var password    = $("#LBOX .pwd").val();
	var auth        = USER + "|" + password;

	/*
	 * Allocate memory for the keypair,seed and signature (all will be integer indecies into wasmMemory.buffer)
	 */
	var mem_offset  = hydro_sign_PUBLICKEYBYTES + hydro_sign_SECRETKEYBYTES;
	var keypair     = Module._malloc(mem_offset);
	var kp_seed     = Module._malloc(mem_offset+=hydro_sign_SEEDBYTES);
	var signature   = Module._malloc(mem_offset+=hydro_sign_BYTES);

	// 1) Regenerate the seed from username|password
	rc = Module.ccall("hydro_pwhash_deterministic",      // function name
					  "number",                          // return type of this function (int): 0 for success -1 for failure
					 ["number","number",                 // uint8_t[]: kp_seed, int: sizeof(kp_seed[])
					  "string","number",                 // char *: auth,       int: strlen(username|password)
					  "string", "number",                // char *: context,    int: OPSLIMIT
					  "number","number"],                // size_t: memlimit,   uint8_t: nr_threads
					 [kp_seed, 32,                       // keypair seed, sizeof(kp_seed)
					  auth, auth.length,                 // username|password, strlen(username|password)
					  "context0", 1000, 0, 1]);

	var seed = SEED = new Uint8Array(wasmMemory.buffer, kp_seed, hydro_sign_SEEDBYTES)

	var str = "----seed----\n";
	for (var x = 0; x<hydro_sign_SEEDBYTES; x++)
		str += seed[x] + " ";
	console.log(str);

	// 2) Recreate the private/public keypair from the seed
	Module.ccall("hydro_sign_keygen_deterministic",      // function name
					  null,                              // return type of this function (int): 0 for success -1 for failure
					 ["number","number"],                // argument types to hydro_sign_keygen_deterministic (two uint8arrays)
					 [keypair, kp_seed]);                // arguments to hydro_sign_keygen_deterministic()

	/*
	 * We want to extract the private/secret key from the keypair (keypair is an index into wasmMemory.buffer that lands on &pk)
	 * typedef struct hydro_sign_keypair {
	 *     uint8_t pk[hydro_sign_PUBLICKEYBYTES];
	 *     uint8_t sk[hydro_sign_SECRETKEYBYTES];
	 * } hydro_sign_keypair;
	 */
	var secret_key = KP = new Uint8Array(wasmMemory.buffer, keypair+hydro_sign_PUBLICKEYBYTES, hydro_sign_SECRETKEYBYTES)
	str     = "";
	console.log("*****secretkey*****:");
	for (var x = 0; x<64; x++)
		str += secret_key[x] + " ";
	console.log(str);

	var chsize = challenge.length;
	rc = Module.ccall("hydro_sign_create",               // function name
					  "number",                          // return type of this function (int): 0 for success -1 for failure
					 ["number","array","number",         // [1] char *: signature,  [2] uint8_t[]: challenge, [3] size_t: challenge_size
					  "string","array"],                 // [4] char *: context,    [5] uint8_t[]: user's private key
					 [signature, challenge, chsize,      // signature, challenge, challenge_size
					  "context0", secret_key]);          // "context0", keypair.sk (user's private key)

	signature = SIG = new Uint8Array(wasmMemory.buffer, signature, hydro_sign_BYTES);
	console.log("****signature*****:");
	str = "";
	for (var x = 0; x<hydro_sign_BYTES; x++)
		str += signature[x] + " ";
	console.log(str);

	WS.send("login " + USER + "|" + btoa(String.fromCharCode.apply(null, signature)));
}

function register_onclick()
{
	var user        = $("#LBOX .usr").val();
	var password    = $("#LBOX .pwd").val();
	var auth        = user + "|" + password;
	var mem_offset  = hydro_sign_PUBLICKEYBYTES + hydro_sign_SECRETKEYBYTES;
	var keypair     = Module._malloc(mem_offset);
	var kp_seed     = Module._malloc(mem_offset+=hydro_sign_SEEDBYTES);

	// 1) Regenerate the seed from username|password
	rc = Module.ccall("hydro_pwhash_deterministic",      // function name
					  "number",                          // return type of this function (int): 0 for success -1 for failure
					 ["number","number",                 // uint8_t[]: kp_seed, int: sizeof(kp_seed[])
					  "string","number",                 // char *: auth,       int: strlen(username|password)
					  "string", "number",                // char *: context,    int: OPSLIMIT
					  "number","number"],                // size_t: memlimit,   uint8_t: nr_threads
					 [kp_seed, 32,                       // keypair seed, sizeof(kp_seed)
					  auth, auth.length,                 // username|password, strlen(username|password)
					  "context0", 1000, 0, 1]);

	var seed = new Uint8Array(wasmMemory.buffer, kp_seed, hydro_sign_SEEDBYTES)

	// 2) Recreate the private/public keypair from the seed
	Module.ccall("hydro_sign_keygen_deterministic",      // function name
					  null,                              // return type of this function (int): 0 for success -1 for failure
					 ["number","number"],                // argument types to hydro_sign_keygen_deterministic (two uint8arrays)
					 [keypair, kp_seed]);                // arguments to hydro_sign_keygen_deterministic()

	var public_key = new Uint8Array(wasmMemory.buffer, keypair, hydro_sign_PUBLICKEYBYTES);

	WS.send("register " + user + "|" + btoa(String.fromCharCode.apply(null, public_key)));
}

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

/* cookies are unique to a single struct connect on the server */
function rpc_set_cookie(av)
{
	localStorage.cookie = av[1]
	console.log("setting local cookie: " + av[1]);
}

// login_menu_onclick
function login() {
	if (USER && USER == localStorage.user) {
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
	localStorage.user = USER=$("#LBOX .usr").val();
	WS.send("login "    + USER + " " + $("#LBOX .pwd").val());
}
function wsregister() {
	$(".error").remove();
	USER=$("#LBOX .usr").val();
	WS.send("register " + USER + " " + $("#LBOX .pwd").val() + " " + $("#LBOX .mail").val())
}


/*
 * Server 
 */
function rpc_auth_challenge() {

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
