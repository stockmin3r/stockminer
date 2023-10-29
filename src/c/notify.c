#include <conf.h>

char *NOTIFICATION  = "notify <li!class=dropdown-header>Notifications!(%d)</li>";

char *NOTIFY_ENTRY  = "<li!class=notification-item>"
					  "<a!href='javascript:;'>"
					     "<div!class=media><i!class=ichart></i></div>"
					     "<div!class=message><p!class=desc>%s!%s!<span!id=nls>%s</span></p></div>"
					     "<span!class=icircle>&#9702;</span>"
					  "</a>"
					  "</li>@";

void watchlist_notifications(struct session *session, struct connection *connection)
{
	struct watchlist *watchlist;
	struct watchcond *watchcond;
	char notify_buf[48 KB];
	char cdate[64];
	struct tm tm;
	time_t utc_time;
	int x, y, notify_len = 0, nr_notifications = 0, condition_met = 0, nbytes;

	utc_time = time(NULL);
	strftime(cdate, 14, "%H:%M:%S (EST)", localtime_r(&utc_time, &tm));
//	mutex_lock(&session->watchlist_lock);
	for (x=0; x<session->nr_watchlists; x++) {
		watchlist = session->watchlists[x];
		if (!watchlist)
			continue;
		if (watchlist->nr_conditions == 0)
			continue;
		for (y=0; y<watchlist->nr_conditions; y++) {
			watchcond = &watchlist->conditions[y];
			if (watchcond->condition == CONDITION_REACHED)
				continue;
			switch (watchcond->condition) {
				case PRICE_GREATER_THAN:
					if (watchcond->stock->current_price >= watchcond->price)
						condition_met = 1;
					break;
				case PRICE_LESSER_THAN:
					if (watchcond->stock->current_price <= watchcond->price)
						condition_met = 1;
					break;
			}
			if (!condition_met)
				continue;
			if (!get_watchstock(watchlist, watchcond->stock))
				watchlist_add(session, watchlist, watchcond->stock);
			printf("watchlist alert: %s %.2f vs: %.2f %s\n", watchlist->name, watchcond->stock->current_price, watchcond->price, cdate);
			nbytes               = sprintf(notify_buf+notify_len, "notify %s %.2f %s@", watchcond->ticker, watchcond->price,  cdate);
			notify_len          += nbytes;
			nr_notifications    += 1;
			watchcond->condition = CONDITION_REACHED;
			condition_met        = 0;
		}
	}
//	mutex_unlock(&session->watchlist_lock);
	if (nr_notifications)
		websocket_send(connection, notify_buf, notify_len);
}
