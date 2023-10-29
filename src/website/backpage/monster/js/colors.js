/**********************************************
 * filename: backpage/monster/colors.js
 * License:  Public Domain
 *********************************************/
function bgcolor(v)
{
	var c;
	if (v == 100)
		c = 'g0';
	else if (v >= 90)
		c = 'g1';
	else if (v >= 80)
		c = 'g2';
	else if (v >= 70)
		c = 'Y';
	else if (v >= 60)
		c = 'O';
	else if (v >= 50)
		c = 'P';
	else if (v >= 0)
		c = 'S';
	return c;
}

function safetab_colors(row,data)
{
	var d63,d128,total,d63col,d128col;

	for (var x = 1; x<=5; x++) {
		total   = data['T_'+x];
		d63     = data['63d_'+x];
		d128    = data['128d_'+x];
		d63col  = (d63/total)*100;
		d128col = (d128/total)*100;
		$('td',row).eq((5*(x-1))+2).addClass(bgcolor(d63col));
		$('td',row).eq((5*(x-1))+3).addClass(bgcolor(d128col));
	}
}

function cellcolor(v, idx, row)
{
	var c;
    if (v == 100.0)
		c = 'darkgreen'
    else if (v >= 90)
		c = 'G';
	else if (v >= 80)
		c = 'B';
	else if (v >= 70)
		c = 'Y';
	else if (v > 1)
		c = 'w';
	else if (v == 0)
		c = 'brown2'
	$('td', row).eq(idx).addClass(c);
}

function esp_colors(row,data,index){
	var one = parseInt(data.O);
	var two = parseInt(data.W);
	var delta = Number(data.D);
	var next = parseFloat(data.next);
	var intra = parseFloat(data.intra);

	if (delta >= 5)
		$('td',row).eq(4).addClass('g');
	else if (delta < 0)
		$('td',row).eq(4).addClass('r');

	if (data.peak === "peak")
		$('td',row).eq(6).addClass('G');

	if (one == 9)
        $('td', row).eq(7).addClass('y');
	else if (one >= 10)
        $('td', row).eq(7).addClass('g');

	if (two <= 2)
        $('td', row).eq(8).addClass('o');
	else if (two == 3)
		$('td', row).eq(8).addClass('y');

	if (data.a1q1==="none")
		$('td',row).eq(9).addClass('grey2');
	else
		cellcolor(parseFloat(data.a1q1),9,row);

	/* A1Q2 */
	if (data.a1q2==="none")
		$('td',row).eq(10).addClass('grey2');
	else
		cellcolor(parseFloat(data.a1q2),10,row);

	if (data.a4q1==="none")
		$('td',row).eq(12).addClass('grey2');
	else
		cellcolor(parseFloat(data.a4q1),12,row);

	if (data.a4q2==="none")
		$('td',row).eq(13).addClass('grey2');
	else
		cellcolor(parseFloat(data.a4q2),13,row);

	if (parseFloat(data.delta) < 0 && parseFloat(data.next) < 0) {
		$('td',row).eq(22).addClass('y');
		yellow=1;
	}
	for (var x=0; x<15; x++) {
		day = parseFloat(data[(x+1)]);
		if (day == 0.0) {
			$('td',row).eq(23+x).addClass('grey2');
			$('td',row).eq(23+x).html("");
			yellow=0;
		} else if (yellow) {
			$('td',row).eq(23+x).addClass('y');
		} else if (day >= 5) {
			yellow=0;
			$('td',row).eq(23+x).addClass('g');
		}
	}
}

function crow(v,i,r)
{
	var c;
	if (v > -5.0 && v < 5.0)
		c='w';
	else if (v >= 5.0 && v < 10.0)
		c='L';
	else if (v >= 10.0)
		c='G';
	else if (v <= -5.0 && v > -10.0)
		c='O';
	else if (v <= -10.0 && v > -15.0)
		c='Y';
	else if (v <= -15.0)
		c='R';
	else
		c='w';
	$('td',r).eq(i).addClass(c);
}

