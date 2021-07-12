//
// Created by epat on 11.07.2021.
//

#include <future>
#include <thread>
#include <iostream>
#include "GbClock.h"

GbClock::GbClock(uint32_t hz) {

    // calculate time between each clock cycle
    Period = std::make_unique<std::chrono::nanoseconds>((int)(1000000000 / hz));

    // spin up a thread to act as a clock
    std::thread clockThread([this]() {
        while(true){
            // just to be able to stop if I ever choose so
            if(stopped) return;

            if (OnClockCycle != nullptr)
                (*OnClockCycle)();

            if(stopped) return;
            std::this_thread::sleep_for(*Period);
        }
    });

    // without that if thread would get aborted, then the whole program would also get aborted with it
    clockThread.detach();
}

void GbClock::Stop() {
    stopped = true;
}


GbClock::~GbClock() = default;
