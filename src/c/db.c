#include <conf.h>
#include <extern.h>
#include <lmdb.h>

/*
 * Eventually there will be multiple different databases with mdbx to be added next
 * Currently, just wrappers for LMDB
 */

#define DATABASE_NAME(database)                      \
({                                                   \
	switch (database) {                              \
		case DATABASE_USERS:                         \
				db_name = "users.db";                \
				DATABASE = LMDB_DATABASE_USERS;      \
				break;                               \
		case DATABASE_QUADVERSE:                     \
				db_name = "quadverse.db";            \
				DATABASE = LMDB_DATABASE_QUADVERSE;  \
				break;                               \
		case DATABASE_WATCHLISTS:                    \
				DATABASE = LMDB_DATABASE_WATCHLISTS; \
				db_name = "watchlists.db";           \
				break;                               \
	}                                                \
	(const char *)db_name;                           \
})

struct db_task {
		uint32_t               database;   // USERS | QUADVERSE | WATCHLISTS
        uint32_t               engine;     // FILE  | LMDB
		uint32_t               op;         // Database Operation (DB_APPEND|DB_OVERWRITE,...)
        char                  *key;        // key (NULL for FILE storage "engine")
        void                  *value;      // value
        uint64_t               vsize;      // size of value
		struct connection     *connection; // connection making the task (can be NULL)
		db_callback_t          callback;   // (if set) called when db op is complete
};

struct db_queue {
        struct db_task        *task;
        TAILQ_ENTRY(db_queue)  tasks;
};

typedef struct db_queue        db_queue_t;
typedef struct db_task         db_task_t;

TAILQ_HEAD(, db_queue)         db_queue_head;

/* DB Names */
enum {
	DATABASE_USERS,          // struct user array
	DATABASE_QUADVERSE,      // QCACHE JSON
	DATABASE_WATCHLISTS,     // struct watchlist array (to become JSON)
};

/* Database Types */
enum {
	DATABASE_FILE,          // Store data into files (struct arrays)
	DATABASE_LMDB           // Store data into LMDB  (JSON)
};

/* Database Operations */
enum {
	DB_APPEND,
	DB_OVERWRITE
};

static MDB_env *LMDB_DATABASE_USERS;        // Users.db
static MDB_env *LMDB_DATABASE_QUADVERSE;    // Quadverse.db
static MDB_env *LMDB_DATABASE_WATCHLISTS;   // Watchlists.db
static int      db_backoff = 2000;          // db thread loop backoff for sleep (incomplete)
static mutex_t  db_mutex;                   // db_task queue lock

static MDB_txn *lmdb_transaction_start    (MDB_env *env, int flags);
static bool     lmdb_transaction_commit   (MDB_txn *transaction);
static void     lmdb_transaction_rollback (MDB_txn *transaction);
static void    *lmdb_get                  (int database, char *key, size_t *value_size_out);
static void     lmdb_set                  (int database, char *key, size_t key_size, void *value, size_t value_size);

void init_db(struct server *server)
{
	/* Users.db */
	mdb_env_create(&LMDB_DATABASE_USERS);
	mdb_env_open  (LMDB_DATABASE_USERS,      "./db/users",      MDB_FIXEDMAP|MDB_NOSYNC, 0664);

	/* QuadVerse.db */
	mdb_env_create(&LMDB_DATABASE_QUADVERSE);
	mdb_env_open  (LMDB_DATABASE_QUADVERSE,  "./db/quadverse",  MDB_FIXEDMAP|MDB_NOSYNC, 0664);

	/* Watchlists.db */
	mdb_env_create(&LMDB_DATABASE_WATCHLISTS);
	mdb_env_open  (LMDB_DATABASE_WATCHLISTS, "./db/watchlists", MDB_FIXEDMAP|MDB_NOSYNC, 0664);

	TAILQ_INIT(&db_queue_head);
}

void db_enqueue(int database, int engine, int op, char *key, void *value, uint64_t vsize, struct connection *connection, db_callback_t callback)
{
	db_queue_t *db_queue_entry = malloc(sizeof(db_queue_t));
	db_task_t  *db_task        = malloc(sizeof(db_task_t));

	if (!db_task || !db_queue_entry)
		return;

	db_task->engine      = engine;
	db_task->op          = op;
	db_task->key         = key;
	db_task->value       = value;
	db_task->vsize       = vsize;
	db_task->database    = database;
	db_task->callback    = callback;
	db_task->connection  = connection;
	db_queue_entry->task = db_task;
	mutex_lock(&db_mutex);
	TAILQ_INSERT_TAIL(&db_queue_head, db_queue_entry, tasks);
	mutex_unlock(&db_mutex);
	printf(BOLDYELLOW "db_enqueue: key: %s" RESET "\n", key);
}

/*
 * Used internally by the db_thread() to dequeue a db task
 */
static db_task_t *
db_dequeue(void)
{
	db_queue_t *db_queue_entry;
	db_task_t  *db_task = NULL;

	mutex_lock(&db_mutex);
	db_queue_entry = TAILQ_FIRST(&db_queue_head);
	if (!db_queue_entry)
		goto out;
	TAILQ_REMOVE(&db_queue_head, db_queue_entry, tasks);
	db_task = db_queue_entry->task;
	free(db_queue_entry);
out:
	mutex_unlock(&db_mutex);
	return (db_task);
}

