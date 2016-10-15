#include <stdio.h>

#include "roadmaster.h"
#include "log.h"

int main(void)
{
    struct roadmaster r;
    roadmaster_init(&r);
    roadmaster_run(&r);

    return 0;
}
