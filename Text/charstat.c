/* File: charstat.c */

/* Version 0.1.1, Martin Titz, 2013 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 16384
#endif

#ifndef PORTABLE
#define GERMAN_UMLAUTS
#endif

static int verbose = 0;
static int error_count = 0;
static unsigned char buffer[BUFFER_SIZE];
static unsigned long byteCount[UCHAR_MAX+1];
const static char otherPrintableChars[] = {
    '!', '"', '#', '$', '%', '&', '?', '(', ')', '*',
    '+', ',', '-', '.', '/', ':', ';', '<', '=', '>',
    '@', '[', '\\', ']', '`', '^', '_', '\'', '{', '|',
    '}', '~'};
#ifdef __unix__
static char *progname;
#else
static const char *const progname = "charstat";
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

static void resetByteCount()
{
    int i;
    for (i = 0; i <= UCHAR_MAX; ++i) {
        byteCount[i] = 0;
    }
}

static int isOtherPrintable(int c)
{
    const size_t n  = sizeof(otherPrintableChars) / sizeof(char);
    size_t i;
    int found = 0;
    for (i = 0; i < n; ++i) {
        if (otherPrintableChars[i] == c) {
            found = 1;
            break;
        }
    }
    return found;
}

#ifdef GERMAN_UMLAUTS
static int isGermanUmlaut(int c)
{
    return c == 0xc4 || c == 0xd6 || c == 0xdc || c == 0xdf
                     || c == 0xe4 || c == 0xf6 || c == 0xfc;
}
#endif

static void printByteCount()
{
    int i;
    unsigned long uppercaseLetter = 0;
    unsigned long lowercaseLetter = 0;
    unsigned long alpha = 0;
    unsigned long digit = 0;
    unsigned long nl = 0, cr = 0;
    unsigned long blank = 0;
    unsigned long space = 0, tab = 0;
#ifdef GERMAN_UMLAUTS
    unsigned long germanUmlauts = 0;
#endif
    unsigned long otherPrintable = 0;
    unsigned long other = 0;
    unsigned long total = 0;
    int hasOtherChar = 0;
    int otherChar[UCHAR_MAX+1];
    memset(otherChar, 0, (UCHAR_MAX+1) * sizeof(int));
    for (i = 0; i <= UCHAR_MAX; ++i) {
        total += byteCount[i];
        if (isalpha(i)) {
            alpha += byteCount[i];
            if (i >= 'a' && i <= 'z')
                lowercaseLetter += byteCount[i];
            if (i >= 'A' && i <= 'Z')
                uppercaseLetter += byteCount[i];
        } else if (isdigit(i)) {
            digit += byteCount[i];
        } else if (i == '\n') {
            nl += byteCount[i];
        } else if (i == '\r') {
            cr += byteCount[i];
        } else if (isblank(i)) {
            blank += byteCount[i];
            if (i == ' ') {
                space += byteCount[i];
            } else if (i == '\t') {
                tab += byteCount[i];
            }
#ifdef GERMAN_UMLAUTS
        } else if (isGermanUmlaut(i)) {
            germanUmlauts += byteCount[i];
#endif
        } else if (isOtherPrintable(i)) {
            otherPrintable += byteCount[i];
        } else {
            other += byteCount[i];
            if (byteCount[i] > 0) {
                otherChar[i] = 1;
                hasOtherChar = 1;
            }
        }
    }

    if (verbose && total > 0) {
        for (i = 0; i <= UCHAR_MAX; ++i) {
            if (byteCount[i] > 0) {
                printf(" %3d %8lu\n", i, byteCount[i]);
            }
        }
        puts("");
    }
    printf("Alphabetic      %9lu\n", alpha);
    printf("    Lowercase   %9lu\n", lowercaseLetter);
    printf("    Uppercase   %9lu\n", uppercaseLetter);
    printf("Digit           %9lu\n", digit);
    printf("New Line        %9lu\n", nl);
    printf("Carriage Return %9lu\n", cr);
    printf("Blank           %9lu\n", blank);
    printf("    Space       %9lu\n", space);
    printf("    Tab         %9lu\n", tab);
#ifdef GERMAN_UMLAUTS
    if (germanUmlauts > 0) {
        printf("German Unlauts  %9lu\n", germanUmlauts);
    }
#endif
    printf("Other Printable %9lu\n", otherPrintable);
    printf("Other           %9lu\n", other);
    printf("TOTAL           %9lu\n", total);
    if (hasOtherChar) {
        puts("");
        printf("Other Characters are: ");
        for (i = 0; i <= UCHAR_MAX; ++i) {
            if (otherChar[i])
                printf(isprint(i) ? " '%c'" : " %2x", i);
        }
        puts("");
    }
}

static void do_handle(FILE *f)
{
    resetByteCount();
    for (;;) {
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, f);
        size_t i;
        for (i = 0; i < bytes_read; ++i) {
            ++byteCount[buffer[i]];
        }
        if (bytes_read < BUFFER_SIZE)
            break;
    }
    if (ferror(f)) {
        fprintf(stderr, "%s: read error (%s)\n", progname, strerror(errno));
        ++error_count;
    }
    printByteCount();
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
