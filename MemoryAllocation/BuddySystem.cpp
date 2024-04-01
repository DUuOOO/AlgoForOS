1.#include <stdlib.h>  
2.#include <stdio.h>  
3.#include <windows.h>  
4.#include <time.h>  
5.#include <math.h>  
6.int MaxNumberofProcess = 8;  
7.HANDLE generatorThread, allocateThread;  
8.HANDLE mutex, syncSemaphore;  
9.struct Block {  
10.    int id;                       //分区的序号  
11.    int sizeK;                  //分区的大小，以2的幂形式2^sizeK  
12.    int startAddr;            //分区的开始位置  
13.    bool status;                //分区的状态：true为空闲，false为被占用  
14.    int pid;                     //如果该分区被占用，则存放占用进程的id; 否则为 - 1  
15.    struct Block* prev;         //指向前面一块内存分区  
16.    struct Block* next;         //指向后面一块内存分区  
17.};  
18.struct PCB {  
19.    int pid;                    //进程的序号  
20.    int neededMem;            //需要的内存分区大小  
21.    int status;                    //1: 内存分配成功；-1：分配失败  
22.    int blockID;                 //如果分配成功，保存占用分区的id,否则为 - 1  
23.    struct PCB* next;        //指向下一个PCB  
24.};  
25.struct Queue {  
26.    struct PCB* list;           // 指向该队列中进程PCB链表的头部   
27.}queue;  
28.struct Queue2 {                 // 队列空闲分区列表根据分区大小设计为降序队列  
29.    int Que_id;  
30.    int size;                      // 存储该队列的空闲分区大小信息  
31.    int num;                      // 记录该队列中分区块数  
32.    struct Block* list;         // 指向该队列中进程Block链表的头部   
33.    struct Queue2* pre = NULL;  // 指向上一个队列列头  
34.    struct Queue2* next = NULL; // 指向下一个队列列头  
35.}queueblock;                    // 存储进程信息的队列  
36.void init_queueblock() {  
37.    Block* new_block = (Block*)malloc(sizeof(struct Block));  
38.    new_block->id = 0;  
39.    new_block->next = NULL;  
40.    new_block->pid = -1;  
41.    new_block->prev = NULL;  
42.    new_block->startAddr = 0;  
43.    new_block->sizeK = 1024;  
44.    new_block->status = true;  
45.    queueblock.list = new_block;  
46.    queueblock.size = int(pow(2, 10));  
47.    queueblock.Que_id = 0;  
48.}  
49.void enqueue(Queue* queue, PCB* process) {                      // 将新产生的进程放入队列  
50.    if (queue->list == NULL) {  
51.        process->next = NULL;  
52.        queue->list = process;  
53.    }  
54.    else {  
55.        PCB* lastProcess = queue->list;  
56.        while (lastProcess->next != NULL) {  
57.            lastProcess = lastProcess->next;  
58.        }  
59.        lastProcess->next = process;  
60.    }  
61.}  
62.PCB* dequeue(Queue* queue) {                            // 取出队头第一个元素 先进先出  
63.    if (queue->list == NULL) {  
64.        return NULL;    // 队列为空  
65.    }  
66.    PCB* process = queue->list;  
67.    queue->list = process->next;  
68.    process->next = NULL;  
69.    return process;  
70.}  
71.void enqueueQ2(Queue2* queue, Block* block) {           // 将新产生的空闲分区放入空闲分区队列  
72.    if (queue == NULL) {  
73.        queue = (Queue2*)malloc(sizeof(struct Queue2));  
74.        queue->list = NULL;  
75.        queue->size = block->sizeK;  
76.    }  
77.    if (queue->list == NULL) {  
78.        block->id = 0;  
79.        block->next = NULL;  
80.        queue->list = block;  
81.    }else {  
82.        Block* lastProcess = queue->list;  
83.        while (lastProcess->next != NULL) {             // 将新加入的空闲分区放到队尾  
84.            lastProcess = lastProcess->next;  
85.        }  
86.        lastProcess->next = block;                      // 更新双向链表指针  
87.        block->prev = lastProcess;  
88.        block->id = lastProcess->id + 1;                // 更新新加入的分区区号  
89.    }  
90.    queue->num += 1;                                    // 记录该链表空闲分区块块数  
91.}  
92.PCB* create_process1(int pid) {  
93.    PCB* new_process = (PCB*)malloc(sizeof(struct PCB));  
94.    if (new_process != NULL) {  
95.        new_process->pid = pid;  
96.        new_process->status = -1;  
97.        new_process->blockID = -1;  
98.        new_process->next = NULL;  
99.    }  
100.    printf("Create process %d", new_process->pid);  
101.    return new_process;  
102.}  
103.PCB* create_process2(int pid) {  
104.    PCB* new_process = (PCB*)malloc(sizeof(struct PCB));  
105.    if (new_process != NULL) {  
106.        new_process->pid = pid;  
107.        new_process->status = -1;  
108.        new_process->blockID = -1;  
109.        new_process->neededMem = int(pow(2, rand() % 6 + 3));// 题2 随机在2^[3, 8]间生成, 设置新进程申请占用的内存大小  
110.        new_process->next = NULL;  
111.    }  
112.    printf("Create process %d, neededMemory is %d K\n", new_process->pid, new_process->neededMem);  
113.    return new_process;  
114.}  
115.DWORD WINAPI generator1(LPVOID lpParam) {  
116.    srand((unsigned)time(NULL)); // 初始化随机数种子，以确保每次运行得到的随机数不同  
117.    int numberOfProcesses = 0;  
118.    int processNeedMeo[3] = { int(pow(2,7)), int(pow(2, 4)), int(pow(2, 8)) };  
119.    while (numberOfProcesses < MaxNumberofProcess) {            // 生成3个进程  
120.        int randomSleepTime = rand() % 100 + 1;                 // 随机等待时间[1, 100] ms   
121.        Sleep(randomSleepTime);                                 // 等待随机时间  
122.  
123.        // 创建新进程  
124.        PCB* new_process = create_process1(numberOfProcesses + 1);  
125.        new_process->neededMem = processNeedMeo[numberOfProcesses];     // 设置新进程申请占用的内存大小  
126.        printf(", neededMemory is %d K\n", new_process->neededMem);  
127.        WaitForSingleObject(mutex, INFINITE);  
128.        enqueue(&queue, new_process);                           // 将进程加入进程队列  
129.        ReleaseSemaphore(mutex, 1, NULL);  
130.        ReleaseSemaphore(syncSemaphore, 1, NULL);               // 通知主程序有程序产生  
131.        numberOfProcesses++;  
132.        printf("\n");  
133.    }  
134.    return 0;  
135.}  
136.DWORD WINAPI generator2(LPVOID lpParam) {  
137.    srand((unsigned)time(NULL)); // 初始化随机数种子，以确保每次运行得到的随机数不同  
138.    int numberOfProcesses = 0;  
139.    while (numberOfProcesses < MaxNumberofProcess) {            // 生成8个进程  
140.        int randomSleepTime = rand() % 100 + 1;                 // 随机等待时间[1, 100] ms   
141.        Sleep(randomSleepTime);                                 // 等待随机时间  
142.  
143.        // 创建新进程  
144.        PCB* new_process = create_process2(numberOfProcesses + 1);  
145.        WaitForSingleObject(mutex, INFINITE);  
146.        enqueue(&queue, new_process);  
147.        ReleaseSemaphore(mutex, 1, NULL);  
148.        ReleaseSemaphore(syncSemaphore, 1, NULL);               // 通知主程序有程序产生  
149.        numberOfProcesses++;  
150.    }  
151.    printf("\n");  
152.    return 0;  
153.}  
154.DWORD WINAPI AlloBuddySys(LPVOID lpParam) {                     // BuddySystem算法  
155.    int NumofBlock = 0;  
156.    Queue* queue = (Queue*)lpParam;  
157.  
158.    while (NumofBlock < MaxNumberofProcess) {  
159.        WaitForSingleObject(syncSemaphore, INFINITE);  
160.        Queue2* QueueOfBlocks = &queueblock;  
161.        WaitForSingleObject(mutex, INFINITE);  
162.        PCB* process = dequeue(queue);  
163.        ReleaseSemaphore(mutex, 1, NULL);  
164.        // 根据进程所需内存大小分配分区  
165.  
166.        bool AllocateQueuejudge = false;  
167.        bool allocatejudge = false;  
168.        Block* blocks = QueueOfBlocks->list;  
169.        while(AllocateQueuejudge == false || allocatejudge == false) {   // 若未选择队列且未进行空闲分区分配  
170.            while (QueueOfBlocks != NULL && allocatejudge == false) {    // 先找是否有大小符合要求的队列  
171.                blocks = QueueOfBlocks->list;  
172.                AllocateQueuejudge = false;   // 新的一次搜索，需要进行一次新的空间划分的判断  
173.                // 如果进程所需要内存小于该空闲块大小且大于该空闲块内存的一半,则停止且进入该队列寻找空闲分区  
174.                if ((process->neededMem * 2 > QueueOfBlocks->size) && (process->neededMem <= QueueOfBlocks->size)) {  
175.                    while (blocks != NULL) {  
176.                        if (blocks->status == true) {                    // 如果存在空闲分区，则进行分配  
177.                            blocks->status = false;                        // 更新空闲分区信息  
178.                            blocks->pid = process->pid;  
179.                            process->blockID = blocks->id;           // 更新进程信息  
180.                            AllocateQueuejudge = true;                 // 更新标志器信息，结束外循环  
181.                            allocatejudge = true;             
182.                            WaitForSingleObject(generatorThread, INFINITE);// 等待生成进程函数运行结束后再打印信息  
183.                            printf("PID    Allocate Size(K)    Block-chain Id    Block Id    StartAdrr\n");  
184.                            printf("%d  %12d %18d  %14d %12d\n", blocks->pid, blocks->sizeK, QueueOfBlocks->Que_id, process->blockID, blocks->startAddr);  
185.                            NumofBlock++;                                // 进程处理完毕， 计数器+1  
186.                            break;                                       // 结束内循环  
187.                        }else {  
188.                            blocks = blocks->next;  
189.                        }  
190.                    }  
191.                    if (blocks == NULL) {                     // 说明适合当前大小的空闲分区已都被占用  
192.                        break;  
193.                    }  
194.                }else if (process->neededMem >= QueueOfBlocks->size) {  
195.                    // 如果进程所需内存大于队列，说明此时在大小合适的队列里没有找到空闲分区，结束循环，需要重新进行空间划分  
196.                    break;  
197.                }  
198.                if (QueueOfBlocks->next == NULL) break;  
199.                QueueOfBlocks = QueueOfBlocks->next;        // 否则，寻找下一条空闲分区队列  
200.            }  
201.            if (AllocateQueuejudge == false || allocatejudge == false) {// 进程未分配成功：没有空闲分区  
202.                if (QueueOfBlocks == NULL) {  
203.                    QueueOfBlocks = &queueblock;  
204.                }  
205.                while (QueueOfBlocks != NULL) {  
206.                    if (QueueOfBlocks->size >= process->neededMem * 2) { // 寻找更大的空闲分区块，进行空间划分  
207.                        blocks = QueueOfBlocks->list;  
208.                        while (blocks != NULL) {  
209.                            if ((blocks->status == true) && (blocks->next==NULL)) {   // 如果存在空闲分区,找到队列最后一个，则进行空间划分和将新划分的空闲分区入队与分配  
210.                                if (blocks->prev != NULL) {  
211.                                    blocks->prev->next = blocks->next;      // 断开分区链接  
212.                                }else  
213.                                {  
214.                                    QueueOfBlocks->list = NULL;  
215.                                }  
216.                                QueueOfBlocks->num -= 1;                    // 修改队列空闲分区块数  
217.                                Block* new_block1 = (Block*)malloc(sizeof(struct Block));  
218.                                Block* new_block2 = (Block*)malloc(sizeof(struct Block));  
219.                                new_block1->sizeK = blocks->sizeK / 2;      // 更新空闲分区信息  
220.                                new_block1->next = NULL;  
221.                                new_block1->prev = NULL;  
222.                                new_block1->status = true;  
223.                                new_block1->pid = -1;  
224.                                new_block1->startAddr = blocks->startAddr;  
225.  
226.                                new_block2->sizeK = blocks->sizeK / 2;      // 更新空闲分区信息  
227.                                new_block2->prev = new_block1;  
228.                                new_block2->next = NULL;  
229.                                new_block2->status = true;  
230.                                new_block2->pid = -1;  
231.                                new_block2->startAddr = new_block1->startAddr + new_block1->sizeK;  
232.  
233.                                if (QueueOfBlocks->next == NULL) { // 如果该队列未有下一个队列，则创建新队列  
234.                                    Queue2* new_queue2 = (Queue2*)malloc((sizeof(struct Queue2)));  
235.                                    new_queue2->size = new_block1->sizeK;  
236.                                    new_queue2->num = 0;  
237.                                    new_queue2->list = NULL;  
238.                                    new_queue2->next = NULL;  
239.                                    new_queue2->pre = NULL;  
240.                                    WaitForSingleObject(mutex, INFINITE);   // new_block1 入队  
241.                                    enqueueQ2(new_queue2, new_block1);  
242.                                    ReleaseSemaphore(mutex, 1, NULL);  
243.  
244.                                    WaitForSingleObject(mutex, INFINITE);   // new_block12 入队  
245.                                    enqueueQ2(new_queue2, new_block2);  
246.                                    ReleaseSemaphore(mutex, 1, NULL);  
247.  
248.                                    QueueOfBlocks->next = new_queue2;  
249.                                    new_queue2->pre = QueueOfBlocks;  
250.                                    new_queue2->Que_id = QueueOfBlocks->Que_id + 1;  
251.                                    AllocateQueuejudge = true;  
252.                                    break;                                  // 结束内循环  
253.                                }  
254.                                else {   // 若该队列已有下一个队列，则获取下一个队列，将新分区入队  
255.                                    Queue2* new_queue2 = QueueOfBlocks->next;  
256.                                    WaitForSingleObject(mutex, INFINITE);   // new_block1 入队  
257.                                    enqueueQ2(new_queue2, new_block1);  
258.                                    ReleaseSemaphore(mutex, 1, NULL);  
259.  
260.                                    WaitForSingleObject(mutex, INFINITE);   // new_block12 入队  
261.                                    enqueueQ2(new_queue2, new_block2);  
262.                                    ReleaseSemaphore(mutex, 1, NULL);  
263.  
264.                                    AllocateQueuejudge = true;  
265.                                    break;                                  // 结束内循环  
266.                                }                                                              
267.                            }else {  
268.                                blocks = blocks->next;  
269.                            }  
270.                        }  
271.                    }  
272.                    // 如果重新进行了空间划分，则搜索划分后的空间进行分配  
273.                    if (AllocateQueuejudge == true) {  
274.                        QueueOfBlocks = QueueOfBlocks->next;  
275.                        break;  
276.                    }  
277.                    else QueueOfBlocks = QueueOfBlocks->pre;  
278.                }  
279.            }  
280.        }  
281.  
282.        if(allocatejudge = false){                                     // 如果不能，则内存分配失败  
283.            WaitForSingleObject(generatorThread, INFINITE);            // 等待生成进程函数运行结束后再打印信息  
284.            printf("Process: %d Allocation failure!\n", process->pid); // 打印失败信息  
285.            NumofBlock++;                                              // 进程处理完毕， 计数器+1  
286.        }  
287.    }  
288.    return 0;  
289.}  
290.bool FindMerge(int TargetStartAddrOfBuddy, Block* BlockChain, Queue2* Q) {  
291.    Queue2* POfQ = *&Q;                                                // 新建指针，防止修改原指针Q指向  
292.    Block* OriBlock = *&BlockChain;  
293.    while (BlockChain != NULL) {  
294.        // 如果该分区空闲且为伙伴地址块，则回收  
295.        if (BlockChain->startAddr == TargetStartAddrOfBuddy) {  
296.            BlockChain->prev->next = BlockChain->next;                  // 修改指针链接，删除空闲块  
297.            if(BlockChain->next != NULL) BlockChain->next->prev = BlockChain->prev;  
298.            Block* new_block = (Block*)malloc(sizeof(struct Block));  
299.            new_block->sizeK = BlockChain->sizeK * 2;                  // 更新空闲分区信息  
300.            new_block->next = NULL;  
301.            new_block->prev = NULL;  
302.            new_block->status = true;  
303.            new_block->pid = -1;  
304.            new_block->startAddr = OriBlock->startAddr;             // 修改新分区起始地址，等待下一次合并检查  
305.            WaitForSingleObject(mutex, INFINITE);  
306.            enqueueQ2(Q->pre, new_block);                  // 将合并的空闲分区块入队，等待查询是否需要进一步合并  
307.            ReleaseSemaphore(mutex, 1, NULL);  
308.            printf("Ori block-chain     merged block1   merged block2   merged block size  new block-chain  new block   new block size\n");  
309.            printf("%8d %18d %14d %20d %14d %14d %14d\n",  
310.                Q->Que_id, OriBlock->id, BlockChain->id, BlockChain->sizeK, Q->pre->Que_id, new_block->id, new_block->sizeK);  
311.            free(BlockChain);                                           // 释放空闲分区资源  
312.            return true;                                                   // 停止寻找  
313.        }  
314.        else {  
315.            BlockChain = BlockChain->next;  
316.        }  
317.    }  
318.    return false;                                       // 未找到伙伴分区块，返回假  
319.}  
320.void Cleanup() {                                   // 回收空闲分区链表中的分区  
321.    Queue2* Q = &queueblock;  
322.    while (Q != NULL) {                        // 逐一查找每个队列中的空闲分区  
323.        Block* blocks = Q->list;  
324.        bool flag = false;                           // 判断是否合并空闲分区成功标志器  
325.        while (blocks != NULL) {  
326.            Block* findblock = *&blocks;  
327.            if (blocks->startAddr % blocks->sizeK * 2 == 0) {  
328.                flag = FindMerge(blocks->startAddr + blocks->sizeK, findblock, Q);  
329.            }  
330.            else {  
331.                flag = FindMerge(blocks->startAddr - blocks->sizeK, findblock, Q);  
332.            }  
333.            if (flag == true) {                             // 如果合并空闲分区成功，返回上一队列查找是否有空闲分区需要合并  
334.                if (blocks->prev != NULL) {                 // 修改指针链表信息,删除当前合并分区  
335.                    blocks->prev->next = blocks->next;  
336.                    if (blocks->next != NULL) {  
337.                        blocks->next->prev = blocks->prev;  
338.                    }  
339.                }  
340.                else {  
341.                    if (blocks->next != NULL) {  
342.                        Q->list = blocks->next;  
343.                    }else {  
344.                        Q->list = NULL;  
345.                    }  
346.                }  
347.                Q = Q->pre;  
348.                break;  
349.            }  
350.            blocks = blocks->next;  
351.        }  
352.        if (blocks == NULL) {  // 如果没有空闲分区合并成功，查找下一条队列是否需要合并  
353.            Q = Q->next;                                
354.        }  
355.    }  
356.    printf("Merge Finish!");  
357.}  
358.int main()  
359.{  
360.    init_queueblock();  
361.    mutex = CreateSemaphore(NULL, 1, 1, NULL);                                  // 创建互斥信号量  
362.    syncSemaphore = CreateSemaphore(NULL, 0, MaxNumberofProcess, NULL);         // 创建同步信号量  
363.    generatorThread = CreateThread(NULL, 0, generator2, NULL, 0, NULL);         // 创建进程产生器线程       
364.    allocateThread = CreateThread(NULL, 0, AlloBuddySys, &queue, 0, NULL);      // 创建进程产生器内存分配线程  
365.    WaitForSingleObject(generatorThread, INFINITE);  
366.    WaitForSingleObject(allocateThread, INFINITE);  
367.    Cleanup();                                                                  // 释放占用的分区块  
368.    CloseHandle(generatorThread);                                 // 释放资源，关闭句柄      
369.    CloseHandle(allocateThread);  
370.    CloseHandle(syncSemaphore);  
371.    CloseHandle(mutex);  
}