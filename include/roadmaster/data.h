#ifndef DATA_H
#define DATA_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#define ROADMASTER_MAGIC            0x55AA
#define ROADMASTER_DEFAULT_RPK      2266

struct rm_data
{
    unsigned long gpio;
    unsigned long rot;
    unsigned long rot_part;
    unsigned long prev_rot;
    struct
    {
        unsigned long global_average;
        unsigned long local_average;
        unsigned long global_target;
        unsigned long local_target;
        unsigned long inst; // km/h
    } speed; // hm/h
    struct
    {
        unsigned long global;
        unsigned long local;
        unsigned long part;
        unsigned long global_target;
        unsigned long local_target;
        unsigned long local_ghost;
        long local_ghost_diff;
    } distance; // dam
    struct
    {
        unsigned long rot_per_km;
    } settings;
    struct
    {
        struct timeval global_start;
        struct timeval local_start;
        struct timeval global_timer;
        struct tm global_timer_tm;
        struct timeval local_timer;
        struct tm local_timer_tm;
        struct timeval local_timer_dec;
        struct tm local_timer_dec_tm;
        struct timeval local_target;
        struct tm local_target_tm;
        struct timeval current;
        struct tm current_tm;
        struct timeval reset;
        struct timeval start;
    } timers;
    struct
    {
        bool forward;
        bool start;
        bool hard_start;
        struct
        {
            bool start;
            bool running;
            bool finish;
        } calib;
    } ctrl;
    uint16_t magic;
};

#endif /* DATA_H */
