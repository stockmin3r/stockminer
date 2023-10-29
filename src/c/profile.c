#include <conf.h>

void session_load_profile(struct session *session)
{
	struct user    *user = session->user;
	struct squeak  *squeak;
	struct filemap  FILEMAP;
	char            path[32];
	char           *map;
	int x, nr_squeaks, nr_followers, nr_following, uid;

	if (!user || *user->uname == '\0')
		return;

	uid = user->uid;
	snprintf(path, sizeof(path)-1, "db/uid/%d/sqdb", uid);
	map = MAP_FILE_RO(path, &FILEMAP);
	if (!map) {
		session->sqdb = NULL;
		return;
	}
	nr_squeaks = FILEMAP.filesize/sizeof(struct squeak);
	squeak = (struct squeak *)map;
	session->sqdb = (struct squeak **)malloc(sizeof(void *) * (nr_squeaks+100));
	for (x=0; x<nr_squeaks; x++) {
		session->sqdb[x] = (struct squeak *)malloc(sizeof(struct squeak));
		memcpy(session->sqdb[x], squeak, sizeof(*squeak));
		printf("loading squeak: %s user: %s\n", squeak->msg, user->uname);
		squeak++;
	}
	user->nr_squeaks = nr_squeaks;
	UNMAP_FILE(map, &FILEMAP);

	/* Followers */
	snprintf(path, sizeof(path)-1, FOLLOWERS_PATH, uid);
	map = MAP_FILE_RO(path, &FILEMAP);
	if (map) {
		nr_followers = (FILEMAP.filesize/4);
		user->nr_followers = nr_followers;
		UNMAP_FILE(map, &FILEMAP);
	}

	/* Following */
	snprintf(path, sizeof(path)-1, FOLLOWING_PATH, uid);
	map = MAP_FILE_RO(path, &FILEMAP);
	if (map) {
		nr_following = (FILEMAP.filesize/4);
		user->nr_following = nr_following;
		UNMAP_FILE(map, &FILEMAP);
	}
}

void squeak_timestamp(unsigned long timestamp, char *timestr)
{
	int interval = (time(NULL)-timestamp)/60;
	char *fmtstr;

	if (interval < 60) {
		if (!interval)
			interval = 1;
		sprintf(timestr, "%d min", interval);
		return;
	}
	interval /= 60;
	if (interval < 60) {
			if (!interval)
				interval = 1;
		sprintf(timestr, "%d hr", interval);
		return;
	}
	interval /= 60;
	if (interval < 60) {
		if (!interval) {
			interval = 1;
			fmtstr = "%d day";
		} else
			fmtstr = "%d days";
		sprintf(timestr, fmtstr, interval);
		return;
	}
}

int squeak_range(struct session *session, char *packet, int start)
{
	struct user   *user = session->user;
	struct squeak *squeak;
	char           timestr[128];
	int            x, packet_len = 0, nr_packed = 0, nr_squeaks = user->nr_squeaks;

	if (!session->sqdb)
		return 0;
	for (x=start; x>=0; x--) {
		squeak = session->sqdb[x];
		if (!squeak)
			continue;
		squeak_timestamp(squeak->timestamp, timestr);
		packet_len += sprintf(packet+packet_len, "m`{\"n\":\"%s\",\"t\":\"%s\",\"m\":\"%s\",\"url\":\"%s\",\"uname\":\"%s\",\"uid\":\"%d\",\"i\":\"%d\",\"l\":\"%d\",\"r\":\"%d\"}`",
							  user->realname, timestr, squeak->msg, squeak->url, user->uname, user->uid, session->avatar?1:0, squeak->nr_likes, squeak->nr_replies);
		if (nr_packed++ >= 5)
			break;
	}
	return (packet_len);
}

void rpc_user_chart(struct rpc *rpc)
{
	char        *csv = rpc->argv[1];
	struct stat  sb;
	char         path[256];
	char         id[16];
	int          csv_size;

	while (1) {
		random_string(id);
		snprintf(path, sizeof(path)-1, "db/uid/%d/%s.csv", rpc->session->user->uid, id);
		if (stat(path, &sb) != -1)
			continue;
	}
	csv_size = strlen(csv);
	if (csv_size == 0 || csv_size > MAX_UCHART_CSV_SIZE)
		return;
	fs_writefile(path, csv, csv_size);
}

void rpc_profile_follow(struct rpc *rpc)
{
	char *username = rpc->argv[1];
	char  path[256];
	int   uid;

	uid = uid_by_username(username);
	if (uid < 0)
		return;
	snprintf(path, sizeof(path)-1, FOLLOWERS_PATH, rpc->session->user->uid);
	fs_appendfile(path, (char *)&uid, sizeof(uid));
}

/*
 *  profile: {r:"Real Name",l:location, url:"https://mysite.com", d:description, j:"JoinDate", img:/user/img/pic.jpeg, fs:nr_followers, fg:nr_following}
 *
 *  msg: {ts:"timestamp", txt:"entire msg", u:"/blob/url", l:"nr_likes", r:"nr_replies" }
 *  squeak`profile`{r:"realname",l:"location"}`m`{ts:"1 day",tx:"this is the msg"}`sqmsg`{ts:"1 day",tx:"msg 2"}
 *
 */
void rpc_profile_set_image(struct rpc *rpc)
{
	struct session *session = rpc->session;
	char           *img_url = rpc->argv[1];
	strncpy(session->user->img_url, img_url, sizeof(session->user->img_url)-1);
	db_user_update(session->user);
}

