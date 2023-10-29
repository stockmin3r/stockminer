#include <conf.h>

mutex_t       session_lock;
mutex_t       cookie_lock;
int           NR_SESSIONS;
struct ihash *UID_HASHTABLE;
struct chash *COOKIE_HASHTABLE;
DLIST_HEAD   (session_list);

struct session *session_alloc(struct connection *connection)
{
	struct session *session;
	struct chash   *chash;
	char           *cookie_ptr;

	session                = (struct session *)zmalloc(sizeof(struct session));
	session->user          = (struct user    *)zmalloc(sizeof(struct user));
	session->user->uid     = -1;
	session->user->session = session;
	connection->session    = session;

	mutex_lock(&session_lock);
	NR_SESSIONS++;
	LIST_ADD(&session->list, &session_list);
	mutex_unlock(&session_lock);

	cookie_ptr     = &session->user->cookies[connection->websocket_id][0];
	random_cookie(cookie_ptr);
	chash          = (struct chash *)zmalloc(sizeof(*chash));
	chash->session = session;
	chash->cookie  = cookie_ptr;
	mutex_lock(&cookie_lock);
	HASH_ADD_STR(COOKIE_HASHTABLE, cookie, chash);
	mutex_unlock(&cookie_lock);

	// calls stock_session_alloc_hook() && workspace_session_alloc_hook() (if enabled)
	module_hook(session, MODULE_SESSION_ALLOC);
	printf(BOLDRED "SESSION_ALLOC: %s" RESET "\n", chash->cookie);
	return (session);
}

/* Called from www.c::www_websocket_request() - when the browser initiates a websocket connection */
struct session *session_get(struct connection *connection, char *request)
{
	struct session *session;
	char           *p, *cookie = strstr(request, "/c=");

	printf(BOLDMAGENTA "connection: %p" RESET "\n", connection);
	if (!cookie || (*(cookie+3)==' '))
		return session_alloc(connection);
	p = strchr(cookie, ' ');
	if (!p)
		return session_alloc(connection);
	*p = 0;
	cookie += 3;
	if (*cookie == ' ')
		cookie++;
	if (*cookie == '\0' || *(cookie+1) == '\0' || *(cookie+2) == '\0')
		return NULL;
	printf(BOLDCYAN "session_get: cookie: %s" RESET "\n", cookie);
	session = session_by_cookie(cookie);
	*p = ' ';
	if (!session) {
		session = session_alloc(connection);
		if (!session)
			return NULL;
	} else {
		connection->has_cookie = 1;
	}
	if (connection->session)
		printf(BOLDGREEN "session_get(): %s" RESET "\n", connection->session->user->cookies[connection->websocket_id]);
	return (session);
}

struct session *session_by_cookie(char *cookie)
{
	struct chash *chash = NULL;

	mutex_lock(&cookie_lock);
	HASH_FIND_STR(COOKIE_HASHTABLE, cookie, chash);
	mutex_unlock(&cookie_lock);
	printf("session_by_cookie: %s chash: %p\n", cookie, chash);
	if (!chash)
		return NULL;
	return chash->session;
}


struct session *session_by_username(char *username)
{
	struct user *user = search_user(username);

	if (!user)
		return NULL;
	return user->session;
}


struct session *session_by_uid(unsigned int uid)
{
	struct ihash *ihash = NULL;

	mutex_lock(&session_lock);
	HASH_FIND_INT(UID_HASHTABLE, &uid, ihash);
	mutex_unlock(&session_lock);
	if (!ihash)
		return NULL;
	return ihash->session;
}

void session_destroy(struct session *session)
{
	mutex_lock(&session_lock);
	LIST_DEL(&session->list);
	NR_SESSIONS -= 1;
	mutex_unlock(&session_lock);
	free(session);
	memset(session, 0, sizeof(struct session));
}

struct list_head *get_session_list()
{
	return &session_list;
}

void SESSION_LOCK()
{
	mutex_lock(&session_lock);
}

void SESSION_UNLOCK()
{
	mutex_unlock(&session_lock);
}

void session_set_config(struct connection *connection)
{
	struct session *session     = connection->session;
	char           *packet      = connection->packet,     *qcache;
	packet_size_t   packet_size = connection->packet_size, qcache_size;

	packet     += packet_size;
	packet_size = 0;

	/* QCACHE JSON */
	if (session->qcache && (qcache=yyjson_mut_write(session->qcache, 0, NULL))) {
		qcache_size = strlen(qcache);
		strcpy(packet+packet_size, "qcache ");
		packet_size += 7;
		memcpy(packet+packet_size, qcache, qcache_size);
		printf(BOLDYELLOW "%s" RESET "\n", qcache);
		packet_size += qcache_size;
		packet[packet_size++] = '@';
	}

	if (session->user->logged_in)
		packet_size += snprintf(packet+packet_size, 128, "user %s %d@", session->user->uname, session->user->uid);

	/* Saved Website Modifications */
	packet_size += websocket_presets            (session, packet+packet_size);
	packet_size += websocket_watchtable_presets (session, packet+packet_size);
	packet_size += conf_public_watchlists       (packet+packet_size);
	packet_size += conf_public_alerts           (packet+packet_size);
	packet_size += conf_private_watchlists      (session, packet+packet_size);
	packet_size += conf_private_alerts          (session, packet+packet_size);
	packet_size += conf_styles                  (session, packet+packet_size);
	packet_size += qpage_packet                 (session, packet+packet_size);

	/* Cookie */
	if (!connection->has_cookie) {
		packet_size += snprintf(packet+packet_size, 24, "cookie %s@", session->user->cookies[connection->websocket_id]);
		printf(BOLDMAGENTA "packet: %s session: %p websocket_id: %d" RESET "\n", packet, session, connection->websocket_id);
	}
	/* Update the packet size */
	connection->packet_size += packet_size;
}

void session_add(struct session *session)
{
	LIST_ADD(&session->list, &session_list);
	module_hook(session, MODULE_SESSION_INIT);
	NR_SESSIONS++;
}
