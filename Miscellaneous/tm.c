#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    const size_t buffer_size = 80;
    char s[buffer_size];
    time_t current_time = time(NULL);
    struct tm *local;

    if (current_time == ((time_t)-1)) {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        exit(EXIT_FAILURE);
    }

    local = localtime(&current_time);
    size_t chars = strftime(s, buffer_size, "%a %d. %b %Y %H:%M:%S %Z", local);
    if (chars == 0) {
        (void) fprintf(stderr, "Conversion failure, internal buffer too small.\n");
        exit(EXIT_FAILURE);
    } else {
        (void) puts(s);
    }
    return EXIT_SUCCESS;
}
