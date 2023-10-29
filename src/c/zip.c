#include <stdinc.h>
#include <zlib.h>

int zip_deflate(char *page, char *page_gz, int page_len)
{
	z_stream dstream;
	int inpos = 0, outpos = 0, nleft = page_len, chunk, ret;

	dstream.zalloc    = Z_NULL;
	dstream.zfree     = Z_NULL;
	dstream.opaque    = Z_NULL;
	dstream.next_in   = (Bytef *)page;

	deflateInit2(&dstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
	do {
		dstream.avail_in = nleft;
		dstream.next_in  = (unsigned char *)page+inpos;
		do {
			dstream.avail_out = nleft;
			dstream.next_out  = (unsigned char *)page_gz+outpos;
			ret = deflate(&dstream, Z_FINISH);
			if (ret == Z_STREAM_ERROR)
				return -1;
			outpos += (nleft-dstream.avail_out);
			nleft  -= dstream.avail_out;
			inpos  += dstream.avail_out;
		} while (dstream.avail_out == 0);
	} while (nleft > 0);
	deflateEnd(&dstream);
	return ((char *)dstream.next_out - page_gz);
}

int zip_compress(char *page, char *page_gz, int page_len)
{
	z_stream dstream;
	int inpos = 0, outpos = 0, nleft = page_len, chunk, ret;

	dstream.zalloc    = Z_NULL;
	dstream.zfree     = Z_NULL;
	dstream.opaque    = Z_NULL;
	dstream.next_in   = (Bytef *)page;

	deflateInit2(&dstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16+15, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
	do {
		dstream.avail_in = nleft;
		dstream.next_in  = (unsigned char *)page+inpos;
		do {
			dstream.avail_out = nleft;
			dstream.next_out  = (unsigned char *)page_gz+outpos;
			ret = deflate(&dstream, Z_FINISH);
			if (ret == Z_STREAM_ERROR)
				return -1;
			outpos += (nleft-dstream.avail_out);
			nleft  -= dstream.avail_out;
			inpos  += dstream.avail_out;
		} while (dstream.avail_out == 0);
	} while (nleft > 0);
	deflateEnd(&dstream);
	return ((char *)dstream.next_out - page_gz);
}

uint64_t zip_decompress2(unsigned char *compressed, unsigned char *uncompressed, int compressed_size, int uncompressed_size)
{
	z_stream istream;
	uint64_t total_out = 0;

	istream.zalloc    = Z_NULL;
	istream.zfree     = Z_NULL;
	istream.opaque    = Z_NULL;

	istream.next_in   = compressed;        // input  buf
	istream.next_out  = uncompressed;      // output buf
	istream.avail_in  = compressed_size;   // input  size
	istream.avail_out = uncompressed_size; // output size

	inflateInit2(&istream, 15+16);
	inflate(&istream, Z_FINISH);
	total_out = istream.total_out;
	inflateEnd(&istream);
	if (total_out != uncompressed_size)
		return 0;
	return (total_out);
}

int zip_decompress(char *page_gz, char *page, int gz_len)
{
	z_stream istream;
	int inpos = 0, outpos = 0, nleft = 1024*1024, chunk, ret;

	istream.zalloc    = Z_NULL;
	istream.zfree     = Z_NULL;
	istream.opaque    = Z_NULL;
	istream.next_in   = (Bytef *)page_gz;  // input char array

	istream.avail_in  = 0;
	inflateInit2(&istream, 15+16);
	do {
		istream.avail_in = nleft;
		istream.next_in  = (unsigned char *)page_gz+inpos;
		do {
			istream.avail_out = nleft;
			istream.next_out  = (unsigned char *)page+outpos;
			ret = inflate(&istream, Z_NO_FLUSH);
			switch (ret) {
				case Z_NEED_DICT:
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					inflateEnd(&istream);
					return -1;
			}
			outpos += (nleft-istream.avail_out);
			nleft  -= istream.avail_out;
			inpos  += istream.avail_out;
			istream.avail_in = nleft;
			istream.next_in  = (unsigned char *)page_gz+nleft;
		} while (istream.avail_out == 0);
	} while (nleft > 0);
	inflateEnd(&istream);
	return (outpos);
}
