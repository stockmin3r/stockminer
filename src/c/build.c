/* License: Public Domain */
#include <conf.h>

#define WEBSITE_MAINPAGE 0
#define WEBSITE_BACKPAGE 1

static bool  website_load_pages    (struct page **pages, int nr_pages, int PAGE_CONTAINER_INDEX);
static void  gzip_mainpage         (struct page *mainpage);
static void  website_gzip_mainpage (struct page *mainpage);
static char *website_set_domain    (char *website, char *domain, port_t port);
static char *website_set_localhost (char *website, port_t port);
static void  free_pages            (struct page *page);

#define DEFER               1
#define JS                  0
#define CSS                 1
#define PERMISSIVE          1
#define LICENSE_PUBLIC      0
#define LICENSE_MIT         1
#define LICENSE_BSD         2
#define LICENSE_ISC         3
#define LICENSE_APACHE2     4
#define LICENSE_APACHE2_MPL 5
#define LICENSE_MPL         6
#define LICENSE_GPL         7
#define LICENSE_HIGHCHARTS  8

const char *mainpage_template =
	"<!--License: Public Domain -->"
	"<!DOCTYPE html>"
	"<html>"
	"<head>"
	"	<meta charset='utf-8'/>"
	"   <meta property='og:type'  content='website'>"
	"   <meta property='og:url'   content='https://www.stockminer.org/'>"
	"   <meta property='og:title' content='Stockminer'>"
	"   <meta property='og:description' content='Stock trading algorithms & Social/Tech Media'>"
	"   <meta property='og:image' content='https://i.imgur.com/YOqdQUX.png'>"
	"	<title>stockminer.org</title>"
	"	$HEAD_JS"
	"	$HEAD_CSS"
	"	<style>"
	"		$CSS"
	"	</style>"
	"</head>"
	"<body>"
	"	$HTML"
	"<script>$JS</script>"
	"	$DEFER_JS"
	"</body>"
	"</html>";

const char *backpage_template =
	"backpage-monster<style>$CSS</style>"
	"$HTML"
	"<script>$JS</script>";

char *build_mainpage(struct server *server)
{
	char                *mainpage;
	char                 buf[1024 KB];
	char                 script[1024];
	char                *argv[5];
	char                *head_css[32];
	char                *head_js[32];
	char                *defer_js[32];
	char                *html_files[64];
	char                *head_js_scripts, *head_css_scripts, *defer_js_scripts;
	char                *line, *url, *license, *permissive, *defer;
	bool                 deferred;
	int                  nbytes, max_buf,  nr_head_css  = 0,  nr_head_js   = 0,  nr_defer_js = 0;
	int                  max_head_js = 32, max_head_css = 32, max_defer_js = 32, debug       = 0;
	int                  nr_lines, argc;

	if (server->production)
		debug = 1;

	fs_readfile_str("config/website.csv", buf, sizeof(buf));
	nr_lines = cstring_line_count(buf);
	if (!nr_lines || nr_lines > 300)
		return NULL;

	char *lines[nr_lines];
	cstring_split(buf, lines, nr_lines, '\n');
	for (int x = 0; x<nr_lines; x++) {
		line = lines[x];
		if (line[0] == '#')
			continue;
		argc  = cstring_split(line, argv, 4, ',');
		if (argc != 4)
			return NULL;
		url        = argv[0];
		license    = argv[1];
		permissive = argv[2];
		defer      = argv[3];
		while (*license == ' ')
			license++;
		while (*permissive)
			permissive++;
		while (*defer == ' ')
			defer++;
		if (*defer == '!')
			deferred = false;
		else
			deferred = true;
		if (strstr(url, ".css")) {
			snprintf(script, sizeof(script)-1, "<link rel=\"stylesheet\" ref=\"%s\"/>", url);
			head_css[nr_head_css++] = strdup(script);
		} else {
			snprintf(script, sizeof(script)-1, "<script %s src=\"%s\"></script>" , (deferred?"defer":""), url);
			if (deferred)
				defer_js[nr_defer_js++] = strdup(script);
			else
				head_js[nr_head_js++]   = strdup(script);
		}
	}

	/* CSS */
	max_buf          = 2048;
	nbytes           = 0;
	head_css_scripts = (char *)malloc(2048);
	for (int x = 0; x<nr_head_css; x++) {
		nbytes += snprintf(head_css_scripts+nbytes, 256, "%s\n\t", head_css[x]);
		REALLOC(head_css_scripts, char *, nbytes, max_buf);
	}

	/* JavaScript */
	max_buf         = 2048;
	nbytes          = 0;
	head_js_scripts = (char *)malloc(2048);
	for (int x = 0; x<nr_head_js; x++) {
		nbytes += snprintf(head_js_scripts+nbytes, 256, "%s\n\t", head_js[x]);
		REALLOC(head_js_scripts, char *, nbytes, max_buf);
	}

	/* JavaScript (defer) */
	max_buf          = 2048;
	nbytes           = 0;
	defer_js_scripts = (char *)malloc(2048);
	for (int x = 0; x<nr_defer_js; x++) {
		nbytes += snprintf(defer_js_scripts+nbytes, 256, "%s\n\t", defer_js[x]);
		REALLOC(defer_js_scripts, char *, nbytes, max_buf);
	}

	mainpage = strdup((char *)mainpage_template);
	mainpage = cstring_inject(mainpage, head_js_scripts,  "$HEAD_JS",  NULL);
	mainpage = cstring_inject(mainpage, head_css_scripts, "$HEAD_CSS", NULL);
	mainpage = cstring_inject(mainpage, defer_js_scripts, "$DEFER_JS", NULL);
	return (mainpage);
}

