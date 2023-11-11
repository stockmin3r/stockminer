#include <conf.h>

uint64_t OPSLIMIT = 1000;

// in progress
void apc_server_auth(struct connection *connection, char *pubkey)
{
	uint8_t nonce[32];
	char    nonce64[64] = {0};
	char    response[4096];
	size_t  nonce64_size, nbytes;

	hydro_random_buf(nonce, sizeof(nonce));
	nonce64_size = base64_encode(nonce, sizeof(nonce), nonce64);
	printf("nonce64: %s\n", nonce64);

	openssl_write_sync(connection, nonce64, nonce64_size);
	nbytes = openssl_read_sync2(connection, response, sizeof(response)-1);
}

void admin_client_auth(char *username, char *password)
{
	uint8_t kp_seed[hydro_sign_SEEDBYTES];
	char   challenge[4096];
	int    username_size;

	if (!username || !password)
		return;

	username_size = strlen(username);
	if (username_size >= MAX_USERNAME_SIZE)
		return;

	apc_send_command("auth");

	memcpy(challenge, username, username_size);
	challenge[username_size] = '|';
	strcpy(challenge, username);
	
	hydro_pwhash_deterministic(kp_seed, sizeof kp_seed, password, strlen(password), "context0", NULL, OPSLIMIT, 0, 1);

}
