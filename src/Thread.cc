#include "CurrentThread.h"
#include "Thread.h"
#include <semaphore.h>
#include <stdio.h>

#define BUFFER_SIZE 32

atomic_int Thread::thread_nums_(0);

Thread::Thread(Task task, const string &name) : started_(false), joined_(false), tid_(false), task_(move(task)), name_(name)
{
    set_default_name();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);

    thread_ = shared_ptr<thread>(new thread([&]() {
        tid_ = Current_thread::tid();
        sem_post(&sem);
        task_();
    }));

    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::set_default_name()
{
    int num = ++thread_nums_;
    if(name_.empty())
    {
        char buffer[BUFFER_SIZE] = {0};
        snprintf(buffer, sizeof(buffer), "Thread %d", num);
    }
}