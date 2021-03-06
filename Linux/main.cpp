#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

/* Mutex
 * API: #include <pthread.h>
 *      1.init mutex
 *      @ pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER
 *      @ int pthread_mutex_init(pthread_mutex_t* restrict mutex, const pthread_mutextattr_t *restrict attr);0:successfully
 *      2.delete mutex
 *      int pthread_mutex_destroy(pthread_mutex_t *mutex);0:successfully
 *      3.lock
 *      int pthread_mutex_lock(pthread_mutex_t *mutex);
 *      4.trylock
 *      int pthread_mutex_trylock(pthread_mutex_t *mutex);
 *      5.unlock
 *      int pthread_mutex_unlock(pthread_mutex_t *mutex);
 *      6.init mutexattr
 *      int pthread_mutexattr_init(pthread_mutexattr_t* attr);
 *      7.delete mutexattr
 *      int pthread_mutexattr_destroy(pthread_mutexattr_t* attr);
 *      8.set mutexattr
 *      int pthread_mutexattr_settype(pthread_mutexattr_t* attr,int type);
 *      9.get mutexattr
 *      int pthread_mutexattr_gettype(const pthread_mutexattr_t*restrict attr, int* restrict type);
 *NOTE:
 *      1.使用 PTHREAD_MUTEX_INITIALIZER 初始化的互斥量无须销毁；
 *      2.不要去销毁一个已经加锁或正在被条件变量使用的互斥体对象
 *      3.创建 mutex 对象后，再对其加锁，加锁后才对其进行解锁操作，解锁后才销毁
 *      4.mutex 锁的类型(pthread_mutex_init 第二个参数)
*/
//! PTHREADMUTEXNORMAL（普通锁)
//个普通锁加锁以后，其他线程会阻塞在 pthread_mutex_lock 调用处， 直到对互斥体加锁的线程释放了锁
/*
pthread_mutex_t g_mutex;
int g_number = 0;
void * worker_thread(void * param)
{
    pthread_t ThId = pthread_self();
    while (1) {
        pthread_mutex_lock(&g_mutex);
        printf("ThId:%d\n",ThId);
        g_number++;
        printf("ThId:%d,value:%d\n",ThId,g_number);
        pthread_mutex_unlock(&g_mutex);
        sleep(1);
    }
}

int main(int argc, char *argv[])
{
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&g_mutex,&mutex_attr);
    pthread_t workthread[5];
    for (int i = 0;i < 5; i++)
    {
        pthread_create(&workthread[i],NULL,worker_thread,NULL);
    }
    for (int i = 0;i < 5; i++)
    {
        pthread_join(workthread[i],NULL);
    }
    pthread_mutex_destroy(&g_mutex);
    pthread_mutexattr_destroy(&mutex_attr);
    return 0;
}
*/
/*
//一个线程如果对一个已经加锁的普通锁再次使用 pthread_mutex_lock 加锁，程序会阻塞在第二次调用 pthread_mutex_lock 代码处
//在这种类型的情况， pthread_mutex_trylock 函数如果拿不到锁，不会阻塞，函数会立即返回，并返回 EBUSY 错误码
int main(int argc, char *argv[])
{
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex,NULL);
    int res = pthread_mutex_lock(&mutex);
    printf("mutex lock,res = %d\n",res);
    res = pthread_mutex_lock(&mutex);//对一个已经加锁的普通锁再次使用 pthread_mutex_lock 加锁，程序会阻塞在第二次调用 pthread_mutex_lock 代码处
    //res = pthread_mutex_trylock(&mutex);//在这种类型的情况， pthread_mutex_trylock 函数如果拿不到锁，不会阻塞，函数会立即返回，并返回 EBUSY 错误码
    printf("mutex lock,res = %d\n",res);
    pthread_mutex_destroy(&mutex);
    return 0;
}
*/
//! PTHREADMUTEXERRORCHECK（检错锁)
//如果一个线程使用 pthread_mutex_lock 对已经加锁的互斥体对象再次加锁，pthread_mutex_lock 会返回 EDEADLK
/*
int main()
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex,&mutex_attr);

    int res = pthread_mutex_lock(&mutex);
    printf("res = %d\n",res);

    res = pthread_mutex_lock(&mutex);
    printf("res = %d\n",res);
    if (res == EDEADLK)
    {
        printf("EDEADLK\n");
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&mutex_attr);
    return 0;
}
*/
//当前线程重复调用 pthread_mutex_lock 会直接返回 EDEADLOCK，其他线程如果对这个互斥体再次调用 pthread_mutex_lock 会阻塞在该函数的调用处
/*
pthread_mutex_t g_mutex;
void *work_thread(void *param)
{
    pthread_t ThId = pthread_self();
    while (1)
    {
        int res = pthread_mutex_lock(&g_mutex);
        if (res == EDEADLK)
        {
            printf("ThId:%d,res:EDEADLK\n");
        }
        else
        {
            printf("ThID:%d,res:%d\n",res);
        }
        //pthread_mutex_unlock(&g_mutex);
        sleep(1);
    }
}
int main()
{
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&g_mutex,&mutex_attr);

    pthread_mutex_lock(&g_mutex);

    pthread_t workthread[5];
    for (int i = 0;i < 5; i++)
    {
        pthread_create(&workthread[i],0,work_thread,NULL);
    }
    for (int i = 0;i < 5; i++)
    {
        pthread_join(workthread[i],NULL);
    }

    pthread_mutex_destroy(&g_mutex);
    pthread_mutexattr_destroy(&mutex_attr);
    return 0;
}
*/
//! PTHREAD_MUTEX_RECURSIVE（嵌套锁）
//该属性允许同一个线程对其持有的互斥体重复加锁，每次成功调
//用 pthread_mutex_lock 一次，该互斥体对象的锁引用计数就会增加一
//次，相反，每次成功调用 pthread_mutex_unlock 一次，锁引用计数就会
//减少一次，当锁引用计数值为 0 时允许其他线程获得该锁，否则其他线
//程调用 pthread_mutex_lock 时尝试获取锁时，会阻塞在那里

