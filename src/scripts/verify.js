const fs = require("fs");
const { exec } = require("child_process");

var highcaps   = fs.readFileSync("/stockminer/data/stocks/HIGHCAPS.TXT", 'utf8').split("\n");
var lowcaps    = fs.readFileSync("/stockminer/data/stocks/LOWCAPS.TXT", 'utf8').split("\n");

function csv_diff(ticker,csv1, csv2)
{
	for (var x =0; x<5; x++) {
		if (csv1[x] != csv2[x]) {
			c1 = parseFloat(csv1[x]);
			c2 = parseFloat(csv2[x]);
			if (Math.abs((((c1/c2)-1)*100.0)) >= 5.0) {
				console.log(ticker + " MISMATCH: " + " yahoo: " + csv1.join(",") + " WSJ: " + csv2.join(",") + " diff: " + (((c1/c2)-1)*100.0));
				var cmd = 'sed -i "s/' + csv2[0] + ".*/" + csv1.join(",") + '/g" ' + "/stockminer/stockdb/csv/" + ticker + ".csv";
				exec(cmd);
				return 1;
			}
		}
	}
	return 0;
}

function verify_eod(stocks)
{
	for (var x = 0; x<stocks.length; x++) {
		var ticker = stocks[x];
//		if (ticker != "DIOD")
//			continue;
		if (fs.existsSync("/stockminer/data/stocks/stockdb/yahoo/" + ticker + ".csv") == false)
			continue;
		var yahoo_csv  = fs.readFileSync("/stockminer/data/stocks/stockdb/yahoo/" + ticker + ".csv", 'utf8').split("\n");
		var wsj_csv    = fs.readFileSync("/stockminer/data/stocks/stockdb/csv/"   + ticker + ".csv", 'utf8').split("\n");
		yahoo_csv.pop();
		wsj_csv.pop();
		if (wsj_csv.length <= 10 || yahoo_csv.length < 1)
			continue;
		var csv1 = yahoo_csv[yahoo_csv.length-1].split(",");
		var csv2 = wsj_csv[wsj_csv.length-1].split(",").slice(0,-1);
		var date = wsj_csv[wsj_csv.length-2].split(",")[0];
		if (date == csv2[0]) {
			console.log("[" + ticker + "] CRITICAL DUPLICATE DATE ERROR: " + date + " " + "date: " + date + " csv1: " + csv1 + " csv2: " + csv2);
			continue;
		}
		csv_diff(ticker,csv1, csv2);
	}
}

function verify_csv(stocks)
{
	for (var x = 0; x<stocks.length; x++) {
		var ticker = stocks[x];
//		if (ticker != "TSLA")
//			continue;
		if (fs.existsSync("/stockminer/data/stocks/stockdb/csv/" + ticker + ".csv") == false)
			continue;
		var wsj_csv    = fs.readFileSync("/stockminer/data/stocks/stockdb/csv/"   + ticker + ".csv", 'utf8').split("\n");
		wsj_csv.pop();
		if (wsj_csv.length <= 10)
			continue;
		for (var y = 0; y<wsj_csv.length-1; y++) {
			var date1 = new Date(wsj_csv[y].split(",")[0]).getTime();
			var date2 = new Date(wsj_csv[y+1].split(",")[0]).getTime();
			if (date1 == date2 || date2 < date1) {
				console.log("REFETCH: " + ticker + "  CORRUPT DATE: " + wsj_csv[y].split(",")[0] + " vs: " + wsj_csv[y+1].split(",")[0] + " date1: " + date1 + " date2: " + date2);
				exec("python3 /stockminer/scripts/fetch.py " + ticker);
				break;
			}
		}
	}

}

verify_eod(highcaps);
verify_eod(lowcaps);

verify_csv(highcaps);
verify_csv(lowcaps);
exec("sed -i 's/,,,/d' /stockminer/data/stocks/stockdb/csv/*.csv");
