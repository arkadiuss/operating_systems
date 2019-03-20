//
// Created by arkadius on 20.03.19.
//

#ifndef INC_2_MONITOR_MONITOR_H
#define INC_2_MONITOR_MONITOR_H

typedef enum Mode {
    ARCHIVE, COPY
} Mode;

void observe(const char* file_name, int step, int time, Mode mode);

#endif //INC_2_MONITOR_MONITOR_H
