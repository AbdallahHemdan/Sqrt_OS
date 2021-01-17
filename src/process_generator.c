#include "headers.h"

void clearResources(int);
int *terminate, msqProcessId;
int shmTerminateId;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    FILE *input;
    int index = 0;
    size_t len = 0;
    char *line = NULL;
    int numberOfProcesses = 0;

    input = fopen("processes.txt", "r");
    while (getline(&line, &len, input) != -1)
        if (line[0] != '#')
            numberOfProcesses++;

    fclose(input);
    input = fopen("processes.txt", "r");
    Process *processes = (Process *)malloc(numberOfProcesses * sizeof(Process));

    while (getline(&line, &len, input) != -1)
    {
        fscanf(input, "%d", &(processes[index].id));
        fscanf(input, "%d", &(processes[index].arrivalTime));
        fscanf(input, "%d", &(processes[index].executaionTime));
        fscanf(input, "%d", &(processes[index].priority));
        fscanf(input, "%d", &(processes[index].processSize));

        processes[index].lastProcess = false;
        strcpy(processes[index].text, "not End");
        processes[index].remainingTime = processes[index].executaionTime;

        if (index == numberOfProcesses - 1)
            processes[index].lastProcess = true;

        index++;
    }

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    char args[2][2];
    char algorithmNumber[2];

    printf("Please enter:\n1 to choose HPF\n2 to choose SRTN\n3 to choose RR\n");
    scanf("%s", args[0]);

    if (!strcmp(args[0], "3"))
    {
        printf("You choose RR algorithm please enter the quantum\n");
        scanf("%s", args[1]);
    }
    else
        strcpy(args[1], "0");

    /* 3. Initiate and create the scheduler and clock processes.
     *   Note: look at compileAndRun funciton in the header. (just send the file name without .c)
    */
    msqProcessId = initMsgq(msqProcessKey);
    terminate = (int *)initShm(terminateKey, &shmTerminateId);
    *terminate = false;

    int clkID = fork();
    if (clkID == 0)
        compileAndRun("clk", NULL, NULL);

    int schedID = fork();
    if (schedID == 0)
        compileAndRun("scheduler", args[0], args[1]);

    // 4. Use this function after creating the clock process to initialize clock
    initClk();

    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    index = 0;
    int last = -1;
    while (!(*terminate))
    {
        int x = getClk(); // 0
        while (last != x)
        {
            last = getClk();
            while (processes[index].arrivalTime == x)
            {
                sendMessage(processes[index], msqProcessId);
                index++;
            }
            Process end;
            strcpy(end.text, "End");
            sendMessage(end, msqProcessId);
        }
    }

    // 7. Clear clock resources
    msgctl(msqProcessId, IPC_RMID, (struct msqid_ds *)0);
    shmctl(shmTerminateId, IPC_RMID, NULL);
    destroyClk(true);
}

void clearResources(int signum)
{
    msgctl(msqProcessId, IPC_RMID, (struct msqid_ds *)0);
    shmctl(shmTerminateId, IPC_RMID, NULL);
    signal(SIGINT, SIG_DFL);
    // kill(getpid(), SIGINT);
}
