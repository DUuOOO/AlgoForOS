// Mary和Sally是亲姐妹，她们有一个共同的银行账户，她们可以分别到ATM机取款；
// 爸爸，妈妈，奶奶，爷爷，舅舅也可以分别到ATM机给银行账户存钱。设账户的初始余额为10元。
// 爸爸，妈妈，奶奶，爷爷，舅舅每次分别存入10，20，30，40，50元，每个人分别存款2次。
// Mary和Sally每次分别取50和100元，每个人分别取款2次。存款和取款的顺序是随机的。
// 假设Mary和Sally的银行账户是不可借记的，即当余额少于取款额时，不能取款，取款线程需要阻塞等待直到账户有足够的钱。
// 利用临界区(Windows系统)或者mutex信号量(Linux系统)编制程序来模拟上述存取款过程，在主程序结束时将账户的最后余额输出，
// 并人工验证一下是否正确。
#include <stdio.h>
#include <windows.h>
#include <time.h>

CRITICAL_SECTION criticalSection;           // 申请临界区
CONDITION_VARIABLE conditionVariable;       // 申请条件变量
int InitAccBalance = 10;

struct cient {
    const char* person;
    int amount;
}cients[7]={
    {"Dad",10},{"Mum", 20},{"Grandma", 30},{"Grandpa", 40},{"Uncle", 50},{"Mary", 50},{"Sally",100}
}; // 存取款人信息

DWORD WINAPI DepositThread(LPVOID param) {  // 存款线程
    cient* info = (cient*)param;
    EnterCriticalSection(&criticalSection);
    InitAccBalance += info->amount;
    printf("Thread ID %ld - %s deposited: $%d. Current balance: $%d\n", GetCurrentThreadId(), info->person, info->amount, InitAccBalance);
    WakeAllConditionVariable(&conditionVariable);       // 每一次存款后，唤醒条件变量阻塞线程，判断余额是否满足取款
    LeaveCriticalSection(&criticalSection);            
    Sleep(rand() % 10);                                //模拟存款时间
    return 0;
}

DWORD WINAPI WithdrawThread(LPVOID param) { // 取款线程
    cient* info = (cient*)param;
    EnterCriticalSection(&criticalSection);
    while (InitAccBalance < info->amount) { // 如果余额不足，则进入等待，进程阻塞，此处也可以用sleep函数模拟
        printf("Thread ID %ld - %s tried to withdraw $%d but there is not enough balance. Waiting for deposit...\n", GetCurrentThreadId(), info->person, info->amount);
        SleepConditionVariableCS(&conditionVariable, &criticalSection, INFINITE);
    }
    InitAccBalance -= info->amount;
    printf("Thread ID %ld - %s withdrew: $%d. Current balance: $%d\n", GetCurrentThreadId(), info->person, info->amount, InitAccBalance);
    LeaveCriticalSection(&criticalSection);
    Sleep(rand() % 10);                // 模拟取款时间
    return 0;
}

int main() {
    srand((unsigned)time(NULL));            // 随机数种子
    InitializeCriticalSection(&criticalSection);        // 初始化临界区变量
    InitializeConditionVariable(&conditionVariable);    // 初始化条件变量
    HANDLE threads[14];                     // 申请线程句柄

    int cts[7] = { 0,0,0,0,0,0,0 };     // 前面五个记录五个人的存 后面两个记录两个人的取
    int DposTimes = 10;                 // 总共的存款次数
    int wdrawTimes = 4;                 // 总共的取款次数

    printf("Initial account balance: 10.\n");
    for (int i = 0; i < 14; i++) {
        int cientchoice = rand() % 7;
        while (cts[cientchoice] >= 2) {
            cientchoice = rand() % 7;   // 随机选取操作人
        }
        cts[cientchoice] += 1;
        if (cientchoice < 5) {          //存款
            threads[i] = CreateThread(NULL, 0, DepositThread, &cients[cientchoice], 0, NULL);
        }
        else {                          // 取款
            threads[i] = CreateThread(NULL, 0, WithdrawThread, &cients[cientchoice], 0, NULL);

        }
    }
    WaitForMultipleObjects(14, threads, true, INFINITE);         // 等待所有进程结束
    printf("Final account balance: $%d\n", InitAccBalance);      // 输出此时余额总数
    for (int i = 0; i < 7; ++i) {                                // 关闭线程句柄
        CloseHandle(threads[i]);
    }   
    DeleteCriticalSection(&criticalSection);                     // 删除临界区
    return 0;
}