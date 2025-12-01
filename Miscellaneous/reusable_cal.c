/* Determine if a calendar can be reused for another year in a given range.  *
 * by Martin Titz, 2025                                                      *
 *                                                                           *
 * A wall calendar is considered to be reusable, if the weekdays are         *
 * matching for the whole year (or the days until February 28th or from      *
 * March to December). But holidays like Easter will still not match.        *
 *                                                                           *
 * A typical call of the compiled program is                                 *
 *     ./reusable_cal 1975 2075 > reusable_cal.txt                           *
 * calculating the reusable calendars for the years from 1975 to 2075 and    *
 * storing them in the file reusable_cal.txt.                                *
 *   (the ./ at beginning is on Unix-like systems required if executable is  *
 *    not residing in a directory which is in the search path for commands.  *
 *                                                                           */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_leap_year(int year)
{
    return year % 4 == 0 && year % 100 != 0 || year % 400 == 0;
}

static void print_case(int no, int first, int last, int cal_class[], const char *months)
{
    printf("\nReusable calendars for %s:\n", months);
    int c_end = no == 1 ? 14 : 7;
    for (int c = 0; c < c_end; ++c) {
        int c2 = -1;
        switch (no) {
          case 2:
            c2 = c + 7;
            break;
          case 3:
            c2 = c + 6;
            if (c2 == 6)
                c2 = 13;
            break;
        }
        int has_entries = 0;
        for (int year = first; year <= last; ++year) {
            if (cal_class[year - first] == c || cal_class[year - first] == c2) {
                printf("%5d", year);
                has_entries = 1;
            }
        }
        if (has_entries)
            putchar('\n');
    }
}

int main(int argc, char *argv[])
{
    char *progname = argv[0];
#ifdef __unix__
    char *ptmp;
    if ((ptmp = strrchr(progname, '/')))
        progname = ptmp + 1;
#endif

    if (argc != 3) {
        fprintf(stderr, "%s: needs two arguments\n", progname);
        return EXIT_FAILURE;
    }

    int first = atoi(argv[1]);
    int last  = atoi(argv[2]);

    if (first < 1583 || last < first || last >= 10000) {
        fprintf(stderr, "%s: arguments must be years from 1583 to 9999\n", progname);
        return EXIT_FAILURE;
    }

    int cal_class[last - first + 1];
    int c = 0;
    for (int year = first; year <= last; ++year) {
        cal_class[year - first] = c;
        if (is_leap_year(year)) {
            cal_class[year - first] += 7;
            ++c;
        }
        c = (c + 1) % 7;
    }

    printf("Reusable calendars for years from %d to %d\n\n", first, last);
    print_case(1, first, last, cal_class, "whole years");
    print_case(2, first, last, cal_class, "January 1st until February 28th");
    print_case(3, first, last, cal_class, "March until December");

    return EXIT_SUCCESS;
}
