#include <conf.h>
#include <watchlist.h>
#include <stocks/algo.h>
#include <watchtable.h>

void watchlist_path(struct session *session, char *path)
{
	struct user *user = session->user;
	char cookie[12]   = {0};

	if (!user->logged_in) {
		sprintf(path, "db/uid/cookie/%s.watch", session->filecookie);
	} else
		sprintf(path, "db/uid/%d.watch", user->uid);
}

struct watchlist *map_watchlist(struct session *session, char *wpath, char *watchlist_name, char **map, struct filemap *filemap)
{
	struct watchlist *watchlist;
	int nr_watchlists, x;

	watchlist = (struct watchlist *)MAP_FILE_RW(wpath, filemap);
	if (!watchlist)
		return NULL;
	*map      = (char *)watchlist;
	nr_watchlists = (filemap->filesize/sizeof(struct watchlist));
	for (x=0; x<nr_watchlists; x++) {
		if (!strcmp(watchlist->name, watchlist_name))
			return (watchlist);
		watchlist++;
	}
	UNMAP_FILE(*map, filemap);
	return (NULL);
}

void session_load_watchlists(struct session *session)
{
	struct watchlist  *watchlist = (struct watchlist *)zmalloc(sizeof(*watchlist));
	struct watchcond  *watchcond;
	struct watchstock *watchstock;
	struct stat sb;
	char path[256];
	int x, y, nr_watchlists, fd;

	watchlist_path(session, path);
	/* morphtab watchtable */
	strcpy(watchlist->name,      "morphtab");
	strcpy(watchlist->basetable, "morphtab");
	session->watchlists[0]  = watchlist;
	watchlist->origin       = NULL;
	watchlist->owner        = session;
	watchlist->config       = IS_MORPHTAB;
	session->morphtab       = watchlist;
	session->morphtab->wtab = NULL;
	session->nr_watchlists  = 1;

	/* main Watchlist is not saved onto the hard drive until a stock is added */
	if (stat(path, &sb) < 0) // if no watchlists are saved on thet hard drive
		return;

	nr_watchlists = (sb.st_size/sizeof(struct watchlist));
	fd = open(path, O_RDONLY);
	if (fd < 0 || fstat(fd, &sb) < 0 || !sb.st_size) {
		printf(BOLDRED "EMPTY WATCHLIST FILE!: session: %p %s" RESET "\n", session, path);
		unlink(path);
		return;
	}

	for (x=0; x<nr_watchlists; x++) {
		if (session->nr_watchlists >= MAX_WATCHLISTS)
			break;
		watchlist = (struct watchlist *)malloc(sizeof(*watchlist));
		read(fd, (char *)watchlist, sizeof(struct watchlist));
		watchlist->wtab   = NULL;
		watchlist->origin = watchlist;
		watchlist->owner  = session;
		session->watchlists[session->nr_watchlists++] = watchlist;
		if (watchlist->config == IS_PUBLIC) {
			printf(BOLDCYAN "LOADING PUBLIC  WATCHLIST: session: %p %s" RESET "\n", session, watchlist->name);
			Watchlists[nr_global_watchlists++] = watchlist;
		} else
			printf(BOLDCYAN "LOADING PRIVATE WATCHLIST: session: %p %s" RESET "\n", session, watchlist->name);
		for (y=0; y<watchlist->nr_stocks; y++) {
			watchstock        = &watchlist->stocks[y];
			watchstock->stock = search_stocks(watchstock->ticker);
		}
		for (y=0; y<watchlist->nr_conditions; y++) {
			watchcond        = &watchlist->conditions[y];
			watchcond->stock = search_stocks(watchcond->ticker);
		}
	}
	close(fd);
}

