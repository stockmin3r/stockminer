require('dotenv').config();
const Discord      = require('discord.js');
const bot          = new Discord.Client({ intents: [
  Discord.GatewayIntentBits.Guilds,
  Discord.GatewayIntentBits.GuildMessages,
  Discord.GatewayIntentBits.DirectMessages,
  Discord.GatewayIntentBits.GuildBans,
  Discord.GatewayIntentBits.MessageContent,
]});
const TOKEN        = process.env.TOKEN;
const fs           = require('fs');
const request      = require('request');
const cheerio      = require('cheerio');
const moment       = require('moment-timezone');

var stockArrayStr  = fs.readFileSync('stocks.txt').toString().split("\n");
var stockArrayInt  = Array(stockArrayStr.length).fill(0);
var newsAlerts     = [];
var newsSeen       = [];
var companyList    = fs.readFileSync('companyList.txt').toString().split("\n");
var main_channel;
var last_offering_s3  = "";
var last_offering_6k  = "";
var last_fda          = "";
var last_halt         = "";
var offering_timer_s3 = 0;
var offering_timer_6k = 0;
var halt_timer        = 0;
var fda_timer         = 0;
var boot_halt         = 1;
var boot_fda          = 1;
var boot_offering_s3  = 1;
var boot_offering_6k  = 1;
var MAX_IGNORE        = 5;
var ignbuf            = [];
var nr_ignore         = 0;
var nr_alerts         = 0;

bot.login(TOKEN);
bot.on('ready', () => {
	console.info(`Logged in as ${bot.user.tag}!`);
//	bot.channels.cache.get("1171834979936911443").send("Panda is uncensored - Ask panda anything!");
	const config = ini.parse(fs.readFileSync('./config.ini', 'utf-8'));
	const allowedChannels = config.channels.split(',');
});

function sendmsg(msg)
{
	main_channel.send(msg);
	test_channel.send(msg);
}

function halt_code(code)
{
	var codeStr = "";
	var use_code = 1;

	if (code.includes("T1")) {
		codeStr = "T1 ";
		use_code = 0;
	}
	if (code.includes("T2")) {
		codeStr += "T2 ";
		use_code = 0;
	}
	if (code.includes("T12")) {
		codeStr += "T12 ";
		use_code = 0;
	}
	if (code.includes("H10")) {
		codeStr += "H10";
		use_code = 0;
	}
	if (use_code)
		return code;
	return codeStr;
}

function halt_ignored(stock)
{
	for (x=0; x<MAX_IGNORE; x++) {
		if (ignbuf[x] === stock)
			return 1;
	}
	return 0;
}

var haltReq = {"id": "2","method":"BL_TradeHalt.GetTradeHalts","params":"[]","version":"1.1"};

function do_halt(channel, timer)
{
	request({
		headers: {
			'Referer': 'http://www.nasdaqtrader.com/trader.aspx?id=TradeHalts'
		},
		uri: 'http://www.nasdaqtrader.com/RPCHandler.axd',
		json: haltReq,
		method: 'POST'
	}, function (err, result, html) {
	if (typeof html === 'undefined')
		return false;
	var strtab = JSON.stringify(html);
	var table  = strtab.substring(11, strtab.length-31);
	const $    = cheerio.load(table);
	var count  = 0;
	var col    = 0;
	var warn   = 0;
	var date   = "";
	var halt   = "";
	var ignore_row = 0;

 	$("table > tbody > tr > td").each((index, element) => {
		if (timer && ignore_row)
			return false;
		if (col == 0) {
			halt = "```css\n[HALT] ";
			col++;
		} else if (col == 1) { /* DATE */
			date = $(element).text().substring(0, 8);
			col++;
			if (timer && !(date === last_halt)) {
				warn = 1;
				last_halt = date;
			}
			halt += "[" + date + "]";
		} else if (col == 2) { /* STOCK */
			halt += " " + $(element).text() + " ";
			if (halt_ignored($(element).text())) {
				ignore_row = 1;
				col++;
				if (warn)
					return false;
				return true;
			}
			col++;
		} else if (col == 5) { /* CODE */
			halt += "[CODE: " + $(element).text() + "]```";
			col++;
		} else if (col == 9) {
			if (ignore_row) {
				col = 0;
				count++;
				ignore_row = 0;
				return true;
			}
			col = 0;
			count++;
			if ((timer && !warn) || boot_halt) {
				if (boot_halt)
					boot_halt = 0;
				return false;
			}
			channel.send(halt);
			if (warn)
				return false;
		} else {
			col++;
		}
		if (count > 5)
			return false;
	 });
});
}

var news_dict = {};

