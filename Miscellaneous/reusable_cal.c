/* Determine if a calendar can be reused for another year in a given range. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_leap_year(int year)
{
    return year % 4 == 0 && year % 100 != 0 || year % 400 == 0;
}

int main(int argc, char *argv[])
{
    char *progname = argv[0];
#ifdef __unix__
    char *ptmp;
    if (ptmp = strrchr(progname, '/'))
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

    int n_years = last - first + 1;
    int cal_class[n_years];
    int c = 0;
    int year;

    for (year = first; year <= last; ++year) {
        cal_class[year - first] = c;
        if (is_leap_year(year)) {
            cal_class[year - first] += 7;
            ++c;
        }
        c = (c+1) % 7;
    }

    printf("Resusable calenders for whole years:\n");
    for (c = 0; c < 14; ++c) {
        int has_entries = 0;
        for (int year = first; year <= last; ++year) {
            if (cal_class[year - first] == c) {
                printf("%5d", year);
                has_entries = 1;
            }
        }
        if (has_entries)
            putchar('\n');
    }

    printf("\nResusable calenders from January 1st until February 28th:\n");
    for (c = 0; c < 7; ++c) {
        int has_entries = 0;
        for (int year = first; year <= last; ++year) {
            if (cal_class[year - first] == c || cal_class[year - first] == c+7) {
                printf("%5d", year);
                has_entries = 1;
            }
        }
        if (has_entries)
            putchar('\n');
    }

    printf("\nResusable calenders from March until December:\n");
    for (c = 0; c < 7; ++c) {
        int has_entries = 0;
        for (int year = first; year <= last; ++year) {
            int c2 = c + 6;
            if (c2 == 6)
                c2 = 13;
            if (cal_class[year - first] == c || cal_class[year - first] == c2) {
                printf("%5d", year);
                has_entries = 1;
            }
        }
        if (has_entries)
            putchar('\n');
    }
}
