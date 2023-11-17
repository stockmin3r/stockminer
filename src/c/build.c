/* License: Public Domain */
#include <conf.h>

#define WEBSITE_MAINPAGE 0
#define WEBSITE_BACKPAGE 1

static bool  website_load_pages    (struct page **pages, int nr_pages, int PAGE_CONTAINER_INDEX);
static void  gzip_mainpage         (struct page *mainpage);
static void  website_gzip_mainpage (struct page *mainpage);
static char *website_set_domain    (char *website, char *domain);
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

struct external_api {
	const char *url;
	const char *hash;   /* SHA-384 */
	int         license;
	int         permissive;
	int         defer;
} EXTERNAL_API [] = {
/* HEAD SCRIPTS */
{"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery",            "rY/jv8mMhqDabXSo+UCggqKtdmBfd3qC2/KvyTDNQ6PcUJXaxK1tMepoQda4g5vB", LICENSE_MIT,         PERMISSIVE, !DEFER, },
{"https://cdn.datatables.net/v/dt/dt-1.10.21/datatables",                 "qIz2SOTATpqA/FQ0hh1w5JAM66ujgfjBn3VIDHRQALyA23QzYbQvMEtlQXGiX07c", LICENSE_MIT,         PERMISSIVE, !DEFER, },
{"https://code.highcharts.com/stock/highstock",                           "lMRDDWIzVUedPfSFOiOJhlEMnZufWQ2qKHeu2YP5xZHGhBLoaXgNvCTgKSEPwkzn", LICENSE_HIGHCHARTS, !PERMISSIVE, !DEFER, },
{"https://code.highcharts.com/stock/indicators/indicators-all",           "jLpemzsfcRDYsAbWw/hErdwRLuVk6AGjCCocwf3okb6JjF4JBuhMv0W7gDk+NPhP", LICENSE_HIGHCHARTS, !PERMISSIVE, !DEFER, },
{"https://code.jquery.com/ui/1.13.2/jquery-ui",                           "4D3G3GikQs6hLlLZGdz5wLFzuqE9v4yVGAcOH86y23JqBDPzj9viv0EqyfIa6YUL", LICENSE_MIT,         PERMISSIVE, !DEFER, },
{"https://unpkg.com/draggabilly@2.2.0/dist/draggabilly.pkgd",             "JvmwKtrs1CjFC41kayyey74PQsYWrxToeXr0tsEyeYgZwi2aT4XZccR8SFqcobPC", LICENSE_MIT,         PERMISSIVE, !DEFER, },
{"https://cdn.plot.ly/plotly-2.26.0",                                     NULL,                                                               LICENSE_MIT,         PERMISSIVE, !DEFER, },
/* CSS */
{"https://cdnjs.cloudflare.com/ajax/libs/jquery-contextmenu/2.7.1/jquery.contextMenu.min.css",NULL,                                           LICENSE_MIT,         PERMISSIVE, !DEFER, },
{"https://cdnjs.cloudflare.com/ajax/libs/animate.css/4.1.1/animate.min.css",                  NULL,                                           LICENSE_MIT,         PERMISSIVE, !DEFER, },
{"https://code.jquery.com/ui/1.12.1/themes/base/jquery-ui.min.css",                           NULL,                                           LICENSE_MIT,         PERMISSIVE, !DEFER, },
{"https://cdnjs.cloudflare.com/ajax/libs/jquery.terminal/2.34.0/css/jquery.terminal.min.css", NULL,                                           LICENSE_MIT,         PERMISSIVE, !DEFER, },
{"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.5.1/styles/default.min.css",         NULL,                                           LICENSE_BSD,         PERMISSIVE, !DEFER  },
{"https://vjs.zencdn.net/7.19.2/video-js.css",                                                NULL,                                           LICENSE_APACHE2,     PERMISSIVE,  DEFER, },
/* TAIL SCRIPTS */
{"https://cdnjs.cloudflare.com/ajax/libs/dompurify/3.0.6/purify",         "cwS6YdhLI7XS60eoDiC+egV0qHp8zI+Cms46R0nbn8JrmoAzV9uFL60etMZhAnSu", LICENSE_APACHE2_MPL,!PERMISSIVE,  DEFER  },
{"https://code.highcharts.com/modules/data",                              "TqGyq0XPLLamTC1+4jrFsi0mxHPTg8JDL4JXACaMTsh2cMMzmloKZjU9vkjYhZA+", LICENSE_HIGHCHARTS, !PERMISSIVE,  DEFER, },
{"https://cdn.datatables.net/buttons/2.2.3/js/dataTables.buttons",        "+s/TnIu83YK7P52PVQgRXxsBRu3lpXimVpMDKXx4z7l/YqHQ5UgMQFDsSR9LTS0e", LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://cdn.datatables.net/buttons/2.2.3/js/buttons.html5",             NULL,                                                               LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://cdn.datatables.net/plug-ins/1.10.21/sorting/natural",           "h3oS/DGBfrFl5LZtSFk9RFU+pzHmURTX7+CFwAjm6QWSTdjwxCNgPIlzA/On5XBM",                                                               LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://cdnjs.cloudflare.com/ajax/libs/jquery-contextmenu/2.7.1/jquery.ui.position",   NULL,                                                LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://code.highcharts.com/stock/modules/drag-panes",                  NULL,                                                               LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://code.highcharts.com/modules/full-screen",                       NULL,                                                               LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://cdnjs.cloudflare.com/ajax/libs/jquery-contextmenu/2.7.1/jquery.contextMenu",   NULL,                                                LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://vjs.zencdn.net/7.19.2/video",                                   NULL,                                                               LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://cdn.jsdelivr.net/npm/pdfjs-dist@2.0.489/build/pdf",             NULL,                                                               LICENSE_APACHE2,     PERMISSIVE,  DEFER, },
{"https://cdnjs.cloudflare.com/ajax/libs/jszip/3.1.3/jszip",              NULL,                                                               LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://cdnjs.cloudflare.com/ajax/libs/ace/1.6.0/ace",                  NULL,                                                               LICENSE_BSD,         PERMISSIVE,  DEFER, },
{"https://cdnjs.cloudflare.com/ajax/libs/jquery.terminal/2.34.0/js/jquery.terminal", NULL,                                                    LICENSE_MIT,         PERMISSIVE,  DEFER, },
{"https://cdn.jsdelivr.net/gh/jedisct1/libsodium.js@master/dist/browsers-sumo/sodium", "zH4lZRYinll6mtnzkRxsShOLz/cM4UQWrD3TQLMXSoOxTJqj3w+SBr1WzijDAdfS", LICENSE_MIT, PERMISSIVE, DEFER}
};

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
	"	<title>$SITENAME</title>"
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
	struct external_api *script;
	char                *domain, *mainpage, *debug_link;
	char                 buf[1024];
	char                *head_css[32];
	char                *head_js[32];
	char                *defer_js[32];
	char                 hash[512];
	char                *html_files[64];
	char                *head_js_scripts, *head_css_scripts, *defer_js_scripts;
	int                  nbytes, max_buf,  nr_head_css  = 0,  nr_head_js   = 0,  nr_defer_js = 0;
	int                  max_head_js = 32, max_head_css = 32, max_defer_js = 32, debug       = 0;

	if (server->production)
		debug = 1;

	mainpage = strdup((char *)mainpage_template);
	for (int x = 0; x<sizeof(EXTERNAL_API)/sizeof(*script); x++) {
		script = &EXTERNAL_API[x];
		if (strstr(script->url, ".css")) {
			// .css
			nbytes = snprintf(buf, sizeof(buf)-1, "<link rel=\"stylesheet\" ref=\"%s\"/>", (char *)script->url);
			head_css[nr_head_css++] = strdup(buf);
		} else {
			// .js
			if (strstr(script->url, "highcharts")) {
				if (debug)
					debug_link = ".src.js";
				else
					debug_link = ".js";
			} else {
				if (debug)
					debug_link = ".js";
				else
					debug_link = ".min.js";
			}
			if (strstr(script->url, "natural"))
				debug_link = ".js";

			if (script->hash)
				snprintf(hash, sizeof(hash)-1, "integrity=sha384-%s crossorigin=\"anonymous\"", script->hash);
			else
				hash[0] = ' ';

			hash[0] = 0; // maybe i shouldn't have bothered with the integrity hashes after all - disable until fix
			nbytes  = snprintf(buf, sizeof(buf)-1, "<script %s src=\"%s%s\" %s></script>" , (script->defer?"defer":""), script->url, debug_link, hash);
			if (script->defer)
				defer_js[nr_defer_js++] = strdup(buf);
			else
				head_js[nr_head_js++]   = strdup(buf);
		}
	}

	/* CSS */
	max_buf          = 2048;
	nbytes           = 0;
	head_css_scripts = malloc(2048);
	for (int x = 0; x<nr_head_css; x++) {
		nbytes += snprintf(head_css_scripts+nbytes, 256, "%s\n\t", head_css[x]);
		REALLOC(head_css_scripts, char *, nbytes, max_buf);
	}

	/* JavaScript */
	max_buf         = 2048;
	nbytes          = 0;
	head_js_scripts = malloc(2048);
	for (int x = 0; x<nr_head_js; x++) {
		nbytes += snprintf(head_js_scripts+nbytes, 256, "%s\n\t", head_js[x]);
		REALLOC(head_js_scripts, char *, nbytes, max_buf);
	}

	/* JavaScript (defer) */
	max_buf          = 2048;
	nbytes           = 0;
	defer_js_scripts = malloc(2048);
	for (int x = 0; x<nr_defer_js; x++) {
		nbytes += snprintf(defer_js_scripts+nbytes, 256, "%s\n\t", defer_js[x]);
		REALLOC(defer_js_scripts, char *, nbytes, max_buf);
	}

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
	website = zmalloc(sizeof(*website));
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
			page           = zmalloc(sizeof(*page));
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
			mainpage_html = website_set_domain(mainpage_html, server->domain);
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

static char *website_set_domain(char *website, char *domain)
{
	char *p;

	while ((p=cstring_inject(website, domain, "localhost", NULL)))
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

	html_files = malloc(96 KB);
	js_files   = malloc(96 KB);
	css_files  = malloc(96 KB);
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

	mainpage_response_gz = malloc(mainpage->filesize + SIZEOF_MAINPAGE + 256);
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
