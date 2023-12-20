#include <conf.h>
#include <extern.h>

static __inline__ void qcache_path(struct session *session, char *path)
{
	if (!session->user->logged_in)
		snprintf(path, 64, "db/uid/cookie/%s.qcache", session->filecookie);
	else
		snprintf(path, 32, QCACHE_PATH, session->user->uid);
}

void qcache_save(struct session *session, struct qpage *qpage, int QVID, int QSID)
{
	struct qpage   *qpage_file;
	struct filemap  filemap;
	char           *qcache_json;
	char            path[256];
	int             x, qsize, nr_qpages;

	/* Private QuadVerse QCACHE save */
	if (!qpage) {
		qcache_path(session, path);
		printf(BOLDYELLOW "qcache_save path: %s" RESET "\n", path);
		if (!session->qcache) {
			session->qcache = qcache_create(session, QVID, QSID, NULL, 0);
			if (!session->qcache)
				return;
		}
		qcache_json = yyjson_mut_write(session->qcache, 0, NULL);
		if (!qcache_json) {
			printf(BOLDRED "qcache_save: qcache_json is empty" RESET "\n");
			return;
		}
		fs_newfile(path, qcache_json, strlen(qcache_json));
		free(qcache_json);
		return;
	}

	/* Public/Live QuadVerse QCACHE save */
	qcache_json = yyjson_mut_write(qpage->qcache, 0, NULL);
	if (!qcache_json)
		return;
	qsize       = strlen(qcache_json);
	memcpy(qpage->qcache_json, qcache_json, qsize);
	qpage->qcache_json[qsize] = 0;
	qpage->qsize = qsize;
	if (qsize > MAX_QCACHE_SIZE) {
		printf(BOLDRED "CRITICAL SIZE ERROR: qsize: %d qcache: %s" RESET "\n", qsize, qcache_json);
		return;
	}
	sprintf(path, QPAGE_PATH, qpage->owners[0].uid);
	qpage_file = (struct qpage *)MAP_FILE_RW(path, &filemap);
	if (!qpage_file)
		return;
	nr_qpages = filemap.filesize/sizeof(struct qpage);
	for (x=0; x<nr_qpages; x++) {
		if (!strcmp(qpage_file->url, qpage->url)) {
			memcpy(qpage_file->qcache_json, qcache_json, qsize);
			qpage_file->qcache_json[qsize] = 0;
			qpage_file->qsize = qsize;
			break;
		}
		qpage_file++;
	}
	UNMAP_FILE((char *)qpage_file, &filemap);
	free(qcache_json);
}

void session_load_qcache(struct session *session)
{
	yyjson_doc *doc;
	char        path[256];

	qcache_path(session, path);
	doc = yyjson_read_file(path, 0, NULL, NULL);
	if (!doc) {
		session->qcache = NULL;
		return;
	}
//	if (verbose)
//		printf(BOLDYELLOW "loading qcache user: %s cookie: %.7s" RESET "\n", session->user->uname, (char *)&session->user->cookie);
	session->qcache = yyjson_doc_mut_copy(doc, NULL);
}

void qcache_add_object(struct session *session, struct qpage *qpage, char *objname, char *objval, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *Object_array, *obj, *key, *val;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	char path[256];

	snprintf(path, sizeof(path)-1, "/%d/%d/quads/%d/workspace/%d/obj", QVID, QSID, QID, WSID);
	printf("adding %s:%s to: %s\n", objname, objval, path);
	Object_array = yyjson_mut_doc_get_pointer(qcache, path);
	if (!Object_array)
		return;

	obj = yyjson_mut_obj(qcache);
	key = yyjson_mut_strncpy(qcache, objname, strlen(objname));
	val = yyjson_mut_strncpy(qcache, objval,  strlen(objval));
	yyjson_mut_obj_put(obj, key, val);
	yyjson_mut_arr_append(Object_array, obj);
	qcache_save(session, qpage, QVID, QSID);
}

