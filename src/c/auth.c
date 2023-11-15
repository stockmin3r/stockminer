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
	char               pubkey64[128];
	int                pwdsize, pubkey64_size;

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

	pubkey64_size = base64_encode(kp.pk, hydro_sign_PUBLICKEYBYTES, pubkey64);

	memset(&user, 0, sizeof(user));
	strcpy(user.uname, username);
	memcpy(user.pubkey, kp.pk, sizeof(kp.pk));
//	strcpy(user.pubkey, pubkey64);
	user.uid = fs_filesize(USERS_PATH)/sizeof(struct user); // race condition, this needs to be done server-side
	fs_appendfile(USERS_PATH, &user, sizeof(user));	
	printf(BOLDGREEN "added user: %s uid: %d pubkey: %s" RESET "\n", username, user.uid, pubkey64);
}

// in progress
void apc_server_auth(struct connection *connection, char **argv)
{
	uint8_t      nonce[32];
	char         nonce64[64] = {0};
	char         challenge[4096];
	char         signature[4096];
	char         sig64[128];
	char         auth[512];
	char         user_pubkey[hydro_sign_PUBLICKEYBYTES];
	char        *p;
	struct user *user;
	size_t       nonce64_size, sig_size, nbytes;

	hydro_random_buf(nonce, sizeof(nonce));
	nonce64_size = base64_encode(nonce, sizeof(nonce), nonce64);
	printf("nonce64: %s sz: %d\n", nonce64, nonce64_size);

	openssl_write_sync(connection, nonce64, nonce64_size);
	nbytes = openssl_read_sync2(connection, auth, sizeof(auth)-1);
	printf("auth: %s nbytes: %d\n", auth, nbytes);
	p = strchr(auth, '|');
	if (!p)
		goto out_error;
	*p++ = 0;
	sig_size = base64_decode(p, sizeof(signature)-1, signature);

	user = search_user(auth);
	if (!user) {
		printf("failed user\n");
		return;
	}
	printf("got user\n");
	
	strcpy(challenge, user->uname);
	strcat(challenge, "|");
	memcpy(challenge+strlen(challenge), nonce, sizeof(nonce));

	if (hydro_sign_verify(signature, signature, strlen(user->uname)+sizeof(nonce)+1, "context0", user->pubkey) == 0) {
		printf("verified user\n");
	} else {
		printf("failed\n");
	}
out_error:
	printf("fucked\n");
}

void admin_client_auth(char *command)
{
	uint8_t            kp_seed[hydro_sign_SEEDBYTES];
	hydro_sign_keypair kp;
	char               challenge[4096];
	char               nonce[4096];
	char               sig64[4096];
	char               auth[4096];
	char              *argv[3];
	char              *username, *password;
	uint8_t            signature[hydro_sign_BYTES];
	int                username_size, nonce_size, sig64_size;

	int argc = cstring_split(command, argv, 3, ' ');
printf("cmd: %s\n", command);
	printf("argc: %d\n", argc);
	username = argv[1];
	password = argv[2];
	printf("u: %s %s\n", username, password);
	if (!username || !password)
		return;

	username_size = strlen(username);
	if (username_size >= MAX_USERNAME_SIZE)
		return;
printf("username size: %d\n", username_size);
	apc_send_command("auth");

	nonce_size = openssl_read_sync2(&apc_connection, nonce, sizeof(nonce)-1);
	printf("nonce size: %d\n", nonce_size);
	if (nonce_size <= 0 || nonce_size >= 96)
		return;

	memcpy(challenge, username, username_size);
	challenge[username_size] = '|';
	strcpy(challenge, username);
	strcat(challenge, "|");
	strcat(challenge, nonce);
	
	hydro_pwhash_deterministic(kp_seed, sizeof(kp_seed), password, strlen(password), "context0", NULL, OPSLIMIT, 0, 1);
	hydro_sign_keygen_deterministic(&kp, kp_seed);
	hydro_sign_create(signature, challenge, strlen(challenge), "context0", kp.sk);

	sig64_size = base64_encode(signature, hydro_sign_BYTES, sig64);
	printf("sig64size: %d\n", sig64_size);
	if (sig64_size >= sizeof(sig64))
		return;
	printf("siglen: %d\n", sig64_size);
	strcpy(auth, username);
	strcat(auth, "|");
	strcat(auth, sig64);

	apc_send_command(auth);
	memset(auth, 0, sizeof(auth));
	if (!openssl_read_sync2(&apc_connection, auth, sizeof(auth)-1)); {
		printf(BOLDRED "failed to read auth response" RESET "\n");
		return;
	}
	printf("%s\n", auth);
}
