#include <err.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "gpio.h"

void gpio_init(struct gpio *gpio)
{
    int fd = open("/dev/mem", O_RDWR);

    if (fd < 0)
        err(2, "/dev/mem");

    gpio->base = mmap(0, GPIO_MAP_SIZE, PROT_READ, MAP_SHARED, fd, GPIO_BASE);
    gpio->addr = gpio->base + GPIO_OFFSET;
    close(fd);
}

void gpio_clean(struct gpio *gpio)
{
    munmap(gpio->base, GPIO_MAP_SIZE);
}

unsigned long gpio_get(struct gpio *gpio)
{
    return *((unsigned long *) gpio->addr);
}
