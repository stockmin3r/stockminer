#include <conf.h>
#include <extern.h>

struct image {
	char         *filename;  // eg: logo.png
	char         *response;  // HTTP Response header + image
	unsigned int  size;      // length of HTTP response header + image
};

int             NR_IMAGES;
struct object  *Objects;
struct image  **IMAGES;

bool WWW_GET_IMAGE(char *filename, struct connection *connection)
{
	char *p = strchr(filename, ' ');

	if (!p)
		return false;
	*p = 0;

	for (int x = 0; x<NR_IMAGES; x++) {
		struct image *image = IMAGES[x];
		if (!strcmp(filename, image->filename)) {
			SSL_write(connection->ssl, image->response, image->size);
			return true;
		}
	}
	return false;
}

void blob_load_images2(char *directory)
{
	char *paths[] = { "www/img/website", "www/img/website/stockminer", "www/img/website/" };
}

void blob_load_images(char *directory)
{
	struct dirmap  dirmap;
	struct image  *image;
	char          *filename, *file, *response;
	char           path[1024];
	int64_t        filesize;
	int            nbytes, max_images = 32;

	if (!fs_opendir(directory, &dirmap))
		return;

	IMAGES = (struct image **)malloc(sizeof(struct image) * max_images);
	while ((filename=fs_readdir(&dirmap)) != NULL) {
		if (*filename == '.')
			continue;
		snprintf(path, sizeof(path)-1, "%s/%s", directory, filename);
		file = fs_mallocfile(path, &filesize);
		if (!file || !filesize)
			continue;
		if (likely(strstr(path, ".woff2"))) {
			response = (char *)malloc(filesize + sizeof(HTTP_FONT) + 64);
			nbytes   = sprintf(response, HTTP_FONT, filesize);
		} else {
			response = (char *)malloc(filesize + sizeof(HTTP_IMAGE) + 64);
			nbytes   = sprintf(response, HTTP_IMAGE, filesize);
		}
		image        = (struct image *)zmalloc(sizeof(*image));
		if (!image) {
			free(file);
			continue;
		}
		image->response     = response;
		memcpy(image->response+nbytes, file, filesize);
		image->size         = filesize + nbytes;
		image->filename     = strdup(filename);
		IMAGES[NR_IMAGES++] = image;
		if (NR_IMAGES >= max_images) {
			max_images *= 2;
			IMAGES      = realloc(IMAGES, max_images);
		}
		free(file);
	}
}

char *load_blob(unsigned int uid, char *key, int64_t *blobsize)
{
	char path[1024];
	snprintf(path, sizeof(path)-1, "db/uid/%d/blob/%s", uid, key);
	return fs_mallocfile(path, blobsize);
}

/* Must be run BEFORE sessions_load() */
void load_objects(struct session *session, unsigned int uid)
{
	struct object *obj = NULL;
	struct dirmap  dirmap;
	char          *key;
	char           buf[128];
	char           path[32];
	char           url[32];

	snprintf(url, sizeof(url)-1, "%d/pic.jpeg", session->user->uid);
	HASH_FIND_STR(Objects, url, obj);
	session->avatar = obj;

	snprintf(path, sizeof(path)-1, "db/uid/%d/blob", uid);
	if (!fs_opendir(path, &dirmap))
		return;
	while ((key=fs_readdir(&dirmap)) != NULL) {
		if (*key == '.')
			continue;
		if (!strcmp(key, "pic.jpeg")) {
			key = buf;
			snprintf(key, sizeof(buf)-1, "%d/pic.jpeg", uid);
		}
		obj           = (struct object *)zmalloc(sizeof(*obj));
		obj->blob     = load_blob(uid, key, (int64_t *)&obj->blobsize);
		obj->uid      = uid;
		strncpy(obj->key, key, 15);
		printf(BOLDGREEN "loading object: %p %s %lld" RESET "\n", obj, obj->key, obj->blobsize);
		HASH_ADD_STR(Objects, key, obj);
	}
	fs_closedir(&dirmap);
}

