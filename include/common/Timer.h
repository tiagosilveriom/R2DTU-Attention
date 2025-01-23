#pragma once

#include <stdint.h>
#include <chrono>

namespace Timer {

    using Timestamp = std::chrono::steady_clock::time_point;
    using Duration = std::chrono::microseconds;

    Timestamp timestamp();

    struct FrequencyTimer {
        Timestamp next_wakeup;
        Duration frame_time;
    };

    FrequencyTimer create_fixed_frequency(int frequency);

    void sleep(FrequencyTimer* timer);

}