void qcache_add_object2(struct session *session, struct qpage *qpage, char *KEY, char *VAL, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *Object_array, *Object, *obj, *obj_value, *key, *val;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	char path[256], *obj_value_str;
	int idx, max, vlen = strlen(VAL);

	snprintf(path, sizeof(path)-1, "/%d/%d/quads/%d/workspace/%d/obj", QVID, QSID, QID, WSID);
	Object_array = yyjson_mut_doc_get_pointer(qcache, path);
	if (!Object_array)
		return;

	/* Make sure object is not a duplicate */
	yyjson_mut_arr_foreach(Object_array, idx, max, Object)  {
		if (!(obj_value=yyjson_mut_obj_get(Object, KEY)))
			continue;
		obj_value_str = (char *)yyjson_mut_get_str(obj_value);
		if (!strncmp(obj_value_str, VAL, vlen))
			return;
	}

	obj = yyjson_mut_obj(qcache);
	key = yyjson_mut_strncpy(qcache, KEY, strlen(KEY));
	val = yyjson_mut_strncpy(qcache, VAL, strlen(VAL));
	yyjson_mut_obj_put(obj, key, val);
	yyjson_mut_arr_append(Object_array, obj);
	qcache_save(session, qpage, QVID, QSID);
}

void qcache_addchart(struct session *session, struct qpage *qpage, char *ticker, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *Object_array, *Object, *obj, *obj_ticker, *key, *val;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	char path[256], *obj_ticker_str;
	int idx, max, ticker_len = strlen(ticker);

	snprintf(path, sizeof(path)-1, "/%d/%d/quads/%d/workspace/%d/obj", QVID, QSID, QID, WSID);
	Object_array = yyjson_mut_doc_get_pointer(qcache, path);
	if (!Object_array)
		return;

	/* Make sure chart is not a duplicate */
	yyjson_mut_arr_foreach(Object_array, idx, max, Object)  {
		if (!(obj_ticker=yyjson_mut_obj_get(Object, "chart")))
			continue;
		obj_ticker_str = (char *)yyjson_mut_get_str(obj_ticker);
		if (!strncmp(obj_ticker_str, ticker, ticker_len))
			return;
	}

	obj = yyjson_mut_obj(qcache);
	key = yyjson_mut_strncpy(qcache, "chart", 5);
	val = yyjson_mut_strncpy(qcache, ticker, strlen(ticker));
	yyjson_mut_obj_put(obj, key, val);
	yyjson_mut_arr_append(Object_array, obj);
	qcache_save(session, qpage, QVID, QSID);
}

yyjson_mut_val *qcache_search_chart(yyjson_mut_doc *qcache, char *ticker, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *Object_array, *Object, *obj_ticker;
	char path[256], *obj_ticker_str;
	int idx, max, ticker_len = strlen(ticker);

	snprintf(path, sizeof(path)-1, "/%d/%d/quads/%d/workspace/%d/obj", QVID, QSID, QID, WSID);
	Object_array = yyjson_mut_doc_get_pointer(qcache, path);
	if (!Object_array)
		return NULL;
	yyjson_mut_arr_foreach(Object_array, idx, max, Object)  {
		if (!(obj_ticker=yyjson_mut_obj_get(Object, "chart")))
			continue;
		obj_ticker_str = (char *)yyjson_mut_get_str(obj_ticker);
		if (!strncmp(obj_ticker_str, ticker, ticker_len))
			return (Object);
	}
	return NULL;
}