void rpc_profile_set(struct rpc *rpc)
{
	struct session *session  = rpc->session;
	char           *profile  = rpc->argv[1];
	char           *argv[5]  = {0};
	char            emsg[64];
	char           *name, *location, *url,*desc;
	int             argc, len, msglen, error;

	argc = cstring_split(profile, argv, 5, '`');
	if (!argc)
		return;

	name     = argv[1];
	location = argv[2];
	url      = argv[3];
	desc     = argv[4];
	printf("name: %s loc: %s url: %s desc: %s\n", name, location, url, desc);

	/* REALNAME */
	if (name) {
		len = strlen(name);
		if (len > sizeof(session->user->realname)-1) {
			error = PROFILE_NAME_ERROR;
			goto out_error;
		}
		memcpy(session->user->realname, name, len);
		session->user->realname[len] = 0;
	}

	/* LOCATION */
	if (location) {
		len = strlen(location);
		if (len > sizeof(session->user->location)-1) {
			error = PROFILE_LOCATION_ERROR;
			goto out_error;
		}
		memcpy(session->user->location, location, len);
		session->user->location[len] = 0;
	}

	/* URL */
	if (url) {
		len = strlen(url);
		if (len > sizeof(session->user->url)-1) {
			error = PROFILE_URL_ERROR;
			goto out_error;
		}
		memcpy(session->user->url, url, len);
		session->user->url[len] = 0;
	}

	/* DESCRIPTION */
	if (desc) {
		len = strlen(desc);
		if (len > sizeof(session->user->desc)-1) {
			error = PROFILE_DESC_ERROR;
			goto out_error;
		}
		memcpy(session->user->desc, desc, len);
		session->user->desc[len] = 0;
	}
	db_user_update(session->user);
	printf(BOLDGREEN "SAVED SESSION" RESET "\n");
	websocket_send(rpc->connection, "err 1 ok", 8);
	return;
out_error:
	msglen = sprintf(emsg, "err 1 fail %d", error);
	websocket_send(rpc->connection, emsg, msglen);
}

int packet_profile(struct session *session, char *packet)
{
	struct user *user = session->user;
	int packet_len;

	if (*session->user->img_url == '\0')
		strcpy(session->user->img_url, "minibat");

	strcpy(packet, "squeak`");
	packet_len  = 7;
	packet_len += snprintf(packet+packet_len, 1024, "profile`{\"r\":\"%s\",\"l\":\"%s\",\"u\":\"%s\",\"d\":\"%s\",\"j\":\"%s\",\"img\":\"%s\",\"fs\":\"%d\",\"fg\":\"%d\"}",
				 user->realname, user->location, user->url, user->desc, "2023", user->img_url, user->nr_followers, user->nr_following);

	if (session->user->nr_squeaks) {
		packet[packet_len++] = '`';
		packet_len += squeak_range(session, packet+packet_len, user->nr_squeaks-1);
		packet[--packet_len] = 0;
	}
	return (packet_len);
}

void rpc_profile_get(struct rpc *rpc)
{
	int packet_len = packet_profile(rpc->session, rpc->packet);
	if (packet_len <= 0)
		return;
	websocket_send(rpc->connection, rpc->packet, packet_len);
	printf(BOLDMAGENTA "%s" RESET "\n", rpc->packet);
}

void rpc_profile_squeak(struct rpc *rpc)
{
	struct session *session = rpc->session;
	char           *SQID    = rpc->argv[1];
	char           *msg     = rpc->argv[2];
	struct user    *user    = session->user;
	struct squeak  *squeak;
	struct qpage   *qpage;
	char            packet[4096];
	char            path[256];
	int             msg_size, packet_len, x;

	if (!session->user || !session->user->logged_in)
		return;

	msg_size = strlen(msg);
	if (msg_size >= MAX_SQUEAK_MSGSIZE)
		return;

	squeak = (struct squeak *)zmalloc(sizeof(*squeak));
	snprintf(path, sizeof(path)-1, "db/uid/%d/sqdb", session->user->uid);
	strcpy(squeak->msg, msg);
	memcpy(squeak->url, SQID, 15);
	squeak->url[15]   = 0;
	squeak->timestamp = time(NULL);
	fs_appendfile(path, (char *)squeak, sizeof(*squeak));

	if (!session->sqdb)
		session->sqdb = (struct squeak **)malloc(sizeof(struct squeak *) * 100);
	if (user->nr_squeaks >= user->max_squeaks)
		session->sqdb = (struct squeak **)realloc(session->sqdb, user->nr_squeaks+100);
	session->sqdb[user->nr_squeaks++] = squeak;

	qpage = session->quadverse[QUADVERSE_PROFILE]->qpage;
	mutex_lock(&qpage->qpage_lock);
	for (x=0; x<qpage->nr_subscribers; x++) {
		struct subscriber *qsub = qpage->subscribers[x];
		if (!qsub)
			continue;
		packet_len = snprintf(packet, 1048, "squeak🗖%d`m`{\"n\":\"%s\",\"t\":\"1min\",\"m\":\"%s\",\"url\":\"%s\",\"uname\":\"%s\",\"uid\":\"%d\",\"i\":\"%d\",\"l\":\"0\",\"r\":\"0\"}`",
							  qsub->QVID, user->realname, msg, SQID, session->user->uname, session->user->uid, session->avatar?1:0);
		websocket_send(qsub->session->websockets[qsub->client_id], packet, packet_len);
		printf(BOLDMAGENTA "%s" RESET "\n", packet);
	}
	mutex_unlock(&qpage->qpage_lock);
}
