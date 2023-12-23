#include <stdinc.h>
#include <conf.h>
#include <extern.h>
#include <curl/curl.h>

size_t curl_get_data(char *buf, size_t size, size_t count, void *data)
{
	struct curldata *cdata = (struct curldata *)data;
	char *ptr;
	size_t realsize = size*count;

	ptr = (char *)realloc(cdata->memory, cdata->size + realsize + 1);
	if (!ptr)
		return 0;
	cdata->memory = ptr;
	memcpy(&(cdata->memory[cdata->size]), buf, realsize);
	cdata->size += realsize;
	cdata->memory[cdata->size] = 0;
	return (realsize);
}

char *curl_get(char *url, char *page)
{
	CURL              *curl;
	struct curldata    cdata;
	struct curl_slist *headers = NULL;
	int                page_len, uncompressed_size;

	cdata.memory = (char *)malloc(1000 KB);
	cdata.size   = 0;
	if (!cdata.memory)
		return NULL;

	headers = curl_slist_append(headers, "Accept-Encoding: gzip");
	headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:77.0) Gecko/20100101 Firefox/77.0");
	if (strstr(url, "api.wsj.net")) {
		headers = curl_slist_append(headers, "Dylan2010.EntitlementToken: 57494d5ed7ad44af85bc59a51dd87c90");
	}
	curl    = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_get_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cdata);
	curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_perform(curl);
//	if (page_len > 3000 KB)
//		goto out;
	uncompressed_size = *(unsigned int *)(cdata.memory+cdata.size-4);
	if (uncompressed_size < 0)
		return NULL;

	page_len = zip_decompress2((unsigned char *)cdata.memory, (unsigned char *)page, cdata.size, uncompressed_size);
	if (page_len <= 0)
		goto out;
	page[page_len] = 0;
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	free(cdata.memory);
	return (page);
out:
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	free(cdata.memory);
//	printf("url failed: %s %.50s\n", url, page);
	return (NULL);
}

char *convert_urlencoded(char *buf)
{
	char *p, *sp;
	int len = strlen(buf);
	int newlen = len, moved = 0;
	short code;

	sp = buf;
	while ((p=strchr(sp, '+'))) {
		*p++ = ' ';
		sp   = p;
	}

	sp = buf;
	while ((p=strchr(sp, '%'))) {
		code = *(short *)(p+1);
		switch (code) {
            case 0x3034:
                *p++ = '@';
                memmove(p, p+2, len-(p+1-buf));
                newlen -= 2;
                break;
			case 0x3132:
				*p++ = '!';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x4633:
				*p++ = '?';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x3432:
				*p++ = '$';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x3632:
				*p++ = '&';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x3532:
				*p++ = '%';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x3332:
				*p++ = '#';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x4232:
				*p++ = '+';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x4432:
				*p++ = '-';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x6532:
				*p++ = '.';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x4332:
				*p++ = ',';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x4132:
				*p++ = '*';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x3832:
				*p++ = '(';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x3932:
				*p++ = ')';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x3232:
				*p++ = '"';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			case 0x3732:
				*p++ = '\'';
				memmove(p, p+2, len-(p+1-buf));
				newlen -= 2;
				moved = 1;
				break;
			default:
				p += 1;
				break;
		}
		sp = p;
	}
	buf[newlen] = 0;
	return buf;
}

int http_get_boundary(char *req, char *cookie, char **bptr)
{
	char *boundary, *p;
	int blen;

	/* Boundary */
	boundary = strstr(req, "boundary");
	if (!boundary) {
		boundary = strstr(cookie+25, "boundary");
		if (!boundary)
			return 0;
	}
	boundary += 9;
	*bptr = boundary;
	p = strchr(boundary, '\r');
	if (!p)
		return 0;
	*p++ = 0;
	blen = (p-boundary);
	return (blen);
}
