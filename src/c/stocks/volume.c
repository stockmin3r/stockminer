#include <conf.h>
#include <curl/curl.h>

int get_int(char *str)
{
	char num[12] = {0};
	int len = 0;

	while (*str != '<') {
		if (*str == ',') {
			str++;
			continue;
		}
		num[len++] = *str++;
		if (len > 10) {
			printf(BOLDRED "get_int: CRITICAL LEN ERROR: %.50s len: %d" RESET "\n", str-8, len);
			return 0;
		}
	}
	return atoi(num);
}


void update_premarket_volume(struct stock *stock)
{
	CURL *curl;
	struct curldata cdata;
	struct curl_slist *headers = NULL;
	char url[256];
	char page[1200 KB];
	char *p;
	int page_len;

	cdata.memory = (char *)malloc(32);
	cdata.size   = 0;
	cdata.stock  = stock;

	headers = curl_slist_append(headers, "Accept-Encoding: gzip");
	snprintf(url, sizeof(url)-1, "https://www.wsj.com/market-data/quotes/%s", stock->sym);

	curl    = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_get_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cdata);
	curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	curl_easy_perform(curl);

	page_len = *(unsigned int *)(cdata.memory+cdata.size-4);
	if (page_len > 1200 KB)
		goto out;

	page_len = zip_decompress(cdata.memory, page, cdata.size);
	if (page_len <= 0)
		goto out;
	page[page_len] = 0;
//	printf("%s\n", page);
	p = strstr(page, "PRE MARKET Vol");
	if (!p)
		goto out;
	p = strchr(p, '>');
	if (!p)
		goto out;
	stock->premarket_volume = get_int(p+1);
//	printf("premarket volume: %d\n", stock->premarket_volume);
out:
	free(cdata.memory);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
}

void update_volumes(int idx, struct XLS *XLS)
{
	struct board *boards[] = { XLS->boards[LOWCAP_GAINER_BOARD], XLS->boards[LOWCAP_LOSER_BOARD], XLS->boards[HIGHCAP_LOSER_BOARD], XLS->boards[HIGHCAP_LOSER_BOARD] };
	struct board *board;
	struct stock *stock;
	int x, y, retries = 0;

	board = boards[idx];
	for (y=0; y<NR_DELTA_STOCKS; y++) {
		stock = board->stocks[y];
		if (!stock)
			continue;
		retries = 0;
		while (retries++ < 3) {
			update_premarket_volume(stock);
			if (!stock->premarket_volume) {
				retries += 1;
				continue;
			}
			break;
		}
	}
}

char *volume(uint64_t vol, char *buf)
{
	char vstr[32];

	if (vol == 0) {
		buf[0] = '0';
		buf[1] = 0;
		return buf;
	}
	cstring_itoa(vstr, vol);
	if (vol >= 1 && vol < 100) {
		buf[0] = vstr[0];
		buf[1] = 0;
		return buf;
	}
	if (vol >= 100 && vol < 1000) {
		buf[0] = vstr[0];
		buf[1] = vstr[1];
		buf[2] = vstr[2];
		buf[3] = 0;
		return buf;
	} else if (vol >= 1000 && vol < 10000) {
		buf[0] = vstr[0];
		if (vstr[1] == '0') {
			buf[1] = 'K';
			buf[2] = 0;
			return (buf);
		}
		buf[1] = '.';
		buf[2] = vstr[1];
		buf[3] = 'K';
		buf[4] = 0;
		return (buf);
	} else if (vol >= 10000 && vol < 100000) {
		buf[0] = vstr[0];
		buf[1] = vstr[1];
		buf[2] = '.';
		buf[3] = vstr[2];
		buf[4] = 'K';
		buf[5] = 0;
		return (buf);
	} else if (vol >= 100000 && vol < 1000000) {
		buf[0] = vstr[0];
		buf[1] = vstr[1];
		buf[2] = vstr[2];
		buf[3] = 'K';
		buf[4] = 0;
		return (buf);
	} else if (vol >= 1000000 && vol < 10000000) {
		buf[0] = vstr[0];
		buf[1] = '.';
		buf[2] = vstr[1];
		buf[3] = vstr[2];
		buf[4] = 'M';
		buf[5] = 0;
		return (buf);
	} else if (vol >= 10000000 && vol < 100000000) {
		buf[0] = vstr[0];
		buf[1] = vstr[1];
		buf[2] = '.';
		buf[3] = vstr[2];
		buf[4] = 'M';
		buf[5] = 0;
		return (buf);
	} else if (vol >= 100000000 && vol < 1000000000) {
		buf[0] = vstr[0];
		buf[1] = vstr[1];
		buf[2] = vstr[2];
		buf[3] = '.';
		buf[4] = vstr[3];
		buf[5] = 'M';
		buf[6] = 0;
		return (buf);
	} else if (vol >= 1000000000 && vol < 10000000000) {
		buf[0] = vstr[0];
		buf[1] = '.';
		buf[2] = vstr[1];
		buf[3] = vstr[2];
		buf[4] = vstr[3];
		buf[5] = 'B';
		buf[6] = 0;
		return (buf);
	} else if (vol >= 10000000000 && vol < 100000000000) {
		buf[0] = vstr[0];
		buf[1] = vstr[1];
		buf[2] = '.';
		buf[3] = vstr[2];
		buf[4] = vstr[3];
		buf[5] = 'B';
		buf[6] = 0;
		return (buf);
	} else {
		buf[0] = vstr[0];
		buf[1] = 0;
		return (buf);
	}
}
