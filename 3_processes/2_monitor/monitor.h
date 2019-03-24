//
// Created by arkadius on 20.03.19.
//

#ifndef INC_2_MONITOR_MONITOR_H
#define INC_2_MONITOR_MONITOR_H

typedef enum Mode {
    ARCHIVE, COPY
} Mode;

int observe(const char* file_name, int interval, int lifespan, Mode mode);
int observe_restricted(const char *file_name, int interval, int lifespan, Mode mode, int cpu_limit, int memory_limit);

#endif //INC_2_MONITOR_MONITOR_H
