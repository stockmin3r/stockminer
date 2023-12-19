#include <conf.h>
#include <extern.h>

static struct server *SERVER;

int    holiday = 1;
time_t current_timestamp;
time_t current_minute;     // current unix timestamp      (UTC)
int    current_minute_utc; // current minute of the hour  (UTC)
int    current_hour;       // current hour   of the day   (UTC)
int    current_day;        // current day    of the week  (UTC)
int    current_week;       // current week   of the month (UTC)
int    current_month;      // current month  of the year  (UTC)
int    trading_day;        // offset from the start of the year (trading days only, needs to go in an eventual market struct)

int TODAY_IS_SATURDAY;
int TODAY_IS_SUNDAY;
int TODAY_IS_MONDAY;

char         **DAYS;
int            nr_trade_days;
char          *QDATE[3];
char           current_date[64];
time_t         QDATESTAMP[3];
struct week    WEEKS[52];
char *days[]  = { "Mon", "Tue", "Wed", "Thu", "Fri" };

struct tz {
	const char *tzname;
	int         utc_offset;
};

struct tz timezones[] =
{
	{ "EST",  5   },
	{ "AEST", 10  }
};

char *WEEKDAYS[] = {
	"MONDAY",
	"TUESDAY",
	"WEDNESDAY",
	"THURSDAY",
	"FRIDAY",
	"SATURDAY",
	"SUNDAY"
};

char *hours[] = { "04:15:", "04:30:", "04:45:", "05:00:", "05:15:", "05:30:", "05:45:", "06:00:",
                  "06:15:", "06:30:", "06:45:", "07:00:", "07:15:", "07:30:", "07:45:", "08:00:",
                  "08:15:", "08:30:", "08:45:", "09:00:", "09:15:", "09:30:", "09:45:", "10:00:",
                  "10:15:", "10:30:", "10:45:", "11:00:", "11:15:", "11:30:", "11:45:", "12:00:",
                  "12:15:", "12:30:", "12:45:", "13:00:", "13:15:", "14:30:", "13:45:", "14:00:",
                  "14:15:", "15:30:", "15:45:", "15:00:", "15:15:", "15:30:", "15:45:", "16:00:",
                  "16:15:", "17:30:", "17:45:", "17:00:", "17:15:", "17:30:", "17:45:", "18:00:",
                  "18:15:", "18:30:", "19:45:", "19:00:", "19:15:", "19:30:", "19:45:", "20:00:" };

/*
 * - time():          UTC (epoch) in seconds
 * - gettimeofday():  UTC (epoch) in seconds + microseconds
 * - clock_gettime(): UTC (epoch) in seconds + nanoseconds
 */

void load_weeks()
{
	struct week *week;
	char         buf[64 KB];
	char        *line, *p;
	time_t       unix_today = QDATESTAMP[1];
	int64_t      filesize;
	int          nr_weeks = 0;

	filesize = fs_readfile_str((char *)STOCKS_WEEKS_PATH, buf, sizeof(buf));
	if (!filesize) {
		printf(BOLDRED "load_weeks(): corrupt stocks/WEEKS.TXT file" RESET "\n");
		return;
	}
	line = buf;
	while ((p=strchr(line, '\n'))) {
		week       = &WEEKS[nr_weeks];
		*(line+10) = '\0';
		*p++       = '\0';
		week->start_date_string = strdup(line);
		week->end_date_string   = strdup(line+11);
		week->start_date_unix   = str2unix(week->start_date_string);
		week->end_date_unix     = str2unix(week->end_date_string);
		if (current_week == -1 && unix_today >= week->start_date_unix && unix_today <= week->end_date_unix)
			current_week = nr_weeks;
		nr_weeks++;
		line = p;
	}
}

struct calendar {
	char *trading_days_filename;
	int   country_id;
};

struct calendar trading_calendar[] = 
{
	{ "US_TRADING_DAYS.TXT", US },
	{ "WW_TRADING_DAYS.TXT", WW }
};