int qcache_chart_indicator_remove(struct session *session, struct qpage *qpage, char *ticker, char *indicator, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *chart, *oldvalue, *newvalue, *key;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	char *p, *p2, *cstr;
	int movelen;

	/* TSLA&macd&atr */
	chart = qcache_search_chart(qcache, ticker, QVID, QSID, QID, WSID);
	if (!chart)
		return 0;

	oldvalue = yyjson_mut_obj_get(chart, "chart");
	if (!oldvalue)
		return 0;
	cstr = (char *)yyjson_mut_get_str(oldvalue);
	if (!cstr)
		return 0;

	p = strstr(cstr, indicator);
	if (!p)
		return 0;
	p2 = strchr(p, '&');
	if (!p2)
		*(p-1) = 0; // only 1 indicator is enabled
	else {
		movelen = strlen(p2+1);
		memmove(p, p2+1, movelen);
		p[movelen] = 0;
	}

	newvalue = yyjson_mut_strcpy(qcache, cstr);
	key      = yyjson_mut_strcpy(qcache, "chart");
	yyjson_mut_obj_put(chart, key, newvalue);
	qcache_save(session, qpage, QVID, QSID);
	return 1;
}

int qcache_chart_indicator_add(struct session *session, struct qpage *qpage, char *ticker, char *indicator, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *chart, *oldvalue, *newvalue, *key;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	const char *cstr;
	char *newstr;
	int old_len, indi_len;

	chart = qcache_search_chart(qcache, ticker, QVID, QSID, QID, WSID);
	if (!chart)
		return 0;

	oldvalue = yyjson_mut_obj_get(chart, "chart");
	if (!oldvalue)
		return 0;
	cstr     = yyjson_mut_get_str(oldvalue);
	if (!cstr)
		return 0;
	old_len  = strlen(cstr);
	indi_len = strlen(indicator) + 2; // & + '\0'

	newstr = (char *)malloc(indi_len+old_len+2);
	memcpy(newstr, cstr, old_len);
	newstr[old_len++] = '&';
	memcpy(&newstr[old_len], indicator, indi_len);
	newstr[old_len+indi_len] = 0;

	newvalue = yyjson_mut_strcpy(qcache, newstr);
	key      = yyjson_mut_strcpy(qcache, "chart");
	yyjson_mut_obj_put(chart, key, newvalue);
	qcache_save(session, qpage, QVID, QSID);
	return 1;
}

void qcache_replace_chart(struct session *session, struct qpage *qpage, char *old_ticker, char *new_ticker, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *Object, *key, *val;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;

	Object = qcache_search_chart(qcache, old_ticker, QVID, QSID, QID, WSID);
	if (!Object)
		return;

	val = yyjson_mut_strncpy(qcache, new_ticker, strlen(new_ticker));
	key = yyjson_mut_strncpy(qcache, "chart", 5);
	yyjson_mut_obj_put(Object, key, val);
	qcache_save(session, qpage, QVID, QSID);
}

void qcache_remove_chart(struct session *session, struct qpage *qpage, char *ticker, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *Object_array, *Object, *val;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	char path[256];
	int idx, max, ticker_len = strlen(ticker);

	snprintf(path, sizeof(path)-1, "/%d/%d/quads/%d/workspace/%d/obj", QVID, QSID, QID, WSID);
	Object_array = yyjson_mut_doc_get_pointer(qcache, path);
	yyjson_mut_arr_foreach(Object_array, idx, max, Object)  {
		if (!(val=yyjson_mut_obj_get(Object, "chart")))
			continue;
		if (yyjson_mut_equals_strn(val, ticker, ticker_len)) {
			yyjson_mut_arr_remove(Object_array, idx);
			break;
		}
	}
	qcache_save(session, qpage, QVID, QSID);
}

void qcache_remove_wstab(struct session *session, struct qpage *qpage, char *obj_type, char *TID, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *Object_array, *Object, *val;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	char path[256];
	char *json_TID;
	int idx, max, TID_len = strlen(TID);


	snprintf(path, sizeof(path)-1, "/%d/%d/quads/%d/workspace/%d/obj", QVID, QSID, QID, WSID);
	Object_array = yyjson_mut_doc_get_pointer(qcache, path);
	yyjson_mut_arr_foreach(Object_array, idx, max, Object)  {
		if (!(val=yyjson_mut_obj_get(Object, obj_type)))
			continue;
		json_TID = (char *)yyjson_mut_get_str(val);
		if (!json_TID)
			continue;
		if (!strncmp(json_TID, TID, TID_len)) {
			yyjson_mut_arr_remove(Object_array, idx);
			break;
		}
	}
	qcache_save(session, qpage, QVID, QSID);
}

