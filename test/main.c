/*
** main.c for Roadmaster
**
** Made by Pierre Surply
** <pierre.surply@gmail.com>
**
** Started on  Sat Dec 28 17:53:07 2013 Pierre Surply
** Last update Mon Dec 30 18:06:49 2013 Pierre Surply
*/

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>

#define MAP_SIZE        4094UL
#define MAP_MASK        (MAP_SIZE - 1)

#define GPIO_BASE       0x7F008000
#define GPE             (0x84)

int main(void)
{
  int fd = open("/dev/mem", O_RDWR);

  char *gpio_base = mmap(0, 4096,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, GPIO_BASE);

  int *gpe = (int *)(gpio_base + GPE);
  int old;
  for (;;)
    {
      if (*gpe != old)
        {
          old = *gpe;
          printf("%08x\n", *gpe);
        }
    }
  fflush(stdout);

  return 0;
}
