#include "headers.h"

void HPF();
void SRTN();
void RR();

struct curProcess
{
    Process process;
    int runningTime;
};

int main(int argc, char *argv[])
{
    initClk();

    int algorithm = stoi(argv[1]);

    // TODO implement the scheduler :)
    // upon termination release the clock resources.

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

bool checkNewProcesses(Process *newProcess)
{
    /* 1. check the message queue for new processes
     *  1.1 if True: update newProcess parameter and return true
     *  1.2 else: return false;
     */
    *newProcess = receiveMessage();

    return (*newProcess).text != "End";
}

void HPF()
{
    // 1. declare all needed variables
    bool lastProcess = 0;
    int lastSecond = -1;
    Process running;
    Node *pq;

    // 2. initilization
    initilize(&pq);
    running.remainingTime = 0;

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 contiune working
    */
    while (!lastProcess || isEmpty(pq))
    {
        while (lastSecond == getClk())
            ;

        lastSecond = getClk();

        Process receivedProcess;

        while (1) // get the new processes
        {
            bool newProcess = checkNewProcesses(&receivedProcess);

            if (!newProcess)
                break;
            else
            {
                push(&pq, receivedProcess, receivedProcess.priority);
            }
        }

        //
    }

    printf("Nice work\nMade with love ❤");
}

void SRTN()
{
    // Same as HPF
}

void RR()
{
    // Same as HPF with queue instead of priority queue
}