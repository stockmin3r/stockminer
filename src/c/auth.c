#include <conf.h>
#include <extern.h>

uint64_t OPSLIMIT = 1000;

void lpc_adduser(struct connection *connection, char **argv)
{
	char              *username = argv[0];
	char              *password = argv[1];
	uint8_t            kp_seed[hydro_sign_SEEDBYTES];
	hydro_sign_keypair kp;
	struct user        user = {0};
	char               pwd[1024];
	int                pwdsize;

	if (!username || !password || strlen(username) >= MAX_USERNAME_SIZE)
		return;

	pwdsize = strlen(username) + strlen(password) + 1;
	if (pwdsize >= sizeof(pwd)-3)
		return;

	strcpy(pwd, username);
	strcat(pwd, "|");
	strcat(pwd, password);

	// Generate a public/private x25519 keypair from the seed
	hydro_pwhash_deterministic(kp_seed, sizeof(kp_seed), pwd, pwdsize, "context0", NULL, OPSLIMIT, 0, 1);
	hydro_sign_keygen_deterministic(&kp, kp_seed);

	memset(&user, 0, sizeof(user));
	strcpy(user.uname, username);
	memcpy(user.pubkey, kp.pk, sizeof(kp.pk));
	user.uid = fs_filesize((char *)DB_USERS_PATH)/sizeof(struct user); // race condition, this needs to be done server-side
	fs_appendfile((char *)DB_USERS_PATH, &user, sizeof(user));
	printf(BOLDGREEN "added user: %s uid: %d" RESET "\n", username, user.uid);
}

#ifdef DEBUG
void auth_debug(unsigned char *seed, unsigned char *pk, unsigned char *sk, unsigned char *signature)
{
	for (int x = 0; x<hydro_sign_SEEDBYTES; x++)
		printf(BOLDGREEN "%x " RESET, (unsigned char)seed[x]);
	printf("\n");
    
	for (int x = 0; x<hydro_sign_PUBLICKEYBYTES; x++)
		printf(BOLDRED "%x " RESET, (unsigned char)pk[x]);
	printf("\n");

	for (int x = 0; x<hydro_sign_SECRETKEYBYTES; x++)
		printf(BOLDYELLOW "%x " RESET, (unsigned char)sk[x]);
    
	printf("\n");
	for (int x = 0; x<hydro_sign_BYTES; x++)
		printf(BOLDMAGENTA "%x " RESET, (unsigned char)signature[x]);
	printf("\n");
}
#else
#define auth_debug
#endif

void apc_server_auth(struct connection *connection, char **argv)
{
	struct session *session;
	struct user    *user;
	uint8_t         nonce[32];
	char            challenge[4096] = {0};
	char            auth[4096] = {0};
	char           *username, *signature;
	bool            verified = false;
	size_t          sig_size, username_size, nbytes;

	hydro_random_buf(nonce, sizeof(nonce));
	openssl_write_sync(connection, nonce, sizeof(nonce));
	nbytes    = openssl_read_sync2(connection, auth, sizeof(auth)-1);
	signature = strchr(auth, '|');
	if (!signature)
		goto out;
	*signature++ = 0;
	user = search_user(auth);
	if (!user)
		goto out;

	username      = user->uname;
	username_size = strlen(user->uname);
	memcpy(challenge, username, username_size);
	challenge[username_size] = '|';
	memcpy(challenge+username_size+1, nonce, sizeof(nonce));

	if (hydro_sign_verify(signature, challenge, username_size+1+sizeof(nonce), "context0", user->pubkey) == 0) {
		verified = true;
		session  = session_by_username(username);
		if (!session)
			goto out;
		connection->session    = session;
		connection->authorized = true; // allow the localhost terminal user to use webscript.c::apc_webscript()
	}
out:
	openssl_write_sync(connection, verified?"1":"0", 1);
}

void admin_client_auth(char *command)
{
	uint8_t            kp_seed[hydro_sign_SEEDBYTES];
	hydro_sign_keypair kp;
	char               challenge[4096];
	char               nonce[32];
	char               auth[4096];
	char              *argv[3];
	char              *username, *password;
	uint8_t            signature[hydro_sign_BYTES];
	int                username_size, challenge_size;

	if (cstring_split(command, argv, 3, ' ') != 3)
		return;

	username = argv[1];
	password = argv[2];
	if (!username || !password)
		return;

	username_size = strlen(username);
	if (username_size >= MAX_USERNAME_SIZE)
		return;

	// 1) Get nonce from the server
	apc_send_command("auth");
	if (openssl_read_sync2(&apc_connection, nonce, sizeof(nonce)) != sizeof(nonce))
		return;

	// 2) Concat user|nonce
	memcpy(challenge, username, username_size);
	challenge[username_size] = '|';
	memcpy(challenge+username_size+1, nonce, sizeof(nonce));

	// 3) Recalculate the pub/priv keys for: user|password
	memcpy(auth, username, username_size);
	auth[username_size] = '|';
	strncpy(auth+username_size+1, password, 128);

	hydro_pwhash_deterministic(kp_seed, sizeof(kp_seed), auth, strlen(auth), "context0", NULL, OPSLIMIT, 0, 1);
	hydro_sign_keygen_deterministic(&kp, kp_seed);
	hydro_sign_create(signature, challenge, username_size+1+sizeof(nonce), "context0", kp.sk);

	// concatenate the signature - username|signature
	memcpy(auth+username_size+1, signature, hydro_sign_BYTES);

	auth_debug(kp_seed, kp.pk, kp.sk, signature);

	openssl_write_sync(&apc_connection, auth, username_size+1+hydro_sign_BYTES);
	auth[0] = 0;
	if (!openssl_read_sync2(&apc_connection, auth, 1)) {
		printf(BOLDRED "failed to read auth response" RESET "\n");
		return;
	}
	if (auth[0] == '1')
		printf(BOLDGREEN "auth success" RESET "\n");
	else
		printf(BOLDRED "auth failure" RESET "\n");
}
