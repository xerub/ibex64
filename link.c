/*
 *  ibex - link external symbols
 *
 *  Copyright (c) 2010, 2015-2016 xerub
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


#include <stdint.h>
#include "plib.h"

#ifndef TARGET_BASEADDR
unsigned long TARGET_BASEADDR;
#endif
#ifndef TARGET_LOADADDR
unsigned long TARGET_LOADADDR;
#endif
#ifndef TARGET_JUMPADDR
unsigned long TARGET_JUMPADDR;
#endif

static printf_t printf_
#ifdef TARGET_PRINTF
    = (printf_t)(TARGET_PRINTF
#ifdef TARGET_BASEADDR
    + TARGET_BASEADDR
#endif
    )
#endif
;

printf_t get_printf_addr(void) { return printf_; }

#define IBOOT_LEN 0x50000
#define MAYBE_UNUSED __attribute__((unused)) static

typedef unsigned long long addr_t;

#define INSN_CALL 0x94000000, 0xFC000000

static addr_t
step64(const uint8_t *buf, addr_t start, size_t length, uint32_t what, uint32_t mask)
{
    addr_t end = start + length;
    while (start < end) {
        uint32_t x = *(uint32_t *)(buf + start);
        if ((x & mask) == what) {
            return start;
        }
        start += 4;
    }
    return 0;
}

static addr_t
bof64(const uint8_t *buf, addr_t start, addr_t where)
{
    for (; where >= start; where -= 4) {
        uint32_t op = *(uint32_t *)(buf + where);
        if ((op & 0xFFC003FF) == 0x910003FD) {
            unsigned delta = (op >> 10) & 0xFFF;
            if ((delta & 0xF) == 0) {
                addr_t prev = where - ((delta >> 4) + 1) * 4;
                uint32_t au = *(uint32_t *)(buf + prev);
                if ((au & 0xFFC003E0) == 0xA98003E0) {
                    return prev;
                }
            }
        }
    }
    return 0;
}

static addr_t
xref64(const uint8_t *buf, addr_t start, addr_t end, addr_t what)
{
    addr_t i;
    uint64_t value[32];

    memset(value, 0, sizeof(value));

    end &= ~3;
    for (i = start & ~3; i < end; i += 4) {
        uint32_t op = *(uint32_t *)(buf + i);
        unsigned reg = op & 0x1F;
        if ((op & 0x9F000000) == 0x90000000) {
            signed adr = ((op & 0x60000000) >> 18) | ((op & 0xFFFFE0) << 8);
            value[reg] = ((long long)adr << 1) + (i & ~0xFFF);
        } else if ((op & 0xFF000000) == 0x91000000) {
            unsigned rn = (op >> 5) & 0x1F;
            unsigned shift = (op >> 22) & 3;
            unsigned imm = (op >> 10) & 0xFFF;
            if (shift == 1) {
                imm <<= 12;
            }
            value[reg] = value[rn] + imm;
        } else if ((op & 0xF9C00000) == 0xF9400000) {
            unsigned rn = (op >> 5) & 0x1F;
            unsigned imm = ((op >> 10) & 0xFFF) << 3;
            value[reg] = value[rn] + imm;	/* XXX address, not actual value */
        } else if ((op & 0x9F000000) == 0x10000000) {
            signed adr = ((op & 0x60000000) >> 18) | ((op & 0xFFFFE0) << 8);
            value[reg] = ((long long)adr >> 11) + i;
        } else if ((op & 0xFF000000) == 0x58000000) {
            unsigned adr = (op & 0xFFFFE0) >> 3;
            value[reg] = adr + i;		/* XXX address, not actual value */
        }
        if (value[reg] == what) {
            return i;
        }
    }
    return 0;
}

static addr_t
follow_call64(const uint8_t *buf, addr_t call)
{
    long long w;
    w = *(uint32_t *)(buf + call) & 0x3FFFFFF;
    w <<= 64 - 26;
    w >>= 64 - 26 - 2;
    return call + w;
}

