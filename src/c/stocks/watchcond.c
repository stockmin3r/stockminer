#include <conf.h>
#include <stocks/stocks.h>

static __inline__ struct watchcond *map_watchcond(struct watchlist *watchlist, unsigned long id, int nr_conditions)
{
	int x;

	for (x=0; x<nr_conditions; x++) {
		struct watchcond *watchcond = &watchlist->conditions[x];
		if (watchcond->id == id) {
			printf("found ID: %.8s pos: %d\n", (char *)&id, x);
			return (watchcond);
		}
	}
	return (NULL);
}

struct watchcond *get_watchcond(struct watchlist *watchlist, struct stock *stock)
{
	int x;

	for (x=0; x<watchlist->nr_conditions; x++) {
		struct watchcond *watchcond = &watchlist->conditions[x];
		if (watchcond->stock == stock)
			return (watchcond);
	}
	return NULL;
}

void watchcond_del(struct watchlist *watchlist, int idx)
{
	struct watchcond *watchcond = &watchlist->conditions[idx];

	if (watchlist->nr_conditions == 1 || watchlist->nr_conditions == (idx-1))
		memset(watchcond, 0, sizeof(*watchcond));
	else {
		memmove(watchcond, (char *)watchcond+sizeof(*watchcond), (watchlist->nr_conditions-idx-1)*sizeof(*watchcond));
		memset((char *)watchcond+sizeof(*watchcond), 0, sizeof(*watchcond));
	}
	watchlist->nr_conditions -= 1;
}

void rpc_remove_alert(struct rpc *rpc)
{
	struct session   *session        = rpc->session;
	char             *watchlist_name = rpc->argv[1];
	char             *alert_id       = rpc->argv[2];
	struct watchlist *watchlist;
	struct filemap    filemap;
	char              path[256];
	char             *map;
	int               x, y, nr_watchlists, nr_alerts, done = 0;

	watchlist = search_watchlist(session, watchlist_name);
	if (!watchlist)
		return;
	watchlist_path(session, path);

//	mutex_lock(&session->watchlist_lock);
	map = MAP_FILE_RW(path, &filemap);
	if (!map)
		return;
	watchlist     = (struct watchlist *)map;
	nr_watchlists = (filemap.filesize/sizeof(struct watchlist));
	/* Disk Watchlists */
	for (x=0; x<nr_watchlists; x++) {
		if (strcmp(watchlist->name, watchlist_name)) {
			watchlist++;
			continue;
		}
		nr_alerts = watchlist->nr_conditions;
		for (y=0; y<nr_alerts; y++) {
			struct watchcond *condition = &watchlist->conditions[y];
			if (*(uint64_t *)alert_id == *(uint64_t *)&condition->id) {
				watchcond_del(watchlist, y);
				printf("removing from disk watchlist: %s alerts: %d\n", watchlist->name, nr_alerts);
				done = 1;
				break;
			}
		}
		if (done)
			break;
	}

	done = 0;
	/* Session Watchlists */
	nr_watchlists = session->nr_watchlists;
	for (x=0; x<nr_watchlists; x++) {
		watchlist = session->watchlists[x];
		if (strcmp(watchlist->name, watchlist_name))
			continue;
		nr_alerts = watchlist->nr_conditions;
		for (y=0; y<nr_alerts; y++) {
			struct watchcond *condition = &watchlist->conditions[y];
			if (*(uint64_t *)alert_id == *(uint64_t *)&condition->id) {
				watchcond_del(watchlist, y);
				printf("removing from memory watchlist: %s alerts: %d\n", watchlist->name, nr_alerts);
				done = 1;
				break;
			}
		}
		if (done)
			break;
	}
//	mutex_unlock(&session->watchlist_lock);
	UNMAP_FILE(path, &filemap);
}

struct watchcond *session_watchcond(struct watchlist *watchlist, uint64_t id)
{
	struct watchcond *watchcond;
	int x, nr_alerts;

	nr_alerts = watchlist->nr_conditions;
	for (x=0; x<nr_alerts; x++) {
		watchcond = &watchlist->conditions[x];
		printf("id: %.8s vs: %.8s\n", (char *)&watchcond->id, (char *)&id);
		if (watchcond->id == id)
			return (watchcond);
	}
	return (NULL);
}

void watchcond_update(struct session *session, struct watchlist *watchlist, struct watchcond *watchcond)
{
	struct watchlist *wp;
	struct watchcond *condition;
	struct filemap    filemap;
	char             *map;
	char              path[256];

	watchlist_path(session, path);
	wp = (struct watchlist *)map_watchlist(session, path, watchlist->name, &map, &filemap);
	if (!wp)
		return;

	condition = map_watchcond(wp, watchcond->id, watchlist->nr_conditions);
	if (!condition) {
		printf(BOLDRED "watchcond update: failed to find condition: %p" RESET "\n", watchcond);
		goto out;
	}
	printf(BOLDGREEN "watchcond_update: DETECTED: %p (watchlist: %p)" RESET "\n", watchcond, watchlist);
	memcpy(condition, watchcond, sizeof(*watchcond));
out:
	UNMAP_FILE(map, &filemap);
}

void watchcond_create(struct session *session, struct watchlist *watchlist, struct watchcond *watchcond)
{
	struct watchlist *wp;
	struct watchcond *condition;
	struct filemap    filemap;
	char             *map;
	char              path[256];

	watchlist_path(session, path);
	wp = (struct watchlist *)map_watchlist(session, path,watchlist->name, &map, &filemap);
	if (!wp)
		return;
	wp->nr_conditions = watchlist->nr_conditions;
	printf("CREATING WATCHCOND: %p nr_conditions: %d watchlist: %p\n", watchcond, watchlist->nr_conditions, watchlist);
	memcpy(&wp->conditions[wp->nr_conditions-1], watchcond, sizeof(*watchcond));
	UNMAP_FILE(map, &filemap);
}
