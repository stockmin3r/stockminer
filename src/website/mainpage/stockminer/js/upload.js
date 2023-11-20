/*********************************************
 * filename: mainpage/stockminer/js/net.js
 * License:  Public Domain
 ********************************************/

/* upload object dialog box */
function upbox(type, action, workspace)
{
	var d = $("#upload").css("display", "block")[0], title, QGID;
	switch (type) {
		case OBJTYPE_IMG:  title = "Load Picture";break;
		case OBJTYPE_DOC:  title = "Load Doc/PDF";break;
		case OBJTYPE_VID:  title = "Load Video";break;
		case OBJTYPE_APP: {
			switch (action) {
				case ACTION_APP_PYSCRIPT_WORKSPACE:
					title = "Load PyScript";
					action = ACTION_APP_PYSCRIPT_WORKSPACE;
					break;
				case ACTION_APP_CANVAS_WORKSPACE:
					title = "Load Canvas";
					action = ACTION_APP_CANVAS_WORKSPACE;
					break;
			}
			break;
		}
	}
	if (!d.objtype)
		$(d).draggable();
	$("h2",d)[0].innerText = title;
	d.objtype = type;
	d.action  = action;
	if (workspace)
		QGID = workspace['ws'+workspace.current_workspace].id;
	d.QGID = QGID ? QGID : window.event.target.parentNode.parentNode.parentNode.id;
}

// GET /ws/action/objtype/filecode/filesize/filename/arg1/argX
//new WebSocket("wss://localhost:port/ws/action/objtype/filecode/filename/filesize/QGID/URL);
function upload()
{
	var n    = window.event.target,p=n.parentNode,r,C,addobj=1,QGID=p.QGID,
	file     = n.previousElementSibling.querySelector(".upload-file").files[0],
	filename = file.name.replaceAll(" ","_"),
	filetype = filename.split(".")[1],
	filecode = FILETYPES[filetype],
	filesize = file.size,
	objtype  = p.objtype,
	action   = p.action;

	r = new FileReader();
	r.onload = function(e) {
		var data      = e.target.result,ws,obj,URL = randstr();
		console.log("new websocket for action: " + action);
		ws            = new WebSocket("wss://localhost:port/ws/"+action+"/"+objtype+"/"+filecode+"/"+filesize+"/"+filename+"/"+QGID+"/"+URL+"/");
		ws.binaryType = "arrayBuffer";
		ws.onopen     = function() {
			ws.send(data);
			QGID = "#"+QGID;
			bshide(QGID);
			$(QGID.substr(0,QGID.indexOf('w'))).css("overflow","auto");
			switch (objtype) {
				case OBJTYPE_IMG:
					obj = new Image();
					obj.className = "ws-img o" + URL;
					obj.src = "data:image/png;base64," + btoa(String.fromCharCode.apply(null, new Uint8Array(data)));
					obj.width="200px";obj.height="200px";
					break;
				case OBJTYPE_APP:
					switch (action) {
						case ACTION_APP_CANVAS_WORKSPACE:
							obj = $("body > .canvas").clone()[0];
							obj.className = "canvas" + NR_PDF++ + " " + "ws-pdf o" + URL;
		//					newobj(obj,QGID,URL,[BLOBS[filetype]]);
							obj.style.display = 'block';
							$(QGID).append(obj);
							obj.C = C = new CanvasDesigner(obj);
							CANVAS.push(C);
		//					obj = $('<object data=/blob/'+URL+' type="application/pdf" width="100%" height="40vh">')[0];
							C.pdf.pdf = null;
							C.pdfHandler.load(C,data);
							addobj = 0;
							break;
						case ACTION_APP_PYSCRIPT_WORKSPACE:
							// pyscript/HTML
							var iframe = document.createElement("iframe");
							$(QGID).append(iframe);
							iframe.setAttribute("style","height:100%;width:100%;");
							iframe.contentWindow.document.open();
							iframe.contentWindow.document.write(data);
							iframe.contentWindow.document.close();
							addobj = 0;
							break;
					}
					break;
			}
			// adding object to a workspace
			if (addobj)
				newobj(obj,QGID,URL,ENC2OBJ[filetype]);
			p.style.display="none";
		};
	};
	if (ENCODINGS[filetype] == 'bin')
		r.readAsArrayBuffer(file);
	else
		r.readAsText(file)
}

function ws_send(url,data,cb)
{
	var ws = new WebSocket(WS_URL+url);
	ws.binaryType = 'arrayBuffer';
	ws.onopen = function() {
		if (cb && cb.pre)
			cb.pre();
		ws.send(data)
		if (cb && cb.post)
			cb.post();
	}
}