/* Semaphore(信号量)
 * API:#include <semaphore.h>
 *      1.init semaphore(信号量对象,信号量是否可以被初始化该信号量的进程 fork 出来的子进程共享，取值为0(不可以共享);1(可以共享),初始状态下资源的数量)
 *      int sem_init(sem_t *sem, int pshared, unsigned int value);
 *      2.销毁一个信号量
 *      int sem_destroy(sem_t *sem);
 *      3.资源计数递增 1,并解锁该信号量对,这样其他由于使用 sem_wait 被阻塞的线程会被唤醒
 *      int sem_post(sem_t *sem);
 *      4.如果当前信号量资源计数为 0，函数 sem_wait 会阻塞调用线程;直到信号量对象的资源计数大于 0 时被唤醒，唤醒后将资源计数递减 1，然后立即返回
 *      int sem_wait(sem_t *sem);
 *      5.函数 sem_trywait 是函数 sem_wait 的非阻塞版本，如果当前信号量对象的资源计数等于 0，sem_trywait 会立即返回不会阻塞调用线程，返回值是 ﹣1，错误码 errno 被设置成 EAGAIN
 *      int sem_trywait(sem_t *sem);
 *      6.函数 sem_timedwait 是带有等待时间的版本，等待时间在第二个参数 abs_timeout 中设置,结构体的定义如下:
 *        strcut timespec
 *        {
 *            time_t tv_sec;
 *            long   tv_nsec;
 *        }
 *      sem_timedwait 在参数 abs_timeout 设置的时间内等待信号量对象的资源计数大于 0，否则超时返回，返回值为 ﹣1，错误码 errno 是 ETIMEDOUT。
 *      当使用 sem_timedwait 时，参数 abs_timeout 不能设置为 NULL，否则程序会在运行时调用 sem_timedwait 产生崩溃。
 *      int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
 * NOTE:
 *      sem_wait、sem_trywait、sem_timedwait 函数将资源计数递减一时会同时锁定信号量对象，因此当资源计数为 1 时，
 *      如果有多个线程调用sem_wait 等函数等待该信号量时，只会有一个线程被唤醒。
 *      当sem_wait 函数返回时，会释放对该信号量的锁。
        sem_wait、sem_trywait、sem_timedwait 函数调用成功后返回值均为0，调用失败返回 ﹣1，可以通过错误码 errno 获得失败原因。
        sem_wait、sem_trywait、sem_timedwait 可以被 Linux 信号中断，被信号中断后，函数立即返回，返回值是 ﹣1，错误码 errno 为 EINTR。
        虽然上述函数没有以 pthread_ 作为前缀，实际使用这个系列的函数时需要链接 pthread 库
*/
/*
#include <semaphore.h>
#include <list>

class Task
{
public:
    Task(int taskId)
    {
        m_taskId = taskId;
    }
    void doTaks()
    {
        printf("consumer taskId:%d,threadId:%d\n",m_taskId,pthread_self());
    }

private:
    int m_taskId;
};

pthread_mutex_t g_mutex;
std::list<Task*> g_tasklist;
sem_t g_sem;

void *consumer_thread(void *param)
{
    Task *pTask = NULL;
    while (1)
    {
        if (0 != sem_wait(&g_sem))
            continue;
        if (g_tasklist.empty())
            continue;
        pthread_mutex_lock(&g_mutex);
        pTask = g_tasklist.front();
        g_tasklist.pop_front();
        pthread_mutex_unlock(&g_mutex);
        pTask->doTaks();
        delete pTask;
        pTask = nullptr;
    }
}

void *producer_thread(void *param)
{
    int taskId = 0;
    Task *pTask = NULL;
    while (1)
    {
        pTask = new Task(taskId);
        pthread_mutex_lock(&g_mutex);
        g_tasklist.emplace_back(pTask);
        printf("producer taskId:%d,threadId:%d\n",taskId,pthread_self());
        pthread_mutex_unlock(&g_mutex);
        sem_post(&g_sem);
        taskId++;
        sleep(1);
    }
}

int main()
{
    pthread_mutex_init(&g_mutex,NULL);
    sem_init(&g_sem,0,0);
    pthread_t consumerthread[5];
    pthread_t producerthread;

    for (int i = 0; i < 5; i++)
    {
        pthread_create(&consumerthread[i],NULL,consumer_thread,NULL);
    }

    pthread_create(&producerthread,NULL,producer_thread,NULL);

    for (int i = 0; i < 5; i++)
    {
        pthread_join(consumerthread[i],NULL);
    }
    pthread_join(producerthread,NULL);

    pthread_mutex_destroy(&g_mutex);
    sem_destroy(&g_sem);
    return 0;
}
*/