void list_watchlists(int http_fd)
{
	struct watchlist  *watchlist;
	struct watchstock *watchstock;
	struct list_head  *session_list;
	struct session    *session;
	char              *wmap, *username = NULL;
	char               path[256];
	char               buf[1024];
	int                fd, x, y, nr_watchlists, count = 0, namelen = 0;

	recv(http_fd, (void *)&namelen, 4, 0);
	if (namelen) {
		recv(http_fd, buf, namelen, 0);
		buf[namelen] = 0;
		username     = buf;
	}

	session_list = get_session_list();
	SESSION_LOCK();
	DLIST_FOR_EACH_ENTRY(session, session_list, list) {
		if (username) {
//			 if (strcmp(username, session->user->uname) && strncmp(username, (char *)&session->user->cookie, 7))
//				continue;
		}
		mutex_lock(&session->session_lock);
		printf("nr_watchlists: %d\n", session->nr_watchlists);
		for (x=0; x<session->nr_watchlists; x++) {
			watchlist = session->watchlists[x];
			printf("Watchlist Name:   " BOLDGREEN  "%s" RESET "\n", watchlist->name);
			printf("BaseTable:        " BOLDYELLOW "%s" RESET "\n", watchlist->basetable);
			if (watchlist->origin)
				printf("Origin:           %s\n", watchlist->origin->name);
			else
				printf("Origin:           Watchlist (%s)\n", watchlist->config & IS_PUBLIC ? "Public" : "Private");

			printf("Number of Stocks: " BOLDWHITE  "%d" RESET "\n", watchlist->nr_stocks);
			for (y=0; y<watchlist->nr_stocks; y++) {
				watchstock = &watchlist->stocks[y];
				if (count++ > 6) {
					printf("\n");
					count = 0;
				}
				printf(BOLDWHITE "%-8s " RESET, watchstock->ticker);
			}
			printf(BOLDMAGENTA "\n--------------------" RESET "\n");
		}
		mutex_unlock(&session->session_lock);
	}
	SESSION_UNLOCK();
}

void rpc_watchlist_delstock(struct rpc *rpc)
{
	char              *watchlist_name = rpc->argv[1];                // argv[1]: ASCII  string of Watchlist
	struct stock      *stock          = search_stocks(rpc->argv[2]); // argv[2]: ticker string
	struct watchlist  *watchlist;
	struct watchstock *watchstock;
	int                x;

	if (!stock || !(watchlist=search_watchlist(rpc->session, watchlist_name)))
		return;

	for (x=0; x<watchlist->nr_stocks; x++) {
		watchstock = &watchlist->stocks[x];
		if (watchstock->stock == stock) {
			if (x == (watchlist->nr_stocks-1)) {
				memset(&watchlist->stocks[x], 0, sizeof(*watchstock));
				watchlist->nr_stocks--;
				break;
			} else {
				memmove(watchstock, (char *)watchstock+sizeof(*watchstock), (watchlist->nr_stocks-x-1)*sizeof(*watchstock));
				watchlist->nr_stocks--;
				break;
			}
		}
	}
}

struct watchstock *get_watchstock(struct watchlist *watchlist, struct stock *stock)
{
	struct watchstock *watchstock = &watchlist->stocks[0];
	int x, nstocks = watchlist->nr_stocks;

	if (!stock)
		return NULL;
	for (x=0; x<nstocks; x++) {
		if (watchstock->stock == stock)
			return watchstock;
		watchstock++;
	}
	return (NULL);
}

void watchlist_remove_global(struct watchlist *watchlist)
{
	int x;

	mutex_lock(&watchlist_lock);
	for (x=0; x<nr_global_watchlists; x++) {
		if (Watchlists[x] == watchlist) {
			if (x == (nr_global_watchlists-1)) {
				Watchlists[x] = NULL;
				nr_global_watchlists--;
				break;
			} else {
				memmove(&Watchlists[x], &Watchlists[x+1], (nr_global_watchlists-x-1)*sizeof(struct watchlist *));
				nr_global_watchlists--;
				break;
			}
		}
	}
	mutex_unlock(&watchlist_lock);
}

