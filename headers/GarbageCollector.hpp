#ifndef HORSERACE_GARBAGECOLLECTOR_HPP
#define HORSERACE_GARBAGECOLLECTOR_HPP

#include <set>
#include <functional>
#include "Thread.hpp"

template<class T>
class GarbageCollector {
    //true if the pointer is still being used and thus shouldn't be deleted
    std::function<bool(const T *)> _check;
    //action done on the pointer before forcing the delete
    std::function<void(T *)> _force;
    std::set<T *> pointers;
    bool autoCollectEnabled;
    Thread scheduler;
    unsigned int auto_shakeDown_period = 60 * 1000;
    ThreadCondition threadCondition;
    Mutex deleterMutex;
public:
    unsigned int getAutoShakeDownPeriod() const {
        return auto_shakeDown_period;
    }

    GarbageCollector<T> &setAutoShakeDownPeriod(unsigned int millis) {
        auto_shakeDown_period = millis;
        return *this;
    }

    bool isAutoShakeDownEnabled() const {
        return scheduler.isRunning();
    }

    void setAutoShakeDownState(bool enabled) {
        autoCollectEnabled = enabled;
        if (isAutoShakeDownEnabled() == enabled)
            return;
        if (enabled) {
            scheduler.start();
        } else
            scheduler.cancel();
    }

    //run function<void()> '_force' to stop the instances and then deletes all
    void forceAll() {
        deleterMutex.lock();
        for (T *p: pointers) {
            if (!p)
                continue;
            _force(p);
            delete p;
        }
        pointers.clear();
        deleterMutex.unlock();
    }

    //delete all pointers that have a '_check' false
    void shakeDown() {
        deleterMutex.lock();
        std::set<T *> deleted;
        for (T *p: pointers) {
            if (!p) {
                deleted.insert(p);
                continue;
            }
            //cerr << p << endl;
            if (_check(p))
                continue;
            deleted.insert(p);
            delete p;
        }
        for (T *p: deleted) {
            pointers.erase(p);
        }
        deleterMutex.unlock();
    }

    void add(T *ptr) {
        pointers.insert(ptr);
    }

    void remove(T *ptr) {
        pointers.erase(ptr);
    }

    GarbageCollector(std::function<bool(const T *)> _check, std::function<void(T *)> _force = [](T *) {}) : _check(
            _check), _force(_force), autoCollectEnabled(true), scheduler([this]() {
        Mutex m;
        while (autoCollectEnabled) {
            m.lock();
            shakeDown();
            threadCondition.timedWait(m, auto_shakeDown_period);
            m.unlock();
        }
    }) { setAutoShakeDownState(autoCollectEnabled); }

    ~GarbageCollector() {
        forceAll();
    }
};


#endif