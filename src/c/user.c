#include <conf.h>
#include <extern.h>

mutex_t         user_lock;
struct uhash   *USER_HASHTABLE;
int             NR_USERS;

/*
 * Called by db.c::db_thread()::db_engine_file() when a user is created
 * Currently this is a FILE type "database" (db/users.db)
 */
void db_user_register_cb(struct connection *connection, int result)
{
	struct uhash *uhash;
	struct user  *user;
	char          path[256];

	if (result < 0) {
		websocket_send(connection, ILLEGAL_USERNAME, 12);
		return;
	}

	user = connection->session->user;
	snprintf(path, sizeof(path)-1,  "db/uid/%d", user->uid);
	fs_mkdir(path, 0644);

	printf("creating user: %s with uid: %d dir: %s\n", user->uname, user->uid, path);
	/*
	 * Add user to USER_HASHTABLE
	 */
	uhash          = (struct uhash *)zmalloc(sizeof(*uhash));
	uhash->session = connection->session;
	strcpy(uhash->username, connection->session->user->uname);
	HASH_ADD_STR(USER_HASHTABLE, username, uhash);
	websocket_send(connection, "regok", 5);
}

/*
 * Called by db.c::db_engine_file() when a user struct needs to be updated
 */
void db_user_update_cb(struct user *user)
{
	struct user    *uptr;
	struct filemap  filemap;
	char           *map;
	int             nr_users;

	printf(BOLDRED "db_user_update_cb: username: %s uid: %d" RESET "\n", user->uname, user->uid);
	mutex_lock(&user_lock);
	map       = MAP_FILE_RW((char *)DB_USERS_PATH, &filemap);
	nr_users  = filemap.filesize/sizeof(struct user);
	uptr      = (struct user *)map;
	printf(BOLDRED "NR USERS: %d" RESET "\n", nr_users);
	for (int x=0; x<nr_users; x++) {
		printf(BOLDGREEN "uptr->uid: %d user->uid: %d" RESET "\n", uptr->uid, user->uid);
		if (uptr->uid == user->uid) {
			memcpy(uptr, user, sizeof(*user));
			break;
		}
		uptr++;
	}
	mutex_unlock(&user_lock);
	UNMAP_FILE(map, &filemap);
}

void rpc_user_register(struct rpc *rpc)
{
	struct session    *session    = rpc->session;
	struct connection *connection = rpc->connection;
	char              *usr        = rpc->argv[1];
	char              *pwd        = rpc->argv[2];
	int                pwdlen     = pwd ? strlen(pwd) : 0;
	int                usrlen     = usr ? strlen(usr) : 0;
	struct user       *user;
	char               pwhash[crypto_pwhash_STRBYTES], *p = usr;
	const char        *error;

	if (unlikely(!usr || !pwd || !pwdlen || !usrlen || usrlen >= MAX_USERNAME_SIZE))
		goto out_error;

	for (int x=0; x<usrlen; x++) {
		if (!isalnum(*p++))
			goto out_error;
	}

	user = session->user;
	strcpy(user->uname, usr);
	user->join_date = time(NULL);
	if (crypto_pwhash_str(pwhash, pwd, pwdlen, crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_MIN) < 0) {
		error = SYSTEM_ERROR;
		goto out_error;
	}
	memset(pwd, 0, pwdlen);
	user->logged_in = 1;
	memcpy(user->pwhash, pwhash, crypto_pwhash_STRBYTES);

	// enqueue db task for the db deathloop to add the user
	db_user_add(user, connection);
	return;
out_error:
	if (connection)
		websocket_send(connection, ILLEGAL_USERNAME, 12);

}

void rpc_register(struct rpc *rpc)
{
	struct session    *session    = rpc->session;
	struct connection *connection = rpc->connection;
	char              *username   = rpc->argv[1];
	char              *pubkey64   = rpc->argv[2];
	char              *p          = username;
	int                usrlen     = strlen(username);
	int                keylen     = strlen(pubkey64);
	char               pubkey[hydro_sign_PUBLICKEYBYTES];
	struct user       *user;

	if (unlikely(!username || !usrlen || usrlen >= MAX_USERNAME_SIZE || keylen > hydro_sign_PUBLICKEYBYTES))
		goto out_error;

	for (int x=0; x<usrlen; x++)
		if (!isalnum(*p++))
			goto out_error;

	user = session->user;
	strcpy(user->uname, username);
	user->join_date = time(NULL);
	base64_decode(pubkey, strlen(pubkey), pubkey);
	memcpy(user->pubkey, pubkey, hydro_sign_PUBLICKEYBYTES);
	strcpy(user->uname, username);
	

	// enqueue db task for the db deathloop to add the user
	db_user_add(user, connection);
	return;
out_error:
	if (connection)
		websocket_send(connection, ILLEGAL_USERNAME, 12);
}

/* To be replaced by a modular auth system, potentially:
 * 1) client side hash calculation via libsodium-sumo
 * 2) pubkey libhydrogen auth
 * 3) terrarium OPRF wasm
 * 4) ...
 */
