// 假设有四个线程，第一个线程输出字符串 “I”，第二个线程输出字符串 “Love”, 第三个线程输出字符串“Jinan”, 第四个线程输出字符串 “University！”。编制C/C++程序，在主程序main函数中创建四个线程并依次启动，设计信号量(Semaphore)同步机制，当主程序运行时，屏幕输出的结果是字符串“I Love Jinan University!”
#include<stdlib.h>     //包含随机数产生函数
#include<time.h>       //与时间有关的函数头文件
#include<windows.h>    //针对Windows操作系统
#include <iostream>
using namespace std;

int maxNumofThread = 4;						// 设置最大线程数 
HANDLE sema_1, sema_2, sema_3, sema_4;		// 定义四个线程的信号量变量 

DWORD WINAPI tread1(LPVOID p){				// 输出的第一个线程1 
	// 设置前驱关系
	WaitForSingleObject(sema_1, INFINITE);	// 申请访问信号量 
	cout << "I ";
	ReleaseSemaphore(sema_2, 1, NULL);		// 释放信号量  
	return 0;
}
DWORD WINAPI tread2(LPVOID p){				// 输出的第一个线程1 
	// 设置前驱关系
	WaitForSingleObject(sema_2, INFINITE);
	cout << "Love ";
	ReleaseSemaphore(sema_3, 1, NULL);
    return 0;
}
DWORD WINAPI tread3(LPVOID p){				// 输出的第一个线程1 
	// 设置前驱关系
	WaitForSingleObject(sema_3, INFINITE);
	cout << "Jinan ";
	ReleaseSemaphore(sema_4, 1, NULL);
    return 0;
}
DWORD WINAPI tread4(LPVOID p){				// 输出的第一个线程1 
	// 设置前驱关系
	WaitForSingleObject(sema_4, INFINITE);
	cout << "University!\n" << endl;
    return 0;
}
int main(){
	HANDLE handle[maxNumofThread];			// 申请线程句柄对象空间 
// 此处ThreadID并不是一个必要的变量，可以在CreateThread设置为NULL，此处设置是为了加强应用规范
	DWORD ThreadID[maxNumofThread];			// 申请线程地址存储空间 
	
    sema_1 = CreateSemaphore(NULL, 1, 1, NULL);		// 创建信号量 
    sema_2 = CreateSemaphore(NULL, 0, 1, NULL);
    sema_3 = CreateSemaphore(NULL, 0, 1, NULL);
    sema_4 = CreateSemaphore(NULL, 0, 1, NULL);
    handle[0] = CreateThread(NULL,0,tread1,NULL,0,&ThreadID[0]);	
    handle[1] = CreateThread(NULL,0,tread2,NULL,0,&ThreadID[1]);
    handle[2] = CreateThread(NULL,0,tread3,NULL,0,&ThreadID[2]);
    handle[3] = CreateThread(NULL,0,tread4,NULL,0,&ThreadID[3]);
    
    WaitForSingleObject(handle[0],INFINITE);		// 申请访问信号量 
    WaitForSingleObject(handle[1],INFINITE);
    WaitForSingleObject(handle[2],INFINITE);
    WaitForSingleObject(handle[3],INFINITE);
    
	CloseHandle(handle[0]); 				// 关闭线程 
	CloseHandle(handle[1]);
	CloseHandle(handle[2]);
	CloseHandle(handle[3]);   
	
	CloseHandle(sema_1);					// 关闭信号量 
    CloseHandle(sema_2);
    CloseHandle(sema_3);
    CloseHandle(sema_4);
    return 0;
}