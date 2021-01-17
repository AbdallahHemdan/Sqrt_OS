#include "headers.h"

/* Modify this file as needed*/
int *remainingtime;
int shmProcessRemainingTimeId;

int main(int agrc, char *argv[])
{
    initClk();
    int remainingTimeSem = initSem('R');

    remainingtime = (int *)initShm(shmProcessKey, &shmProcessRemainingTimeId);
    int last = -1; // -1 => just to enter in the first time

    while (*remainingtime > 0)
    {
        // down(remainingTimeSem);
        while (last == getClk())
            ;
        if (last != getClk())
        {
            (*remainingtime)--;
            last = getClk();
        }
        // up(remainingTimeSem);
    }

    destroyClk(false);

    return 0;
}
