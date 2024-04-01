// HRRN调度算法
// 假设有10个进程，每个进程的到达时间(1-20之间的整数)、需要的运行时间(10-50之间的整数)都是随机生产。
// 模拟实现最高响应比优先调度算法HRRN，结果输出这10个进程的执行顺序，并计算输出每个进程的等待时间以及总的平均等待时间。
#include "stdio.h"              //包含输入输出函数
#include "stdlib.h"             //包含随机数产生函数
#include "time.h"               //与时间有关的函数头文件
#include "windows.h"          
#include "stdbool.h"
#include <string>
struct PCB {
    int pid;       //进程标识符，即进程的名字
                        //以下部分为用于进程调度的信息
    char state;         //‘r’: 运行状态；‘w’:就绪状态
    float priority;       //进程优先级
    int neededTime;     //进程需要的运行时间 
    int arrivalTime;
    int totalWaitTime;  //进程累计已等待的CPU时间 
                        //以下部分为进程的控制信息
};
int compareArrivalTime(const void* p1, const void* p2) {
    const struct PCB* process1 = (const struct PCB*)p1;
    const struct PCB* process2 = (const struct PCB*)p2;

    return process1->arrivalTime - process2->arrivalTime;
}
// 响应比 = 作业周转时间/作业处理时间 =（作业等待时间 + 作业处理时间） / 作业处理时间
void HRRN(PCB pcbs[]) {
    printf("HRRN调度算法:\n");
    int FastArrivalTime = 60;
    int TotWT = 0;
    int LastProPid;
    int CurrentProPid;
    for (int i = 0; i < 10; i++) {      // 找出第一个到达的进程运行
        if (pcbs[i].arrivalTime < FastArrivalTime) {
            FastArrivalTime = pcbs[i].arrivalTime;
            LastProPid = i;
        }
    }
    pcbs[LastProPid].totalWaitTime = pcbs[LastProPid].arrivalTime;  // 第一个运行进程的等待时间为0
    TotWT = pcbs[LastProPid].totalWaitTime + pcbs[LastProPid].neededTime;    // 找出第一个运行的进程，计算总共等待时间=等待时间+运行时间
    pcbs[LastProPid].state = 'r';        //将进程程序设置为‘r',从就绪队列中删除
    printf("Process %d finished, waiting time is %d\n", pcbs[LastProPid].pid, 0);
    // 运行完最先到达的程序后 计算每次的最优响应比 决定哪个程序先运行
    for (int j = 0; j < 9; j++) {           // 还有9个进程未运行
        float MaxPripro = 0.0;
        printf("    Response Ratio of remaining process (pid:Response Ratio):\n    ");
        for (int i = 0; i < 10; i++) {      // 计算各个未运行的进程的响应比, 响应比越高，越先运行程序
            if (pcbs[i].state == 'w') {
                pcbs[i].priority = (TotWT - pcbs[i].arrivalTime) / (pcbs[i].neededTime * 1.0);
                printf("%d:%3f | ", pcbs[i].pid,pcbs[i].priority);
                if (pcbs[i].priority > MaxPripro) {
                    MaxPripro = pcbs[i].priority;
                    CurrentProPid = i;
                }
            }
        }
        pcbs[CurrentProPid].state = 'r';    //将进程程序设置为‘r',从就绪队列中删除
        pcbs[CurrentProPid].totalWaitTime = pcbs[LastProPid].totalWaitTime + pcbs[LastProPid].neededTime - pcbs[CurrentProPid].arrivalTime;
        TotWT = pcbs[CurrentProPid].totalWaitTime + pcbs[CurrentProPid].neededTime;
        LastProPid = CurrentProPid;        
        printf("\nProcess %d finished, waiting time is %d, Max Response Ratio: %.3f \n", pcbs[CurrentProPid].pid, pcbs[CurrentProPid].totalWaitTime, pcbs[CurrentProPid].priority);
    }
    printf("All Processed are completed!\n");
    printf("Average waiting time is %.4f", TotWT / 10.0);
}
int main(int argc, wchar_t* argv[])
{
    srand((unsigned)time(NULL)); //用当前时间做随机数种子，每次运行rand()时产生的随机数序列都不相同
    PCB pcbs[10];
    for (int i = 0; i < 10; i++) {
        pcbs[i].pid = i;
        pcbs[i].neededTime = rand() % 50 + 1;
        pcbs[i].arrivalTime = rand() % 20 + 1;
        pcbs[i].state = 'w';
        printf("Generator: Process %d is generated, neededTime = %d, arrivalTime = %d\n", i, pcbs[i].neededTime, pcbs[i].arrivalTime);
    }
    HRRN(pcbs);
    return 0;
}