void rpc_watchlist_remove(struct rpc *rpc)
{
	struct session   *session   = rpc->session;
	char             *name      = rpc->argv[1]; // argv[1]: watchlist name
	struct watchlist *watchlist = search_watchlist(session, name);
	struct filemap    filemap;
	char              path[256];
	char             *map;
	int               x, filesize, nr_watchlists, last_list = 0;

	if (!watchlist) {
		watchlist = search_watchlist(session, name);
		if (!watchlist)
			return;
	}
	watchlist_remove_global(watchlist);
	watchlist_path(session, path);

	map = MAP_FILE_RW(path, &filemap);
	if (!map)
		return;
//	mutex_lock(&session->watchlist_lock);
	watchlist     = (struct watchlist *)map;
	filesize      = filemap.filesize;
	nr_watchlists = (filesize/sizeof(struct watchlist)); // physical watchlists don't include the static morphtab watchlist
	for (x=0; x<nr_watchlists; x++) {
		if (!strcmp(watchlist->name, name)) {
			memset(watchlist, 0, sizeof(*watchlist));
			if (x+1 == nr_watchlists && nr_watchlists != 1) {
				printf("REMOVING LAST WATCHLIST: %d\n", x);
				fs_truncate(path, filesize-sizeof(*watchlist));
				last_list = 1;
				break;
			}
			memmove(watchlist, ((char *)watchlist+sizeof(*watchlist)), (nr_watchlists-x-1)*sizeof(*watchlist));
			printf("REMOVING MIDDLE WATCHLIST\n");
			filesize -= sizeof(*watchlist);
			UNMAP_FILE(map, &filemap);
			fs_truncate(path, filesize);
			break;
		}
		watchlist++;
	}
	// [morphtab][Highcaps][BioTech][Comm]   x=1, nr_watchlists=4
	nr_watchlists = session->nr_watchlists;
	printf("nr_memory_watch: %d\n", nr_watchlists);
	if (!last_list && nr_watchlists != 2)
		memmove(&session->watchlists[x+1], &session->watchlists[x+2], (nr_watchlists-x-2)*sizeof(void *));
	else if (nr_watchlists == 2)
		session->watchlists[1] = NULL;
	else if (last_list) {
		printf("last list setting NULL to: %s\n", session->watchlists[x+2]->name);
		session->watchlists[x+2] = NULL;}
	session->nr_watchlists -= 1;
//	mutex_unlock(&session->watchlist_lock);
}

void memwatch_addstock(struct watchlist *watchlist, struct stock *stock)
{
	struct watchstock *watchstock = &watchlist->stocks[watchlist->nr_stocks++];
	*(unsigned long *)(watchstock->ticker) = 0;
	*(unsigned long *)(watchstock->ticker) = *(unsigned long *)stock->sym;
	watchstock->stock = stock;
}

void rpc_watchlist_save(struct rpc *rpc)
{
	struct session    *session        = rpc->session;
	char              *watchlist_name = rpc->argv[1];       // argv[1]: watchlist name
	char              *basetable      = rpc->argv[2];       // argv[2]: the watchtable "containing" the watchlist
	int                pub            = (*rpc->argv[3])-48; // argv[3]: watchlist is public or private
	char              *watchlist_id   = rpc->argv[4];       // argv[4]: random watchlist ID
	struct watchlist  *new_watchlist, *watchtable;
	struct watchstock *watchstock;
	char               path[256];
	int                morphtab = 1;

	if (!watchlist_name || search_watchlist(session, watchlist_name) || strlen(watchlist_id) > 7)
		return;

	if (strcmp(basetable, "morphtab"))
		morphtab = 0;

	watchtable = search_watchtable(session, basetable);
	if (!watchtable)
		return;
	new_watchlist = (struct watchlist *)malloc(sizeof(*new_watchlist));
	memcpy(new_watchlist, watchtable, sizeof(*watchtable));
	strncpy(new_watchlist->name,      watchlist_name, WATCHLIST_NAME_MAX);
	strncpy(new_watchlist->basetable, basetable,      BASETABLE_NAME_MAX);
	new_watchlist->watchlist_id = *(unsigned long *)watchlist_id;
	watchtable->origin          = new_watchlist;
	new_watchlist->config       = pub; // public watchlist
//	mutex_lock(&session->watchlist_lock);
	session->watchlists[session->nr_watchlists++] = new_watchlist;
	if (morphtab) {
		memset(&watchtable->stocks[0], 0, sizeof(struct watchstock) * watchtable->nr_stocks);
		watchtable->nr_stocks = 0;
	}
//	mutex_unlock(&session->watchlist_lock);

	// Public Watchlists
	if (pub) {
//		spin_lock(&watchlist_lock);
		Watchlists[nr_global_watchlists++] = new_watchlist;
//		spin_unlock(&watchlist_lock);
	}

	/* Watchlist Store */
	watchlist_path(session, path);
	fs_appendfile(path, (char *)new_watchlist, sizeof(*new_watchlist));
}