function cmt_colors(row,data,index)
{
	var two = parseInt(data.two);
	if (two <= 2)
        $('td', row).eq(3).addClass('O');
	else if (two == 3)
		$('td', row).eq(3).addClass('Y');
	crow(data.high,  4, row);
	crow(data.low,   5, row);
	crow(data.daily, 7, row);
	crow(data.day3,  8, row);
	crow(data.day5,  9, row);
	crow(data.day8,  10, row);
	crow(data.day13, 11, row);
	crow(data.day21, 12, row);
}

function msr2_colors(row,data,index)
{
	var ANYDAY  = 2;
	var ANYRET  = 3;
	var ANYSTD  = 4;
	var ANYAVG  = 5;
	var A1Q1    = 6;
	var A1RET   = 7;
	var A1STD   = 8;
	var A1AVG   = 9;
	var A1TOTAL = 10;
	var A4Q1    = 11;
	var A4RET   = 12;
	var A4ATD   = 13;
	var A4AVG   = 14;
	var A4TOTAL = 15;
	var a1q1,a1q2,a4q1,a4q2;
	var count = 0;
	$('#MSR2').on('contextmenu', 'td', function(event){
		msr_ticker = $(this).parent().children("td:first").text();
    });
	var two = parseInt(data.two);
	if (two <= 2)
        $('td', row).eq(1).addClass('O');
	else if (two == 3)
		$('td', row).eq(1).addClass('Y');

	anyday = parseFloat(data.any);
	a1q1   = parseFloat(data.a1q1);
	a4q1   = parseFloat(data.a4q1);

	if (data.any === "none")
		$('td', row).eq(ANYDAY).addClass('grey');
	else
		cellcolor(anyday, ANYDAY, row);

	if (data.a1q1 === "none")
		$('td', row).eq(A1Q1).addClass('grey');
	else
		cellcolor(a1q1, A1Q1, row);

	if (data.a4q1 === "none")
		$('td', row).eq(A4Q1).addClass('grey');
	else
		cellcolor(a4q1, A4Q1, row);

}