/*
 * Main build function: called by www.c and on website reload
 */
struct website *build_website(struct server *server)
{
	struct website *website;
	struct page    *page;
	struct dirmap   dirmap;
	char           *filename, *directory, *mainpage_html, *backpage_html;
	char           *page_dirs[] = { "src/website/mainpage", "src/website/backpage" };
	char            path[1024];
	char            backpage_json[256];
	char            mainpage_json[256];

	/*****************************************************************
	 * Allocate memory for the "website" and all the struct page's that
	 * represent src/website/{mainpage,backpage}
	 */
	website = (struct website *)zmalloc(sizeof(*website));
	if (!website)
		return NULL;
	website->mainpage = (struct page **)zmalloc(sizeof(struct page *) * 4);
	website->backpage = (struct page **)zmalloc(sizeof(struct page *) * 4);

	/*****************************************************************
	 * Load the names of all the mainpages && backpages
	 */
	for (int x = 0; x < sizeof(page_dirs)/sizeof(char *); x++) {
		directory = page_dirs[x];
		if (!fs_opendir(directory, &dirmap))
			continue;

		while ((filename=fs_readdir(&dirmap)) != NULL) {
			if (*filename == '.')
				continue;
			page           = (struct page *)zmalloc(sizeof(*page));
			page->filename = filename;
			if (strstr(directory, "mainpage"))
				website->mainpage[website->nr_mainpages++] = page;
			else if (strstr(directory, "backpage"))
				website->backpage[website->nr_backpages++] = page;
		}
	}

	/********************************************************************
	 * Load the HTML,JS,CSS SRC FILES for all mainpages and all backpages
	 */
	website_load_pages(website->mainpage, website->nr_mainpages, WEBSITE_MAINPAGE);
	website_load_pages(website->backpage, website->nr_backpages, WEBSITE_BACKPAGE);

	/***************************************************************
	 * Inject <script> & <link> tags into the mainpage.html template
	 */
	mainpage_html = build_mainpage(server);
	if (!mainpage_html)
		return NULL;

