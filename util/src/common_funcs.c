#include "util/common_funcs.h"
#include <stdio.h>
#include <time.h>


#define NS_PER_MS 1000000
#define NS_PER_S 1000000000

void sleep_ms(long long ms) {
    struct timespec ts;
    ts.tv_nsec = ms * NS_PER_MS;
    ts.tv_sec = ts.tv_nsec / NS_PER_S;
    ts.tv_nsec %= NS_PER_S;

    int ret = nanosleep(&ts, NULL);
    if (ret != 0) {
        perror("Nanosleep error");
    }
}
