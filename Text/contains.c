/* File: contains.c */

/* Version 1.0, Martin Titz, 2002 */

#include <errno.h>
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

static int error_count = 0;
static int verbose = 1;
static unsigned char block1[BUFFER_SIZE], block2[BUFFER_SIZE];

#ifdef __unix__
static char *progname;
#else
static const char *const progname = "contains";
#endif

static void usage(void)
{
    fprintf(stderr, "usage: %s [OPTION]... FILE1 FILE2\n", progname);
}

static size_t read_block(unsigned char buffer[], FILE *f)
{
    size_t bytes = fread(buffer, 1, BUFFER_SIZE, f);
    if (ferror(f)) {
	fprintf(stderr, "%s: read error (%s)\n", progname, strerror(errno));
	++error_count;
	bytes = 0;
    }
    return bytes;
}

static int contains(FILE *f1, FILE *f2)
{
    size_t bytes1, bytes2;
    size_t c1, c2;

    bytes2 = read_block(block2, f2);
    c2 = 0;
    while ((bytes1 = read_block(block1, f1)) > 0) {
	c1 = 0;
	while (c1 < bytes1) {
	    /* trying to find character block1[c1] in file2,
	       starting with position block2[c2] */
	    do {
		if (c2 >= bytes2) {
		    bytes2 = read_block(block2, f2);
		    c2 = 0;
		    if (bytes2 == 0)
			return 0;	/* reached eof or error */
		}
	    } while (block1[c1] != block2[c2++]);

	    /* next character of file1 */
	    ++c1;
	}
    }
    return 1;
}

static int do_files(const char *filename1, const char *filename2)
{
    FILE *file1, *file2;
    int result;

    if ((file1 = fopen(filename1, "rb")) == 0) {
	fprintf(stderr, "%s: can't open %s (%s)\n",
		progname, filename1, strerror(errno));
	++error_count;
	return 0;
    }
    if ((file2 = fopen(filename2, "rb")) == 0) {
	fprintf(stderr, "%s: can't open %s (%s)\n",
		progname, filename2, strerror(errno));
	++error_count;
	fclose(file1);
	return 0;
    }
    result = contains(file1, file2);
    fclose(file1);
    fclose(file2);
    return result;
}

int main(int argc, char *argv[])
{
    int do_opts = 1;
    int files_index = 0;
    char *fnames[2];
#ifdef __unix__
    char *ptmp;
    progname = argv[0];
    if (ptmp = strrchr(progname, '/'))
	progname = ptmp + 1;
#endif
    for (--argc, ++argv;  argc > 0;  --argc, ++argv) {
	if (**argv == '-' && do_opts) {
	    while (*++*argv) {
		switch (**argv) {
		    case 'h':
		    case '?':
			usage();
			return EXIT_SUCCESS;
		    case 'q':
			verbose = 0;
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
	} else if (files_index < 2) {
	    fnames[files_index++] = *argv;
	} else {
	    fprintf(stderr, "%s: too much filenames.\n", progname);
	    usage();
	    return EXIT_FAILURE;
	}
    }
    if (files_index == 2) {
	int result = do_files(fnames[0], fnames[1]);
	if (error_count == 0) {
	    if (verbose)
		printf("File %s is%s part of %s\n", fnames[0],
		       result ? "" : " not", fnames[1]);
	    return result ? EXIT_SUCCESS : EXIT_FAILURE;
	}
    } else {
	fprintf(stderr, "%s: too few filenames.\n", progname);
	usage();
	return EXIT_FAILURE;
    }
    return error_count ? EXIT_FAILURE : EXIT_SUCCESS;
}