function msr_colors(row,data,index)
{
	var a1q1,a1q2,a4q1,a4q2;
	var count = 0;
	$('#MSR').on('contextmenu', 'td', function(event){
		msr_ticker = $(this).parent().children("td:first").text();
    });
	var delta = parseFloat(data.delta);
	if (delta > 0) {
		$('td',row).eq(0).addClass('g');
		$('td',row).eq(1).addClass('g');
		$('td',row).eq(2).addClass('g');
	} else {
		$('td',row).eq(0).addClass('brown');
		$('td',row).eq(1).addClass('brown');
		$('td',row).eq(2).addClass('brown');
	}
	var one = parseInt(data.one);
	var two = parseInt(data.two);
	if (one == 9)
        $('td', row).eq(4).addClass('Y');
	else if (one >= 10)
        $('td', row).eq(4).addClass('G');
	if (two <= 2)
        $('td', row).eq(5).addClass('O');
	else if (two == 3)
		$('td', row).eq(5).addClass('Y');

// 13 th col first a1q anyday
	for (var x=0; x<6; x++) {
		switch (x) {
			case 0:
				a1q1 = parseFloat(data.a1q1_2020_any5);
				a1q2 = parseFloat(data.a1q2_2020_any5);
				if (data.a1q1_2020_any5 === "none")
					$('td', row).eq(x+count+13).addClass('grey');
				if (data.a1q2_2020_any5 === "none")
					$('td', row).eq(x+count+14).addClass('grey');
				break;
			case 1:
				a1q1 = parseFloat(data.a1q1_2019_any5);
				a1q2 = parseFloat(data.a1q2_2019_any5);
				if (data.a1q1_2019_any5 === "none")
					$('td', row).eq(x+count+13).addClass('grey');
				if (data.a1q2_2019_any5 === "none")
					$('td', row).eq(x+count+14).addClass('grey');
				break;
			case 2:
				a1q1 = parseFloat(data.a1q1_2018_any5);
				a1q2 = parseFloat(data.a1q2_2018_any5);
				if (data.a1q1_2018_any5 === "none")
					$('td', row).eq(x+count+13).addClass('grey');
				if (data.a1q2_2018_any5 === "none")
					$('td', row).eq(x+count+14).addClass('grey');
				break;
		}
		if (x < 3) {
			cellcolor(a1q1, x+count+13, row);
			cellcolor(a1q2, x+count+14, row);
		}
		count++;
	}
	count = 0;
	for (var x=0; x<6; x++) {
		switch (x) {
			case 0:
				a1q1 = parseFloat(data.a1q1_2020);
				a1q2 = parseFloat(data.a1q2_2020);
				if (data.a1q1_2020 === "none")
					$('td', row).eq(x+count+22).addClass('grey');
				if (data.a1q2_2020 === "none")
					$('td', row).eq(x+count+23).addClass('grey');
				break;
			case 1:
				a1q1 = parseFloat(data.a1q1_2019);
				a1q2 = parseFloat(data.a1q2_2019);
				if (data.a1q2_2019 === "none")
					$('td', row).eq(x+count+22).addClass('grey');
				if (data.a1q2_2019 === "none")
					$('td', row).eq(x+count+23).addClass('grey');
				break;
			case 2:
				a1q1 = parseFloat(data.a1q1_2018);
				a1q2 = parseFloat(data.a1q2_2018);
				if (data.a1q1_2018 === "none")
					$('td', row).eq(x+count+22).addClass('grey');
				if (data.a1q2_2018 === "none")
					$('td', row).eq(x+count+23).addClass('grey');
				break;
		}
		if (x < 3) {
			cellcolor(a1q1, x+count+22, row);
			cellcolor(a1q2, x+count+23, row);
		}
		count++;
	}
	count=0;
	for (var x=0; x<6; x++) {
		switch (x) {
			case 0:
				a4q1 = parseFloat(data.a4q1_2020);
				a4q2 = parseFloat(data.a4q2_2020);
				if (data.a4q1_2020 === "none") {
					$('td', row).eq(x+count+34).addClass('grey');
					a4q1=0;
				}
				if (data.a4q2_2020 === "none") {
					$('td', row).eq(x+count+35).addClass('grey');
					a4q2=0;
				}
				break;
			case 1:
				a4q1 = parseFloat(data.a4q1_2019);
				a4q2 = parseFloat(data.a4q2_2019);
				if (data.a4q1_2019 === "none") {
					$('td', row).eq(x+count+34).addClass('grey');
					a4q1=0;
				}
				if (data.a4q2_2019 === "none") {
					$('td', row).eq(x+count+35).addClass('grey');
					a4q2=0;
				}
				break;
			case 2:
				a4q1 = parseFloat(data.a4q1_2018);
				a4q2 = parseFloat(data.a4q2_2018);
				if (data.a4q1_2018 === "none") {
					$('td', row).eq(x+count+34).addClass('grey');
					a4q1=0;
				}
				if (data.a4q2_2018 === "none") {
					$('td', row).eq(x+count+35).addClass('grey');
					a4q2=0;
				}
				break;
		}
		if (x < 3) {
			cellcolor(a4q1, x+count+34, row);
			cellcolor(a4q2, x+count+35, row);
		}
		count++;
	}
	count = 0;
	for (var x = 0; x<6; x++) {
		switch (x) {
			case 0:
				a1q1 = parseFloat(data.a1q1_2020_any10);
				a1q2 = parseFloat(data.a1q2_2020_any10);
				if (data.a1q1_2020_any10 === "none")
					$('td', row).eq(x+count+46).addClass('grey');
				if (data.a1q2_2020_any10 === "none")
					$('td', row).eq(x+count+47).addClass('grey');
				break;
			case 1:
				a1q1 = parseFloat(data.a1q1_2019_any10);
				a1q2 = parseFloat(data.a1q2_2019_any10);
				if (data.a1q1_2019_any10 === "none")
					$('td', row).eq(x+count+46).addClass('grey');
				if (data.a1q2_2019_any10 === "none")
					$('td', row).eq(x+count+47).addClass('grey');
				break;
			case 2:
				a1q1 = parseFloat(data.a1q1_2018_any10);
				a1q2 = parseFloat(data.a1q2_2018_any10);
				if (data.a1q1_2018_any10 === "none")
					$('td', row).eq(x+count+46).addClass('grey');
				if (data.a1q2_2018_any10 === "none")
					$('td', row).eq(x+count+47).addClass('grey');
				break;
		}
		if (x < 3) {
			cellcolor(a1q1, x+count+46, row);
			cellcolor(a1q2, x+count+47, row);
		}
		count++;
	}
	count = 0;
	for (var x = 0; x<6; x++) {
		switch (x) {
			case 0:
				a1q1 = parseFloat(data.a1q1_2020_10);
				a1q2 = parseFloat(data.a1q2_2020_10);
				if (data.a1q1_2020_10 === "none")
					$('td', row).eq(x+count+52).addClass('grey');
				if (data.a1q2_2020_10 === "none")
					$('td', row).eq(x+count+53).addClass('grey');
				break;
			case 1:
				a1q1 = parseFloat(data.a1q1_2019_10);
				a1q2 = parseFloat(data.a1q2_2019_10);
				if (data.a1q1_2019_10 === "none")
					$('td', row).eq(x+count+52).addClass('grey');
				if (data.a1q2_2019_10 === "none")
					$('td', row).eq(x+count+53).addClass('grey');
				break;
			case 2:
				a1q1 = parseFloat(data.a1q1_2018_10);
				a1q2 = parseFloat(data.a1q2_2018_10);
				if (data.a1q1_2018_10 === "none")
					$('td', row).eq(x+count+52).addClass('grey');
				if (data.a1q2_2018_10 === "none")
					$('td', row).eq(x+count+53).addClass('grey');
				break;
		}
		if (x < 3) {
			cellcolor(a1q1, x+count+52, row);
			cellcolor(a1q2, x+count+53, row);
		}
		count++;
	}
	count = 0;
	for (var x = 0; x<6; x++) {
		switch (x) {
			case 0:
				a4q1 = parseFloat(data.a4q1_2020_10);
				a4q2 = parseFloat(data.a4q2_2020_10);
				if (data.a4q1_2020_10 === "none")
					$('td', row).eq(x+count+67).addClass('grey');
				if (data.a4q2_2020_10 === "none")
					$('td', row).eq(x+count+68).addClass('grey');
				break;
			case 1:
				a4q1 = parseFloat(data.a4q1_2019_10);
				a4q2 = parseFloat(data.a4q2_2019_10);
				if (data.a4q1_2019_10 === "none")
					$('td', row).eq(x+count+67).addClass('grey');
				if (data.a4q2_2019_10 === "none")
					$('td', row).eq(x+count+68).addClass('grey');
				break;
			case 2:
				a4q1 = parseFloat(data.a4q1_2018_10);
				a4q2 = parseFloat(data.a4q2_2018_10);
				if (data.a4q1_2018_10 === "none")
					$('td', row).eq(x+count+67).addClass('grey');
				if (data.a4q2_2018_10 === "none")
					$('td', row).eq(x+count+68).addClass('grey');
				break;

		}
		if (x < 3) {
			cellcolor(a4q1, x+count+67, row);
			cellcolor(a4q2, x+count+68, row);
		}
		count++;
	}
}

