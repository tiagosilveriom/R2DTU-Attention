
#include "Calibration.h"
#include <l10n/FBT.h>

void Calibration::enter(Robot& r) {
    r.setup_tts(LANG_DANISH, 0.9);
    counter = 0;
}

bool Calibration::tick(Robot& r) {
    if (counter == 0) {
        r.set_head_angles(0.0, 0.0, 500);
    }
    if (counter == 50) {
        r.world.get_source(0).request_calibration();
        r.world.get_source(1).request_calibration();
        r.world.get_source(2).request_calibration();
    }

    if (counter % 1000 == 0) {
        //r.say("If you are looking for the red cube. It is now in box 2. Lasse has moved it, while you were away.");
    }

    bool done = !(r.world.get_source(0).should_calibrate() || r.world.get_source(1).should_calibrate() || r.world.get_source(2).should_calibrate());


    if (done && move_stage == 0) {
        r.calibration_done();
        auto start = r.world.get_source(2).camera_to_world_space({0,0,0});
        auto ahead = r.world.get_source(2).camera_to_world_space({0,0,1.0});
        auto forward = ahead - start;
        auto angle = atan2(forward.x, forward.z);
        r.turn(angle);
        r.current_theta = 0.0f;
        move_stage++;
        counter = 100;
    }

    if (done && move_stage == 1 && counter == 200) {
        r.move_to(glm::vec3 {0.0f, 0.0f, -0.6f});
        r.log_info("Robot", "Calibration done");
        return true;
    }
    ++counter;
    return false;
}
