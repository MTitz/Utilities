#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

static void pr_winsize(int);
static void usage(void);

static char *progname;

enum output { BOTH, ONLY_COL, ONLY_ROW };
static enum output options = BOTH;

int main(int argc, char *argv[])
{
    char *ptmp;
    progname = argv[0];
    if (ptmp = strrchr(progname, '/'))
        progname = ptmp + 1;

    if (argc == 2) {
        if (strcmp(argv[1], "--col") == 0 || strcmp(argv[1], "--columns") == 0) {
            options = ONLY_COL;
        } else if (strcmp(argv[1], "--row") == 0 || strcmp(argv[1], "--rows") == 0) {
            options = ONLY_ROW;
        } else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            usage();
            return 0;
        } else {
            fprintf(stderr, "Wrong argument: %s\n", argv[1]);
            return 1;
        }
    } else if (argc > 2) {
        fprintf(stderr, "Too many arguments.\n");
        return 1;
    }
    if (isatty(STDIN_FILENO))
        pr_winsize(STDIN_FILENO);
    return 0;
}

static void pr_winsize(int fd)
{
    struct winsize size;
    if (ioctl(fd, TIOCGWINSZ, (char *) &size) < 0)
        fprintf(stderr, "TIOCGWINSZ error\n");
    switch (options) {
      case BOTH:
        printf("%dx%d\n", size.ws_col, size.ws_row);
        break;
      case ONLY_COL:
        printf("%d\n", size.ws_col);
        break;
      case ONLY_ROW:
        printf("%d\n", size.ws_row);
        break;
    }
}

static void usage(void)
{
    printf("usage: %s [--col|--columns|--row|--rows]\n", progname);
}
