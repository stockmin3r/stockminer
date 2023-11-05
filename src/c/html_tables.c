#include <conf.h>
#include <extern.h>

struct www     *WWW_HASHTABLE;
struct colmap   COLMAP[256] = { 100, 999 };
int             NR_COLUMNS = 1;
pthread_mutex_t colmap_lock;

void apc_www(struct connection *connection, char **argv)
{
	struct www *www      = NULL, *tmp;
	char       *url      = argv[0];
	char       *colors[] = { BOLDRED, BOLDGREEN, BOLDMAGENTA, BOLDCYAN, BOLDWHITE, BOLDYELLOW };
	char        msg[512 KB];
	char       *color;
	int         nbytes, color_index = 0;

	if (*url != '*')
		HASH_FIND_STR(WWW_HASHTABLE, url, www);

	if (!www) {
		printf(BOLDRED "no entry for: %s" RESET "\n", url);
		HASH_ITER(hh, WWW_HASHTABLE, www, tmp) {
			printf(BOLDWHITE "URL: %s nr_tables: %d www: %p" RESET "\n", url, www->nr_tables, www);
		}
		return;
	}

	for (int x=0; x<www->nr_tables; x++) {
		struct table *table = www->tables[x];
		if (!table)
			continue;
		color  = colors[color_index++];
		nbytes = snprintf(msg, sizeof(msg)-1, "%s%s" RESET, color, table->html);
		if (color_index >= 6)
			color_index = 0;
		printf("%s\n", msg);
	}
	printf(BOLDGREEN "URL: %s nr_tables: %d www: %p" RESET "\n", url, www->nr_tables, www);
}

void rpc_wget_table(struct rpc *rpc)
{
	char          *packet        = rpc->packet;
	char          *url           = rpc->argv[1];
	char          *watchtable_id = rpc->argv[2];
	char          *QGID          = rpc->argv[3];
	struct www    *www = NULL;
	struct table **tables;
	struct table  *table;
	char           buf[256];
	char          *table_name;
	int            nr_tables, packet_len, x;

	HASH_FIND_STR(WWW_HASHTABLE, url, www);
	if (!www) {
		printf("adding url: %s\n", url);
		tables = LIBXLS_wget_table(url, &nr_tables);
		if (nr_tables >= 256)
			return;
		printf("nr_tables: %d\n", nr_tables);
		if (!tables)
			return;
		www = (struct www *)zmalloc(sizeof(*www));
		if (!www)
			return;
		www->URL       = strdup(url);
		www->tables    = tables;
		www->nr_tables = nr_tables;
		HASH_ADD_STR(WWW_HASHTABLE, URL, www);
	} else {
		tables    = www->tables;
		nr_tables = www->nr_tables;
		printf("url already found! %s nr_tables: %d\n", www->URL, www->nr_tables);
	}

	strcpy(packet, "wget ");
	packet_len = 5;
	for (x=0; x<nr_tables; x++) {
		table = tables[x];
		if (table->caption)
			table_name = table->caption;
		else {
			sprintf(buf, "Table^%d", x);
			table_name = buf;
		}
		packet_len += snprintf(packet+packet_len, 256, "%s`%d ", table_name, table->nr_rows);
	}
	if (packet_len == 5)
		return;
	packet[--packet_len] = 0;
	printf("packet: %s\n", packet);
	websocket_send(rpc->connection, packet, packet_len);
}

char *html_property(char *element, char *property)
{
	int len, x;

	len = strlen(property);
	property = strstr(element, property);
	if (!property)
		return NULL;
	if (*(property+len) == '\"')
		len++;
	else if (*(property+len) == '\'')
		len++;
	for (x=0; x<32; x++) {
		char c = *(property+len+x);
		if (c == ' ' || c == '\'' || c == '\"') {
			*(property+len+x) = 0;
			break;
		}
	}
	property = strdup(property+len);
	*(property+len+x) =  ' ';
	return (property);
}
/*
<table id="tablepress-3494" class="tablepress tablepress-id-3494 dataTable no-footer" role="grid" aria-describedby="tablepress-3494_info">
<thead>
<tr class="row-1 odd" role="row">
	<th class="column-1 sorting" tabindex="0" rowspan="1" colspan="1" style="width: 33.9833px;">S.No.</th>
	<th class="column-2 sorting" tabindex="0" rowspan="1" colspan="1" style="width: 287.083px;">Company</th>
	<th class="column-3 sorting" tabindex="0" rowspan="1" colspan="1" style="width: 42.35px;">Ticker</th>
	<th class="column-4 sorting" tabindex="0" rowspan="1" colspan="1" style="width: 153.017px;" aria-label="Subsector: activate to sort column ascending">Subsector</th>
	<th class="column-5 sorting" tabindex="0" colspan="1" style="width: 73.5667px;">Country</th>
</tr>
</thead>
<tbody class="row-hover">*/

