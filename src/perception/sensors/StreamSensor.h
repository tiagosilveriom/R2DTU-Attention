#pragma once

#include <core/Node.h>
#include "../Perception.h"

class StreamSensor : public Sensor {
public:
    StreamSensor(Node* node, std::string channel);

    Option<FrameSet> poll_frames() override;

    const Intrinsics& get_intrinsics() { return intrinsics; }
private:
    Subscriber* subscriber;
    Intrinsics intrinsics;
};