int remove_object(struct session *session, char *URL)
{
	struct object *obj;
	char           path[256];

	HASH_FIND_STR(Objects, URL, obj);
	if (!obj) {
		printf("failed to locate hash: %s\n", URL);
		return 0;
	}
	HASH_DEL(Objects, obj);
//	free(object); XXX
	snprintf(path, sizeof(path)-1, "db/uid/%d/blob/%s", session->user->uid, URL);
	unlink(path);
	return 1;
}

int add_object(char *key, char *blob, unsigned int blobsize, int uid)
{
	struct object *obj;
	char           path[256];
	int            keylen;

	if (unlikely(!key || !blob || !blobsize))
		return 0;

	obj           = (struct object *)zmalloc(sizeof(*obj));
	obj->blobsize = blobsize;
	obj->blob     = blob;
	snprintf(path, sizeof(path)-1, "db/uid/%d/blob/%s", uid, key);
	fs_writefile(path, blob, blobsize);

	keylen = MAX(strlen(key), 15);
	memcpy(obj->key, key, keylen);
	obj->key[keylen] = 0;
	key = obj->key;
	HASH_ADD_STR(Objects, key, obj);
	printf(BOLDCYAN "add_object: %s len: %d" RESET "\n", key, (int)strlen(key));
	return 1;
}

int HTTP_BLOB(char *URL, struct connection *connection)
{
	struct object *obj = NULL;
	char *p;

	if (*(URL+15) == ':') {
		*(URL+15) = 0;
	} else {
		p = strchr(URL, ' ');
		if (!p) {
			SSL_write(connection->ssl, HTTP_404, sizeof(HTTP_404)-1);
			return 0;
		}
		*p = 0;
	}
	HASH_FIND_STR(Objects, URL, obj);
	if (!obj) {
		printf("failed blob: %s\n", URL);
		SSL_write(connection->ssl, HTTP_404, sizeof(HTTP_404)-1);
		return 0;
	}
	printf("serving blob: %s size: %lld\n", URL, obj->blobsize);
	openssl_write_sync(connection, obj->blob, obj->blobsize);
	return 1;
}

struct table **csv_parse_tables(char *csv, int csvsize, int *nr_tables_out)
{
	struct table *table;
	struct table *tables[256];
	struct row   *row;
	char         *column_array[256];
	char         *column  = csv;
	char        **columns = column_array;
	char         *series[256];
	char         *p, *p2, *sp, *rows;
	int           max_columns = 256, nr_tables = 0, nr_series = 0, nr_rows, nr_cells, nr_columns;

	if (csvsize <= 0)
		return NULL;
	csv[csvsize] = 0;
/*
	// CSV Series
	table = zmalloc(sizeof(struct table));
	while (sp=strstr(csv, "--")) {
		series[nr_series++] = sp+2;
		column = strchr(sp, '\n');
		if (!p)
			return NULL;
		*column++ = 0;

		// CSV Headers
		p = strchr(column, '\n');
		if (!p)
			return NULL;
		*p++   = 0;
		rows   = p;
		while (p=strchr(column, ',')) {
			*p++ = 0;
			column_array[table->nr_columns++] = column;
			// realloc column array if greater than max_columns (256 by default, expands by 32 each time)
			if (table->nr_columns >= max_columns) {
				columns = memdup(column_array, (max_columns+32)*sizeof(void *));
				if (!columns)
					return NULL;
				max_columns += 32;
			}
			column = p;
		}
		table->columns = memdup((char *)columns, nr_columns*sizeof(void *));

		// CSV Rows
		nr_rows = line_count(rows);
		table->rows = malloc(sizeof(struct row) * nr_rows);
		while (p=strchr(rows, '\n')) {
			*p++ = 0;
			table->rows[nr_rows++] = row = malloc(sizeof(struct row));
			row->cells = malloc(sizeof(char *) * nr_columns);
			cstring_split(rows, row->cells, nr_columns, ',');
			rows = p;
		}
		tables[nr_tables++] = table;
	}*/
	*nr_tables_out = nr_tables;
	return (struct table **)memdup((char *)tables, nr_tables*sizeof(void *));
}

