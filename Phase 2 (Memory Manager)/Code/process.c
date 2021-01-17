#include "headers.h"

/* Modify this file as needed*/
int *remainingtime;
int shmProcessRemainingTimeId;

int main(int agrc, char *argv[])
{
    initClk();

    int last = -1; // -1 => just to enter in the first time
    remainingtime = (int *)initShm(shmProcessKey, &shmProcessRemainingTimeId);

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
