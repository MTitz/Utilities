/* File: joinlines.c */

/* Version 0.1.5, Martin Titz, 2013, 2014, 2016 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 16777216
#endif

static int addPagebreaks = 0;
static int ignoreEmptyLines = 0;
static int minLineLength = 0;
static int maxNewlines = 0;
static int verbose = 0;
static int error_count = 0;
static int experimentalMode = 0;
static unsigned char buffer[BUFFER_SIZE];
#ifdef __unix__
static char *progname;
#else
static const char *const progname = "joinlines";
#endif

static void usage(void)
{
    fprintf(stderr, "usage: %s [-E] [-l minLineLength] [-m maxNewlines] [-p] [-v] [file]...\n", progname);
    fprintf(stderr, "  -E   ignore empty lines\n"
                    "  -l   set minmal line length for joining\n"
                    "  -p   add pagebreaks\n"
                    "  -e   experimental joining of pdf2text files\n"
        );
}

static void arg_err(char c)
{
    fprintf(stderr, "%s: option -%c requires an argument.\n", progname, c);
    usage();
    exit(EXIT_FAILURE);
}

static int noLinefeedExpected(unsigned char lastCh)
{
    return isalnum(lastCh) || lastCh == ',' || lastCh == ';' || lastCh == '%';
}

static void join_lines(FILE *file, size_t n)
{
    size_t i;
    int lfCount = 0;
    int crCount = 0;
    int nlCount = 0;
    int lastLineLength = 0;
    unsigned char lastCh = '\0';  /* only for experimental mode */
    for (i = 0; i < n; ++i) {
        unsigned char ch = buffer[i];
        if (verbose >= 2)
            fprintf(stderr, " processing character %d\n", ch);
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
                /* Here one or more newlines and a first character afterwards had been read in. */
                if (verbose >= 2)
                    fprintf(stderr, "lastLineLength = %d\n", lastLineLength);
                if (nlCount <= 1 + ignoreEmptyLines && lastLineLength >= minLineLength && (!experimentalMode || noLinefeedExpected(lastCh))) {
                    /* Join lines. */
                    if (ch != ' ')
                        fputc(' ', file);
                } else {
                    /* Keep lines separate by emitting newlines in Unix style. */
                    int j;
                    if (maxNewlines > 0 && nlCount > maxNewlines) {
                        nlCount = maxNewlines;
                    }
                    for (j = 0; j < nlCount; ++j) {
                        fputc('\n', file);
                    }
                    for (j = 0; j < addPagebreaks; ++j) {
                        fputc('\n', file);
                    }
                }
                lfCount = 0;
                crCount = 0;
                nlCount = 0;
                lastLineLength = 1; /* We already have read the first character of this line. */
            } else {
                ++lastLineLength;
            }
            fputc(ch, file);

            /* Set lastCh to last 'relevant' character, here ch cannot be LF or CR */
            if (ch != ' ' && ch != '\t') {
                lastCh = ch;
            }
        }
    }
    fputc('\n', file);
    int fileNo = fileno(file);
    if (fileNo > 0 && fileNo != fileno(stdout) && ftruncate(fileNo, ftell(file)) != 0) {
        fprintf(stderr, "%s: ftruncate error (%s)\n", progname, strerror(errno));
    }
}

static void do_handle(FILE *file)
{
    size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
    if (ferror(file)) {
        fprintf(stderr, "%s: read error (%s)\n", progname, strerror(errno));
        ++error_count;
    } else if (feof(file)) {
        if (file != stdin) {
            /* Process bytes_read bytes stored in the buffer */
            fseek(file, 0L, SEEK_SET);
            if (ferror(file)) {
                fprintf(stderr, "%s: seek error (%s)\n", progname, strerror(errno));
                ++error_count;
            } else {
                join_lines(file, bytes_read);
            }
        } else {
            join_lines(stdout, bytes_read);
        }
    } else {
        fprintf(stderr, "%s: could not read until end of file.\n", progname);
        ++error_count;
    }
}

static void do_file(const char *fname)
{
    FILE *file;

    if ((file = fopen(fname, "rb+")) == 0) {
        fprintf(stderr, "%s: can't open %s (%s)\n",
                progname, fname, strerror(errno));
        ++error_count;
        return;
    }
    if (verbose) {
        printf("\nFile %s:\n\n", fname);
    }
    do_handle(file);
    fclose(file);
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
                    case 'e':
                        experimentalMode = 1;
                        break;
                    case 'E':
                        ++ignoreEmptyLines;
                        break;
                    case 'h':
                    case '?':
                        usage();
                        return EXIT_SUCCESS;
                    case 'l':
                        if (*++*argv || --argc && *++argv) {
                            minLineLength = atoi(*argv);
                        } else
                            arg_err('l');
                        goto nextarg;
                    case 'm':
                        if (*++*argv || --argc && *++argv) {
                            maxNewlines = atoi(*argv);
                        } else
                            arg_err('m');
                        goto nextarg;
                    case 'p':
                        ++addPagebreaks;
                        break;
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
        nextarg: ;
    }
    if (!files_done)
        do_handle(stdin);
    return error_count ? EXIT_FAILURE : EXIT_SUCCESS;
}
