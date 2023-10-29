#include <conf.h>
#include <stocks/options.h>

int csv_load_all(char *csvbuf, struct ohlc **ohlc)
{
	struct ohlc *OHLC;
	char *point, *p, *p2;
	int nr_points = 0;

	point = csvbuf+2;
	OHLC  = (struct ohlc *)malloc(512 * sizeof(struct ohlc));
	while ((p=strchr(point, ']'))) {
		OHLC[nr_points].timestamp = strtoul(point, NULL, 10);
		p2 = strchr(point, ',');
		OHLC[nr_points].open      = strtod(p2+1, NULL);
		p2 = strchr(p2+1, ',');
		OHLC[nr_points].high      = strtod(p2+1, NULL);
		p2 = strchr(p2+1, ',');
		OHLC[nr_points].low       = strtod(p2+1, NULL);
		p2 = strchr(p2+1, ',');
		OHLC[nr_points].close     = strtod(p2+1, NULL);
		p2 = strchr(p2+1, ',');
		OHLC[nr_points].volume    = strtoul(p2+1, NULL, 10);
		nr_points += 1;
		if (*(point+1) == ']')
			break;
		point = p + 3;
	}
	*ohlc = &OHLC[0];
	return (nr_points);
}

int csv_load(char *csvbuf, struct ohlc *OHLC)
{
	char *point, *p;
	int nr_points = 0;

	p = strstr(csvbuf, "]]");
	if (!p)
		return 0;
	p -= 2;
	while (*p != '[') {
		p--;
		if (p < csvbuf)
			return 0;
	}
	p -= 1;
	while (*p != '[') {
		p--;
		if (p < csvbuf)
			return 0;
	}
	point = p+1;
	while ((p=strchr(point, ']'))) {
		OHLC[nr_points].timestamp = strtoul(point, NULL, 10);
		p = strchr(point, ',');
		OHLC[nr_points].open      = strtod(p+1, NULL);
		p = strchr(p+1, ',');
		OHLC[nr_points].high      = strtod(p+1, NULL);
		p = strchr(p+1, ',');
		OHLC[nr_points].low       = strtod(p+1, NULL);
		p = strchr(p+1, ',');
		OHLC[nr_points].close     = strtod(p+1, NULL);
		p = strchr(p+1, ',');
		OHLC[nr_points].volume    = strtoul(p+1, NULL, 10);
		nr_points += 1;
		if (nr_points >= 2)
			break;
		point = strchr(p, '[');
		if (!point)
			return nr_points;
		point++;
	}
	return (nr_points);
}

int opstock_csv_load(struct opstock *opstock, char *path)
{
	char csvbuf[4 KB];

	opstock->OHLC = (struct ohlc *)malloc(sizeof(struct ohlc) * TRADE_MINUTES);
	memset(opstock->OHLC, 0, sizeof(struct ohlc) * TRADE_MINUTES);
	fs_readfile_str(path, csvbuf, sizeof(csvbuf));
	return (opstock->nr_ohlc=csv_load(csvbuf, opstock->OHLC));
}

int csv_exists(char *ticker, char *contract)
{
	struct stat sb;
	char path[256];

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, contract);
	if (stat(path, &sb) == 0)
		return 1;
	return 0;
}

int csv_points(char *csv)
{
	char *p;
	int nr_points = 0;

	while ((p=strchr(csv, ']'))) {
		nr_points += 1;
		if (*(p+1) == ']')
			break;
		csv = p + 1;
	}
	return (nr_points);
}

void option_load_csv(char *ticker, char *contract, struct opchain *opchain, int opindex, char optype)
{
	struct stat sb;
	struct Option *op;
	char path[256];
	char *buf;
	int nr_days, nr_calls, nr_puts, fd;

	nr_days  = opchain->nr_days-1;
	nr_calls = opchain->nr_calls;
	nr_puts  = opchain->nr_puts;
	if (!opchain->csv_calls) {
		opchain->csv_calls         = (char **)malloc(sizeof(char *) * nr_calls);
		memset(opchain->csv_calls,    '\0',          sizeof(char *) * nr_calls);
		opchain->csv_puts          = (char **)malloc(sizeof(char *) * nr_puts);
		memset(opchain->csv_puts,     '\0',          sizeof(char *) * nr_puts);
		opchain->csv_calls_len     = (int *)malloc(nr_calls * sizeof(int));
		memset(opchain->csv_calls_len,   0, nr_calls * sizeof(int));
		opchain->csv_puts_len      = (int *)malloc(nr_puts  * sizeof(int));
		memset(opchain->csv_puts_len,    0, nr_puts  * sizeof(int));
		opchain->csv_call_points   = (int *)malloc(nr_calls * sizeof(int));
		memset(opchain->csv_call_points, 0, nr_calls * sizeof(int));
		opchain->csv_put_points    = (int *)malloc(nr_puts  * sizeof(int));
		memset(opchain->csv_put_points,  0, nr_puts  * sizeof(int));
	}

	if (optype == OPTION_CALL) {
		op  = opchain->call_options[0];
		op += opindex;
		snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, op->contract);
	} else {
		op  = opchain->put_options[0];
		op += opindex;
		snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, op->contract);
	}
	if (stat(path, &sb) == -1 || sb.st_size < 10)
		return;


	fd = open(path, O_RDONLY);
	if (fd < 0)
		return;
	buf = (char *)malloc(sb.st_size+1);
	read(fd, buf, sb.st_size);
	close(fd);
	buf[sb.st_size] = 0;
	if (optype == OPTION_CALL) {
		opchain->csv_calls[opindex]       = buf;
		opchain->csv_calls_len[opindex]   = sb.st_size;
		opchain->csv_call_points[opindex] = csv_points(buf);
	} else {
		opchain->csv_puts[opindex]        = buf;
		opchain->csv_puts_len[opindex]    = sb.st_size;
		opchain->csv_put_points[opindex]  = csv_points(buf);
	}
}
