// 多级反馈队列调度算法
// 假设有5个运行队列，它们的优先级分别为1，2，3，4，5，它们的时间片长度分别为10ms,20ms,40ms,80ms,160ms，
// 即第i个队列的优先级比第i-1个队列要低一级，但是时间片比第i-1个队列的要长一倍。
// 多级反馈队列调度算法包括四个部分：主程序main，进程产生器generator，进程调度器函数scheduler，进程运行器函数executor。
// 结果输出：在进程创建、插入队列、执行时的相关信息，并计算输出总的平均等待时间。其中，generator用线程来实现，
// 每隔一个随机时间(例如在[1,100]ms之间)产生一个新的进程PCB，并插入到第1个队列的进程链表尾部。
// Scheduler依次探测每个队列，寻找进程链表不为空的队列，然后调用Executor, executor把该队列进程链表首部的进程取出来执行。
// 要设置1个互斥信号量来实现对第1个队列的互斥访问，因为generator和executor有可能同时对第1个队列进行操作。
// 同时要设置1个同步信号量，用于generator和scheduler的同步：generator每产生1个新进程，就signal一次这个同步信号量；
// 只有所有队列不为空时，scheduler才会运行，否则scheduler要等待这个同步信号量。
// 当所有进程运行完毕后，scheduler退出，主程序结束。
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
// 要求在进程创建时、执行时、进程在队列间移动时、进程运行结束时都输出相应的信息，例如进程的到达时间、
// 进程运行了多长的时间片，进程从被移动到了第几个队列，进程运行结束等信息。

struct PCB {
    DWORD pid;       //进程标识符，即进程的名字
    //以下部分为用于进程调度的信息
    char state;         //‘r’: 运行状态；‘w’:就绪状态，‘b’:阻塞状态
    int priority;       //进程优先级
    int arrivalTime;    //进程的创建时间(到达时间)
    int neededTime;     //进程需要的运行时间 
    int usedTime;       //进程已累计运行的时间
    int totalWaitTime;  //进程已等待的CPU时间总和 
    //以下部分为进程的控制信息
    struct PCB* next;   //指向下一个PCB的链表指针
};
struct Queue {
    int priority;       //该队列的优先级
    int timeSlice;      //该队列的时间片长度
    struct PCB* list;   //指向该队列中进程PCB链表的头部 
}queues[5];             // 5个队列 
HANDLE mutex;           // 互斥信号量
HANDLE syncSemaphore;   // 同步信号量
HANDLE generatorThread, schedulerThread;
int MaxNumberofProcess = 20;

void init_queues() {
    for (int i = 0; i < 5; ++i) {
        queues[i].priority = i + 1;
        queues[i].timeSlice = 10 * int(pow(2, i));
        queues[i].list = NULL;
    }
}

void enqueue(Queue* queue, PCB* process) {
    if (queue->list == NULL) {
        process->next = NULL;
        queue->list = process;
    }
    else {
        PCB* lastProcess = queue->list;
        while (lastProcess->next != NULL) {
            lastProcess = lastProcess->next;
        }
        //process->arrivalTime = GetTickCount();        // 设置到达时间为当前系统时间
        lastProcess->next = process;
    }
    if (process->priority == 1) {
        printf("Process %d arrive timestamp is %d\n", process->pid, process->arrivalTime);
    }
}

PCB* dequeue(Queue* queue) {                            // 取出队头第一个元素 先进先出
    if (queue->list == NULL) {
        return NULL;    // 队列为空
    }
    PCB* process = queue->list;
    queue->list = process->next;
    process->next = NULL;
    return process;
}

