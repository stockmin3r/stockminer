const fs = require("fs");

var calendar = fs.readFileSync("/stockminer/data/stocks/CALENDAR-2022.TXT", 'utf8').split("\n");

function calendar_weeks()
{
	var start_of_week = 0;
	for (var x = 0; x<calendar.length-1; x++) {
		var day1 = calendar[x];
		var day2 = calendar[x+1];
		if (!start_of_week)
			start_of_week = day1;
		d1 = parseInt(day1.split("-")[2]);
		d2 = parseInt(day2.split("-")[2]);
		if (d2-d1 >= 3) {
			console.log(start_of_week + " " + calendar[x]);
			start_of_week = 0;
		}
	}
}

calendar_weeks();