void upload_csv(struct session *session, struct connection *connection, int action, char *csv_data, char *csv_filename, int csv_size, char **argv)
{
	struct workspace  *workspace;
	struct tables    **tables;
	struct wsid        wsid;
	char               path[256];
	char              *p, *QGID = NULL;
	int                pathsize, chart_type;

	printf("upload_csv: %s\n", csv_data);
	if (!argv[0] || !argv[1])
		return;

	printf("action: %d\n", action);
	switch (action) {
		case ACTION_DATA_CHART_WORKSPACE:
			QGID       = argv[1];
			chart_type = atoi(argv[2]);
			printf("chart_type: %d, QGID: %s\n", chart_type, QGID);asm("int3");
			break;
	}

	if (!workspace_id(session, QGID, &wsid))
		return;
	if (!(workspace=get_workspace(session, &wsid, NULL))) {
		printf(BOLDRED "upload_csv(): get workspace()" RESET "\n");
		return;
	}
//	workspace->tables = csv_parse_tables(csv_data, csv_size, &workspace->nr_tables);
//	if (!workspace->tables)
//		return;

	/* User Upload Chart CSVs */
	fs_mkdir(path, 0644);
	pathsize         = snprintf(path, 24, "db/uid/%d/csv/", session->user->uid);
	random_cookie(path+pathsize);
	pathsize        += 8;
	path[pathsize++] = '.';
	path[pathsize++] = 'c';
	path[pathsize++] = 's';
	path[pathsize++] = 'v';
	printf("path: %s\n", path);
	fs_newfile(path, csv_data, csv_size);
}

void *excel_to_chart(struct job *job)
{
	printf("output: %s\n", job->output);
	return NULL;
}

void *excel_to_datatable(struct job *job)
{
	printf("output: %s\n", job->output);
	return NULL;
}

struct dataset {
	unsigned short  datatype;
	unsigned short  filetype;
	uint64_t        filesize;
	char           *filename;
	char           *hash;
};

void upload_excel(struct session *session, struct connection *connection, int action, char *excel_data, char *excel_filename, int excel_filesize, char **argv)
{
	switch (action) {
		case ACTION_DATA_CHART_WORKSPACE:
			printf("action: data ");
			LIBXLS_rpc_excel2table(session, connection, excel_data, excel_filename, excel_filesize, argv, excel_to_chart);
			break;
		case ACTION_DATA_CHART_SQUEAK:
			printf("action: data ");
			LIBXLS_rpc_excel2table(session, connection, excel_data, excel_filename, excel_filesize, argv, excel_to_chart);
			break;
		case ACTION_DATA_TABLE_WORKSPACE:
			printf("action: data ");
			LIBXLS_rpc_excel2table(session, connection, excel_data, excel_filename, excel_filesize, argv, excel_to_datatable);
			break;
	}
}

struct blob {
	int     objtype;
	int     filetype;
	char   *name;
};

struct blob blobs[] = {};

