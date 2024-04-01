1.#include <stdlib.h>  
2.#include <stdio.h>  
3.#include <windows.h>  
4.#include <time.h>  
5.struct Block {  
6.    int id;                     	// 分区序号  
7.    int size;                  	// 分区大小  
8.    int startAddr;         	// 分区开始的位置  
9.    bool status;            	// 分区状态，true:空闲，false:被占用  
10.    int pid;                   	// 如果该分区被占用，则存放占用进程的id, 否则为-1  
11.    struct Block* prev;      // 指向前面一块内存分区  
12.    struct Block* next;      // 指向后面一块内存分区  
13.};  
14.struct PCB {  
15.    int pid;                       // 进程序号  
16.    int neededMem;           // 需要的内存分区大小  
17.    int status;                   // 内存分配成功为1；分配失败为-1  
18.    int blockID;                 // 若分配成功，保存占用分区的id, 否则为-1  
19.    struct PCB* next;         // 指向下一个PCB  
20.};  
21.struct Queue {  
22.    struct PCB* list;          // 指向该队列中进程PCB链表的头部   
23.}queue;                           // 存储进程信息的队列  
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
35.void enqueue(Queue* queue, PCB* process) { // 将新产生的进程放入队列  
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
48.PCB* dequeue(Queue* queue) {  // 取出队头第一个元素 先进先出  
49.    if (queue->list == NULL) {  
50.        return NULL;                       // 队列为空  
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
63.        new_process->neededMem = rand() % 101 + 100;// 内存大小随机在[100,200]  
64.        new_process->next = NULL;  
65.    }  
66.    printf("Create process %d, neededMemory is %d K\n", new_process->pid, new_process->neededMem);  
67.    return new_process;  
68.}  
69.DWORD WINAPI allocalateFF(LPVOID lpParam) {  // FF算法  
70.    int NumofBlock = 0;  
71.    Queue* queue = (Queue*)lpParam;  
72.    WaitForSingleObject(generatorThread, INFINITE);  // 等待生成进程生成结束再打印信息  
73.    printf("Allocate process    Allocate size(K)    Block Id    StartAdrr   Remaind memory\n");                                 // 等待生成进程生成结束再打印信息  
74.    while (NumofBlock < MaxNumberofProcess) {  
75.        Block* blocks = queueblock.list;  
76.        WaitForSingleObject(mutex, INFINITE);  
77.        PCB* process = dequeue(queue);  
78.        ReleaseSemaphore(mutex, 1, NULL);  
79.        while (process != NULL) {  
80.            WaitForSingleObject(syncSemaphore, INFINITE);  
81.            Block* new_block = (Block*)malloc(sizeof(struct Block));  
82.            if (MaxMemory > process->neededMem) { // 判断剩余分区大小是否能够进行进程内存分配  
83.                bool allocatejudge = false;  
84.                while (blocks != NULL) {          // 寻找可以进行内存分配的空闲分区  
85.                    if ((blocks->status == true) && (blocks->size > process->neededMem)) {  
86.                        allocatejudge = true;  
87.                        break;  
88.                    }  
89.                    blocks = blocks->next;  
90.                }  
91.                if (allocatejudge == true) {  
92.// 切割空闲分区， 将新的空闲分区插入分区链表  
93.                    new_block->startAddr = blocks->startAddr;  
94.                    new_block->id = blocks->id;  
95.                    new_block->startAddr = blocks->startAddr;  
96.                    new_block->status = false;  
97.                    new_block->pid = process->pid;  
98.                    new_block->size = process->neededMem;  
99.  
100.                    process->blockID = new_block->id;       // 分配成功，更新进程信息  
101.                    process->status = 1;                    	  // 进程分配状态改为成功  
102.// 修改剩余空闲分区开始地址  
103.                    blocks->startAddr = new_block->startAddr + new_block->size;  
104.                    blocks->size -= new_block->size;        // 修改剩余空闲分区内存空间  
105.  
106.                    MaxMemory -= new_block->size;        // 更新剩余内存  
107.  
108.                    Block* last_block = blocks->prev;       // 更新双向链表指针  
109.                    Block* next_block = blocks->next;  
110.                    if (last_block != NULL) {           	  // 插入new_block  
111.                        last_block->next = new_block;  
112.                    }  
113.                    else {                              // 如果为NULL，说明blocks为队首元素  
114.                        queueblock.list = new_block;  
115.                    }  
116.                    blocks->prev = new_block;  
117.                    new_block->prev = last_block;  
118.                    new_block->next = blocks;  
119.  
120.                    blocks->startAddr = new_block->size + new_block->startAddr;  
121.  
122.                    while (blocks != NULL) {   // 修改分区链表分区号  
123.                        blocks->id += 1;  
124.                        blocks->startAddr = blocks->prev->startAddr + blocks->prev->size;               // 修改分区区地址开始位置  
125.                        blocks = blocks->next;  
126.                    }  
127.                    WaitForSingleObject(generatorThread, INFINITE);// 等待生成进程生成结束再打印信息  
128.                    printf("%8d %20d %14d %12d %12d\n", process->pid, process->neededMem, process->blockID, new_block->startAddr, MaxMemory);  
129.                    NumofBlock++;           // 进程处理完毕， 计数器+1  
130.                    break;  
131.                }  
132.                else {                           // 如果不能，则内存分配失败, 进程放到队尾  
133.                    WaitForSingleObject(generatorThread, INFINITE);// 等待生成进程生成结束再打印信息  
134.                    printf("Process: %d Allocation failure!\n", process->pid);  
135.                    NumofBlock++;          // 进程处理完毕， 计数器+1  
136.                }  
137.            }  
138.            else {                               // 如果不能，则内存分配失败, 进程放到队尾  
139.                WaitForSingleObject(generatorThread, INFINITE); // 等待生成进程生成结束再打印信息  
140.                printf("Process: %d Allocation failure!\n", process->pid);  
141.                NumofBlock++;// 剩余最大内存<进程所需的内存，无法进行内存分配，计数器+1  
142.            }  
143.            process = process->next;  
144.        }  
145.    }  
146.    return 0;  
147.}  
148.DWORD WINAPI generator(LPVOID lpParam) {  
149.    srand((unsigned)time(NULL)); // 初始化随机数种子，以确保每次运行得到的随机数不同  
150.    int numberOfProcesses = 0;  
151.    while (numberOfProcesses < MaxNumberofProcess) { // 生成10个进程  
152.        int randomSleepTime = rand() % 100 + 1;                 // 随机等待时间[1, 100] ms   
153.        Sleep(randomSleepTime);                               	  // 等待随机时间  
154.  
155.        // 创建新进程  
156.        PCB* new_process = create_process(numberOfProcesses + 1);  
157.        WaitForSingleObject(mutex, INFINITE);  
158.        enqueue(&queue, new_process);  
159.        ReleaseSemaphore(mutex, 1, NULL);  
160.        ReleaseSemaphore(syncSemaphore, 1, NULL);        // 通知主程序有程序产生  
161.        numberOfProcesses++;  
162.    }  
163.    return 0;  
164.}  
165.void init_queueblock() {  
166.    Block* new_block = (Block*)malloc(sizeof(struct Block));  
167.    new_block->id = 0;  
168.    new_block->next = NULL;  
169.    new_block->pid = -1;  
170.    new_block->prev = NULL;  
171.    new_block->startAddr = 0;  
172.    new_block->size = 1024;  
173.    new_block->status = true;  
174.    queueblock.list = new_block;  
175.}  
176.void cleanup() {                                        // 释放队列中的所有进程资源  
177.    Block* blocks = queueblock.list;  
178.    printf("Recall block id     Size(K)\n");  
179.    while (blocks != NULL) {  
180.        Block* last_block = blocks->prev;  
181.        if (blocks->status == false) {  
182.            blocks->status = true;                      // 回收分区资源，修改状态  
183.            blocks->pid = -1;  
184.            printf("%8d %15d\n", blocks->id, blocks->size);  
185.            if (last_block != NULL && last_block->status == true) { // 向前合并分区资源  
186.                last_block += blocks->size;  
187.                last_block->next = blocks->next;    // 修改双向链表指向  
188.                last_block->next->prev = last_block;  
189.                free(blocks);  
190.                blocks = last_block->next;              // 跳转到下一个分区块  
191.            }  
192.  
193.        }  
194.        else {  
195.            blocks = blocks->next;                     // 跳转到下一个分区块  
196.        }  
197.    }  
198.}  
199.int main()  
200.{  
201.    init_queueblock();  
202.    mutex = CreateSemaphore(NULL, 1, 1, NULL);        // 创建互斥信号量  
203.  // 创建同步信号量  
204.    syncSemaphore = CreateSemaphore(NULL, 0, MaxNumberofProcess, NULL);
205.  // 创建进程产生器线程
206.    generatorThread = CreateThread(NULL, 0, generator, NULL, 0, NULL);       
207.    allocateThread = CreateThread(NULL, 0, allocalateFF, &queue, 0, NULL);  
208.  // 创建进程产生器内存分配线程
209.    WaitForSingleObject(generatorThread, INFINITE);  
210.    WaitForSingleObject(allocateThread, INFINITE);  
211.    cleanup();                                       // 释放占用的分区块  
212.    CloseHandle(generatorThread);    // 释放资源，关闭句柄      
213.    CloseHandle(allocateThread);  
214.    CloseHandle(syncSemaphore);  
215.    CloseHandle(mutex);  
}  