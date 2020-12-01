#include <iostream>
#include <windows.h>
#include <string>
/* 临界区(关键段)
 * CriticalSection
 * 临界区内的代码同一时刻只允许一个线程去执行
 * AIP：
 *      1、初始化临界区
 *      void InitializaCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
 *      2、删除临界区
 *      void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
 *      3、尝试进入临界区
 *      bool TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
 *      4、进入临界区
 *      void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
 *      5、离开临界区
 *      void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
 *      6、初始化创建带有自旋计数的临界区
 *      bool InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount);
 */

//利用RAII封装的临界区对象自解机制
class CSObject
{
    CSObject(CRITICAL_SECTION &lpCriticalSection):CS(lpCriticalSection)
    {
        EnterCriticalSection(&CS);
    }
    ~CSObject()
    {
        LeaveCriticalSection(&CS);
    }
    CRITICAL_SECTION &CS;
};

CRITICAL_SECTION CS;
int g_number;

unsigned int __stdcall WorkerThreadProc(LPVOID lpThreadParameter)
{
    DWORD dwThreadID = ::GetCurrentThreadId();
    while(true)
    {
        EnterCriticalSection(&CS);
        std::cout << "EnterCriticalSection, ThreadID: "<< dwThreadID << std::endl;
        g_number++;
        std::cout << "EnterCriticalSection, ThreadID: "<< dwThreadID << ",Value: " << g_number << std::endl;
        LeaveCriticalSection(&CS);
        Sleep(1000);
    }
}

int main()
{
    InitializeCriticalSection(&CS);
    HANDLE hWorkerThread1 = (HANDLE)_beginthreadex(NULL,0,WorkerThreadProc,NULL,0,NULL);
    HANDLE hWorkerThread2 = (HANDLE)_beginthreadex(NULL,0,WorkerThreadProc,NULL,0,NULL);
    WaitForSingleObject(hWorkerThread1,INFINITE);
    WaitForSingleObject(hWorkerThread2,INFINITE);

    CloseHandle(hWorkerThread1);
    CloseHandle(hWorkerThread2);

    DeleteCriticalSection(&CS);
    return 0;
}


/* 事件
 * Event
 * API:
 *      1、创建Event(安全属性、对象受信时行为(true:需要手动调用RsetEvent重置事件状态；false:自动重置事件状态)、初始状态是否受信(true:有信号;false:无信号)、事件名称)
 *      HANDLE CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, bool bManualReset, bool bInitialState, LPCTSTR lpName);
 *      2、设置受信
 *      bool SetEvent(HANDLE hEvent);
 *      3重置事件句柄
 *      bool ResetEvent(HANDLE hEvent);
*/
//!1
bool g_bTaskCompleted = false;
std::string g_TaskResult;
HANDLE g_hTaskEvent = NULL;

unsigned int __stdcall WorkerThreadProc(LPVOID lpThreadParameter)
{
    Sleep(3000);
    g_TaskResult = "task completed";
    g_bTaskCompleted = true;
    SetEvent(g_hTaskEvent);
    return 0;
}

int main()
{
    g_hTaskEvent = CreateEvent(NULL,true,false,NULL);
    HANDLE WorkerThread = (HANDLE)_beginthreadex(NULL,0,WorkerThreadProc,NULL,0,NULL);
    DWORD dwResult = WaitForSingleObject(g_hTaskEvent,INFINITE);
    if (dwResult == WAIT_OBJECT_0)
    {
        std::cout << g_TaskResult << std::endl;
    }
    CloseHandle(WorkerThread);
    CloseHandle(g_hTaskEvent);
    return 0;
}
*/
//!2
HANDLE g_event;
int g_number = 0;
unsigned int WINAPI WorkerThreadProcSub(LPVOID lpThreadParameter)
{
    DWORD ThId = ::GetCurrentThreadId();
    for (int i = 0;i < 100; i++)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(g_event,INFINITE))
        {
            printf("id:%d\n",(int)ThId);
            //ResetEvent(g_event);
            g_number--;
            SetEvent(g_event);
            printf("id:%d,-v=%d\n",(int)ThId,g_number);
        }
    }

    return 0;
}

unsigned int WINAPI WorkerThreadProcAdd(LPVOID lpThreadParameter)
{
    DWORD ThId = ::GetCurrentThreadId();
    for (int i = 0;i < 100; i++)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(g_event,INFINITE))
        {
            printf("id:%d\n",(int)ThId);
            //ResetEvent(g_event);
            g_number++;
            SetEvent(g_event);
            printf("id:%d,+v=%d\n",(int)ThId,g_number);
        }
    }

    return 0;
}

