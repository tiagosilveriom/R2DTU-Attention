#include <common/Timer.h>
#include <chrono>
#include <thread>

namespace Timer {

    Timestamp timestamp() {
        return std::chrono::steady_clock::now();
    }


    FrequencyTimer create_fixed_frequency(int frequency) {
        FrequencyTimer timer;
        timer.frame_time = std::chrono::milliseconds(1000) / frequency;
        timer.next_wakeup = timestamp() + timer.frame_time;
        return timer;
    }

    void sleep(FrequencyTimer* timer) {
        std::this_thread::sleep_until(timer->next_wakeup);
        timer->next_wakeup += timer->frame_time;
    }
}
