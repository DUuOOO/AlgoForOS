// 银行家算法
// 银行家算法
// 数据结构：
//（1）可用资源向量Available,这是一个一维数组Available[j],j=1,…m，表示第j种资源的可用数量，其中m为资源的种类个数
//（2）最大资源需求矩阵Max,这是一个n*m的二维数组，其中n为进程个数。单元Max[i,j]存储的数值表示第i个进程最多需要多少第j种资源
//（3）分配矩阵Allocation，这是一个n*m的二维数组。单元Allocation[i,j]存储的是已经分配给第i个进程的第j种资源的数量
//（4）需求矩阵Need，这也是一个n*m的矩阵，单元Need[i,j]存储的数值表示进程i还需要多少第j种资源的数量才能完成退出。
// 假设系统中有n=5个进程和m=3种资源。这m种资源每一种资源的最大可用数量Available[i],i=1,…,m，用随机数生成，
// 取值范围为[1,10]。每个进程i对资源j的最大需求Max[i,j],i=1,…,n,j=1,…m，也用随机数生成，其取值范围为从1到Available[j]。
// 初始分配矩阵Allocation[i,j]也用随机数生成，其中，有50%的概率Allocation[i,j]取值为0，有50%的概率Allocation[i,j]随机从1到Max[i,j]中取值。
// 如果在给第i个进程生成初始分配矩阵Allocation后，发现某种资源j的最大可用数量已经分配光了，
// 那么从第i+1个进程开始所有的进程都分配不到该资源j，也就是说Allocation[k,j]=0,k=i+1,…,n。
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int n = 5;        // 进程数
const int m = 3;        // 资源种类数
int Available[m];       // 每种资源的最大可用数量
int Max[n][m];          // 最大需求矩阵
int Allocation[n][m];   // 初始分配矩阵

void generateResourcesMaxMatrixAllocationMatrix() {                 // 初始化最大资源可用数量
    srand((unsigned)time(NULL));                    // 设置随机数种子
    for (int j = 0; j < m; j++) {
        Available[j] = rand() % 10 + 1;             // 随机生成每种资源的最大可用数量，取值范围为[1, 10]
        for (int i = 0; i < n; i++) {
            Max[i][j] = rand() % Available[j] + 1; // 随机生成最大需求矩阵，取值范围为[1, Available[j]]
                                                   // 初始化分配矩阵
            if (rand() % 2 == 1) {
                Allocation[i][j] = rand() % Max[i][j] + 1; // 50%的概率Allocation[i][j]随机从[1, Max[i][j]]中取值
            }
            else {
                Allocation[i][j] = 0;               // 50%的概率Allocation[i][j]取值为0
            }
        }
    }
}

int checkSafety() {
    int Work[m];                    // 可用资源向量
    int Finish[n];                  // 进程完成标志数组
    int TempAllocation[n][m];       // 临时分配矩阵，用于模拟资源分配过程
    int i, j;
    printf("Available:\n");
    for (int i = 0; i < m; i++) {   // 计算经过初始分配矩阵后，剩余的资源数目
        Available[i] = Available[i] - Allocation[0][i] - Allocation[1][i] - Allocation[2][i] - Allocation[3][i] - Allocation[4][i];
        printf("%d ", Available[i]);
    }
    printf("\nWork:\n");
    for (i = 0; i < m; i++) {       // 初始化Work数组,将剩余最大资源复制到work数组
        Work[i] = Available[i];
        printf("%d ", Work[i]);
    }

    for (i = 0; i < n; i++) {       // 初始化finish数组
        Finish[i] = 0;
    }
    printf("\nNeeded:\n");
    for (i = 0; i < n; i++) {       // 更新新的分配矩阵到临时分配矩阵
        for (j = 0; j < m; j++) {
            TempAllocation[i][j] =  Max[i][j] - Allocation[i][j];
            printf("%d ", TempAllocation[i][j]);
        }
        printf("\n");
    }
    int safeSequence[n];                        // 用于存储安全序列
    int count = 0;
    while (count < n) {
        bool found = false;                     // 判断每一轮能否找到能够被分配到资源的进程，若不存在，则系统处于不安全状态
        for (int i = 0; i < n; i++) {           // 寻找当前是否有可进行分配的序列
            if (Finish[i] == 0) {               // 若该进程未完成
                bool canExecute = true;          
                for (int j = 0; j < m; j++) {
                    if (Max[i][j] - TempAllocation[i][j] > Work[j]) {   // 若剩余资源大于需要资源，则进行分配，进程结束后释放资源
                        canExecute = false;
                        break;
                    }
                }
                if (canExecute) {               // 若可以进行分配，则运行进程，释放资源
                    for (int j = 0; j < m; j++) {
                        Work[j] += TempAllocation[i][j];
                    }
                    Finish[i] = 1;
                    safeSequence[count] = i;    // 记录进程i到安全序列
                    count++;
                    found = true;
                }
            }
        }
        if (!found) {                           // 如果当前系统未有可以分配的进程，则存在死锁
            return false;
        }
    }
    printf("安全序列: ");                       // 打印安全序列
    for (int i = 0; i < n; i++) {
        printf("%d ", safeSequence[i]);
    }
    printf("\n");

    return true;
}


int main() {
    generateResourcesMaxMatrixAllocationMatrix();

    printf("Initial Available:\n");
    for (int i = 0; i < m; i++) {
        printf("%d ", Available[i]);
    }
    printf("\n");

    printf("Max:\n");
    for (int i = 0; i < n; i++) {
        printf("p%d: ",i);
        for (int j = 0; j < m; j++) {
            printf("%d ", Max[i][j]);
        }
        printf("\n");
    }

    printf("Allocated:\n");
    for (int i = 0; i < n; i++) {
        printf("p%d: ", i);
        for (int j = 0; j < m; j++) {
            printf("%d ", Allocation[i][j]);
        }
        printf("\n");
    }

    if (checkSafety()) {
        printf("存在安全分配序列。\n");
    }
    else {
        printf("Deadlock。\n");
    }
    return 0;
}