void rpc_user_login(struct rpc *rpc)
{
	struct connection *connection   = rpc->connection;
	struct session    *prev_session = connection->session, *session;
	char              *username     = rpc->argv[1], *signature64;
	char               challenge[1024];
	char               signature[256];
	int                username_size;

	printf("rpc_user_login: %s\n", rpc->argv[1]);

	username    = rpc->argv[1];
	signature64 = strchr(username, '|');
	if (!signature64)
		goto out_error;
	*signature64++ = 0;

	session = session_by_username(username);
	if (!session)
		goto out_error;

	base64_decode(signature64, strlen(signature64), signature);
	username_size = strlen(session->user->uname);
	memcpy(challenge, username, username_size);
	challenge[username_size] = '|';
	memcpy(challenge+username_size+1, connection->nonce, sizeof(connection->nonce));
	printf("challenge: %s\n", challenge);
	for (int x = 0; x<hydro_sign_BYTES; x++)
		printf(BOLDMAGENTA "%x ", (unsigned char)signature[x]);
	printf(RESET "\n");
	for (int x = 0; x<hydro_sign_PUBLICKEYBYTES; x++)
		printf(BOLDRED "%x ", (unsigned char)session->user->pubkey[x]);
	printf(RESET "\n");

	if (hydro_sign_verify(signature, challenge, username_size+1+sizeof(connection->nonce), "context0", session->user->pubkey) == -1)
		goto out_error;

	session->user->logged_in = 1;

	// enqueue task for the db loop to update the user, at some point in the future
	db_user_update(session->user);

	// send the backpage which requires a login
	www_send_backpage(session, connection, 1);
	printf(BOLDGREEN "user_login success: %s" RESET "\n", session->user->uname);
	return;
out_error:
	printf("fail\n");
	websocket_send(connection, "err 0 fail", 10);
}

unsigned int uid_by_username(char *username)
{
	struct user *user = search_user(username);
	if (!user)
		return -1;
	return (user->uid);
}

/* user_lock will be locked by caller in db.c */
bool user_exists(char *username)
{
	struct uhash *uhash = NULL;
	int rc = true;

	HASH_FIND_STR(USER_HASHTABLE, username, uhash);
	if (!uhash)
		rc = false;
	return (rc);
}

struct user *search_user(char *username)
{
	struct uhash *uhash = NULL;
	struct user  *user  = NULL;

	if (!username)
		return (NULL);

	mutex_lock(&user_lock);
	HASH_FIND_STR(USER_HASHTABLE, username, uhash);
	if (uhash)
		user = uhash->session->user;
	mutex_unlock(&user_lock);
	return (user);
}

void apc_user_add(struct connection *connection, char **argv)
{
	struct rpc rpc = {0};

	rpc.argv = argv;
	rpc_user_register(&rpc);
}

void init_users()
{
	struct filemap  filemap;
	struct session *session;
	struct user    *user, *up;
	struct uhash   *uhash;
	struct ihash   *ihash;
	struct chash   *chash;
	char            cookie[COOKIE_SIZE+1];
	char           *map, *username;
	int             username_len;

	if (!(map=MAP_FILE_RO((char *)DB_USERS_PATH, &filemap)))
		return;

	NR_USERS  = filemap.filesize/sizeof(struct user);
	up        = (struct user *)map;
	for (int x=0; x<NR_USERS; x++) {
		user = (struct user *)malloc(sizeof(struct user));
		memcpy(user, up, sizeof(struct user));
		username = user->uname;
		up++;
		username_len = strlen(username);
		if (username_len >= MAX_USERNAME_SIZE)
			continue;

		// init session of user
		user->session = session = (struct session *)zmalloc(sizeof(struct session));
		session->user = user;

		printf(BOLDCYAN "creating session for username: %s user: %p session: %p" RESET "\n", username, user, session);

		// load user's objects
//		load_objects(session, user->uid);

		// add user's session to session.c::session_list
		session_add(session);

		// Add user to USER_HASHTABLE
		uhash          = (struct uhash *)zmalloc(sizeof(*uhash));
		uhash->session = session;
		strcpy(uhash->username, username);
		HASH_ADD_STR(USER_HASHTABLE, username, uhash);

		// Add user to COOKIE_HASHTABLE (STR)
		chash          = (struct chash *)zmalloc(sizeof(*chash));
		chash->session = session;
		for (int y=0; y<MAX_WEBSOCKETS; y++) {
			if (!user->cookies[y][0])
				continue;
			strncpy(cookie, &user->cookies[y][0], COOKIE_SIZE);
			chash->cookie = &user->cookies[y][0];
			HASH_ADD_STR(COOKIE_HASHTABLE, cookie, chash);
			if (*session->filecookie == '\0') {
				strncpy(session->filecookie, &user->cookies[y][0], 15);
				cstring_strchr_replace(session->filecookie, '/', '-');
			}
			printf("adding cookie: [%s] to chash: [%p] session: [%p]\n", cookie, chash, session);
		}
		// Add user to UID_HASHTABLE (INT)
		ihash          = (struct ihash *)zmalloc(sizeof(*chash));
		ihash->session = session;
		ihash->uid     = user->uid;
		HASH_ADD_INT(UID_HASHTABLE, uid, ihash);
	}
	UNMAP_FILE(map, &filemap);
}