	/***************************************************************************************
	 * Inject the html+js+css into the $HTML|$JS|$CSS sections of the mainpage.html template
	 */
	for (int x = 0; x<website->nr_mainpages; x++) {
		page           = website->mainpage[x];
		mainpage_html  = cstring_inject(mainpage_html, page->css,  "$CSS",  NULL);
		mainpage_html  = cstring_inject(mainpage_html, page->html, "$HTML", NULL);
		mainpage_html  = cstring_inject(mainpage_html, page->js,   "$JS",   NULL);

		// inject the website.json for this mainpage
		snprintf(path, sizeof(path)-1, "src/website/mainpage/%s/website.json", page->filename);
		page->json     = fs_mallocfile_str(path, NULL);
		snprintf(mainpage_json, sizeof(mainpage_json)-1,  "@MAINPAGE_%s", page->filename);
		mainpage_html  = cstring_inject(mainpage_html, page->json, mainpage_json, NULL);
		if (server->production)
			mainpage_html = website_set_domain(mainpage_html, server->domain, 443);
		else
			mainpage_html = website_set_localhost(mainpage_html, server->https_port);
		page->file     = mainpage_html;
		page->filesize = strlen(mainpage_html);
		website_gzip_mainpage(page); // sets HTTP_MAINPAGE header + gzipped content of page->file
		free_pages(page);
	}
	if (!mainpage_html)
		return NULL;

	/***************************************************************************************
	 * Inject the html+js+css into the $HTML|$JS|$CSS sections of the backpage.html template
	 */
	backpage_html = strdup(backpage_template);
	for (int x = 0; x<website->nr_backpages; x++) {
		page           = website->backpage[x];
		backpage_html  = cstring_inject(backpage_html, page->css,  "$CSS",  NULL);
		backpage_html  = cstring_inject(backpage_html, page->html, "$HTML", NULL);
		backpage_html  = cstring_inject(backpage_html, page->js,   "$JS",   NULL);

		// inject the website.json for this mainpage
		snprintf(path, sizeof(path)-1, "src/website/backpage/%s/website.json", page->filename);
		page->json     = fs_mallocfile_str(path, NULL);
		snprintf(backpage_json, sizeof(backpage_json)-1,  "@BACKPAGE_%s", page->filename);
		backpage_html  = cstring_inject(backpage_html, page->json, backpage_json, NULL);
		page->file     = backpage_html;
		page->filesize = strlen(backpage_html);
		free_pages(page);
	}
	return (website);
}

static char *website_set_domain(char *website, char *domain, port_t port)
{
	char *p;
	char hostport[256];

	while ((p=cstring_inject(website, domain, "localhost", NULL)))
		website = p;

	hostport[0] = ':';
	cstring_itoa(hostport+1, port);
	while ((p=cstring_inject(website, hostport, ":port", NULL)))
		website = p;
	return (website);
}

static char *website_set_localhost(char *website, port_t port)
{
	char  *p;
	char   hostname_port[64];

	snprintf(hostname_port, sizeof(hostname_port)-1, "localhost:%d", port);
	while ((p=cstring_inject(website, hostname_port, "localhost:port", NULL)))
		website = p;

	return (website);
}

