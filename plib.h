/*
 *  ibex - pseudo-library
 *
 *  Copyright (c) 2010, 2014-2016 xerub
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PLIB_H_included
#define PLIB_H_included

#include <stdarg.h>
#include <stddef.h>
#include <offsets.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define AES_ENCRYPT		0x10
#define AES_DECRYPT		0x11

#define KBAG_IV_SIZE		0x10
#define KBAG_KEY_SIZE		0x20
#define KBAG_KEY_IV_SIZE	(KBAG_IV_SIZE + KBAG_KEY_SIZE)

#define SHSH_KEY		0x100
#define GID_KEY			0x20000200
#define UID_KEY			0x20000201

typedef struct CmdArg {
    signed long integer;	/* strtol(str, 0, 0) */
    unsigned long uinteger;	/* strtoul(str, 0, 0) */
    signed long inthex;		/* strtol(str, 0, 16) */
    unsigned char boolean;	/* evaluated from "true", "false" or CmdArg.integer */
    char *string __attribute__((aligned(8)));
} CmdArg;

#ifndef TARGET_BASEADDR
extern unsigned long TARGET_BASEADDR;
#endif
#ifndef TARGET_LOADADDR
extern unsigned long TARGET_LOADADDR;
#endif
#ifndef TARGET_JUMPADDR
extern unsigned long TARGET_JUMPADDR;
#endif

typedef int (*printf_t)(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
typedef int (*snprintf_t)(char *buf, size_t max, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

typedef int (*aes_crypto_cmd_t)(int op, void *inbuf, void *outbuf, unsigned len, unsigned aes, char *key, char *iv);
extern aes_crypto_cmd_t aes_crypto_cmd_;

typedef int (*create_envvar_t)(const char *var, const char *val, int wtf);
extern create_envvar_t create_envvar_;

/* asm stuff */

int _printf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
int _snprintf(char *str, size_t size, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
void flush_cache(void);
void disable_int(void);

/* our stuff */

int link(void *);

/* misc C stuff */

unsigned div(unsigned N, unsigned D);

unsigned char *
boyermoore_horspool_memmem(const unsigned char* haystack, size_t hlen,
                           const unsigned char* needle,   size_t nlen);

int xtoi(const char *hptr);
long xtol(const char *hptr);

/* standard C stuff */

int atoi(const char *nptr);

int strcmp(const char *s1, const char *s2);
int memcmp(const void *b1, const void *b2, size_t len);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmem(const void* haystack, size_t hlen, const void* needle, size_t nlen);

#endif
