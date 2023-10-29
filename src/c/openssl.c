#include <conf.h>
#include <extern.h>

#define MAX_RETRIES 3

static int
openssl_connect_retry(SSL *ssl)
{
	int err, ret, retries = 0;

	while ((ret=SSL_connect(ssl)) < 0) {
		err = SSL_get_error(ssl, ret);
		ERR_print_errors_fp(stdout);
		if ((err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) || (retries > MAX_RETRIES))
			return (false);
		retries++;
	}
	return (true);
}

bool openssl_connect_sync(struct connection *connection, unsigned int ipaddr, unsigned short port)
{
	connection->ctx = SSL_CTX_new(TLS_client_method());
	connection->ssl = SSL_new(connection->ctx);
	connection->fd  = net_tcp_connect2(ipaddr, port);
	SSL_set_fd(connection->ssl, connection->fd);
	SSL_set_connect_state(connection->ssl);

	for (int x=0; x<MAX_RETRIES; x++) {
		if (openssl_connect_retry(connection->ssl) == true)
			return (true);
	}
	return (false);
}

// incomplete
struct connection *openssl_connect_async(void)
{
	struct connection *connection = zmalloc(sizeof(*connection));
	if (!connection)
		return NULL;
	connection->ctx       = SSL_CTX_new(TLS_client_method());
	connection->ssl       = SSL_new(connection->ctx);
	connection->protocol |= TLS_CLIENT;
	SSL_set_fd(connection->ssl, connection->fd);
	SSL_set_connect_state(connection->ssl);
	return (connection);
}

struct connection *openssl_accept_async(ssl_server_t *ssl_server, int fd)
{
	struct connection *connection = zmalloc(sizeof(*connection));
	if (!connection) {
		errno = ENOMEM;
		return NULL;
	}
	connection->fd  = fd;
	connection->ssl = SSL_new(ssl_server->ssl_ctx);
	SSL_set_fd(connection->ssl, fd);
	SSL_set_accept_state(connection->ssl);
	return (connection);
}

packet_size_t openssl_read_sync(struct connection *connection, char *buf, packet_size_t max_packet_size)
{
	packet_size_t packet_size = 0;
	int           nbytes;

	do {
		nbytes = SSL_read(connection->ssl, buf+packet_size, MIN(max_packet_size, 16 KB));
		if (nbytes == 0 || errno == EBADF)
			return packet_size;
		if (nbytes == -1) {
			net_select_waitfd(connection->fd, 1);
			continue;
		}
		packet_size += nbytes;
//		printf("packet_size: %d rc: %d packet_size: %d\n", packet_size, nbytes, packet_size);
	} while (packet_size != max_packet_size);
	buf[packet_size] = 0;
	return (packet_size);
}

packet_size_t openssl_read_sync2(struct connection *connection, char *buf, packet_size_t max_packet_size)
{
	SSL          *ssl         = connection->ssl;
	packet_size_t packet_size = 0;
	int           nbytes;

	do {
		nbytes = SSL_read(ssl, buf+packet_size, MIN(max_packet_size, 16 KB));
		if (nbytes > 0)
			packet_size += nbytes;
	} while (SSL_pending(ssl));
	buf[nbytes] = 0;
	return (packet_size);
}

packet_size_t openssl_write_sync(struct connection *connection, char *packet, packet_size_t packet_size)
{
	char *p = packet;
	int   i = 0;

	do {
		i = SSL_write(connection->ssl, p, MIN(packet_size, 16 KB));
		if (i == 0 || errno == EPIPE)
			return -1;
//		printf("len: %d vs: %d err: %d ERRNO: %s\n", i, packet_len, SSL_get_error(connection->ssl, i), strerror(errno));
		if (i == -1) {
			net_select_waitfd(connection->fd, 1);
			continue;
		}
		packet_size -= i;
		p           += i;
	} while (packet_size > 0);
	return (p-(char *)packet);
}

void openssl_destroy(struct connection *connection)
{
	SSL_shutdown(connection->ssl);
	SSL_CTX_free(connection->ctx);
	SSL_free(connection->ssl);
	close(connection->fd);
}

packet_size_t openssl_read_http(struct connection *connection, char *buf, packet_size_t max_packet_size)
{
	char         *p;
	packet_size_t nbytes, gzlen, header_len, total_len;

	buf[0] = 0;
	nbytes = openssl_read_sync(connection, buf, max_packet_size);
	if (nbytes <= 0)
		return 0;

	p = strstr(buf, "gth:");
	if (!p)
		return 0;
	gzlen = atoi(p+5);
	if (gzlen <= 0 || gzlen >= max_packet_size)
		return 0;
	p = strstr(p, "\r\n\r\n");
	if (!p)
		return 0;
	p += 4;
	header_len = p-buf;
	total_len  = gzlen + header_len;
	if (total_len != nbytes) {
		int len = openssl_read_sync(connection, buf+nbytes, total_len-nbytes+4);
		if (len+nbytes != total_len)
			return 0;
		return total_len;
	}
	return nbytes;
}

