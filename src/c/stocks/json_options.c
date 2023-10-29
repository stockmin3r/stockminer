#include <conf.h>
#include <stocks/options.h>

char option_oi_calls_json[8 KB];
char option_oi_calls_json2[8 KB];
char option_vol_calls_json[8 KB];
char option_vol_calls_json2[8 KB];
int option_oi_calls_buffer  = 2;
int option_oi_calls_json_len;
int option_oi_calls_json_len2;
int option_vol_calls_buffer = 2;
int option_vol_calls_json_len;
int option_vol_calls_json_len2;

char option_oi_puts_json[8 KB];
char option_oi_puts_json2[8 KB];
char option_vol_puts_json[8 KB];
char option_vol_puts_json2[8 KB];
int option_oi_puts_buffer   = 2;
int option_oi_puts_json_len;
int option_oi_puts_json_len2;
int option_vol_puts_json_len;
int option_vol_puts_json_len2;
int option_vol_puts_buffer  = 2;

char *json_option_oi_calls_leaders(int *json_len) {
	switch (option_oi_calls_buffer) {
		case 1:*json_len = option_oi_calls_json_len; return option_oi_calls_json;
		case 2:*json_len = option_oi_calls_json_len2;return option_oi_calls_json2;
	}
	return NULL;
}
char *json_option_vol_calls_leaders(int *json_len) {
	switch (option_vol_calls_buffer) {
		case 1:*json_len = option_vol_calls_json_len; return option_vol_calls_json;
		case 2:*json_len = option_vol_calls_json_len2;return option_vol_calls_json2;
	}
	return NULL;
}
char *json_option_oi_puts_leaders(int *json_len) {
	switch (option_oi_puts_buffer) {
		case 1:*json_len = option_oi_puts_json_len; return option_oi_puts_json;
		case 2:*json_len = option_oi_puts_json_len2;return option_oi_puts_json2;
	}
	return NULL;
}
char *json_option_vol_puts_leaders(int *json_len) {
	switch (option_vol_puts_buffer) {
		case 1:*json_len = option_vol_puts_json_len; return option_vol_puts_json;
		case 2:*json_len = option_vol_puts_json_len2;return option_vol_puts_json2;
	}
	return NULL;
}

void update_json_options_oi_calls()
{
	struct Option  *option;
	struct opstock *opstock;
	char *jptr, *json;
	int nbytes, using_buffer, x;

	switch (option_oi_calls_buffer) {
		case 1:json = option_oi_calls_json2;using_buffer = 2;break;
		case 2:json = option_oi_calls_json; using_buffer = 1;break;
	}
	jptr   = json; *jptr  = '['; nbytes = 1;

	for (x=0; x<nr_options_oi_calls; x++) {
		opstock = options_oi_calls[x];
		if (!opstock)
			continue;
		option = opstock->option;
		nbytes += snprintf(jptr+nbytes, 8191, "{\"oN\":\"%s\",\"oP\":\"%.2f\",\"oD\":\"%.2f\",\"oV\":\"%d\",\"oI\":\"%d\"},",
		option->contract, option->lastPrice, option->percentChange, option->volume, option->openInterest);
	}
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;

	switch (using_buffer) {
		case 1:option_oi_calls_buffer = 1;option_oi_calls_json_len  = nbytes;break;
		case 2:option_oi_calls_buffer = 2;option_oi_calls_json_len2 = nbytes;break;
	}
}

void update_json_options_vol_calls()
{
	struct Option  *option;
	struct opstock *opstock;
	char *jptr, *json;
	int nbytes, using_buffer, x;

	switch (option_vol_calls_buffer) {
		case 1:json = option_vol_calls_json2;using_buffer = 2;break;
		case 2:json = option_vol_calls_json; using_buffer = 1;break;
	}
	jptr   = json; *jptr  = '['; nbytes = 1;

	for (x=0; x<nr_options_vol_calls; x++) {
		opstock = options_vol_calls[x];
		if (!opstock)
			continue;
		option  = opstock->option;
		nbytes += snprintf(jptr+nbytes, 8191, "{\"oN\":\"%s\",\"oP\":\"%.2f\",\"oD\":\"%.2f\",\"oV\":\"%d\",\"oI\":\"%d\"},",
		option->contract, option->lastPrice, option->percentChange, option->volume, option->openInterest);
	}
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;
	option_vol_calls_json_len = nbytes;

	switch (using_buffer) {
		case 1:option_vol_calls_buffer   = 1;option_vol_calls_json_len = nbytes;break;
		case 2:option_vol_calls_buffer   = 2;option_vol_calls_json_len = nbytes;break;
	}
}

/* PUTS */
void update_json_options_oi_puts()
{
	struct Option  *option;
	struct opstock *opstock;
	char *jptr, *json;
	int nbytes, using_buffer, x;

	switch (option_oi_puts_buffer) {
		case 1:json = option_oi_puts_json2;using_buffer = 2;break;
		case 2:json = option_oi_puts_json; using_buffer = 1;break;
	}
	jptr = json; *jptr = '['; nbytes = 1;

	for (x=0; x<nr_options_oi_puts; x++) {
		opstock = options_oi_puts[x];
		if (!opstock)
			continue;
		option = opstock->option;
		nbytes += snprintf(jptr+nbytes, 8191, "{\"oN\":\"%s\",\"oP\":\"%.2f\",\"oD\":\"%.2f\",\"oV\":\"%d\",\"oI\":\"%d\"},",
		option->contract, option->lastPrice, option->percentChange, option->volume, option->openInterest);
	}
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;

	switch (using_buffer) {
		case 1:option_oi_puts_buffer = 1;option_oi_puts_json_len  = nbytes;break;
		case 2:option_oi_puts_buffer = 2;option_oi_puts_json_len2 = nbytes;break;
	}
}

void update_json_options_vol_puts()
{
	struct Option  *option;
	struct opstock *opstock;
	char *jptr, *json;
	int nbytes, using_buffer, x;

	switch (option_vol_puts_buffer) {
		case 1:json = option_vol_puts_json2;using_buffer = 2;break;
		case 2:json = option_vol_puts_json; using_buffer = 1;break;
	}
	jptr = json; *jptr  = '['; nbytes = 1;

	for (x=0; x<nr_options_vol_puts; x++) {
		opstock = options_vol_puts[x];
		if (!opstock)
			continue;
		option = opstock->option;
		nbytes += snprintf(jptr+nbytes, 8191, "{\"oN\":\"%s\",\"oP\":\"%.2f\",\"oD\":\"%.2f\",\"oV\":\"%d\",\"oI\":\"%d\"},",
		option->contract, option->lastPrice, option->percentChange, option->volume, option->openInterest);
	}
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;

	switch (using_buffer) {
		case 1:option_vol_puts_buffer   = 1;option_vol_puts_json_len  = nbytes;break;
		case 2:option_vol_puts_buffer   = 2;option_vol_puts_json_len2 = nbytes;break;
	}
}