int parse_table_row(char *html_tag, char *html, char **cells, char *end)
{
	char *p, *td, *row;
	int clen, nr_cells = 0;

	row = strstr(html, "<tr");
	if (!row)
		return 0;

	while ((td=strstr(row, html_tag))) {
		if (td >= end)
			break;
		if (td[3] != ' ' && td[3] != '>') {
			html = td + 3;
			continue;
		}
		td  += 3;
		p    = td;
		clen = 0;
		while (*p != '<') {
			p++;
			if (clen++ > 256)
				return 0;
			if (*p == '>')
				td = p+1;
		}
		*p = 0;
		cells[nr_cells++] = strdup(td);
		printf("cell: %s\n", td);
		*p++ = '>';
		row  = p;
		if (nr_cells >= 256)
			break;
		if (row+4 >= end)
			break;
	}
	return (nr_cells);
}

int parse_table_columns(char *thead, char **columns, char *end)
{
	char *p, *th;
	int clen, nr_columns = 0;

	while ((th=strstr(thead, "<th"))) {
		if (th >= end)
			break;
		if (th[3] != ' ' && th[3] != '>') {
			thead = th + 3;
			continue;
		}
		th  += 3;
		p    = th;
		clen = 0;
		while (*p != '<') {
			p++;
			if (clen++ > 256)
				return 0;
			if (*p == '>')
				th = p+1;
		}
		*p = 0;
		columns[nr_columns++] = strdup(th);
		printf("column: %s\n", th);
		*p++ = '>';
		thead = p;
		if (nr_columns >= 256)
			break;
		if (th+4 >= end)
			break;
	}
	return (nr_columns);
}


int parse_table_rows(struct table *table)
{
	char **cells, *row, *tr, *td, *p;
	int nr_rows = table->nr_rows, nr_columns = table->nr_columns, len, x, y;

	if (nr_rows <= 0)
		return 0;

	table->rows = (struct row *)malloc(sizeof(struct row) * nr_rows);
	if (!table->rows)
		return 0;

	tr = table->tbody;
	for (x=0; x<nr_rows; x++) {
		row = strstr(tr, "<tr");
		if (!row)
			return 0;
		table->rows[x].cells = cells = (char **)malloc(sizeof(char *) * nr_columns);
		/* scan through the row */
		for (y=0; y<nr_columns; y++) {
			td = strstr(row, "<td");
			if (!td)
				return 0;
			len = 0;
			p = td+3;
			while (*p != '<') {
				if (len++ > 256)
					return 0;
				if (*p == '>')
					td = p+1;
				p++;
			}
			*p       = 0;
			cells[y] = strdup(td);
			*p++     = '>';
			row      = p;
		}
		tr = p;
	}
	return 0;
}

int count_table_rows(char *tbody, char *end)
{
	char *p = tbody;
	int nr_rows = 0;

	while ((p=strstr(tbody, "</tr>"))) {
		if (p >= end)
			break;
		nr_rows++;
		tbody = p + 5;
	}
	return (nr_rows);
}

#define TABLE_MENU       "{\"orderable\":false,\"data\":null},"
#define DATA_FORMAT_HTML "\"dataFormat\":\"HTML\", \"columns\":["