function do_news(stock)
{
	request('https://finviz.com/quote.ashx?t=' + stock, function(error, resp, html) {
	if (typeof html === 'undefined')
		return false;
	const $  = cheerio.load(html);
	var date = '';
	var news = '';
	var col  = 0;
	var links = {};

	$('#news-table > tbody > tr > td').each(function(i, element) {
		if (col == 0) {
			date = $(this).text();
			col++;
			if (news_dict[stock] != date) {
				warn = 1;
				news = '```diff\n+ ' + date;
				news_dict[stock] = date;
			} else {
				return false;
			}
		} else if (col == 1) {
			col++;
			news = date + ' [' + $(this).text() + '](' + $(this).find('a').attr('href') + ')';
			main_channel.send(news);
		}
    });
});
}

function do_alert(stock)
{
	newsAlerts[nr_alerts++] = stock;
}

function do_offering_6k(channel, timer, num)
{
	request('https://sec.report/Form/6-K', function(error, resp, html) {
	if (typeof html === 'undefined')
		return false;
    const $      = cheerio.load(html);
	var offering = "```diff\n- ";
	var col      = 0;
	var count    = 0;
	var warn     = 0;
	var date     = "";

    $('.table > tbody > tr > td').each(function(i, element) {
		if (col == 0) {
			offering = "```diff\n- " + $(this).text();
			date = $(this).text();
			col++;
			if (timer && !(date === last_offering_6k)) {
				warn = 1;
				offering = "```diff\n- [OFFERING 6-K] " + date;
				last_offering_6k = date;
			}
		} else if (col == 1) {
			col++;
		} else {
			offering += $(this).text();
			col = 0;
			offering += "```";
			if ((timer && !warn) || boot_offering_6k) {
				if (boot_offering_6k)
					boot_offering_6k = 0;
				return false;
			}
			channel.send(offering);
			if (warn) {
				return false;
			}
			offering = "";
			count++;
		}
		if (count >= 5)
			return false;
    });
});
}

function do_offering_s3(channel, timer)
{
	request('https://sec.report/Form/S-3', function(error, resp, html) {
	if (typeof html === 'undefined')
		return false;
    const $      = cheerio.load(html);
	var offering = "```diff\n- ";
	var col      = 0;
	var count    = 0;
	var warn     = 0;
	var date     = "";

    $('.table > tbody > tr > td').each(function(i, element) {
		if (col == 0) {
			offering = "```diff\n- " + $(this).text();
			date = $(this).text();
			col++;
			if (timer && !(date === last_offering_s3)) {
				warn = 1;
				offering = "```diff\n- [OFFERING S-3] " + date;
				last_offering_s3 = date;
			}
		} else if (col == 1) {
			col++;
		} else {
			offering += $(this).text();
			col = 0;
			offering += "```";
			if ((timer && !warn) || boot_offering_s3) {
				if (boot_offering_s3) {
					boot_offering_s3 = 0;
				}
				return false;
			}
			channel.send(offering);
			if (warn) {
				return false;
			}
			offering = "";
			count++;
		}
		if (count >= 5)
			return false;
    });
});
}

/*
 * News Timer
 */
setInterval(function() {
	for (let x=0; x<newsAlerts.length; x++) {
		do_news(newsAlerts[x]);
	}
}, 5000);


/*
 * S-3 Offering Timer
 */
setInterval(function() {
	if (offering_timer_s3)
		do_offering_s3(main_channel, 1);
}, 5000);

/*
 * FDA Timer
 */
setInterval(function() {
	if (fda_timer)
		do_fda(1);
}, 5000);

/*
 * 8-K Offering Timer
 */
setInterval(function() {
	if (offering_timer_6k)
		do_offering_6k(main_channel, 1);
}, 5000);

/*
 * Trade Halt Timer
 */
setInterval(function() {
	if (halt_timer)
		do_halt(main_channel, 1);
}, 3000);

/*
 * Timezone Timer
 */
setInterval(function() {
	var time = parseInt(moment().tz("America/New_York").format("HH"), 10);
	if (time >= 4 && time < 20) {
		offering_timer_s3 = 1;
		offering_timer_6k = 0;
		halt_timer     = 1;
		fda_timer      = 1;
	} else {
		offering_timer_s3 = 0;
		offering_timer_6k = 0;
		halt_timer     = 0;
		fda_timer      = 0;
	}
}, 10000);

function halt_ignore(stock)
{
	if (nr_ignore++ > MAX_IGNORE)
		nr_ignore = 0;
	ignbuf[nr_ignore] = stock;
}