void time_load_EOD()
{
	char     *date;
	char      buf[32 KB];
	char      path[256];
	char     *p;
	struct tm utc_tm;
	time_t    current_unix;
	int64_t   filesize;
	int       nr_trading_days = 0;

	/*
	 * The current date (%Y-%m-%d format) (in UTC) eg: 2023-12-09 also represents the "next EOD"
	 * since all markets will finish by the end of this UTC date.
 	 */
	time(&current_unix);
	gmtime_r(&current_unix, &utc_tm);
	sprintf(current_date, "%d-%d-%d", utc_tm.tm_year+1900, utc_tm.tm_mon+1, utc_tm.tm_mday);
	current_month = utc_tm.tm_mon;

	for (int x = 0; x<sizeof(trading_calendar)/sizeof(struct calendar); x++) {
		struct calendar *calendar = &trading_calendar[x];

		snprintf(path, sizeof(path)-1, "%s/%s", STOCKS_DIR_PATH, calendar->trading_days_filename);
		filesize = fs_readfile_str(path, buf, sizeof(buf));
		if (!filesize) {
			printf(BOLDRED "[-] time_load_EOD(): file read error" RESET "\n");
			exit(-1);
		}

		nr_trading_days = cstring_line_count(buf);
		if (nr_trading_days <= 0 || nr_trading_days > 365) {
			printf(BOLDRED "[-] time_load_EOD(): corrupt stocks/DAYS.TXT file" RESET "\n");
			exit(-1);
		}
		DAYS = (char **)malloc(sizeof(char *) * nr_trading_days);
		if (!DAYS)
			exit(-1);

		nr_trading_days = 0;
		date = buf;
		while ((p=strchr(date, '\n'))) {
			*p++ = 0;
//			printf("[%d] %s vs %s %d %d\n", x, date, current_date, strlen(date), strlen(current_date));
			DAYS[nr_trading_days] = strdup(date);
			nr_trading_days++;
			date = p;
		}

		for (int y = 0; y < nr_trading_days; y++) {
			if (!strncmp(DAYS[y], current_date, strlen(current_date))) {
				char *previous_trading_day = DAYS[y-1];
				char *current_trading_day  = DAYS[y];
				char *next_trading_day     = DAYS[y+1]; // XXX: must fix for wraparound for next day in the next year
				market_set_EOD(current_unix, calendar->country_id, previous_trading_day, current_trading_day, next_trading_day);
				break;
			}

			/*
			 * If DAYS[y] has overshot the current_date that means it's been a holiday or it's the weekend
			 * This is only relavant for non 24-7 markets
			 */
			time_t utc_day = str2utc(DAYS[y]);
//			printf("utc_day: %lu vs: curunix: %lu DAYS[y]: %s\n", utc_day, current_unix, DAYS[y]);
			if (utc_day > current_unix) {
				market_set_EOD(current_unix, calendar->country_id, DAYS[y-1], DAYS[y], DAYS[y+1]);
				break;
			}
		}
	}
}

void time_EOD()
{
	char timestr[64];

	QDATESTAMP[0] = QDATESTAMP[1];
	QDATESTAMP[1] = QDATESTAMP[2];
	QDATESTAMP[2] = QDATESTAMP[1] + (24*3600);
	printf(BOLDMAGENTA "time_EOD: QDATE[0]: %s QDATE[1]: %s QDATE[2]: %s\n", QDATE[0], QDATE[1], QDATE[2]);
	QDATE[0]      = QDATE[1];
	QDATE[1]      = QDATE[2];
	QDATE[2]      = strdup(unix2str(QDATESTAMP[2], timestr));
	printf(BOLDMAGENTA "time_EOD: QDATE[0]: %s QDATE[1]: %s QDATE[2]: %s\n", QDATE[0], QDATE[1], QDATE[2]);
}

