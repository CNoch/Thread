#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <list>
using namespace std;
/* mutex(互斥量)
 * ************************************************************************************************
 * 互斥量                          版本                  作用
 * mutex                          C++11             最基本的互斥量
 * timed_mutex                    C++11             有超时机制的互斥量
 * recursive_mutex                C++11             可重入的互斥量
 * recursive_timed_mutex          C++11             结合timed_mutex和recursive_mutex特点的互斥量
 * shared_timed_mutex             C++14             具有超时机制的可共享互斥量
 * shared_mutex                   C++17             共享的互斥量
 * ************************************************************************************************
 * API:#include <mutex>
 *      1、加锁
 *      @ void lock();
 *      @ void try_lock();
 *      2、解锁
 *      void unlock();
 *
 * ****************************************************************************
 * 互斥量管理                        版本                      作用
 * lock_guard                      C++11                基于作用域的互斥量管理
 * unique_lock                     C++11                更加灵活的互斥量管理
 * shared_lock                     C++14                共享互斥量的管理
 * scope_lock                      C++17                多互斥量避免死锁的管理
 * *****************************************************************************
 *
 * //!重复加锁,是一个错误的做法。所谓“行为未定义”即在不同平台上可能会有不同的行为。
 * int main()
 * {
 *     std::mutex mutex;
 *     mutex.lock();
 *     mutex.lock();
 *     mutex.unlock();
 *     return 0;
 * }
*/
/*
int g_number = 0;
std::mutex g_mutex;

void worker_thread(int id)
{
    for (int i = 0;i < 3; i++)
    {
        g_mutex.lock();
        ++g_number;
        printf("id:%d,g_number:%d\n",id,g_number);
        g_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    std::thread t1(worker_thread,0);
    std::thread t2(worker_thread,10);
    t1.join();
    t2.join();
    return 0;
}
*/

/* std::condition_variable(条件变量)
 * API:#include <conditionvariabl>
 *      1、
*/

class Task
{
public:
    Task(int taskID)
    {
        m_taskID = taskID;
    }
    void doTask()
    {
        std::cout << "handle a task, taskID: " << m_taskID << ", threadID: " << std::this_thread::get_id() << std::endl;
    }
private:
    int m_taskID;
};

std::mutex g_mutex;
std::list<Task*> g_tasklist;
std::condition_variable g_cond;

void consumer_thread()
{
    Task *pTask = NULL;
    while (true)
    {
        std::unique_lock<std::mutex> guard(g_mutex);
        while (g_tasklist.empty())
        {
            //如果获得了互斥锁，但是条件不合适的话，pthread_cond_wait 会释放锁，不往下执行
            //当发生变化后，条件合适，pthread_cond_wait 将直接获得锁
            g_cond.wait(guard);
        }
        pTask = g_tasklist.front();
        g_tasklist.pop_front();
        if (pTask == NULL)
            continue;
        pTask->doTask();
        delete pTask;
        pTask = nullptr;
    }
}

void producer_thread()
{
    int taskID = 0;
    Task * pTask = NULL;
    while (true) {
        pTask = new Task(taskID);
        //使用括号减小guard锁的作用范围
        {
            std::lock_guard<std::mutex> guard(g_mutex);
            g_tasklist.push_back(pTask);
            std::cout << "producer a task, taskID: " << taskID << ", threadID: " << std::this_thread::get_id() << std::endl;;
        }
        //释放信号量，通知消费者线程
        g_cond.notify_one();
        taskID++;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    //创建5个消费者线程
    std::thread ct1(consumer_thread);
    std::thread ct2(consumer_thread);
    std::thread ct3(consumer_thread);
    std::thread ct4(consumer_thread);
    std::thread ct5(consumer_thread);
    //创建1个生产者线程
    std::thread pt(producer_thread);

    pt.join();
    ct1.join();
    ct2.join();
    ct3.join();
    ct4.join();
    ct5.join();
    return 0;
}
