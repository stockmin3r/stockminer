#include <conf.h>

static struct server *SERVER;

int    holiday = 1;
time_t current_minute;
time_t current_timestamp;
int    current_day;
int    current_week;

int TODAY_IS_FRIDAY;
int TODAY_IS_SATURDAY;
int TODAY_IS_SUNDAY;
int TODAY_IS_MONDAY;
int TRADETIME_YESTERDAY;

char         **DAYS;
int            nr_trade_days;
char          *QDATE[3];
time_t         QDATESTAMP[3];
struct week    WEEKS[52];
char *days[]  = { "Mon", "Tue", "Wed", "Thu", "Fri" };

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
	current_timestamp = (time_now-(time_now%60))*1000;
	current_minute    = current_timestamp;
//	printf("(start: %s) time_now: %lu time_start: %lu, diff: %d current: %lu\n", timestr, time_now, time_start, time_now-time_start, current_timestamp);
}

void load_EOD()
{
	struct stat sb;
	char buf[256];
	char *p;
	int fd, nbytes;

	fd = open("data/stocks/stockdb/csv/^GSPC.csv", O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(-1);
	}

	fstat(fd, &sb);
	lseek(fd, -256, SEEK_END);
	nbytes      = read(fd, buf, 256);
	buf[nbytes] = 0;
	p           = buf+nbytes;
	while (*p != '-')
		p--;

	*(p+3)        = 0;
	p            -= 7;
	QDATE[0]      = strdup(p);
	QDATESTAMP[0] = str2unix(p);
	printf(BOLDMAGENTA "load_EOD: %s" RESET "\n", QDATE[0]);
	close(fd);
}

void load_weeks()
{
	struct week *week;
	char         buf[64 KB];
	char        *line, *p;
	time_t       unix_today = QDATESTAMP[1];
	int64_t     filesize;
	int          nr_weeks = 0;

	filesize = fs_readfile_str((char *)STOCK_WEEKS_TXT, buf, sizeof(buf));
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

void load_days()
{
	char     *date;
	char      buf[32 KB];
	char     *p;
	time_t   unix_today = QDATESTAMP[1];
	int64_t filesize;
	int      x;

	filesize = fs_readfile_str((char *)STOCK_DAYS_TXT, buf, sizeof(buf));
	if (!filesize) {
		printf(BOLDRED "[-] load_days(): file read error" RESET "\n");
		exit(-1);
	}
	nr_trade_days = cstring_line_count(buf);
	if (nr_trade_days <= 0 || nr_trade_days > 255) {
		printf(BOLDRED "[-] load_days(): corrupt stocks/DAYS.TXT file" RESET "\n");
		exit(-1);
	}
	DAYS = (char **)malloc(sizeof(char *) * nr_trade_days);
	if (!DAYS) {
		printf(BOLDRED "[-] no memory to alloc DAYS" RESET "\n");
		exit(-1);
	}
	date = buf;
	while ((p=strchr(date, '\n'))) {
		*p++  = 0;
		DAYS[x] = strdup(date);
		date = p;
		if (unix_today == str2unix(date))
			current_day = x;
		x++;
	}
}

void init_time(struct server *server)
{
	char timestr[64];

	SERVER = server;

	/* current day */
	load_EOD();
	if (TODAY_IS_SUNDAY) {
		printf(BOLDMAGENTA "TODAY IS SUNDAY" RESET "\n");
	}else if (TODAY_IS_MONDAY) {
		printf(BOLDMAGENTA "TODAY IS MONDAY" RESET "\n");
	}

	QDATE[1]      = strdup(unix2str(get_timestamp(), timestr));
	QDATESTAMP[1] = str2unix(QDATE[1]);
	QDATE[2]      = strdup(unix2str(QDATESTAMP[1]+(24*3600), timestr));
	QDATESTAMP[2] = str2unix(QDATE[2]);

	// load trading calendar
	load_days();
	load_weeks();
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

time_t get_current_time(char *timestr)
{
	struct tm  ltm;
	struct tm *local_tm;
	time_t     utc_time;

	utc_time = time(NULL);
	local_tm = localtime_r(&utc_time, &ltm);
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
		tm_ny = localtime_r(&time_ny, &nytm);
	}
	asctime_r(tm_ny, timestr);
	return (time_ny);
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
	sprintf(tbuf, "%s 00:00", timestr);
	strptime(tbuf, "%Y-%m-%d %H:%M", &ltm);
	ltm.tm_isdst = -1;
	timestamp = mktime(&ltm);
	return timestamp+SERVER->UTCOFF;
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
				thread_create(market_update, (void *)hour_args);
			}
			return (market=DAY_MARKET);
		} else if ((hour >= 4 && hour <= 9) && minute == 0) {
			if (hour == 4)
				memset(&TRADETIME[0], 0, sizeof(TRADETIME));
			// Every hour from 4am-9am pull failed tickers
			if (!TRADETIME[hour]) {
				unsigned long hour_args = (unsigned long)hour;
				TRADETIME[hour] = 1;
				thread_create(market_update, (void *)hour_args);
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

