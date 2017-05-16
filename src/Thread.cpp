#include <stdlib.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <sys/time.h>
#include "../headers/Thread.hpp"

using namespace std;

Mutex::Mutex() : mutex(PTHREAD_MUTEX_INITIALIZER) {}

void Mutex::lock() {
    pthread_mutex_lock(&mutex);
}

void Mutex::unlock() {
    pthread_mutex_unlock(&mutex);
}

bool Mutex::isLocked() {
    static Mutex m;
    bool ret = false;
    m.lock();
    int r = pthread_mutex_trylock(&mutex);
    if (r == 0) {    //could lock so it's not locked
        pthread_mutex_unlock(&mutex);
    } else if (r == EBUSY)
        ret = true;
    m.unlock();
    return ret;
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&mutex);
}

ThreadCondition::ThreadCondition() : condition(PTHREAD_COND_INITIALIZER) {}

void ThreadCondition::wait(Mutex &m) {
    pthread_cond_wait(&condition, &m.mutex);
}

void ThreadCondition::timedWait(Mutex &m, long millis) {
    struct timeval tv;
    gettimeofday(&tv, NULL);    //epoch time

    timespec t; //epoch target time
    t.tv_sec = tv.tv_sec + millis / 1000;
    t.tv_nsec = (tv.tv_usec + millis % 1000) * 1000;

    pthread_cond_timedwait(&condition, &m.mutex, &t);
}

void ThreadCondition::signal() {
    pthread_cond_signal(&condition);
}

void ThreadCondition::broadcast() {
    pthread_cond_broadcast(&condition);
}

ThreadCondition::~ThreadCondition() {
    pthread_cond_destroy(&condition);
}

struct passArgToThread {
    Thread *th;
    void *arg;
};

void *wrapper(void *f) {
    void (*_f)(void);
    _f = (void (*)(void)) f;
    _f();
    return nullptr;
}

Thread::Thread(std::function<void()> f, std::function<void()> os) : routine(f), onStop(os), running(false),
                                                                    onStopCalledOnLastRun(false) {}
//(std::function<void *(void *)>)[&f](void*)->void*{f(); return nullptr;}

Thread::~Thread() {
    //cout << "DELETING THREAD!" << endl;
    cancel();
}

void Thread::start() {
    onStopCalledOnLastRun = false;
    pthread_create(&thread, NULL, trick, (void *) this);
}

void Thread::usleep(long millis) {
    this_thread::sleep_for(chrono::milliseconds(millis));
}

void Thread::_onStop() {
    if (onStopCalledOnLastRun)
        return;
    onStopCalledOnLastRun = true;
    onStop();
}

void *Thread::trick(void *c) {//http://stackoverflow.com/a/1151615
    ((Thread *) c)->run();
    return nullptr;
}

void Thread::run() {
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr);
    running = true;
    routine();
    running = false;
    _onStop();
}

bool Thread::isRunning() const {
    return running;
}

void Thread::join() const {
    pthread_join(thread, nullptr);
}

void Thread::cancel() {
    running = false;
    _onStop();
    //cout << "FINISHED CANCELING!" << endl;
    pthread_cancel(thread);
}