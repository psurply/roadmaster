#ifndef GPIO_H
#define GPIO_H

#define GPIO_MAP_SIZE        4094UL
#define GPIO_MAP_MASK        (MAP_SIZE - 1)

#ifdef MINI6410
# define GPIO_BASE       0x7F008000
# define GPIO_OFFSET     0x84
# define GPIO_DEFAULT    0x1F
# define BIT_SENSOR      2
# define BIT_START       4
# define BIT_RESET       8
#else
# define GPIO_BASE       0x56000000
# define GPIO_OFFSET     0x64
# define GPIO_DEFAULT    0xEEFF
# define BIT_SENSOR      (1 << 11)     // 0xE6FF
# define BIT_START       (1 << 9)      // 0xECFF
# define BIT_RESET       (1 << 3)      // 0xEEF7
#endif

// green: ecff
// red: eef7

#define TEST_BIT(Prev, Current, Mask)    \
  (~(Prev) & Mask) == 0 &&               \
    (~(Current) & Mask) != 0

struct gpio
{
    char *base;
    char *addr;
};

void gpio_init(struct gpio *gpio);
void gpio_clean(struct gpio *gpio);
unsigned long gpio_get(struct gpio *gpio);

#endif /* GPIO_H */