int connection_ssl_recv(struct connection *connection)
{
	printf("connection_ssl_recv: connection: %p packet: %p size: %ldd\n", connection, connection->packet, connection->packet_size);
	int nbytes = SSL_read(connection->ssl, connection->packet, 16 KB);

	printf("ssl read: %d\n", nbytes);
	if (nbytes > 0) {
		printf("%s\n", connection->packet);
		return nbytes;
	}
	int err = SSL_get_error(connection->ssl, err);
	if (err == SSL_ERROR_SSL || err == SSL_ERROR_ZERO_RETURN || errno != EWOULDBLOCK) {
		printf("ssl error: %d\n", err);
		return 0;
	}
	errno = EWOULDBLOCK;
	return -1;
}

int connection_ssl_send(struct connection *connection)
{
	int nbytes = SSL_read(connection->ssl, connection->packet, connection->packet_size);
	if (nbytes > 0)
		return nbytes;
	int err = SSL_get_error(connection->ssl, err);
	if (err == SSL_ERROR_SSL || err == SSL_ERROR_ZERO_RETURN || errno != EWOULDBLOCK)
		return 0;
	errno = EWOULDBLOCK;
	return -1;
}

int connection_ssl_handshake(struct connection *connection)
{
	int ret = SSL_do_handshake(connection->ssl);
	if (ret == 1) {
		connection->recv   = connection_ssl_recv;
		connection->send   = connection_ssl_send;
		connection->state |= NETWORK_STATE_TLS_HANDSHAKE_DONE;
		return 1;
	}

	int err = SSL_get_error(connection->ssl, ret);
	switch (err) {
		case SSL_ERROR_WANT_READ:
			connection->events |= EPOLLIN;
			connection->events &= ~EPOLLOUT;
			event_mod(connection);
			break;
		case SSL_ERROR_WANT_WRITE:
			connection->events |= EPOLLOUT;
			connection->events &= ~EPOLLIN;
			event_mod(connection);
			break;
		default:
			event_del(connection);
			return 0; // EDEAD
	}
	return -1; // EAGAIN
}

struct ssl_server *openssl_server(const char *cert, const char *key)
{
	struct ssl_server *ssl_server;

	ssl_server          = (struct ssl_server *)zmalloc(sizeof(*ssl_server));
	ssl_server->ssl_ctx = SSL_CTX_new(TLS_server_method());

	if (cert && SSL_CTX_use_certificate_file(ssl_server->ssl_ctx, cert, SSL_FILETYPE_PEM) <= 0) {
		printf(BOLDRED "SSL_CTX_use_certificate_file: www/cert.pem error" RESET "\n");
		return (NULL);
	}
	if (key && SSL_CTX_use_PrivateKey_file(ssl_server->ssl_ctx, key, SSL_FILETYPE_PEM) <= 0) {
		printf(BOLDRED "SSL_CTX_use_PrivateKey_file: www/key.pem error" RESET "\n");
		return (NULL);
	}
	SSL_CTX_set_verify(ssl_server->ssl_ctx, SSL_VERIFY_NONE, NULL);
	SSL_CTX_set_mode  (ssl_server->ssl_ctx, SSL_OP_NO_COMPRESSION);
//	SSL_CTX_set_mode  (ssl_server->ssl_ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
	SSL_CTX_set_session_cache_mode(ssl_server->ssl_ctx, SSL_SESS_CACHE_OFF);
	return (ssl_server);
}

void openssl_server_sync(www_callback_t callback, port_t port, bool create_thread)
{
	struct sockaddr_in  srv, cli;
	struct connection  *connection = NULL;
	ssl_server_t       *ssl_server;
	socklen_t           slen       = 16;
	int                 server_fd, client_fd, err;

	ssl_server = openssl_server("www/cert.pem", "www/key.pem");
	if (!ssl_server)
		return;

	server_fd = net_tcp_bind(LOCALHOST, port);
	if (server_fd == -1) {
		printf(BOLDRED "[-] openssl_server_sync(): Failed to bind to port (%d)" RESET "\n", port);
		return;
	}

	for (;;) {
		if (!connection) {
			connection      = (struct connection *)zmalloc(sizeof(*connection));
			connection->ssl = SSL_new(ssl_server->ssl_ctx);
		}
		client_fd = accept4(server_fd, (struct sockaddr *)&cli, &slen, SOCK_CLOEXEC);
		if (client_fd < 0)
			continue;
		connection->fd = client_fd;
		SSL_set_fd(connection->ssl, client_fd);
		SSL_set_accept_state(connection->ssl);
		if (!(connection->packet=malloc(MAX_PACKET_SIZE))) {
			close(client_fd);
			continue;
		}

		if ((err=SSL_accept(connection->ssl)) <= 0 && SSL_get_error(connection->ssl, err)) {
			close(client_fd);
			SSL_free(connection->ssl);
			connection = NULL;
			continue;
		}
		if (create_thread)
			thread_create(callback, connection);
		else
			callback(connection);
		connection = NULL;
	}
}
