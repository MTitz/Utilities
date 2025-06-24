/* File: linelengths.c */

/* Version 0.1.4, Martin Titz, 2014 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 16777216
#endif

#ifndef MAXLENGTH
#define MAXLENGTH 50000
#endif

static int classify = 0;
static int verbose = 0;
static int statistics = 0;
static int error_count = 0;
static unsigned char buffer[BUFFER_SIZE];
static size_t linelengths[MAXLENGTH];
#ifdef __unix__
static char *progname;
#else
static const char *const progname = "linelengths";
#endif

static void usage(void)
{
    fprintf(stderr, "usage: %s [-c] [-s] [-v] [file]...\n", progname);
}

/* static void arg_err(char c)
{
    fprintf(stderr, "%s: option -%c requires an argument.\n", progname, c);
    usage();
    exit(EXIT_FAILURE);
} */


static void print_statistics(size_t data[], size_t maxLength, size_t n)
{
    unsigned long sum = 0UL;
    size_t lineCount = 0;
    size_t i;

    for (i = 0; i <= maxLength; ++i) {
        lineCount += data[i];
        sum += i * data[i];
    }
    printf("\nStatistics:\n\n");
    printf("Total bytes read: %10lu\n", n);
    printf("Number of lines:  %10lu\n", lineCount);
    printf("Sum of line bytes:%10lu\n", sum);
    if (lineCount > 0) {
        printf("Average line length: %10.2f\n", (double)sum / lineCount);
    }
}

static void classify_file(const char *fname, size_t n)
{
    enum FileTypes {
        Empty,
        OnlyShortLines,
        NormalParagraphs,
        LongParagraphs,
        Undefined
    };
    const char * classificationText;
    const size_t lineThreshold1 = 60;
    const size_t lineThreshold2 = 90;
    size_t count[] = { 0, 0, 0 };
    size_t i;

    if (MAXLENGTH + 10 < lineThreshold2) {
        fprintf(stderr, "%s: constant MAXLENGTH set too low\n", progname);
        return;
    }

    for (i = 0; i < lineThreshold1; ++i) {
        count[0] += linelengths[i];
    }
    for (i = lineThreshold1; i < lineThreshold2; ++i) {
        count[1] += linelengths[i];
    }
    for (i = lineThreshold2; i <= n; ++i) {
        count[2] += linelengths[i];
    }

    enum FileTypes classification;
    if (count[1] == 0 && count[2] == 0) {
        classification = count[0] > 0 ? OnlyShortLines : Empty;
    } else if (count[2] == 0) {
        classification = NormalParagraphs;
    } else if (count[2] >= 10 && count[2] >= 2 * count[1]) {
        classification = LongParagraphs;
    } else if (count[2] <=  5 && count[1] >=  50
            || count[2] <= 10 && count[1] >= 200
            || count[2] <= 20 && count[1] >= 500) {
        classification = NormalParagraphs;
    } else {
        classification = Undefined;
    }

    switch (classification) {
        case Empty:
            classificationText = "Empty";
            break;
        case OnlyShortLines:
            classificationText = "Only short lines";
            break;
        case NormalParagraphs:
            classificationText = "Normal length lines";
            break;
        case LongParagraphs:
            classificationText = "Long lines";
            break;
        case Undefined:
            classificationText = "Undefined";
            break;
    }
    printf("%s -> %s\n",
            fname == 0 ? "<stdin>" : fname,
            classificationText);
    if (verbose) {
        printf("    classification counts: %lu %lu %lu -> %d\n",
               count[0], count[1], count[2], classification);
    }
}

static void process_lines(const char *fname, size_t n)
{
    size_t i;
    size_t length = 0;
    size_t maxLength = 0;
    int tooLong = 0;
    for (i = 0; i < MAXLENGTH; ++i) {
        linelengths[i] = 0;
    }
    for (i = 0; i < n; ++i) {
        unsigned char ch = buffer[i];

        /* Ignore '\r' before '\n' */
        if (ch == '\r' && i+1 < n && buffer[i+1] == '\n') {
            continue;
        }

        if (ch == '\n') {
            if (length >= MAXLENGTH) {
                length = MAXLENGTH-1;
                ++tooLong;
            }
            if (length > maxLength) {
                maxLength = length;
            }
            ++linelengths[length];
            length = 0;
        } else {
            ++length;
        }
    }

    /* Process remaining bytes if file does not end with \n */
    if (length > 0) {
        if (length >= MAXLENGTH) {
            length = MAXLENGTH-1;
            ++tooLong;
        }
        if (length > maxLength) {
            maxLength = length;
        }
        ++linelengths[length];
        length = 0;
    }

    if (classify) {
        classify_file(fname, maxLength);
    } else {
        for (i = 0; i <= maxLength; ++i) {
            if (linelengths[i] > 0) {
                printf("%lu  %lu\n", i, linelengths[i]);
            }
        }
        if (tooLong) {
                printf("\nToo long lines detected: %d\n", tooLong);
        }
        if (statistics) {
            print_statistics(linelengths, maxLength, n);
        }
    }
}

static void do_handle(FILE *file, const char *fname)
{
    size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
    if (ferror(file)) {
        fprintf(stderr, "%s: read error (%s)\n", progname, strerror(errno));
        ++error_count;
    } else if (feof(file)) {
        /* Process bytes_read bytes stored in the buffer */
        process_lines(fname, bytes_read);
    } else {
        fprintf(stderr, "%s: could not read until end of file.\n", progname);
        ++error_count;
    }
}

static void do_file(const char *fname)
{
    FILE *file;

    if ((file = fopen(fname, "rb")) == 0) {
        fprintf(stderr, "%s: can't open %s (%s)\n",
                progname, fname, strerror(errno));
        ++error_count;
        return;
    }
    if (verbose && !classify) {
        printf("\nFile %s:\n\n", fname);
    }
    do_handle(file, fname);
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
                if (files_done && !classify)
                    puts("");
                if (verbose)
                    puts("\nstdin:\n");
                do_handle(stdin, 0);
                ++files_done;
                continue;
            }
            while (*++*argv) {
                switch (**argv) {
                    case 'c':
                        classify = 1;
                        break;
                    case 'h':
                    case '?':
                        usage();
                        return EXIT_SUCCESS;
                    case 's':
                        statistics = 1;
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
            if (files_done && !classify)
                puts("");
            do_file(*argv);
            ++files_done;
        }
        /* nextarg: ; */
    }
    if (!files_done)
        do_handle(stdin, 0);
    return error_count ? EXIT_FAILURE : EXIT_SUCCESS;
}