/* cond(条件变量)
 * API:#include <semaphore.h>
 *      1.初始化条件变量
 *      @ pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 *      @ int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
 *      2.销毁条件变量
 *      int pthread_cond_destroy(pthread_cond_t * cond);
 *      3.等待条件变量的满足,如果条件变量代表的条件不满足,调用 pthread_cond_wait 的线程会一直等待下去
 *      int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t * restrict mutex);
 *      4.pthread_cond_wait 非阻塞版本,它会在指定时间内等待条件满足,超过参数 abstime 设置的时候后 pthread_cond_timedwait 函数会立即返回
 *      int pthread_cond_timedwait(pthread_cond_t* restrict cond,pthread_mutex_t * restrict mutex, const struct timespec* restrict abstime);
 *      5.唤醒因调用 pthread_cond_wait 等待的线程,一次随机唤醒一个线程
 *      int pthread_cond_signal(pthread_cond_t * cond);
 *      6.唤醒因调用 pthread_cond_wait 等待的线程,可以同时唤醒多个线程
*/
/*
#include <list>
#include <semaphore.h>
class Task
{
public:
    Task(int taskId)
    {
        m_taskId = taskId;
    }
    void doTaks()
    {
        printf("handle , taskId:%d,threadId:%d\n",m_taskId,pthread_self());
    }
private:
    int m_taskId;
};
pthread_mutex_t g_mutex;
std::list<Task*> g_tasklist;
pthread_cond_t g_cond;

void * consumer_thread(void *param)
{
    Task * pTask = NULL;
    while (1)
    {
        pthread_mutex_lock(&g_mutex);
        while (g_tasklist.empty())
        {
            //如果获得了互斥锁,但是条件不合适的话,pthread_cond_wait 会释放锁,不往下执行
            //当发生变化后,条件合适, pthread_cond_wait 将直接获得锁
            //当 pthread_cond_wait 函数阻塞时，它会释放其绑定的互斥体，并阻塞线程，因此在调用该函数前应该对互斥体有个加锁操作
            pthread_cond_wait(&g_cond,&g_mutex);
            //当收到条件信号时， pthread_cond_wait 会返回并对其绑定的互斥体进行加锁，因此在其下面一定有个对互斥体进行解锁的操作
        }
        pTask = g_tasklist.front();
        g_tasklist.pop_front();
        pthread_mutex_unlock(&g_mutex);
        if (pTask == NULL)
            continue;
        pTask->doTaks();;
        delete pTask;
        pTask = NULL;
    }
}

void *producer_thread(void* param)
{
    int taskID = 0;
    Task * pTask = NULL;
    while (1)
    {
        pTask = new Task(taskID);
        pthread_mutex_lock(&g_mutex);
        g_tasklist.emplace_back(pTask);
        printf("producer,taskID:%d,threadID:%d\n",taskID,pthread_self());
        pthread_mutex_unlock(&g_mutex);
        //释放信号量，通知消费者线程
        pthread_cond_signal(&g_cond);
        taskID++;
        sleep(1);
    }
}

int main()
{
    pthread_mutex_init(&g_mutex,NULL);
    pthread_cond_init(&g_cond,NULL);
    pthread_t consumerthread[5];
    for (int i = 0;i < 5;i++)
    {
        pthread_create(&consumerthread[i],NULL,consumer_thread,NULL);
    }
    pthread_t producerthread;
    sleep(1);
    pthread_create(&producerthread,NULL,producer_thread,NULL);
    pthread_join(producerthread,NULL);
    for (int i = 0;i < 5; i++)
    {
        pthread_join(consumerthread[i],NULL);
    }

    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&g_cond);
    return 0;
}
*/