int main()
{
    g_event = CreateEvent(NULL,false,true,NULL);

    HANDLE Th1 = (HANDLE)_beginthreadex(NULL,0,WorkerThreadProcAdd,NULL,0,NULL);
    HANDLE Th2 = (HANDLE)_beginthreadex(NULL,0,WorkerThreadProcSub,NULL,0,NULL);

    WaitForSingleObject(Th1,INFINITE);
    WaitForSingleObject(Th2,INFINITE);

    CloseHandle(Th1);
    CloseHandle(Th2);
    CloseHandle(g_event);
    return 0;
}


/* Mutex(互斥量)
 * API:
 *      1、创建互斥量(安全属性一般为NULL、设置调用CreateMutex是否立即拥有Mutex(true:拥有;false:不拥有)、对象名称)
 *      HANDLE CreateMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes, bool bInitialOwner, LPCTSTR lpName);
 *      2、释放互斥量
 *      bool ReleaseMutex(HANDLE hMutex);
*/

HANDLE g_Mutex = NULL;
int g_number = 0;

unsigned int WINAPI WorkerThreadProc(LPVOID plThreadParameter)
{
    DWORD ThId = GetCurrentThreadId();
    while (true) {
        if (WAIT_OBJECT_0 == WaitForSingleObject(g_Mutex,1000))
        {
            g_number++;
            printf("ThId:%d,value=%d\n",ThId,g_number);
            ReleaseMutex(g_Mutex);
        }
        Sleep(1000);
    }
    return 0;
}

int main()
{
    g_Mutex = CreateMutex(NULL,false,NULL);
    HANDLE hWorkerThread[5];
    for (int i = 0; i < 5; i++)
    {
        hWorkerThread[i] = (HANDLE)_beginthreadex(NULL,0,WorkerThreadProc,NULL,0,NULL);
    }

    WaitForMultipleObjects(5,hWorkerThread,true,INFINITE);
    for (int i = 0;i < 5; i++)
    {
        CloseHandle(hWorkerThread[i]);
    }
    CloseHandle(g_Mutex);
    return 0;
}


/* Semaphore(信号量)
 * API:
 *      1、创建信号量(安全属性NULL、资源数量、最大资源数量、对象名称)
 *      HANDLE CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, long lInitialCount, long lMaximumCount, LPCTSTR lpName);
 *      2、释放信号量(信号量句柄、需要增加的资源数量、函数执行成功后返回上一次资源的数量，若用不到可传入NULL)
 *      bool ReleaseSemaphore(HANDLE hSemaphore, long lReleaseCount, LPLONG lpPreviousCount);
*/

#include <time.h>
#include <list>
HANDLE g_Semaphore = NULL;
std::list<std::string> g_listChatMsg;

CRITICAL_SECTION g_CS;//保护消息的临界区对象

unsigned int WINAPI netWorkThread(LPVOID param)
{
    //srand(time(0));
    int nMsgIndex = 0;
    while (true)
    {
        EnterCriticalSection(&g_CS);
        int count = rand() % 4 + 1;
        for (int i = 0; i < count; i++)
        {
            nMsgIndex++;
            SYSTEMTIME st;
            GetLocalTime(&st);
            char szChatMsg[64] = {};
            sprintf_s(szChatMsg,64,"[%04d-%02d-%02d %02d:%02d:%02d:%03d] A new msg, NO.%d.",
                      st.wYear,
                      st.wMonth,
                      st.wDay,
                      st.wHour,
                      st.wMinute,
                      st.wSecond,
                      st.wMilliseconds,
                      nMsgIndex);
            g_listChatMsg.emplace_back(szChatMsg);
        }
        LeaveCriticalSection(&g_CS);
        ReleaseSemaphore(g_Semaphore,count,NULL);
        Sleep(1000);
    }
    return 0;
}

unsigned int WINAPI workerThread(LPVOID param)
{
    DWORD ThId = GetCurrentProcessId();
    std::string current;
    while (true)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(g_Semaphore,INFINITE))
        {
            EnterCriticalSection(&g_CS);
            if (!g_listChatMsg.empty())
            {
                current = g_listChatMsg.front();
                g_listChatMsg.pop_front();
                printf("ThId:%d, msg:%s\n",ThId, current.c_str());
            }
            LeaveCriticalSection(&g_CS);
        }
    }
    return 0;
}

int main()
{
    srand(time(0));
    InitializeCriticalSection(&g_CS);
    g_Semaphore = CreateSemaphore(NULL,0,4,NULL);

    HANDLE netThread = (HANDLE)_beginthreadex(NULL,0,netWorkThread,NULL,0,NULL);
    HANDLE workThread[4];
    for (int i = 0; i < 4; i++)
    {
        workThread[i] = (HANDLE)_beginthreadex(NULL,0,workerThread,NULL,0,NULL);
    }
    WaitForMultipleObjects(4,workThread,true,INFINITE);
    WaitForSingleObject(netThread,INFINITE);

    for (int i = 0;i < 4; i++)
    {
        CloseHandle(workThread[i]);
    }
    CloseHandle(netThread);
    CloseHandle(g_Semaphore);
    DeleteCriticalSection(&g_CS);
    return 0;
}