MAYBE_UNUSED addr_t
find_xref(const char *pattern, size_t patlen)
{
    const unsigned char *str = boyermoore_horspool_memmem((void *)TARGET_BASEADDR, IBOOT_LEN, (void *)pattern, patlen);
    addr_t ref = 0;
    if (str) {
        ref = xref64((void *)TARGET_BASEADDR, 0, IBOOT_LEN, (addr_t)(str - TARGET_BASEADDR));
    }
    return ref;
}

MAYBE_UNUSED addr_t
find_easy(const char *pattern, size_t patlen)
{
    addr_t bof;
    addr_t mm = find_xref(pattern, patlen);
    if (!mm) {
        return 0;
    }
    bof = bof64((void *)TARGET_BASEADDR, 0, mm);
    if (bof) {
        bof += TARGET_BASEADDR;
    }
    return bof;
}

MAYBE_UNUSED printf_t
find_printf(void)
{
    addr_t call;
    addr_t ref = find_xref("jumping into image at", sizeof("jumping into image at") - 1);
    if (!ref) {
        return NULL;
    }
    call = step64((void *)TARGET_BASEADDR, ref, 16, INSN_CALL);
    if (!call) {
        return NULL;
    }
    return (printf_t)(TARGET_BASEADDR + follow_call64((void *)TARGET_BASEADDR, call));
}

MAYBE_UNUSED aes_crypto_cmd_t
find_aes_crypto_cmd(void)
{
    return (aes_crypto_cmd_t)find_easy("aes_crypto_cmd", sizeof("aes_crypto_cmd") - 1);
}

MAYBE_UNUSED int
nullsub()
{
    return 0;
}

MAYBE_UNUSED int
stub_aes_crypto_cmd(int crypt_type, void *inbuf, void *outbuf, unsigned int inbuf_len, unsigned int aes_key_type, char *iv, char *key)
{
    aes_crypto_cmd_t p = find_aes_crypto_cmd();
    if (p) {
        aes_crypto_cmd_ = p;
        return aes_crypto_cmd_(crypt_type, inbuf, outbuf, inbuf_len, aes_key_type, iv, key);
    }
    printf_("unresolved aes_crypto_cmd\n");
    return -1;
}

int
link(void *caller)
{
    static int virgin = 1;
    extern unsigned char _start[];
    extern unsigned char __bss_start[];
    extern unsigned char _end[];
    if (virgin) {
        virgin = 0;
        memset(__bss_start, 0, _end - __bss_start);

#ifndef TARGET_BASEADDR
        unsigned int *p;
        for (p = caller; !TARGET_BASEADDR; p--) {
            if (*p == 0) {
                int i;
                for (i = -55; i < 0; i++) {
                    if (p[i]) {
                        p += i;
                        break;
                    }
                }
                if (i == 0) {
                    p = (unsigned int *)((uintptr_t)p & ~0xFFF);
                    if (memmem(p + 128, 0x80, ", Apple Inc.", sizeof(", Apple Inc.") - 1)) {
                        TARGET_BASEADDR = (uintptr_t)p;
                    }
                }
            }
        }
#else
        (void)caller;
#endif
#ifndef TARGET_LOADADDR
        TARGET_LOADADDR = (uintptr_t)_start;
#endif
#ifndef TARGET_JUMPADDR
        TARGET_JUMPADDR = TARGET_LOADADDR + 0x4000000;
#endif

#ifndef TARGET_PRINTF
        printf_ = find_printf();
        if (!printf_) {
            printf_ = (printf_t)nullsub;
        }
#elif !defined(TARGET_BASEADDR)
        printf_ = (printf_t)(TARGET_BASEADDR + TARGET_PRINTF);
#endif

#ifndef TARGET_AES_CRYPTO_CMD
        aes_crypto_cmd_ = stub_aes_crypto_cmd;
#elif !defined(TARGET_BASEADDR)
        aes_crypto_cmd_ = (aes_crypto_cmd_t)(TARGET_BASEADDR + TARGET_AES_CRYPTO_CMD);
#endif
    }
    return 0;
}

#ifndef TARGET_BASEADDR
#define TARGET_BASEADDR 0
#endif

aes_crypto_cmd_t aes_crypto_cmd_
#ifdef TARGET_AES_CRYPTO_CMD
    = (aes_crypto_cmd_t)(TARGET_BASEADDR + TARGET_AES_CRYPTO_CMD)
#endif
;
