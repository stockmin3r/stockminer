/**********************************************
 * filename: mainpage/stockminer/js/options.js
 * License:  Public Domain
 *********************************************/
function init_options()
{
	T['OIC'] = $('#OIC').DataTable({
    "columns": [{"data":"oN"},{"data":"oP"},{"data":"oD"},{"data":"oV"},{"data":"oI"}],
	"columnDefs": [{"targets": 5, "visible":false}],
	"bPaginate":false,"order": [[4,"desc"]],
	"createdRow": function (row,data,index){opcolor(row,data,index)},
	"paging":false,"bDestroy":false,"info":false,"searching":false
	});
	T['VOLC'] = $('#VOLC').DataTable({
    "columns": [{"data":"oN"},{"data":"oP"},{"data":"oD"},{"data":"oV"},{"data":"oI"}],
	"columnDefs": [{"targets":5,"visible":false}],
	"createdRow": function (row,data,index){opcolor(row,data,index)},
	"bPaginate":false,"order":[[3,"desc"]],"paging":false,"bDestroy":false,"info":false,"searching":false});

	T['OIP'] = $('#OIP').DataTable({
    "columns": [{"data":"oN"},{"data":"oP"},{"data":"oD"},{"data":"oV"},{"data":"oI"}],
	"columnDefs": [{"targets": 5, "visible":false}],
	"createdRow": function (row,data,index){opcolor(row,data,index)},
	"bPaginate":false,"order": [[4,"desc"]],
	"paging":false,"bDestroy":false,"info":false,"searching":false
	});
	T['VOLP'] = $('#VOLP').DataTable({
    "columns": [{"data":"oN"},{"data":"oP"},{"data":"oD"},{"data":"oV"},{"data":"oI"}],
	"columnDefs": [{"targets":5,"visible":false}],
	"createdRow": function (row,data,index){opcolor(row,data,index)},
	"bPaginate":false,"order":[[3,"desc"]],"paging":false,"bDestroy":false,"info":false,"searching":false});

	$('#OIC tbody').on ('click', 'tr',function(){CUROP = T['OIC'].row(this).data().oN;opsel()});
	$('#VOLC tbody').on('click', 'tr',function(){CUROP = T['VOL'].row(this).data().oN;opsel()});
	T['OPCH'] = newtable("OPCH", "asc", [{"data":"oP"}, {"data":"oB"}, {"data":"oA"}, {"data":"oOI"}, {"data":"oIV"}, {"data":"oVOL"},{"data":"oS"},{"data":"oPP"},{"data":"oBP"},{"data":"oAP"},{"data":"oOIP"},{"data":"oIVP"},{"data":"oVOLP"}],6);
	T['OPCC'] = newtable("OPCC", "desc", [{"orderable":false,"data":null,"defaultContent":''},{"data":"oP"},{"data":"oS"},{"data":"oB"},{"data":"oH"},{"data":"oRC"},{"data":"oAPR"},{"data":"oRU"},{"data":"oARPu"},{"data":"oOTM"},{"data":"oND"},{"data":"oNDP"}],2);
}

function delta_color(d,r,i){
	var c = null;
    if      (d >= 10.0) c = 'limegreen2';
    else if (d >= 5.0)  c = 'green2';
	else if (d >  0.0)  c = 'yellow2';
	else if (d < 0.0)   c = 'brown';
	if (c)
		$('td',r).eq(i).addClass(c);
}
function opcolor(row,data,index) {delta_color(parseInt(data.oD), row, 2);contract_color(data.oN, row)}
function space(n)                {switch (n) {case 3:return"X";case 2:return"XX";case 1:return"XXX";default:return"";}}
function contract_color(C, row)
{
	var i = C.indexOf('2'),strike,slen,
	m = /[1-9]/.exec(C.substr(i+7));
	strike = m.index;
	slen   = C.substr(i+7+strike).indexOf('0');
	$('td', row).CNAME = C;
	C="<span class=optick>"+C.substr(0,i)+"</span><span class=hide>"+space(i)+"</span>"+C.substr(i);

/*	C="<span class=optick>"+C.substr(0,i)+"</span><span class=hide>"+space(i)+"</span>"+"<span class=purple>"+C.substr(i,2)+"</span><span class=blue2>"+C.substr(i+2,2)+
	  "</span><span class=yellow2>"+C.substr(i+4,2)+"</span><span class=none>"+C.substr(i+6,1)+C.substr(i+7,0)+"</span>" +
	  C.substr(i+7+1, strike-1) + "</span><span class=opstrike>" + C.substr(i+7+strike, slen) + "</span>" + C.substr(i+7+strike+slen);*/
	$('td', row)[0].innerHTML = C;
}

function ntm(idx,off,node)
{
	for (var x=0; x<6; x++) {
		var cell = node.row(idx).nodes().to$()[0].cells[x+off];
		cell.classList.add("NTM");
		cell.classList.remove("ITM");
		cell.classList.remove("OTM");
	}
}

function optab(av){
	var tgt = -1,
	curprice = parseFloat(av[3]);
	table(av);
	T['OPCH'].rows().every(function (index, element) {
		var strike = parseFloat(this.data().oS);
		if (tgt == -1)
			if (strike > curprice)
				tgt = index;
		if (strike >= curprice) {
			for (var x=0; x<13; x++) {
				if (x == 6)
					continue;
				this.row(index).nodes().to$()[0].cells[x].classList.add("OTM");
			}
		} else {
			for (var x=0; x<13; x++) {
				if (x == 6)
					continue;
				this.row(index).nodes().to$()[0].cells[x].classList.add("ITM");
			}
		}
	});
	for (var x=0; x<6; x++) ntm(tgt+x,   0, T['OPCH']);
	for (var x=0; x<6; x++) ntm(tgt+x-6, 7, T['OPCH']);
}

function opsel(){
	if      (OPSEL==OPCHART) WS.send("opchart " + CUROP + " OPCHART");
	else if (OPSEL==OPCHAIN) WS.send("OPCHAIN " + CUROP);
	else if (OPSEL==OPCC)    WS.send("opcc "    + CUROP);
}
function opchart() {WS.send("opchart " + CUROP + " OPCHART"); OPSEL=OPCHART}
function opchain() {WS.send("OPCHAIN " + CUROP);              OPSEL=OPCHAIN}
function opcc()    {WS.send("opcc "    + CUROP);              OPSEL=OPCC}
