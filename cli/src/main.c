#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <roadmaster/data.h>

static struct rm_data *roadmaster_alloc(void)
{
    struct rm_data *d;
    int fd_shm = shm_open("/roadmaster", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd_shm < 0)
        err(2, "shm_open");

    if (ftruncate(fd_shm, sizeof (struct rm_data)) < 0)
        err(2, "ftruncate");

    d = mmap(NULL, sizeof (struct rm_data), PROT_READ | PROT_WRITE,
                MAP_SHARED, fd_shm, 0);
    if (d == MAP_FAILED)
        err(2, "mmap");

    return d;
}

static int print_rm_data(struct rm_data *d)
{
    printf("Magic:\t\t%x\n", d->magic);
    printf("GPIO:\t\t%lx\n", d->gpio);
    printf("Rotations:\t%lu\n", d->rot);
    printf("Speed:\n");
    printf(" G. average:\t%lu.%lu km/h\n",
            d->speed.global_average / 10,
            d->speed.global_average % 10);
    printf(" L. average:\t%lu.%lu km/h\n",
            d->speed.local_average / 10,
            d->speed.local_average % 10);
    printf(" G. target:\t%lu.%lu km/h\n",
            d->speed.global_target / 10,
            d->speed.global_target % 10);
    printf(" L. target:\t%lu.%lu km/h\n",
            d->speed.local_target / 10,
            d->speed.local_target % 10);
    printf(" Inst :\t\t%lu km/h\n",
            d->speed.inst);
    printf("Distance:\n");
    printf(" Global:\t%.02f km (%lu dam)\n",
           (double) d->distance.global / 100., d->distance.global);
    printf(" Local:\t\t%.02f km (%lu dam)\n",
           (double) d->distance.local / 100., d->distance.local);
    printf(" Partial:\t%0.02f km (%lu dam)\n",
            (double) d->distance.part / 100., d->distance.part);
    printf(" Global T.:\t%0.02f km (%lu dam)\n",
            (double) d->distance.global_target / 100.,
            d->distance.global_target);
    printf(" Local T.:\t%0.02f km (%lu dam)\n",
            (double) d->distance.local_target / 100.,
            d->distance.local_target);
    printf(" Local T.:\t%0.02f km (%lu dam)\n",
            (double) d->distance.local_target / 100.,
            d->distance.local_target);
    printf(" Ghost:\t%0.02f km (%lu dam)\n",
            (double) d->distance.local_ghost / 100.,
            d->distance.local_ghost);
    printf(" Ghost diff:\t%0.02f km (%lu dam)\n",
            (double) d->distance.local_ghost_diff / 100.,
            d->distance.local_ghost_diff);
    printf("Timers:\n");
    printf(" Global start:\t%ld.%06ld\n",
            d->timers.global_start.tv_sec,
            d->timers.global_start.tv_usec);
    printf(" Local start:\t%ld.%06ld\n",
            d->timers.local_start.tv_sec,
            d->timers.local_start.tv_usec);
    printf(" Global timer:\t%ld.%06ld (%d:%d:%d)\n",
            d->timers.global_timer.tv_sec,
            d->timers.global_timer.tv_usec,
            d->timers.global_timer_tm.tm_hour,
            d->timers.global_timer_tm.tm_min,
            d->timers.global_timer_tm.tm_sec);
    printf(" Local timer:\t%ld.%06ld (%d:%d:%d)\n",
            d->timers.local_timer.tv_sec,
            d->timers.local_timer.tv_usec,
            d->timers.local_timer_tm.tm_hour,
            d->timers.local_timer_tm.tm_min,
            d->timers.local_timer_tm.tm_sec);
    printf(" Local timer d:\t%ld.%06ld (%d:%d:%d)\n",
            d->timers.local_timer_dec.tv_sec,
            d->timers.local_timer_dec.tv_usec,
            d->timers.local_timer_dec_tm.tm_hour,
            d->timers.local_timer_dec_tm.tm_min,
            d->timers.local_timer_dec_tm.tm_sec);
    printf(" Local target:\t%ld.%06ld (%d:%d:%d)\n",
            d->timers.local_target.tv_sec, d->timers.current.tv_usec,
            d->timers.local_target_tm.tm_hour,
            d->timers.local_target_tm.tm_min,
            d->timers.local_target_tm.tm_sec);
    printf(" Current:\t%ld.%06ld (%d:%d:%d)\n",
            d->timers.current.tv_sec, d->timers.current.tv_usec,
            d->timers.current_tm.tm_hour,
            d->timers.current_tm.tm_min,
            d->timers.current_tm.tm_sec);
    printf("Settings:\n");
    printf(" Rot/km:\t%lu\n", d->settings.rot_per_km);
    printf("Control:\n");
    printf(" Forward:\t%d\n", d->ctrl.forward);
    printf(" Start:\t\t%d\n", d->ctrl.start);
    printf(" Hard start:\t%d\n", d->ctrl.hard_start);
    printf(" Calibration:\n");
    printf("  Start:\t%d\n", d->ctrl.calib.start);
    printf("  Running:\t%d\n", d->ctrl.calib.running);
    printf("  Finish:\t%d\n", d->ctrl.calib.finish);

    return 0;
}

static int start_calib(struct rm_data *d)
{
    d->ctrl.calib.start = true;
    return 0;
}

static int stop_calib(struct rm_data *d)
{
    d->ctrl.calib.finish = true;
    return 0;
}

static int forward(struct rm_data *d)
{
    d->ctrl.forward = true;
    return 0;
}

static int backward(struct rm_data *d)
{
    d->ctrl.forward = false;
    return 0;
}

struct cmd
{
    char *name;
    int (*f)(struct rm_data *r);
};

static int usage();

struct cmd commands[] =
{
    {"help", usage},
    {"show", print_rm_data},
    {"start-calib", start_calib},
    {"stop-calib", stop_calib},
    {"forward", forward},
    {"backward", backward},
    {NULL, NULL}
};

static int usage(struct rm_data *r)
{
    printf("Commands:\n");
    for (int i = 0; commands[i].name; ++i)
        printf("\t%s\n", commands[i].name);
    return 0;
}

int main(int argc, char *argv[])
{
    struct rm_data *d = roadmaster_alloc();
    if (argc > 1)
    {
        for (int i = 0; commands[i].name; ++i)
        {
            if (strcmp(argv[1], commands[i].name) == 0)
                return commands[i].f(d);
        }
    }
    print_rm_data(d);
    return 1;
}