static bool
website_load_pages(struct page **pages, int nr_pages, int PAGE_CONTAINER_INDEX)
{
	struct page    *page;
	struct dirmap   dirmap;
	char           *filename, *file, *page_name;
	char           *html_files, *js_files, *css_files;
	char            dirpath[512];
	char            filepath[1024];
	int             max_html_size = 96 KB, max_js_size = 96 KB, max_css_size = 96 KB;
	int             html_filesize = 0, js_filesize = 0, css_filesize = 0;
	int64_t         filesize;
	char           *page_dirs[] = { "src/website/mainpage", "src/website/backpage" };
	char           *src_dirs[]  = { "css", "js", "html" };

	html_files = (char *)malloc(96 KB);
	js_files   = (char *)malloc(96 KB);
	css_files  = (char *)malloc(96 KB);
	html_files[0] = 0;
	js_files[0] = 0;
	css_files[0] = 0;
	if (!html_files || !js_files || !css_files)
		return NULL;
	for (int x = 0; x < nr_pages; x++) {
		for (int y = 0; y<sizeof(src_dirs)/sizeof(char *); y++) {
			page      = pages[x];
			page_name = page->filename;
			snprintf(dirpath, sizeof(dirpath)-1, "%s/%s/%s", page_dirs[PAGE_CONTAINER_INDEX], page_name, src_dirs[y]);
			if (!fs_opendir(dirpath, &dirmap))
				continue;

			while ((filename=fs_readdir(&dirmap)) != NULL) {
				if (*filename == '.' || strchr(filename, '~'))
					continue;
				snprintf(filepath, sizeof(filepath)-1, "%s/%s", dirpath, filename);
				file = fs_mallocfile_str(filepath, &filesize);
				if (!file || !filesize)
					continue;

				if (strstr(filename, ".html")) {
					REALLOC(html_files, char *, (html_filesize+filesize), max_html_size);
					strcat(html_files, file);
					html_filesize += filesize;
				} else if (strstr(filename, ".js")) {
					REALLOC(js_files, char *, (js_filesize+filesize), max_js_size);
					strcat(js_files, file);
					js_filesize += filesize;
				} else if (strstr(filename, ".css")) {
					REALLOC(css_files, char *, (css_filesize+filesize), max_css_size);
					strcat(css_files, file);
					css_filesize += filesize;
				}
			} // for (all source code filenames in this html|js|css directory)
		} // for (all source code directories (html|js|css) in this mainpage or backpage)
		page->html = html_files;
		page->js   = js_files;
		page->css  = css_files;
	} // for (all mainpages or backpages)
	return true;
}

static void
website_gzip_mainpage(struct page *mainpage)
{
	char          ETag[36];
	unsigned char ebuf[36];
	char         *mainpage_response_gz;
	int           zbytes;

	mainpage_response_gz = (char *)malloc(mainpage->filesize + SIZEOF_MAINPAGE + 256);
	strcpy(mainpage_response_gz, HTTP_MAINPAGE);
	zbytes = zip_compress(mainpage->file, mainpage_response_gz+SIZEOF_MAINPAGE, mainpage->filesize);
	md5_string(md5_checksum(mainpage_response_gz+SIZEOF_MAINPAGE, ebuf, zbytes), ETag);
	mainpage->etag = *(uint64_t *)ETag;
	*(uint64_t  *)(strstr(mainpage_response_gz, "ETag:")   +7) = *(uint64_t *)ETag;
	cstring_itoa ((strstr(mainpage_response_gz, "Length: ")+8), zbytes);
	free(mainpage->file);
	mainpage->file = mainpage_response_gz;
	mainpage->filesize = (SIZEOF_MAINPAGE+zbytes);
}

static void
free_pages(struct page *page)
{
	free(page->html);
	free(page->js);
	free(page->css);
	page->html = NULL; page->js = NULL; page->css = NULL;
}

/*
// from ancient iteration of build.c
void minify_page(struct page *page)
{
	char cmd[256];
	char path[256];

	// 1) Write page to build/page first
	unlink("build/page");
	fs_writefile("build/page", (char *)page->file, page->filesize);
	if (page->filetype == JS_FILE) {
		// 2) Invoke Minifier on build/page
		sprintf(cmd, "terser build/page > build/%s", page->filename);
		system(cmd);
	} else {
		sprintf(cmd, "html-minifier --minify-css true --remove-comments --collapse-whitespace build/page > build/%s", page->filename);
		system(cmd);
	}
	// 3) Reload minified page into pages structure
	sprintf(path, "build/%s", page->filename);
	page->file = fs_mallocfile((char *)page->path, &page->filesize);
}
*/
