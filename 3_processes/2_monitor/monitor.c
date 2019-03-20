//
// Created by arkadius on 20.03.19.
//

#include <stdio.h>
#include "monitor.h"


void observe(const char *file_name, int step, int time, Mode mode) {
    printf("I will observe %s for %ds every %ds\n", file_name, time, step);
}
