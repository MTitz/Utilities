/* File: joinlines.c */

/* Version 0.1.1, Martin Titz, 2013 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 16777216
#endif

static int verbose = 0;
static int error_count = 0;
static unsigned char buffer[BUFFER_SIZE];
#ifdef __unix__
static char *progname;
#else
static const char *const progname = "joinlines";
#endif

static void usage(void)
{
    fprintf(stderr, "usage: %s [-v] [file]...\n", progname);
}

#ifdef UNUSED
static void arg_err(char c)
{
    fprintf(stderr, "%s: option -%c requires an argument.\n", progname, c);
    usage();
    exit(EXIT_FAILURE);
}
#endif


static void join_lines(FILE *f, size_t n)
{
    size_t i;
    int lfCount = 0;
    int crCount = 0;
    int nlCount = 0;
    for (i = 0; i < n; ++i) {
        unsigned char ch = buffer[i];
        if (ch == '\n') {
            ++lfCount;
        } else if (ch == '\r') {
            ++crCount;
        } else {
            if (lfCount > 0 && crCount > 0) {
                nlCount = lfCount;
            } else if (lfCount > 0) {
                nlCount = lfCount;
            } else if (crCount > 0) {
                nlCount = crCount;
            } else {
                nlCount = 0;
            }
            if (nlCount > 0) {
                if (nlCount == 1) {
                    fputc(' ', f);
                } else {
                    int j;
                    for (j = 0; j < nlCount; ++j) {
                        fputc('\n', f);
                    }
                }
                lfCount = 0;
                crCount = 0;
                nlCount = 0;
            }
            fputc(ch, f);
        }
    }
}

static void do_handle(FILE *f)
{
    size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, f);
    if (ferror(f)) {
        fprintf(stderr, "%s: read error (%s)\n", progname, strerror(errno));
        ++error_count;
    } else if (feof(f)) {
        /* Process bytes_read bytes stored in the buffer */
        fseek(f, 0L, SEEK_SET);
        if (ferror(f)) {
            fprintf(stderr, "%s: seek error (%s)\n", progname, strerror(errno));
            ++error_count;
        } else {
            join_lines(f, bytes_read);
        }
    } else {
        fprintf(stderr, "%s: could not read until end of file.\n", progname);
        ++error_count;
    }
}

static void do_file(const char *fname)
{
    FILE *f;

    if ((f = fopen(fname, "rb+")) == 0) {
        fprintf(stderr, "%s: can't open %s (%s)\n",
                progname, fname, strerror(errno));
        ++error_count;
        return;
    }
    if (verbose) {
        printf("\nFile %s:\n\n", fname);
    }
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
                    case 'h':
                    case '?':
                        usage();
                        return EXIT_SUCCESS;
                    case 'v':
                        ++verbose;
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
#ifdef UNUSED
        nextarg: ;
#endif
    }
    if (!files_done)
        do_handle(stdin);
    return error_count ? EXIT_FAILURE : EXIT_SUCCESS;
}
