#include "headers.h"

void HPF();
void SRTN();
void RR();

int msqProcessId, *shmId, *terminate;
int shmProcessRemainingTimeId, shmTerminateId;

int main(int argc, char *argv[])
{
    initClk();
    int algorithm = atoi(argv[0]);
    int quantum = atoi(argv[1]);

    msqProcessId = initMsgq(msqProcessKey);
    shmId = (int *)initShm(shmProcessKey, &shmProcessRemainingTimeId);
    terminate = (int *)initShm(terminateKey, &shmTerminateId);
    *terminate = false;

    switch (algorithm)
    {
    case 1:
        HPF();
        break;
    case 2:
        SRTN();
        break;
    case 3:
        RR(quantum);
        break;
    }
    *terminate = true;
    printf("Nice work Made with love ❤\n");
    shmctl(shmProcessRemainingTimeId, IPC_RMID, NULL);
    // TODO: upon termination release the clock resources.
    destroyClk(true);
}

bool checkNewProcess(Process *newProcess)
{
    /* 1. check the message queue for new processes
     *  1.1 if True: update newProcess parameter and return true
     *  1.2 else: return false;
     */
    *newProcess = receiveMessage(msqProcessId);
    return strcmp((*newProcess).text, "End");
}

void HPF()
{
    printf("HPF started\n");

    // 1. declare all needed variables
    bool lastProcess = 0;
    int lastSecond = -1;
    Process running;
    Node *pq;

    // 2. initilization
    initializePQ(&pq);
    running.remainingTime = 0;
    printf("At time x process y started arr w total z remain y wait k\n");
    *shmId = -1;

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 contiune working
    */
    while (!(lastProcess && isEmptyPQ(&pq) && *shmId == -1))
    {
        Process receivedProcess;
        while (1) // get the new processes
        {
            bool newProcess = checkNewProcess(&receivedProcess);
            if (!newProcess)
                break;
            else
            {
                lastProcess = receivedProcess.lastProcess;
                push(&pq, receivedProcess, receivedProcess.priority);
            }
        }

        if (*shmId == 0) // the running process has finished
        {
            printf("At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(),
                   running.id, running.arrivalTime, running.executaionTime,
                   (getClk() - running.arrivalTime) - running.executaionTime,
                   getClk() - running.arrivalTime, (getClk() - running.arrivalTime) * 1.0 / running.executaionTime);
            *shmId = -1;
        }

        if (!isEmptyPQ(&pq) && *shmId == -1)
        {
            running = pop(&pq);
            *shmId = running.executaionTime + 1;
            int processID = fork();

            if (processID == 0)
            {
                printf("At time %d process %d started arr %d total %d remain %d wait %d\n",
                       getClk(), running.id, running.arrivalTime, running.executaionTime,
                       running.executaionTime, getClk() - running.arrivalTime);

                compileAndRun("process", NULL, NULL);
            }
        }

        while (lastSecond == getClk())
            ;

        lastSecond = getClk();
    }
}

