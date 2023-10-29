#ifndef __PROFILE_H
#define __PROFILE_H

#define MAX_SQUEAK_MSGSIZE  1024
#define MAX_UCHART_CSV_SIZE 500 KB

#define FOLLOWERS_PATH "db/uid/%d/followers"
#define FOLLOWING_PATH "db/uid/%d/following"

struct squeak {
	char           msg[1024];
	char           url[16];
	time_t         timestamp;
	int            nr_likes;
	int            nr_dislikes;
	unsigned short nr_replies;
}__attribute__((packed));

struct resqueak {
	char           url[16];
	unsigned int   magic;
};

#define REQUEAK_MAGIC 0xdeadbeef

#define PROFILE_NAME_ERROR     1
#define PROFILE_LOCATION_ERROR 2
#define PROFILE_URL_ERROR      3
#define PROFILE_DESC_ERROR     4

#endif