/*
// Object type
#define OBJTYPE_IMG                    1  // Image    - (png,jpg,jpeg)
#define OBJTYPE_DOC                    2  // Document - (pdf,docx)
#define OBJTYPE_VID                    3  // Video    - (mp4,etc)
#define OBJTYPE_GIF                    6  // GIF      - (gif animation)
#define OBJTYPE_DATA                   7  // Data     - process dataset (CSV,JSON,BSON,etc) to chart/table/app/?
#define OBJTYPE_EXCEL                  8  // Excel    - XLSX file (to be converted to HTML and acted acted on: turn into chart/table/app/?
#define OBJTYPE_APP                    9  // APP      - wasm, pyscript

// File Extensions
#define FILETYPE_TXT                   1
#define FILETYPE_HTML                  2
#define FILETYPE_PDF                   4
#define FILETYPE_PNG                   5
#define FILETYPE_JPG                   5
#define FILETYPE_JPEG                  5
#define FILETYPE_GIF                   6
#define FILETYPE_CSV                   7
#define FILETYPE_EXCEL                 8

// Object Actions
#define ACTION_IMG_WORKSPACE           3  // Send   Image  to a Workspace
#define ACTION_IMG_QUADVERSE           4  // Upload Image  of a custom QuadVerse
#define ACTION_IMG_PROFILE             5  // Upload Image  of a User Profile
#define ACTION_IMG_BG_PROFILE          6  // Upload Image  of a User Profile Background
#define ACTION_IMG_SQUEAK              7  // Upload Image  to a Squeak
#define ACTION_DOC_WORKSPACE           8  // Send   PDF    to a Workspace
#define ACTION_DOC_SQUEAK              9  // Send   PDF    to a Squeak/Tweet
#define ACTION_DATA_CHART_WORKSPACE   10  // Make a Chart     out of a Dataset  (CSV,JSON,etc) OBJTYPE_DATA/DATAFILETYPE_CSV/TICKER||CHART_TITLE/
#define ACTION_DATA_CHART_SQUEAK      11  // Make a Chart     out of a Dataset  (and Squeak it)
#define ACTION_DATA_TABLE_WORKSPACE   12  // Make a DataTable out of a Dataset  (CSV,JSON,etc)
#define ACTION_DATA_TABLE_SQUEAK      13  // Make a DataTable out of a Dataset  (and squeak it)
#define ACTION_DATA_STORE_ENCRYPTED   14  // Send data to P2P connection for storage and encrypt it
#define ACTION_DATA_RANKS_UPDATE      15  // Send ranks.csv for stock ranks update
#define ACTION_APP_PYSCRIPT_WORKSPACE 16  // Load a HTML file with Python in it (into a workspace)
#define ACTION_APP_PYSCRIPT_SQUEAK    17  // Load a HTML file with Python in it (and squeak it)
#define ACTION_APP_CANVAS_WORKSPACE   18  // Load a Canvas App (into a workspace)
#define ACTION_APP_CANVAS_SQUEAK      19  // Load a Canvas App (and squeak it)

#define WEBSOCKET_UPLOAD_IMG     '1' // Image upload into a workspace
#define WEBSOCKET_UPLOAD_PDF     '2' // PDF   upload into a workspace
#define WEBSOCKET_UPLOAD_VID     '3' // Video upload into a workspace
#define WEBSOCKET_UPLOAD_QPIC    '4' // Upload Custom QuadVerse Image
#define WEBSOCKET_UPLOAD_PROPIC  '5' // Upload Custom Profile Image
#define WEBSOCKET_UPLOAD_GIF     '6' // Upload GIF into a workspace
#define WEBSOCKET_UPLOAD_CSV     '7'
#define WEBSOCKET_UPLOAD_EXCEL   '8'
*/

// GET /ws/action/objtype/filetype/filename/filesize/arg1/argX
// new WebSocket("wss://localhost:443/ws/"+action+"/"+objtype+"/"+filetype+"/"+filename.replaceAll(" ","_")+"/"+file.size+"/"+QGID+"/"+URL+"/");
// GET /ws/
//         8/action/objtype/filetype/filesize/filename/arg1/argX
//         /action/objtype/filetype/filesize/filename/arg1/argX

