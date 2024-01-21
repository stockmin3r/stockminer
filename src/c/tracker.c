#include <conf.h>
#include <lib.h>
#include <extern.h>

#define TRACKER_ACTION_ADD 1
#define TRACKER_ACTION_DEL 2

#define TRACKER_TYPE_GIT   1

struct tracker {
	struct session **sessions;
	char            *url;
	char            *object;        /* The name of the target object that is being tracked (eg linux.git) */
	char            *directory;
	char            *output;        /* The latest "value" of the object residing at the url */
	uint64_t         nr_sessions;   /* Number of sessions attached to this tracker */
	int              type;
};

struct tracker_queue {
        struct tracker             *tracker;
        TAILQ_ENTRY(tracker_queue)  trackers;
};

typedef struct tracker_queue        tracker_queue_t;
typedef struct tracker              tracker_t;

TAILQ_HEAD(, tracker_queue)         tracker_queue_head;
mutex_t                             tracker_lock;

static void tracker_add(struct session *session, char *url);

void rpc_tracker(struct rpc *rpc)
{
	struct session  *session;
	tracker_t       *tracker;
	char            *action = rpc->argv[1];
	char            *target = rpc->argv[2];
	char            *url    = rpc->argv[3];
	char            *p;
	int              target_action, target_type;

	if (!session->user->logged_in)
		return;

	if (!strcmp(action, "add"))
		target_action = TRACKER_ACTION_ADD;
	else if (strcmp(action, "del"))
		target_action = TRACKER_ACTION_DEL;
	else
		return;

	tracker = zmalloc(sizeof(*tracker));
	if (!strcmp(target, "git")) {
		target_type = TRACKER_TYPE_GIT;
		p = strrchr(url, '/');
		if (!p || *(p+1) == '\0')
			return;
		tracker->object = p+1;
		p = strrchr(url, '.');
		if (!p)
			return;
		*p = 0;
		tracker->object = strdup(tracker->object);
		*p = '.';
	} else
		return;

	printf(BOLDWHITE "tracker: %s %s" RESET "\n", tracker->object, tracker->url);

	if (!session->trackers) {
		session->trackers = (tracker_t **)zmalloc(4 * sizeof(void *));
		if (!session->trackers)
			return;
		session->max_trackers = 4;
	} else if (session->nr_trackers >= session->max_trackers) {
		session->max_trackers *= 2;
		session->trackers      = (tracker_t **)realloc(session->trackers, session->max_trackers * sizeof(void *));
	}

	session->trackers[session->nr_trackers++] = tracker;
}

static void tracker_mod(struct session *session, int action, char *args)
{
	switch (action) {
		case TRACKER_ACTION_ADD:
			tracker_add(session, args);
			break;
	}
}

static void tracker_add(struct session *session, char *url)
{
	tracker_queue_t *tracker_queue_entry = (tracker_queue_t *)zmalloc(sizeof(*tracker_queue_entry));
	tracker_t       *tracker             = (tracker_t       *)zmalloc(sizeof(*tracker));

	tracker->url     = strdup(url);
	tracker->sessions[tracker->nr_sessions] = session;
	tracker_queue_entry->tracker = tracker;

	mutex_lock(&tracker_lock);
	TAILQ_INSERT_TAIL(&tracker_queue_head, tracker_queue_entry, trackers);
	mutex_unlock(&tracker_lock);
}

void tracker_git_update(struct tracker *tracker)
{
	char  cmd[2048];
	char  buf[512];
	char *p;
	FILE *fp;
	int   nbytes;

	snprintf(cmd, sizeof(cmd)-1, "cd src/git/%s && git ls-remote %s --verify HEAD", tracker->object, tracker->url);
	fp = popen(cmd, "r");
	nbytes = fread(buf, 255, 1, fp);
	pclose(fp);
	if (nbytes <= 0)
		return;
	buf[nbytes] = 0;
	p = strchr(buf, ' ');
	if (!p)
		return;
	*p = 0;
	tracker->output = strdup(buf);
	printf("tracker git update: %s\n", buf);
}

void *tracker_process(void *args)
{
	struct session  *session;
	tracker_queue_t *tracker_queue;
	tracker_t       *tracker;
	char             cmd[4096];
	packet_size_t    packet_size;

	mutex_lock(&tracker_lock);
	TAILQ_FOREACH(tracker_queue, &tracker_queue_head, trackers) {
		tracker = tracker_queue->tracker;
		switch (tracker->type) {
			case TRACKER_TYPE_GIT:
				tracker_git_update(tracker);
				break;
		}
	}
	TAILQ_FOREACH(tracker_queue, &tracker_queue_head, trackers) {
		tracker     = tracker_queue->tracker;
		packet_size = snprintf(cmd, sizeof(cmd)-1, "tracker update %s", tracker->output);
		for (int x = 0; x<tracker->nr_sessions; x++) {
			session = tracker->sessions[x];
			pthread_mutex_lock(&session->session_lock);
			websockets_sendall(session, cmd, packet_size);
		}
	}
	mutex_unlock(&tracker_lock);
}

void init_tracker()
{
	tasklet_enqueue(&tracker_process, NULL, 20, 1);
}