/* rwlock(读写锁)
 * API: #include <pthread.h>
 *      1.初始化读写锁
 *      @ pthread_rwlock_t myrwlock = PTHREAD_RWLOCK_INITIALIZER;
 *      @ int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
 *      2.销毁读写锁
 *      int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
 *      3.请求读锁的系统 API 接口
 *      @ int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
 *      @ int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
 *      @ int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock, const struct timespec *abstime);
 *      4.请求写锁的系统 API 接口
 *      @ int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
 *      @ int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
 *      @ int pthread_rwlock_timedwrlock(pthread_rwlock_t * abstime);
 *      5.锁的释放
 *      int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
 *      6.初始化读写锁的属性
 *      int pthread_rwlockattr_init(pthread_rwlockattr_t * attr);
 *      7.销毁读写锁的属性
 *      int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);
 *      7.设置读写锁的属性
 *      int pthread_rwlockattr_setkid_np(pthread_rwlockattr_t * attr, int pref);
 *      8.查询读写锁的属性
 *      int pthread_rwlockattr_getkind_np(const pthread_rwlockattr_t *attr, int *pref);
 *      pref 即设置读写锁的类型: enum
 *                            {
 *                               //读者优先（即同时请求读锁和写锁时，请求读锁的线程优先获得锁）
                                 PTHREAD_RWLOCK_PREFER_READER_NP,
                                 //不要被名字所迷惑，也是读者优先
                                 PTHREAD_RWLOCK_PREFER_WRITER_NP,
                                 //写者优先（即同时请求读锁和写锁时，请求写锁的线程优先获得锁）
                                 PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
                                 PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_READER_NP
 *                            }
 *NOTE:
 *      1.读锁用于共享模式
 *      @ 如果当前读写锁已经被某线程以读模式占有了，其他线程调用 pthread_rwlock_rdlock （请求读锁）会立刻获得读锁；
 *      @ 如果当前读写锁已经被某线程以读模式占有了，其他线程调用 pthread_rwlock_wrlock （请求写锁）会陷入阻
 *      2.写锁用的是独占模
 *      @ 如果当前读写锁被某线程以写模式占有，无论调用 pthread_rwlock_rdlock 还是 pthread_rwlock_wrlock 都会陷入阻塞，
 *        即写模式下不允许任何读锁请求通过，也不允许任何写锁请求通过，读锁请求和写锁请求都要陷入阻塞，直到线程释放写
 *
 *              *****************************************************************
 *              * 锁当前状态/其他线程请求锁类型          请求读锁           请求写锁    *
 *              *****************************************************************
 *              *         无锁                        通过               通过      *
 *              *****************************************************************
 *              *      已经获得读锁                    通过               阻止      *
 *              *****************************************************************
 *              *      已经获得写锁                    阻止               阻止      *
 *              *****************************************************************
 *
*/
int g_number = 0;
pthread_rwlock_t g_rwlock;

void *read_thread(void *param)
{
    while (1)
    {
        //请求读锁
        pthread_rwlock_rdlock(&g_rwlock);
        printf("read thread ID:%d,g_number:%d\n",pthread_self(),g_number);

        pthread_rwlock_unlock(&g_rwlock);
        sleep(1);
    }
}

void *write_thread(void *param)
{
    while(1)
    {
        //请求写锁
        pthread_rwlock_wrlock(&g_rwlock);
        ++g_number;
        printf("write thread ID:%d,g_number:%d\n",pthread_self(),g_number);
        pthread_rwlock_unlock(&g_rwlock);
        sleep(1);

    }
}

int main()
{
    //!默认属性，其行为是请求读锁的线程优先获得到锁
    //pthread_rwlock_init(&g_rwlock,NULL);
    //!锁对象的属性修改成请求写锁优先
    pthread_rwlockattr_t rwlockattr;
    pthread_rwlockattr_init(&rwlockattr);
    pthread_rwlockattr_setkind_np(&rwlockattr,PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    pthread_rwlock_init(&g_rwlock,&rwlockattr);
    pthread_t readthread[5];
    for (int i = 0;i < 5; i++)
    {
        pthread_create(&readthread[i],NULL,read_thread,NULL);
    }
    pthread_t writethread;
    pthread_create(&writethread,NULL,write_thread,NULL);
    for (int i = 0; i < 5; i++)
    {
        pthread_join(readthread[i],NULL);
    }
    pthread_join(writethread,NULL);

    pthread_rwlock_destroy(&g_rwlock);
    pthread_rwlockattr_destroy(&rwlockattr);

    return 0;
}