// GET /ws/4/pandas.html/0/filesize:7025/{ARG1}QGID:P0Q1q3ws0/{ARG2}URL:ee8k4rwtj55rftj/
int websocket_upload_object(struct session *session, struct connection *connection, char *req)
{
	struct quadverse *quadverse;
	struct wsid       WSID;
	struct frame      frames[6];
	char              ubuf[128];
	char             *argv[12];
	char             *packet, *blob, *blob_type, *blob_fmt, *QGID, *URL, *filename, *p;
	int               argc, action, objtype, filetype, filesize, objsize, packet_len, nr_frames, hsize, content_len_offset, dirty = 0, qsubargs = 0, websocket_header_size;

	p = strstr(req, " HTTP/1.1\r\n\r\n");
	if (!p)
		return 0;
	*p = 0;

	argc = cstring_split(req, argv, 10, '/');
	printf("req: %s argc: %d\n", req, argc);
	if (argc < 6 || argc >= 10)
		return 0;

	action   = atoi(argv[0]);
	objtype  = atoi(argv[1]);
	filetype = atoi(argv[2]);
	filesize = atoi(argv[3]);
	filename = argv[4];
	printf("argc: %d action: %d objtype: %d filetype: %d filesize: %d filename: %s\n", argc, action, objtype, filetype, filesize, filename);
	argc    -= 5;

	if (filesize <= 0)
		return 0;

	if (filesize > 7 MB)
		packet = (char *)malloc(filesize+1024);
	else
		packet = (char *)alloca(filesize+1024);
	if (!packet)
		return 0;

	if (filesize > 64 KB)
		websocket_header_size = 14;
	else if (filesize > 255)
		websocket_header_size = 8;
	else
		websocket_header_size = 6;
	packet_len = openssl_read_sync(connection, packet, filesize+websocket_header_size);
	if (!packet_len)
		return 0;
	blob = (char *)malloc(packet_len+512);
	if (!blob)
		return 0;
	switch (objtype) {
		case OBJTYPE_IMG:
			strcpy(blob, HTTP_IMAGE2);
			hsize = sizeof(HTTP_IMAGE2)-1;
			content_len_offset = IMG_OFFSET;
			switch (action) {
				case ACTION_IMG_WORKSPACE: // send image to a workspace
					blob_type = "img";
					blob_fmt  = "qupdate img {\"QGID\":\"%s\",\"URL\":\"%s\"}";
					break;
				case ACTION_IMG_QUADVERSE: // alter image of one of your custom QuadVerses
					QGID = NULL;
					break;
				case ACTION_IMG_PROFILE:
					strcpy(session->user->img_url, ubuf);
					snprintf(ubuf, 32, "%d/pic.jpeg", session->user->uid);
					dirty = 1;
					URL = ubuf;
					break;
			}
			break;
		case OBJTYPE_DOC:
			strcpy(blob, HTTP_PDF);
			hsize     = sizeof(HTTP_PDF)-1;
			content_len_offset = PDF_OFFSET;
			blob_type = "pdf";
			blob_fmt  = "qupdate pdf {\"QGID\":\"%s\",\"URL\":\"%s\"}";
			break;
		case OBJTYPE_DATA:
			hsize     = 0;
			qsubargs  = 1;
			break;
	}
	nr_frames = websocket_recv(packet, packet_len, &frames[0], blob+hsize, 1);
	if (nr_frames <= 0)
		return 0;
	objsize = frames[0].data_length;
	if (argc >= 1) {
		// GET /ws/action/objtype/filetype/filesize/filename/argv
		if (argc < 1 || !argv[0] || *argv[0] == '\0')
			return 0;
		switch (filetype) {
			case FILETYPE_CSV:
				upload_csv(session,   connection, action, blob, filename, objsize, &argv[4]);
				return 0;
			case FILETYPE_EXCEL:
				upload_excel(session, connection, action, blob, filename, objsize, &argv[4]);
				return 0;
		}
	}

	printf("websock upload object objsize: %d vs filesize: %d\n", objsize, filesize);
	if (objsize < 0 || objsize > (filesize+10))
		return 0;
	cstring_itoa(blob+content_len_offset, objsize);

	if (!add_object(URL, blob, objsize+hsize, session->user->uid))
		return 0;
	printf("added object: %s\n", URL);
	if (!QGID)
		goto out;

	/* Workspace Object */
	if (!workspace_id(session, QGID, &WSID))
		return 0;
	quadverse = session->quadverse[WSID.QVID];
	if (!quadverse)
		return 0;
	if (session->qcache || quadverse->qpage) {
		if (quadverse->qpage)
			WSID.QVID = 0;
		qcache_add_object(session, quadverse->qpage, blob_type, URL, WSID.QVID, WSID.QSID, WSID.QID, WSID.WSID);
	}

	/* Notify user's other browsers/tabs */
	packet_len = snprintf(packet, 512, blob_fmt, QGID, URL);
	websockets_sendall(session, packet, packet_len);
out:
	if (dirty)
		db_user_update(session->user);
	return 1;
}