void set_current_minute()
{
	struct tm       ltm;
	time_t          time_start,time_now, time_diff;
	char            timestr[256];

	memset(&ltm, 0, sizeof(ltm));

	/* current minute */
	time_now          = get_current_time(timestr);
	strptime(timestr, "%Y-%m-%d %H:%M", &ltm);
	ltm.tm_isdst      = -1;
	time_start        = mktime(&ltm);
	time_diff         = (time_now-time_start)/60;
	time_diff        *= 60;
	current_timestamp = (time_now-(time_now%60));
	current_minute    = current_timestamp;
//	printf("(start: %s) time_now: %lu time_start: %lu, diff: %d current: %lu\n", timestr, time_now, time_start, time_now-time_start, current_timestamp);
}

time_t get_current_time(char *timestr)
{
	struct tm  ltm;
	struct tm *local_tm;
	time_t     epoch;

	epoch    = time(NULL);
	local_tm = localtime_r(&epoch, &ltm);
	strftime(timestr, 18,"%Y-%m-%d 09:00", local_tm);
	return mktime(local_tm);
}

time_t get_timestamp()
{
	struct tm  ltm;
	struct tm *local_tm;
	time_t     utc_time;

	utc_time = time(NULL);
	local_tm = localtime_r(&utc_time, &ltm);
	local_tm->tm_sec += SERVER->TIMEZONE;
	return mktime(local_tm);
}

char *ny_date(char *buf)
{
	struct tm  ltm;
	struct tm *local_tm;
	time_t     utc_time;
	int        offset = SERVER->TIMEZONE;
	char       timestr[16];

	utc_time          = time(NULL);
	utc_time         += SERVER->TIMEZONE;
	local_tm          = localtime_r(&utc_time, &ltm);
	local_tm->tm_sec += offset;
	strftime(timestr, 16, "%m/%d", local_tm);
	// 04/04
	// 04/11
	buf[5] = 0;
	if (*(timestr) == '0') {
		*buf = *(timestr+1);
		*(buf+1) = '/';
		if (*(timestr+3) == '0')
			*(buf+2) = *(timestr+4);
		else {
			*(buf+2) = *(timestr+3);
			*(buf+3) = *(timestr+4);
		}
	} else {
		// 12/12
		*buf = *timestr;
		*(buf+1) = *(timestr+1);
		*(buf+2) = '/';
		if (*(timestr+3) == '0')
			*(buf+3) = *(timestr+4);
		else {
			*(buf+3) = *(timestr+3);
			*(buf+3) = *(timestr+4);
		}
	}
	printf("date: %s\n", buf);
	return buf;
}

time_t ny_time(char *timestr)
{
	struct tm   ltm;
	char        tbuf[32];
	time_t      timestamp;

	memset(&ltm, 0, sizeof(ltm));
	snprintf(tbuf, sizeof(tbuf)-1, "%s 00:00", timestr);
	strptime(tbuf, "%Y-%m-%d %H:%M", &ltm);
	ltm.tm_isdst = -1;
	timestamp = mktime(&ltm);
	return timestamp+SERVER->UTCOFF;
}

time_t str2utc(char *timestr)
{
	struct tm ltm;

	memset(&ltm, 0, sizeof(ltm));
	strptime(timestr, "%Y-%m-%d", &ltm);
	ltm.tm_isdst = -1;
	return timegm(&ltm);
}

time_t str2unix(char *timestr)
{
	struct tm ltm;

	memset(&ltm, 0, sizeof(ltm));
	strptime(timestr, "%Y-%m-%d", &ltm);
	ltm.tm_isdst = -1;
	return mktime(&ltm);
}

time_t str2unix2(char *timestr)
{
	struct tm ltm;

	memset(&ltm, 0, sizeof(ltm));
	strptime(timestr, "%m/%d/%y", &ltm);
	ltm.tm_isdst = -1;
	return mktime(&ltm);
}

char *unix2str(time_t timestamp, char *timestr)
{
	struct tm *tm, ltm;

	tm = localtime_r(&timestamp, &ltm);
	strftime(timestr, 32, "%Y-%m-%d", tm);
	return (timestr);
}

