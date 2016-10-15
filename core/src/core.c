#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "roadmaster.h"
#include "log.h"

static void roadmaster_alloc(struct roadmaster *r)
{
    r->fd_shm = shm_open("/roadmaster", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (r->fd_shm < 0)
        err(2, "shm_open");

    if (ftruncate(r->fd_shm, sizeof (struct rm_data)) < 0)
        err(2, "ftruncate");

    r->d = mmap(NULL, sizeof (struct rm_data), PROT_READ | PROT_WRITE,
                MAP_SHARED, r->fd_shm, 0);
    if (r->d == MAP_FAILED)
        err(2, "mmap");
}

void roadmaster_save(struct roadmaster *r)
{
    int fd = open(ROADMASTER_BACKUP, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    write(fd, r->d, sizeof (struct rm_data));

    close(fd);
}

int roadmaster_load(struct roadmaster *r)
{
    int fd = open(ROADMASTER_BACKUP, O_RDONLY);
    if (fd < 0)
        return 1;

    read(fd, r->d, sizeof (struct rm_data));

    close(fd);
    return (r->d->magic != ROADMASTER_MAGIC);
}

static unsigned long compute_average_speed(unsigned long distance,
                                           struct timeval *tv)
{
    if (tv->tv_sec == 0)
        return 0;
    return (distance * 3600) / (tv->tv_sec * 10);
}

static void compute_ghost(struct roadmaster *r)
{
    r->d->distance.local_ghost = (r->d->speed.local_target
        * r->d->timers.local_timer.tv_sec) / 360;
    r->d->distance.local_ghost_diff = r->d->distance.local
        - r->d->distance.local_ghost;
}

void roadmaster_refresh(union sigval value)
{
    struct roadmaster *r = value.sival_ptr;

    gettimeofday(&r->d->timers.current, NULL);
    timersub(&r->d->timers.current, &r->d->timers.global_start,
            &r->d->timers.global_timer);
    r->d->speed.global_average = compute_average_speed(r->d->distance.global,
            &r->d->timers.global_timer);
    compute_ghost(r);

    if (!r->d->ctrl.start)
    {
        timersub(&r->d->timers.current, &r->d->timers.local_start,
                &r->d->timers.local_timer);
        timersub(&r->d->timers.local_target, &r->d->timers.local_timer,
                &r->d->timers.local_timer_dec);
        r->d->speed.local_average = compute_average_speed(r->d->distance.local,
                &r->d->timers.local_timer);
    }

    localtime_r(&r->d->timers.current.tv_sec, &r->d->timers.current_tm);
    gmtime_r(&r->d->timers.global_timer.tv_sec,
                &r->d->timers.global_timer_tm);
    gmtime_r(&r->d->timers.local_timer.tv_sec,
                &r->d->timers.local_timer_tm);
    gmtime_r(&r->d->timers.local_timer_dec.tv_sec,
                &r->d->timers.local_timer_dec_tm);

    roadmaster_save(r);
}

static void roadmaster_inst_speed(union sigval value)
{
    struct roadmaster *r = value.sival_ptr;

    if (r->d->settings.rot_per_km == 0)
        r->d->speed.inst = 0;
    else
        r->d->speed.inst = (r->d->speed.inst
                + (((r->d->rot - r->d->prev_rot) * 3600)
                    / (r->d->settings.rot_per_km))) / 2;
    r->d->prev_rot = r->d->rot;
}

void roadmaster_init(struct roadmaster *roadmaster)
{
    struct sigevent sev_refresh =
    {
        .sigev_notify = SIGEV_THREAD,
        .sigev_notify_function = roadmaster_refresh,
        .sigev_value.sival_ptr = roadmaster,
        .sigev_notify_attributes = NULL
    };
    struct sigevent sev_inst_speed =
    {
        .sigev_notify = SIGEV_THREAD,
        .sigev_notify_function = roadmaster_inst_speed,
        .sigev_value.sival_ptr = roadmaster,
        .sigev_notify_attributes = NULL
    };

    roadmaster_alloc(roadmaster);
    gpio_init(&roadmaster->gpio);
    timer_create(CLOCK_REALTIME, &sev_refresh, &roadmaster->timer_refresh);
    timer_create(CLOCK_REALTIME, &sev_inst_speed,
            &roadmaster->timer_inst_speed);
    if (roadmaster->d->magic != ROADMASTER_MAGIC
            && roadmaster_load(roadmaster))
    {
        _DEBUG("%s", "Resetting Roadmaster...");
        roadmaster_reset(roadmaster);
    }
}

static void roadmaster_reset_timer(struct roadmaster *r, struct timeval *tv)
{
    gettimeofday(&r->d->timers.current, NULL);
    memcpy(tv, &r->d->timers.current, sizeof (struct timeval));
}

static void roadmaster_soft_reset(struct roadmaster *r)
{
    r->d->gpio = 0;
    r->d->rot = 0;
    r->d->rot_part = 0;
    r->d->prev_rot = 0;
    r->d->speed.global_average = 0;
    r->d->speed.local_average = 0;
    r->d->speed.local_target = 0;
    r->d->speed.global_target = 0;
    r->d->speed.inst = 0;
    r->d->distance.global = 0;
    r->d->distance.local = 0;
    r->d->distance.part = 0;
    r->d->distance.global_target = 0;
    r->d->distance.local_ghost = 0;
    r->d->distance.local_ghost_diff = 0;
    r->d->ctrl.forward = true;
    r->d->ctrl.start = false;
    r->d->ctrl.hard_start = false;
    r->d->ctrl.calib.start = false;
    r->d->ctrl.calib.running = false;
    r->d->ctrl.calib.finish = false;
    timerclear(&r->d->timers.global_timer);
    timerclear(&r->d->timers.local_timer);
    timerclear(&r->d->timers.local_timer_dec);
    timerclear(&r->d->timers.local_target);
    roadmaster_reset_timer(r, &r->d->timers.global_start);
    roadmaster_reset_timer(r, &r->d->timers.local_start);
    roadmaster_reset_timer(r, &r->d->timers.reset);
    r->d->magic = ROADMASTER_MAGIC;
}

void roadmaster_reset(struct roadmaster *r)
{
    roadmaster_soft_reset(r);
    r->d->settings.rot_per_km = ROADMASTER_DEFAULT_RPK;
    r->d->magic = ROADMASTER_MAGIC;
}

static void roadmaster_start(struct roadmaster *r)
{
    struct itimerspec its;

    its.it_value.tv_sec = ROADMASTER_FREQ_REFRESH;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    timer_settime(r->timer_refresh, 0, &its, NULL);

    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    timer_settime(r->timer_inst_speed, 0, &its, NULL);
}

static void roadmaster_update_distance(struct roadmaster *r,
        unsigned long incr)
{
    if (!r->d->ctrl.forward)
        incr = -incr;
    r->d->distance.local += incr;
    r->d->distance.global += incr;
    r->d->distance.part += incr;
}

static void roadmaster_handle_rot(struct roadmaster *r)
{
    unsigned long rot_per_dm = r->d->settings.rot_per_km / 100;

    ++r->d->rot;
    ++r->d->rot_part;
    if (r->d->ctrl.start)
    {
        r->d->ctrl.start = false;
        roadmaster_reset_timer(r, &r->d->timers.local_start);
        if (r->d->ctrl.hard_start)
        {
            r->d->ctrl.hard_start = false;
            roadmaster_reset_timer(r, &r->d->timers.global_start);
        }
    }
    if (!r->d->ctrl.calib.running)
    {
        while (r->d->rot_part > rot_per_dm)
        {
            roadmaster_update_distance(r, 1);
            r->d->rot_part -= rot_per_dm;
        }
    }
    else
        r->d->settings.rot_per_km = r->d->rot_part;
}

static void roadmaster_handle_reset(struct roadmaster *r)
{
    _DEBUG("%s", "reset");
    roadmaster_reset_timer(r, &r->d->timers.reset);
    r->d->distance.part = 0;
    r->d->ctrl.start = 0;
    r->d->ctrl.hard_start = 0;
}

static void roadmaster_handle_hard_reset(struct roadmaster *r)
{
    struct timeval tv_reset;

    gettimeofday(&r->d->timers.current, NULL);
    timersub(&r->d->timers.current, &r->d->timers.reset,
            &tv_reset);
    if (tv_reset.tv_sec < 3)
        return;
    r->d->distance.local = 0;
    r->d->distance.part = 0;
    roadmaster_reset_timer(r, &r->d->timers.local_start);
    if (tv_reset.tv_sec < 6)
        return;
    roadmaster_soft_reset(r);
}

static void roadmaster_handle_start(struct roadmaster *r)
{
    _DEBUG("%s", "start");
    roadmaster_handle_reset(r);
    roadmaster_reset_timer(r, &r->d->timers.start);
}

static void roadmaster_handle_hard_start(struct roadmaster *r)
{
    struct timeval tv_reset;

    gettimeofday(&r->d->timers.current, NULL);
    timersub(&r->d->timers.current, &r->d->timers.start,
            &tv_reset);
    if (tv_reset.tv_sec < 2)
        return;
    r->d->distance.part = 0;
    r->d->distance.local = 0;
    r->d->ctrl.start = true;
    if (tv_reset.tv_sec < 4)
        return;
    _DEBUG("%s", "hard_start");
    r->d->distance.global = 0;
    r->d->distance.local = 0;
    r->d->ctrl.hard_start = true;
    if (tv_reset.tv_sec < 10)
        return;
    roadmaster_handle_hard_reset(r);
    roadmaster_reset_timer(r, &r->d->timers.start);
}

static void roadmaster_check_gpio(struct roadmaster *r)
{
    static unsigned long prev = GPIO_DEFAULT;
    unsigned long current = gpio_get(&r->gpio);
    r->d->gpio = current;

    if (TEST_BIT(prev, current, BIT_SENSOR))
        roadmaster_handle_rot(r);
    if (TEST_BIT(prev, current, BIT_RESET))
        roadmaster_handle_reset(r);
    else if ((~(current) & BIT_RESET) != 0)
        roadmaster_handle_hard_reset(r);
    if (TEST_BIT(prev, current, BIT_START))
        roadmaster_handle_start(r);
    else if ((~(current) & BIT_START) != 0)
        roadmaster_handle_hard_start(r);
    prev = current;
}

static void roadmaster_check_ctrl(struct roadmaster *r)
{
    if (r->d->ctrl.calib.running)
    {
        r->d->settings.rot_per_km = r->d->rot_part;
        if (r->d->ctrl.calib.finish)
        {
            r->d->ctrl.calib.running = false;
            r->d->ctrl.calib.finish = false;
        }
    }
    else
    {
        if (r->d->ctrl.calib.start)
        {
            r->d->rot_part = 0;
            r->d->ctrl.calib.running = true;
            r->d->ctrl.calib.start = false;
        }
    }
}

void roadmaster_run(struct roadmaster *r)
{
    roadmaster_start(r);
    for (;;)
    {
        roadmaster_check_gpio(r);
        roadmaster_check_ctrl(r);
    }
}
