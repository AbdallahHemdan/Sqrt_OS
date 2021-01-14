#include "headers.h"

/* Modify this file as needed*/
int *remainingtime;

int main(int agrc, char *argv[])
{
    initClk();

    //TODO it needs to get the remaining time from somewhere

    remainingtime = (int *)initShm(shmProcessKey, remainingtime);
    int last = -1; // -1 => just to enter in the first time
    while (*remainingtime > 0)
    {
        while (last == getClk())
            ;
        if (last != getClk())
        {
            (*remainingtime)--;
            last = getClk();
        }
    }

    destroyClk(false);

    return 0;
}