char *unix2str2(time_t timestamp, char *timestr)
{
	struct tm *tm, ltm;

	tm = localtime_r(&timestamp, &ltm);
	tm->tm_sec += SERVER->TIMEZONE;
	strftime(timestr, 32, "%Y-%m-%d %H:%M", tm);
	return (timestr);
}

char *unix2str3(time_t timestamp, char *timestr)
{
	struct tm *tm, ltm;

	tm = localtime_r(&timestamp, &ltm);
	tm->tm_sec += SERVER->TIMEZONE;
	strftime(timestr, 32, "%b%d", tm);
	return (timestr);
}

/*
 * convert from 1/31/21 to 2021-01-31
 */
char *MDY2YMD(char *from, char *to)
{
	char *date[4];
	char *year, *month, *day, *to_date = to;

	from = strdup(from);

	if (!cstring_split(from, date, 3, '/')) {
		*to = 0;
		return "";
	}

	month = date[0];
	day   = date[1];
	year  = date[2];

	*to++ = '2';
	*to++ = '0';
	*to++ = year[0];
	*to++ = year[1];
	*to++ = '-';
	*to++ = month[0];
	if (month[1] != '\0')
		*to++ = month[1];
	*to++ = '-';
	*to++ = day[0];
	if (day[1] != '\0')
		*to++ = day[1];
	return (to_date);
}


/* 2021-12-01 */
int splitdate_YMD(char *date, int *year, int *month, int *day)
{
	char *p = strchr(date, '-');
	int y,m,d;
	if (!p)
		return 0;

	*year  = y = atoi(date);
	*month = m = atoi(p+1);
	p      = strchr(p+1, '-');
	if (!p)
		return 0;
	*day = d = atoi(p+1);
	switch (y) {
		case 2020: y = 0; break;
		case 2021: y = 1; break;
		case 2022: y = 2; break;
		default: return 0;
	}
	*year = y;
	if (y < 0 || y > 2)
		return 0;
	if (m < 0 || m > 12)
		return 0;
	if (d < 0 || d > 31)
		return 0;
	return 1;
}

/* 12/01/21 */
int splitdate_MDY(char *date, int *year, int *month, int *day)
{
	char *p = strchr(date, '/');
	if (!p)
		return 0;

	*month = atoi(date);
	*day   = atoi(p+1);
	p      = strchr(p+1, '/');
	if (!p)
		return 0;
	*year = atoi(p+1);
	switch (*year) {
		case 20: *year = 0; break;
		case 21: *year = 1; break;
		case 22: *year = 2; break;
		default: return 0;
	}
	if (*year < 0 || *year > 2)
		return 0;
	if (*month <= 0 || *month > 12)
		return 0;
	if (*day < 0 || *day > 31)
		return 0;
	return 1;
}

char TRADETIME[24];


time_t get_ny_time(char *timestr)
{
	struct tm       ltm;
	struct tm      *local_tm;
	struct tm      *tm_ny;
	struct tm       nytm;
	time_t          time_ny, utc_time;
	int             offset = SERVER->TIMEZONE;

	utc_time = time(NULL);
	local_tm = tm_ny = localtime_r(&utc_time, &ltm);
	if (!SERVER->production) {
		local_tm->tm_sec += offset;
		time_ny = mktime(local_tm);
		tm_ny   = localtime_r(&time_ny, &nytm);
	}
	asctime_r(tm_ny, timestr);
	return (time_ny);
}

