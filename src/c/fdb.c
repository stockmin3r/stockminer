#include <conf.h>

void fdb_user_list(void)
{
	struct filemap  filemap;
	struct user    *user;
	char            buf[256];
	char            daystr[256];
	char           *umap;
	time_t          timenow = time(NULL);
	int             nr_users, days;

	umap     = MAP_FILE_RO("db/users.db", &filemap);
	nr_users = (filemap.filesize/sizeof(struct user));
	user     = (struct user *)umap;
	for (int x=0; x<nr_users; x++) {
		days = (timenow-user->join_date)/86400;
		snprintf(daystr, sizeof(daystr)-1, BOLDGREEN "(%d days)" RESET, days);
		printf(BOLDBLUE "%-24s [uid: %d] %s logged_in: %d" RESET "\n", user->uname, user->uid, daystr, user->logged_in);
		user++;
	}
	UNMAP_FILE(umap, &filemap);
}

uid_t fdb_user_uid(char *username)
{
	struct user    *user;
	struct filemap  filemap;
	char           *umap;
	int             nr_users, id = -1;

	umap     = MAP_FILE_RO("db/users.db", &filemap);
	nr_users = (filemap.filesize/sizeof(struct user));
	user     = (struct user *)umap;
	for (int x=0; x<nr_users; x++) {
		if (!strcmp(username, user->uname)) {
			id = x;
			break;
		}
		user++;
	}
	UNMAP_FILE(umap, &filemap);
	return (id);
}

void fdb_watchlist(char *username)
{
	struct watchlist  *watchlist;
	struct watchstock *watchstock;
	struct filemap     filemap;
	struct user       *user;
	char              *umap, *stock;
	char               path[256];
	uid_t              uid;
	int                nr_watchlists, nr_users, count = 0, x, y;

	uid = fdb_user_uid(username);
	if (uid >= 0) {
		snprintf(path, sizeof(path)-1, "db/uid/%d.watch", uid);
		umap = MAP_FILE_RO(path, &filemap);
		if (!umap) {
			printf(BOLDRED "user: %s has no watchlists" RESET "\n", username);
			return;
		}
	} else {
		snprintf(path, sizeof(path)-1, "db/uid/free/%s.watch", username);
		umap = MAP_FILE_RO(path, &filemap);
		if (!umap) {
			printf(BOLDRED "user: %s not found" RESET "\n", username);
			return;
		}
	}
	watchlist = (struct watchlist *)umap;
	nr_watchlists = (filemap.filesize/sizeof(struct watchlist));
	for (x=0; x<nr_watchlists; x++) {
		printf("WatchList Name:   %s %s\n", watchlist->name, watchlist->config==IS_PUBLIC?"(PUB)":"(PRIV)");
		printf("BaseTable:        %s\n", watchlist->basetable);
		printf("Number of Stocks: %d\n", watchlist->nr_stocks);
		printf("--------------------\n");
		for (y=0; y<watchlist->nr_stocks; y++) {
			watchstock = &watchlist->stocks[y];
			if (count++ > 6) {
				printf("\n");
				count = 0;
			}
			printf(BOLDWHITE "%-8s " RESET, watchstock->ticker);
		}
		printf(BOLDGREEN "\n--------------------------------------" RESET "\n");
		watchlist++;
	}
	UNMAP_FILE(path, &filemap);
}

void fdb_tradetime(char *ticker)
{
	struct filemap    filemap;
	struct tradetime *tradetime;
	char              path[256];

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.tradetime", ticker);
	tradetime = (struct tradetime *)MAP_FILE_RO(path, &filemap);
	if (!tradetime) {
		printf("no tradetime data for: %s\n", ticker);
		exit(-1);
	}
	printf("4am: %d\n", tradetime->time_4AM);
	printf("5am: %d\n", tradetime->time_5AM);
	printf("6am: %d\n", tradetime->time_6AM);
	printf("7am: %d\n", tradetime->time_7AM);
	printf("8am: %d\n", tradetime->time_8AM);
	printf("9am: %d\n", tradetime->time_9AM);
}
