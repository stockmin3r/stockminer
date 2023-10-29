#include <stdinc.h>
#include <conf.h>

static char *test_scripts;
static int   test_scripts_len;
static int   max_scripts_len;

void init_webscript()
{
	struct dirmap  dirmap;
	struct stat    sb;
	char          *file, *filename;
	char           path[512];
	int64_t        filesize, pathlen;

	if (!fs_opendir("scripts/jobs", &dirmap))
		return;
	test_scripts    = (char *)malloc(64 KB);
	max_scripts_len = 64 KB;
	strcpy(test_scripts, "qsh ");
	test_scripts_len = 4;
	while ((filename=fs_readdir(&dirmap)) != NULL) {
		if (*filename == '.')
			continue;
		pathlen = snprintf(path, sizeof(path)-32, "scripts/jobs/%s", filename);
		if (path[pathlen-1] == '~')
			continue;
		if (stat(path, &sb) == -1)
			continue;
		file = fs_mallocfile_str(path, &filesize);
		if (!file)
			continue;
		if (test_scripts_len + filesize + 4 > max_scripts_len) {
			max_scripts_len += MAX(64 KB, filesize+1);
			test_scripts     = (char *)realloc(test_scripts, max_scripts_len);
			if (!test_scripts) {
				printf(BOLDRED "[-] no memory to allocate test scripts!" RESET "\n");
				exit(-1);
			}
		}
		test_scripts_len += snprintf(test_scripts+test_scripts_len, max_scripts_len-test_scripts_len-32, "%s%s!", filename, file);
		free(file);
	}
	fs_closedir(&dirmap);
}
