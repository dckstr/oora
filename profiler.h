#include <stdio.h>
#include <stdbool.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include "papi.h"

#define EVENTS_NUMBER 4

int events[EVENTS_NUMBER] = {PAPI_FP_OPS, PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L2_DCH};
char *event_descriptions[EVENTS_NUMBER] = {"FP ops", "L1 DCM", "L2 DCM", "L2 DCH"};
long long values[EVENTS_NUMBER] = {0,};
int eventSet = PAPI_NULL;
int papi_err;
bool papi_supported = true;

void profiler_init() {
    cpu_set_t c_set;
    CPU_ZERO(&c_set);
    CPU_SET(0, &c_set);
    if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &c_set) != 0)
    {
        perror("sched_setaffinity");
    } 

    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI is unsupported\n");
        papi_supported = false;
    }

    if (PAPI_num_counters() < EVENTS_NUMBER) {
        fprintf(stderr, "PAPI is unsupported\n");
        papi_supported = false;
    }

    if ((papi_err = PAPI_create_eventset(&eventSet)) != PAPI_OK) {
        fprintf(stderr, "Could not create event set: %s\n", PAPI_strerror(papi_err));
    }

    for (int i = 0; i < EVENTS_NUMBER; ++i) {
        if ((papi_err = PAPI_add_event(eventSet, events[i])) != PAPI_OK) {
            fprintf(stderr, "Could not add event %s: %s\n", event_descriptions[i], PAPI_strerror(papi_err));
        }
    }
}

void profiler_start() {
    if (papi_supported) {
        if ((papi_err = PAPI_start(eventSet)) != PAPI_OK) {
            fprintf(stderr, "Could not start counters: %s\n", PAPI_strerror(papi_err));
        }
    }
}

void profiler_stop() {
    if (papi_supported) {
        if ((papi_err = PAPI_stop(eventSet, values)) != PAPI_OK) {
            fprintf(stderr, "Could not get values: %s\n", PAPI_strerror(papi_err));
        }
    }
}

void profiler_print() {
    for (int i = 0; i < EVENTS_NUMBER; i++) {
        printf("%s: %ld\n", event_descriptions[i], values[i]);
    }
}
