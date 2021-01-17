#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define BUFFERSIZE 10

int buffer[BUFFERSIZE];

union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    ushort *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};
struct shmid_ds status_buffer;

void down(int sem, int sum_num)
{
    struct sembuf p_op;

    p_op.sem_num = sum_num;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &p_op, 1) == -1)
    {
        perror("Error in down()");
        exit(-1);
    }
}

void up(int sem, int sem_num)
{
    struct sembuf v_op;

    v_op.sem_num = sem_num;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &v_op, 1) == -1)
    {
        perror("Error in up()");
        exit(-1);
    }
}

union Semun semun;

int initSem(char keyNum, int size, int initial_value)
{
    int semKey = ftok("keyfile", keyNum);
    int semId = semget(semKey, size, 0666);

    if (semId == -1)
    {
        semId = semget(semKey, size, 0666 | IPC_CREAT);

        if (semId == -1)
        {
            perror("Error in create sem");
            exit(-1);
        }

        semun.val = initial_value; /* initial value of the semaphore, Binary semaphore */
        for (int i = 0; i < size; i++)
        {
            if (semctl(semId, i, SETVAL, semun) == -1)
            {
                perror("Error in semctl");
                exit(-1);
            }
        }
    }

    return semId;
}

short existedIdx = 0;
void *initShm(char key, int *id)
{
    key_t shmKey = ftok("keyfile", key);
    int shmId = shmget(shmKey, sizeof(int) * BUFFERSIZE, IPC_CREAT | 0666);

    if (!~shmId)
    {
        perror("cannot create shared memory\n");
        exit(-1);
    }

    *id = shmId;
    void *addr = shmat(shmId, (void *)0, 0);
    return addr;
}

void clearResources(int sigNum);

// memory sem: mutex
// producer sem: number of empty places
// consumer sem: number of existed items

int memorySem, producerSem, consumerSem, bufferMemory, idxMemory, idxSem;

int main()
{
    signal(SIGINT, clearResources);

    memorySem = initSem('m', 1, 1); // binary semaphore
    idxSem = initSem('s', 1, 1);    // binary semaphore
    producerSem = initSem('p', BUFFERSIZE, 1);
    consumerSem = initSem('c', BUFFERSIZE, 0);

    int *bufferAddr = initShm('b', &bufferMemory);
    int status = shmctl(bufferMemory, IPC_STAT, &status_buffer);
    int item = (status_buffer.shm_nattch - 1) * 100;

    int *idx = initShm('i', &idxMemory);
    status = shmctl(idxMemory, IPC_STAT, &status_buffer);
    int semNum = status_buffer.shm_nattch;

    while (1)
    {
        down(idxSem, 0);
        down(producerSem, *idx);
        down(memorySem, 0);

        // produce an item and write it to the buffer
        buffer[*idx] = item++;
        bufferAddr[*idx] = buffer[*idx];
        printf("producer %d, produced item: %d \n", semNum, bufferAddr[*idx]);

        *idx = ((*idx) + 1) % BUFFERSIZE;

        up(memorySem, 0);
        up(consumerSem, ((*idx) - 1 + BUFFERSIZE) % BUFFERSIZE);
        up(idxSem, 0);
    }

    clearResources(SIGINT);
    return 0;
}

void clearResources(int sigNum)
{
    int status = shmctl(bufferMemory, IPC_STAT, &status_buffer);
    if (status_buffer.shm_nattch == 1)
        system("ipcrm -a");

    signal(SIGINT, SIG_DFL);
    kill(getpid(), SIGINT);
}