// GET /ws/action/objtype/filetype/filename/filesize/arg1/argX
/*
OBJTYPES      = { img:1, doc:2, vid:3, app:4, data:5},
OBJCODES      = { 1:'img', 2:'doc',3:'vid',4:'app',5:'data'},
FILETYPES     = { txt:1, html:2, pdf:3, png:4, jpg: 5, jpeg: 6, gif: 7, csv: 8, xlsx: 9 },
FILEGROUP     = { png:'img',jpg:'img',jpeg:'img',gif:'img',xlsx:'doc',pdf:'doc',csv:'data',html:'app' },
ENCODINGS     = { png:'bin',jpg:'bin',jpeg:'bin',gif:'bin',xlsx:'bin',pdf:'bin',csv:'txt',html:'txt',py:'txt'},

#define OBJTYPE_IMG                    1  // Image    - (png,jpg,jpeg)
#define OBJTYPE_DOC                    2  // Document - (pdf,docx)
#define OBJTYPE_VID                    3  // Video    - (mp4,etc)
#define OBJTYPE_GIF                    6  // GIF      - (gif animation)
#define OBJTYPE_DATA                   7  // Data     - process dataset (CSV,JSON,BSON,etc) to chart/table/app/?
#define OBJTYPE_EXCEL                  8  // Excel    - XLSX file (to be converted to HTML and acted acted on: turn into chart/table/app/?
#define OBJTYPE_APP                    9  // APP      - pyScript,C/C++,R,webRTC-pdf-mod
*/


function sendfile(args)
{
	if (!file || !(file=args.file))
		return;

	var data = args.data,
	obj      = args.obj,
	callback = args.callback,
	action   = args.action    ? args.action  : 0,
	objtype  = args.objtype   ? args.objtype : 0,
	filetype = args.filetype  ? FILETYPES[filename.split(".")[1]] : 0,
	filename = (args.filename ? args.filename:file.name).replaceAll(" ","_"),
	filesize = args.filesize  ? args.filesize:file.size,
	argv     = args.argv      ? args.argv:'',r,URL;

	if (!action || !objtype || !filetype || !filename || !filesize)
		return;

	if (data) {
		URL = action+"/"+objtype+"/"+filetype+"/"+filesize+"/"+filename+"/"+argv;
		ws_send(URL, data, callback);
		return;
	}

	r = new FileReader();
	r.onload = function(e) {
		data = e.target.result;
		if (obj)
			obj.style.display='block';
		filecode = FILETYPES[filetype];
		if (FILEGROUP[filetype] == 'img')
			obj.src = "data:image/" + filetype + ";base64," + btoa(String.fromCharCode.apply(null, new Uint8Array(data)));
		if (obj)
			obj.upload = function() {ws_send(action+"/"+objtype+"/"+filecode+"/"+filesize+"/"+filename+"/"+argv, data, callback)}
	}
	if (ENCODINGS[filetype] == 'bin')
		r.readAsArrayBuffer(file);
	else
		r.readAsText(file);
}

/* User Charts */
var CSV;
function upload_csv()
{
	var file = $("#upload-csv-file")[0], upload = $("#upload-csv")[0], QGID = upload.QGID, type, csv;
	if (file.files.length) {
		$("#upload-csv-file")[0].upload();
		return;
	}
	type = $(".upload-select option:selected", upload).val();
	csv  = $("textarea", upload).val();
	sendfile({action:ACTION_DATA_CHART_SQUEAK,objtype:OBJTYPE_DATA,filetype:FILETYPE_CSV,data:csv,filesize:csv.length,filename:"dataset.csv",args:QGID});
	CSV = csv;
	chart({ticker:"NewChart",type:type,dtype:"csv",div:"newchart-"+QGID,data:csv.replaceAll("\n", "\\n")});
}

/* rpc: qreload */
function rpc_qreload(av) {
	var path = av[1];
	if (!path)
		path = "";
	window.location.href = "/" + path // reload the page
}

/* If user uploads a pic then qexport() will manually call rpc_qreload() to load the newly created QPAGE (via URL)
 * Otherwise, the server will be instructed on using selecting a random SVG for the QuadVerse's menu "image"
 * and will therefore itself send a reload message as no image need be uploaded.
 */
function qexport(URL)
{
	var files = $("#QUPLOAD")[0].files, file=files?files[0]:0, data, r, ws, qpic = $("#QSEL option:selected").val();
	WS.send("qexport " + URL + " " + (file?'0':qpic) + " " + W.PID + " " + W.current_workspace);
	if (!file)
		return;
	sendfile({action:ACTION_IMG_QUADVERSE,file:file,objtype:OBJTYPE_IMG,callback:{post:function(){rpc_qreload([0, USER+"/"+URL])}}});
/*	r = new FileReader();
	r.onload = function(e) {
		data          = e.target.result;
		ws            = new WebSocket("wss://localhost:port/ws/4/-/"+file.size+"/-/"+URL+"/");
		ws.binaryType = "arrayBuffer";
		ws.onopen     = function() {
			ws.send(data);
			rpc_qreload([0, USER+"/"+URL]);
		}
	}
	r.readAsArrayBuffer(file);*/
}