void qcache_remove_object(struct session *session, struct qpage *qpage, char *obj_type, char *obj_name, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *Object_array, *Object, *val;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	char path[256];
	int idx, max, obj_name_len = strlen(obj_name);

	sprintf(path, "/%d/%d/quads/%d/workspace/%d/obj", QVID, QSID, QID, WSID);
	Object_array = yyjson_mut_doc_get_pointer(qcache, path);
	yyjson_mut_arr_foreach(Object_array, idx, max, Object)  {
		if (!(val=yyjson_mut_obj_get(Object, obj_type)))
			continue;
		if (yyjson_mut_equals_strn(val, obj_name, obj_name_len)) {
			printf("REMOVING OBJECT: %s\n", obj_name);
			yyjson_mut_arr_remove(Object_array, idx);
			break;
		}
	}
	qcache_save(session, qpage, QVID, QSID);
}

void qcache_setgrid(struct session *session, struct qpage *qpage, char *grid, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	yyjson_mut_val *qkey, *qval;
	char path[256];

	if (WSID == -1)
		sprintf(path, "/%d/%d", QVID, QSID);
	else
		sprintf(path, "/%d/%d/quads/%d/workspace/%d", QVID, QSID, QID, WSID);
	yyjson_mut_val *qobj = yyjson_mut_doc_get_pointer(qcache, path);
	if (!qobj)
		return;
	qkey = yyjson_mut_strncpy(qcache, "grid", 4);
	qval = yyjson_mut_strncpy(qcache, grid, strlen(grid));
	yyjson_mut_obj_put(qobj, qkey, qval);
	qcache_save(session, qpage, QVID, QSID);
}

void qcache_setname(struct session *session, struct qpage *qpage, char *title, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	yyjson_mut_val *qkey, *qval;
	char path[256];
	int len = strlen(title);

	if (len > 24)
		return;

	if (WSID == -1)
		sprintf(path, "/%d/%d", QVID, QSID);
	else
		sprintf(path, "/%d/%d/quads/%d/workspace/%d", QVID, QSID, QID, WSID);

	yyjson_mut_val *qobj = yyjson_mut_doc_get_pointer(qcache, path);
	if (!qobj)
		return;
	qkey = yyjson_mut_strncpy(qcache, "title", 5);
	qval = yyjson_mut_strncpy(qcache, title, len);
	yyjson_mut_obj_put(qobj, qkey, qval);
	qcache_save(session, qpage, QVID, QSID);
}

yyjson_mut_val *qcache_get_blob(yyjson_mut_doc *qcache, char *blob_url, char *blob_type, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_val *Object_array, *Object, *val;
	char            path[256];
	char           *blobstr;
	size_t          idx, max;

	sprintf(path, "/%d/%d/quads/%d/workspace/%d/obj", QVID, QSID, QID, WSID);
	Object_array = yyjson_mut_doc_get_pointer(qcache, path);
	if (!Object_array)
		return NULL;

	yyjson_mut_arr_foreach(Object_array, idx, max, Object)  {
		val = yyjson_mut_obj_get(Object, blob_type);
		if (!val)
			continue;
		blobstr = (char *)yyjson_mut_get_str(val);
		if (!strncmp(blobstr, blob_url, 15))
			return (Object);
	}
	return NULL;
}

void qcache_set_position(struct session *session, struct qpage *qpage, char *blob_url, char *blob_type, int QVID, int QSID, int QID, int WSID)
{
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	yyjson_mut_val *Object, *url,*key;
	int url_size = strlen(blob_url);
	int blobtype_size = strlen(blob_type);

	if (url_size > 64 || blobtype_size > 8)
		return;

	Object = qcache_get_blob(qcache, blob_url, blob_type, QVID, QSID, QID, WSID);
	if (!Object)
		return;
	url = yyjson_mut_strncpy(qcache, blob_url,  url_size);
	key = yyjson_mut_strncpy(qcache, blob_type, blobtype_size);
	yyjson_mut_obj_put(Object, key, url);
	qcache_save(session, qpage, QVID, QSID);
}

