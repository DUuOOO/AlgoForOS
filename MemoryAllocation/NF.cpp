1.#include <stdlib.h>  
2.#include <stdio.h>  
3.#include <windows.h>  
4.#include <time.h>    
5.struct Block {  
6.    int id;                     // 分区序号  
7.    int size;                   // 分区大小  
8.    int startAddr;          // 分区开始的位置  
9.    bool status;             // 分区状态，true:空闲，false:被占用  
10.    int pid;                    // 如果该分区被占用，则存放占用进程的id, 否则为-1  
11.    struct Block* prev;         // 指向前面一块内存分区  
12.    struct Block* next;         // 指向后面一块内存分区  
13.};  
14.struct PCB {  
15.    int pid;                    // 进程序号  
16.    int neededMem;     // 需要的内存分区大小  
17.    int status;                // 内存分配成功为1；分配失败为-1  
18.    int blockID;            // 若分配成功，保存占用分区的id, 否则为-1  
19.    struct PCB* next;  // 指向下一个PCB  
20.};  
21.struct Queue {  
22.    struct PCB* list;    // 指向该队列中进程PCB链表的头部   
23.}queue;                       // 存储进程信息的队列  
24.  
25.struct Queue2 {  
26.    struct Block* list;        // 指向该队列中进程PCB链表的头部   
27.}queueblock;                    // 存储进程信息的队列  
28.int MaxNumberofProcess = 10;  
29.int MaxMemory = 1024;  
30.int startAdd = 0;  
31.Block blocks;                   // 初始化分区  
32.HANDLE generatorThread, allocateThread;  
33.HANDLE mutex, syncSemaphore;  
34.  
35.void enqueue(Queue* queue, PCB* process) {                      // 将新产生的进程放入队列  
36.    if (queue->list == NULL) {  
37.        process->next = NULL;  
38.        queue->list = process;  
39.    }  
40.    else {  
41.        PCB* lastProcess = queue->list;  
42.        while (lastProcess->next != NULL) {  
43.            lastProcess = lastProcess->next;  
44.        }  
45.        lastProcess->next = process;  
46.    }  
47.}  
48.PCB* dequeue(Queue* queue) {                                    // 取出队头第一个元素 先进先出  
49.    if (queue->list == NULL) {  
50.        return NULL;    // 队列为空  
51.    }  
52.    PCB* process = queue->list;  
53.    queue->list = process->next;  
54.    process->next = NULL;  
55.    return process;  
56.}  
57.PCB* create_process(int pid) {  
58.    PCB* new_process = (PCB*)malloc(sizeof(struct PCB));  
59.    if (new_process != NULL) {  
60.        new_process->pid = pid;  
61.        new_process->status = -1;  
62.        new_process->blockID = -1;  
63.        new_process->neededMem = rand() % 101 + 100;            // 内存大小随机在[100,200]  
64.        new_process->next = NULL;  
65.    }  
66.    printf("Create process %d, neededMemory is %d K\n", new_process->pid, new_process->neededMem);  
67.    return new_process;  
68.}  
69.DWORD WINAPI allocalateNF(LPVOID lpParam) {             // NF算法  
70.    int NumofBlock = 0;  
71.    Queue* queue = (Queue*)lpParam;  
72.    int NextIndexOfBlock = 0;                                   // 记录NF算法找的已分配的上一个空闲分区序号，第一次空闲分区分配从分区0开始，之后的更新分区序号标记器  
73.    WaitForSingleObject(generatorThread, INFINITE);               // 等待生成进程生成结束再打印信息  
74.    printf("Allocate process    Allocate size(K)    Block Id    StartAdrr   Remaind memory\n");// 等待生成进程生成结束再打印信息  
75.    while (NumofBlock < MaxNumberofProcess) {  
76.        WaitForSingleObject(mutex, INFINITE);  
77.        Block* blocks = queueblock.list;  
78.        while (blocks->id != NextIndexOfBlock) { // 根据标志读取到上一次分配成功的下一个空闲分区  
79.            blocks = blocks->next;  
80.        }  
81.        PCB* process = dequeue(queue);  
82.        ReleaseSemaphore(mutex, 1, NULL);  
83.        while (process != NULL) {  
84.            WaitForSingleObject(syncSemaphore, INFINITE);   
85.            Block* new_block = (Block*)malloc(sizeof(struct Block));  
86.            if (MaxMemory > process->neededMem) {// 判断剩余分区大小是否能够进行进程内存分配  
87.                bool allocatejudge = false;  
88.                while (blocks != NULL) {                       // 寻找可以进行内存分配的空闲分区  
89.                    if ((blocks->status == true) && (blocks->size > process->neededMem)) {  
90.                        allocatejudge = true;  
91.                        break;  
92.                    }  
93.                    blocks = blocks->next;  
94.                }  
95.                if (allocatejudge == true) {  
96.                    new_block->startAddr = blocks->startAddr;               // 切割空闲分区， 将新的空闲分区插入分区链表  
97.                    new_block->id = blocks->id;  
98.                    new_block->startAddr = blocks->startAddr;  
99.                    new_block->status = false;  
100.                    new_block->pid = process->pid;  
101.                    new_block->size = process->neededMem;  
102.  
103.                    process->blockID = new_block->id;          // 分配成功，更新进程信息  
104.                    process->status = 1;                                    // 进程分配状态改为成功  
105.                    blocks->startAddr = new_block->startAddr + new_block->size;// 修改剩余空闲分区开始地址  
106.                    blocks->size -= new_block->size;             // 修改剩余空闲分区内存空间  
107.  
108.                    MaxMemory -= new_block->size;            // 更新剩余内存  
109.  
110.                    Block* last_block = blocks->prev;            // 更新双向链表指针  
111.                    Block* next_block = blocks->next;  
112.                    if (last_block != NULL) {                          // 插入new_block  
113.                        last_block->next = new_block;  
114.                    }  
115.                    else {                                                  // 如果为NULL，说明blocks为队首元素  
116.                        queueblock.list = new_block;  
117.                    }  
118.                    blocks->prev = new_block;  
119.                    new_block->prev = last_block;  
120.                    new_block->next = blocks;  
121.  
122.                    blocks->startAddr = new_block->size + new_block->startAddr;  
123.  
124.                    while (blocks != NULL) {                                // 修改分区链表分区号  
125.                        blocks->id += 1;  
126.                        blocks->startAddr = blocks->prev->startAddr + blocks->prev->size;               // 修改分区区地址开始位置  
127.                        blocks = blocks->next;  
128.                    }  
129.                    WaitForSingleObject(generatorThread, INFINITE);         // 等待生成进程生成结束再打印信息  
130.                    printf("%8d %20d %14d %12d %12d\n", process->pid, process->neededMem, process->blockID, new_block->startAddr, MaxMemory);  
131.                    NumofBlock++;                                          // 进程处理完毕， 计数器+1  
132.                    NextIndexOfBlock = process->blockID + 1;               // 更新上一个成功分配的空闲分区的下一个空闲分区区号记录标志器  
133.                    break;  
134.                }  
135.                else {                                                     // 如果不能，则内存分配失败, 进程放到队尾  
136.                    WaitForSingleObject(generatorThread, INFINITE);         // 等待生成进程生成结束再打印信息  
137.                    printf("Process: %d Allocation failure!\n", process->pid);  
138.                    NumofBlock++;                                          // 进程处理完毕， 计数器+1  
139.                }  
140.            }  
141.            else {                                                         // 如果不能，则内存分配失败, 进程放到队尾  
142.                WaitForSingleObject(generatorThread, INFINITE);         // 等待生成进程生成结束再打印信息  
143.                printf("Process: %d Allocation failure!\n", process->pid);  
144.// 剩余最大内存不大于进程所需的内存，无法进行内存分配，计数器+1 
145.                NumofBlock++;  
146.            }  
147.            process = process->next;  
148.        }  
149.    }  
150.    return 0;  
151.}  
152.DWORD WINAPI generator(LPVOID lpParam) {  
153.    srand((unsigned)time(NULL));  // 初始化随机数种子，以确保每次运行得到的随机数不同  
154.    int numberOfProcesses = 0;  
155.    while (numberOfProcesses < MaxNumberofProcess) { // 生成10个进程  
156.        int randomSleepTime = rand() % 100 + 1;                 // 随机等待时间[1, 100] ms   
157.        Sleep(randomSleepTime);                                	   // 等待随机时间  
158.  
159.        // 创建新进程  
160.        PCB* new_process = create_process(numberOfProcesses + 1);  
161.        WaitForSingleObject(mutex, INFINITE);  
162.        enqueue(&queue, new_process);  
163.        ReleaseSemaphore(mutex, 1, NULL);  
164.        ReleaseSemaphore(syncSemaphore, 1, NULL);               // 通知主程序有程序产生  
165.        numberOfProcesses++;  
166.    }  
167.    return 0;  
168.}  
169.void init_queueblock() {  
170.    Block* new_block = (Block*)malloc(sizeof(struct Block));  
171.    new_block->id = 0;  
172.    new_block->next = NULL;  
173.    new_block->pid = -1;  
174.    new_block->prev = NULL;  
175.    new_block->startAddr = 0;  
176.    new_block->size = 1024;  
177.    new_block->status = true;  
178.    queueblock.list = new_block;  
179.}  
180.void cleanup() {                                        // 释放队列中的所有进程资源  
181.    Block* blocks = queueblock.list;  
182.    printf("Recall block id     Size(K)\n");  
183.    while (blocks != NULL) {  
184.        Block* last_block = blocks->prev;  
185.        if (blocks->status == false) {  
186.            blocks->status = true;                      // 回收分区资源，修改状态  
187.            blocks->pid = -1;  
188.            printf("%8d %15d\n", blocks->id, blocks->size);  
189.            if (last_block != NULL && last_block->status == true) {            // 向前合并分区资源  
190.                last_block += blocks->size;  
191.                last_block->next = blocks->next;        // 修改双向链表指向  
192.                last_block->next->prev = last_block;  
193.                free(blocks);  
194.                blocks = last_block->next;              // 跳转到下一个分区块  
195.            }  
196.  
197.        }  
198.        else {  
199.            blocks = blocks->next;                          // 跳转到下一个分区块  
200.        }  
201.    }  
202.}  
203.int main()  
204.{  
205.    init_queueblock();  
206.    mutex = CreateSemaphore(NULL, 1, 1, NULL);                                  // 创建互斥信号量  
207.    syncSemaphore = CreateSemaphore(NULL, 0, MaxNumberofProcess, NULL);         // 创建同步信号量  
208.    generatorThread = CreateThread(NULL, 0, generator, NULL, 0, NULL);          // 创建进程产生器线程       
209.    allocateThread = CreateThread(NULL, 0, allocalateNF, &queue, 0, NULL);        // 创建进程产生器内存分配线程  
210.    WaitForSingleObject(generatorThread, INFINITE);  
211.    WaitForSingleObject(allocateThread, INFINITE);  
212.    cleanup();                                                                  // 释放占用的分区块  
213.    CloseHandle(generatorThread);                              // 释放资源，关闭句柄      
214.    CloseHandle(allocateThread);  
215.    CloseHandle(syncSemaphore);  
216.    CloseHandle(mutex);  
217.}  