PCB* create_process(int priority, int pid) {
    PCB* new_process = (PCB*)malloc(sizeof(struct PCB));
    if (new_process != NULL) {
        new_process->pid = pid;
        new_process->priority = priority;
        new_process->arrivalTime = GetTickCount();
        new_process->neededTime = rand() % 199 + 2;      
        new_process->state = 'w';
        new_process->usedTime = 0;
        new_process->totalWaitTime = 0;
        new_process->next = NULL;
    }
    printf("Create process %d, priority is %d, neededTime is %d ms, and state is %c\n", new_process->pid, priority, new_process->neededTime, new_process->state);
    return new_process;
}
void executor(PCB* process) {
    int timeSlice = queues[process->priority - 1].timeSlice;       // 获取对应队列的时间片长度
    if(process->priority==1)  WaitForSingleObject(mutex, INFINITE); // 等待互斥信号量，保护对队列的访问;
    process->state = 'r';
    printf("Executing process %d with priority %d for %d ms, state is %c\n", process->pid, process->priority, timeSlice, process->state);
    Sleep(timeSlice);                             // 模拟进程执行，使用Sleep函数,执行队列时间片时间

    // 更新进程的等待时间
    for (int i = 0; i < 5; ++i) {
        PCB* currentProcess = queues[i].list;
        while (currentProcess != NULL) {
            currentProcess->totalWaitTime += timeSlice;
            currentProcess = currentProcess->next;
        }
    }
    if(process->priority == 1) ReleaseSemaphore(mutex, 1, NULL); // 释放互斥信号量;
}
DWORD WINAPI scheduler(LPVOID lpParam) {
    int i = 0;
    int number = 0;
    while (number < MaxNumberofProcess) {
        while (i < 5) {                                          // 遍历五个队列
            WaitForSingleObject(syncSemaphore, INFINITE);        // 等待同步信号量
            WaitForSingleObject(mutex, INFINITE);                // 等待互斥信号量，保护对队列的访问
            PCB* process = dequeue(&queues[i]);                  // 从队列中取出进程
            ReleaseSemaphore(mutex, 1, NULL);
            if (process != NULL) {                               // 执行进程
                executor(process);
                // 更新进程信息
                process->usedTime += queues[i].timeSlice;
                process->totalWaitTime += GetTickCount() - process->arrivalTime;
                if (process->usedTime < process->neededTime) {
                    process->state = 'b';
                    printf("The process %d priority is %d, neededTime is %d ms, and state is %c\n", process->pid, process->priority, process->neededTime, process->state);
                    // 时间片用完但进程还未完成，将进程移动到下一级队列的尾部
                    process->priority += 1;
                    enqueue(&queues[i + 1], process);
                    printf("--The process %d move to the next queue, priority is %d, used time is %d ms\n", process->pid, process->priority, process->usedTime);
                    ReleaseSemaphore(syncSemaphore, 1, NULL);    // 释放同步信号量
                }
                else {                                           // 进程执行完毕，释放进程资源
                    printf("The process %d finish! Priority is %d, neededTime is %d ms, and state is %c\n", process->pid, process->priority, process->neededTime, process->state);
                    number++;
                    free(process);
                }
                break;
            }
            else {                                              // 队列为空，继续下一个队列
                i++;
                ReleaseSemaphore(syncSemaphore, 1, NULL);       // 释放同步信号量
            }

        }
        //ReleaseSemaphore(syncSemaphore, 1, NULL);             // 释放同步信号量
    }
    return 0;
}
DWORD WINAPI generator(LPVOID lpParam) {
    srand((unsigned)time(NULL));                                // 初始化随机数种子，以确保每次运行得到的随机数不同
    int numberOfProcesses = 0;
    while (numberOfProcesses < MaxNumberofProcess) {            // 生成20个进程
        WaitForSingleObject(mutex, INFINITE);                   // 等待互斥信号量，保护对第一个队列的访问
        int randomSleepTime = rand() % 100 + 1;                 // 随机等待时间[1, 100] ms 
        Sleep(randomSleepTime);                                 // 等待随机时间

        // 创建新进程
        PCB* new_process = create_process(1, numberOfProcesses + 1);           // 优先级初始为1
        enqueue(&queues[0], new_process);                       // 插入到第一个队列尾部
        numberOfProcesses++;
        ReleaseSemaphore(mutex, 1, NULL);                       // 释放互斥信号量
        ReleaseSemaphore(syncSemaphore, 1, NULL);               // 通知主程序有新进程产生
    }
    return 0;
}
void cleanup() {
    // 释放队列中的所有进程资源
    for (int i = 0; i < 5; ++i) {
        while (queues[i].list != NULL) {
            WaitForSingleObject(mutex, INFINITE);       // 等待互斥信号量
            PCB* process = dequeue(&queues[i]);
            free(process);
            ReleaseSemaphore(mutex, 1, NULL);           // 释放互斥信号量 
        }
    }
}
int main(int argc, wchar_t* argv[]) {
    init_queues();                                                              // 初始化队列
    mutex = CreateSemaphore(NULL, 1, 1, NULL);                                  // 创建互斥信号量
    syncSemaphore = CreateSemaphore(NULL, 0, MaxNumberofProcess, NULL);                          // 创建同步信号量
    generatorThread = CreateThread(NULL, 0, generator, NULL, 0, NULL);          // 创建进程产生器线程    
    schedulerThread = CreateThread(NULL, 0, scheduler, NULL, 0, NULL);          // 创建调度器线程    
    WaitForSingleObject(generatorThread, INFINITE);
    WaitForSingleObject(schedulerThread, INFINITE);
    cleanup();
    CloseHandle(mutex);                                 // 释放资源，关闭句柄
    CloseHandle(syncSemaphore);
    CloseHandle(generatorThread);
    CloseHandle(schedulerThread);
    return 0;
}