int LIBXLS_deftab2(char *packet, char *URL, char *QGID, char *table_names)
{
	struct www    *www = NULL;
	struct table **tables;
	struct table  *table;
	char          *table_names_array[256];
	char         **cells;
	int            nr_tables, nr_columns, nr_rows, packet_len, len, column, x, y, z;

	nr_tables = cstring_split(table_names, table_names_array, 255, '`');
	if (nr_tables <= 0)
		return 0;

	HASH_FIND_STR(WWW_HASHTABLE, URL, www);
	if (!www)
		return 0;

	if ((len=strlen(QGID)) > 18)
		return 0;
	tables = www->tables;
	if (!tables)
		return 0;

 
/*	dt = { QGID: "P0Q0q0ws0", tables: [{dataFormat:"dict", columns: [{2000:"Gene"},{2001:"Organs"}], data: [{2000:"BDNF",2001:"Brain"}]}, {tableFormat:"csv", csv: "Telco,Gene\n"}] } */

	packet_len = snprintf(packet, 255, "deftab {\"QGID\":\"%s\", tables: [{\"dataFormat\":\"dict\", \"columns\":[,", QGID);
	for (x=0; x<nr_tables; x++) {
		table = tables[x];
		len   = strlen(table_names_array[x]);
		printf(BOLDWHITE "searching: %s vs %s" RESET "\n", table->caption, table_names_array[x]);
		if (strcmp(table->caption, table_names_array[x]))
			continue;

		// define columns
		// deftab  P0Q0q0ws0`DataFormat`1000|Company|1001|Sector~[{1000:RR},{1001:Telco}]`2000|Gene|2001|Organs~[{2000:BDNF},{2001:Brain}]
		printf("found table\n");
		nr_columns = table->nr_columns;
		if (!nr_columns && table->html) {
			memcpy(packet+packet_len, DATA_FORMAT_HTML, sizeof(DATA_FORMAT_HTML)-1);
			packet_len += sizeof(DATA_FORMAT_HTML)-1;
		}
		column = table->colmap->min;
		for (y=0; y<nr_columns; y++)
			packet_len += snprintf(packet+packet_len, 32, "{\"%d\":\"%s\"},", column++, table->columns[y]);
		packet[packet_len-1] = ']';
		packet[packet_len++] = ',';
		strcpy(packet+packet_len, "\"data\":[");
		packet_len += 8;

		nr_rows = table->nr_rows;
		for (y=0; y<nr_rows; y++) {
			cells  = table->rows[y].cells;
			column = table->colmap->min;
			packet[packet_len++] = '{';
			for (z=0; z<nr_columns; z++)
				packet_len += snprintf(packet+packet_len, 256, "{\"%d\":\"%s\"},", column++, cells[z]);
			packet[packet_len-1] = '}';
			packet[packet_len++] = ',';
		}
		packet[packet_len-1] = ']';
		packet[packet_len++] = ',';
	}
	if (packet_len <= 32)
		return 0;
	// deftab watchtable_id QGID columns(definition) rows(data)
	// column definition: STAB = [{"orderable":false,"data":null,"defaultContent":''},{"data":"900"},{"data":"901"},{"data":"903"},{"data":"904"}],
	// row cell data: [{"901":"TSLA","901":89.99}]
	packet[--packet_len] = '}';
	packet[packet_len] = 0;
	printf(BOLDYELLOW "%s" RESET "\n", packet);
	return (packet_len);
}


