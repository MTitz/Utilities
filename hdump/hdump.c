/* File: hdump.c */

/* Version 1.01, Martin Titz, 1996, 1997, 2000 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef BUFSIZ
#if BUFSIZ>0 && BUFSIZ%16==0
#define BUFFER_SIZE BUFSIZ
#endif
#endif

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif

#if BUFFER_SIZE%16!=0
#error BUFFER_SIZE must be a multiple of 16
#endif

#define COL_HEX 12
#define COL_ASC 63
#define COLS    (COL_ASC + 16)

static const unsigned char lc[16] = "0123456789abcdef";
static const unsigned char uc[16] = "0123456789ABCDEF";
static const unsigned char *hc  = lc;
static int eightbit             = 0;
static int error_count          = 0;
static int verbose              = 0;
static unsigned long begin      = 0;
static unsigned long end        = ULONG_MAX;
static unsigned char c          = '.';
static unsigned char block[BUFFER_SIZE];
#ifdef __unix__
static char *progname;
#else
static const char *const progname = "hdump";
#endif

static void usage(void)
{
    fprintf(stderr, "usage: %s [-b begin[bkm]] [-c char] [-e end[bkm]] [-l]"
            " [-n count[bkm]] [-u]\n             [-v] [-7] [-8] [file]...\n",
            progname);
}

static void arg_err(char c)
{
    fprintf(stderr, "%s: option -%c requires an argument.\n", progname, c);
    usage();
    exit(EXIT_FAILURE);
}

static unsigned long get_ulong_mod16(const char *s)
{
    unsigned long base, n;
    char ch;
    n = 0;
    if (*s == '0') {
        ++s;
        if (*s == 'x' || *s == 'X') {
            ++s;
            base = 16;
        } else
            base = 8;
    } else
        base = 10;
    while (ch = *s++) {
        if (ch < '0')
            break;
        if (base == 8 && ch <= '7' || base >= 10 && ch <= '9')
            ch -= '0';
        else if (base == 16) {
            if (ch >= 'a' && ch <= 'f')
                ch -= 'a'-10;
            else if (ch >= 'A' && ch <= 'F')
                ch -= 'A'-10;
            else
                break;
        } else
            break;
        n = base * n + ch;
    }
    --s;
    if (*s == 'b' || *s == 'B') {
        n <<= 5;
        ++s;
    } else if (*s == 'k' || *s == 'K') {
        n <<= 6;
        ++s;
    } else if (*s == 'm' || *s == 'M') {
        n <<= 16;
        ++s;
    } else
        n >>= 4;
    if (*s) {
        fprintf(stderr, "%s: garbage after end of number.\n", progname);
        usage();
        exit(EXIT_FAILURE);
    }
    return n;
}

static void tohex(unsigned long n, int digits, unsigned char *out)
{
    while (--digits >= 0) {
        out[digits] = hc[n & 0xf];
        n >>= 4;
    }
}

static void do_handle(FILE *f)
{
    size_t bytes_read, line;
    unsigned long count;
    unsigned char s[COLS+1];

    memset(s, ' ', COLS);
    s[4] = ':';
    s[8] = '0';
    s[COLS] = '\0';
    if (begin > 0 && fseek(f, begin << 4, SEEK_SET) != 0) {
        fprintf(stderr, "%s: fseek error (%s)\n", progname, strerror(errno));
        ++error_count;
        return;
    }
    for (count = begin; bytes_read = fread(block, 1, BUFFER_SIZE, f); ) {
        for (line = 0; line < bytes_read; line += 16, ++count) {
            size_t i, end_pos;
            unsigned char *t;
            if (count > end)
                return;
            end_pos = 16;
            if (bytes_read-line < 16) {
                end_pos = bytes_read - line;
                memset(s+COL_HEX, ' ', COLS-COL_HEX);
            }
            for (i = 0, t = s+COL_HEX; i < end_pos; ++i, t += 2) {
                unsigned char b = block[line+i];
                *t = hc[b >> 4];
                *++t = hc[b & 0xf];
                if (i == 7)
                    ++t;
                s[COL_ASC+i] = b>=32 && b<127 || eightbit && b>=128 ? b : c;
            }
            tohex(count >> 12,   4, s);
            tohex(count & 0xfff, 3, s+5);
            puts((const char*)s);
        }
        if (bytes_read < BUFFER_SIZE)
            break;
    }
    if (ferror(f)) {
        fprintf(stderr, "%s: read error (%s)\n", progname, strerror(errno));
        ++error_count;
    }
}

static void do_file(const char *fname)
{
    FILE *f;

    if ((f = fopen(fname, "rb")) == 0) {
        fprintf(stderr, "%s: can't open %s (%s)\n",
                progname, fname, strerror(errno));
        ++error_count;
        return;
    }
    if (verbose)
        printf("\nFile %s:\n\n", fname);
    do_handle(f);
    fclose(f);
}

int main(int argc, char *argv[])
{
    int do_opts = 1;
    int files_done = 0;
#ifdef __unix__
    char *ptmp;
    progname = argv[0];
    if (ptmp = strrchr(progname, '/'))
        progname = ptmp + 1;
#endif
    for (--argc, ++argv;  argc > 0;  --argc, ++argv) {
        if (**argv == '-' && do_opts) {
            if (argv[0][1] == 0) {
                if (files_done)
                    puts("");
                if (verbose)
                    puts("\nstdin:\n");
                do_handle(stdin);
                ++files_done;
                continue;
            }
            while (*++*argv) {
                switch (**argv) {
                    case 'b':
                        if (*++*argv || --argc && *++argv) {
                            begin = get_ulong_mod16(*argv);
                        } else
                            arg_err('b');
                        goto nextarg;
                    case 'c':
                        if (*++*argv || --argc && *++argv)
                            c = **argv;
                        if (*++*argv)
                            arg_err('c');
                        goto nextarg;
                    case 'e':
                        if (*++*argv || --argc && *++argv) {
                            end = get_ulong_mod16(*argv);
                        } else
                            arg_err('e');
                        goto nextarg;
                    case 'h':
                    case '?':
                        usage();
                        return EXIT_SUCCESS;
                    case 'l':
                        hc = lc;
                        break;
                    case 'n':
                        if (*++*argv || --argc && *++argv) {
                            end = begin + get_ulong_mod16(*argv);
                        } else
                            arg_err('n');
                        goto nextarg;
                    case 'u':
                        hc = uc;
                        break;
                    case 'v':
                        ++verbose;
                        break;
                    case '7':
                        eightbit = 0;
                        break;
                    case '8':
                        eightbit = 1;
                        break;
                    case '-':
                        do_opts = 0;
                        break;
                    default:
                        fprintf(stderr, "%s: unknown command line flag '%c'.\n",
                                progname, **argv);
                        usage();
                        return EXIT_FAILURE;
                }
            }
        } else {
            if (files_done)
                puts("");
            do_file(*argv);
            ++files_done;
        }
        nextarg: ;
    }
    if (!files_done)
        do_handle(stdin);
    return error_count ? EXIT_FAILURE : EXIT_SUCCESS;
}