yyjson_mut_val *qcache_new_workspace(struct session *session, struct qpage *qpage, char *title, int QVID, int QSID, int QID, int WSID)
{
	char            path[256];
	char           *ws_title = (char *)malloc(32);
	yyjson_mut_doc *qcache   = qpage?qpage->qcache:session->qcache;
	yyjson_mut_val *JWorkspace_array, *JWorkspace;
	yyjson_mut_val *Object_array, *Object_key;
	int             nr_workspaces;

	if (!qcache || !ws_title)
		return NULL;
	snprintf(path, sizeof(path)-1, "/%d/%d/quads/%d/workspace", QVID, QSID, QID);
	printf(BOLDGREEN "qcache_new_workspace: %s title: %s P%dQ%dq%dws%d" RESET "\n", path, title, QVID, QSID, QID, WSID);
	JWorkspace_array = yyjson_mut_doc_get_pointer(qcache, path);
	if (!JWorkspace_array)
		return NULL;
	nr_workspaces = yyjson_mut_arr_size(JWorkspace_array);
	for (int x=nr_workspaces; x<WSID; x++)
		yyjson_mut_arr_add_null(qcache, JWorkspace_array);

	JWorkspace = yyjson_mut_obj(qcache);
	strncpy(ws_title, title, 24);
	yyjson_mut_obj_add_str(qcache, JWorkspace, "title", ws_title);
	yyjson_mut_obj_add_str(qcache, JWorkspace, "grid",  "grid100");

	Object_array = yyjson_mut_arr(qcache);
	Object_key   = yyjson_mut_strncpy(qcache, "obj", 3);
	yyjson_mut_obj_add(JWorkspace, Object_key, Object_array);

	if (WSID >= nr_workspaces || (!WSID && !nr_workspaces)) {
		printf("inserting new workspace: %d nr_workspaces before: %d\n", WSID, nr_workspaces);
		yyjson_mut_arr_insert(JWorkspace_array, JWorkspace, WSID);
	} else {
		printf("replacing new workspace: %d nr_workspaces before: %d\n", WSID, nr_workspaces);
		yyjson_mut_arr_replace(JWorkspace_array, WSID, JWorkspace);
	}
	return (JWorkspace);
}

// {\"title\":\"QuadSpace 6\",\"grid\":\"grid50\",\"quads\":[{\"workspace\":[{\"title\":\"TSLA\",\"grid\":\"grid100\",\"obj\":[{\"chart\":\"TSLA\"}]}]}
void qcache_remove_quadspace(struct session *session, struct qpage *qpage, int QVID, int QSID)
{
	yyjson_mut_val *JQuadspace_array, *empty;
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;

	JQuadspace_array = yyjson_mut_arr_get(qcache->root, QVID);
	printf(BOLDRED "qcache_remove_quadspace: QSID: %d" RESET "\n", QSID);
	empty = yyjson_mut_strcpy(qcache, "-1");
	yyjson_mut_arr_replace(JQuadspace_array, QSID, empty);
	qcache_save(session, qpage, QVID, QSID);
}

void qcache_remove_workspace(struct session *session, struct qpage *qpage, int QVID, int QSID, int QID, int WSID)
{
	char path[256];
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	yyjson_mut_val *JWorkspace_array, *empty;

	snprintf(path, sizeof(path)-1, "/%d/%d/quads/%d/workspace", QVID, QSID, QID);
	JWorkspace_array = yyjson_mut_doc_get_pointer(qcache, path);
	printf("qcache_remove_workspace: %s\n", path);
	empty = yyjson_mut_strcpy(qcache, "-1");
	yyjson_mut_arr_replace(JWorkspace_array, WSID, empty);
	qcache_save(session, qpage, QVID, QSID);
}

