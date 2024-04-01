1.#include <stdio.h>  
2.#include <stdlib.h>  
3.#include <time.h>  
4.  
5.#define MaxSequenceLen 20  
6.#define MaxNumOfPages 10  
7.#define NumAllocatedPages 3  
8.  
9.int RequestSequence[MaxSequenceLen];          // 页面访问序列数组  
10.int AccessTimeRecord[MaxNumOfPages];        // 记录页面访问时间数组  
11.int AllocatedPageArray[NumAllocatedPages];  // 系统分配给进程的页面数组  
12.  
13.int HitCount = 0;    // 命中次数  
14.int MissCount = 0;  // 缺页次数  
15.  
16.// 初始化页面访问时间数组  
17.void InitializeAccessTimeRecord() {  
18.    for (int i = 0; i < MaxNumOfPages; ++i) {  
19.        AccessTimeRecord[i] = -1;  
20.    }  
21.}  
22.  
23.// 初始化页面数组  
24.void InitializeAllocatedPageArray() {  
25.    for (int i = 0; i < NumAllocatedPages; ++i) {  
26.        AllocatedPageArray[i] = -1;  
27.    }  
28.}  
29.  
30.// 查找页面在数组中的位置  
31.int FindPageInArray(int page) {  
32.    for (int i = 0; i < NumAllocatedPages; ++i) {  
33.        if (AllocatedPageArray[i] == page) {  
34.            return i;  
35.        }  
36.    }  
37.    return -1;  // 页面不在数组中  
38.}  
39.  
40.// LRU页面置换算法  
41.void LRUPageReplacement(int page) {  
42.    int minTime = AccessTimeRecord[0];  
43.    int minIndex = 0;  
44.  
45.    // 找到最久未被访问的页面  
46.    for (int i = 1; i < NumAllocatedPages; ++i) {  
47.        if (AccessTimeRecord[i] < minTime) {  
48.            minTime = AccessTimeRecord[i];  
49.            minIndex = i;  
50.        }  
51.    }  
52.  
53.    // 替换页面  
54.    AllocatedPageArray[minIndex] = page;  
55.    AccessTimeRecord[minIndex] = 0;  
56.}  
57.  
58.int main() {  
59.    srand((unsigned)time(NULL));   // 初始化随机数种子，以确保每次运行得到的随机数不同  
60.    int ProcessSize = 10;                  // 进程大小  
61.    int AllocatedMemorySize = 3;   // 物理内存空间  
62.    int RequestSequenceLength = 20;                             // 页面访问序列长度  
63.  
64.    for (int i = 0; i < RequestSequenceLength; ++i) {    // 随机生成页面访问序列  
65.        RequestSequence[i] = rand() % ProcessSize;  
66.    }  
67.  
68.    InitializeAccessTimeRecord();  
69.    InitializeAllocatedPageArray();  
70.                                                                // 模拟页面访问  
71.    for (int i = 0; i < RequestSequenceLength; ++i) {  
72.        int requestedPage = RequestSequence[i];  
73.  
74.        int pageIndex = FindPageInArray(requestedPage);         // 检查页面是否在物理内存中  
75.        if (pageIndex != -1) {                                  // 命中  
76.            HitCount++;  
77.            AccessTimeRecord[pageIndex] = 0;  
78.        }  
79.        else {                                                  // 缺页，进行页面置换  
80.            MissCount++;  
81.            LRUPageReplacement(requestedPage);  
82.        }  
83.        for (int j = 0; j < NumAllocatedPages; ++j) {           // 更新页面访问时间  
84.            if (AllocatedPageArray[j] != -1) {  
85.                AccessTimeRecord[j]++;  
86.            }  
87.        }  
88.  
89.        // 输出每次处理后的页面被占用情况  
90.        printf("Allocated Page Array after processing page %d: [", requestedPage);  
91.        for (int j = 0; j < NumAllocatedPages; ++j) {  
92.            if (AllocatedPageArray[j] != -1) {  
93.                printf("%d ", AllocatedPageArray[j]);  
94.            }  
95.        }  
96.        printf("]\n");  
97.    }  
98.  
99.    printf("\nFinal Result:\n");                                // 输出结果  
100.    printf("Hit Count: %d\n", HitCount);             // 输出命中率  
101.    printf("Miss Count: %d\n", MissCount);        // 输出缺页次数  
102.  // 输出缺页率
103.    printf("Page Fault Rate: %.2f%%\n", (float)MissCount / RequestSequenceLength * 100);  
104.    return 0;  
} 