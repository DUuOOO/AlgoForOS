// SJF调度算法。假设有10个进程，每个进程的到达时间(1-20之间的整数)、需要的运行时间(10-50之间的整数)都是随机生产。
//模拟实现短作业优先调度算法SJF，结果输出这10个进程的执行顺序，并计算输出每个进程的等待时间以及总的平均等待时间。
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
// 每次选择已到达且运行时间最短的
void SJF(PCB pcbs[]) {	// PCB控制块
    printf("SJF调度算法:\n");
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
    pcbs[LastProPid].totalWaitTime = 0;  // 第一个运行进程的等待时间为0
    TotWT = pcbs[LastProPid].arrivalTime + pcbs[LastProPid].neededTime;    // 找出第一个运行的进程，计算运行完该进程真实时间=等待时间+运行时间
    pcbs[LastProPid].state = 'r';        //将进程程序设置为‘r',从就绪队列中删除
    printf("Process %d finished, waiting time is %d\n", pcbs[LastProPid].pid, pcbs[LastProPid].totalWaitTime);
    for (int i = 0; i < 9; i++) {   // 还有9个进程未运行
        bool ProArrive = true;     // 判断是否有进程已经在第一个进程运行过程中到达
        int MinNeededTime = 60;
        for (int j = 0; j < 10; j++) {                    // 在10个进程中进行搜索出已到达且未运行的、需要运行时间最短的进程
            if ((pcbs[j].arrivalTime <= TotWT) && (pcbs[j].state == 'w')) {  // 如果进程已到达且在就绪队列中
                ProArrive = false;
                if (pcbs[j].neededTime < MinNeededTime) {
                    MinNeededTime = pcbs[j].neededTime;
                    CurrentProPid = j;
                }
            }
        }
        if (ProArrive) {    //如果未到达，等待进程到达，运行最快到达的
            FastArrivalTime = 60;
            for (int j = 0; j < 10; j++) {
                if (pcbs[j].state == 'w') {
                    if (pcbs[j].arrivalTime < FastArrivalTime) {
                        FastArrivalTime = pcbs[j].arrivalTime;
                        CurrentProPid = j;
                    }
                }
            }
            TotWT = pcbs[CurrentProPid].arrivalTime + pcbs[CurrentProPid].neededTime; // 更新最新运行时间
            pcbs[CurrentProPid].totalWaitTime = 0;
        }
        else {             // 如果进程到达，执行最小运行时间进程
            pcbs[CurrentProPid].totalWaitTime = TotWT - pcbs[CurrentProPid].arrivalTime;
            TotWT = TotWT + pcbs[CurrentProPid].neededTime;                           // 总的真实时间
        }
        LastProPid = CurrentProPid;
        pcbs[CurrentProPid].state = 'r';
        printf("Process %d finished, waiting time is %d\n", pcbs[CurrentProPid].pid, pcbs[CurrentProPid].totalWaitTime);
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
    SJF(pcbs);
    return 0;
}