yyjson_mut_val *qcache_new_quad(struct session *session, struct qpage *qpage, yyjson_mut_val *Quad_array, int QVID, int QSID, int QID)
{
	yyjson_mut_doc *qcache = qpage?qpage->qcache:session->qcache;
	yyjson_mut_val *Quad, *JWorkspace_array, *JWorkspace, *JWorkspace_key;
	yyjson_mut_val *Object_array, *Object_key;

	Quad             = yyjson_mut_obj(qcache);
	JWorkspace_key   = yyjson_mut_strncpy(qcache, "workspace", 9);
	JWorkspace_array = yyjson_mut_arr(qcache);
	JWorkspace       = yyjson_mut_obj(qcache);
	yyjson_mut_obj_add_str(qcache, JWorkspace, "title", "NewTab");
	yyjson_mut_obj_add_str(qcache, JWorkspace, "grid",  "grid100");

	Object_array = yyjson_mut_arr(qcache);
	Object_key   = yyjson_mut_strncpy(qcache, "obj", 3);
	yyjson_mut_obj_add(JWorkspace, Object_key, Object_array);
	yyjson_mut_obj_put(Quad, JWorkspace_key, JWorkspace_array);
	yyjson_mut_arr_insert(Quad_array, Quad, QID);
	return (JWorkspace);
}

yyjson_mut_doc *qcache_new_quadspace(struct session *session, struct qpage *qpage, int QVID, int QSID, int *broadcast, char *title, int stockpage)
{
	char           *qsp_title;
	char           *qsp_grid = NULL;
	yyjson_mut_doc *qcache   = qpage?qpage->qcache:session->qcache;
	yyjson_mut_val *JQuadspace, *JQuadverse_array, *JQuadspace_array, *Quad_array, *quad_key;
	int              nr_quadspaces, nr_quadverses, x;

	JQuadverse_array = qcache->root;
	nr_quadverses    = yyjson_mut_arr_size(JQuadverse_array);
	JQuadspace_array = yyjson_mut_arr_get (JQuadverse_array, QVID);
	if (((QVID >= nr_quadverses) && QVID) || !JQuadspace_array || yyjson_mut_is_null(JQuadspace_array)) {
		JQuadspace_array = yyjson_mut_arr(qcache);
		for (x=nr_quadverses; x<QVID; x++)
			yyjson_mut_arr_add_null(qcache, JQuadverse_array);
		if (QVID >= nr_quadverses)
			yyjson_mut_arr_insert(JQuadverse_array, JQuadspace_array, QVID);
		else
			yyjson_mut_arr_replace(JQuadverse_array, QVID, JQuadspace_array);
		if (broadcast)
			*broadcast = 1;
		printf("block 1 broadcast: %d\n", QSID);
	} else {
		JQuadspace = yyjson_mut_arr_get(JQuadspace_array, QSID);
		if (!JQuadspace || yyjson_mut_is_null(JQuadspace) ||  yyjson_mut_equals_str(JQuadspace, "-1")) {
			printf("yyjson_mut_is_null QSID: %d broadcasting\n", QSID);
			if (broadcast)
				*broadcast = 1;
		} else printf("yyjson_mut_is_null FAIL: %d nr_QVID: %d res: %p\n", QSID, nr_quadverses ,yyjson_mut_arr_get(JQuadspace_array, QSID));
	}

	JQuadspace = yyjson_mut_obj(qcache);
	nr_quadspaces = yyjson_mut_arr_size(JQuadspace_array);
	for (int x=nr_quadspaces; x<QSID; x++)
		yyjson_mut_arr_add_null(qcache, JQuadspace_array);
	if (QSID >= nr_quadspaces || (!QSID && !nr_quadspaces))
		yyjson_mut_arr_insert(JQuadspace_array, JQuadspace, QSID);
	else
		yyjson_mut_arr_replace(JQuadspace_array, QSID, JQuadspace);

	if (stockpage) {
		qsp_title = strdup(title);
		yyjson_mut_obj_add_str(qcache, JQuadspace, "title", qsp_title);
		yyjson_mut_obj_add_int(qcache, JQuadspace, "sp", 1UL);
	} else {
		qsp_title  = (char *)malloc(16);
		sprintf(qsp_title, "QuadSpace^%d", QSID);
		qsp_grid   = strdup("grid50");
		yyjson_mut_obj_add_str(qcache, JQuadspace, "title", qsp_title);
		yyjson_mut_obj_add_str(qcache, JQuadspace, "grid",  qsp_grid);
		Quad_array = yyjson_mut_arr(qcache);
		quad_key   = yyjson_mut_strncpy(qcache, "quads", 5);
		yyjson_mut_obj_add(JQuadspace, quad_key, Quad_array);
		// 4 is static here but really it should be configurable
		for (int x=0; x<4; x++) {
			qcache_new_quad(session, qpage, Quad_array, QVID, QSID, x);
			qcache_new_workspace(session, qpage, "NewTab", QVID, QSID, x, 0);
		}
	}
	qcache_save(session, qpage, QVID, QSID);
	return (qcache);
}