function SB_createRow(row,data,index){
	var a1q1,a1q2,a4q1,a4q2;
	var count = 0;
	var total_2020 = parseInt(data.total1);
	var total_2019 = parseInt(data.total2);
	var total_2018 = parseInt(data.total3);
	var total_2017 = parseInt(data.total4);

	var one = parseInt(data.one);
	var two = parseInt(data.two);
	if (one == 9)
        $('td', row).eq(7).addClass('Y');
	else if (one >= 10)
        $('td', row).eq(7).addClass('G');
	if (two <= 2)
        $('td', row).eq(8).addClass('O');
	else if (two == 3)
		$('td', row).eq(8).addClass('Y');

	$('td',row).eq(17).addClass('totals');
	$('td',row).eq(18).addClass('totals');
	$('td',row).eq(19).addClass('totals');
	$('td',row).eq(20).addClass('totals');
	$('td',row).eq(29).addClass('totals');
	$('td',row).eq(30).addClass('totals');
	$('td',row).eq(31).addClass('totals');
	$('td',row).eq(32).addClass('totals');

	for (var x=0; x<8; x++) {
		switch (x) {
			case 0:
				a1q1 = parseFloat(data.a1q1_2020);
				a1q2 = parseFloat(data.a1q2_2020);
				if (data.a1q1_2020 === "none")
					$('td', row).eq(x+count+9).addClass('grey');
				if (data.a1q2_2020 === "none")
					$('td', row).eq(x+count+10).addClass('grey');
				break;
			case 1:
				a1q1 = parseFloat(data.a1q1_2019);
				a1q2 = parseFloat(data.a1q2_2019);
				if (data.a1q1_2019 === "none")
					$('td', row).eq(x+count+9).addClass('grey');
				if (data.a1q2_2019 === "none")
					$('td', row).eq(x+count+10).addClass('grey');
				break;
			case 2:
				a1q1 = parseFloat(data.a1q1_2018);
				a1q2 = parseFloat(data.a1q2_2018);
				if (data.a1q1_2018 === "none")
					$('td', row).eq(x+count+9).addClass('grey');
				if (data.a1q2_2018 === "none")
					$('td', row).eq(x+count+10).addClass('grey');
				break;
			case 3:
				a1q1 = parseFloat(data.a1q1_2017);
				a1q2 = parseFloat(data.a1q2_2017);
				if (data.a1q1_2017 === "none")
					$('td', row).eq(x+count+9).addClass('grey');
				if (data.a1q2_2017 === "none")
					$('td', row).eq(x+count+10).addClass('grey');
				break;
		}
		if (x < 4) {
			cellcolor(a1q1, x+count+9, row);
			cellcolor(a1q2, x+count+10, row);
		}
		count++;
	}
	count = 0;
	for (var x = 0; x<8; x++) {
		switch (x) {
			case 0:
				a4q1 = parseFloat(data.a4q1_2020);
				a4q2 = parseFloat(data.a4q2_2020);
				if (data.a4q1_2020 === "none")
					$('td', row).eq(x+count+21).addClass('grey');
				if (data.a4q2_2020 === "none")
					$('td', row).eq(x+count+22).addClass('grey');
				break;
			case 1:
				a4q1 = parseFloat(data.a4q1_2019);
				a4q2 = parseFloat(data.a4q2_2019);
				if (data.a4q1_2019 === "none")
					$('td', row).eq(x+count+21).addClass('grey');
				if (data.a4q2_2019 === "none")
					$('td', row).eq(x+count+22).addClass('grey');
				break;
			case 2:
				a4q1 = parseFloat(data.a4q1_2018);
				a4q2 = parseFloat(data.a4q2_2018);
				if (data.a4q1_2018 === "none")
					$('td', row).eq(x+count+21).addClass('grey');
				if (data.a4q2_2018 === "none")
					$('td', row).eq(x+count+22).addClass('grey');
				break;
			case 3:
				a4q1 = parseFloat(data.a4q1_2017);
				a4q2 = parseFloat(data.a4q2_2017);
				if (data.a4q1_2017 === "none")
					$('td', row).eq(x+count+21).addClass('grey');
				if (data.a4q2_2017 === "none")
					$('td', row).eq(x+count+22).addClass('grey');
				break;

		}
		if (x < 4) {
			cellcolor(a4q1, x+count+21, row);
			cellcolor(a4q2, x+count+22, row);
		}
		count++;
	}
}