void SRTN()
{
    printf("SRTN started\n");

    // 1. declare all needed variables
    Node *pq;
    Process running;
    int lastSecond = -1;
    bool lastProcess = 0;

    // 2. initilization
    *shmId = -1;
    initializePQ(&pq);
    running.remainingTime = 0;
    printf("At time x process y started arr w total z remain y wait k\n");

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 contiune working
    */
    while (!(lastProcess && isEmptyPQ(&pq) && *shmId == -1))
    {
        Process receivedProcess;
        while (1) // get the new processes
        {
            bool newProcess = checkNewProcess(&receivedProcess);
            if (!newProcess)
                break;
            else
            {
                lastProcess = receivedProcess.lastProcess;
                push(&pq, receivedProcess, receivedProcess.remainingTime);
            }
        }

        if (*shmId == 0) // the running process has finished
        {
            printf("At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(),
                   running.id, running.arrivalTime, running.executaionTime, (getClk() - running.arrivalTime) - running.executaionTime,
                   getClk() - running.arrivalTime, (getClk() - running.arrivalTime) * 1.0 / running.executaionTime);
            *shmId = -1;
        }

        if (!isEmptyPQ(&pq))
        {
            Process top = pop(&pq);

            if (top.remainingTime < *shmId)
            {
                running.remainingTime = *shmId;
                push(&pq, running, running.remainingTime);
                *shmId = -1;

                kill(running.pid, SIGSTOP);
                printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n",
                       getClk(), running.id, running.arrivalTime, running.executaionTime,
                       running.remainingTime, getClk() - running.arrivalTime - (running.executaionTime - running.remainingTime));

                while (lastSecond == getClk())
                    ;

                lastSecond = getClk();
            }

            if (*shmId == -1)
            {
                *shmId = top.remainingTime + 1;
                running = top;

                if (top.remainingTime < top.executaionTime) //Cont
                {
                    kill(running.pid, SIGCONT);
                    printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n",
                           getClk(), running.id, running.arrivalTime, running.executaionTime,
                           running.remainingTime, getClk() - running.arrivalTime - (running.executaionTime - running.remainingTime));
                }
                else
                {
                    running.pid = fork();

                    if (running.pid == 0) // start a new process
                    {
                        printf("At time %d process %d started arr %d total %d remain %d wait %d\n",
                               getClk(), running.id, running.arrivalTime, running.executaionTime,
                               running.executaionTime, getClk() - running.arrivalTime);

                        compileAndRun("process", NULL, NULL);
                    }
                }
            }
            else // push it again into the pq
            {
                push(&pq, top, top.remainingTime);
            }
        }

        while (lastSecond == getClk())
            ;

        lastSecond = getClk();
    }
}

void RR(int quantum)
{
    printf("RR started\n");

    // 1. declare all needed variables
    Node *pq;
    Process running;
    bool lastProcess = 0;
    int lastSecond = -1, quantumCnt = 0;

    // 2. initilization
    *shmId = -1;
    queue *q;
    q = malloc(sizeof(queue));
    initialize(q);
    running.remainingTime = 0;
    printf("At time x process y started arr w total z remain y wait k\n");

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 contiune working
    */
    while (!(lastProcess && isEmpty(q) && *shmId == -1))
    {
        quantumCnt--;
        Process receivedProcess;

        while (1) // get the new processes
        {
            bool newProcess = checkNewProcess(&receivedProcess);
            if (!newProcess)
                break;
            else
            {
                lastProcess = receivedProcess.lastProcess;
                enqueue(q, receivedProcess);
            }
        }

        if (*shmId == 0) // the running process has finished
        {
            printf("At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(),
                   running.id, running.arrivalTime, running.executaionTime,
                   (getClk() - running.arrivalTime) - running.executaionTime,
                   getClk() - running.arrivalTime, (getClk() - running.arrivalTime) * 1.0 / running.executaionTime);
            *shmId = -1;
            quantumCnt = quantum;
        }

        if (!isEmpty(q) && (*shmId <= 0 || quantumCnt == 0))
        {
            quantumCnt = quantum;

            // stop current running process
            if (*shmId != -1)
            {
                running.remainingTime = *shmId;
                enqueue(q, running);
                *shmId = -1;

                kill(running.pid, SIGSTOP);
                printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n",
                       getClk(), running.id, running.arrivalTime, running.executaionTime,
                       running.remainingTime, getClk() - running.arrivalTime - (running.executaionTime - running.remainingTime));
            }

            Process top = dequeue(q);

            *shmId = top.remainingTime + 1;
            running = top;

            if (top.remainingTime < top.executaionTime) //cont
            {
                kill(running.pid, SIGCONT);
                printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n\n",
                       getClk(), running.id, running.arrivalTime, running.executaionTime,
                       running.remainingTime, getClk() - running.arrivalTime - (running.executaionTime - running.remainingTime));
            }
            else
            {
                running.pid = fork();

                if (running.pid == 0) //start
                {
                    printf("At time %d process %d started arr %d total %d remain %d wait %d\n\n",
                           getClk(), running.id, running.arrivalTime, running.executaionTime,
                           running.executaionTime, getClk() - running.arrivalTime);

                    compileAndRun("process", NULL, NULL);
                }
            }
        }

        while (lastSecond == getClk())
            ;
        lastSecond = getClk();
    }
}