yyjson_mut_doc *qcache_create(struct session *session, int QVID, int QSID, char *title, int stockpage)
{
	yyjson_mut_doc   *qcache           = yyjson_mut_doc_new(NULL);
	yyjson_mut_val   *JQuadverse_array = yyjson_mut_arr(qcache);
	yyjson_mut_val   *JQuadspace_array, *JQuadspace;

	if (!JQuadverse_array)
		return NULL;
	yyjson_mut_doc_set_root(qcache, JQuadverse_array);
	for (int x=0; x<QVID; x++)
		yyjson_mut_arr_add_null(qcache, JQuadverse_array);

	JQuadspace_array = yyjson_mut_arr(qcache);
	for (int x=0; x<QSID; x++)
		yyjson_mut_arr_add_null(qcache, JQuadspace_array);

	if (!qcache)
		return NULL;
	session->qcache = qcache;
	qcache_new_quadspace(session, NULL, QVID, QSID, NULL, title, stockpage);
	return session->qcache;
}

int conf_private_watchlists(struct session *session, char *packet)
{
	struct watchlist *watchlist;
	char             *username;
	int               nbytes, packet_len = 0, nr_watchlists;

	nr_watchlists = session->nr_watchlists;
	if (nr_watchlists == 1)
		return 0;
	strcpy(packet, "watchlist");
	packet_len = 5;
	for (int x=0; x<nr_watchlists; x++) {
		watchlist = session->watchlists[x];
		if (!watchlist || watchlist->config == IS_MORPHTAB)
			continue;
		if (!session->user || !session->user->logged_in)
			username = "Anonymous";
		else
			username = session->user->uname;
		// XXX: REALLOC+snprintf
		nbytes      = sprintf(packet+packet_len, " %s:%d:%s:%d:%s", watchlist->name, watchlist->nr_conditions, (char *)&watchlist->watchlist_id, watchlist->nr_stocks, username);
		packet_len += nbytes;
	}
	if (packet_len == 5)
		return 0;
	*(packet+packet_len++) = '@';
	*(packet+packet_len)   = 0;
	return (packet_len);
}

int conf_public_watchlists(char *packet)
{
	struct watchlist *watchlist;
	char             *username;
	int               nbytes, packet_len = 0, nr_watchlists;

	nr_watchlists = nr_global_watchlists;
	if (!nr_watchlists)
		return 0;
	strcpy(packet, "gwatch");
	packet_len = 6;
	for (int x=0; x<nr_watchlists; x++) {
		watchlist = Watchlists[x];
		if (!watchlist)
			continue;
		if (!watchlist->owner->user || !watchlist->owner->user->logged_in)
			username = "Anonymous";
		else
			username = watchlist->owner->user->uname;
		nbytes      = sprintf(packet+packet_len, " %s:%d:%s:%d:%s", watchlist->name, watchlist->nr_conditions, (char *)&watchlist->watchlist_id, watchlist->nr_stocks, username);
		packet_len += nbytes;
	}
	*(packet+packet_len++) = '@';
	*(packet+packet_len) = 0;
	if (verbose)
		printf("global watchlist packet: %s\n", packet);
	return (packet_len);
}