void db_engine_file(db_task_t *db_task)
{
	switch (db_task->database) {
		case DATABASE_USERS:
			struct user *user = (struct user *)db_task->value;
			user->uid         = ATOMIC_INC(&NR_USERS);
			if (db_task->op == DB_APPEND)
				fs_appendfile(USERS_PATH, (char *)db_task->value, sizeof(struct user));
			else if (db_task->op == DB_OVERWRITE)
				db_user_update_cb(user);
			printf("db_engine_file: NR_USERS: %d user->uid: %d logged_in: %d" RESET "\n", NR_USERS, user->uid, user->logged_in);
			if (db_task->callback)
				db_task->callback(db_task->connection, 1);
			break;
		case DATABASE_QUADVERSE:
			break;
	}
}

void db_engine_lmdb(db_task_t *db_task)
{
	switch (db_task->database) {
		case DATABASE_USERS: {
			struct user *user = (struct user *)db_task->value; uint64_t rc;

			ATOMIC_INC(&NR_USERS);
			lmdb_set(DATABASE_USERS, user->uname, strlen(user->uname), user, sizeof(user));
			user = (struct user *)lmdb_get(DATABASE_USERS, user->uname, &rc);
			printf("rc: %d user: %p\n", rc, user);
		};
	}
}

void *db_thread(void *args)
{
	db_task_t *db_task;

	/* Database Thread Loop */
	for (;;) {
		db_task = db_dequeue();
		if (!db_task) {
			usleep(db_backoff);
			continue;
		}
		switch (db_task->engine) {
			case DATABASE_FILE:
				db_engine_file(db_task);
				break;
			case DATABASE_LMDB:
				db_engine_lmdb(db_task);
				break;
		}
	}
}
#define NR_TASKS 3
void *db_submit_tasks(void *args)
{
	struct db_task *db;
	struct user    *user;
	char           *users[NR_TASKS] = { "user", "user2", "user3" };
	return NULL;
	/* Database Thread Loop */
	for (int x = 0; x<NR_TASKS; x++) {
		sleep(1);
		user = zmalloc(sizeof(*user));
		strcpy(user->uname, users[x]);
		printf("enqueing: %s\n", users[x]);
		db_enqueue(DATABASE_USERS, DATABASE_FILE, DB_APPEND, users[x], (void *)user, sizeof(*user), NULL, NULL);
	}
}

void db_user_add(struct user *user, struct connection *connection)
{
	printf("adding user: %s\n", user->uname);
	db_enqueue(DATABASE_USERS, DATABASE_FILE, DB_APPEND, NULL, user, sizeof(user), connection, &db_user_register_cb);
}

void db_user_update(struct user *user)
{
	printf("updating user: %s\n", user->uname);
	struct db_task *task = zmalloc(sizeof(*task));

	db_enqueue(DATABASE_USERS, DATABASE_FILE, DB_OVERWRITE, NULL, user, sizeof(user), NULL, NULL);
}

static void
lmdb_set(int database, char *key, uint64_t key_size, void *value, uint64_t value_size)
{
	MDB_txn    *txn;
	MDB_dbi     dbi;      // typedef unsigned int MDB_dbi;
	MDB_val     db_key;
	MDB_val     db_value;
	MDB_env    *DATABASE;
	const char *db_name = DATABASE_NAME(database);

	db_key.mv_data   = key;
	db_key.mv_size   = key_size;
	db_value.mv_data = value;
	db_value.mv_size = value_size;

	printf(BOLDRED "LMDB_SET: db_name: %s key: %s" RESET "\n", db_name, key);

	mdb_txn_begin (DATABASE, NULL, 0, &txn);
	int rc = mdb_dbi_open  (txn, db_name, MDB_CREATE, &dbi);
	rc     = mdb_put       (txn, dbi, &db_key, &db_value, 0);
printf("rc2: %d\n", rc);	
	mdb_txn_commit(txn);
	mdb_dbi_close (DATABASE, dbi);

}

static void *
lmdb_get(int database, char *key, size_t *value_size_out)
{
	MDB_txn    *txn;
	MDB_val     db_key;
	MDB_val     db_value;
	MDB_dbi     dbi;
	MDB_env    *DATABASE;
	const char *db_name  = DATABASE_NAME(database);

	mdb_txn_begin(DATABASE, NULL, MDB_RDONLY, &txn);
	if (mdb_dbi_open(txn, db_name, 0, &dbi)) {
		printf("failed to open: %s\n", db_name);
		goto rollback;
	}

	db_key.mv_size = strlen(key);
	db_key.mv_data = key;
	if (!mdb_get(txn, dbi, &db_key, &db_value))
		goto rollback;
	mdb_dbi_close(DATABASE, dbi);
	if (value_size_out)
		*value_size_out = db_value.mv_size;
	return (void *)db_value.mv_data;
rollback:
	lmdb_transaction_rollback(txn);
out:
	return NULL;
}

static MDB_txn *
lmdb_transaction_start(MDB_env *env, int flags)
{
	MDB_txn *txn;

	if (mdb_txn_begin(env, NULL, flags, &txn))
		return (NULL);
	return txn;
}

static bool
lmdb_transaction_commit(MDB_txn *transaction)
{
	if (mdb_txn_commit(transaction) != 0)
		return true;
	return false;
}

static void
lmdb_transaction_rollback(MDB_txn *transaction)
{
	mdb_txn_abort(transaction);
}
