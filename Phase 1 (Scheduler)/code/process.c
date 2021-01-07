#include "headers.h"

/* Modify this file as needed*/
int *remainingtime, *shmId;

int main(int agrc, char *argv[])
{
    initClk();

    //TODO it needs to get the remaining time from somewhere

    *shmId = (int *)initShm(shmProcessKey);

    remainingtime = *shmId;
    int last = -1; // -1 => just to enter in the first time
    while (remainingtime > 0)
    {
        if (last != getclk())
        {
            remainingtime--;
            last = getclk();
        }
    }

    destroyClk(false);

    return 0;
}