function do_short(stock)
{
    request('https://fintel.io/ss/us/' + stock, function(error, resp, html) {
    const $      = cheerio.load(html);
    var col      = 0;
	var msg      = '';

    $('.table-sm > tbody > tr > td').each(function(i, element) {
		if (col == 11) {
			msg = "```md\n# (" + stock + ") Short Volume Ratio: " + $(this).text() + "```";
			main_channel.send(msg);
			return false;
		} else {
			col++;
		}
	});
});
}

function list_alerts()
{
	var alerts = '``` ';
	for (let x = 0; x<newsAlerts.length; x++) {
		alerts += newsAlerts[x] + ', ';
	}
	alerts += '```';
	main_channel.send(alerts);
}

function do_fda(timer)
{
	request('https://www.accessdata.fda.gov/scripts/cder/daf/index.cfm?event=report.page', function(error, resp, html) {
	if (typeof html === 'undefined')
		return false;
    const $      = cheerio.load(html);
	var col      = 0;
	var warn     = 0;
	var msg      = '';
	var drug     = '';
	var company  = '';
	var count    = 0;

    $('table > tbody > tr > td').each(function(i, element) {
		if (col == 1) {
			col++;
			drug = $(this).text();
			if (timer && !(drug === last_fda)) {
				warn = 1;
				last_fda = drug;
			}
		} else if (col == 4) {
			col++;
			company = $(this).text();
		} else if (col == 6) {
			col = 0;
			if ($(this).text().indexOf("Approved") != -1)
				msg = '```diff\n+ [FDA Approval] [' + drug + '] ' + company + '[APPROVED]```';
			else
				msg = '```diff\n- [FDA Approval] [' + drug + '] ' + company + '[DENIED]```';
			if ((timer && !warn) || boot_fda) {
				if (boot_fda)
					boot_fda = 0;
				return false;
			}
			main_channel.send(msg);
			if (warn)
				return false;
			if (count++ > 5)
				return false;
		} else {
			col++;
		}
    });
});
}

function do_finviz(stock)
{
	request('https://finviz.com/quote.ashx?t=' + stock, function(error, resp, html) {
    const $      = cheerio.load(html);
	var col      = 0;
	var count    = 0;
	var warn     = 0;

    $('.snapshot-table2 > tbody > tr > td').each(function(i, element) {
		if (col == 9) {
			col++;
			if (row == 1)
				msg = "```diff\n- Float: " + $(this).text();
			else if (row == 2)
				msg += "Short: " + $(this).text();
		} else if (col == 11) {
			row++;
		} else {
			col++;
		}
    });
});
}

function do_help()
{
	var help = "";

	help += "```diff\n+ !alert [STOCK] poll finviz every second for latest news\n  ";
	help += "!als (list news alerts)\n  "
	help += "!fda (list FDA drug approvals)\n "
	help += "!s3 (prints last 5 S-3 filings)\n  "
	help += "!6k (prints last 5 6-K filings)\n  "
	help += "!short [STOCK] (prints short percentage of previous day)\n "
	help += "!halt (prints last 5 trade halts)\n  "
	help += "!img [STOCK] (prints graph of STOCK)\n  "
	help += "!ignore [STOCK] (ignore trade halt stock)```";
	main_channel.send(help);
}

/*
 * Message Handler
 */
bot.on('messageCreate', msg => {
	var argv = "";
	var count = 0;
	console.log(msg.content);
	main_channel = msg.guild.channels.cache.find(channel => channel.name === "general");
	console.log(msg.content);
	if (msg.author.bot)
		return;
	argv = msg.content.slice().trim().split(/ +/g);
	if (msg.content.startsWith('!img')) {
		var url = "https://finviz.com/chart.ashx?t=" + argv[1] + "&ty=c&ta=1&p=d&s=l.png";
		msg.channel.send('', {files: [url]});
	} else if (msg.content.startsWith('!alert')) {
		do_alert(argv[1]);
	} else if (msg.content.startsWith('!halt')) {
		do_halt(main_channel, 0);
	} else if (msg.content.startsWith('!short')) {
		do_short(argv[1]);
	} else if (msg.content.startsWith("!s3")) {
		do_offering_s3(msg.channel, 0);
	} else if (msg.content.startsWith("!6k")) {
		if (argv[1])
			count = parseInt(argv[1], 10);
		do_offering_6k(msg.channel, count);
	} else if (msg.content.startsWith("!ignore")) {
		halt_ignore(argv[1]);
	} else if (msg.content.startsWith("!help")) {
		do_help();
	} else if (msg.content.startsWith("!news")) {
		do_news(argv[1]);
	} else if (msg.content.startsWith("!lsa")) {
		list_alerts();
	} else if (msg.content.startsWith("!fda")) {
		do_fda(0);
	} else if (msg.content.startsWith("!msg")) {
		do_msg(msg.author.id);
	}
});
