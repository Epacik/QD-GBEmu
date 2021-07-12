//
// Created by epat on 11.07.2021.
//

#ifndef GBEMU_GBCLOCK_H
#define GBEMU_GBCLOCK_H

#include <cstdint>
#include <functional>
#include <chrono>


class GbClock {
public:
    explicit GbClock(uint32_t hz);
    ~GbClock();

    void Stop();
    std::shared_ptr<std::function<void()>> OnClockCycle = nullptr;

private:
    bool stopped = false;
    std::unique_ptr<std::chrono::nanoseconds> Period;
};


#endif //GBEMU_GBCLOCK_H
