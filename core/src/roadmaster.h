#ifndef ROADMASTER_H
#define ROADMASTER_H

#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <roadmaster/data.h>
#include "gpio.h"


#define ROADMASTER_FREQ_REFRESH     1
#define ROADMASTER_FREQ_INST_SPEED  500000000
#define ROADMASTER_BACKUP       "/root/.roadmaster.bak"

struct roadmaster
{
    struct rm_data *d;
    struct gpio gpio;
    timer_t timer_refresh;
    timer_t timer_inst_speed;
    int fd_shm;
};

void roadmaster_init(struct roadmaster *roadmaster);
void roadmaster_reset(struct roadmaster *roadmaster);
void roadmaster_run(struct roadmaster *r);

#endif /* ROADMASTER_H */
