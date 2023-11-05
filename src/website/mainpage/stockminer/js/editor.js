/* ***********************************************************************************************
 *                                    CSS EDITOR
 ************************************************************************************************/
function editor_keyup()
{
	var editor = $("#editor")[0],tab = $("#morphtab")[0], new_class = $("#css-class")[0].value, old_class = tab.classList[0], utsave = $("#utsave")[0];
	css_editor.setValue(css_editor.getValue().replaceAll(tab.old_class, new_class));
	css_update(new_class);
	utsave.innerText   = 'Save';
	utsave.style.color = 'limegreen';
}
function editor_tab(e)
{
	$(".UTL").css("display", "none");
	switch (e) {
		case 1:
			$("#html-editor").css("display", "block");
			break;
		case 2:
			$("#css-editor").css("display", "block");
			break;
		case 2:
			$("#js-editor").css("display", "block");
			break;
	}
}
function css_save()
{
	var utsave = $("#utsave")[0], save = utsave.innerText, css = css_editor.getValue(), css_name = $("#css-class")[0].value;
	if (save == 'Saved')
		return;
	utsave.style.color = "green";
	utsave.innerText   = "Saved";
	if (!localStorage[css_name]) {
		var nr_css = localStorage.nr_table_css;
		if (!nr_css)
			nr_css = 1;
		else
			nr_css++;
		localStorage.nr_table_css = nr_css;
		console.log("css_save new: nr_css: " + nr_css + " TCLASS_LEN: " + TCLASS.length);
		nr_css += TCLASS.length-1;
		localStorage['TCLASS'+nr_css] = css_name;
		TCLASS[nr_css] = localStorage['TCLASS'+nr_css];
		Themes['Theme'+nr_css] = {"name":TCLASS[nr_css], callback:function(i){TableTheme(TCLASS[i.substr(5)])}};
	}
	/* Update the user's CSS */
	localStorage[css_name] = css;
	WS.send("css " + $("#morphtab")[0].old_class + " " + css);
}
function js_save()
{
	var jsave = $("#jsave")[0], save = jsave.innerText, js = js_editor.getValue(), js_name = $("#js-class")[0].value;
	if (save == 'Saved')
		return;
	jsave.style.color = "green";
	jsave.innerText   = "Saved";
	if (!localStorage[js_name]) {
		var nr_js = localStorage.nr_table_js;
		if (!nr_js)
			nr_js = 1;
		else
			nr_js++;
		localStorage.nr_table_js = nr_js;
		console.log("js_save new: nr_js: " + nr_js + " JCLASS_LEN: " + JCLASS.length);
		nr_js += JCLASS.length-1;
		localStorage['JCLASS'+nr_js] = js_name;
		JCLASS[nr_js] = localStorage['JCLASS'+nr_js];
		JEXT['Theme'+nr_js] = {"name":JCLASS[nr_js], callback:function(i){TableTheme(JCLASS[i.substr(5)])}};
	}
	/* Update the user's CSS */
	localStorage[css_name] = css;
	WS.send("css " + $("#morphtab")[0].old_class + " " + css);
}

function rpc_styles(av)
{
	for (var x=1; x<av.length; x++) {
		var css = av[x].split("-");
		CSS.push({name:css[0],pub:css[1]});
	}
}

function cssload()
{
	T['Styles'].clear();
	for(var x=0;x<CSS.length;x++)
		T['Watchlist-CSS'].rows.add([{"S":CSS[x].style,"C":CSS[x].owner,"T":CSS[x].type,"R":CSS[x].rating}]).draw()
}

function JSEditor(){}
function html_update(){}
function js_update(){}

var EDITOR     = { watchtable: { div: [ "#morphtab-box", "body > .watchtable-tools" ] }};
var EDITOR_OPS = { html: html_update, css: css_update, js: js_update};

function css_update(new_class)
{
	var editor = $("#editor")[0], css = editor['css'].getValue(), div = $(EDITOR[editor.component].div[0])[0];

	if (!new_class && !(new_class = $("#css-class").val()))
		return;
	if (!div.STYLE) {
		div.STYLE = document.createElement("style");
		div.STYLE.innerHTML = css;
		$("#css-editor").append(div.STYLE);
	} else
		div.STYLE.innerHTML = css;
	console.log("setting class: " + new_class);
	div.STYLE.className = "ut-"+new_class;
	div.old_class = div.classList[0];
	div.classList.replace(div.old_class,new_class);
	div.old_class = new_class;
}
var DIV;
function editor_close()
{
	var editor = $("#editor")[0], component = EDITOR[editor.component], div = component.div, parentDiv = component.parentDiv, sel;
	DIV = div;
	window.event.target.parentNode.style.display = "none";
	for (var x = 0; x<div.length; x++) {
		sel = div[x];
		if (sel.indexOf(">") == -1)
			$(parentDiv[0]).append($(sel).detach());
		else
			$(sel).remove();
	}

//	var tab = $($("#morphtab")[0].parentNode.parentNode.parentNode).detach();
//	$("#P0Q1q2").append(tab);
}

function Editor(component, lang)
{
	var editor = $("#editor")[0], component, div, sel;

	editor.style.display = 'block';
	if (component==0)
		component = EDITOR[editor.component];
	else {
		// switching components
		if (editor.component && editor.component != component) {

		}
		editor.component = component;
		component = EDITOR[component];
	}
	//	$(".UTR", editor).append($("#morphtab")[0].parentNode.parentNode.parentNode);
	div = component.div;
	for (var x = 0; x<div.length; x++) {
		sel = div[x];
		if (sel.indexOf(">") != -1)
			sel = $(sel).clone();
		else
			sel = $(sel).detach();
		$(".UTR", editor).append(sel);
	}

	if (!editor[lang]) {
		editor = editor[lang] = ace.edit(lang+"-editor", {mode:"ace/mode/"+lang, selectionStyle:"text"});
		editor.setTheme("ace/theme/dracula");
		EDITOR_OPS[lang]();
//		css_update();
		editor.commands.addCommand({
		    name:'save',
		    bindKey: {win: "Ctrl-S", "mac": "Cmd-S"},
		    exec: function(e) {
				EDITOR_OPS[lang]();
//				css_update();
			}
		});
		editor.textInput.getElement().onkeyup = function(){
			var utsave = $("#utsave")[0];
			utsave.style.color = "lightgreen";
			utsave.innerText   = "Save";
		}
	}
}