// ancient, to be removed
int get_time()
{
	//             2 45 78
	//          9  H MM SS
	// Sat Jul 18 16:07:05 2020
	char timestr[256];
	char hour, hour2, minute, minute2, seconds, seconds2;
	int x;

	get_ny_time(timestr);
	if (*timestr == '\0')
		return market;
	if (*(timestr) == 'S') {
		if (*(timestr+1) == 'u')
			TODAY_IS_SUNDAY = 1;
		else
			TODAY_IS_SATURDAY = 1;
		return NO_MARKET;
	} else if (*timestr == 'M')
		TODAY_IS_MONDAY = 1;
	if (holiday)
		return NO_MARKET;

	hour     = *(timestr+11);
	hour2    = *(timestr+12);
	minute   = *(timestr+14);
	minute2  = *(timestr+15);
	seconds  = *(timestr+17);
	seconds2 = *(timestr+18);


	if (hour2 > '0')
		hour = ((hour-48)*10)+(hour2-48);
	else
		hour = hour2-48;

	if (minute > '0')
		minute = ((minute-48)*10)+(minute2-48);
	else
		minute = minute2-48;


	// It's at least 4am, find out if it's > 9:30am
	if (hour >= 4 && hour < 10) {
		if (hour == 9 && minute >= 30) {
			if (!TRADETIME[9] && minute == 30) {
				unsigned long hour_args = (unsigned long)hour;
				TRADETIME[9] = 1;
				printf("DAYMARKET hour: %d minute: %d\n", hour, minute);
				thread_create(market_update_thread, (void *)hour_args);
			}
			return (market=DAY_MARKET);
		} else if ((hour >= 4 && hour <= 9) && minute == 0) {
			if (hour == 4)
				memset(&TRADETIME[0], 0, sizeof(TRADETIME));
			// Every hour from 4am-9am pull failed tickers
			if (!TRADETIME[hour]) {
				unsigned long hour_args = (unsigned long)hour;
				TRADETIME[hour] = 1;
				thread_create(market_update_thread, (void *)hour_args);
				printf("PREMARKET hour: %d minute: %d\n", hour, minute);
			}
			return (market=PRE_MARKET);
		} else {
//			printf(BOLDRED "SHOULD NOT REACH" RESET "\n");
			return (market=PRE_MARKET);
		}
	}

	// Still not 4pm - DAY_MARKET
	if (hour < 16)
		return (market=DAY_MARKET);

	// After 4pm: AFH_MARKET
	if (hour >= 16 && hour < 20)
		return (market=AFH_MARKET);

	// 20:00 - 8pm
	if ((hour == 20 && minute == 0) && !TRADETIME[hour]) {
		TRADETIME[hour] = 1;
		thread_create(market_end, NULL);
	}
	return NO_MARKET;
}

int weekday_offset(const char *day)
{
	for (int x = 0; x<sizeof(WEEKDAYS)/sizeof(char *); x++)
		if (!strcmp(WEEKDAYS[x], day))
			return x;
	return -1;
}


int timezone_offset(const char *tzname)
{
	for (int x = 0; x<sizeof(timezones)/sizeof(struct timezone); x++) {
		if (!strcmp(timezones[x].tzname, tzname))
			return timezones[x].utc_offset;
	}
	return 0;
}


int utc_timezone_offset()
{
	struct tm loc_tm;
	struct tm gmt_tm;
	time_t    loc_timestamp;
	time_t    gmt_timestamp;

	/* Get the local unix epoch timestamp & the broken down struct tm for it */
	time(&loc_timestamp);
 	localtime_r(&loc_timestamp, &loc_tm);

	/* GMT (UTC without summer time) */        
	gmtime_r(&loc_timestamp, &gmt_tm);

	/* Convert the broken down struct tm's to the unix timestamp */
	loc_timestamp = mktime(&loc_tm);
	gmt_timestamp = mktime(&gmt_tm);

     if (gmt_tm.tm_isdst == 1)
		gmt_timestamp -= 3600;

	return (loc_timestamp - gmt_timestamp);
}

void init_time(struct server *server)
{
    char timestr[64];

	SERVER = server;
	server->TIMEZONE = utc_timezone_offset();

	time_load_EOD();
	market_update();

	QDATE[1]      = strdup(unix2str(get_timestamp(), timestr));
	QDATESTAMP[1] = str2unix(QDATE[1]);
	QDATE[2]      = strdup(unix2str(QDATESTAMP[1]+(24*3600), timestr));
	QDATESTAMP[2] = str2unix(QDATE[2]);

	load_weeks();
}