int LIBXLS_deftab(char *packet, char *URL, char *QGID, char *table_names)
{
	struct www    *www = NULL;
	struct table **tables;
	struct table  *table;
	char          *table_names_array[256];
	char         **cells;
	int            nr_tables, nr_columns, nr_rows, packet_len, len, column, x, y, z;

	nr_tables = cstring_split(table_names, table_names_array, 255, '`');
	if (nr_tables <= 0)
		return 0;

	HASH_FIND_STR(WWW_HASHTABLE, URL, www);
	if (!www)
		return 0;

	if ((len=strlen(QGID)) > 18)
		return 0;
	tables = www->tables;
	if (!tables)
		return 0;
	packet_len = snprintf(packet, 32, "deftab %s`", QGID);

	for (x=0; x<nr_tables; x++) {
		table = tables[x];
		len   = strlen(table_names_array[x]);
		printf(BOLDWHITE "searching: %s vs %s" RESET "\n", table->caption, table_names_array[x]);
		if (strcmp(table->caption, table_names_array[x]))
			continue;

		// define columns
		// deftab  P0Q0q0ws0`DataFormat`1000|Company|1001|Sector~[{1000:RR},{1001:Telco}]`2000|Gene|2001|Organs~[{2000:BDNF},{2001:Brain}]
		printf("found table\n");
		nr_columns = table->nr_columns;
		column = table->colmap->min;
		for (y=0; y<nr_columns; y++)
			packet_len += snprintf(packet+packet_len, 32, "%d|%s|", column++, table->columns[y]);
		packet[packet_len-1] = '~';
		packet[packet_len++] = '[';

		nr_rows = table->nr_rows;
		for (y=0; y<nr_rows; y++) {
			cells  = table->rows[y].cells;
			column = table->colmap->min;
			packet[packet_len++] = '{';
			for (z=0; z<nr_columns; z++)
				packet_len += snprintf(packet+packet_len, 256, "\"%d\":\"%s\",", column++, cells[z]);
			packet[packet_len-1] = '}';
			packet[packet_len++] = ',';
		}
		packet[packet_len-1] = ']';
		packet[packet_len++] = '`';
	}
	if (packet_len <= 32)
		return 0;
	// deftab watchtable_id QGID columns(definition) rows(data)
	// column definition: STAB = [{"orderable":false,"data":null,"defaultContent":''},{"data":"900"},{"data":"901"},{"data":"903"},{"data":"904"}],
	// row cell data: [{"901":"TSLA","901":89.99}]
	packet[--packet_len] = 0;
	printf(BOLDYELLOW "%s" RESET "\n", packet);
	return (packet_len);
}

struct colmap *register_colmap(int nr_columns)
{
	struct colmap *colmap = NULL;

	pthread_mutex_lock(&colmap_lock);
	if (NR_COLUMNS <= 256) {
		colmap = &COLMAP[NR_COLUMNS-1];
		COLMAP[NR_COLUMNS].min = colmap->max+1;
		COLMAP[NR_COLUMNS].max = nr_columns;
		colmap = &COLMAP[NR_COLUMNS];
		NR_COLUMNS++;
		fs_appendfile("db/colmap.db", (char *)colmap, sizeof(*colmap));
	}
	pthread_mutex_unlock(&colmap_lock);
	return (colmap);
}

struct table **LIBXLS_wget_table(char *URL, int *ntables)
{
	struct table  *table = NULL;
	struct table  *tables[256];
	struct table **tp;
	char          *columns[256];
	char           page[1024 KB];
	char           buf[64];
	char          *p, *p2, *html, *pTable, *start, *end;
	int            nr_tables = 0, nr_columns, x;

	if (!curl_get(URL, page))
		return NULL;

