#pragma once

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

inline uint32_t free_memory() {

    const size_t BUF_SIZE = 128;
    char buffer[BUF_SIZE];

    auto fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        printf("Failed to open /proc/meminfo\n");
        return 0;
    }

    uint32_t mem_available;

    while (fgets(&buffer[0], BUF_SIZE, fp) != nullptr) {
       if (strncmp("MemAvailable:", buffer, 13) == 0) {
           sscanf(&buffer[13], "%u kB", &mem_available);
           break;
       }
    }

    fclose(fp);

    uint32_t mb = mem_available / 1024;

    return mb;
}

inline uint32_t cpu_usage() {
    static long double previous[4];
    long double current[4];

    auto fp = fopen("/proc/stat", "r");
    if (!fp) {
        printf("Failed to open /proc/stat\n");
        return 0;
    }

    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &current[0], &current[1], &current[2], &current[3]);

    auto non_idle_previous = previous[0] + previous[1] + previous[2];
    auto non_idle_current = current[0] + current[1] + current[2];

    long double load = ((non_idle_current) - (non_idle_previous)) /
                ((non_idle_current + current[3]) - (non_idle_previous + previous[3]));

    memcpy(previous, current, sizeof(long double) * 4);

    fclose(fp);

    return (uint32_t) (load * 100);
}
