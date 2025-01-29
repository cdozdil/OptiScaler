#pragma once

#include "pch.h"

class OwnedMutex {
private:
    std::mutex mtx;
    uint32_t owner{}; // don't use 0

public:
    void lock(uint32_t _owner) {
        mtx.lock();
        owner = _owner;
    }

    // Only unlocks if owner matches
    void unlockThis(uint32_t _owner) {
        if (owner == _owner) {
            mtx.unlock();
            owner = 0;
        }
    }

    uint32_t getOwner() {
        return owner;
    }
};

class OwnedLockGuard {
private:
    OwnedMutex& _mutex;
    uint32_t _owner_id;

public:
    OwnedLockGuard(OwnedMutex& mutex, uint32_t owner_id) : _mutex(mutex), _owner_id(owner_id) {
        _mutex.lock(_owner_id);
    }

    ~OwnedLockGuard() {
        _mutex.unlockThis(_owner_id);
    }
};
