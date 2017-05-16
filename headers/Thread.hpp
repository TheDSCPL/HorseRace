#pragma once

#include <pthread.h>
#include <functional>

class ThreadCondition;

class Mutex {
public:
    friend class ThreadCondition;

    pthread_mutex_t mutex;
public:
    Mutex();

    void lock();

    void unlock();

    bool isLocked();

    ~Mutex();
};

class ThreadCondition {
    pthread_cond_t condition;
public:
    ThreadCondition();

    void wait(Mutex &m);

    void timedWait(Mutex &m, long millis);

    void signal();

    void broadcast();

    ~ThreadCondition();
};

class Thread {
    pthread_t thread;
    //it is !!!NOT!!! safe for the Thread to suicide in the onStop function
    std::function<void()> routine, onStop;
    volatile bool running, onStopCalledOnLastRun;

    void _onStop();

    static void *trick(void *_c);

    void run();

    Thread(Thread const &);

    void operator=(Thread const &);

public:
    Thread(std::function<void()>, std::function<void()> = []() {});

    ~Thread();

    void start();

    static void usleep(long millis);

    bool isRunning() const;

    void join() const;

    void cancel();
};