int conf_private_alerts(struct session *session, char *packet)
{
	struct watchlist *watchlist;
	struct watchcond *watchcond;
	int               nr_watchlists, nr_alerts, packet_len = 0, has_alerts = 0;

	nr_watchlists = session->nr_watchlists;
	for (int x=0; x<nr_watchlists; x++) {
		watchlist = session->watchlists[x];
		nr_alerts = watchlist->nr_conditions;
		if (nr_alerts && !has_alerts) {
			strcpy(packet, "exec ");
			packet_len = 5;
		}
		for (int y=0; y<nr_alerts; y++) {
			watchcond = &watchlist->conditions[y];
			memcpy(packet+packet_len, watchcond->exec, watchcond->exec_len);
			packet_len += watchcond->exec_len;
			packet[packet_len] = '!';
			packet_len += 1;
			has_alerts = 1;
		}
	}
	if (packet_len == 5)
		return 0;
	packet[packet_len-1] = '@';
	return (packet_len);
}

int conf_public_alerts(char *packet)
{
	struct watchlist *watchlist;
	struct watchcond *watchcond;
	int               nr_watchlists, nr_alerts, packet_len = 6, exec_len;

	strcpy(packet, "gexec ");
	nr_watchlists = nr_global_watchlists;
	for (int x=0; x<nr_watchlists; x++) {
		watchlist = Watchlists[x];
		nr_alerts = watchlist->nr_conditions;
		for (int y=0; y<nr_alerts; y++) {
			watchcond          = &watchlist->conditions[y];
			memcpy(packet+packet_len, watchcond->exec, watchcond->exec_len);
			packet_len        += watchcond->exec_len;
			packet[packet_len] = '!';
			packet_len        += 1;
		}
	}
	if (packet_len == 6)
		return 0;
	packet[packet_len-1] = '@';
	packet[packet_len]   = 0;
	printf(BOLDBLUE "GEXEC: %s exec_len: %d" RESET "\n", packet, packet_len);
	return (packet_len);
}


int conf_styles(struct session *session, char *packet)
{
	struct style *style = session->styles;
	int nr_css, packet_len, css_name_len;

	if (!style)
		return 0;
	strcpy(packet, "css ");
	packet_len = 4;

	/* Private WatchTable Styles */
	nr_css = session->nr_table_css;
	for (int x=0; x<nr_css; x++) {
		char *css_name = style->css_name[x];
		if (!css_name)
			continue;
		packet_len += sprintf(packet+packet_len, " %s-0", css_name);
	}

	/* Public WatchTable Styles */
/*	nr_css = Global.nr_public_css_themes;
	for (int x=0; x<nr_css; x++) {
		char *css = Global.css_themes[x];
		css_name_len = strlen(css);
		if (css_name_len > 32)
			continue;
		memcpy(packet+packet_len, css, css_name_len);
		packet_len += css_name_len;
		packet[packet_len++];
	}*/
	if (packet_len == 4)
		return 0;
	packet[packet_len++] = '@';
	return (packet_len);
}

void apc_json_clear(struct connection *connection, char **argv)
{
	struct session *session = session_by_username(argv[0]);

	if (!session) {
		printf(BOLDRED "cmd_json_clear(): username: %s does not exist" RESET "\n", argv[0]);
		return;
	}
	session->qcache = NULL;
}

void apc_obj_list(struct connection *connection, char **argv)
{
	struct object *obj = NULL;
	struct object *tmp = NULL;

	HASH_ITER(hh, Objects, obj, tmp) {
		printf("obj: %p key: %s uid: %d\n", obj, obj->key, obj->uid);
	}
}