	html  = page;
	while ((pTable=strstr(html, "<table"))) {
		table = (struct table *)zmalloc(sizeof(*table));
		if (!table)
			return NULL;
		p = strchr(pTable, '>');
		if (!p)
			return NULL;
		*p++           = 0;
		table->html    = pTable;
		html           = p;
		table->id      = html_property(pTable, "id=");
		table->Class   = html_property(pTable, "class=");
		table->caption = (char *)memmem(p, 128, "<caption", 8);
		if (table->caption) {
			p = strchr(table->caption, '>');
			if (p) {
				p2 = strchr(p+1, '<');
				if (p2) {
					*p2 = 0;
					table->caption = strdup(p+1);
					*p2 = ' ';
				}
			}
		} else {
			snprintf(buf, sizeof(buf)-1, "Table^%d", nr_tables);
			table->caption = strdup(buf);
		}

		end = strstr(html, "</table>");
		if (!end)
			break;
		table->table_size = end-pTable;
		html = end+8;
		tables[nr_tables++] = table;

		// parse table columns <thead>
		if (!(table->thead=strstr(pTable, "<thead"))) {
			if (!(table->thead=strstr(pTable, "<tbody")))
				continue;
			start = strstr(pTable, "<tr");
			if (!start)
				continue;
			end = strstr(start+4, "</tr>");
			if (!end)
				continue;
			table->nr_columns = nr_columns = parse_table_row("<tr", start, columns, end);
			table->tbody = strstr(end, "<tr>");
			if (!table->tbody)
				continue;
		} else {
			if (!(table->tbody=strstr(table->thead, "<tbody")))
				continue;
			if (!(end=strstr(table->tbody, "</table>")))
				continue;
			table->nr_columns = nr_columns = parse_table_columns(table->thead+5, columns, table->tbody);
			if (nr_columns <= 0)
				continue;
		}
		table->columns = (char **)malloc(sizeof(char *) * nr_columns);
		table->colmap  = register_colmap(nr_columns);
		if (!table->colmap || !table->columns)
			continue;
		for (x=0; x<nr_columns; x++)
			table->columns[x] = columns[x];
		table->nr_rows = count_table_rows(table->tbody, end);
		if (!parse_table_rows(table))
			continue;
		printf(BOLDWHITE "<table>" RESET BOLDGREEN " ID: %-10s CLASS: %-12s CAPTION: %-10s" RESET "\n", table->id?table->id:"", table->Class?table->Class:"",table->caption?table->caption:"");
	}
	tp = (struct table **)malloc(sizeof(struct table *) * nr_tables);
	if (!tp)
		return NULL;
	for (x=0; x<nr_tables; x++)
		tp[x] = tables[x];
	if (ntables)
		*ntables = nr_tables;
	return (tp);
}
/*
<table class="infobox" style="width: 22em"><link rel="mw-deduplicated-inline-style" href="mw-data:TemplateStyles:r1066479718"/>
	<tbody>
		<tr>
			<td colspan="2" class="infobox-subheader" style="font-size: 125%; background-color: #b0c4de;">KOSPI</td>
		</tr>
			<link rel="mw-deduplicated-inline-style" href="mw-data:TemplateStyles:r1066479718"/>
		<tr>
			<th scope="row" class="infobox-label" style="font-weight: normal;">
				<a href="/wiki/Hangul" title="Hangul">Hangul</a>
			</th>
			<td class="infobox-data"><div style="display:inline;font-size:1rem;"><span title="Korean-language text"><span lang="ko-Hang">코스피지수</span></span></div>
			</td>
		</tr>
		<tr>
			<th scope="row" class="infobox-label" style="font-weight: normal;"><a href="/wiki/Hanja" title="Hanja">Hanja</a></th>
			<td class="infobox-data"><div style="display:inline;font-size:1rem;"><span title="Korean-language text"><span lang="ko-Hani">코스피指數</span></span></div></td>
		</tr>
		<tr>
			<th scope="row" class="infobox-label" style="font-weight: normal;"><a href="/wiki/Revised_Romanization_of_Korean" title="Revised Romanization of Korean">Revised Romanization</a></th><td class="infobox-data"><span title="Revised Romanization of Korean transliteration"><i lang="ko-Latn">Koseupi jisu</i></span></td></tr><tr><th scope="row" class="infobox-label" style="font-weight: normal;"><a href="/wiki/McCune%E2%80%93Reischauer" title="McCune–Reischauer">McCune–Reischauer</a></th>
				<td class="infobox-data"><span title="McCune–Reischauer transliteration"><i lang="ko-Latn">K'osŭp'i chisu</i></span>
				</td>
		</tr>
		<link rel="mw-deduplicated-inline-style" href="mw-data:TemplateStyles:r1066479718"/>
	</tbody>
</table>
*/

typedef void *(*job_exec_q)(void *args);

void LIBXLS_rpc_excel2table(struct session *session, struct connection *connection, char *excel_file, char *excel_filepath, int excel_size, char **subargv, void *(*completion)(struct job *job))
{
	// libreoffice --invisible --convert-to html db/%d/excel/%s.xlsx --outdir db/uid/%d/html/%s.html
	struct job *job = new_job(JOB_SCHEDULER_POPEN|JOB_READ_OUTPUT,completion);
	char cmdline[32 KB];
	int nbytes, uid = session->user->uid;

	job->exename = "libreoffice";
	job->exepath = "/usr/bin/libreoffice";
	snprintf(job->output_filepath, 255, "db/uid/%d/html/%s.html", uid, excel_filepath);
	nbytes = snprintf(cmdline, sizeof(cmdline)-1, "libreoffice --invisible --convert-to html db/uid/%d/excel/%s.xlsx --outdir %s", uid, excel_filepath, job->output_filepath);
	if (nbytes > MAX_CMDLINE_SIZE)
		return;
	job->cmdline = strdup(cmdline);
	thread_create((job_exec_q)job->execute, job);
}
