#include <conf.h>
#include <extern.h>

void cryptocompare_handler(struct connection *connection)
{

}

void *cryptocompare_thread(void *args)
{
	websocket_connect_sync("streamer.cryptocompare.com", Server.CC_ADDR, "/v2?format=streamer", cryptocompare_handler);
}
