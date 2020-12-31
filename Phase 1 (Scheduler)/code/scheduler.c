#include "headers.h"

void HPF();
void SRTN();
void RR();

int main(int argc, char *argv[])
{
    initClk();

    int algorithm = stoi(argv[1]);

    //TODO implement the scheduler :)
    //upon termination release the clock resources.

    switch (algorithm)
    {
    case 1:
        HPF();
        break;
    case 2:
        SRTN();
        break;
    case 3:
        RR();
        break;
    }

    destroyClk(true);
}

bool checkNewProcesses(/* process *newProcess */)
{
    /* 1. check the message queue for new processes
     *  1.1 if True: update newProcess parameter and return true
     *  1.2 else: return false;
     */
}

void HPF()
{
    // 1. initilize prioirty queue

    /* 2. start working with processes
     *  2.1 check every second if there is new processes (consuming)
     *  2.2 push the new processes if there is any
     *  2.2 contiune working
    */
}

void SRTN()
{
    // Same as HPF
}

void RR()
{
    // Same as HPF with queue instead of priority queue
}