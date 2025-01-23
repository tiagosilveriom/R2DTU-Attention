#pragma once

#include <unordered_set>
#include <vector>
#include <common/Message.h>
#include <msgs/NaoMsgs.h>

class Trajectory;

class Timepoint {
public:
    Timepoint(int32_t time) : timestamp(time) {}

    void add(uint8_t act, float value) {
        actuators.push_back(act);
        values.push_back(value);
    }

private:
    friend class Trajectory;

    int32_t timestamp;
    std::vector<uint8_t> actuators;
    std::vector<float> values;
};

class Trajectory {
public:

    Trajectory() {
    }

    void add(Timepoint timepoint) {
        timepoints.push_back(timepoint);
        num_total_values += timepoint.values.size();
    }

    std::shared_ptr<MessageHeader> pack() {
        MessageBuilder<TrajectoryCmdMsg> msg;

        auto num_timepoints = timepoints.size();

        auto timestamps = (int32_t *) malloc(num_timepoints * sizeof(int32_t));
        auto num_values = (uint8_t*) malloc(num_timepoints * sizeof(uint8_t));
        auto actuators = (uint8_t*) malloc(num_total_values * sizeof(uint8_t));
        auto values = (float*) malloc(num_total_values * sizeof(float));


        auto i = 0u;
        auto p = 0u;
        for (auto& timepoint : timepoints) {
            timestamps[i] = timepoint.timestamp;
            num_values[i] = timepoint.values.size();

            for (auto j = 0; j < num_values[i]; ++j) {
                actuators[p + j] = timepoint.actuators[j];
                values[p + j] = timepoint.values[j];
            }
            p += num_values[i];
            ++i;
        }

        msg->num_timepoints = num_timepoints;
        msg.write_blob(&msg->timestamps, (uint8_t*) timestamps, num_timepoints * sizeof(int32_t));
        msg.write_blob(&msg->num_values, num_values, num_timepoints * sizeof(uint8_t));
        msg.write_blob(&msg->actuators, actuators, num_total_values * sizeof(uint8_t));
        msg.write_blob(&msg->values, (uint8_t*) values, num_total_values * sizeof(float));

        auto header = msg.pack();

        free(values);
        free(actuators);
        free(num_values);
        free(timestamps);

        return header;
    }

private:
    std::vector<Timepoint> timepoints;
    uint32_t num_total_values = 0;
};

