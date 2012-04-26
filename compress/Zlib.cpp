// Zlib.cpp

#include "framework/Framework.h"
#include "framework/compress/Zlib.h"
#include "framework/compress/zlib/zlib.h"
#include "framework/compress/zlib/zutil.h"

#include <stdio.h>

namespace framework
{
    namespace compress
    {
        struct internal_state {int dummy;}; /* for buggy compilers */

#ifndef Z_BUFSIZE
#  ifdef MAXSEG_64K
#    define Z_BUFSIZE 4096 /* minimize memory usage for 16-bit DOS */
#  else
#    define Z_BUFSIZE 16384
#  endif
#endif
#ifndef Z_PRINTF_BUFSIZE
#  define Z_PRINTF_BUFSIZE 4096
#endif

#define ALLOC(size) malloc(size)
#define TRYFREE(p) {if (p) free(p);}

        static int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

        /* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

        typedef struct gz_stream
        {
            z_stream stream;
            int      z_err;   /* error code for last stream operation */
            int      z_eof;   /* set if end of input file */
            FILE     *file;   /* .gz file */
            Byte     *inbuf;  /* input buffer */
            Byte     *outbuf; /* output buffer */
            uLong    crc;     /* crc32 of uncompressed data */
            char     *msg;    /* error message */
            char     *path;   /* path name for debugging only */
            int      transparent; /* 1 if input file is not a .gz file */
            char     mode;    /* 'w' or 'r' */
            long     startpos; /* start of compressed data in file (header skipped) */
        } gz_stream;

        static int get_stream_byte(gz_stream *s)
        {
            if (s->z_eof) return EOF;
            s->stream.avail_in--;
            return *(s->stream.next_in)++;
        }

        static void check_compressed_header(gz_stream *s)
        {
            int method; /* method byte */
            int flags;  /* flags byte */
            uInt len;
            int c;

            /* Check the gzip magic header */
            for (len = 0; len < 2; len++) {
                c = get_stream_byte(s);
                if (c != gz_magic[len]) {
                    if (len != 0) s->stream.avail_in++, s->stream.next_in--;
                    if (c != EOF) {
                        s->stream.avail_in++, s->stream.next_in--;
                        s->transparent = 1;
                    }
                    s->z_err = s->stream.avail_in != 0 ? Z_OK : Z_STREAM_END;
                    return;
                }
            }
            method = get_stream_byte(s);
            flags = get_stream_byte(s);
            if (method != Z_DEFLATED || (flags & RESERVED) != 0) {
                s->z_err = Z_DATA_ERROR;
                return;
            }

            /* Discard time, xflags and OS code: */
            for (len = 0; len < 6; len++) (void)get_stream_byte(s);

            if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
                len  =  (uInt)get_stream_byte(s);
                len += ((uInt)get_stream_byte(s))<<8;
                /* len is garbage if EOF but the loop below will quit anyway */
                while (len-- != 0 && get_stream_byte(s) != EOF) ;
            }
            if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
                while ((c = get_stream_byte(s)) != 0 && c != EOF) ;
            }
            if ((flags & COMMENT) != 0) {   /* skip the .gz file comment */
                while ((c = get_stream_byte(s)) != 0 && c != EOF) ;
            }
            if ((flags & HEAD_CRC) != 0) {  /* skip the header crc */
                for (len = 0; len < 2; len++) (void)get_stream_byte(s);
            }
            s->z_err = s->z_eof ? Z_DATA_ERROR : Z_OK;
        }

        static int destroy_handle (gz_stream *s)
        {
            int err = Z_OK;

            if (!s) return Z_STREAM_ERROR;

            TRYFREE(s->msg);

            if (s->stream.state != NULL) {
                err = inflateEnd(&(s->stream));
            }
            if (s->file != NULL && fclose(s->file)) {
#ifdef ESPIPE
                if (errno != ESPIPE) /* fclose is broken for pipes in HP/UX */
#endif
                    err = Z_ERRNO;
            }
            if (s->z_err < 0) err = s->z_err;

            TRYFREE(s->path);
            TRYFREE(s);
            return err;
        }

        static uLong get_Long (gz_stream *s)
        {
            uLong x = (uLong)get_stream_byte(s);
            int c;

            x += ((uLong)get_stream_byte(s))<<8;
            x += ((uLong)get_stream_byte(s))<<16;
            c = get_stream_byte(s);
            if (c == EOF) s->z_err = Z_DATA_ERROR;
            x += ((uLong)c)<<24;
            return x;
        }

        int ZLib::gzunzip(unsigned char * compressed, size_t com_len, unsigned char *uncompressed, size_t *uncom_len)
        {
            int err;
            gz_stream *s = NULL;

            s = (gz_stream *)ALLOC(sizeof(gz_stream));
            if (!s) return Z_NULL;

            s->stream.zalloc = (alloc_func)0;
            s->stream.zfree = (free_func)0;
            s->stream.opaque = (voidpf)0;
            s->stream.next_in = s->inbuf = compressed;
            s->stream.next_out = s->outbuf = uncompressed;
            s->stream.avail_in = com_len;
            s->stream.avail_out = *uncom_len;
            s->file = NULL;
            s->z_err = Z_OK;
            s->z_eof = 0;
            s->crc = crc32(0L, Z_NULL, 0);
            s->msg = NULL;
            s->transparent = 0;

            s->path = NULL;

            s->mode = 'r';

            err = inflateInit2(&(s->stream), -MAX_WBITS);
            if (err != Z_OK || s->inbuf == Z_NULL) {
                return destroy_handle(s), Z_NULL;
            }

            check_compressed_header(s); /* skip the .gz header */
            s->startpos = (com_len - s->stream.avail_in);

            Bytef *start = s->stream.next_out;

            while (s->stream.avail_out != 0) {
                s->z_err = inflate(&(s->stream), Z_NO_FLUSH);

                if (s->z_err == Z_STREAM_END) {
                    /* Check CRC and original size */
                    s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));
                    start = s->stream.next_out;

                    if (get_Long(s) != s->crc) {
                        s->z_err = Z_DATA_ERROR;
                    } else {
                        (void)get_Long(s);
                        check_compressed_header(s);
                        if (s->z_err == Z_OK) {
                            uLong total_in = s->stream.total_in;
                            uLong total_out = s->stream.total_out;

                            inflateReset(&(s->stream));
                            s->stream.total_in = total_in;
                            s->stream.total_out = total_out;
                            s->crc = crc32(0L, Z_NULL, 0);
                        }
                    }
                }
                if (s->z_err != Z_OK || s->z_eof) break;
            }
            s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));

            int ret = (int)(*uncom_len - s->stream.avail_out);
            *uncom_len = s->stream.avail_out;

            destroy_handle(s);
            return ret;
        }

    }// namespace compress
}// namespace framework